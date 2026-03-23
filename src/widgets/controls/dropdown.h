#pragma once
#include "../ui_core.h"

namespace ui
{
extern const ImVec4 popup_bg;

void dd_draw_button(ImDrawList* dl, ImVec2 bmin, ImVec2 bmax, float h, const char* display, float anim_open,
										ImGuiID text_anim_id = 0);
bool dd_item(const char* text, bool selected, bool close_on_click, float forced_w = 0);
bool dd_item_check(const char* text, bool selected, float forced_w = 0);
bool flat_dropdown_single(const char* id, int* sel, const char** items, int count);
bool flat_dropdown_multi(const char* id, bool* sels, const char** items, int count);
} // namespace ui
