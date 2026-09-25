#pragma once

#include "necrum/core/color.h"
#include "necrum/core/input.h"

// Immediate-mode widget library.
//
// Conventions shared by every widget:
//  * `label` follows ImGui rules: "Visible text##unique_id". Hidden labels
//    ("##id") are fine; the visible part is also what search matches against.
//  * Stacked widgets (slider, combo, listbox, input, progress) draw their label
//    on a line above the control and use ImGui::CalcItemWidth() for width.
//    Inline widgets (checkbox, toggle, keybind, color) span the available width.
//  * Return value: true when the value changed (buttons: when clicked).
//  * Everything is animated, themed through nc::theme() and filtered by
//    nc::search automatically.
namespace nc
{

// ---- Text -----------------------------------------------------------------
void text(const char* fmt, ...) IM_FMTARGS(1);
void text_dim(const char* fmt, ...) IM_FMTARGS(1);
void text_colored(ImU32 col, const char* fmt, ...) IM_FMTARGS(2);
void text_wrapped(const char* fmt, ...) IM_FMTARGS(1);

// Dim caption for the widget that follows (searchable).
void label(const char* text);

// Horizontal divider, optionally with a caption.
void separator(const char* caption = nullptr);

// Small "(?)" marker on the current line that shows a tooltip.
void help_marker(const char* text);

// Themed tooltip for the previous item when hovered.
void tooltip(const char* fmt, ...) IM_FMTARGS(1);

// ---- Buttons & toggles ----------------------------------------------------
enum class ButtonStyle
{
	Default, // outlined
	Primary, // accent filled
	Ghost,	 // no frame until hovered
	Danger,
};

bool button(const char* label, const ImVec2& size = ImVec2(0, 0), ButtonStyle style = ButtonStyle::Default);
bool icon_button(const char* id, const char* icon, float size = 22.0f, const char* tip = nullptr);

bool checkbox(const char* label, bool* v);
bool toggle(const char* label, bool* v); // switch style
bool radio(const char* label, int* v, int value);

// ---- Sliders --------------------------------------------------------------
// `display_scale` multiplies the value only for display, e.g. 0..1 shown as percent
// with fmt "%.0f%%" and display_scale 100.
bool slider(const char* label, float* v, float min, float max, const char* fmt = "%.2f", float display_scale = 1.0f);
bool slider(const char* label, int* v, int min, int max, const char* fmt = "%d");
bool range_slider(const char* label, float* lo, float* hi, float min, float max, const char* fmt = "%.2f - %.2f",
									float display_scale = 1.0f);
bool range_slider(const char* label, int* lo, int* hi, int min, int max, const char* fmt = "%d - %d");

// ---- Selection ------------------------------------------------------------
// `allow_none`: clicking the selected item again clears the selection (-1).
bool combo(const char* label, int* selected, const char* const items[], int count, bool allow_none = false);
bool combo(const char* label, int* selected, const std::vector<std::string>& items, bool allow_none = false);
bool multi_combo(const char* label, bool* selected, const char* const items[], int count);
bool multi_combo(const char* label, uint32_t* mask, const char* const items[], int count);
bool listbox(const char* label, int* selected, const char* const items[], int count, float height = 120.0f);
bool listbox(const char* label, int* selected, const std::vector<std::string>& items, float height = 120.0f);

// Horizontal button group: [ Low | Medium | High ]
bool segmented(const char* label, int* selected, const char* const items[], int count);

// Icon / text strip with animated underline (sub-navigation above page content).
bool tab_strip(const char* id, int* selected, const char* const items[], int count, bool icons = true);

// ---- Input ----------------------------------------------------------------
bool input_text(const char* label, char* buf, size_t buf_size, const char* hint = nullptr,
								ImGuiInputTextFlags flags = 0);
bool input_text(const char* label, std::string* str, const char* hint = nullptr, ImGuiInputTextFlags flags = 0);
bool input_int(const char* label, int* v, int step = 1, int min = INT32_MIN, int max = INT32_MAX);
bool input_float(const char* label, float* v, float step = 0.1f, float min = -FLT_MAX, float max = FLT_MAX,
								 const char* fmt = "%.2f");

// Keybind: label on the left, "[ KEY ]" on the right. Left click to listen,
// Escape clears, right click picks the mode (toggle / hold / always).
bool keybind(const char* label, Keybind* bind);

// Color: label on the left, swatch on the right. Left click opens the picker,
// right click offers copy / paste / reset. `reset` enables "Reset".
bool color_edit(const char* label, HSV* color, const HSV* reset = nullptr, bool alpha = false);

// Swatch/keybind attached to the right end of the previous line, e.g.
//   nc::checkbox("Box", &box); nc::inline_color("##box_col", &box_col);
bool inline_color(const char* id, HSV* color, const HSV* reset = nullptr, bool alpha = false);
bool inline_keybind(const char* id, Keybind* bind);

// ---- Display --------------------------------------------------------------
void progress_bar(const char* label, float fraction, const char* overlay = nullptr);
void spinner(const char* id, float radius = 8.0f, float thickness = 2.0f);
void badge(const char* text, ImU32 color = 0);
void plot_lines(const char* label, const float* values, int count, float min = FLT_MAX, float max = FLT_MAX,
								float height = 50.0f);
void plot_histogram(const char* label, const float* values, int count, float min = FLT_MAX, float max = FLT_MAX,
										float height = 50.0f);

// Live graph of the framework animation curve (useful next to a speed slider).
void animation_preview(const char* id, float height = 50.0f);

// ---- Popups ---------------------------------------------------------------
// Themed popup helpers: begin_popup() pushes the framework popup style.
bool begin_popup(const char* id, float width = 0.0f, ImGuiWindowFlags flags = 0);
void end_popup();
bool popup_item(const char* label, bool selected = false, bool close_on_click = true);

// Modal confirm dialog. Call every frame; open with ImGui::OpenPopup(id).
// Returns 1 on confirm, 0 on cancel, -1 while undecided/closed.
int confirm_dialog(const char* id, const char* title, const char* message, const char* confirm = "Confirm",
									 const char* cancel = "Cancel");

} // namespace nc
