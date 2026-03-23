#pragma once
#include "../ui_core.h"
#include "menu/theme.h"

namespace ui
{
void draw_panel(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* title, float rounding = 4.0f);
} // namespace ui
