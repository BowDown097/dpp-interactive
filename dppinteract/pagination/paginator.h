#pragma once
#include <set>
#include <span>
#include <dpp/snowflake.h>

#define DEFAULT_PAGINATOR_OVERRIDES(paginator_type) \
    paginator_type& set_footer(dppinteract::paginator_footer f) final \
    { dppinteract::paginator::set_footer(f); return *this; } \
    paginator_type& set_start_page_index(int index) final \
    { dppinteract::paginator::set_start_page_index(index); return *this; } \
    paginator_type& set_users(std::span<const dpp::snowflake> user_ids) final \
    { dppinteract::paginator::set_users(user_ids); return *this; } \
    paginator_type& add_user(dpp::snowflake user_id) final \
    { dppinteract::paginator::add_user(user_id); return *this; } \
    paginator_type& set_options(std::span<std::pair<const char*, dppinteract::paginator_action>> opts) final \
    { dppinteract::paginator::set_options(opts); return *this; } \
    paginator_type& add_option(const char* emote, dppinteract::paginator_action action) final \
    { dppinteract::paginator::add_option(emote, action); return *this; } \
    paginator_type& with_default_buttons() final \
    { dppinteract::paginator::with_default_buttons(); return *this; } \

namespace dpp { class button_click_t; class component; class embed; }

namespace dppinteract
{
    class interaction_page;

    enum paginator_action
    {
        paa_no_action,
        paa_forward,
        paa_backward,
        paa_skip_to_end,
        paa_skip_to_start,
        paa_exit
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
        virtual paginator& set_users(std::span<const dpp::snowflake> user_ids);
        virtual paginator& add_user(dpp::snowflake user_id);
        virtual paginator& set_options(std::span<std::pair<const char*, paginator_action>> opts);
        virtual paginator& add_option(const char* emote, paginator_action action);
        virtual paginator& with_default_buttons();

        virtual dpp::embed embed_for(int page_index);
        virtual interaction_page get_or_load_current_page();
        virtual interaction_page get_or_load_page(int page_index) = 0;
        virtual void handle_button_click(const dpp::button_click_t& event);
        virtual int max_page_index() const = 0;

        static paginator_action component_id_to_action(std::string_view id);
        int current_page_index() const;
        dpp::component get_component(bool disable_all) const;
        bool is_interactor(dpp::snowflake user_id) const;
        bool should_disable(paginator_action action, bool disable_all) const;
        bool should_exit() const;
    protected:
        bool exit_flag{};
        paginator_footer footer = paf_page_number;
        std::vector<std::pair<const char*, paginator_action>> options;
        int start_page_index{};
        std::set<dpp::snowflake> user_ids;

        dpp::embed gen_page_embed(interaction_page& page);
    private:
        int m_current_page_index{};
    };
}
