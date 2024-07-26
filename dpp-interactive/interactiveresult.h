#pragma once

namespace dpp
{
    enum class interactive_status { Unknown, Success, Timeout };

    template<typename T>
    struct interactive_result
    {
        interactive_status status = interactive_status::Success;
        const T* value;

        bool success() const { return status == interactive_status::Success; }
        bool timed_out() const { return status == interactive_status::Timeout; }
    };
}
