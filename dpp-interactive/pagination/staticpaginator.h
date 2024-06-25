#pragma once
#include "interactionpage.h"
#include "paginator.h"
#include <dpp/snowflake.h>

namespace dpp
{
    class static_paginator final : public paginator
    {
    public:
        DEFAULT_PAGINATOR_OVERRIDES(static_paginator)

        static_paginator& set_pages(std::span<const interaction_page> pages);
        static_paginator& add_page(const interaction_page& page);

        const interaction_page& get_or_load_current_page() const override;
        const interaction_page& get_or_load_page(int page_index) const override;
        int max_page_index() const override;

        embed embed_for(int page_index) override;
    private:
        std::vector<interaction_page> pages;
    };
}
