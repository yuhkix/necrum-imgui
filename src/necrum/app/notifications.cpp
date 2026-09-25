#include "notifications.h"

#include "necrum/core/draw.h"
#include "necrum/core/fonts.h"
#include "necrum/core/theme.h"
#include "necrum/fonts/icons_fa.h"

#include <cstdarg>

namespace nc
{

namespace
{
struct Toast
{
	std::string message;
	Notify type;
	float duration;
	float remaining;
	float anim = 0.0f;
};

std::vector<Toast>& toasts()
{
	static std::vector<Toast> list;
	return list;
}

void push(Notify type, float seconds, const char* fmt, va_list args)
{
	char buf[512];
	vsnprintf(buf, sizeof(buf), fmt, args);
	toasts().push_back({buf, type, seconds, seconds});
}

void type_style(Notify type, ImU32* col, const char** icon)
{
	const Theme& t = theme();
	switch (type)
	{
	case Notify::Success:
		*col = t.success;
		*icon = ICON_FA_CIRCLE_CHECK;
		break;
	case Notify::Warning:
		*col = t.warning;
		*icon = ICON_FA_CIRCLE_EXCLAMATION;
		break;
	case Notify::Error:
		*col = t.error;
		*icon = ICON_FA_CIRCLE_XMARK;
		break;
	default:
		*col = t.info;
		*icon = ICON_FA_CIRCLE_INFO;
		break;
	}
}
} // namespace

void notify(Notify type, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	push(type, 4.0f, fmt, args);
	va_end(args);
}

void notify_for(Notify type, float seconds, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	push(type, seconds, fmt, args);
	va_end(args);
}

NotifyStyle& notify_style()
{
	static NotifyStyle style;
	return style;
}

void clear_notifications()
{
	toasts().clear();
}

// Called from nc::end_frame().
void render_notifications()
{
	auto& list = toasts();
	if (list.empty())
		return;

	const Theme& t = theme();
	const NotifyStyle& st = notify_style();
	ImDrawList* dl = ImGui::GetForegroundDrawList();
	const float dt = ImGui::GetIO().DeltaTime;
	const bool bottom = is_bottom(st.corner);
	const bool right = st.corner == Corner::TopRight || st.corner == Corner::BottomRight;

	ImVec2 base = anchor_position(st.corner, ImVec2(st.width, st.height), st.margin);
	float offset = 0.0f;
	int shown = 0;

	for (int i = (int)list.size() - 1; i >= 0; --i)
	{
		Toast& n = list[i];
		n.remaining -= dt;
		bool alive = n.remaining > 0.0f && shown < st.max_visible;
		n.anim += ((alive ? 1.0f : 0.0f) - n.anim) * std::min(1.0f, 10.0f * dt);
		if (!alive && n.anim < 0.01f && n.remaining <= 0.0f)
		{
			list.erase(list.begin() + i);
			continue;
		}
		if (alive)
			shown++;

		const float a = saturate(n.anim);
		float slide = (1.0f - a) * (st.width * 1.5f + st.margin) * (right ? 1.0f : -1.0f);
		if (st.corner == Corner::TopCenter || st.corner == Corner::BottomCenter)
			slide = 0.0f;
		// Grow to fit long messages, keeping the anchored edge fixed.
		const float width = std::max(st.width, ImGui::CalcTextSize(n.message.c_str()).x + 52.0f);
		float x = base.x + slide;
		if (right)
			x -= width - st.width;
		else if (st.corner == Corner::TopCenter || st.corner == Corner::BottomCenter)
			x -= (width - st.width) * 0.5f;
		ImVec2 min(x, base.y + (bottom ? -offset : offset));
		ImVec2 max(min.x + width, min.y + st.height);

		ImU32 accent;
		const char* icon;
		type_style(n.type, &accent, &icon);

		dl->AddRectFilled(min, max, alpha_mul(t.popup_bg, 0.95f * a), 5.0f, ImDrawFlags_RoundCornersTop);
		dl->AddRect(min, max, alpha_mul(t.popup_border, 0.8f * a), 5.0f, ImDrawFlags_RoundCornersTop);
		ImU32 glow = alpha_mul(accent, 0.14f * a), clear = alpha_mul(accent, 0.0f);
		dl->AddRectFilledMultiColor(ImVec2(min.x + 1.0f, max.y - 10.0f), ImVec2(max.x - 1.0f, max.y - 1.0f), clear, clear,
																glow, glow);
		dl->AddLine(ImVec2(min.x, max.y - 1.0f), ImVec2(max.x, max.y - 1.0f), alpha_mul(accent, 0.9f * a), 1.0f);

		// Remaining-time bar
		float life = saturate(n.remaining / std::max(0.01f, n.duration));
		dl->AddLine(ImVec2(min.x, max.y - 1.0f), ImVec2(min.x + width * life, max.y - 1.0f), alpha_mul(accent, a), 2.0f);

		ImFont* font = ImGui::GetFont();
		float fs = ImGui::GetFontSize();
		float ty = min.y + (st.height - fs) * 0.5f;
		dl->AddText(fonts::icons(), fonts::icons()->LegacySize, ImVec2(min.x + 12.0f, ty), alpha_mul(accent, 0.92f * a),
								icon);
		dl->PushClipRect(min, ImVec2(max.x - 8.0f, max.y), true);
		dl->AddText(font, fs, ImVec2(min.x + 36.0f, ty), alpha_mul(t.text, a), n.message.c_str());
		dl->PopClipRect();

		offset += (st.height + st.spacing) * a;
	}
}

} // namespace nc
