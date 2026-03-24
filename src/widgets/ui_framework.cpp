#include "ui_framework.h"
#include "menu/theme.h"

#include "core/web_image.h"
#include "core/web_image_imgui.h"

namespace ui
{

ImU32 col_accent = IM_COL32(200, 65, 65, 255);
ImU32 col_accent_dim = IM_COL32(160, 50, 50, 200);

std::string search_query_norm_;
char search_query_[256] = {0};
bool search_focus_request_ = false;
bool is_search_pass_ = false;
int search_hits_count_ = 0;
std::vector<std::string> search_tokens_;
bool tab_has_hits[5] = {false};
bool side_tab_has_hits[5][20] = {false};

int top_tab_ = 0;
int side_tab_ = 0;
int prev_top_tab_ = 0;
int prev_side_tab_ = 0;
float tab_fade_ = 1.0f;
float anim_side_y_ = 0.0f;
int weapon_tab_ = 0;

std::unordered_map<ImGuiID, float> s_anim;
std::unordered_map<ImGuiID, text_swap_anim> s_text_swap;
ImGuiID s_listening = 0;

void draw_search_bar(float w)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 wp = ImGui::GetWindowPos();

	const float right_reserved = 5.0f * 30.0f + 24.0f;
	const float x = wp.x + 56.0f;
	const float right_bound = wp.x + w - right_reserved;
	const float max_w = right_bound - x;
	if (max_w < 120.0f)
		return;

	const float h = 22.0f;
	const float y = wp.y + (HEADER_H - h) * 0.5f;
	const float box_w = std::min(224.0f, max_w);
	ImVec2 bmin(x, y), bmax(x + box_w, y + h);
	ImGuiID field_id = ImGui::GetID("##menu_search_field");

	const float dt = std::max(g_dt, 1.0f / 240.0f);
	auto damp = [dt](float cur, float target, float speed)
	{
		float t = 1.0f - std::exp(-speed * dt);
		return cur + (target - cur) * t;
	};

	float& filled_anim = ganim(field_id ^ 0x31AB98Fu, has_search_query() ? 1.0f : 0.0f);
	filled_anim = damp(filled_anim, has_search_query() ? 1.0f : 0.0f, 7.0f);
	float& focus_anim = ganim(field_id ^ 0x4D29EC3u, 0.0f);
	float& typed_w_anim = ganim(field_id ^ 0x6B7C411u, 0.0f);
	float& type_pulse = ganim(field_id ^ 0x52F912Du, 0.0f);

	dl->AddRectFilled(bmin, bmax, styled(IM_COL32(10, 10, 11, 245)), 4.0f);
	dl->AddRect(bmin, bmax, styled(IM_COL32(45, 45, 49, 220)), 4.0f);

	ImGui::SetCursorScreenPos(bmin);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, 3.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, IM_COL32(0, 0, 0, 0));

	const float clear_slot_w = (search_query_[0] != '\0') ? 20.0f : 0.0f;
	ImGui::SetNextItemWidth(box_w - clear_slot_w);
	if (search_focus_request_)
	{
		ImGui::SetKeyboardFocusHere();
		search_focus_request_ = false;
	}
	bool changed = ImGui::InputTextWithHint("##menu_search_field", "Search features...", search_query_,
																					IM_ARRAYSIZE(search_query_), ImGuiInputTextFlags_EscapeClearsAll);
	if (changed)
		sync_search_query();

	bool active = ImGui::IsItemActive();
	bool hovered = ImGui::IsMouseHoveringRect(bmin, bmax, true);
	focus_anim = damp(focus_anim, active ? 1.0f : 0.0f, 14.0f);
	float& hover_anim = ganim(field_id ^ 0x9F22A5Bu, 0.0f);
	hover_anim = damp(hover_anim, hovered ? 1.0f : 0.0f, 12.0f);

	ImGui::PopStyleColor(6);
	ImGui::PopStyleVar(2);

	float focus_ease = focus_anim * focus_anim * (3.0f - 2.0f * focus_anim);
	float hover_ease = hover_anim * hover_anim * (3.0f - 2.0f * hover_anim);
	float filled_ease = filled_anim * filled_anim * (3.0f - 2.0f * filled_anim);
	float interact_ease = std::max(focus_ease, hover_ease);
	float active_mix = std::clamp(interact_ease * 1.0f + filled_ease * 0.36f, 0.0f, 1.0f);

	dl->AddRectFilled(ImVec2(bmin.x + 1.0f, bmin.y + 1.0f), ImVec2(bmax.x - 1.0f, bmax.y - 1.0f),
										styled(col_accent, 0.010f + interact_ease * 0.03f + filled_ease * 0.016f), 3.0f);
	dl->AddRect(bmin, bmax, styled(col_accent, 0.025f + active_mix * 0.13f), 4.0f);

	if (active_mix > 0.001f)
	{
		float cx = (bmin.x + bmax.x) * 0.5f;
		float half = std::max(0.0f, (box_w * 0.5f - 1.5f) * active_mix);
		float lx = cx - half;
		float rx = cx + half;

		ImVec2 gmin(lx + 1.0f, bmax.y - 10.0f);
		ImVec2 gmax(rx - 1.0f, bmax.y - 1.0f);
		ImU32 gcol = styled(col_accent, 0.14f * active_mix);
		ImU32 gclear = styled(col_accent, 0.0f);
		if (gmax.x > gmin.x)
		{
			dl->AddRectFilledMultiColor(gmin, gmax, gclear, gclear, gcol, gcol);
			dl->AddLine(ImVec2(lx, bmax.y - 1.0f), ImVec2(rx, bmax.y - 1.0f),
									styled(col_accent, (0.62f + interact_ease * 0.28f) * active_mix), 1.0f);
		}
	}

	// Custom animated text layout & cursor matching UIEngine
	ImGuiInputTextState* state = ImGui::GetInputTextState(field_id);
	float scroll_x = state ? state->Scroll.x : 0.0f;

	ImVec2 text_rect_min(bmin.x + 24.0f, bmin.y + 1.0f);
	ImVec2 text_rect_max(bmax.x - clear_slot_w - 4.0f, bmax.y - 1.0f);
	dl->PushClipRect(text_rect_min, text_rect_max, true);

	float text_y = bmin.y + (h - ImGui::GetFontSize()) * 0.5f;
	ImVec2 draw_pos(text_rect_min.x - scroll_x, text_y);

	if (search_query_[0] == '\0' && !active)
		dl->AddText(draw_pos, styled(col_text_dim), "Search features...");
	else
	{
		// Draw Selection
		if (state && state->HasSelection())
		{
			int sel_min = ImMin(state->GetSelectionStart(), state->GetSelectionEnd());
			int sel_max = ImMax(state->GetSelectionStart(), state->GetSelectionEnd());
			float sel_start_x = ImGui::CalcTextSize(search_query_, search_query_ + sel_min).x;
			float sel_end_x = ImGui::CalcTextSize(search_query_, search_query_ + sel_max).x;
			dl->AddRectFilled(ImVec2(draw_pos.x + sel_start_x, text_rect_min.y + 2.0f),
												ImVec2(draw_pos.x + sel_end_x, text_rect_max.y - 2.0f), styled(col_accent, 0.35f));
		}
		dl->AddText(draw_pos, styled(col_text), search_query_);
	}

	float target_cursor_x = 0.0f;
	if (state)
		target_cursor_x = ImGui::CalcTextSize(search_query_, search_query_ + state->GetCursorPos()).x;
	else if (active)
		target_cursor_x = ImGui::CalcTextSize(search_query_).x;

	float& anim_cursor_x = ganim(field_id ^ 0x6B7C411u, target_cursor_x);
	anim_cursor_x = damp(anim_cursor_x, target_cursor_x, 20.0f);

	float& cursor_alpha = ganim(field_id ^ 0x61FA83Au, 0.0f);
	if (active)
		cursor_alpha += 6.0f * g_dt;
	else
		cursor_alpha -= 6.0f * g_dt;
	cursor_alpha = std::clamp(cursor_alpha, 0.0f, 1.0f);

	if (cursor_alpha > 0.01f)
	{
		float cx = draw_pos.x + anim_cursor_x;
		dl->AddLine(ImVec2(cx, text_rect_min.y + 4.0f), ImVec2(cx, text_rect_max.y - 4.0f), styled(col_text, cursor_alpha),
								1.0f);
	}

	dl->PopClipRect();

	if (theme::font_icons)
	{
		dl->AddText(theme::font_icons, 12.0f, ImVec2(bmin.x + 8.0f, bmin.y + 4.0f),
								styled(IM_COL32(148, 148, 152, 255), 0.62f + interact_ease * 0.24f + filled_ease * 0.10f),
								ICON_FA_MAGNIFYING_GLASS);
	}

	if (search_query_[0] != '\0')
	{
		ImVec2 cmin(bmax.x - 18.0f, bmin.y + 3.0f);
		ImGui::SetCursorScreenPos(cmin);
		ImGui::PushID("search_clear");
		ImGui::InvisibleButton("##clear", ImVec2(14.0f, 14.0f));
		bool clear_hover = ImGui::IsItemHovered();
		if (ImGui::IsItemClicked())
		{
			search_query_[0] = '\0';
			sync_search_query();
			typed_w_anim = 0.0f;
			type_pulse = 0.0f;
		}
		float& clear_anim = ganim(ImGui::GetID("##clear_hov"), clear_hover ? 1.0f : 0.0f);
		clear_anim = damp(clear_anim, clear_hover ? 1.0f : 0.0f, 12.0f);
		if (theme::font_icons)
		{
			dl->AddText(theme::font_icons, 11.0f, ImVec2(cmin.x + 1.5f, cmin.y + 1.0f),
									styled(IM_COL32(135, 135, 140, 255), 0.52f + clear_anim * 0.36f), ICON_FA_XMARK);
		}
		ImGui::PopID();
	}
}

void draw_header(float w)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 wp = ImGui::GetWindowPos();

	dl->AddRectFilled(wp, ImVec2(wp.x + w, wp.y + HEADER_H), styled(IM_COL32(14, 14, 14, 255)), 5.0f,
										ImDrawFlags_RoundCornersTop);

	const char* logo_url = "https://i.ibb.co/k2tJg9Gx/necrum.png";
	ImTextureID logo_tex = ::web_image::get_web_image(logo_url);
	float lx = wp.x + 9, ly = wp.y + (HEADER_H - 32) * 0.5f;
	float lsize = 32.0f;

	if (logo_tex)
	{
		dl->AddImageRounded(logo_tex, {lx, ly}, {lx + lsize, ly + lsize}, {0, 0}, {1, 1}, styled(IM_COL32_WHITE), 9.0f);
	}
	else if (theme::font_icons)
	{
		for (int i = 0; i < 3; i++)
		{
			float spread = (float)(i + 1) * 1.5f;
			ImVec4 ac_f = ImGui::ColorConvertU32ToFloat4(col_accent);
			ImU32 gc = styled(IM_COL32((int)(ac_f.x * 255), (int)(ac_f.y * 255), (int)(ac_f.z * 255), 14 - i * 3));
			for (float dx = -spread; dx <= spread; dx += spread)
				for (float dy = -spread; dy <= spread; dy += spread)
					dl->AddText(theme::font_icons, lsize, ImVec2(lx + dx, ly + dy), gc, ICON_FA_STAR_AND_CRESCENT);
		}
		dl->AddText(theme::font_icons, lsize, {lx, ly}, styled(col_accent), ICON_FA_STAR_AND_CRESCENT);
	}

	draw_search_bar(w);

	float ix_start = wp.x + w - 5 * 30 - 14;
	float iy = wp.y + HEADER_H * 0.5f;
	for (int i = 0; i < 5; i++)
	{
		ImVec2 center(ix_start + i * 30 + 15, iy);

		ImGui::SetCursorScreenPos({center.x - 12, center.y - 12});
		ImGui::PushID(100 + i);
		ImGui::InvisibleButton("##hi", ImVec2(24, 24));
		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		if (ImGui::IsItemClicked())
		{
			top_tab_ = i;
			side_tab_ = 0;
		}
		float& hover_a = ganim(ImGui::GetID("hov"), 0.0f);
		hover_a = alerp(hover_a, hovered ? 1.0f : 0.0f);
		float& active_a = ganim(ImGui::GetID("act"), 0.0f);
		active_a = alerp(active_a, active ? 1.0f : 0.0f);
		ImGui::PopID();

		if (hover_a > 0.01f || active_a > 0.01f)
		{
			dl->AddRectFilled(ImVec2(center.x - 14, center.y - 14), ImVec2(center.x + 14, center.y + 14),
												styled(IM_COL32(255, 255, 255, 255), hover_a * 0.05f + active_a * 0.05f), 4.0f);
		}

		ImVec2 tsize = theme::font_icons->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, hdr_icons_fa[i]);
		ImVec2 tpos(center.x - tsize.x * 0.5f, center.y - tsize.y * 0.5f);

		if (top_tab_ == i)
		{
			dl->AddText(theme::font_icons, 14.0f, tpos, styled(col_accent), hdr_icons_fa[i]);
		}
		else
		{
			dl->AddText(theme::font_icons, 14.0f, tpos, styled(col_text_dim), hdr_icons_fa[i]);
			if (hover_a > 0.01f)
				dl->AddText(theme::font_icons, 14.0f, tpos, styled(IM_COL32(255, 255, 255, 255), hover_a), hdr_icons_fa[i]);
		}
	}

	dl->AddLine(ImVec2(wp.x, wp.y + HEADER_H - 1), ImVec2(wp.x + w, wp.y + HEADER_H - 1), styled(col_accent, 0.31f),
							1.0f);
}

void draw_sidebar(float h)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 sp = ImGui::GetCursorScreenPos();

	dl->AddRectFilledMultiColor(sp, ImVec2(sp.x + SIDEBAR_W, sp.y + h), styled(IM_COL32(9, 9, 9, 255)),
															styled(IM_COL32(11, 11, 11, 255)), styled(IM_COL32(10, 10, 10, 255)),
															styled(IM_COL32(8, 8, 8, 255)));
	dl->AddLine(ImVec2(sp.x + SIDEBAR_W - 1, sp.y), ImVec2(sp.x + SIDEBAR_W - 1, sp.y + h), styled(col_divider));

	constexpr float ROW_H = 30.0f;
	constexpr float IND_H = 18.0f;
	float base_y = sp.y + 24;

	auto items = get_sidebar_items(top_tab_);

	std::vector<int> visible_rows;
	visible_rows.reserve(items.size());
	for (int i = 0; i < (int)items.size(); ++i)
	{
		if (!has_search_query() || side_tab_has_hits[top_tab_][i] || search_match({items[i].label}))
			visible_rows.push_back(i);
	}

	if (visible_rows.empty())
	{
		ImVec2 text_pos(sp.x + 11.0f, base_y + 3.0f);
		dl->AddText(text_pos, styled(IM_COL32(122, 122, 126, 255)), "No matching sections");
		return;
	}

	if (std::find(visible_rows.begin(), visible_rows.end(), side_tab_) == visible_rows.end())
	{
		side_tab_ = visible_rows.front();
	}

	ImGui::PushFont(theme::font_bold);
	for (int row = 0; row < (int)visible_rows.size(); row++)
	{
		int i = visible_rows[row];
		bool sel = (side_tab_ == i);
		float ly = base_y + row * ROW_H;

		ImGui::SetCursorScreenPos(ImVec2(sp.x, ly));
		ImGui::PushID(200 + i + top_tab_ * 16);
		ImGui::InvisibleButton("##sb", ImVec2(SIDEBAR_W - 2, ROW_H));
		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		if (ImGui::IsItemClicked())
			side_tab_ = i;

		float& hover_a = ganim(ImGui::GetID("hov"), 0.0f);
		hover_a = alerp(hover_a, hovered ? 1.0f : 0.0f);
		float& active_a = ganim(ImGui::GetID("act"), 0.0f);
		active_a = alerp(active_a, active ? 1.0f : 0.0f);
		ImGui::PopID();

		if (hover_a > 0.01f || active_a > 0.01f)
		{
			dl->AddRectFilled(ImVec2(sp.x + 10, ly), ImVec2(sp.x + SIDEBAR_W - 10, ly + ROW_H),
												styled(IM_COL32(255, 255, 255, 255), hover_a * 0.03f + active_a * 0.03f), 4.0f);
		}

		const char* icon = items[i].icon;
		const char* text = items[i].label;

		float text_y = ly + (ROW_H - ImGui::GetFontSize()) * 0.5f;
		ImVec2 icon_pos(sp.x + 22, text_y);
		ImVec2 text_pos(sp.x + 22 + ImGui::CalcTextSize(icon).x, text_y);

		if (sel)
		{
			dl->AddText(icon_pos, styled(col_accent), icon);
			dl->AddText(text_pos, styled(col_text), text);
		}
		else
		{
			dl->AddText(icon_pos, styled(col_text_dim), icon);
			dl->AddText(text_pos, styled(col_text_dim), text);
			if (hover_a > 0.01f)
			{
				dl->AddText(icon_pos, styled(IM_COL32(255, 255, 255, 255), hover_a), icon);
				dl->AddText(text_pos, styled(IM_COL32(255, 255, 255, 255), hover_a), text);
			}
		}
	}
	ImGui::PopFont();

	// Animated indicator bar
	int selected_row = 0;
	for (int row = 0; row < (int)visible_rows.size(); ++row)
	{
		if (visible_rows[row] == side_tab_)
		{
			selected_row = row;
			break;
		}
	}
	float target_off = selected_row * ROW_H;
	if (anim_side_y_ < 0)
		anim_side_y_ = target_off;
	anim_side_y_ = alerp(anim_side_y_, target_off);

	float ind_y = base_y + anim_side_y_ + (ROW_H - IND_H) * 0.5f;
	dl->AddRectFilled(ImVec2(sp.x + 3, ind_y), ImVec2(sp.x + 5, ind_y + IND_H), styled(col_accent), 1.0f);
}

void draw_weapon_tabs(float w)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	float pad = 6.0f;
	float strip_w = w - pad * 2;
	float tab_w = strip_w / 6.0f;
	ImVec2 smin(pos.x + pad, pos.y + 4);
	ImVec2 smax(smin.x + strip_w, smin.y + TAB_STRIP_H);

	dl->AddRectFilled(smin, smax, styled(IM_COL32(14, 14, 14, 255)), 1.0f);
	dl->AddRect(smin, smax, styled(col_border), 1.0f);

	// Animated underline position
	ImGuiID auid = ImGui::GetID("##wt_anim");
	float& ax = ganim(auid, weapon_tab_ * tab_w);
	ax = alerp(ax, weapon_tab_ * tab_w);

	// Draw animated underline
	dl->AddRectFilled(ImVec2(smin.x + ax + 2, smax.y - 3), ImVec2(smin.x + ax + tab_w - 2, smax.y - 1),
										styled(col_accent));

	for (int i = 0; i < 6; i++)
	{
		ImVec2 tmin(smin.x + i * tab_w, smin.y);
		ImVec2 tmax(tmin.x + tab_w, smax.y);
		ImVec2 center((tmin.x + tmax.x) * 0.5f, (tmin.y + tmax.y) * 0.5f);

		ImGui::SetCursorScreenPos(tmin);
		ImGui::PushID(400 + i);
		ImGui::InvisibleButton("##wt", ImVec2(tab_w, TAB_STRIP_H));
		bool hovered = ImGui::IsItemHovered();
		bool active = ImGui::IsItemActive();
		if (ImGui::IsItemClicked())
			weapon_tab_ = i;

		float& hover_a = ganim(ImGui::GetID("hov"), 0.0f);
		hover_a = alerp(hover_a, hovered ? 1.0f : 0.0f);
		float& active_a = ganim(ImGui::GetID("act"), 0.0f);
		active_a = alerp(active_a, active ? 1.0f : 0.0f);
		ImGui::PopID();

		if (hover_a > 0.01f || active_a > 0.01f)
		{
			dl->AddRectFilled(tmin, ImVec2(tmax.x, tmax.y - 1),
												styled(IM_COL32(255, 255, 255, 255), hover_a * 0.04f + active_a * 0.04f));
		}

		static const char* weapon_tab_icons[6] = {
				ICON_FA_CROSSHAIRS,				ICON_FA_BOLT,					ICON_FA_FIRE, ICON_FA_BULLSEYE,
				ICON_FA_SKULL_CROSSBONES, ICON_FA_SHIELD_HALVED};

		ImVec2 tsize = theme::font_icons->CalcTextSizeA(14.0f, FLT_MAX, 0.0f, weapon_tab_icons[i]);
		ImVec2 tpos(center.x - tsize.x * 0.5f, center.y - tsize.y * 0.5f);

		if (weapon_tab_ == i)
		{
			// Bottom-up gradient glow from the active line
			ImVec2 g_min(tmin.x, tmax.y - 12); // Start 12px above bottom
			ImU32 g_col = styled(col_accent, 0.15f);
			ImU32 g_transparent = styled(col_accent, 0.0f);
			dl->AddRectFilledMultiColor(g_min, tmax, g_transparent, g_transparent, g_col, g_col);

			dl->AddText(theme::font_icons, 14.0f, tpos, styled(col_accent), weapon_tab_icons[i]);
		}
		else
		{
			dl->AddText(theme::font_icons, 14.0f, tpos, styled(IM_COL32(60, 60, 60, 255)), weapon_tab_icons[i]);
			if (hover_a > 0.01f)
				dl->AddText(theme::font_icons, 14.0f, tpos, styled(IM_COL32(200, 200, 200, 255), hover_a), weapon_tab_icons[i]);
		}
		if (i > 0)
			dl->AddLine({tmin.x, tmin.y + 4}, {tmin.x, tmax.y - 4}, styled(IM_COL32(30, 30, 30, 120)));
	}

	ImGui::SetCursorScreenPos(ImVec2(pos.x, smax.y + 4));
}

void draw_content(float w, float h, const std::function<void()>& content_draw)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 wp = ImGui::GetCursorScreenPos();
	dl->AddRectFilled(wp, ImVec2(wp.x + w, wp.y + h), styled(col_content));

	// Tab change fade
	if (side_tab_ != prev_side_tab_ || top_tab_ != prev_top_tab_)
	{
		tab_fade_ = 0.0f;
		prev_side_tab_ = side_tab_;
		prev_top_tab_ = top_tab_;
	}
	tab_fade_ = alerp(tab_fade_, 1.0f);

	ImGui::BeginChild("##ci", ImVec2(w, h), false, ImGuiWindowFlags_NoScrollbar);
	if (top_tab_ == 0)
	{
		ImGui::Dummy(ImVec2(0, .5));
		draw_weapon_tabs(w);
		ImGui::Spacing();
	}
	else
	{
		ImGui::Dummy(ImVec2(0, 3));
	}

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * tab_fade_);
	if (content_draw)
		content_draw();
	ImGui::PopStyleVar();
	ImGui::EndChild();
}

void draw_footer(float w)
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 wp = ImGui::GetWindowPos();
	float y = wp.y + WIN_H - FOOTER_H;

	dl->AddRectFilled(ImVec2(wp.x, y), ImVec2(wp.x + w, wp.y + WIN_H), styled(col_footer), 5.0f,
										ImDrawFlags_RoundCornersBottom);
	dl->AddLine({wp.x, y}, {wp.x + w, y}, styled(col_divider));

	float ty = y + (FOOTER_H - 11) * 0.5f;
	dl->AddText({wp.x + 10, ty}, styled(col_text_dim), "build:");
	float bw = ImGui::CalcTextSize("build: ").x;
	dl->AddText({wp.x + 10 + bw, ty}, styled(col_accent), "dev");

	float lw = ImGui::CalcTextSize("active user: ").x;
	float nw = ImGui::CalcTextSize("Yuhki").x;
	float rx = wp.x + w - lw - nw - 10;
	dl->AddText({rx, ty}, styled(col_text_dim), "active user:");
	dl->AddText({rx + lw, ty}, styled(col_accent), "Yuhki");
}

void draw_watermark()
{
	static char buf[256] = {0};
	static float w_text = 0.0f;
	static float last_update_time = -1.0f;
	float current_time = (float)ImGui::GetTime();

	if (current_time - last_update_time >= 0.5f)
	{
		auto now = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(now);
		struct tm tm_info;
		localtime_s(&tm_info, &t);
		char time_buf[16];
		strftime(time_buf, sizeof(time_buf), "%H:%M", &tm_info);

		snprintf(buf, sizeof(buf), "uid: 1 | user: Yuhki | %03.0f fps | %s", ImGui::GetIO().Framerate, time_buf);
		w_text = ImGui::CalcTextSize(buf).x;
		last_update_time = current_time;
	}

	ImGui::PushFont(theme::font_bold);
	float w_logo = ImGui::CalcTextSize("necrum").x;
	ImGui::PopFont();
	float w_sep = ImGui::CalcTextSize(" | ").x;

	ImVec2 pad = {8.0f, 4.0f};
	float total_w = w_logo + w_sep + w_text + pad.x * 2.0f;
	float font_h = ImGui::GetFontSize();
	float total_h = font_h + pad.y * 2.0f;

	ImVec2 pos = {ImGui::GetIO().DisplaySize.x - total_w - 15.0f, 15.0f};

	ImDrawList* dl = ImGui::GetForegroundDrawList(); // Always on top

	dl->AddRectFilled(pos, ImVec2(pos.x + total_w, pos.y + total_h), styled(col_header), 4.0f,
										ImDrawFlags_RoundCornersTop);
	dl->AddRect(pos, ImVec2(pos.x + total_w, pos.y + total_h), styled(col_groove_brd), 4.0f, ImDrawFlags_RoundCornersTop);

	ImVec2 wm_gmin(pos.x + 1.0f, pos.y + total_h - 10.0f);
	ImVec2 wm_gmax(pos.x + total_w - 1.0f, pos.y + total_h - 1.0f);
	ImU32 wm_gcol = styled(col_accent, 0.14f);
	ImU32 wm_trans = styled(col_accent, 0.0f);
	dl->AddRectFilledMultiColor(wm_gmin, wm_gmax, wm_trans, wm_trans, wm_gcol, wm_gcol);
	dl->AddLine(ImVec2(pos.x, pos.y + total_h - 1.0f), ImVec2(pos.x + total_w, pos.y + total_h - 1.0f),
							styled(col_accent, 0.9f), 1.0f);

	float cx = pos.x + pad.x;
	float cy = pos.y + pad.y;

	ImGui::PushFont(theme::font_bold);
	dl->AddText(ImVec2(cx, cy - 1.0f), styled(col_accent), "necrum");
	ImGui::PopFont();
	cx += w_logo;

	dl->AddText(ImVec2(cx, cy - 1.0f), styled(IM_COL32(100, 100, 100, 255)), " | ");
	cx += w_sep;

	dl->AddText(ImVec2(cx, cy - 1.0f), styled(col_text_dim), buf);
}

std::string get_key_name(int vk)
{
	if (vk == 0)
		return "NONE";

	switch (vk)
	{
	case VK_LBUTTON:
		return "M1";
	case VK_RBUTTON:
		return "M2";
	case VK_MBUTTON:
		return "M3";
	case VK_XBUTTON1:
		return "M4";
	case VK_XBUTTON2:
		return "M5";
	case VK_BACK:
		return "BACK";
	case VK_TAB:
		return "TAB";
	case VK_RETURN:
		return "ENTER";
	case VK_SHIFT:
		return "SHIFT";
	case VK_CONTROL:
		return "CTRL";
	case VK_MENU:
		return "ALT";
	case VK_PAUSE:
		return "PAUSE";
	case VK_CAPITAL:
		return "CAPS";
	case VK_ESCAPE:
		return "ESC";
	case VK_SPACE:
		return "SPACE";
	case VK_PRIOR:
		return "PGUP";
	case VK_NEXT:
		return "PGDN";
	case VK_END:
		return "END";
	case VK_HOME:
		return "HOME";
	case VK_LEFT:
		return "LEFT";
	case VK_UP:
		return "UP";
	case VK_RIGHT:
		return "RIGHT";
	case VK_DOWN:
		return "DOWN";
	case VK_INSERT:
		return "INS";
	case VK_DELETE:
		return "DEL";
	}

	if (vk >= VK_F1 && vk <= VK_F12)
	{
		char buf[8];
		snprintf(buf, sizeof(buf), "F%d", vk - VK_F1 + 1);
		return std::string(buf);
	}

	char buf[32] = {};
	UINT scan = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
	if (GetKeyNameTextA((LONG)scan << 16, buf, sizeof(buf)))
		return std::string(buf);

	snprintf(buf, sizeof(buf), "0x%02X", vk);
	return std::string(buf);
}

} // namespace ui
