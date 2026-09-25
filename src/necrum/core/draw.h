#pragma once

#include "theme.h"

// Low-level drawing primitives shared by widgets, the app shell and user code.
// All functions respect the current style alpha (see nc::styled).
namespace nc::draw
{

// Soft layered glow around a rectangle. `tint` defaults to the theme accent.
void glow(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float strength = 1.0f, ImU32 tint = 0);

// Accent filled rect with glow.
void accent_fill(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float alpha = 1.0f);

// Drop shadow behind a card.
void shadow(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float alpha = 1.0f);

// Vertical gradient that fades out upwards from `b.y` (the "accent underline glow").
void underline_glow(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float alpha, float height = 10.0f);

// Panel/card frame: header strip + body + border + accent top arc + title text.
// Returns the y coordinate where the body starts.
float card(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* title, float rounding = -1.0f,
					 float header_height = -1.0f);

// Down-pointing chevron rotated by `open` (0 = down, 1 = up).
void chevron(ImDrawList* dl, ImVec2 center, float size, float open, ImU32 col);

// Check mark inside a box; `t` animates the stroke.
void check_mark(ImDrawList* dl, ImVec2 min, float size, ImU32 col, float t = 1.0f);

// Text helpers
enum class Align
{
	Left,
	Center,
	Right
};
void text(ImDrawList* dl, ImVec2 pos, ImU32 col, const char* text, const char* text_end = nullptr);
void text_aligned(ImDrawList* dl, ImVec2 anchor, Align align, float center_y, ImU32 col, const char* text,
									const char* text_end = nullptr);
void text_font(ImDrawList* dl, ImFont* font, float size, ImVec2 pos, ImU32 col, const char* text,
							 const char* text_end = nullptr);
ImVec2 text_size(ImFont* font, float size, const char* text, const char* text_end = nullptr);

// Cross-fading text: when `text` changes for `id` the old string fades/slides out.
void text_swap(ImDrawList* dl, ImGuiID id, float x, float center_y, Align align, ImU32 col, const char* text);

} // namespace nc::draw
