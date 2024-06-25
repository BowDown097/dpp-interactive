#pragma once
#include <map>
#include <set>
#include <span>
#include <string_view>

#define DEFAULT_PAGINATOR_OVERRIDES(paginator_type) \
    paginator_type& set_footer(dpp::paginator_footer f) final \
    { dpp::paginator::set_footer(f); return *this; } \
    paginator_type& set_start_page_index(int index) final \
    { dpp::paginator::set_start_page_index(index); return *this; } \
    paginator_type& set_users(std::span<const snowflake> user_ids) final \
    { dpp::paginator::set_users(user_ids); return *this; } \
    paginator_type& add_user(snowflake user_id) final \
    { dpp::paginator::add_user(user_id); return *this; } \
    paginator_type& set_options(const std::map<const char*, dpp::paginator_action>& opts) final \
    { dpp::paginator::set_options(opts); return *this; } \
    paginator_type& add_option(const char* emote, dpp::paginator_action action) final \
    { dpp::paginator::add_option(emote, action); return *this; } \
    paginator_type& with_default_buttons() final \
    { dpp::paginator::with_default_buttons(); return *this; } \

namespace dpp
{
    class button_click_t;
    class component;
    class embed;
    class interaction_page;
    class snowflake;

    enum paginator_action
    {
        paa_no_action,
        paa_forward,
        paa_backward,
        paa_skip_to_end,
        paa_skip_to_start,
        paa_exit,
        paa_jump
    };

    enum paginator_footer
    {
        paf_none = 1 << 0,
        paf_page_number = 1 << 1,
        paf_users = 1 << 2
    };

    class paginator
    {
    public:
        virtual ~paginator() = default;

        virtual paginator& set_footer(paginator_footer f);
        virtual paginator& set_start_page_index(int index);
        virtual paginator& set_users(std::span<const snowflake> user_ids);
        virtual paginator& add_user(snowflake user_id);
        virtual paginator& set_options(const std::map<const char*, paginator_action>& opts);
        virtual paginator& add_option(const char* emote, paginator_action action);
        virtual paginator& with_default_buttons();

        virtual embed embed_for(int page_index) = 0;
        virtual const interaction_page& get_or_load_current_page() const = 0;
        virtual const interaction_page& get_or_load_page(int page_index) const = 0;
        virtual void handle_button_click(const button_click_t& event);
        virtual int max_page_index() const = 0;

        int current_page_index() const;
        component get_component(bool disable_all) const;
        static paginator_action id_to_action(std::string_view component_id);
        bool is_interactor(snowflake user_id) const;
        bool should_disable(paginator_action action, bool disable_all) const;
    protected:
        paginator_footer footer = paf_page_number;
        std::map<const char*, paginator_action> options;
        int start_page_index{};
        std::set<snowflake> user_ids;

        embed gen_page_embed(interaction_page& page);
    private:
        int m_current_page_index{};
    };
}
