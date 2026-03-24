#include "color_picker.h"
#include "../ui_core.h"
#include "dropdown.h"

namespace ui
{
char s_color_clipboard[8];

bool flat_color_picker(const char* label, float* hue, float* sat, float* val, float def_h, float def_s, float def_v)
{
	ImGui::PushID(label);
	bool changed = false;

	float full_w = ImGui::GetContentRegionAvail().x;
	float line_h = 16.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* dl = ImGui::GetWindowDrawList();

	ImGui::InvisibleButton("##swatch", ImVec2(full_w, line_h));
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		ImGui::OpenPopup("##cpop");
	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("##cctx");

	const char* display_end = ImGui::FindRenderedTextEnd(label);
	if (display_end > label)
	{
		dl->AddText(ImVec2(pos.x, pos.y + 1), styled(IM_COL32(130, 130, 130, 255)), label, display_end);
	}

	float sw_w = 26.0f;
	float sw_h = 12.0f;
	float bx = pos.x + full_w - sw_w - 2;
	float by = pos.y + (line_h - sw_h) / 2;
	ImVec2 sw_min(bx, by), sw_max(bx + sw_w, by + sw_h);

	ImU32 preview = hsv_to_u32(*hue, *sat, *val);
	dl->AddRectFilled(sw_min, sw_max, styled(preview), 3.0f);
	dl->AddRect(sw_min, sw_max, styled(col_groove_brd), 3.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 6));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.04f, 0.04f, 0.04f, 0.98f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.14f, 0.14f, 0.8f));
	if (ImGui::BeginPopup("##cctx"))
	{
		ImVec4 ac_c = ImGui::ColorConvertU32ToFloat4(col_accent);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(ac_c.x, ac_c.y, ac_c.z, 0.25f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(ac_c.x, ac_c.y, ac_c.z, 0.40f));
		if (ImGui::Selectable("Copy Color"))
		{
			float r, g, b;
			ImGui::ColorConvertHSVtoRGB(*hue / 360.0f, *sat, *val, r, g, b);
			snprintf(s_color_clipboard, sizeof(s_color_clipboard), "#%02X%02X%02X", (int)(r * 255), (int)(g * 255),
							 (int)(b * 255));
		}
		if (s_color_clipboard[0] == '#')
		{
			if (ImGui::Selectable("Paste Color"))
			{
				unsigned int hr = 0, hg = 0, hb = 0;
				sscanf(s_color_clipboard, "#%02X%02X%02X", &hr, &hg, &hb);
				float r = hr / 255.0f, g = hg / 255.0f, b = hb / 255.0f;
				ImGui::ColorConvertRGBtoHSV(r, g, b, *hue, *sat, *val);
				*hue *= 360.0f;
				changed = true;
			}
		}
		if (def_h >= 0.0f)
		{
			if (ImGui::Selectable("Reset Color"))
			{
				*hue = def_h;
				*sat = def_s;
				*val = def_v;
				changed = true;
			}
		}
		ImGui::PopStyleColor(2);
		ImGui::EndPopup();
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);

	ImGui::SetNextWindowSize(ImVec2(200, 230));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.035f, 0.035f, 0.04f, 0.98f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.14f, 0.14f, 0.14f, 0.7f));
	if (ImGui::BeginPopup("##cpop", ImGuiWindowFlags_NoMove))
	{
		ImDrawList* pdl = ImGui::GetWindowDrawList();
		float pw = ImGui::GetContentRegionAvail().x;
		float sq_size = pw - 22.0f;
		float bar_w = 12.0f;
		float gap = 6.0f;

		ImVec2 sp = ImGui::GetCursorScreenPos();

		ImVec2 sv_min = sp, sv_max(sp.x + sq_size, sp.y + sq_size);
		ImU32 white = styled(IM_COL32(255, 255, 255, 255));
		ImU32 hue_col = styled(hsv_to_u32(*hue, 1.0f, 1.0f));
		pdl->AddRectFilledMultiColor(sv_min, sv_max, white, hue_col, hue_col, white);
		ImU32 black = styled(IM_COL32(0, 0, 0, 255)), clear = styled(IM_COL32(0, 0, 0, 0));
		pdl->AddRectFilledMultiColor(sv_min, sv_max, clear, clear, black, black);
		pdl->AddRect(sv_min, sv_max, styled(col_groove_brd), 1.0f);

		ImGui::SetCursorScreenPos(sv_min);
		ImGui::InvisibleButton("##sv", ImVec2(sq_size, sq_size));
		if (ImGui::IsItemActive())
		{
			ImVec2 mp = ImGui::GetIO().MousePos;
			*sat = ui::saturate((mp.x - sv_min.x) / sq_size);
			*val = 1.0f - ui::saturate((mp.y - sv_min.y) / sq_size);
			changed = true;
		}
		float cx = sv_min.x + *sat * sq_size;
		float cy = sv_min.y + (1.0f - *val) * sq_size;
		pdl->AddCircle(ImVec2(cx, cy), 5.0f, styled(IM_COL32(0, 0, 0, 200)), 12, 2.0f);
		pdl->AddCircle(ImVec2(cx, cy), 4.0f, styled(IM_COL32(255, 255, 255, 255)), 12, 1.5f);

		ImVec2 hm(sp.x + sq_size + gap, sp.y);
		ImVec2 hx(hm.x + bar_w, sp.y + sq_size);
		for (int i = 0; i < 6; i++)
		{
			float h1 = (float)i / 6.0f, h2 = (float)(i + 1) / 6.0f;
			float r1, g1, b1, r2, g2, b2;
			ImGui::ColorConvertHSVtoRGB(h1, 1, 1, r1, g1, b1);
			ImGui::ColorConvertHSVtoRGB(h2, 1, 1, r2, g2, b2);
			ImU32 c1 = styled(IM_COL32((int)(r1 * 255), (int)(g1 * 255), (int)(b1 * 255), 255));
			ImU32 c2 = styled(IM_COL32((int)(r2 * 255), (int)(g2 * 255), (int)(b2 * 255), 255));
			float sh = sq_size / 6.0f;
			pdl->AddRectFilledMultiColor(ImVec2(hm.x, hm.y + i * sh), ImVec2(hx.x, hm.y + (i + 1) * sh), c1, c1, c2, c2);
		}
		pdl->AddRect(hm, hx, styled(col_groove_brd), 1.0f);

		ImGui::SetCursorScreenPos(hm);
		ImGui::InvisibleButton("##hue", ImVec2(bar_w, sq_size));
		if (ImGui::IsItemActive())
		{
			*hue = ui::saturate((ImGui::GetIO().MousePos.y - hm.y) / sq_size) * 360.0f;
			changed = true;
		}
		float hy = hm.y + (*hue / 360.0f) * sq_size;
		pdl->AddRectFilled(ImVec2(hm.x - 1, hy - 2), ImVec2(hx.x + 1, hy + 2), styled(IM_COL32(255, 255, 255, 255)), 1.0f);
		pdl->AddRect(ImVec2(hm.x - 1, hy - 2), ImVec2(hx.x + 1, hy + 2), styled(IM_COL32(0, 0, 0, 180)), 1.0f);

		ImGui::SetCursorScreenPos(ImVec2(sp.x, sp.y + sq_size + 6));
		float r, g, b;
		ImGui::ColorConvertHSVtoRGB(*hue / 360.0f, *sat, *val, r, g, b);
		char hex[16];
		snprintf(hex, sizeof(hex), "#%02X%02X%02X", (int)(r * 255), (int)(g * 255), (int)(b * 255));
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.6f, 0.6f, 0.6f, 1)));
		ImGui::TextUnformatted(hex);
		ImGui::PopStyleColor();

		ImVec2 psp = ImGui::GetCursorScreenPos();
		pdl->AddRectFilled(psp, ImVec2(psp.x + pw, psp.y + 12), styled(hsv_to_u32(*hue, *sat, *val)), 3.0f);
		pdl->AddRect(psp, ImVec2(psp.x + pw, psp.y + 12), styled(col_groove_brd), 3.0f);
		ImGui::Dummy(ImVec2(pw, 12));

		ImGui::EndPopup();
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);

	ImGui::PopID();
	return changed;
}
} // namespace ui
