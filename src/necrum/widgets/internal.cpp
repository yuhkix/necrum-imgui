#include "internal.h"

namespace nc::detail
{

float stacked_label(const char* label, const char* right_text)
{
	const float w = ImGui::CalcItemWidth();
	if (!has_visible_label(label) && !right_text)
		return w;

	const Theme& t = theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	const float h = ImGui::GetFontSize();

	if (has_visible_label(label))
		dl->AddText(pos, styled(t.text_label), label, label_end(label));
	if (right_text && *right_text)
	{
		ImVec2 sz = ImGui::CalcTextSize(right_text);
		dl->AddText(ImVec2(pos.x + w - sz.x, pos.y), styled(t.text_dim), right_text);
	}

	ImGuiStyle& style = ImGui::GetStyle();
	float saved = style.ItemSpacing.y;
	style.ItemSpacing.y = 5.0f;
	ImGui::Dummy(ImVec2(w, h));
	style.ItemSpacing.y = saved;
	return w;
}

void push_popup_style(ImVec2 padding)
{
	const Theme& t = theme();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, t.popup_rounding);
	ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, styled(t.popup_bg));
	ImGui::PushStyleColor(ImGuiCol_Border, styled(t.popup_border));
}

void pop_popup_style()
{
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(4);
}

bool list_row(const char* text, bool selected, float width, RowStyle style)
{
	const Theme& t = theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const char* text_end = label_end(text);
	ImVec2 text_sz = ImGui::CalcTextSize(text, text_end);

	const float lead = style == RowStyle::Plain ? 16.0f : 22.0f;
	const float pad_r = 8.0f;
	float w = std::max(width, lead + text_sz.x + pad_r);
	const float h = t.row_height;

	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::PushID(text);
	ImGuiID id = ImGui::GetID("##row");
	bool pressed = ImGui::InvisibleButton("##row", ImVec2(w, h));
	bool hovered = ImGui::IsItemHovered();
	ImGui::PopID();

	float hov = anim::animate(anim::key(id, "hov"), hovered);
	float sel = anim::animate(anim::key(id, "sel"), selected);

	const float mx = 4.0f;
	ImVec2 rmin(pos.x + mx, pos.y), rmax(pos.x + w - mx, pos.y + h);
	float bg = hov * 0.10f + (style == RowStyle::Plain ? sel * 0.08f : 0.0f);
	if (bg > 0.001f)
	{
		draw::glow(dl, rmin, rmax, 3.0f, bg);
		dl->AddRectFilled(rmin, rmax, styled(t.accent_col, bg), 3.0f);
	}

	const float cy = pos.y + h * 0.5f;
	if (style == RowStyle::Plain)
	{
		if (sel > 0.001f)
		{
			float bar_w = 1.0f + sel * 1.6f;
			float inset = 2.0f - sel * 0.5f;
			ImVec2 bmin(pos.x + 1.0f, pos.y + inset), bmax(pos.x + 1.0f + bar_w, pos.y + h - inset);
			draw::glow(dl, bmin, bmax, 1.2f, sel * 0.95f);
			dl->AddRectFilled(bmin, bmax, styled(t.accent_col, sel * 0.95f), 1.2f);
		}
	}
	else if (style == RowStyle::Check)
	{
		const float sq = 9.0f;
		ImVec2 smin(pos.x + 8.0f, cy - sq * 0.5f), smax(smin.x + sq, smin.y + sq);
		if (sel > 0.001f)
			draw::glow(dl, smin, smax, 2.0f, sel * 0.85f);
		dl->AddRectFilled(smin, smax, styled(lerp_color(t.control_off, t.accent_col, sel * 0.85f)), 2.0f);
		dl->AddRect(smin, smax, styled(lerp_color(t.field_border, t.accent_col, sel)), 2.0f);
		draw::check_mark(dl, ImVec2(smin.x + 1.5f, smin.y + 1.5f), sq - 3.0f, styled(t.accent_text, sel), sel);
	}
	else
	{
		const float r = 3.5f;
		ImVec2 c(pos.x + 8.0f + r, cy);
		dl->AddCircle(c, r, styled(lerp_color(t.field_border, t.accent_col, saturate(sel + hov * 0.35f))), 12, 1.2f);
		if (sel > 0.001f)
			dl->AddCircleFilled(c, 2.0f, styled(t.accent_col, sel), 12);
	}

	float text_t = saturate((style == RowStyle::Plain ? sel : 0.0f) + hov * 0.35f);
	ImU32 tc = lerp_color(t.text_label, t.accent_col, text_t);
	if (style != RowStyle::Plain)
		tc = lerp_color(tc, t.text, sel * 0.6f);
	dl->AddText(ImVec2(pos.x + lead, cy - text_sz.y * 0.5f), styled(tc), text, text_end);
	return pressed;
}

void field_frame(ImDrawList* dl, ImVec2 min, ImVec2 max, float hover, float focus)
{
	const Theme& t = theme();
	dl->AddRectFilled(min, max, styled(t.field_bg), t.frame_rounding);
	ImU32 border = lerp_color(t.field_border, t.accent_col, saturate(hover * 0.35f + focus * 0.6f));
	dl->AddRect(min, max, styled(border), t.frame_rounding);

	if (focus > 0.001f)
	{
		float cx = (min.x + max.x) * 0.5f;
		float half = (max.x - min.x) * 0.5f * focus;
		ImVec2 a(cx - half + 1.0f, min.y), b(cx + half - 1.0f, max.y);
		if (b.x > a.x)
			draw::underline_glow(dl, a, b, t.accent_col, focus, std::min(10.0f, max.y - min.y - 2.0f));
	}
}

ItemAnim item_anim(ImGuiID id)
{
	ItemAnim a;
	a.hover = anim::animate(anim::key(id, "hov"), ImGui::IsItemHovered());
	a.active = anim::animate(anim::key(id, "act"), ImGui::IsItemActive());
	return a;
}

} // namespace nc::detail
