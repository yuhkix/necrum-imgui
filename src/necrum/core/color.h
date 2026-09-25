#pragma once

#include "base.h"

namespace nc
{

// Hue in degrees [0, 360), saturation/value in [0, 1].
struct HSV
{
	float h = 0.0f;
	float s = 1.0f;
	float v = 1.0f;
	float a = 1.0f;

	ImU32 u32(float alpha_mul = 1.0f) const;
	ImVec4 rgba() const;

	static HSV from_u32(ImU32 col);
	static HSV from_rgb(float r, float g, float b, float a = 1.0f);

	bool operator==(const HSV& o) const { return h == o.h && s == o.s && v == o.v && a == o.a; }
	bool operator!=(const HSV& o) const { return !(*this == o); }
};

constexpr ImU32 rgba(int r, int g, int b, int a = 255)
{
	return IM_COL32(r, g, b, a);
}

ImU32 hsv(float h_deg, float s, float v, float a = 1.0f);

// Linear blend of two packed colors (all channels, including alpha).
ImU32 lerp_color(ImU32 a, ImU32 b, float t);

// Multiplies the alpha channel of `c` by `mul`.
ImU32 alpha_mul(ImU32 c, float mul);

// Replaces the alpha channel of `c`.
ImU32 with_alpha(ImU32 c, float alpha);

// Multiplies alpha by the current ImGui style alpha (window fade) and `mul`.
// Every framework widget draws through this so fading a window fades its content.
ImU32 styled(ImU32 c, float mul = 1.0f);

// Perceived brightness in [0, 1]; handy to pick readable text on colored fills.
float luminance(ImU32 c);

// "#RRGGBB" / "#RRGGBBAA" <-> packed color.
std::string to_hex(ImU32 c, bool with_alpha = false);
bool from_hex(const char* text, ImU32* out);

} // namespace nc
