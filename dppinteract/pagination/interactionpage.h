#pragma once
#include "paginator.h"
#include <memory>

namespace dpp { class embed; class embed_author; class embed_field; class embed_footer; class embed_image; class user; }

namespace dppinteract
{
    class interaction_page
    {
    public:
        interaction_page& set_title(std::string_view text);
        interaction_page& set_description(std::string_view text);
        interaction_page& set_footer(const dpp::embed_footer& f);
        interaction_page& set_footer(std::string_view text, std::string_view icon_url);
        interaction_page& set_color(uint32_t col);
        interaction_page& set_timestamp(time_t tstamp);
        interaction_page& set_url(std::string_view u);
        interaction_page& add_field(std::string_view name, std::string_view value, bool is_inline);
        interaction_page& set_author(const dpp::embed_author& a);
        interaction_page& set_author(std::string_view name, std::string_view url, std::string_view icon_url);
        interaction_page& set_image(std::string_view url);
        interaction_page& set_video(std::string_view url);
        interaction_page& set_thumbnail(std::string_view url);
        interaction_page& set_paginator_footer(paginator_footer f, int current_page_index, int max_page_index,
                                               std::span<const dpp::user*> users = {});
        dpp::embed to_embed() const;
    private:
        // smart pointers are used instead of optional for forward declaring
        std::shared_ptr<dpp::embed_author> author;
        std::shared_ptr<uint32_t> color;
        std::string description;
        std::vector<dpp::embed_field> fields;
        std::shared_ptr<dpp::embed_footer> footer;
        std::shared_ptr<dpp::embed_image> image;
        std::shared_ptr<dpp::embed_image> thumbnail;
        time_t timestamp{};
        std::string title;
        std::string url;
        std::shared_ptr<dpp::embed_image> video;
    };
}
