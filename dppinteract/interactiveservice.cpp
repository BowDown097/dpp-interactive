#include "interactiveservice.h"
#include <dpp/cluster.h>

// https://stackoverflow.com/a/20846873
// hellish workaround, but it works. required for send_paginator callback below
template<class F>
auto make_copyable_function(F&& f)
{
    auto spf = std::make_shared<std::decay_t<F>>(std::forward<F>(f));
    return [spf](auto&&... args) -> decltype(auto) {
        return (*spf)(decltype(args)(args)...);
    };
}

namespace dppinteract
{
    interactive_service::interactive_service(interactive_service_config config)
        : check_thread(std::make_unique<std::jthread>(&interactive_service::check_data_map, this)), config(config) {}

    void interactive_service::check_data_map(std::stop_token stopToken)
    {
        while (!stopToken.stop_requested())
        {
            std::this_thread::sleep_for(config.check_interval);
            std::erase_if(data_map, [](const auto& data_pair) {
                return std::chrono::steady_clock::now() > data_pair.second.timeout_point ||
                       data_pair.second.paginator->should_exit();
            });
        }
    }

    void interactive_service::handle_button_click(const dpp::button_click_t& event)
    {
        if (!data_map.contains(event.command.message_id))
        {
            event.reply("This interaction has expired.");
            return;
        }

        interactive_data& data = data_map[event.command.message_id];
        if (!data.paginator->is_interactor(event.command.get_issuing_user().id))
        {
            event.reply("You do not have permission to interact with this message.");
            return;
        }

        data.paginator->handle_button_click(event);
        event.reply(dpp::ir_update_message, message_for(data.paginator.get(), event.command.channel_id));

        if (data.reset_timeout_on_input)
            data.timeout_point = std::chrono::steady_clock::now() + data.timeout_secs;
    }

    void interactive_service::handle_message_create(const dpp::message_create_t& event)
    {
        for (entity_filter_data_base* filter : entity_filters)
        {
            using MessageCallback = entity_filter_callback<const dpp::message&>;
            if (MessageCallback* callback = dynamic_cast<MessageCallback*>(filter->cb.get()))
            {
                if (callback->func(event.msg))
                {
                    filter->set_result(&event.msg);
                    filter->cv.notify_one();
                }
            }
        }
    }

    dpp::message interactive_service::message_for(paginator* paginator, dpp::snowflake channel_id)
    {
        dpp::component comp = paginator->get_component(false);
        dpp::embed embed = paginator->embed_for(paginator->current_page_index());

        dpp::message out(channel_id, embed);
        if (!comp.components.empty())
            out.components.push_back(comp);

        return out;
    }

    void interactive_service::send_paginator(std::unique_ptr<paginator> paginator, const dpp::message_create_t& event,
                                             bool reset_timeout_on_input, std::chrono::seconds timeout)
    {
        dpp::message msg = message_for(paginator.get(), event.msg.channel_id);
        event.reply(msg, false, make_copyable_function(
                    [paginator = std::move(paginator), reset_timeout_on_input, timeout, this]
                    (const dpp::confirmation_callback_t& conf) mutable
        {
            if (conf.is_error())
            {
                dpp::utility::log_error()(conf);
                return;
            }

            std::chrono::seconds timeout_secs = timeout.count() > 0 ? timeout : config.default_timeout;
            data_map[conf.get<dpp::message>().id] = interactive_data {
                .paginator = std::move(paginator),
                .reset_timeout_on_input = reset_timeout_on_input,
                .timeout_point = std::chrono::steady_clock::now() + timeout_secs,
                .timeout_secs = timeout_secs
            };
        }));
    }

    void interactive_service::setup_event_handlers(dpp::cluster* cluster)
    {
        cluster->on_button_click(std::bind(&interactive_service::handle_button_click, this, std::placeholders::_1));
#ifdef DPP_CORO
        cluster->on_message_create(std::bind(&interactive_service::handle_message_create, this, std::placeholders::_1));
#endif
    }
}
