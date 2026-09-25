#include "color.h"

namespace nc
{

namespace
{
int channel(ImU32 c, int shift)
{
	return (int)((c >> shift) & 0xFF);
}
} // namespace

ImU32 HSV::u32(float alpha_mul) const
{
	return hsv(h, s, v, a * alpha_mul);
}

ImVec4 HSV::rgba() const
{
	ImVec4 out(0, 0, 0, a);
	ImGui::ColorConvertHSVtoRGB(h / 360.0f, s, v, out.x, out.y, out.z);
	return out;
}

HSV HSV::from_u32(ImU32 col)
{
	ImVec4 f = ImGui::ColorConvertU32ToFloat4(col);
	return from_rgb(f.x, f.y, f.z, f.w);
}

HSV HSV::from_rgb(float r, float g, float b, float a)
{
	HSV out;
	ImGui::ColorConvertRGBtoHSV(r, g, b, out.h, out.s, out.v);
	out.h *= 360.0f;
	out.a = a;
	return out;
}

ImU32 hsv(float h_deg, float s, float v, float a)
{
	float r, g, b;
	float h = std::fmod(h_deg, 360.0f);
	if (h < 0.0f)
		h += 360.0f;
	ImGui::ColorConvertHSVtoRGB(h / 360.0f, saturate(s), saturate(v), r, g, b);
	return IM_COL32((int)(r * 255.0f + 0.5f), (int)(g * 255.0f + 0.5f), (int)(b * 255.0f + 0.5f),
									(int)(saturate(a) * 255.0f + 0.5f));
}

ImU32 lerp_color(ImU32 a, ImU32 b, float t)
{
	t = saturate(t);
	auto mix = [t](int x, int y) { return (int)(x + (y - x) * t + 0.5f); };
	return IM_COL32(mix(channel(a, IM_COL32_R_SHIFT), channel(b, IM_COL32_R_SHIFT)),
									mix(channel(a, IM_COL32_G_SHIFT), channel(b, IM_COL32_G_SHIFT)),
									mix(channel(a, IM_COL32_B_SHIFT), channel(b, IM_COL32_B_SHIFT)),
									mix(channel(a, IM_COL32_A_SHIFT), channel(b, IM_COL32_A_SHIFT)));
}

ImU32 alpha_mul(ImU32 c, float mul)
{
	float a = channel(c, IM_COL32_A_SHIFT) / 255.0f;
	a = saturate(a * mul);
	return (c & ~IM_COL32_A_MASK) | ((ImU32)(a * 255.0f + 0.5f) << IM_COL32_A_SHIFT);
}

ImU32 with_alpha(ImU32 c, float alpha)
{
	return (c & ~IM_COL32_A_MASK) | ((ImU32)(saturate(alpha) * 255.0f + 0.5f) << IM_COL32_A_SHIFT);
}

ImU32 styled(ImU32 c, float mul)
{
	return alpha_mul(c, ImGui::GetStyle().Alpha * mul);
}

float luminance(ImU32 c)
{
	return (0.2126f * channel(c, IM_COL32_R_SHIFT) + 0.7152f * channel(c, IM_COL32_G_SHIFT) +
					0.0722f * channel(c, IM_COL32_B_SHIFT)) /
				 255.0f;
}

std::string to_hex(ImU32 c, bool alpha)
{
	char buf[16];
	if (alpha)
		snprintf(buf, sizeof(buf), "#%02X%02X%02X%02X", channel(c, IM_COL32_R_SHIFT), channel(c, IM_COL32_G_SHIFT),
						 channel(c, IM_COL32_B_SHIFT), channel(c, IM_COL32_A_SHIFT));
	else
		snprintf(buf, sizeof(buf), "#%02X%02X%02X", channel(c, IM_COL32_R_SHIFT), channel(c, IM_COL32_G_SHIFT),
						 channel(c, IM_COL32_B_SHIFT));
	return buf;
}

bool from_hex(const char* text, ImU32* out)
{
	if (!text || !out)
		return false;
	if (*text == '#')
		++text;
	size_t len = strlen(text);
	if (len != 6 && len != 8)
		return false;
	for (size_t i = 0; i < len; ++i)
		if (!std::isxdigit((unsigned char)text[i]))
			return false;

	unsigned int r = 0, g = 0, b = 0, a = 255;
	if (len == 8)
		sscanf(text, "%02x%02x%02x%02x", &r, &g, &b, &a);
	else
		sscanf(text, "%02x%02x%02x", &r, &g, &b);
	*out = IM_COL32(r, g, b, a);
	return true;
}

} // namespace nc
