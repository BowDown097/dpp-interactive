#include "interactiveservice.h"
#include "pagination/interactionpage.h"
#include <dpp/cluster.h>

namespace dpp
{
    interactive_service::interactive_service(dpp::cluster* cluster, interactive_service_config config)
        : check_thread(std::make_unique<std::jthread>(&interactive_service::check_data_map, this)),
          cluster(cluster),
          config(config) {}

    void interactive_service::check_data_map()
    {
        std::stop_token st = check_thread->get_stop_token();
        while (!st.stop_requested())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::erase_if(data_map, [](const auto& data_pair) {
                return std::chrono::steady_clock::now() > data_pair.second.timeout_point;
            });
        }
    }

    TASK(void) interactive_service::handle_button_click(const button_click_t& event)
    {
        if (!data_map.contains(event.command.message_id))
        {
            event.reply("This interaction has expired.");
            RETURN_NO_VALUE;
        }

        interactive_data& data = data_map[event.command.message_id];
        if (!data.paginator->is_interactor(event.command.get_issuing_user().id))
        {
            event.reply("You do not have permission to interact with this message.");
            RETURN_NO_VALUE;
        }

        event.reply();
        data.paginator->handle_button_click(event);
        AWAIT(send_or_modify(data.paginator.get(), event.command.channel_id, data.message.get()));

        if (data.reset_timeout_on_input)
            data.timeout_point = std::chrono::steady_clock::now() + data.timeout_secs;

        RETURN_NO_VALUE;
    }

    TASK(message) interactive_service::send_or_modify(paginator* paginator, snowflake channel_id, message* msg)
    {
        component comp = paginator->get_component(false);
        embed embed = paginator->embed_for(paginator->current_page_index());

        if (msg)
        {
            if (!comp.components.empty())
                msg->components = { comp };

            msg->embeds = { embed};

#ifdef DPP_CORO
            co_await cluster->co_message_edit(*msg);
#else
            m_cluster->message_edit(*msg);
#endif

            RETURN(*msg);
        }

        message out_msg(channel_id, embed);
        if (!comp.components.empty())
            out_msg.components = { comp };

#ifdef DPP_CORO
        confirmation_callback_t conf = co_await cluster->co_message_create(out_msg);
        co_return conf.get<message>();
#else
        return m_cluster->message_create_sync(out_msg);
#endif
    }

    TASK(void) interactive_service::send_paginator(std::unique_ptr<paginator> paginator, snowflake channel_id,
                                                   bool reset_timeout_on_input, std::chrono::seconds timeout)
    {
        message msg = AWAIT(send_or_modify(paginator.get(), channel_id));
        std::chrono::seconds timeout_secs = timeout.count() > 0 ? timeout : config.default_timeout;

        data_map.emplace(std::piecewise_construct, std::forward_as_tuple(msg.id),
            std::forward_as_tuple(
                std::make_unique<message>(std::move(msg)),
                std::move(paginator),
                reset_timeout_on_input,
                std::chrono::steady_clock::now() + timeout_secs,
                timeout_secs));
    }
}
