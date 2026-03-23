#include "hotkey.h"
#include "../ui_core.h"
#include "dropdown.h"
#include <algorithm>

namespace ui
{
bool hotkey_inline(const char* label, int* key, int* mode)
{
	const char* hk_mode_names[] = {"Toggle", "Hold", "Always"};
	ImGui::PushID(label);
	ImGuiID uid = ImGui::GetID("##hk");
	bool listening = (s_listening == uid);
	float& anim = ganim(uid, listening ? 1.0f : 0.0f);
	anim = alerp(anim, listening ? 1.0f : 0.0f);
	const char* label_end = ImGui::FindRenderedTextEnd(label);
	bool has_visible_label = (label_end > label);

	const char* kdisp = "NONE";
	char kbuf[32] = {};
	if (*mode == 2)
		kdisp = "ON";
	else if (listening)
		kdisp = "...";
	else if (*key != 0)
	{
		std::string name = get_key_name(*key);
		kdisp = name.c_str(); // name is local but kdisp is assigned to bracket which is snprintf'd immediately
		snprintf(kbuf, sizeof(kbuf), "%s", name.c_str());
		kdisp = kbuf;
	}
	char bracket[48];
	snprintf(bracket, sizeof(bracket), "[ %s ]", kdisp);
	float bw = ImGui::CalcTextSize(bracket).x;

	float full_w = ImGui::GetContentRegionAvail().x;
	if (!has_visible_label)
		full_w = std::max(full_w, bw + 8.0f);
	ImVec2 pos = ImGui::GetCursorScreenPos();
	float line_h = 16.0f;

	ImGui::InvisibleButton("##hk", ImVec2(full_w, line_h));
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && *mode != 2)
		s_listening = uid;
	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("##hk_ctx");
	bool hovered = ImGui::IsItemHovered();

	ImDrawList* dl = ImGui::GetWindowDrawList();
	if (has_visible_label)
		dl->AddText(ImVec2(pos.x, pos.y + 1), styled(IM_COL32(130, 130, 130, 255)), label, label_end);

	float right_x = pos.x + full_w - 2.0f;
	ImVec4 ac_hk = ImGui::ColorConvertU32ToFloat4(col_accent);
	ImU32 bcol = styled(IM_COL32((int)(120 + (ac_hk.x * 255 - 120) * anim), (int)(116 + (ac_hk.y * 255 - 116) * anim),
															 (int)(116 + (ac_hk.z * 255 - 116) * anim), 255));
	if (hovered && !listening)
		bcol = styled(col_accent, 0.7f);
	if (*mode == 2)
		bcol = styled(col_accent);
	draw_text_swap_right(dl, ImGui::GetID("##hk_text"), right_x, pos.y + line_h * 0.5f + 0.5f, bcol, bracket);

	const float hk_popup_pad_x = 5.0f;
	const float hk_indicator_left = 4.0f;
	const float hk_indicator_r = 3.5f;
	const float hk_label_gap = 4.0f;
	const float hk_item_pad_r = 5.0f;
	float hk_max_text_w = 0.0f;
	for (int i = 0; i < 3; ++i)
		hk_max_text_w = std::max(hk_max_text_w, ImGui::CalcTextSize(hk_mode_names[i]).x);
	float hk_content_w = hk_indicator_left + hk_indicator_r * 2.0f + hk_label_gap + hk_max_text_w + hk_item_pad_r;
	float hk_popup_w = std::max(76.0f, hk_content_w + hk_popup_pad_x * 2.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(hk_popup_pad_x, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, popup_bg);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.14f, 0.14f, 0.8f));
	ImGui::SetNextWindowSize(ImVec2(hk_popup_w, 0), ImGuiCond_Appearing);
	ImGui::SetNextWindowSizeConstraints(ImVec2(hk_popup_w, 0), ImVec2(180, 200));
	if (ImGui::BeginPopup("##hk_ctx"))
	{
		ImDrawList* cdl = ImGui::GetWindowDrawList();
		for (int i = 0; i < 3; i++)
		{
			ImVec2 text_sz = ImGui::CalcTextSize(hk_mode_names[i]);
			float iw = ImGui::GetContentRegionAvail().x;
			float ih = 20.0f;
			ImVec2 ip = ImGui::GetCursorScreenPos();
			ImGui::PushID(i);
			ImGuiID row_id = ImGui::GetID("##m");
			ImGui::InvisibleButton("##m", ImVec2(iw, ih));
			bool hov = ImGui::IsItemHovered();
			bool clk = ImGui::IsItemClicked();
			ImGui::PopID();
			float& hov_anim = ganim(row_id ^ 0x72CC31Bu, hov ? 1.0f : 0.0f);
			hov_anim = alerp(hov_anim, hov ? 1.0f : 0.0f);
			float& sel_anim = ganim(row_id ^ 0x26A7B9Du, *mode == i ? 1.0f : 0.0f);
			sel_anim = alerp(sel_anim, *mode == i ? 1.0f : 0.0f);
			float row_alpha = hov_anim * 0.11f + sel_anim * 0.08f;
			if (row_alpha > 0.001f)
			{
				accent_glow_rect(cdl, ip, ImVec2(ip.x + iw, ip.y + ih), 3.0f, row_alpha);
				cdl->AddRectFilled(ip, ImVec2(ip.x + iw, ip.y + ih), styled(col_accent, row_alpha), 3.0f);
			}

			float dot_cx = ip.x + hk_indicator_left + hk_indicator_r;
			float dot_cy = ip.y + ih * 0.5f;
			ImVec4 ac = ImGui::ColorConvertU32ToFloat4(col_accent);
			float dot_t = std::clamp(sel_anim + hov_anim * 0.35f, 0.0f, 1.0f);
			ImU32 dot_col =
					styled(IM_COL32((int)(36 + (ac.x * 255.0f - 36.0f) * dot_t), (int)(36 + (ac.y * 255.0f - 36.0f) * dot_t),
													(int)(36 + (ac.z * 255.0f - 36.0f) * dot_t), 255));
			cdl->AddCircle(ImVec2(dot_cx, dot_cy), hk_indicator_r, dot_col, 12, 1.2f);
			if (sel_anim > 0.001f)
				cdl->AddCircleFilled(ImVec2(dot_cx, dot_cy), 2.0f, styled(col_accent, sel_anim), 12);

			float text_t = std::clamp(sel_anim + hov_anim * 0.30f, 0.0f, 1.0f);
			ImU32 tc = styled(IM_COL32((int)(155 + (ac.x * 255.0f - 155.0f) * text_t),
																 (int)(152 + (ac.y * 255.0f - 152.0f) * text_t),
																 (int)(150 + (ac.z * 255.0f - 150.0f) * text_t), 255));
			float label_x = dot_cx + hk_indicator_r + hk_label_gap;
			cdl->AddText(ImVec2(label_x, ip.y + (ih - text_sz.y) * 0.5f), tc, hk_mode_names[i]);
			if (clk)
			{
				*mode = i;
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);

	if (listening)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.ClearInputKeys();
		io.ClearInputCharacters();
		ImGui::SetKeyOwner(ImGuiKey_Escape, uid);
		ImGui::ClearActiveID();

		for (int vk = 1; vk < 256; vk++)
		{
			if (vk == VK_INSERT || vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_SHIFT || vk == VK_CONTROL ||
					vk == VK_MENU)
				continue;
			if (GetAsyncKeyState(vk) & 1)
			{
				*key = (vk == VK_ESCAPE) ? 0 : vk;
				s_listening = 0;
				break;
			}
		}
	}
	ImGui::PopID();
	return listening;
}
} // namespace ui
