#pragma once
#include "interactivedata.h"
#include <unordered_map>

namespace std { class jthread; class stop_token; }

namespace dpp
{
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
    private:
        std::unique_ptr<std::jthread> check_thread;
        interactive_service_config config;
        std::unordered_map<uint64_t, interactive_data> data_map;

        void check_data_map(std::stop_token stopToken);
        message message_for(paginator* paginator, snowflake channel_id);
    };
}
