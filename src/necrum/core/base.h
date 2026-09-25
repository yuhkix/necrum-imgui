#pragma once

// Shared includes and small math helpers used across the whole framework.
// Everything in src/necrum only depends on Dear ImGui and the C++ standard
// library; platform specific code lives in src/necrum/platform.

#include "ext/imgui/imgui.h"
#include "ext/imgui/imgui_internal.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace nc
{

template <typename T>
constexpr T clamp(T v, T lo, T hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}

constexpr float saturate(float v)
{
	return clamp(v, 0.0f, 1.0f);
}

constexpr float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

inline ImVec2 lerp(const ImVec2& a, const ImVec2& b, float t)
{
	return ImVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

// Returns the end of the visible part of a label ("Visible##hidden_id").
inline const char* label_end(const char* label)
{
	return ImGui::FindRenderedTextEnd(label);
}

inline bool has_visible_label(const char* label)
{
	return label && label_end(label) > label;
}

// Screen anchors used by notifications, overlays and the watermark.
enum class Corner
{
	TopLeft,
	TopCenter,
	TopRight,
	BottomLeft,
	BottomCenter,
	BottomRight,
};

// Top-left position of a `size` box placed in `corner` of the display with `margin`.
inline ImVec2 anchor_position(Corner corner, ImVec2 size, float margin)
{
	ImVec2 d = ImGui::GetIO().DisplaySize;
	float x = margin, y = margin;
	switch (corner)
	{
	case Corner::TopCenter:
	case Corner::BottomCenter:
		x = (d.x - size.x) * 0.5f;
		break;
	case Corner::TopRight:
	case Corner::BottomRight:
		x = d.x - size.x - margin;
		break;
	default:
		break;
	}
	if (corner == Corner::BottomLeft || corner == Corner::BottomCenter || corner == Corner::BottomRight)
		y = d.y - size.y - margin;
	return ImVec2(x, y);
}

inline bool is_bottom(Corner c)
{
	return c == Corner::BottomLeft || c == Corner::BottomCenter || c == Corner::BottomRight;
}

} // namespace nc
