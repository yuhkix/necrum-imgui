#pragma once

#include "color.h"

namespace nc
{

// Every color and metric a widget draws with comes from the active Theme.
// Nothing in the framework hard-codes a palette, so swapping the theme (or
// editing a single token at runtime) restyles the whole UI.
struct Theme
{
	std::string name = "Custom";
	bool dark = true;

	// Accent. Edit `accent` (HSV) and call refresh(), or let nc::new_frame() do it.
	HSV accent{265.91f, 0.69f, 1.0f, 1.0f};

	// Surfaces
	ImU32 window_bg = 0;
	ImU32 window_border = 0;
	ImU32 header_bg = 0;
	ImU32 sidebar_bg = 0;
	ImU32 content_bg = 0;
	ImU32 footer_bg = 0;
	ImU32 panel_bg = 0;
	ImU32 panel_header_bg = 0;
	ImU32 panel_border = 0;
	ImU32 popup_bg = 0;
	ImU32 popup_border = 0;
	ImU32 field_bg = 0;			// sliders, inputs, combo buttons
	ImU32 field_border = 0;
	ImU32 control_off = 0;	// unchecked checkbox / toggle track
	ImU32 divider = 0;
	ImU32 hover = 0;				// hover highlight tint (light on dark themes, dark on light themes)
	ImU32 shadow = 0;
	ImU32 badge_bg = 0;

	// Text
	ImU32 text = 0;					// primary text
	ImU32 text_label = 0;		// widget labels
	ImU32 text_dim = 0;			// secondary text, inactive icons
	ImU32 text_disabled = 0;

	// Status colors (toasts, badges)
	ImU32 success = IM_COL32(46, 204, 113, 255);
	ImU32 info = IM_COL32(52, 152, 219, 255);
	ImU32 warning = IM_COL32(241, 196, 15, 255);
	ImU32 error = IM_COL32(231, 76, 60, 255);

	// Metrics
	float window_rounding = 5.0f;
	float panel_rounding = 4.0f;
	float frame_rounding = 3.0f;
	float popup_rounding = 4.0f;
	float glow = 1.0f;						// accent glow strength, 0 disables glows
	float control_height = 18.0f; // combo buttons, inputs, buttons
	float slider_height = 14.0f;
	float checkbox_size = 11.0f;
	float row_height = 20.0f;			// popup / list rows
	float panel_header_height = 28.0f;
	ImVec2 panel_padding{24.0f, 16.0f};

	// Derived from `accent` by refresh()
	ImU32 accent_col = 0;
	ImU32 accent_dim = 0;
	ImU32 accent_text = 0; // readable text drawn on top of an accent fill

	void refresh();
};

// The active theme. Mutable on purpose: pages can bind color pickers straight to tokens.
Theme& theme();
void set_theme(const Theme& t);

// Pushes the active theme into ImGuiStyle so stock ImGui widgets match.
void apply_imgui_style(const Theme& t = theme());

namespace themes
{
Theme necrum();		// near-black, violet accent (default)
Theme crimson();	// near-black, red accent (the original necrum look)
Theme graphite(); // softer dark gray, blue accent
Theme daylight(); // light theme

struct Preset
{
	const char* name;
	Theme (*make)();
};
const std::vector<Preset>& presets();
} // namespace themes

} // namespace nc
