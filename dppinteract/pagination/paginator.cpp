#include "paginator.h"
#include "interactionpage.h"
#include <charconv>
#include <dpp/cache.h>
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <dpp/unicode_emoji.h>

namespace dppinteract
{
    paginator& paginator::set_footer(paginator_footer f)
    {
        footer = f;
        return *this;
    }

    paginator& paginator::set_start_page_index(int index)
    {
        start_page_index = index;
        return *this;
    }

    paginator& paginator::set_users(std::span<const dpp::snowflake> user_ids)
    {
        this->user_ids.insert(user_ids.begin(), user_ids.end());
        return *this;
    }

    paginator& paginator::add_user(dpp::snowflake user_id)
    {
        user_ids.insert(user_id);
        return *this;
    }

    paginator& paginator::set_options(std::span<std::pair<const char*, paginator_action>> opts)
    {
        options.assign(opts.begin(), opts.end());
        return *this;
    }

    paginator& paginator::add_option(const char* emote, paginator_action action)
    {
        options.push_back(std::make_pair(emote, action));
        return *this;
    }

    paginator& paginator::with_default_buttons()
    {
        options.clear();

        add_option(dpp::unicode_emoji::left_arrow, paa_backward);
        add_option(dpp::unicode_emoji::right_arrow, paa_forward);
        add_option(dpp::unicode_emoji::previous_track, paa_skip_to_start);
        add_option(dpp::unicode_emoji::next_track, paa_skip_to_end);
        add_option(dpp::unicode_emoji::stop_sign, paa_exit);

        return *this;
    }

    int paginator::current_page_index() const
    {
        return m_current_page_index;
    }

    dpp::embed paginator::embed_for(int page_index)
    {
        interaction_page p = get_or_load_page(page_index);
        return gen_page_embed(p);
    }

    dpp::embed paginator::gen_page_embed(interaction_page& page)
    {
        if (footer & paf_users)
        {
            auto user_objs = user_ids
                | std::views::transform([](dpp::snowflake user_id) { return dpp::find_user(user_id); })
                | std::views::filter([](const dpp::user* user) { return user != nullptr; });
            std::vector<const dpp::user*> user_objs_vec(user_objs.begin(), user_objs.end());
            page.set_paginator_footer(footer, current_page_index(), max_page_index(), user_objs_vec);
        }
        else if (max_page_index() > 0)
        {
            page.set_paginator_footer(footer, current_page_index(), max_page_index());
        }

        return page.to_embed();
    }

    dpp::component paginator::get_component(bool disable_all) const
    {
        dpp::component out;

        for (const auto& [emote, action] : options)
        {
            out.add_component(
                dpp::component()
                    .set_type(dpp::cot_button)
                    .set_emoji(emote)
                    .set_style(action == paa_exit ? dpp::cos_danger : dpp::cos_primary)
                    .set_id(std::to_string(action))
                    .set_disabled(should_disable(action, disable_all))
            );
        }

        return out;
    }

    interaction_page paginator::get_or_load_current_page()
    {
        return get_or_load_page(m_current_page_index);
    }

    void paginator::handle_button_click(const dpp::button_click_t& event)
    {
        paginator_action action = component_id_to_action(event.custom_id);
        switch (action)
        {
        case paa_no_action:
            break;
        case paa_forward:
            m_current_page_index++;
            break;
        case paa_backward:
            m_current_page_index--;
            break;
        case paa_skip_to_end:
            m_current_page_index = max_page_index();
            break;
        case paa_skip_to_start:
            m_current_page_index = start_page_index;
            break;
        case paa_exit:
            exit_flag = true;
            break;
        }
    }

    paginator_action paginator::component_id_to_action(std::string_view id)
    {
        uint8_t action_num{};
        if (auto [_, err] = std::from_chars(id.data(), id.data() + id.size(), action_num); err != std::errc())
            throw std::logic_error("Failed to parse paginator_action from component ID - this should never happen!");
        if (action_num > paa_exit)
            throw std::out_of_range("Paginator component ID is greater than the max paginator_action - this should never happen!");
        return static_cast<paginator_action>(action_num);
    }

    bool paginator::is_interactor(dpp::snowflake user_id) const
    {
        return user_ids.contains(user_id);
    }

    bool paginator::should_disable(paginator_action action, bool disable_all) const
    {
        if (disable_all)
            return true;

        switch (action)
        {
        case paa_forward:
            return m_current_page_index == max_page_index();
        case paa_skip_to_end:
            return max_page_index() <= 0 || m_current_page_index == max_page_index();
        case paa_backward:
        case paa_skip_to_start:
            return m_current_page_index == 0;
        default:
            return false;
        }
    }

    bool paginator::should_exit() const
    {
        return exit_flag;
    }
}
