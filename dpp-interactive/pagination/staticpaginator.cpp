#include "staticpaginator.h"
#include <dpp/cache.h>
#include <dpp/message.h>

namespace dpp
{
    static_paginator& static_paginator::set_pages(std::span<const interaction_page> pages)
    {
        this->pages.assign(pages.begin(), pages.end());
        return *this;
    }

    static_paginator& static_paginator::add_page(const interaction_page& page)
    {
        pages.push_back(page);
        return *this;
    }

    embed static_paginator::embed_for(int page_index)
    {
        interaction_page& page = pages.at(page_index);
        return gen_page_embed(page);
    }

    const interaction_page& static_paginator::get_or_load_current_page() const
    {
        return get_or_load_page(current_page_index());
    }

    const interaction_page& static_paginator::get_or_load_page(int page_index) const
    {
        return pages.at(page_index);
    }

    int static_paginator::max_page_index() const
    {
        return pages.size() - 1;
    }
}
