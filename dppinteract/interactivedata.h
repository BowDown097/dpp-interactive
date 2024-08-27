#pragma once
#include "pagination/paginator.h"
#include <chrono>

namespace dppinteract
{
    struct interactive_data
    {
        std::unique_ptr<dppinteract::paginator> paginator;
        bool reset_timeout_on_input{};
        std::chrono::steady_clock::time_point timeout_point;
        std::chrono::seconds timeout_secs;
    };
}
