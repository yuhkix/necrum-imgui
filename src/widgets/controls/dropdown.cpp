#include "dropdown.h"
#include "../ui_core.h"
#include <algorithm>

namespace ui
{
const ImVec4 popup_bg = ImVec4(0.032f, 0.032f, 0.032f, 0.98f);

void dd_draw_button(ImDrawList* dl, ImVec2 bmin, ImVec2 bmax, float h, const char* display, float anim_open,
										ImGuiID text_anim_id)
{
	dl->AddRectFilled(bmin, bmax, styled(IM_COL32(8, 8, 8, 255)), 3.0f);
	dl->AddRect(bmin, bmax, styled(col_groove_brd), 3.0f);

	ImU32 text_col = styled(col_text_dim);
	float text_left = bmin.x + 8.0f;
	float text_center_y = bmin.y + h * 0.5f;
	dl->PushClipRect(ImVec2(text_left, bmin.y + 1.0f), ImVec2(bmax.x - 20.0f, bmax.y - 1.0f), true);
	if (text_anim_id != 0)
		draw_text_swap_left(dl, text_anim_id, text_left, text_center_y, text_col, display);
	else
	{
		ImVec2 ts = ImGui::CalcTextSize(display);
		dl->AddText(ImVec2(text_left, text_center_y - ts.y * 0.5f), text_col, display);
	}
	dl->PopClipRect();

	float tri_size = 4.0f;
	ImVec2 center(bmax.x - 12, bmin.y + h * 0.5f);
	float rot = anim_open * 3.14159f;

	auto rotate = [&](ImVec2 v, float a)
	{
		float s = sinf(a), c = cosf(a);
		return ImVec2(v.x * c - v.y * s, v.x * s + v.y * c);
	};

	ImVec2 v1 = rotate(ImVec2(-tri_size, -tri_size * 0.5f), rot);
	ImVec2 v2 = rotate(ImVec2(tri_size, -tri_size * 0.5f), rot);
	ImVec2 v3 = rotate(ImVec2(0, tri_size * 0.8f), rot);

	dl->AddTriangleFilled(ImVec2(center.x + v1.x, center.y + v1.y), ImVec2(center.x + v2.x, center.y + v2.y),
												ImVec2(center.x + v3.x, center.y + v3.y), styled(col_text_dim));
}

bool dd_item(const char* text, bool selected, bool close_on_click, float forced_w)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 text_sz = ImGui::CalcTextSize(text);
	float tw = text_sz.x;
	float text_x = 16.0f;
	float pad_r = 6.0f;
	float iw = forced_w > 0 ? forced_w : (text_x + tw + pad_r);
	if (iw < text_x + tw + pad_r)
		iw = text_x + tw + pad_r;
	float ih = 20.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::PushID(text);
	ImGuiID iid = ImGui::GetID("##di");
	ImGui::InvisibleButton("##di", ImVec2(iw, ih));
	bool hovered = ImGui::IsItemHovered();
	bool clicked = ImGui::IsItemClicked();
	ImGui::PopID();

	float& hov_anim = ganim(iid ^ 0x31F2A41u, hovered ? 1.0f : 0.0f);
	hov_anim = alerp(hov_anim, hovered ? 1.0f : 0.0f);
	float& sel_anim = ganim(iid ^ 0x9AF31E1u, selected ? 1.0f : 0.0f);
	sel_anim = alerp(sel_anim, selected ? 1.0f : 0.0f);

	float sel_bg_alpha = sel_anim * 0.08f;
	float bg_alpha = hov_anim * 0.10f + sel_bg_alpha;
	float margin_x = 4.0f;
	if (bg_alpha > 0.001f)
	{
		accent_glow_rect(dl, ImVec2(pos.x + margin_x, pos.y), ImVec2(pos.x + iw - margin_x, pos.y + ih), 3.0f, bg_alpha);
		dl->AddRectFilled(ImVec2(pos.x + margin_x, pos.y), ImVec2(pos.x + iw - margin_x, pos.y + ih),
											styled(col_accent, bg_alpha), 3.0f);
	}
	if (sel_anim > 0.001f)
	{
		float bar_w = 1.0f + sel_anim * 1.6f;
		float inset = 2.0f - sel_anim * 0.5f;
		accent_glow_rect(dl, ImVec2(pos.x + 1.0f, pos.y + inset), ImVec2(pos.x + 1.0f + bar_w, pos.y + ih - inset), 1.2f,
										 sel_anim * 0.95f);
		dl->AddRectFilled(ImVec2(pos.x + 1.0f, pos.y + inset), ImVec2(pos.x + 1.0f + bar_w, pos.y + ih - inset),
											styled(col_accent, sel_anim * 0.95f), 1.2f);
	}

	ImVec4 ac = ImGui::ColorConvertU32ToFloat4(col_accent);
	float text_t = ui::saturate(sel_anim + hov_anim * 0.35f);
	ImU32 tc =
			styled(IM_COL32((int)(155 + (ac.x * 255.0f - 155.0f) * text_t), (int)(152 + (ac.y * 255.0f - 152.0f) * text_t),
											(int)(150 + (ac.z * 255.0f - 150.0f) * text_t), 255));
	dl->AddText(ImVec2(pos.x + text_x, pos.y + (ih - text_sz.y) * 0.5f), tc, text);
	if (clicked && close_on_click)
		ImGui::CloseCurrentPopup();
	return clicked;
}

bool dd_item_check(const char* text, bool selected, float forced_w)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 text_sz = ImGui::CalcTextSize(text);
	float tw = text_sz.x;
	float pad_l = 6.0f, pad_r = 6.0f;
	float sq = 8.0f, gap = 5.0f;
	float iw = forced_w > 0 ? forced_w : (pad_l + sq + gap + tw + pad_r);
	if (iw < pad_l + sq + gap + tw + pad_r)
		iw = pad_l + sq + gap + tw + pad_r;
	float ih = 20.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::PushID(text);
	ImGuiID iid = ImGui::GetID("##di");
	ImGui::InvisibleButton("##di", ImVec2(iw, ih));
	bool hovered = ImGui::IsItemHovered();
	bool clicked = ImGui::IsItemClicked();
	ImGui::PopID();

	float& hov_anim = ganim(iid ^ 0x4BE4E53u, hovered ? 1.0f : 0.0f);
	hov_anim = alerp(hov_anim, hovered ? 1.0f : 0.0f);
	float& sel_anim = ganim(iid ^ 0xA2481D5u, selected ? 1.0f : 0.0f);
	sel_anim = alerp(sel_anim, selected ? 1.0f : 0.0f);

	float bg_alpha = hov_anim * 0.10f;
	float margin_x = 4.0f;
	if (bg_alpha > 0.001f)
		dl->AddRectFilled(ImVec2(pos.x + margin_x, pos.y), ImVec2(pos.x + iw - margin_x, pos.y + ih),
											styled(col_accent, bg_alpha), 3.0f);
	float cx = pos.x + pad_l;
	ImVec2 sq_min(cx, pos.y + (ih - sq) / 2);
	ImVec2 sq_max(sq_min.x + sq, sq_min.y + sq);

	ImVec4 ac = ImGui::ColorConvertU32ToFloat4(col_accent);
	ImU32 sq_fill = styled(IM_COL32((int)(6 + (ac.x * 255.0f - 6.0f) * (sel_anim * 0.85f)),
																	(int)(6 + (ac.y * 255.0f - 6.0f) * (sel_anim * 0.85f)),
																	(int)(6 + (ac.z * 255.0f - 6.0f) * (sel_anim * 0.85f)), 255));
	float br_t = ui::saturate(sel_anim);
	ImU32 sq_brd = styled(IM_COL32((int)(36 + (ac.x * 255.0f - 36.0f) * br_t), (int)(36 + (ac.y * 255.0f - 36.0f) * br_t),
																 (int)(36 + (ac.z * 255.0f - 36.0f) * br_t), 255));
	if (sel_anim > 0.001f)
		accent_glow_rect(dl, sq_min, sq_max, 2.0f, sel_anim * 0.85f);
	dl->AddRectFilled(sq_min, sq_max, sq_fill, 2.0f);
	dl->AddRect(sq_min, sq_max, sq_brd, 2.0f);
	if (sel_anim > 0.001f)
	{
		float inset = 1.5f + (1.0f - sel_anim) * 1.8f;
		dl->AddRectFilled(ImVec2(sq_min.x + inset, sq_min.y + inset), ImVec2(sq_max.x - inset, sq_max.y - inset),
											styled(col_accent, 0.8f * sel_anim), 1.4f);
	}

	float text_t = ui::saturate(hov_anim * 0.30f);
	ImU32 tc =
			styled(IM_COL32((int)(155 + (ac.x * 255.0f - 155.0f) * text_t), (int)(155 + (ac.y * 255.0f - 155.0f) * text_t),
											(int)(155 + (ac.z * 255.0f - 155.0f) * text_t), 255));
	dl->AddText(ImVec2(cx + sq + gap, pos.y + (ih - text_sz.y) * 0.5f), tc, text);
	return clicked;
}

bool flat_dropdown_single(const char* id, int* sel, const char** items, int count)
{
	ImGui::PushID(id);
	float w = ImGui::CalcItemWidth();
	float h = 18.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##dd", ImVec2(w, h));
	bool open = ImGui::IsPopupOpen("##pop");

	float& anim = ganim(ImGui::GetID("##drot"), open ? 1.0f : 0.0f);
	anim = alerp(anim, open ? 1.0f : 0.0f);

	if (ImGui::IsItemClicked())
		ImGui::OpenPopup("##pop");

	const char* display = (*sel >= 0 && *sel < count) ? items[*sel] : "-";
	ImGuiID dd_text_id = ImGui::GetID("##dd_text");
	dd_draw_button(ImGui::GetWindowDrawList(), pos, ImVec2(pos.x + w, pos.y + h), h, display, anim, dd_text_id);

	bool changed = false;
	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + h + 1));
	ImGui::SetNextWindowSize(ImVec2(w, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, styled(IM_COL32(10, 10, 10, 255)));
	ImGui::PushStyleColor(ImGuiCol_Border, styled(col_groove_brd));
	if (ImGui::BeginPopup("##pop", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		for (int i = 0; i < count; i++)
			if (dd_item(items[i], *sel == i, false, w))
			{
				*sel = (*sel == i) ? -1 : i;
				changed = true;
				ImGui::CloseCurrentPopup();
			}
		ImGui::EndPopup();
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
	ImGui::PopID();
	return changed;
}

bool flat_dropdown_multi(const char* id, bool* sels, const char** items, int count)
{
	ImGui::PushID(id);
	float w = ImGui::CalcItemWidth();
	float h = 18.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##dd", ImVec2(w, h));
	bool open = ImGui::IsPopupOpen("##pop");

	float& anim = ganim(ImGui::GetID("##drot"), open ? 1.0f : 0.0f);
	anim = alerp(anim, open ? 1.0f : 0.0f);

	if (ImGui::IsItemClicked())
		ImGui::OpenPopup("##pop");

	char display[128] = "-";
	int off = 0;
	for (int i = 0; i < count; i++)
	{
		if (!sels[i])
			continue;
		if (off > 0 && off < (int)sizeof(display) - 2)
		{
			display[off++] = ',';
			display[off++] = ' ';
		}
		int len = (int)strlen(items[i]);
		int room = (int)sizeof(display) - off - 1;
		if (len > room)
			len = room;
		memcpy(display + off, items[i], len);
		off += len;
	}
	if (off > 0)
		display[off] = '\0';

	ImGuiID dd_text_id = ImGui::GetID("##dd_text");
	dd_draw_button(ImGui::GetWindowDrawList(), pos, ImVec2(pos.x + w, pos.y + h), h, display, anim, dd_text_id);

	bool changed = false;
	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + h + 1));
	ImGui::SetNextWindowSize(ImVec2(w, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 3.0f);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, styled(IM_COL32(10, 10, 10, 255)));
	ImGui::PushStyleColor(ImGuiCol_Border, styled(col_groove_brd));
	if (ImGui::BeginPopup("##pop", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
	{
		for (int i = 0; i < count; i++)
			if (dd_item_check(items[i], sels[i], w))
			{
				sels[i] = !sels[i];
				changed = true;
			}
		ImGui::EndPopup();
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
	ImGui::PopID();
	return changed;
}

} // namespace ui
