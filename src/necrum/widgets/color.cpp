#include "internal.h"

namespace nc
{

using namespace detail;

namespace
{
bool picker_popup(HSV* c, bool alpha)
{
	const Theme& t = theme();
	bool changed = false;
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const float pw = ImGui::GetContentRegionAvail().x;
	const float bar_w = 12.0f, gap = 6.0f;
	const float bars = alpha ? 2.0f : 1.0f;
	const float sq = pw - (bar_w + gap) * bars;
	ImVec2 sp = ImGui::GetCursorScreenPos();

	// Saturation / value square
	ImVec2 sv_min = sp, sv_max(sp.x + sq, sp.y + sq);
	ImU32 white = styled(IM_COL32_WHITE), hue_col = styled(hsv(c->h, 1.0f, 1.0f));
	dl->AddRectFilledMultiColor(sv_min, sv_max, white, hue_col, hue_col, white);
	dl->AddRectFilledMultiColor(sv_min, sv_max, 0, 0, styled(IM_COL32_BLACK), styled(IM_COL32_BLACK));
	dl->AddRect(sv_min, sv_max, styled(t.field_border), 1.0f);
	ImGui::SetCursorScreenPos(sv_min);
	ImGui::InvisibleButton("##sv", ImVec2(sq, sq));
	if (ImGui::IsItemActive())
	{
		ImVec2 mp = ImGui::GetIO().MousePos;
		c->s = saturate((mp.x - sv_min.x) / sq);
		c->v = 1.0f - saturate((mp.y - sv_min.y) / sq);
		changed = true;
	}
	ImVec2 knob(sv_min.x + c->s * sq, sv_min.y + (1.0f - c->v) * sq);
	dl->AddCircle(knob, 5.0f, styled(IM_COL32(0, 0, 0, 200)), 12, 2.0f);
	dl->AddCircle(knob, 4.0f, styled(IM_COL32_WHITE), 12, 1.5f);

	// Hue bar
	ImVec2 hmin(sv_max.x + gap, sp.y), hmax(hmin.x + bar_w, sp.y + sq);
	for (int i = 0; i < 6; ++i)
	{
		float seg = sq / 6.0f;
		ImU32 c1 = styled(hsv(i * 60.0f, 1.0f, 1.0f)), c2 = styled(hsv((i + 1) * 60.0f, 1.0f, 1.0f));
		dl->AddRectFilledMultiColor(ImVec2(hmin.x, hmin.y + i * seg), ImVec2(hmax.x, hmin.y + (i + 1) * seg), c1, c1, c2,
																c2);
	}
	dl->AddRect(hmin, hmax, styled(t.field_border), 1.0f);
	ImGui::SetCursorScreenPos(hmin);
	ImGui::InvisibleButton("##hue", ImVec2(bar_w, sq));
	if (ImGui::IsItemActive())
	{
		c->h = saturate((ImGui::GetIO().MousePos.y - hmin.y) / sq) * 360.0f;
		changed = true;
	}
	float hy = hmin.y + (c->h / 360.0f) * sq;
	dl->AddRectFilled(ImVec2(hmin.x - 1.0f, hy - 2.0f), ImVec2(hmax.x + 1.0f, hy + 2.0f), styled(IM_COL32_WHITE), 1.0f);
	dl->AddRect(ImVec2(hmin.x - 1.0f, hy - 2.0f), ImVec2(hmax.x + 1.0f, hy + 2.0f), styled(IM_COL32(0, 0, 0, 180)),
							1.0f);

	// Alpha bar
	if (alpha)
	{
		ImVec2 amin(hmax.x + gap, sp.y), amax(amin.x + bar_w, sp.y + sq);
		const float cell = bar_w * 0.5f;
		for (int row = 0; row * cell < sq; ++row)
			for (int col = 0; col < 2; ++col)
			{
				ImVec2 cmin(amin.x + col * cell, amin.y + row * cell);
				ImVec2 cmax(cmin.x + cell, std::min(cmin.y + cell, amax.y));
				dl->AddRectFilled(cmin, cmax, styled(((row + col) & 1) ? IM_COL32(200, 200, 200, 255) : IM_COL32(120, 120, 120, 255)));
			}
		ImU32 opaque = styled(hsv(c->h, c->s, c->v, 1.0f)), clear = styled(hsv(c->h, c->s, c->v, 0.0f));
		dl->AddRectFilledMultiColor(amin, amax, opaque, opaque, clear, clear);
		dl->AddRect(amin, amax, styled(t.field_border), 1.0f);
		ImGui::SetCursorScreenPos(amin);
		ImGui::InvisibleButton("##alpha", ImVec2(bar_w, sq));
		if (ImGui::IsItemActive())
		{
			c->a = 1.0f - saturate((ImGui::GetIO().MousePos.y - amin.y) / sq);
			changed = true;
		}
		float ay = amin.y + (1.0f - c->a) * sq;
		dl->AddRectFilled(ImVec2(amin.x - 1.0f, ay - 2.0f), ImVec2(amax.x + 1.0f, ay + 2.0f), styled(IM_COL32_WHITE), 1.0f);
		dl->AddRect(ImVec2(amin.x - 1.0f, ay - 2.0f), ImVec2(amax.x + 1.0f, ay + 2.0f), styled(IM_COL32(0, 0, 0, 180)),
								1.0f);
	}

	// Hex field + preview
	ImGui::SetCursorScreenPos(ImVec2(sp.x, sp.y + sq + 8.0f));
	ImGuiID hex_id = ImGui::GetID("##hex");
	char hex[16];
	std::string cur = to_hex(c->u32(), alpha);
	snprintf(hex, sizeof(hex), "%s", cur.c_str());
	float field_w = pw - 30.0f;
	ImVec2 fpos = ImGui::GetCursorScreenPos();
	field_frame(dl, fpos, ImVec2(fpos.x + field_w, fpos.y + t.control_height), 0.0f,
							anim::animate(anim::key(hex_id, "f"), ImGui::GetActiveID() == hex_id));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, (t.control_height - ImGui::GetFontSize()) * 0.5f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, styled(t.text_label));
	ImGui::SetNextItemWidth(field_w);
	if (ImGui::InputText("##hex", hex, sizeof(hex),
											 ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue |
													 ImGuiInputTextFlags_AutoSelectAll))
	{
		ImU32 parsed;
		if (from_hex(hex, &parsed))
		{
			HSV next = HSV::from_u32(parsed);
			if (!alpha)
				next.a = c->a;
			*c = next;
			changed = true;
		}
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();

	ImVec2 pv(fpos.x + field_w + 6.0f, fpos.y);
	dl->AddRectFilled(pv, ImVec2(pv.x + 24.0f, pv.y + t.control_height), styled(c->u32()), t.frame_rounding);
	dl->AddRect(pv, ImVec2(pv.x + 24.0f, pv.y + t.control_height), styled(t.field_border), t.frame_rounding);
	return changed;
}

bool context_popup(HSV* c, const HSV* reset)
{
	bool changed = false;
	if (begin_popup("##color_ctx", 120.0f))
	{
		if (popup_item("Copy"))
			ImGui::SetClipboardText(to_hex(c->u32(), true).c_str());
		const char* clip = ImGui::GetClipboardText();
		ImU32 parsed;
		if (clip && from_hex(clip, &parsed) && popup_item("Paste"))
		{
			*c = HSV::from_u32(parsed);
			changed = true;
		}
		if (reset && popup_item("Reset"))
		{
			*c = *reset;
			changed = true;
		}
		end_popup();
	}
	return changed;
}

bool swatch(ImGuiID id, HSV* c, const HSV* reset, bool alpha, ImVec2 hit_pos, ImVec2 hit_size, const char* caption)
{
	const Theme& t = theme();
	ImGui::SetCursorScreenPos(hit_pos);
	ImGui::InvisibleButton("##sw", hit_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		ImGui::OpenPopup("##color_pop");
	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("##color_ctx");
	float hov = anim::animate(anim::key(id, "hov"), ImGui::IsItemHovered());

	ImDrawList* dl = ImGui::GetWindowDrawList();
	const float sw_w = 26.0f, sw_h = 12.0f;
	if (caption)
	{
		ImVec2 cs = ImGui::CalcTextSize(caption, label_end(caption));
		dl->AddText(ImVec2(hit_pos.x, hit_pos.y + (hit_size.y - cs.y) * 0.5f),
								styled(lerp_color(t.text_label, t.text, hov * 0.6f)), caption, label_end(caption));
	}
	ImVec2 smin(hit_pos.x + hit_size.x - sw_w - 2.0f, hit_pos.y + (hit_size.y - sw_h) * 0.5f);
	ImVec2 smax(smin.x + sw_w, smin.y + sw_h);
	if (hov > 0.01f)
		draw::glow(dl, smin, smax, 3.0f, hov * 0.5f, c->u32(1.0f));
	dl->AddRectFilled(smin, smax, styled(c->u32()), 3.0f);
	dl->AddRect(smin, smax, styled(lerp_color(t.field_border, t.text, hov * 0.3f)), 3.0f);

	bool changed = context_popup(c, reset);

	ImGui::SetNextWindowPos(ImVec2(smax.x, smax.y + 4.0f), ImGuiCond_Appearing, ImVec2(1.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(alpha ? 218.0f : 200.0f, 0.0f));
	push_popup_style(ImVec2(8.0f, 8.0f));
	if (ImGui::BeginPopup("##color_pop", ImGuiWindowFlags_NoMove))
	{
		changed |= picker_popup(c, alpha);
		ImGui::EndPopup();
	}
	pop_popup_style();
	if (changed)
		ImGui::MarkItemEdited(id);
	return changed;
}
} // namespace

bool color_edit(const char* label, HSV* color, const HSV* reset, bool alpha)
{
	if (!search::filter(label))
		return false;
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##col");
	ImVec2 pos = ImGui::GetCursorScreenPos();
	float w = ImGui::GetContentRegionAvail().x;
	float h = ImGui::GetFontSize() + 2.0f;
	bool changed = swatch(id, color, reset, alpha, pos, ImVec2(w, h), has_visible_label(label) ? label : nullptr);
	ImGui::PopID();
	return changed;
}

bool inline_color(const char* id_str, HSV* color, const HSV* reset, bool alpha)
{
	if (search::dry_run() || !search::filter(id_str))
		return false;
	ImGui::PushID(id_str);
	ImGuiID id = ImGui::GetID("##col");
	ImGui::SameLine();
	const float w = 30.0f;
	float h = ImGui::GetFontSize() + 2.0f;
	float right = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
	ImVec2 pos(right - w, ImGui::GetCursorScreenPos().y);
	bool changed = swatch(id, color, reset, alpha, pos, ImVec2(w, h), nullptr);
	ImGui::PopID();
	return changed;
}

} // namespace nc
