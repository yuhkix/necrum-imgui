#pragma once

#include "necrum/core/base.h"

// Layout building blocks: panels (titled cards with their own scroll region)
// and columns (equal-width side-by-side groups).
namespace nc
{

enum PanelFlags_
{
	PanelFlags_None = 0,
	PanelFlags_NoTitle = 1 << 0,			// body only, no header strip
	PanelFlags_NoScroll = 1 << 1,			// never show a scrollbar
	PanelFlags_NoSearchHint = 1 << 2, // don't print "No matches" while searching
	PanelFlags_NoPadding = 1 << 3,		// content touches the card edges (custom drawing)
};
using PanelFlags = int;

// Titled card. Always pair with end_panel(), whatever the return value.
//   size.x == 0 -> width of the current column / available width
//   size.y == 0 -> grow to fit content
//   size.y  < 0 -> fill the remaining height (like ImGui: -1 = exactly to the bottom)
//   size.y  > 0 -> fixed height
bool begin_panel(const char* title, ImVec2 size = ImVec2(0, 0), PanelFlags flags = 0);
void end_panel();

// Screen-space rect of the innermost open panel's body (for custom drawing).
ImVec2 panel_body_min();
ImVec2 panel_body_max();

// Equal-width columns. Everything between begin/next/end is placed in the current column.
void begin_columns(int count, float gap = 16.0f);
void next_column();
void end_columns();

// Width available in the current layout slot (column or window).
float available_width();

// Vertical gap.
void spacing(float pixels = 8.0f);

} // namespace nc
