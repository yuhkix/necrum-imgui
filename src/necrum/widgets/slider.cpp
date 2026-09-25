#include "internal.h"

namespace nc
{

using namespace detail;

namespace
{
struct Groove
{
	ImVec2 min, max;
	float w;
};

// Reserves the groove item and returns its rect. `id` is the widget id.
Groove groove_item(const char* label, float height)
{
	float w = stacked_label(label);
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##sl", ImVec2(w, height));
	return {pos, ImVec2(pos.x + w, pos.y + height), w};
}

void draw_groove(ImDrawList* dl, const Groove& g, float hover, float active)
{
	const Theme& t = theme();
	dl->AddRectFilled(g.min, g.max, styled(t.field_bg), t.frame_rounding);
	dl->AddRect(g.min, g.max, styled(lerp_color(t.field_border, t.accent_col, hover * 0.35f + active * 0.3f)),
							t.frame_rounding);
}

// Value text sits right-aligned on the label row, above the groove.
void draw_value(ImDrawList* dl, const Groove& g, const char* txt, float active)
{
	ImVec2 sz = ImGui::CalcTextSize(txt);
	const Theme& t = theme();
	dl->AddText(ImVec2(g.max.x - sz.x, g.min.y - sz.y - 5.0f), styled(lerp_color(t.text_dim, t.text, active)), txt);
}

float mouse_t(const Groove& g)
{
	return saturate((ImGui::GetIO().MousePos.x - g.min.x - 1.0f) / std::max(1.0f, g.w - 2.0f));
}

template <typename T>
bool slider_impl(const char* label, T* v, T lo, T hi, const char* fmt, float display_scale)
{
	if (!search::filter(label))
		return false;
	const Theme& t = theme();
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##sl");

	const float range = (float)(hi - lo);
	auto norm = [&](T x) { return range != 0.0f ? saturate((float)(x - lo) / range) : 0.0f; };

	Groove g = groove_item(label, t.slider_height);
	const T before = *v;
	if (ImGui::IsItemActive())
	{
		float k = mouse_t(g);
		if constexpr (std::is_integral_v<T>)
			*v = (T)(lo + (T)std::lround(k * range));
		else
			*v = (T)(lo + k * range);
	}
	// Arrow keys nudge the value while hovered/focused.
	if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
	{
		T step = std::is_integral_v<T> ? (T)1 : (T)(range / 100.0f);
		if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
			*v = clamp((T)(*v - step), lo, hi);
		if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
			*v = clamp((T)(*v + step), lo, hi);
	}
	ItemAnim a = item_anim(id);
	float vis = anim::animate(id, norm(*v));
	ImGui::PopID();

	ImDrawList* dl = ImGui::GetWindowDrawList();
	draw_groove(dl, g, a.hover, a.active);
	if (vis > 0.002f)
	{
		ImVec2 fmin(g.min.x + 1.0f, g.min.y + 1.0f);
		ImVec2 fmax(g.min.x + 1.0f + (g.w - 2.0f) * vis, g.max.y - 1.0f);
		if (a.active > 0.01f)
			draw::glow(dl, fmin, fmax, t.frame_rounding - 1.0f, a.active * 0.5f);
		dl->AddRectFilled(fmin, fmax, styled(lerp_color(t.accent_dim, t.accent_col, a.hover * 0.3f + a.active * 0.5f)),
											t.frame_rounding - 1.0f);
	}

	char buf[64];
	if constexpr (std::is_integral_v<T>)
		snprintf(buf, sizeof(buf), fmt, *v);
	else
		snprintf(buf, sizeof(buf), fmt, (double)(*v * display_scale));
	draw_value(dl, g, buf, a.active);

	bool changed = *v != before;
	if (changed)
		ImGui::MarkItemEdited(id);
	return changed;
}

template <typename T>
bool range_impl(const char* label, T* lo, T* hi, T mn, T mx, const char* fmt, float display_scale)
{
	if (!search::filter(label))
		return false;
	const Theme& t = theme();
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##sl");
	ImGuiID grab_id = anim::key(id, "grab");

	const float range = (float)(mx - mn);
	auto norm = [&](T x) { return range != 0.0f ? saturate((float)(x - mn) / range) : 0.0f; };

	Groove g = groove_item(label, t.slider_height);
	const T before_lo = *lo, before_hi = *hi;

	// Decide which handle to drag when the drag starts, then keep it.
	float& grab = anim::value(grab_id, 0.0f);
	if (ImGui::IsItemActivated())
	{
		float k = mouse_t(g);
		grab = std::fabs(k - norm(*lo)) <= std::fabs(k - norm(*hi)) ? 0.0f : 1.0f;
		if (*lo == *hi)
			grab = k < norm(*lo) ? 0.0f : 1.0f;
	}
	if (ImGui::IsItemActive())
	{
		float k = mouse_t(g);
		T val;
		if constexpr (std::is_integral_v<T>)
			val = (T)(mn + (T)std::lround(k * range));
		else
			val = (T)(mn + k * range);
		if (grab < 0.5f)
			*lo = clamp(std::min(val, *hi), mn, mx);
		else
			*hi = clamp(std::max(val, *lo), mn, mx);
	}
	ItemAnim a = item_anim(id);
	float vlo = anim::animate(anim::key(id, "lo"), norm(*lo));
	float vhi = anim::animate(anim::key(id, "hi"), norm(*hi));
	ImGui::PopID();

	ImDrawList* dl = ImGui::GetWindowDrawList();
	draw_groove(dl, g, a.hover, a.active);
	float xlo = g.min.x + 1.0f + (g.w - 2.0f) * vlo;
	float xhi = g.min.x + 1.0f + (g.w - 2.0f) * vhi;
	dl->AddRectFilled(ImVec2(xlo, g.min.y + 1.0f), ImVec2(xhi, g.max.y - 1.0f), styled(t.accent_dim));
	for (float x : {xlo, xhi})
	{
		ImVec2 hmin(x - 2.0f, g.min.y), hmax(x + 2.0f, g.max.y);
		draw::glow(dl, hmin, hmax, 1.0f, 0.6f + a.active * 0.4f);
		dl->AddRectFilled(hmin, hmax, styled(t.accent_col), 1.0f);
	}

	char buf[64];
	if constexpr (std::is_integral_v<T>)
		snprintf(buf, sizeof(buf), fmt, *lo, *hi);
	else
		snprintf(buf, sizeof(buf), fmt, (double)(*lo * display_scale), (double)(*hi * display_scale));
	draw_value(dl, g, buf, a.active);

	bool changed = *lo != before_lo || *hi != before_hi;
	if (changed)
		ImGui::MarkItemEdited(id);
	return changed;
}
} // namespace

bool slider(const char* label, float* v, float min, float max, const char* fmt, float display_scale)
{
	return slider_impl<float>(label, v, min, max, fmt, display_scale);
}

bool slider(const char* label, int* v, int min, int max, const char* fmt)
{
	return slider_impl<int>(label, v, min, max, fmt, 1.0f);
}

bool range_slider(const char* label, float* lo, float* hi, float min, float max, const char* fmt, float display_scale)
{
	return range_impl<float>(label, lo, hi, min, max, fmt, display_scale);
}

bool range_slider(const char* label, int* lo, int* hi, int min, int max, const char* fmt)
{
	return range_impl<int>(label, lo, hi, min, max, fmt, 1.0f);
}

} // namespace nc
