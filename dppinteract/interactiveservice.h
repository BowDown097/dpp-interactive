#pragma once
#include "entityfilterdata.h"
#include "interactivedata.h"
#include "interactiveresult.h"
#include <unordered_map>

#ifdef DPP_CORO
#include <cassert> // needed for async.h, which doesn't include this for some reason
#include <dpp/coro/async.h>
#endif

namespace dpp { class cluster; class message; class message_create_t; }
namespace std { class jthread; class stop_token; }

namespace dppinteract
{
    struct interactive_service_config
    {
        std::chrono::seconds check_interval{1};
        std::chrono::seconds default_timeout{30};
    };

    class interactive_service
    {
    public:
        template<typename T>
        using callback_function = std::function<void(interactive_result<T>)>;
        template<typename T>
        using filter_function = std::function<bool(const T&)>;

        explicit interactive_service(interactive_service_config config = {});
        void handle_button_click(const dpp::button_click_t& event);
        void send_paginator(std::unique_ptr<paginator> paginator, const dpp::message_create_t& event,
                            bool reset_timeout_on_input = false, std::chrono::seconds timeout = {});
        void setup_event_handlers(dpp::cluster* cluster);

        void handle_message_create(const dpp::message_create_t& event);

        void next_message(filter_function<dpp::message> filter, callback_function<dpp::message> callback,
                          std::chrono::seconds timeout = {})
        { next_entity(std::move(filter), std::move(callback), timeout); }

#ifdef DPP_CORO
        decltype(auto) next_message(filter_function<dpp::message> filter, std::chrono::seconds timeout = {})
        { return next_entity(std::move(filter), timeout); }
#endif
    private:
        std::unique_ptr<std::jthread> check_thread;
        interactive_service_config config;
        std::unordered_map<uint64_t, interactive_data> data_map;
        std::vector<entity_filter_data_base*> entity_filters;

        void check_data_map(std::stop_token stopToken);
        dpp::message message_for(paginator* paginator, dpp::snowflake channel_id);

        template<class T>
        void next_entity(filter_function<T> filter, auto&& cb, std::chrono::seconds timeout = {})
        {
            std::thread t([this, filter = std::move(filter), cb = std::forward<decltype(cb)>(cb), timeout] {
                cb(process_entity(std::move(filter), timeout));
            });
            t.detach();
        }

#ifdef DPP_CORO
        template<class T>
        dpp::async<interactive_result<T>> next_entity(filter_function<T> filter, std::chrono::seconds timeout = {})
        {
            return dpp::async<interactive_result<T>>([this, filter = std::move(filter), timeout](auto&& cb) {
                next_entity(std::move(filter), std::forward<decltype(cb)>(cb), timeout);
            });
        }
#endif

        template<class T>
        interactive_result<T> process_entity(filter_function<T> filter, std::chrono::seconds timeout)
        {
            std::unique_ptr<entity_filter_data<T>> data = std::make_unique<entity_filter_data<T>>(
                std::make_unique<entity_filter_callback<const T&>>(std::move(filter)));
            entity_filters.push_back(data.get());

            std::unique_lock lk(data->mutex);
            std::chrono::seconds timeout_secs = timeout.count() > 0 ? timeout : config.default_timeout;

            bool found_result = data->cv.wait_for(lk, timeout_secs, [&data] { return data->result != nullptr; });
            std::erase(entity_filters, data.get());

            return found_result
                ? interactive_result<T> { .value = data->result }
                : interactive_result<T> { .status = interactive_status::Timeout, .value = data->result };
        }
    };
}
