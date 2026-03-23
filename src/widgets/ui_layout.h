#pragma once

#include "ui_controls.h"
#include <functional>

namespace ui
{
void draw_search_bar(float w);
void draw_header(float w);
void draw_sidebar(float h);
void draw_weapon_tabs(float w);
void draw_content(float w, float h, const std::function<void()>& content_draw);
void draw_footer(float w);
void draw_watermark();
} // namespace ui
