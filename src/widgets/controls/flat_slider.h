#pragma once
#include "../ui_core.h"

namespace ui
{
inline bool flat_slider(const char* id, float* v, float lo, float hi, const char* fmt = "%.2f%%",
												float display_scale = 100.0f)
{
	ImGui::PushID(id);
	ImGuiID uid = ImGui::GetID("##sl");
	float& vis = ganim(uid, (*v - lo) / (hi - lo));

	float w = ImGui::CalcItemWidth();
	float h = 14.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##sl", ImVec2(w, h));
	if (ImGui::IsItemActive())
	{
		float t = std::clamp((ImGui::GetIO().MousePos.x - pos.x) / w, 0.0f, 1.0f);
		*v = lo + t * (hi - lo);
	}
	float target_t = (*v - lo) / (hi - lo);
	vis = alerp(vis, target_t);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 gmin = pos, gmax(pos.x + w, pos.y + h);
	dl->AddRectFilled(gmin, gmax, styled(col_groove), 3.0f);
	dl->AddRect(gmin, gmax, styled(col_groove_brd), 3.0f);
	if (vis > 0.005f)
		dl->AddRectFilled(ImVec2(gmin.x + 1, gmin.y + 1), ImVec2(gmin.x + 1 + (w - 2) * vis, gmax.y - 1),
											styled(col_accent_dim), 2.0f);

	char buf[32];
	snprintf(buf, sizeof(buf), fmt, *v * display_scale);
	ImVec2 tsz = ImGui::CalcTextSize(buf);
	dl->AddText(ImVec2(gmax.x - tsz.x - 4, gmin.y + (h - tsz.y) * 0.5f), styled(col_text_micro), buf);

	ImGui::PopID();
	return ImGui::IsItemActive();
}

inline bool flat_slider(const char* id, int* v, int lo, int hi, const char* fmt = "%d")
{
	ImGui::PushID(id);
	ImGuiID uid = ImGui::GetID("##sl");
	float& vis = ganim(uid, (float)(*v - lo) / (hi - lo));

	float w = ImGui::CalcItemWidth();
	float h = 14.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##sl", ImVec2(w, h));
	if (ImGui::IsItemActive())
	{
		float t = std::clamp((ImGui::GetIO().MousePos.x - pos.x) / w, 0.0f, 1.0f);
		*v = lo + (int)std::round(t * (hi - lo));
	}
	float target_t = (float)(*v - lo) / (hi - lo);
	vis = alerp(vis, target_t);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 gmin = pos, gmax(pos.x + w, pos.y + h);
	dl->AddRectFilled(gmin, gmax, styled(col_groove), 3.0f);
	dl->AddRect(gmin, gmax, styled(col_groove_brd), 3.0f);
	if (vis > 0.005f)
		dl->AddRectFilled(ImVec2(gmin.x + 1, gmin.y + 1), ImVec2(gmin.x + 1 + (w - 2) * vis, gmax.y - 1),
											styled(col_accent_dim), 2.0f);

	char buf[32];
	snprintf(buf, sizeof(buf), fmt, *v);
	ImVec2 tsz = ImGui::CalcTextSize(buf);
	dl->AddText(ImVec2(gmax.x - tsz.x - 4, gmin.y + (h - tsz.y) * 0.5f), styled(col_text_micro), buf);

	ImGui::PopID();
	return ImGui::IsItemActive();
}

inline bool range_slider(const char* id, float* lo, float* hi, float mn, float mx, const char* fmt = "%.0f - %.0f")
{
	ImGui::PushID(id);
	ImGuiID uid_lo = ImGui::GetID("##rlo");
	ImGuiID uid_hi = ImGui::GetID("##rhi");
	float& vis_lo = ganim(uid_lo, (*lo - mn) / (mx - mn));
	float& vis_hi = ganim(uid_hi, (*hi - mn) / (mx - mn));

	float w = ImGui::CalcItemWidth();
	float h = 14.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##rs", ImVec2(w, h));

	if (ImGui::IsItemActive())
	{
		float mx_t = std::clamp((ImGui::GetIO().MousePos.x - pos.x) / w, 0.0f, 1.0f);
		float mv = mn + mx_t * (mx - mn);
		float d_lo = fabsf(mx_t - (*lo - mn) / (mx - mn));
		float d_hi = fabsf(mx_t - (*hi - mn) / (mx - mn));
		if (d_lo < d_hi)
			*lo = std::clamp(std::min(mv, *hi - 0.01f), mn, mx);
		else
			*hi = std::clamp(std::max(mv, *lo + 0.01f), mn, mx);
	}
	vis_lo = alerp(vis_lo, (*lo - mn) / (mx - mn));
	vis_hi = alerp(vis_hi, (*hi - mn) / (mx - mn));

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 gmin = pos, gmax(pos.x + w, pos.y + h);
	dl->AddRectFilled(gmin, gmax, styled(col_groove), 3.0f);
	dl->AddRect(gmin, gmax, styled(col_groove_brd), 3.0f);

	float x_lo = gmin.x + 1 + (w - 2) * vis_lo;
	float x_hi = gmin.x + 1 + (w - 2) * vis_hi;
	dl->AddRectFilled(ImVec2(x_lo, gmin.y + 1), ImVec2(x_hi, gmax.y - 1), styled(col_accent_dim));
	accent_glow_rect(dl, ImVec2(x_lo - 2, gmin.y), ImVec2(x_lo + 2, gmax.y), 0.0f);
	dl->AddRectFilled(ImVec2(x_lo - 2, gmin.y), ImVec2(x_lo + 2, gmax.y), styled(col_accent));
	accent_glow_rect(dl, ImVec2(x_hi - 2, gmin.y), ImVec2(x_hi + 2, gmax.y), 0.0f);
	dl->AddRectFilled(ImVec2(x_hi - 2, gmin.y), ImVec2(x_hi + 2, gmax.y), styled(col_accent));

	char buf[32];
	snprintf(buf, sizeof(buf), fmt, *lo * 100.0f, *hi * 100.0f);
	ImVec2 ts = ImGui::CalcTextSize(buf);
	dl->AddText(ImVec2(gmax.x - ts.x - 4, gmin.y + (h - ts.y) * 0.5f), styled(col_text_micro), buf);

	ImGui::PopID();
	return ImGui::IsItemActive();
}

inline bool range_slider(const char* id, int* lo, int* hi, int mn, int mx, const char* fmt = "%d - %d")
{
	ImGui::PushID(id);
	ImGuiID uid_lo = ImGui::GetID("##rlo");
	ImGuiID uid_hi = ImGui::GetID("##rhi");
	float& vis_lo = ganim(uid_lo, (float)(*lo - mn) / (mx - mn));
	float& vis_hi = ganim(uid_hi, (float)(*hi - mn) / (mx - mn));

	float w = ImGui::CalcItemWidth();
	float h = 14.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##rs", ImVec2(w, h));

	if (ImGui::IsItemActive())
	{
		float mx_t = std::clamp((ImGui::GetIO().MousePos.x - pos.x) / w, 0.0f, 1.0f);
		float mv = mn + mx_t * (mx - mn);
		float d_lo = fabsf(mx_t - (float)(*lo - mn) / (mx - mn));
		float d_hi = fabsf(mx_t - (float)(*hi - mn) / (mx - mn));
		if (d_lo < d_hi)
			*lo = std::clamp((int)std::min(mv, (float)*hi - 1.0f), mn, mx);
		else
			*hi = std::clamp((int)std::max(mv, (float)*lo + 1.0f), mn, mx);
	}
	vis_lo = alerp(vis_lo, (float)(*lo - mn) / (mx - mn));
	vis_hi = alerp(vis_hi, (float)(*hi - mn) / (mx - mn));

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 gmin = pos, gmax(pos.x + w, pos.y + h);
	dl->AddRectFilled(gmin, gmax, styled(col_groove), 3.0f);
	dl->AddRect(gmin, gmax, styled(col_groove_brd), 3.0f);

	float x_lo = gmin.x + 1 + (w - 2) * vis_lo;
	float x_hi = gmin.x + 1 + (w - 2) * vis_hi;
	dl->AddRectFilled(ImVec2(x_lo, gmin.y + 1), ImVec2(x_hi, gmax.y - 1), styled(col_accent_dim));
	accent_glow_rect(dl, ImVec2(x_lo - 2, gmin.y), ImVec2(x_lo + 2, gmax.y), 0.0f);
	dl->AddRectFilled(ImVec2(x_lo - 2, gmin.y), ImVec2(x_lo + 2, gmax.y), styled(col_accent));
	accent_glow_rect(dl, ImVec2(x_hi - 2, gmin.y), ImVec2(x_hi + 2, gmax.y), 0.0f);
	dl->AddRectFilled(ImVec2(x_hi - 2, gmin.y), ImVec2(x_hi + 2, gmax.y), styled(col_accent));

	char buf[32];
	snprintf(buf, sizeof(buf), fmt, *lo, *hi);
	ImVec2 ts = ImGui::CalcTextSize(buf);
	dl->AddText(ImVec2(gmax.x - ts.x - 4, gmin.y + (h - ts.y) * 0.5f), styled(col_text_micro), buf);

	ImGui::PopID();
	return ImGui::IsItemActive();
}
} // namespace ui
