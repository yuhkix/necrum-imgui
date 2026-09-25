#pragma once

#include "web_image.h"

// Draw-list helpers for web images. Nothing is drawn until the image is loaded.
namespace nc::web_image
{

inline bool draw(ImDrawList* dl, const std::string& url, ImVec2 min, ImVec2 max, ImU32 tint = IM_COL32_WHITE,
								 float rounding = 0.0f, ImDrawFlags flags = ImDrawFlags_RoundCornersAll)
{
	ImTextureID tex = get(url);
	if (!tex || !is_loaded(url))
		return false;
	if (rounding > 0.0f)
		dl->AddImageRounded(tex, min, max, ImVec2(0, 0), ImVec2(1, 1), tint, rounding, flags);
	else
		dl->AddImage(tex, min, max, ImVec2(0, 0), ImVec2(1, 1), tint);
	return true;
}

inline bool draw_circle(ImDrawList* dl, const std::string& url, ImVec2 center, float radius,
												ImU32 tint = IM_COL32_WHITE)
{
	return draw(dl, url, ImVec2(center.x - radius, center.y - radius), ImVec2(center.x + radius, center.y + radius),
							tint, radius);
}

} // namespace nc::web_image
