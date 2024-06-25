#pragma once
#include "interactivedata.h"
#include "utils/ezcoro.h"
#include <unordered_map>

namespace std { class jthread; }

namespace dpp
{
    struct interactive_service_config
    {
        std::chrono::seconds default_timeout{30};
    };

    class interactive_service
    {
    public:
        explicit interactive_service(dpp::cluster* cluster, interactive_service_config config = {});
        TASK(void) handle_button_click(const button_click_t& event);
        TASK(void) send_paginator(std::unique_ptr<paginator> paginator, snowflake channel_id,
                                  bool reset_timeout_on_input = false, std::chrono::seconds timeout = {});
    private:
        std::unique_ptr<std::jthread> check_thread;
        dpp::cluster* cluster;
        interactive_service_config config;
        std::unordered_map<uint64_t, interactive_data> data_map;

        void check_data_map();
        TASK(message) send_or_modify(paginator* paginator, snowflake channel_id, message* msg = nullptr);
    };
}
