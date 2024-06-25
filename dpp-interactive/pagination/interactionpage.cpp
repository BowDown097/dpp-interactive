#include "interactionpage.h"
#include <dpp/message.h>
#include <dpp/utility.h>
#include <format>

namespace dpp
{
    interaction_page& interaction_page::set_title(std::string_view text)
    {
        title = utility::utf8substr(text, 0, 256);
        return *this;
    }

    interaction_page& interaction_page::set_description(std::string_view text)
    {
        description = utility::utf8substr(text, 0, 4096);
        return *this;
    }

    interaction_page& interaction_page::set_footer(const embed_footer& f)
    {
        footer = std::make_shared<embed_footer>(f);
        return *this;
    }

    interaction_page& interaction_page::set_footer(std::string_view text, std::string_view icon_url)
    {
        footer = std::make_shared<embed_footer>();
        footer->set_text(std::string(text));
        footer->set_icon(std::string(icon_url));
        return *this;
    }

    interaction_page& interaction_page::set_color(uint32_t col)
    {
        // Mask off alpha, as discord doesn't use it
        color = std::make_shared<uint32_t>(col & 0x00FFFFFF);
        return *this;
    }

    interaction_page& interaction_page::set_timestamp(time_t tstamp)
    {
        timestamp = tstamp;
        return *this;
    }

    interaction_page& interaction_page::set_url(std::string_view u)
    {
        url = u;
        return *this;
    }

    interaction_page& interaction_page::add_field(std::string_view name, std::string_view value, bool is_inline)
    {
        if (fields.size() < 25)
        {
            fields.push_back(embed_field {
                .name = utility::utf8substr(name, 0, 256),
                .value = utility::utf8substr(value, 0, 1024),
                .is_inline = is_inline
            });
        }

        return *this;
    }

    interaction_page& interaction_page::set_author(const embed_author& a)
    {
        author = std::make_shared<embed_author>(a);
        return *this;
    }

    interaction_page& interaction_page::set_author(std::string_view name, std::string_view url, std::string_view icon_url)
    {
        author = std::make_shared<embed_author>(embed_author {
            .name = utility::utf8substr(name, 0, 256),
            .url = std::string(url),
            .icon_url = std::string(icon_url)
        });
        return *this;
    }

    interaction_page& interaction_page::set_image(std::string_view url)
    {
        image = std::make_shared<embed_image>(embed_image { .url = std::string(url) });
        return *this;
    }

    interaction_page& interaction_page::set_video(std::string_view url)
    {
        video = std::make_shared<embed_image>(embed_image { .url = std::string(url) });
        return *this;
    }

    interaction_page& interaction_page::set_thumbnail(std::string_view url)
    {
        thumbnail = std::make_shared<embed_image>(embed_image { .url = std::string(url) });
        return *this;
    }

    interaction_page& interaction_page::set_paginator_footer(paginator_footer f, int current_page_index,
                                                             int max_page_index, std::span<const user*> users)
    {
        footer = std::make_shared<embed_footer>();

        if (f & paf_users)
        {
            if (users.empty())
            {
                footer->text += "Interactors: Everyone";
            }
            else if (users.size() == 1)
            {
                footer->text += "Interactor: " + users.front()->get_mention();
                footer->icon_url = users.front()->get_avatar_url();
            }
            else
            {
                std::string usersJoined = users.front()->get_mention();
                for (auto it = std::next(users.begin()); it != users.end(); ++it)
                {
                    usersJoined += ", ";
                    usersJoined += (*it)->get_mention();
                }

                footer->text += "Interactors: " + usersJoined;
            }

            footer->text += '\n';
        }

        if (f & paf_page_number)
            footer->text = std::format("Page {}/{}", current_page_index + 1, max_page_index + 1);

        return *this;
    }

    embed interaction_page::to_embed() const
    {
        embed out = embed()
            .set_description(description)
            .set_timestamp(timestamp)
            .set_title(title)
            .set_url(url);
        out.fields = fields;

        if (author)
            out.set_author(*author);
        if (color)
            out.set_color(*color);
        if (footer)
            out.set_footer(*footer);
        if (image)
            out.set_image(image->url);
        if (thumbnail)
            out.set_thumbnail(thumbnail->url);
        if (video)
            out.set_video(video->url);

        return out;
    }
}
