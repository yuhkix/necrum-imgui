#include "internal.h"

namespace nc
{

using namespace detail;

namespace
{
// Combo button + popup placement shared by combo() and multi_combo().
// Returns true while the popup is open (caller must then call end_combo_popup()).
bool combo_button(const char* label, const char* preview, ImGuiID* out_id, float* out_width)
{
	const Theme& t = theme();
	ImGuiID id = ImGui::GetID("##cmb");
	float w = stacked_label(label);
	float h = t.control_height;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##cmb", ImVec2(w, h));
	bool clicked = ImGui::IsItemClicked();
	bool hovered = ImGui::IsItemHovered();
	bool open = ImGui::IsPopupOpen("##cmb_pop");
	if (clicked && !open)
		ImGui::OpenPopup("##cmb_pop");

	float hov = anim::animate(anim::key(id, "hov"), hovered);
	float op = anim::animate(anim::key(id, "open"), open);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 max(pos.x + w, pos.y + h);
	field_frame(dl, pos, max, hov, op * 0.6f);

	dl->PushClipRect(ImVec2(pos.x + 1.0f, pos.y + 1.0f), ImVec2(max.x - 20.0f, max.y - 1.0f), true);
	draw::text_swap(dl, anim::key(id, "txt"), pos.x + 8.0f, pos.y + h * 0.5f, draw::Align::Left,
									styled(lerp_color(t.text_label, t.text, hov)), preview);
	dl->PopClipRect();
	draw::chevron(dl, ImVec2(max.x - 11.0f, pos.y + h * 0.5f), 3.5f, op, styled(lerp_color(t.text_dim, t.text, hov)));

	*out_id = id;
	*out_width = w;
	ImGui::SetNextWindowPos(ImVec2(pos.x, max.y + 2.0f));
	ImGui::SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(w, t.row_height * 10.5f + 4.0f));
	push_popup_style(ImVec2(0.0f, 2.0f));
	bool popup = ImGui::BeginPopup("##cmb_pop", ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	if (!popup)
		pop_popup_style();
	return popup;
}

void end_combo_popup()
{
	ImGui::EndPopup();
	pop_popup_style();
}
} // namespace

bool combo(const char* label, int* selected, const char* const items[], int count, bool allow_none)
{
	if (!search::filter(label))
		return false;
	ImGui::PushID(label);
	const char* preview = (*selected >= 0 && *selected < count) ? items[*selected] : "-";
	ImGuiID id;
	float w;
	bool changed = false;
	if (combo_button(label, preview, &id, &w))
	{
		for (int i = 0; i < count; ++i)
		{
			if (list_row(items[i], *selected == i, w))
			{
				*selected = (allow_none && *selected == i) ? -1 : i;
				changed = true;
				ImGui::CloseCurrentPopup();
			}
			if (*selected == i && ImGui::IsWindowAppearing())
				ImGui::SetScrollHereY();
		}
		end_combo_popup();
	}
	if (changed)
		ImGui::MarkItemEdited(id);
	ImGui::PopID();
	return changed;
}

bool combo(const char* label, int* selected, const std::vector<std::string>& items, bool allow_none)
{
	std::vector<const char*> ptrs;
	ptrs.reserve(items.size());
	for (const auto& s : items)
		ptrs.push_back(s.c_str());
	return combo(label, selected, ptrs.data(), (int)ptrs.size(), allow_none);
}

bool multi_combo(const char* label, bool* selected, const char* const items[], int count)
{
	if (!search::filter(label))
		return false;
	ImGui::PushID(label);

	std::string preview;
	for (int i = 0; i < count; ++i)
	{
		if (!selected[i])
			continue;
		if (!preview.empty())
			preview += ", ";
		preview += items[i];
	}
	if (preview.empty())
		preview = "-";

	ImGuiID id;
	float w;
	bool changed = false;
	if (combo_button(label, preview.c_str(), &id, &w))
	{
		for (int i = 0; i < count; ++i)
		{
			if (list_row(items[i], selected[i], w, RowStyle::Check))
			{
				selected[i] = !selected[i];
				changed = true;
			}
		}
		end_combo_popup();
	}
	if (changed)
		ImGui::MarkItemEdited(id);
	ImGui::PopID();
	return changed;
}

bool multi_combo(const char* label, uint32_t* mask, const char* const items[], int count)
{
	bool flags[32] = {};
	count = std::min(count, 32);
	for (int i = 0; i < count; ++i)
		flags[i] = (*mask >> i) & 1u;
	bool changed = multi_combo(label, flags, items, count);
	if (changed)
	{
		*mask = 0;
		for (int i = 0; i < count; ++i)
			if (flags[i])
				*mask |= 1u << i;
	}
	return changed;
}

bool listbox(const char* label, int* selected, const char* const items[], int count, float height)
{
	if (!search::filter(label))
		return false;
	const Theme& t = theme();
	ImGui::PushID(label);
	float w = stacked_label(label);
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + height), styled(t.field_bg), t.frame_rounding);
	dl->AddRect(pos, ImVec2(pos.x + w, pos.y + height), styled(t.field_border), t.frame_rounding);

	bool changed = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 2.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
	if (ImGui::BeginChild("##lb", ImVec2(w, height), ImGuiChildFlags_AlwaysUseWindowPadding,
												ImGuiWindowFlags_NoBackground))
	{
		float row_w = ImGui::GetContentRegionAvail().x;
		for (int i = 0; i < count; ++i)
		{
			if (list_row(items[i], *selected == i, row_w))
			{
				*selected = i;
				changed = true;
			}
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleVar(3);
	ImGui::PopID();
	return changed;
}

bool listbox(const char* label, int* selected, const std::vector<std::string>& items, float height)
{
	std::vector<const char*> ptrs;
	ptrs.reserve(items.size());
	for (const auto& s : items)
		ptrs.push_back(s.c_str());
	return listbox(label, selected, ptrs.data(), (int)ptrs.size(), height);
}

bool segmented(const char* label, int* selected, const char* const items[], int count)
{
	if (!search::filter(label) || count <= 0)
		return false;
	const Theme& t = theme();
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##seg");
	float w = stacked_label(label);
	float h = t.control_height + 2.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 max(pos.x + w, pos.y + h);
	dl->AddRectFilled(pos, max, styled(t.field_bg), t.frame_rounding);
	dl->AddRect(pos, max, styled(t.field_border), t.frame_rounding);

	const float seg_w = w / (float)count;
	float sel_x = anim::animate(anim::key(id, "x"), (float)std::max(0, *selected) * seg_w);
	if (*selected >= 0)
	{
		ImVec2 smin(pos.x + sel_x + 2.0f, pos.y + 2.0f), smax(pos.x + sel_x + seg_w - 2.0f, max.y - 2.0f);
		draw::glow(dl, smin, smax, t.frame_rounding, 0.35f);
		dl->AddRectFilled(smin, smax, styled(t.accent_col, 0.9f), t.frame_rounding);
	}

	bool changed = false;
	for (int i = 0; i < count; ++i)
	{
		ImGui::SetCursorScreenPos(ImVec2(pos.x + seg_w * i, pos.y));
		ImGui::PushID(i);
		if (ImGui::InvisibleButton("##s", ImVec2(seg_w, h)) && *selected != i)
		{
			*selected = i;
			changed = true;
		}
		float hov = anim::animate(ImGui::GetID("hov"), ImGui::IsItemHovered());
		ImGui::PopID();

		const bool sel = *selected == i;
		ImU32 tc = sel ? t.accent_text : lerp_color(t.text_dim, t.text, hov);
		ImVec2 ts = ImGui::CalcTextSize(items[i], label_end(items[i]));
		dl->AddText(ImVec2(pos.x + seg_w * i + (seg_w - ts.x) * 0.5f, pos.y + (h - ts.y) * 0.5f), styled(tc), items[i],
								label_end(items[i]));
		if (i > 0 && !sel && *selected != i - 1)
			dl->AddLine(ImVec2(pos.x + seg_w * i, pos.y + 5.0f), ImVec2(pos.x + seg_w * i, max.y - 5.0f),
									styled(t.divider));
	}
	ImGui::SetCursorScreenPos(pos);
	ImGui::Dummy(ImVec2(w, h));
	if (changed)
		ImGui::MarkItemEdited(id);
	ImGui::PopID();
	return changed;
}

bool tab_strip(const char* id_str, int* selected, const char* const items[], int count, bool icons)
{
	if (search::dry_run() || count <= 0)
		return false;
	const Theme& t = theme();
	ImGui::PushID(id_str);
	ImGuiID id = ImGui::GetID("##ts");

	const float w = ImGui::GetContentRegionAvail().x;
	const float h = 28.0f;
	const float tab_w = w / (float)count;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImVec2 max(pos.x + w, pos.y + h);
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(pos, max, styled(t.header_bg), 1.0f);
	dl->AddRect(pos, max, styled(t.field_border), 1.0f);

	float ax = anim::animate(anim::key(id, "x"), (float)std::max(0, *selected) * tab_w);
	dl->AddRectFilled(ImVec2(pos.x + ax + 2.0f, max.y - 3.0f), ImVec2(pos.x + ax + tab_w - 2.0f, max.y - 1.0f),
										styled(t.accent_col));

	ImFont* font = icons ? fonts::icons() : ImGui::GetFont();
	float font_size = icons ? font->LegacySize : ImGui::GetFontSize();
	bool changed = false;
	for (int i = 0; i < count; ++i)
	{
		ImVec2 tmin(pos.x + i * tab_w, pos.y);
		ImVec2 tmax(tmin.x + tab_w, max.y);
		ImGui::SetCursorScreenPos(tmin);
		ImGui::PushID(i);
		if (ImGui::InvisibleButton("##t", ImVec2(tab_w, h)) && *selected != i)
		{
			*selected = i;
			changed = true;
		}
		ItemAnim a = item_anim(ImGui::GetID("##t"));
		ImGui::PopID();

		if (a.hover > 0.01f || a.active > 0.01f)
			dl->AddRectFilled(tmin, ImVec2(tmax.x, tmax.y - 1.0f), styled(t.hover, a.hover * 0.04f + a.active * 0.04f));

		const bool sel = *selected == i;
		float sel_a = anim::animate(anim::key(id, items[i]), sel);
		if (sel_a > 0.01f)
			draw::underline_glow(dl, tmin, tmax, t.accent_col, sel_a * 0.9f, 12.0f);

		ImVec2 ts = draw::text_size(font, font_size, items[i], label_end(items[i]));
		ImVec2 tp(tmin.x + (tab_w - ts.x) * 0.5f, tmin.y + (h - ts.y) * 0.5f);
		ImU32 tc = lerp_color(lerp_color(t.text_disabled, t.text, a.hover), t.accent_col, sel_a);
		dl->AddText(font, font_size, tp, styled(tc), items[i], label_end(items[i]));
		if (i > 0)
			dl->AddLine(ImVec2(tmin.x, tmin.y + 4.0f), ImVec2(tmin.x, tmax.y - 4.0f), styled(t.divider, 0.6f));
	}
	ImGui::SetCursorScreenPos(pos);
	ImGui::Dummy(ImVec2(w, h));
	ImGui::PopID();
	return changed;
}

bool begin_popup(const char* id, float width, ImGuiWindowFlags flags)
{
	if (width > 0.0f)
		ImGui::SetNextWindowSizeConstraints(ImVec2(width, 0.0f), ImVec2(width, FLT_MAX));
	push_popup_style(ImVec2(4.0f, 4.0f));
	if (ImGui::BeginPopup(id, flags))
	{
		search::push_suspend();
		return true;
	}
	pop_popup_style();
	return false;
}

void end_popup()
{
	search::pop_suspend();
	ImGui::EndPopup();
	pop_popup_style();
}

bool popup_item(const char* label, bool selected, bool close_on_click)
{
	bool pressed = list_row(label, selected, ImGui::GetContentRegionAvail().x);
	if (pressed && close_on_click)
		ImGui::CloseCurrentPopup();
	return pressed;
}

int confirm_dialog(const char* id, const char* title, const char* message, const char* confirm, const char* cancel)
{
	const Theme& t = theme();
	int result = -1;
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(300.0f, 0.0f));
	push_popup_style(ImVec2(16.0f, 14.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 10.0f));
	ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, with_alpha(t.shadow, 0.45f));
	if (ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		search::push_suspend();
		{
			fonts::Scope bold(fonts::bold());
			ImGui::PushStyleColor(ImGuiCol_Text, styled(t.text));
			ImGui::TextUnformatted(title);
			ImGui::PopStyleColor();
		}
		ImGui::PushStyleColor(ImGuiCol_Text, styled(t.text_label));
		ImGui::TextWrapped("%s", message);
		ImGui::PopStyleColor();

		float bw = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
		if (button(cancel, ImVec2(bw, 0.0f)))
		{
			result = 0;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (button(confirm, ImVec2(bw, 0.0f), ButtonStyle::Primary))
		{
			result = 1;
			ImGui::CloseCurrentPopup();
		}
		search::pop_suspend();
		ImGui::EndPopup();
	}
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	pop_popup_style();
	return result;
}

} // namespace nc
