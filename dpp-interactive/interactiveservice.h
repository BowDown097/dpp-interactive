#pragma once
#include "interactivedata.h"
#include "interactiveresult.h"
#include <unordered_map>

#ifdef DPP_CORO
#include "entityfilterdata.h"
#include <dpp/coro/task.h>
#endif

namespace std { class jthread; class stop_token; }

namespace dpp
{
    class cluster;
    class message;
    class message_create_t;

    struct interactive_service_config
    {
        std::chrono::seconds default_timeout{30};
    };

    class interactive_service
    {
    public:
        explicit interactive_service(interactive_service_config config = {});
        void handle_button_click(const button_click_t& event);
        void send_paginator(std::unique_ptr<paginator> paginator, const message_create_t& event,
                            bool reset_timeout_on_input = false, std::chrono::seconds timeout = {});
        void setup_event_handlers(cluster* cluster);

#ifdef DPP_CORO
        void handle_message_create(const message_create_t& event);
        decltype(auto) next_message(std::function<bool(const message&)> filter, std::chrono::seconds timeout = {})
        { return next_entity(filter, timeout); }
#endif
    private:
        std::unique_ptr<std::jthread> check_thread;
        interactive_service_config config;
        std::unordered_map<uint64_t, interactive_data> data_map;

#ifdef DPP_CORO
        std::vector<entity_filter_data_base*> entity_filters;
#endif

        void check_data_map(std::stop_token stopToken);
        message message_for(paginator* paginator, snowflake channel_id);

#ifdef DPP_CORO
        template<class T>
        task<interactive_result<T>> next_entity(std::function<bool(const T&)> filter, std::chrono::seconds timeout = {})
        {
            std::unique_ptr<entity_filter_data<T>> data = std::make_unique<entity_filter_data<T>>(
                std::make_unique<entity_filter_callback<const T&>>(filter));
            entity_filters.push_back(data.get());

            std::unique_lock lk(data->mutex);
            std::chrono::seconds timeout_secs = timeout.count() > 0 ? timeout : config.default_timeout;

            bool found_result = data->cv.wait_for(lk, timeout_secs, [&data] { return data->result != nullptr; });
            std::erase(entity_filters, data.get());

            co_return found_result
                ? interactive_result<T> { .value = data->result }
                : interactive_result<T> { .status = interactive_status::Timeout, .value = data->result };
        }
#endif
    };
}
