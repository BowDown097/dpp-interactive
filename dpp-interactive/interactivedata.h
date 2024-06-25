#pragma once
#include <chrono>

namespace dpp
{
    class button_click_t;
    class cluster;
    class message;
    class paginator;

    struct interactive_data
    {
        std::unique_ptr<dpp::message> message;
        std::unique_ptr<dpp::paginator> paginator;
        bool reset_timeout_on_input{};
        std::chrono::time_point<std::chrono::steady_clock> timeout_point;
        std::chrono::seconds timeout_secs;
        ~interactive_data();
    };
}
