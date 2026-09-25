#pragma once

#include "necrum/core/base.h"

#include <deque>

// The application shell: a complete main window with header (logo, search,
// tab icons), sidebar (pages of the current tab), animated content area and
// footer. You only register tabs/pages and draw widgets inside them:
//
//     nc::Shell shell({.title = "my tool"});
//     auto& general = shell.add_tab("General", ICON_FA_GEAR);
//     general.add_page("Display", ICON_FA_DESKTOP, [&] {
//         nc::begin_columns(2);
//         if (nc::begin_panel("Window", {0, -1})) { nc::checkbox("VSync", &vsync); }
//         nc::end_panel();
//         nc::next_column();
//         ...
//         nc::end_columns();
//     });
//     // every frame:
//     shell.render();
//
// Search, tab/page switching, fading, toggling with a key and the overlay
// edit mode are handled by the shell.
namespace nc
{

struct Page
{
	std::string title;
	const char* icon = nullptr;
	std::function<void()> draw;
	bool has_hits = true; // updated by search
};

struct Tab
{
	std::string title;
	const char* icon = nullptr;
	std::deque<Page> pages; // deque: references returned by add_page() stay valid
	// Optional strip drawn above every page of this tab (e.g. nc::tab_strip for sub-categories).
	std::function<void()> toolbar;
	bool has_hits = true; // updated by search

	Page& add_page(std::string title, const char* icon, std::function<void()> draw);
};

struct FooterItem
{
	std::string label;
	std::string value;
};

struct ShellConfig
{
	std::string title = "necrum";
	std::string logo_url;				// optional web image shown in the header
	const char* logo_icon = nullptr; // Font Awesome fallback when no logo image is available
	ImVec2 size{680.0f, 470.0f};
	float header_height = 46.0f;
	float footer_height = 22.0f; // 0 hides the footer
	float sidebar_width = 130.0f;
	bool search = true;
	const char* search_hint = "Search features...";
	ImGuiKey toggle_key = ImGuiKey_Insert; // ImGuiKey_None disables toggling
	bool start_open = true;
	std::vector<FooterItem> footer_left;
	std::vector<FooterItem> footer_right;
};

class Shell
{
public:
	explicit Shell(ShellConfig config = {});

	Tab& add_tab(std::string title, const char* icon = nullptr);
	Tab* tab(int index);
	int tab_count() const { return (int)tabs_.size(); }

	ShellConfig& config() { return config_; }

	// Draws the window (when open or fading). Call once per frame.
	void render();

	bool is_open() const { return open_; }
	void set_open(bool open) { open_ = open; }
	void toggle() { open_ = !open_; }

	// Current fade (0 hidden .. 1 fully visible).
	float alpha() const { return alpha_; }
	bool visible() const { return alpha_ > 0.01f; }

	void select(int tab, int page = 0);
	int current_tab() const { return tab_; }
	int current_page() const { return page_; }

	// Window position / size of the last drawn frame.
	ImVec2 window_pos() const { return win_pos_; }
	ImVec2 window_size() const { return config_.size; }

private:
	void draw_header(ImDrawList* dl, ImVec2 pos, float width);
	void draw_search(ImDrawList* dl, ImVec2 pos, float x_max);
	void draw_sidebar(float height);
	void draw_content(float width, float height);
	void draw_footer(ImDrawList* dl, ImVec2 pos, float width);
	void run_search();

	ShellConfig config_;
	std::deque<Tab> tabs_; // deque: references returned by add_tab() stay valid
	int tab_ = 0;
	int page_ = 0;
	int prev_tab_ = -1;
	int prev_page_ = -1;
	bool open_ = true;
	float alpha_ = 0.0f;
	float content_fade_ = 1.0f;
	bool focus_search_ = false;
	ImVec2 win_pos_{0.0f, 0.0f};
	ImGuiID id_ = 0;
};

// Asks the shell not to move its window during the next frame (for pages that
// implement their own drag interactions inside the content area).
void block_window_move();

} // namespace nc
