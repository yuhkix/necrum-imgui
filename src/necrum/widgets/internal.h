#pragma once

// Helpers shared by widget implementations. Not part of the public API.

#include "necrum/core/anim.h"
#include "necrum/core/draw.h"
#include "necrum/core/fonts.h"
#include "necrum/core/search.h"
#include "necrum/core/theme.h"
#include "widgets.h"

namespace nc::detail
{

inline ImVec4 f4(ImU32 c)
{
	return ImGui::ColorConvertU32ToFloat4(c);
}

// Draws the caption line of a stacked widget and reserves its space.
// Returns the width available for the control below it.
float stacked_label(const char* label, const char* right_text = nullptr);

// Pushes the framework popup look (padding, rounding, colors). Pair with pop_popup_style().
void push_popup_style(ImVec2 padding = ImVec2(0.0f, 2.0f));
void pop_popup_style();

// A selectable row inside popups / list boxes.
enum class RowStyle
{
	Plain,	// accent bar when selected
	Check,	// check box on the left (multi select)
	Radio,	// radio dot on the left
};
bool list_row(const char* text, bool selected, float width, RowStyle style = RowStyle::Plain);

// Frame used by combo buttons and inputs. `focus` animates the accent underline.
void field_frame(ImDrawList* dl, ImVec2 min, ImVec2 max, float hover, float focus);

// Hover/active animation for the last item.
struct ItemAnim
{
	float hover;
	float active;
};
ItemAnim item_anim(ImGuiID id);

} // namespace nc::detail
