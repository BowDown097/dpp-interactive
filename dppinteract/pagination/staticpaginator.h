#pragma once
#include "interactionpage.h"
#include "paginator.h"

namespace dppinteract
{
    class static_paginator final : public paginator
    {
    public:
        DEFAULT_PAGINATOR_OVERRIDES(static_paginator)
        static_paginator& set_pages(std::span<const interaction_page> pages);
        static_paginator& add_page(const interaction_page& page);
        interaction_page get_or_load_page(int page_index) override;
        int max_page_index() const override;
    private:
        std::vector<interaction_page> pages;
    };
}
