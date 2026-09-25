#include "internal.h"

#include <cstdarg>

namespace nc
{

using namespace detail;

namespace
{
void text_v(ImU32 col, bool wrapped, const char* fmt, va_list args)
{
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, args);
	if (!search::filter(buf))
		return;
	ImGui::PushStyleColor(ImGuiCol_Text, styled(col));
	if (wrapped)
		ImGui::TextWrapped("%s", buf);
	else
		ImGui::TextUnformatted(buf);
	ImGui::PopStyleColor();
}
} // namespace

void text(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	text_v(theme().text, false, fmt, args);
	va_end(args);
}

void text_dim(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	text_v(theme().text_dim, false, fmt, args);
	va_end(args);
}

void text_colored(ImU32 col, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	text_v(col, false, fmt, args);
	va_end(args);
}

void text_wrapped(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	text_v(theme().text_label, true, fmt, args);
	va_end(args);
}

void label(const char* txt)
{
	if (!search::filter(txt))
		return;
	ImGui::PushStyleColor(ImGuiCol_Text, styled(theme().text_label));
	ImGui::TextUnformatted(txt, label_end(txt));
	ImGui::PopStyleColor();
}

void separator(const char* caption)
{
	if (search::dry_run() || search::active())
		return;
	const Theme& t = theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	float w = ImGui::GetContentRegionAvail().x;
	float h = caption ? ImGui::GetFontSize() : 1.0f;
	float cy = pos.y + h * 0.5f;

	if (caption && *caption)
	{
		ImVec2 sz = ImGui::CalcTextSize(caption);
		dl->AddText(ImVec2(pos.x, pos.y), styled(t.text_dim), caption);
		dl->AddLine(ImVec2(pos.x + sz.x + 8.0f, cy), ImVec2(pos.x + w, cy), styled(t.divider));
	}
	else
	{
		dl->AddLine(ImVec2(pos.x, cy), ImVec2(pos.x + w, cy), styled(t.divider));
	}
	ImGui::Dummy(ImVec2(w, h));
}

void help_marker(const char* txt)
{
	if (search::dry_run())
		return;
	ImGui::SameLine(0.0f, 4.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, styled(theme().text_dim));
	ImGui::TextUnformatted("(?)");
	ImGui::PopStyleColor();
	tooltip("%s", txt);
}

void tooltip(const char* fmt, ...)
{
	if (search::dry_run() || !ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
		return;

	char buf[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	const Theme& t = theme();
	push_popup_style(ImVec2(8.0f, 6.0f));
	ImGui::PushStyleColor(ImGuiCol_Text, t.text);
	if (ImGui::BeginTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 24.0f);
		ImGui::TextUnformatted(buf);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
	ImGui::PopStyleColor();
	pop_popup_style();
}

bool button(const char* label, const ImVec2& size_arg, ButtonStyle style)
{
	if (!search::filter(label))
		return false;
	const Theme& t = theme();
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##btn");

	const char* end = label_end(label);
	ImVec2 text_sz = ImGui::CalcTextSize(label, end);
	ImVec2 size = size_arg;
	if (size.x == 0.0f)
		size.x = text_sz.x + 24.0f;
	else if (size.x < 0.0f)
		size.x = std::max(4.0f, ImGui::GetContentRegionAvail().x + size.x + 1.0f);
	if (size.y <= 0.0f)
		size.y = t.control_height + 6.0f;

	ImVec2 pos = ImGui::GetCursorScreenPos();
	bool pressed = ImGui::InvisibleButton("##btn", size);
	ItemAnim a = item_anim(id);
	ImGui::PopID();

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 max(pos.x + size.x, pos.y + size.y);
	const float r = t.frame_rounding;

	ImU32 fill, border, text_col;
	switch (style)
	{
	case ButtonStyle::Primary:
		draw::glow(dl, pos, max, r, 0.35f + a.hover * 0.4f);
		fill = lerp_color(t.accent_col, t.hover, a.hover * 0.08f - a.active * 0.04f);
		border = t.accent_col;
		text_col = t.accent_text;
		break;
	case ButtonStyle::Danger:
		fill = lerp_color(t.field_bg, t.error, 0.12f + a.hover * 0.25f + a.active * 0.2f);
		border = lerp_color(t.field_border, t.error, 0.5f + a.hover * 0.5f);
		text_col = lerp_color(t.text, t.error, 0.5f);
		break;
	case ButtonStyle::Ghost:
		fill = with_alpha(t.accent_col, a.hover * 0.10f + a.active * 0.08f);
		border = with_alpha(t.accent_col, a.hover * 0.35f);
		text_col = lerp_color(t.text_label, t.text, a.hover);
		break;
	default:
		fill = lerp_color(t.field_bg, t.accent_col, a.hover * 0.10f + a.active * 0.12f);
		border = lerp_color(t.field_border, t.accent_col, a.hover * 0.6f);
		text_col = lerp_color(t.text_label, t.text, a.hover);
		break;
	}

	dl->AddRectFilled(pos, max, styled(fill), r);
	dl->AddRect(pos, max, styled(border), r);
	float press = a.active * 1.0f;
	dl->AddText(ImVec2(pos.x + (size.x - text_sz.x) * 0.5f, pos.y + (size.y - text_sz.y) * 0.5f + press * 0.5f),
							styled(text_col), label, end);
	return pressed;
}

bool icon_button(const char* id_str, const char* icon, float size, const char* tip)
{
	if (search::dry_run())
		return false;
	const Theme& t = theme();
	ImGui::PushID(id_str);
	ImGuiID id = ImGui::GetID("##ib");
	ImVec2 pos = ImGui::GetCursorScreenPos();
	bool pressed = ImGui::InvisibleButton("##ib", ImVec2(size, size));
	ItemAnim a = item_anim(id);
	if (tip)
		tooltip("%s", tip);
	ImGui::PopID();

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 max(pos.x + size, pos.y + size);
	float bg = a.hover * 0.06f + a.active * 0.05f;
	if (bg > 0.001f)
		dl->AddRectFilled(pos, max, styled(t.hover, bg), 4.0f);

	ImFont* f = fonts::icons();
	float fs = std::min(f->LegacySize, size - 6.0f);
	ImVec2 isz = draw::text_size(f, fs, icon);
	ImVec2 ip(pos.x + (size - isz.x) * 0.5f, pos.y + (size - isz.y) * 0.5f);
	dl->AddText(f, fs, ip, styled(lerp_color(t.text_dim, t.text, a.hover)), icon);
	return pressed;
}

bool checkbox(const char* label, bool* v)
{
	if (!search::filter(label))
		return false;
	const Theme& t = theme();
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##cb");

	const char* end = label_end(label);
	ImVec2 label_sz = ImGui::CalcTextSize(label, end);
	const float sz = t.checkbox_size;
	const float gap = label_sz.x > 0.0f ? 7.0f : 0.0f;
	const float h = std::max(sz + 2.0f, label_sz.y + 2.0f);

	ImVec2 pos = ImGui::GetCursorScreenPos();
	bool pressed = ImGui::InvisibleButton("##cb", ImVec2(sz + gap + label_sz.x, h));
	if (pressed)
	{
		*v = !*v;
		ImGui::MarkItemEdited(id);
	}
	bool hovered = ImGui::IsItemHovered();
	ImGui::PopID();

	float on = anim::animate(id, *v);
	float hov = anim::animate(anim::key(id, "hov"), hovered);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 bmin(pos.x, pos.y + (h - sz) * 0.5f);
	ImVec2 bmax(bmin.x + sz, bmin.y + sz);
	if (on > 0.001f)
		draw::glow(dl, bmin, bmax, 3.0f, on);
	dl->AddRectFilled(bmin, bmax, styled(lerp_color(t.control_off, t.accent_col, on)), 3.0f);
	dl->AddRect(bmin, bmax, styled(lerp_color(t.field_border, t.accent_col, std::max(on, hov * 0.5f))), 3.0f);
	draw::check_mark(dl, ImVec2(bmin.x + 2.0f, bmin.y + 2.0f), sz - 4.0f, styled(t.accent_text, on * 0.9f), on);

	if (end > label)
	{
		ImU32 tc = lerp_color(t.text_label, t.text, std::max(on, hov * 0.6f));
		dl->AddText(ImVec2(bmin.x + sz + gap, pos.y + (h - label_sz.y) * 0.5f), styled(tc), label, end);
	}
	return pressed;
}

bool toggle(const char* label, bool* v)
{
	if (!search::filter(label))
		return false;
	const Theme& t = theme();
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##tg");

	const char* end = label_end(label);
	const float track_w = 26.0f, track_h = 14.0f;
	const float w = std::max(ImGui::GetContentRegionAvail().x, track_w);
	const float h = std::max(track_h + 2.0f, ImGui::GetFontSize() + 2.0f);

	ImVec2 pos = ImGui::GetCursorScreenPos();
	bool pressed = ImGui::InvisibleButton("##tg", ImVec2(w, h));
	if (pressed)
	{
		*v = !*v;
		ImGui::MarkItemEdited(id);
	}
	bool hovered = ImGui::IsItemHovered();
	ImGui::PopID();

	float on = anim::animate(id, *v);
	float hov = anim::animate(anim::key(id, "hov"), hovered);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 tmin(pos.x + w - track_w, pos.y + (h - track_h) * 0.5f);
	ImVec2 tmax(tmin.x + track_w, tmin.y + track_h);
	const float r = track_h * 0.5f;
	if (on > 0.001f)
		draw::glow(dl, tmin, tmax, r, on * 0.8f);
	dl->AddRectFilled(tmin, tmax, styled(lerp_color(t.control_off, t.accent_col, on)), r);
	dl->AddRect(tmin, tmax, styled(lerp_color(t.field_border, t.accent_col, std::max(on, hov * 0.5f))), r);

	float knob_r = r - 3.0f;
	float kx = lerp(tmin.x + r, tmax.x - r, anim::ease_out_cubic(on));
	ImU32 knob = t.dark ? lerp_color(t.text_dim, IM_COL32(245, 245, 248, 255), on) : IM_COL32(255, 255, 255, 255);
	dl->AddCircleFilled(ImVec2(kx, tmin.y + r), knob_r + hov * 0.6f, styled(knob), 16);

	if (end > label)
	{
		ImVec2 lsz = ImGui::CalcTextSize(label, end);
		ImU32 tc = lerp_color(t.text_label, t.text, std::max(on, hov * 0.6f));
		dl->AddText(ImVec2(pos.x, pos.y + (h - lsz.y) * 0.5f), styled(tc), label, end);
	}
	return pressed;
}

bool radio(const char* label, int* v, int value)
{
	if (!search::filter(label))
		return false;
	const Theme& t = theme();
	ImGui::PushID(label);
	ImGui::PushID(value);
	ImGuiID id = ImGui::GetID("##rd");

	const char* end = label_end(label);
	ImVec2 label_sz = ImGui::CalcTextSize(label, end);
	const float d = t.checkbox_size + 1.0f;
	const float h = std::max(d + 2.0f, label_sz.y + 2.0f);
	ImVec2 pos = ImGui::GetCursorScreenPos();
	bool pressed = ImGui::InvisibleButton("##rd", ImVec2(d + 7.0f + label_sz.x, h));
	if (pressed && *v != value)
	{
		*v = value;
		ImGui::MarkItemEdited(id);
	}
	else
		pressed = false;
	bool hovered = ImGui::IsItemHovered();
	ImGui::PopID();
	ImGui::PopID();

	float on = anim::animate(id, *v == value);
	float hov = anim::animate(anim::key(id, "hov"), hovered);
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 c(pos.x + d * 0.5f, pos.y + h * 0.5f);
	dl->AddCircleFilled(c, d * 0.5f, styled(t.control_off), 20);
	dl->AddCircle(c, d * 0.5f, styled(lerp_color(t.field_border, t.accent_col, std::max(on, hov * 0.5f))), 20);
	if (on > 0.001f)
		dl->AddCircleFilled(c, d * 0.25f * on + 0.5f, styled(t.accent_col, on), 16);
	ImU32 tc = lerp_color(t.text_label, t.text, std::max(on, hov * 0.6f));
	dl->AddText(ImVec2(pos.x + d + 7.0f, pos.y + (h - label_sz.y) * 0.5f), styled(tc), label, end);
	return pressed;
}

void progress_bar(const char* label, float fraction, const char* overlay)
{
	if (!search::filter(label))
		return;
	const Theme& t = theme();
	char pct[16];
	if (!overlay)
	{
		snprintf(pct, sizeof(pct), "%.0f%%", saturate(fraction) * 100.0f);
		overlay = pct;
	}
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##pb");
	float w = stacked_label(label, overlay);
	const float h = 6.0f;
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::Dummy(ImVec2(w, h));
	ImGui::PopID();

	float vis = anim::animate(id, saturate(fraction), 8.0f);
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 max(pos.x + w, pos.y + h);
	dl->AddRectFilled(pos, max, styled(t.field_bg), h * 0.5f);
	dl->AddRect(pos, max, styled(t.field_border), h * 0.5f);
	if (vis > 0.002f)
	{
		ImVec2 fmax(pos.x + std::max(h, w * vis), max.y);
		draw::glow(dl, pos, fmax, h * 0.5f, 0.6f);
		dl->AddRectFilled(pos, fmax, styled(t.accent_col), h * 0.5f);
	}
}

void spinner(const char* id_str, float radius, float thickness)
{
	if (search::dry_run())
		return;
	ImGui::PushID(id_str);
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::Dummy(ImVec2(radius * 2.0f, radius * 2.0f));
	ImGui::PopID();

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 c(pos.x + radius, pos.y + radius);
	float time = (float)ImGui::GetTime();
	float start = time * 6.0f;
	float sweep = IM_PI * (0.6f + 0.5f * std::sin(time * 2.4f));
	dl->AddCircle(c, radius - thickness * 0.5f, styled(theme().field_border), 32, thickness);
	dl->PathArcTo(c, radius - thickness * 0.5f, start, start + sweep, 24);
	dl->PathStroke(styled(theme().accent_col), 0, thickness);
}

void badge(const char* txt, ImU32 color)
{
	if (search::dry_run())
		return;
	const Theme& t = theme();
	ImVec2 sz = ImGui::CalcTextSize(txt);
	ImVec2 pos = ImGui::GetCursorScreenPos();
	float h = sz.y + 2.0f, w = sz.x + 12.0f;
	ImGui::Dummy(ImVec2(w, h));
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImU32 bg = color ? color : t.badge_bg;
	dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), styled(bg), h * 0.5f);
	ImU32 tc = color ? (luminance(color) > 0.6f ? IM_COL32(16, 16, 20, 255) : IM_COL32(250, 250, 252, 255)) : t.text_label;
	dl->AddText(ImVec2(pos.x + 6.0f, pos.y + 1.0f), styled(tc), txt);
}

} // namespace nc
