#pragma once
#include "../ext/imgui/imgui.h"
#include "web_image.h"

namespace ImGui
{
inline void AddWebImage(const char* web_image_url, const ImVec2& p_min, const ImVec2& p_max,
												const ImU32 col = IM_COL32_WHITE)
{
	ImTextureID tex_id = web_image::get_web_image(web_image_url);
	if (!tex_id)
		return;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImage(tex_id, p_min, p_max, ImVec2(0, 0), ImVec2(1, 1), col);
}

inline void AddWebImageRounded(const char* web_image_url, const ImVec2& p_min, const ImVec2& p_max,
															 float rounding = 4.0f, ImDrawFlags flags = ImDrawFlags_RoundCornersAll,
															 const ImU32 col = IM_COL32_WHITE)
{
	ImTextureID tex_id = web_image::get_web_image(web_image_url);
	if (!tex_id)
		return;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddImageRounded(tex_id, p_min, p_max, ImVec2(0, 0), ImVec2(1, 1), col, rounding, flags);
}

inline void AddWebImageCircle(const char* web_image_url, const ImVec2& center, float radius,
															const ImU32 col = IM_COL32_WHITE)
{
	ImTextureID tex_id = web_image::get_web_image(web_image_url);
	if (!tex_id)
		return;

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 p_min = ImVec2(center.x - radius, center.y - radius);
	ImVec2 p_max = ImVec2(center.x + radius, center.y + radius);

	draw_list->AddImageRounded(tex_id, p_min, p_max, ImVec2(0, 0), ImVec2(1, 1), col, radius,
														 ImDrawFlags_RoundCornersAll);
}
} // namespace ImGui
