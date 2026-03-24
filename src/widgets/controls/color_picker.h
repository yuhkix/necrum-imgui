#pragma once
#include "../ui_core.h"

namespace ui
{
bool flat_color_picker(const char* label, float* hue, float* sat, float* val, float def_h = -1.0f, float def_s = -1.0f, float def_v = -1.0f);
} // namespace ui
