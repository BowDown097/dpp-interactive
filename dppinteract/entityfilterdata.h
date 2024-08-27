#pragma once
#include <condition_variable>
#include <functional>

namespace dppinteract
{
    struct entity_filter_callback_base
    {
        virtual ~entity_filter_callback_base() = default;
    };

    template<typename... Args>
    struct entity_filter_callback : entity_filter_callback_base
    {
        std::function<bool(Args...)> func;
        explicit entity_filter_callback(std::function<bool(Args...)> f) : func(std::move(f)) {}
    };

    struct entity_filter_data_base
    {
        std::unique_ptr<entity_filter_callback_base> cb;
        std::condition_variable cv;
        std::mutex mutex;

        explicit entity_filter_data_base(std::unique_ptr<entity_filter_callback_base> cb)
            : cb(std::move(cb)) {}

        virtual void set_result(const void* result) = 0;
    };

    template<typename T>
    struct entity_filter_data : entity_filter_data_base
    {
        const T* result{};

        explicit entity_filter_data(std::unique_ptr<entity_filter_callback_base> cb)
            : entity_filter_data_base(std::move(cb)) {}

        void set_result(const void* result) override
        {
            this->result = static_cast<const T*>(result);
        }
    };
}
