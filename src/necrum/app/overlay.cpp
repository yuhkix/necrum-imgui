#include "overlay.h"

#include "necrum/core/anim.h"
#include "necrum/core/draw.h"
#include "necrum/core/input.h"
#include "necrum/core/search.h"
#include "necrum/core/theme.h"

namespace nc
{

namespace
{
constexpr float k_header = 32.0f;
constexpr float k_row = 24.0f;
bool g_editable = false;
int g_open = 0;
} // namespace

void set_overlays_editable(bool editable)
{
	g_editable = editable;
}

bool overlays_editable()
{
	return g_editable;
}

bool begin_overlay(const char* title, bool visible, const OverlayOptions& o)
{
	const Theme& t = theme();
	ImGuiID id = ImHashStr(title);
	float alpha = anim::animate(anim::key(id, "overlay"), visible);
	if (alpha <= 0.001f)
		return false;

	if (o.position.x >= 0.0f && o.position.y >= 0.0f)
		ImGui::SetNextWindowPos(o.position, ImGuiCond_FirstUseEver, o.pivot);
	else
		ImGui::SetNextWindowPos(anchor_position(o.anchor, ImVec2(o.width, 120.0f), o.margin), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(o.width, 0.0f), ImVec2(o.width, FLT_MAX));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize |
													 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
													 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground |
													 ImGuiWindowFlags_NoCollapse;
	if (!g_editable || !o.draggable)
		flags |= ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 10.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);

	char window_id[160];
	snprintf(window_id, sizeof(window_id), "##overlay_%s", title);
	ImGui::Begin(window_id, nullptr, flags);
	g_open++;

	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetWindowPos();
	ImVec2 size = ImGui::GetWindowSize();
	ImVec2 max(pos.x + size.x, pos.y + size.y);
	draw::glow(dl, pos, max, 8.0f, 0.45f, t.shadow);
	draw::glow(dl, ImVec2(pos.x + 2.0f, pos.y + 2.0f), ImVec2(max.x - 2.0f, max.y - 2.0f), 7.0f, 0.20f);
	draw::card(dl, pos, max, title, 8.0f, k_header);

	if (o.badge && *o.badge)
	{
		ImVec2 bs = ImGui::CalcTextSize(o.badge);
		float bh = 16.0f, bw = bs.x + 12.0f;
		ImVec2 bmin(max.x - bw - 12.0f, pos.y + (k_header - bh) * 0.5f);
		ImU32 bg = o.badge_color ? o.badge_color : t.badge_bg;
		ImU32 fg = o.badge_color ? (luminance(bg) > 0.6f ? IM_COL32(16, 16, 20, 255) : IM_COL32(250, 250, 252, 255))
														 : t.text_dim;
		dl->AddRectFilled(bmin, ImVec2(bmin.x + bw, bmin.y + bh), styled(bg), bh * 0.5f);
		dl->AddText(ImVec2(bmin.x + 6.0f, bmin.y + (bh - bs.y) * 0.5f), styled(fg), o.badge);
	}

	// Content starts below the header. Overlays are HUD, never filtered by search.
	ImGui::SetCursorPosY(k_header + 10.0f);
	ImGui::Dummy(ImVec2(o.width - 28.0f, 0.0f));
	ImGui::PushItemWidth(-FLT_MIN);
	search::push_suspend();
	return true;
}

void end_overlay()
{
	IM_ASSERT(g_open > 0 && "end_overlay() without a visible begin_overlay()");
	g_open--;
	search::pop_suspend();
	ImGui::PopItemWidth();
	ImGui::End();
	ImGui::PopStyleVar(3);
}

void overlay_row(const char* label, const char* value, bool active)
{
	const Theme& t = theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 pos = ImGui::GetCursorScreenPos();
	float w = ImGui::GetContentRegionAvail().x;
	ImGui::Dummy(ImVec2(w, k_row));

	float cy = pos.y + k_row * 0.5f;
	ImVec2 dot(pos.x + 3.0f, cy);
	if (active)
	{
		float pulse = std::sin((float)ImGui::GetTime() * 6.0f) * 0.5f + 0.5f;
		dl->AddCircle(dot, 3.2f + pulse * 2.0f, styled(t.accent_col, 0.4f * (1.0f - pulse)), 20, 1.0f);
		dl->AddCircleFilled(dot, 3.0f, styled(t.accent_col), 16);
	}
	else
		dl->AddCircleFilled(dot, 3.0f, styled(t.text_dim, 0.5f), 16);

	float fh = ImGui::GetFontSize();
	dl->AddText(ImVec2(dot.x + 12.0f, cy - fh * 0.5f), styled(active ? t.text : t.text_dim), label);
	if (value && *value)
	{
		ImVec2 vs = ImGui::CalcTextSize(value);
		dl->AddText(ImVec2(pos.x + w - vs.x, cy - fh * 0.5f), styled(active ? t.text_label : t.text_dim), value);
	}
}

void overlay_text(const char* txt)
{
	ImGui::PushStyleColor(ImGuiCol_Text, styled(theme().text_dim));
	ImGui::TextUnformatted(txt);
	ImGui::PopStyleColor();
}

void keybind_overlay(bool visible, const OverlayOptions& options)
{
	int active_count = 0;
	for (const auto& tb : keybinds::tracked())
		if (tb.bind->bound() && tb.bind->active())
			active_count++;

	char badge[32];
	snprintf(badge, sizeof(badge), "%d active", active_count);
	OverlayOptions o = options;
	if (!o.badge)
		o.badge = badge;

	if (!begin_overlay("Keybinds", visible, o))
		return;

	bool any = false;
	for (const auto& tb : keybinds::tracked())
	{
		const Keybind& b = *tb.bind;
		if (!b.bound() || b.mode == KeybindMode::Off)
			continue;
		any = true;
		char value[64];
		snprintf(value, sizeof(value), "%s  %s", b.mode == KeybindMode::Always ? "" : key_name(b.key),
						 keybind_mode_name(b.mode));
		overlay_row(tb.name.c_str(), value, b.active());
	}
	if (!any)
		overlay_text("No active binds");
	end_overlay();
}

} // namespace nc
