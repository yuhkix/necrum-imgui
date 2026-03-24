#include "menu.h"
#include "core/fonts/FontAwesome.h"
#include "core/web_image_imgui.h"
#include "widgets/ui_framework.h"
#include "menu/theme.h"
#include "pch.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <functional>
#include <imgui.h>
#include <imgui_internal.h>
#include <unordered_map>
#include <windows.h>

namespace menu
{
using namespace ui;

struct SideItem
{
	const char* icon;
	const char* label;
};

std::string Menu::format_key_name(int key) const
{
	return ui::get_key_name(key);
}

float Menu::draw_hotkey_overlay(const ImVec2& pos, float width)
{
	if (hotkey_alpha_ <= 0.001f)
		return 0.0f;

	struct HotkeyRow
	{
		const char* label;
		int* key;
		int* mode;
		bool* toggle_state;
		int* key_prev;
	};
	HotkeyRow rows[] = {
			{"Roll resolver", &roll_resolver_key_, &roll_resolver_mode_, &roll_resolver_toggle_state_,
			 &roll_resolver_key_prev_},
			{"Double tap", &double_tap_key_, &double_tap_mode_, &double_tap_toggle_state_, &double_tap_key_prev_}};

	std::vector<HotkeyRow*> active;
	for (auto& r : rows)
	{
		if ((r.mode && *r.mode == 2) || (r.key && *r.key != 0))
			active.push_back(&r);
	}

	constexpr float header_h = 32.0f;
	const float row_h = 24.0f;
	const float gap = 4.0f;
	const float h_padding = 14.0f;
	const float v_padding = 10.0f;

	float content_h = active.empty() ? row_h : (float)active.size() * row_h + ((float)active.size() - 1.0f) * gap;
	float height = header_h + v_padding * 2.0f + content_h;

	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize |
													 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
													 ImGuiWindowFlags_NoNav;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, hotkey_alpha_);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

	if (ImGui::Begin("##overlay_hotkeys", nullptr, flags))
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 wpos = ImGui::GetWindowPos();
		ImVec2 br(wpos.x + width, wpos.y + height);

		accent_glow_rect(dl, wpos, br, 8.0f, 0.45f, IM_COL32(0, 0, 0, 200));
		accent_glow_rect(dl, ImVec2(wpos.x + 2, wpos.y + 2), ImVec2(br.x - 2, br.y - 2), 7.0f, 0.20f, col_accent);

		draw_panel(dl, wpos, br, "Keybinds", 8.0f);

		// Active count badge
		char count_buf[32];
		snprintf(count_buf, sizeof(count_buf), "%zu active", active.size());
		ImVec2 count_sz = ImGui::CalcTextSize(count_buf);
		float badge_h = 16.0f;
		float badge_w = count_sz.x + 12.0f;
		ImVec2 badge_min(br.x - badge_w - 12.0f, wpos.y + 8.0f);
		dl->AddRectFilled(badge_min, ImVec2(badge_min.x + badge_w, badge_min.y + badge_h),
											styled(IM_COL32(35, 35, 38, 240)), badge_h * 0.5f);
		dl->AddText(ImVec2(badge_min.x + 6.0f, badge_min.y + 1.0f), styled(col_text_dim), count_buf);

		float y = wpos.y + header_h + v_padding;
		if (active.empty())
		{
			dl->AddText(ImVec2(wpos.x + h_padding, y), styled(col_text_dim), "No active binds");
		}
		else
		{
			for (auto* row : active)
			{
				// Handle toggle logic for mode 0 (Toggle)
				bool key_pressed_now = (row->key && *row->key != 0 && (GetAsyncKeyState(*row->key) & 0x8000)) != 0;
				bool key_pressed_before = (*row->key_prev) != 0;

				// Detect key press (transition from not pressed to pressed)
				if (key_pressed_now && !key_pressed_before && *row->mode == 0)
				{
					*row->toggle_state = !*row->toggle_state;
				}
				*row->key_prev = key_pressed_now ? 1 : 0;

				// Determine if this hotkey should show as active
				bool pressed = false;
				if (*row->mode == 0)
				{
					// Toggle mode: use toggle state
					pressed = *row->toggle_state;
				}
				else if (*row->mode == 1)
				{
					// Hold mode: use current key press state
					pressed = key_pressed_now;
				}
				else if (*row->mode == 2)
				{
					// Always mode: always active
					pressed = true;
				}

				float center_y = y + row_h * 0.5f;
				ImVec2 dot_pos(wpos.x + h_padding, center_y);

				// Draw glowy indicator dot
				if (pressed)
				{
					float pulse = (sinf((float)ImGui::GetTime() * 6.0f) * 0.5f + 0.5f);
					dl->AddCircle(dot_pos, 3.2f + pulse * 2.0f, styled(col_accent, 0.4f * (1.0f - pulse)), 20, 1.0f);
					dl->AddCircleFilled(dot_pos, 3.0f, styled(col_accent), 16);
				}
				else
				{
					dl->AddCircleFilled(dot_pos, 3.0f, styled(col_text_dim, 0.5f), 16);
				}

				// Align all text to center_y for better visual alignment
				float text_y = center_y - ImGui::GetFontSize() * 0.5f + 1.0f;

				dl->AddText(ImVec2(dot_pos.x + 12.0f, text_y), pressed ? styled(col_text) : styled(col_text_dim), row->label);

				std::string key_name = format_key_name(*row->key);
				const char* mode_name = (*row->mode == 1) ? "Hold" : (*row->mode == 2 ? "Always" : "Toggle");

				ImVec2 mode_sz = ImGui::CalcTextSize(mode_name);
				ImVec2 key_sz = ImGui::CalcTextSize(key_name.c_str());

				dl->AddText(ImVec2(br.x - h_padding - mode_sz.x, text_y), styled(col_text_dim, 0.6f), mode_name);
				dl->AddText(ImVec2(br.x - h_padding - mode_sz.x - 8.0f - key_sz.x, text_y),
										pressed ? styled(col_text) : styled(col_text_dim), key_name.c_str());

				y += row_h + gap;
			}
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);
	return height;
}

float Menu::draw_spectator_overlay(const ImVec2& pos, float width)
{
	if (spec_alpha_ <= 0.001f)
		return 0.0f;

	std::vector<std::string> names = spectator_names_;

	// For demo purposes, add a rounded avatar spectator if list is empty
	if (names.empty())
	{
		names.push_back("Necrum Demo");
		spectator_avatar_urls_["Necrum Demo"] = "https://i.ibb.co/k2tJg9Gx/necrum.png"; // Example premium-looking avatar
	}

	constexpr float header_h = 32.0f;
	const float row_h = 28.0f;
	const float gap = 4.0f;
	const float h_padding = 14.0f;
	const float v_padding = 10.0f;

	float content_h = names.empty() ? row_h : (float)names.size() * row_h + ((float)names.size() - 1.0f) * gap;
	float height = header_h + v_padding * 2.0f + content_h;

	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize |
													 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
													 ImGuiWindowFlags_NoNav;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, spec_alpha_);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

	if (ImGui::Begin("##overlay_specs", nullptr, flags))
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 wpos = ImGui::GetWindowPos();
		ImVec2 br(wpos.x + width, wpos.y + height);

		accent_glow_rect(dl, wpos, br, 8.0f, 0.45f, IM_COL32(0, 0, 0, 200));
		accent_glow_rect(dl, ImVec2(wpos.x + 2, wpos.y + 2), ImVec2(br.x - 2, br.y - 2), 7.0f, 0.20f, col_accent);

		draw_panel(dl, wpos, br, "Spectators", 8.0f);

		char count_buf[32];
		snprintf(count_buf, sizeof(count_buf), "%zu watching", names.size());
		ImVec2 count_sz = ImGui::CalcTextSize(count_buf);
		float badge_h = 16.0f;
		float badge_w = count_sz.x + 12.0f;
		ImVec2 badge_min(br.x - badge_w - 12.0f, wpos.y + 8.0f);
		dl->AddRectFilled(badge_min, ImVec2(badge_min.x + badge_w, badge_min.y + badge_h),
											styled(IM_COL32(35, 35, 38, 240)), badge_h * 0.5f);
		dl->AddText(ImVec2(badge_min.x + 6.0f, badge_min.y + 1.0f), styled(col_text_dim), count_buf);

		float y = wpos.y + header_h + v_padding;
		if (names.empty())
		{
			dl->AddText(ImVec2(wpos.x + h_padding, y), styled(col_text_dim), "Safe to play...");
		}
		else
		{
			auto draw_spectator = [&](const std::string& name)
			{
				float center_y = y + row_h * 0.5f;
				ImVec2 avatar_pos(wpos.x + h_padding + 10.0f, center_y);

				ImU32 avatar_col = avatar_color_for_name(name);
				std::string avatar_url = get_spectator_avatar_url(name);

				float avatar_radius = 10.0f;
				ImVec2 avatar_min(avatar_pos.x - avatar_radius, avatar_pos.y - avatar_radius);
				ImVec2 avatar_max(avatar_pos.x + avatar_radius, avatar_pos.y + avatar_radius);

				// Draw avatar if available, otherwise fallback to initials
				ImGui::AddWebImageRounded(avatar_url.c_str(), avatar_min, avatar_max, avatar_radius,
																	ImDrawFlags_RoundCornersAll, styled(IM_COL32_WHITE));

				dl->AddCircle(avatar_pos, 10.5f, styled(IM_COL32(255, 255, 255, 15)), 24, 1.0f);

				if (!web_image::is_web_image_loaded(avatar_url))
				{
					char initial[2] = {0};
					if (!name.empty())
						initial[0] = (char)std::toupper((unsigned char)name[0]);

					if (theme::font_bold && initial[0] != 0)
					{
						ImVec2 init_sz = theme::font_bold->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, initial);

						dl->AddCircleFilled(avatar_pos, 8.0f, alpha_mul(avatar_col, 0.3f), 16);

						dl->AddText(theme::font_bold, 11.0f,
												ImVec2(avatar_pos.x - init_sz.x * 0.5f, avatar_pos.y - init_sz.y * 0.5f + 0.5f),
												styled(IM_COL32(255, 255, 255, 180)), initial);
					}
				}

				dl->AddText(ImVec2(avatar_pos.x + 18.0f, y + row_h * 0.5f - 8.0f), styled(col_text), name.c_str());
				y += row_h + gap;
			};

			if (names.empty())
			{
				draw_spectator("necrum");
			}
			else
			{
				for (const auto& name : names)
				{
					draw_spectator(name);
				}
			}
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);
	return height;
}

float Menu::draw_bomb_timer_overlay(const ImVec2& pos, float width)
{
	if (bomb_alpha_ <= 0.001f)
		return 0.0f;

	constexpr float header_h = 32.0f;
	const float height = 152.0f;
	const float h_padding = 14.0f;
	const float v_padding = 10.0f;

	ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize |
													 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
													 ImGuiWindowFlags_NoNav;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, bomb_alpha_);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));

	if (ImGui::Begin("##overlay_bomb", nullptr, flags))
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 wpos = ImGui::GetWindowPos();
		ImVec2 br(wpos.x + width, wpos.y + height);

		accent_glow_rect(dl, wpos, br, 8.0f, 0.45f, IM_COL32(0, 0, 0, 200));
		accent_glow_rect(dl, ImVec2(wpos.x + 2, wpos.y + 2), ImVec2(br.x - 2, br.y - 2), 7.0f, 0.20f, col_accent);

		draw_panel(dl, wpos, br, "Bomb Timer", 8.0f);

		float total = (bomb_time_total_ > 0.1f) ? bomb_time_total_ : 40.0f;
		float time_left = bomb_time_left_;
		if (time_left < 0.0f || time_left > total)
		{
			float cycle = fmodf((float)ImGui::GetTime(), total + 3.0f);
			time_left = total - std::min(cycle, total);
		}
		float t_norm = ui::saturate(time_left / total);

		float dmg = std::max(0.0f, bomb_predicted_damage_);
		int hp_after = std::max(0, player_health_ - (int)std::round(dmg));
		bool survives = hp_after > 0;
		const char* fate = survives ? "SURVIVE" : "DEAD";
		ImU32 fate_col = survives ? styled(IM_COL32(120, 210, 140, 255)) : styled(IM_COL32(235, 90, 90, 255));

		ImVec2 fate_sz = ImGui::CalcTextSize(fate);
		float badge_h = 16.0f;
		float badge_w = fate_sz.x + 12.0f;
		ImVec2 badge_min(br.x - badge_w - 12.0f, wpos.y + 8.0f);
		dl->AddRectFilled(badge_min, ImVec2(badge_min.x + badge_w, badge_min.y + badge_h), fate_col, badge_h * 0.5f);
		dl->AddText(ImVec2(badge_min.x + 6.0f, badge_min.y + 1.0f), IM_COL32(10, 10, 15, 240), fate);

		char timer_buf[32];
		snprintf(timer_buf, sizeof(timer_buf), "%.1fs", time_left);
		ImFont* timer_font = theme::font_bold ? theme::font_bold : ImGui::GetFont();
		dl->AddText(timer_font, 28.0f, ImVec2(wpos.x + h_padding, wpos.y + header_h + 2.0f),
								styled(IM_COL32(240, 240, 245, 255)), timer_buf);

		ImVec2 bar_min(wpos.x + h_padding, wpos.y + header_h + 42.0f);
		ImVec2 bar_max(br.x - h_padding, bar_min.y + 14.0f);
		dl->AddRectFilled(bar_min, bar_max, styled(col_groove), 4.0f);

		float fill_w = (bar_max.x - bar_min.x) * t_norm;
		ImU32 bar_col = styled(col_accent);

		if (time_left < 10.0f)
		{
			float pulse = (sinf((float)ImGui::GetTime() * 10.0f) * 0.5f + 0.5f);
			dl->AddRectFilled(bar_min, ImVec2(bar_min.x + fill_w, bar_max.y), styled(col_accent, 0.4f * pulse), 4.0f);
		}

		dl->AddRectFilled(bar_min, ImVec2(bar_min.x + fill_w, bar_max.y), bar_col, 4.0f);
		dl->AddRect(bar_min, bar_max, styled(col_groove_brd), 4.0f);

		char hp_buf[64], dmg_buf[48];
		snprintf(hp_buf, sizeof(hp_buf), "Estimated HP: %d", hp_after);
		dl->AddText(ImVec2(wpos.x + h_padding, bar_max.y + 10.0f), fate_col, hp_buf);

		snprintf(dmg_buf, sizeof(dmg_buf), "Damage: %.0f", dmg);
		dl->AddText(ImVec2(wpos.x + h_padding, bar_max.y + 26.0f), styled(col_text_dim), dmg_buf);

		const char* summary = survives ? "Outside blast zone" : "Inside blast zone";
		ImVec2 sum_sz = ImGui::CalcTextSize(summary);
		dl->AddText(ImVec2(br.x - h_padding - sum_sz.x, bar_max.y + 26.0f),
								survives ? styled(col_text_dim) : styled(IM_COL32(235, 90, 90, 200)), summary);
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);
	return height;
}

void Menu::render_overlays()
{
	ImVec2 display = ImGui::GetIO().DisplaySize;
	const float overlay_w = 270.0f;
	const float margin = 18.0f;

	// Keybinds: Middle-Left edge
	ImVec2 hk_pos(margin, display.y * 0.45f);
	draw_hotkey_overlay(hk_pos, overlay_w);

	// Spectators: Middle-Right edge
	ImVec2 spec_pos(display.x - overlay_w - margin, display.y * 0.45f);
	draw_spectator_overlay(spec_pos, overlay_w);

	// Bomb Timer: Top-Center
	const float bomb_w = overlay_w;
	ImVec2 bomb_pos((display.x - bomb_w) * 0.5f, margin + 40.0f);
	draw_bomb_timer_overlay(bomb_pos, bomb_w);
}

void Menu::draw_content(float pw, float ph)
{
	(void)pw;
	(void)ph;

	sync_search_query();

	switch (ui::top_tab_)
	{
	case 0:
		switch (ui::side_tab_)
		{
		case 0:
			page_ragebot();
			break;
		case 1:
			page_exploits();
			break;
		case 2:
			page_antiaim();
			break;
		default:
			page_ragebot();
			break;
		}
		break;
	case 1:
		page_visuals();
		break;
	case 2:
		page_misc();
		break;
	case 3:
		page_legitbot();
		break;
	case 4:
		page_settings();
		break;
	default:
		page_ragebot();
		break;
	}
}
void Menu::draw_intro()

{

	float time = (float)ImGui::GetTime();

	if (intro_start_time_ < 0.0f)

		intro_start_time_ = time;

	float elapsed = time - intro_start_time_;

	ImVec2 display = ImGui::GetIO().DisplaySize;

	ImVec2 center(display.x * 0.5f, display.y * 0.5f);

	ImDrawList* dl = ImGui::GetForegroundDrawList();

	constexpr float APPEAR_END = 0.65f;

	constexpr float REVEAL_END = 1.45f;

	constexpr float HOLD_END = 2.40f;

	constexpr float OUT_END = 3.15f;

	constexpr float INTRO_END = 3.25f;

	if (elapsed >= INTRO_END)

	{

		intro_playing_ = false;

		return;
	}

	auto saturate = [](float v) { return ui::saturate(v); };

	auto smoothstep = [&](float a, float b, float v)

	{
		float t = saturate((v - a) / (b - a));

		return t * t * (3.0f - 2.0f * t);
	};

	auto ease_out_cubic = [&](float t)

	{
		t = saturate(t);

		float inv = 1.0f - t;

		return 1.0f - inv * inv * inv;
	};

	float appear_t = smoothstep(0.0f, APPEAR_END, elapsed);

	float reveal_t = smoothstep(0.22f, REVEAL_END, elapsed);

	float out_t = smoothstep(HOLD_END, OUT_END, elapsed);

	float intro_alpha = 1.0f - out_t * out_t;

	if (intro_alpha <= 0.001f)

	{

		intro_playing_ = false;

		return;
	}

	dl->AddRectFilledMultiColor(ImVec2(0, 0), display,

															IM_COL32(5, 5, 8, (int)(255 * intro_alpha)),

															IM_COL32(5, 5, 8, (int)(255 * intro_alpha)),

															IM_COL32(2, 2, 4, (int)(255 * intro_alpha)),

															IM_COL32(2, 2, 4, (int)(255 * intro_alpha)));

	ImVec4 acf = ImGui::ColorConvertU32ToFloat4(col_accent);

	ImVec2 ambient(display.x * 0.72f, display.y * 0.28f);

	for (int i = 0; i < 3; ++i)

	{

		float t = (float)i / 2.0f;

		float radius = 230.0f + 90.0f * i;

		float a = (0.030f * (1.0f - t)) * intro_alpha;

		dl->AddCircleFilled(ambient, radius,

												IM_COL32((int)(acf.x * 255.0f), (int)(acf.y * 255.0f),

																 (int)(acf.z * 255.0f), (int)(a * 255.0f)),

												72);
	}

	float panel_alpha = appear_t * intro_alpha;

	float panel_scale = 0.975f + 0.025f * appear_t;

	float panel_y_off = (1.0f - appear_t) * 10.0f - out_t * 6.0f;

	float panel_w = 324.0f * panel_scale;

	float panel_h = 146.0f * panel_scale;

	float header_h = 30.0f * panel_scale;

	ImVec2 pmin(center.x - panel_w * 0.5f,

							center.y - panel_h * 0.5f + panel_y_off);

	ImVec2 pmax(pmin.x + panel_w, pmin.y + panel_h);

	dl->AddRectFilled(ImVec2(pmin.x - 8.0f, pmin.y - 8.0f),

										ImVec2(pmax.x + 8.0f, pmax.y + 8.0f),

										IM_COL32(0, 0, 0, (int)(36.0f * panel_alpha)), 8.0f);

	dl->AddRectFilled(pmin, ImVec2(pmax.x, pmin.y + header_h),

										styled(col_header, panel_alpha), 4.0f,

										ImDrawFlags_RoundCornersTop);

	dl->AddRectFilled(ImVec2(pmin.x, pmin.y + header_h), pmax,

										styled(col_panel, panel_alpha), 4.0f,

										ImDrawFlags_RoundCornersBottom);

	dl->AddRect(pmin, pmax, styled(col_groove_brd, panel_alpha), 4.0f);

	dl->AddLine(ImVec2(pmin.x + 1.0f, pmin.y + header_h - 1.0f),

							ImVec2(pmax.x - 1.0f, pmin.y + header_h - 1.0f),

							styled(IM_COL32(82, 82, 88, 255), panel_alpha * 0.34f), 1.0f);

	float logo_alpha = reveal_t * intro_alpha;

	float logo_y_off = (1.0f - reveal_t) * 5.0f - out_t * 2.0f;

	float logo_base_y = pmin.y + 66.0f * panel_scale + logo_y_off;

	if (logo_alpha > 0.01f)

	{

		const char* logo_url = "https://i.ibb.co/k2tJg9Gx/necrum.png";

		ImTextureID logo_tex = ::web_image::get_web_image(logo_url);

		if (logo_tex)

		{

			float lsize = 56.0f * panel_scale;

			ImVec2 lpos(center.x - lsize * 0.5f, logo_base_y - lsize * 0.5f);

			dl->AddImageRounded(logo_tex, lpos, ImVec2(lpos.x + lsize, lpos.y + lsize), {0, 0}, {1, 1},
													IM_COL32(255, 255, 255, (int)(255.0f * logo_alpha)), 12.0f * panel_scale);
		}
	}

	if (theme::font_logo && logo_alpha > 0.01f)

	{

		float font_size = 22.0f * panel_scale;

		const char* name = "necrum";

		ImVec2 name_ts =

				theme::font_logo->CalcTextSizeA(font_size, FLT_MAX, 0.0f, name);

		ImVec2 name_pos(center.x - name_ts.x * 0.5f,

										logo_base_y + 36.0f * panel_scale);

		dl->AddText(theme::font_logo, font_size,

								ImVec2(name_pos.x, name_pos.y + 1.0f),

								IM_COL32(0, 0, 0, (int)(108 * logo_alpha)), name);

		float clip_w = name_ts.x * reveal_t;

		if (clip_w > 0.5f)

		{

			dl->PushClipRect(

					name_pos, ImVec2(name_pos.x + clip_w, name_pos.y + name_ts.y + 2.0f),

					true);

			dl->AddText(theme::font_logo, font_size, name_pos,

									IM_COL32(220, 220, 225, (int)(255 * logo_alpha)), name);

			dl->PopClipRect();
		}
	}

	// Accent line + glow (matching watermark/subtab style).

	float line_t = smoothstep(0.48f, REVEAL_END, elapsed);

	float line_w = (panel_w - 72.0f * panel_scale) * line_t;

	float line_y = pmax.y - 14.0f * panel_scale;

	ImVec2 gmin(center.x - line_w * 0.5f, line_y - 10.0f);

	ImVec2 gmax(center.x + line_w * 0.5f, line_y - 1.0f);

	ImU32 gcol = styled(col_accent, 0.14f * intro_alpha);

	ImU32 gclear = styled(col_accent, 0.0f);

	dl->AddRectFilledMultiColor(gmin, gmax, gclear, gclear, gcol, gcol);

	float breathe = 0.86f + 0.14f * sinf(time * 2.1f);

	dl->AddLine(ImVec2(center.x - line_w * 0.5f, line_y - 1.0f),

							ImVec2(center.x + line_w * 0.5f, line_y - 1.0f),

							styled(col_accent, 0.90f * intro_alpha * breathe), 1.0f);
}

void Menu::render()
{
	if (GetAsyncKeyState(VK_INSERT) & 1)
		target_alpha_ = (target_alpha_ > 0.5f) ? 0.0f : 1.0f;

	// Recompute accent colors from HSV state
	col_accent = hsv_to_u32(accent_hue_, accent_sat_, accent_val_);
	col_accent_dim = hsv_to_u32(accent_hue_, accent_sat_, accent_val_ * 0.8f, 0.78f);

	esp_preview_hold_ = false;
	g_speed = anim_speed_;
	g_dt = ImGui::GetIO().DeltaTime;

	alpha_ += (target_alpha_ - alpha_) * anim_speed_ * g_dt;
	alpha_ = ui::saturate(alpha_);

	if (std::abs(alpha_ - target_alpha_) < 0.005f)
		alpha_ = target_alpha_;

	// Animate overlay alphas smoothly using user's speed
	auto update_overlay_alpha = [&](float& alpha, bool show)
	{
		float target = show ? 1.0f : 0.0f;
		alpha += (target - alpha) * anim_speed_ * g_dt;
		alpha = ui::saturate(alpha);
		if (std::abs(alpha - target) < 0.005f)
			alpha = target;
	};

	update_overlay_alpha(hotkey_alpha_, show_hotkey_overlay_);
	update_overlay_alpha(spec_alpha_, show_spectator_overlay_);
	update_overlay_alpha(bomb_alpha_, show_bomb_overlay_);

	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false))
		search_focus_request_ = true;

	if (intro_playing_)
	{
		draw_intro();
	}
	else
	{
		ui::draw_watermark();
		render_overlays();

		if (alpha_ > 0.005f)
		{
			render_main();
		}
	}

	render_notifications();
}

void Menu::render_main()
{
	// Center the menu on screen (always)
	ImVec2 display = ImGui::GetIO().DisplaySize;
	ImVec2 menu_pos((display.x - WIN_W) * 0.5f, (display.y - WIN_H) * 0.5f);

	if (!intro_centered_)
	{
		ImGui::SetNextWindowPos(menu_pos, ImGuiCond_Always);
		intro_centered_ = true;
		add_notification("Successfully loaded menu!", NotifyType::_SUCCESS);
	}
	else
	{
		ImGui::SetNextWindowPos(menu_pos, ImGuiCond_Once);
	}

	ImGui::SetNextWindowSize(ImVec2(WIN_W, WIN_H), ImGuiCond_Once);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
													 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

	if (esp_dragging_ >= 0 || esp_preview_hold_)
		flags |= ImGuiWindowFlags_NoMove;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha_);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.03f, 0.03f, 0.04f, alpha_));

	bool open = true;
	if (ImGui::Begin("##menu", &open, flags))
	{
		ImVec2 ws = ImGui::GetWindowSize();
		float body_h = ws.y - HEADER_H - FOOTER_H;
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 wp = ImGui::GetWindowPos();

		dl->AddRect(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), styled(IM_COL32(40, 38, 52, 50)), 5.0f);

		ui::draw_header(ws.x);

		ImGui::SetCursorPos(ImVec2(0, HEADER_H));
		ImGui::BeginChild("##sb", ImVec2(SIDEBAR_W, body_h), false, ImGuiWindowFlags_NoScrollbar);
		ui::draw_sidebar(body_h);
		ImGui::EndChild();

		ImGui::SameLine(0, 0);

		float cw = ws.x - SIDEBAR_W;
		ImGui::BeginChild("##ct", ImVec2(cw, body_h), false, ImGuiWindowFlags_NoScrollbar);

		ui::draw_content(cw, body_h, [this, cw, body_h]() { this->draw_content(cw, body_h); });

		ImGui::EndChild();

		ui::draw_footer(ws.x);
	}

	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);
}

void Menu::add_notification(const char* text, NotifyType type)

{

	Notification n;

	n.message = text;

	n.type = type;

	n.time = (float)ImGui::GetTime();

	n.expiration = 4.0f; // 4 seconds duration

	n.anim = 0.0f;

	notifications_.push_back(n);
}

void Menu::render_notifications()

{

	ImDrawList* dl = ImGui::GetForegroundDrawList(); // Always on top

	ImVec2 display_size = ImGui::GetIO().DisplaySize;

	float dt = ImGui::GetIO().DeltaTime;

	float margin = 20.0f;

	float width = 280.0f;

	float height = 45.0f;

	float spacing = 10.0f;

	float current_y = display_size.y - margin - height;

	for (int i = (int)notifications_.size() - 1; i >= 0; i--)

	{

		auto& n = notifications_[i];

		// Handle lifetime

		n.expiration -= dt;

		// Smooth animation (Lerp progress)

		float target_anim = (n.expiration > 0.0f) ? 1.0f : 0.0f;

		n.anim += (target_anim - n.anim) * 10.0f * dt;

		if (n.expiration <= -1.0f && n.anim < 0.01f)

		{

			notifications_.erase(notifications_.begin() + i);

			continue;
		}

		// Slide in from right and fade

		float x_offset = (1.0f - n.anim) * (width + margin);

		float alpha = n.anim;

		ImVec2 pos_min(display_size.x - margin - width + x_offset, current_y);

		ImVec2 pos_max(display_size.x - margin + x_offset, current_y + height);

		// Accent line / icon color based on notification type

		ImU32 accent_base;

		const char* icon = "";

		switch (n.type)

		{

		case NotifyType::_SUCCESS:

			accent_base = IM_COL32(46, 204, 113, 255);

			icon = ICON_FA_CIRCLE_CHECK;

			break;

		case NotifyType::_INFO:

			accent_base = IM_COL32(52, 152, 219, 255);

			icon = ICON_FA_CIRCLE_INFO;

			break;

		case NotifyType::_WARNING:

			accent_base = IM_COL32(241, 196, 15, 255);

			icon = ICON_FA_CIRCLE_EXCLAMATION;

			break;

		case NotifyType::_ERROR:

			accent_base = IM_COL32(231, 76, 60, 255);

			icon = ICON_FA_CIRCLE_XMARK;

			break;
		}

		ImU32 accent = alpha_mul(accent_base, alpha);

		ImU32 bg_col = styled(IM_COL32(14, 14, 16, (int)(242 * alpha)));

		ImU32 border_col = styled(IM_COL32(40, 40, 45, (int)(188 * alpha)));

		dl->AddRectFilled(pos_min, pos_max, bg_col, 5.0f,

											ImDrawFlags_RoundCornersTop);

		dl->AddRect(pos_min, pos_max, border_col, 5.0f, ImDrawFlags_RoundCornersTop,

								1.0f);

		ImVec2 bot_gmin(pos_min.x + 1.0f, pos_max.y - 10.0f);

		ImVec2 bot_gmax(pos_max.x - 1.0f, pos_max.y - 1.0f);

		ImU32 bot_col = alpha_mul(accent_base, alpha * 0.14f);

		ImU32 bot_trans = alpha_mul(accent_base, 0.0f);

		dl->AddRectFilledMultiColor(bot_gmin, bot_gmax, bot_trans, bot_trans,

																bot_col, bot_col);

		dl->AddLine(ImVec2(pos_min.x, pos_max.y - 1.0f),

								ImVec2(pos_max.x, pos_max.y - 1.0f),

								alpha_mul(accent_base, alpha * 0.90f), 1.0f);

		// Icon & Text

		ImVec2 text_pos(pos_min.x + 10,

										pos_min.y + (height - ImGui::GetFontSize()) * 0.5f);

		// Draw Icon (if FontAwesome is loaded, otherwise just text)

		dl->AddText(text_pos, alpha_mul(accent, 0.92f), icon);

		ImVec2 msg_pos(text_pos.x + 22, text_pos.y);

		dl->AddText(msg_pos, alpha_mul(IM_COL32(240, 240, 245, 255), alpha),

								n.message.c_str());

		current_y -= (height + spacing) * n.anim;
	}
}

void Menu::sync_search_query()
{
	static char last_query[256] = {0};
	if (strcmp(last_query, ui::search_query_) != 0)
	{
		strcpy_s(last_query, ui::search_query_);
		run_search_pass();
	}
}

void Menu::run_search_pass()
{
	if (!ui::has_search_query())
	{
		for (int i = 0; i < 5; ++i)
		{
			ui::tab_has_hits[i] = true;
			for (int s = 0; s < 20; ++s)
				ui::side_tab_has_hits[i][s] = true;
		}
		return;
	}

	ui::is_search_pass_ = true;
	int old_top = ui::top_tab_;
	int old_side = ui::side_tab_;

	int best_tab = -1;
	int max_hits = 0;
	int best_side = 0;

	ImGui::Begin("##search_dry_run", nullptr,
							 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground |
									 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
									 ImGuiWindowFlags_NoNav);
	ImGui::SetWindowPos(ImVec2(-9999, -9999));

	for (int i = 0; i < 5; ++i)
	{
		ui::top_tab_ = i;
		int group_hits = 0;
		int sides = ui::side_counts[i];

		for (int s = 0; s < sides; ++s)
		{
			ui::search_hits_count_ = 0;
			ui::side_tab_ = s;

			ImGui::PushID(i * 100 + s);
			switch (i)
			{
			case 0:
				if (s == 0)
					page_ragebot();
				else if (s == 1)
					page_exploits();
				else if (s == 2)
					page_antiaim();
				break;
			case 1:
				page_visuals();
				break;
			case 2:
				page_misc();
				break;
			case 3:
				page_legitbot();
				break;
			case 4:
				page_settings();
				break;
			}
			ImGui::PopID();

			ui::side_tab_has_hits[i][s] = (ui::search_hits_count_ > 0);
			group_hits += ui::search_hits_count_;

			if (ui::search_hits_count_ > max_hits)
			{
				max_hits = ui::search_hits_count_;
				best_tab = i;
				best_side = s;
			}
		}
		ui::tab_has_hits[i] = (group_hits > 0);
	}

	ImGui::End();

	if (best_tab != -1)
	{
		ui::top_tab_ = best_tab;
		ui::side_tab_ = best_side;
	}
	else
	{
		ui::top_tab_ = old_top;
		ui::side_tab_ = old_side;
	}

	ui::is_search_pass_ = false;
	ui::search_hits_count_ = 0;
}

std::string Menu::get_spectator_avatar_url(const std::string& name)
{
	auto it = spectator_avatar_urls_.find(name);
	if (it != spectator_avatar_urls_.end())
		return it->second;
	return "";
}

ImU32 Menu::avatar_color_for_name(const std::string& name)
{
	size_t h = std::hash<std::string>{}(name);
	return IM_COL32((h & 0x7F) + 100, ((h >> 8) & 0x7F) + 100, ((h >> 16) & 0x7F) + 100, 255);
}

} // namespace menu
