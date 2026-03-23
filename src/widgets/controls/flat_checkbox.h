#pragma once
#include "../ui_core.h"

namespace ui
{
inline bool flat_checkbox(const char* label, bool* v)
{
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##cb");
	const char* display_end = ImGui::FindRenderedTextEnd(label);
	ImVec2 label_sz = ImGui::CalcTextSize(label, display_end);
	float sz = 11.0f;
	float gap = 7.0f;
	float hit_h = std::max(sz + 2.0f, label_sz.y + 2.0f);
	ImVec2 pos = ImGui::GetCursorScreenPos();

	ImGui::InvisibleButton("##cb", ImVec2(sz + gap + label_sz.x, hit_h));
	bool clicked = ImGui::IsItemClicked();
	if (clicked)
		*v = !*v;

	float& anim = ganim(id, *v ? 1.0f : 0.0f);
	float target = *v ? 1.0f : 0.0f;
	anim = alerp(anim, target);

	ImVec2 bmin(pos.x, pos.y + (hit_h - sz) * 0.5f);
	ImVec2 bmax(bmin.x + sz, bmin.y + sz);
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec4 ac = ImGui::ColorConvertU32ToFloat4(col_accent);
	ImU32 fill = styled(IM_COL32((int)(6 + (ac.x * 255 - 6) * anim), (int)(6 + (ac.y * 255 - 6) * anim),
															 (int)(6 + (ac.z * 255 - 6) * anim), (int)(255 * std::max(0.3f, anim) + 0.5f)));
	ImU32 brd = styled(IM_COL32((int)(36 + (ac.x * 255 - 36) * anim), (int)(36 + (ac.y * 255 - 36) * anim),
															(int)(36 + (ac.z * 255 - 36) * anim), 255));
	if (fill != 0)
		accent_glow_rect(dl, bmin, bmax, 3.0f, anim);
	dl->AddRectFilled(bmin, bmax, fill, 3.0f);
	dl->AddRect(bmin, bmax, brd, 3.0f);

	float text_bright = 0.50f + 0.28f * anim;
	ImU32 text_col =
			styled(IM_COL32((int)(text_bright * 255.0f), (int)(text_bright * 255.0f), (int)(text_bright * 255.0f), 255));
	if (display_end > label)
	{
		dl->AddText(ImVec2(pos.x + sz + gap, pos.y + (hit_h - label_sz.y) * 0.5f), text_col, label, display_end);
	}

	ImGui::PopID();
	return clicked;
}
} // namespace ui
