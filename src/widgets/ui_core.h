#pragma once

#include "core/fonts/FontAwesome.h"
#include "pch.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <imgui.h>
#include <imgui_internal.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>

#undef min
#undef max

namespace ui
{

static constexpr ImU32 col_bg = IM_COL32(8, 8, 8, 255);
static constexpr ImU32 col_header = IM_COL32(14, 14, 14, 255);
static constexpr ImU32 col_sidebar = IM_COL32(10, 10, 10, 255);
static constexpr ImU32 col_content = IM_COL32(12, 12, 12, 255);
static constexpr ImU32 col_panel = IM_COL32(9, 9, 9, 255);
static constexpr ImU32 col_panel_title = IM_COL32(14, 14, 15, 255);
static constexpr ImU32 col_footer = IM_COL32(7, 7, 7, 255);
extern ImU32 col_accent;
extern ImU32 col_accent_dim;

static ImU32 hsv_to_u32(float h, float s, float v, float a = 1.0f)
{
	float r, g, b;
	ImGui::ColorConvertHSVtoRGB(h / 360.0f, s, v, r, g, b);
	return IM_COL32((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(a * 255));
}
static constexpr ImU32 col_text = IM_COL32(188, 188, 188, 255);
static constexpr ImU32 col_text_dim = IM_COL32(88, 88, 88, 255);
static constexpr ImU32 col_text_micro = IM_COL32(116, 116, 116, 255);
static constexpr ImU32 col_border = IM_COL32(32, 32, 32, 200);
static constexpr ImU32 col_groove = IM_COL32(16, 16, 16, 255);
static constexpr ImU32 col_groove_brd = IM_COL32(36, 36, 36, 180);
static constexpr ImU32 col_divider = IM_COL32(26, 26, 26, 255);

static constexpr float WIN_W = 680.0f;
static constexpr float WIN_H = 470.0f;
static constexpr float HEADER_H = 46.0f;
static constexpr float FOOTER_H = 22.0f;
static constexpr float SIDEBAR_W = 130.0f;
static constexpr float TAB_STRIP_H = 28.0f;

inline float g_speed = 12.0f;
inline float g_dt = 0.016f;

inline float alerp(float cur, float target)
{
	float dt = std::max(g_dt, 1.0f / 240.0f);
	return cur + (target - cur) * (1.0f - std::exp(-g_speed * dt));
}

// Per-control animation state
extern std::unordered_map<ImGuiID, float> s_anim;
static float& ganim(ImGuiID id, float def = 0.0f)
{
	auto it = s_anim.find(id);
	if (it == s_anim.end())
	{
		s_anim[id] = def;
		return s_anim[id];
	}
	return it->second;
}

static ImU32 styled(ImU32 c, float alpha_mul = 1.0f)
{
	float a = ((c >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f;
	a *= ImGui::GetStyle().Alpha * alpha_mul;
	return (c & ~IM_COL32_A_MASK) | (static_cast<ImU32>(a * 255.0f) << IM_COL32_A_SHIFT);
}

static ImU32 alpha_mul(ImU32 c, float mul)
{
	float a = ((c >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f;
	a = std::clamp(a * mul, 0.0f, 1.0f);
	return (c & ~IM_COL32_A_MASK) | (static_cast<ImU32>(a * 255.0f + 0.5f) << IM_COL32_A_SHIFT);
}

static const char* hdr_icons_fa[] = {ICON_FA_GUN, ICON_FA_EYE, ICON_FA_WRENCH, ICON_FA_BOLT, ICON_FA_GEAR};

static const int side_counts[] = {3, 3, 3, 2, 2};

extern int top_tab_;
extern int side_tab_;
extern int prev_top_tab_;
extern int prev_side_tab_;
extern float tab_fade_;
extern float anim_side_y_;
extern int weapon_tab_;

struct SideItem
{
	const char* icon;
	const char* label;
};

static std::vector<SideItem> get_sidebar_items(int top_tab)
{
	switch (top_tab)
	{
	case 0:
		return {{ICON_FA_GUN, "  RAGEBOT"},
						{ICON_FA_BOLT, "  EXPLOITS"},
						{ICON_FA_GHOST, "  ANTI-AIM"}};
	case 1:
		return {
				{ICON_FA_EYE, "  ESP"}, {ICON_FA_USER, "  CHAMS"}, {ICON_FA_SHIELD_HALVED, "  GLOW"}};
	case 2:
		return {
				{ICON_FA_WRENCH, "  GENERAL"}, {ICON_FA_GEAR, "  MOVEMENT"}, {ICON_FA_USER, "  SKINS"}};
	case 3:
		return {{ICON_FA_SHIELD_HALVED, "  AIMBOT"}, {ICON_FA_HAND_POINTER, "  TRIGGER"}};
	case 4:
		return {{ICON_FA_FILE_LINES, "  CONFIGS"}, {ICON_FA_CODE, "  SCRIPTS"}};
	default:
		return {};
	}
}

extern std::string search_query_norm_;
extern char search_query_[256];
extern bool search_focus_request_;
extern bool is_search_pass_;
extern bool is_searching_all_;
extern bool tab_has_hits[5];
extern int search_hits_count_;

static std::string normalize_search_text(const char* text)
{
	std::string out;
	if (!text)
		return out;

	out.reserve(strlen(text));
	bool pending_space = false;
	while (*text)
	{
		unsigned char ch = (unsigned char)(*text++);
		if (std::isspace(ch))
		{
			if (!out.empty())
				pending_space = true;
			continue;
		}
		if (pending_space)
		{
			out.push_back(' ');
			pending_space = false;
		}
		out.push_back((char)std::tolower(ch));
	}
	return out;
}

inline void sync_search_query()
{
	search_query_norm_ = normalize_search_text(search_query_);
}

inline bool has_search_query()
{
	return !search_query_norm_.empty();
}

inline bool search_match(std::initializer_list<const char*> terms)
{
	if (!has_search_query())
		return !is_search_pass_;

	for (const char* term : terms)
	{
		if (!term)
			continue;
		if (normalize_search_text(term).find(search_query_norm_) != std::string::npos)
		{
			if (is_search_pass_)
				search_hits_count_++;
			return true;
		}
	}
	return false;
}

inline bool page_has_search_hits(int top_tab, int side_tab,
																 const std::vector<std::vector<std::vector<std::string>>>& page_search_terms)
{
	if (!has_search_query())
		return true;

	if (top_tab >= 0 && top_tab < (int)page_search_terms.size() && side_tab >= 0 &&
			side_tab < (int)page_search_terms[top_tab].size())
	{
		for (const auto& term : page_search_terms[top_tab][side_tab])
		{
			if (normalize_search_text(term.c_str()).find(search_query_norm_) != std::string::npos)
				return true;
		}
	}
	return false;
}

inline void draw_search_empty_hint(const char* text)
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.42f, 0.42f, 0.45f, 1.0f)));
	ImGui::TextUnformatted(text);
	ImGui::PopStyleColor();
}

static void accent_glow_rect(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float alphaMul = 1.0f, ImU32 tint = 0)
{
	const ImU32 base_tint = tint ? tint : col_accent;
	const ImVec4 tf = ImGui::ColorConvertU32ToFloat4(base_tint);
	const int steps = 5;
	const float base_alpha = 0.14f * std::clamp(alphaMul, 0.0f, 1.0f);

	for (int i = 0; i < steps; ++i)
	{
		float t = (float)i / (float)(steps - 1);
		float fade = (1.0f - t) * (1.0f - t);
		float spread = 1.0f + (float)i * 1.7f;
		ImU32 glow =
				IM_COL32((int)(tf.x * 255.0f), (int)(tf.y * 255.0f), (int)(tf.z * 255.0f), (int)(255.0f * base_alpha * fade));
		dl->AddRectFilled(ImVec2(a.x - spread, a.y - spread), ImVec2(b.x + spread, b.y + spread), styled(glow),
											rounding + spread);
	}
}

static void accent_fill(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float alphaMul = 1.0f)
{
	accent_glow_rect(dl, a, b, rounding, alphaMul);
	dl->AddRectFilled(a, b, styled(col_accent, alphaMul), rounding);
}

extern bool tab_has_hits[5];

struct text_swap_anim
{
	std::string current;
	std::string previous;
	float blend = 1.0f;
};
extern std::unordered_map<ImGuiID, text_swap_anim> s_text_swap;

static text_swap_anim& text_swap_state(ImGuiID id, const char* text)
{
	std::string next = text ? text : "";
	text_swap_anim& st = s_text_swap[id];
	if (st.current.empty())
	{
		st.current = next;
		st.previous.clear();
		st.blend = 1.0f;
	}
	else if (st.current != next)
	{
		st.previous = st.current;
		st.current = next;
		st.blend = 0.0f;
	}
	st.blend = alerp(st.blend, 1.0f);
	if (st.blend > 0.995f)
		st.previous.clear();
	return st;
}

static void draw_text_swap_left(ImDrawList* dl, ImGuiID id, float left_x, float center_y, ImU32 col, const char* text)
{
	text_swap_anim& st = text_swap_state(id, text);
	float in_t = std::clamp(st.blend, 0.0f, 1.0f);
	float out_t = st.previous.empty() ? 0.0f : (1.0f - in_t);
	float shift = (1.0f - in_t) * 2.0f;

	if (out_t > 0.001f)
	{
		ImVec2 psz = ImGui::CalcTextSize(st.previous.c_str());
		dl->AddText(ImVec2(left_x, center_y - psz.y * 0.5f - shift), alpha_mul(col, out_t), st.previous.c_str());
	}

	if (in_t > 0.001f)
	{
		ImVec2 csz = ImGui::CalcTextSize(st.current.c_str());
		dl->AddText(ImVec2(left_x, center_y - csz.y * 0.5f + (1.0f - in_t) * 2.0f), alpha_mul(col, in_t),
								st.current.c_str());
	}
}

static void draw_text_swap_right(ImDrawList* dl, ImGuiID id, float right_x, float center_y, ImU32 col, const char* text)
{
	text_swap_anim& st = text_swap_state(id, text);
	float in_t = std::clamp(st.blend, 0.0f, 1.0f);
	float out_t = st.previous.empty() ? 0.0f : (1.0f - in_t);
	float shift = (1.0f - in_t) * 2.0f;

	if (out_t > 0.001f)
	{
		ImVec2 psz = ImGui::CalcTextSize(st.previous.c_str());
		dl->AddText(ImVec2(right_x - psz.x, center_y - psz.y * 0.5f - shift), alpha_mul(col, out_t), st.previous.c_str());
	}

	if (in_t > 0.001f)
	{
		ImVec2 csz = ImGui::CalcTextSize(st.current.c_str());
		dl->AddText(ImVec2(right_x - csz.x, center_y - csz.y * 0.5f + (1.0f - in_t) * 2.0f), alpha_mul(col, in_t),
								st.current.c_str());
	}
}

// Shared state for controls
extern ImGuiID s_listening;
extern std::unordered_map<ImGuiID, float[120]> s_history;
extern char s_color_clipboard[8]; // shared hex clipboard

// Internal utilities
std::string get_key_name(int vk);

} // namespace ui
