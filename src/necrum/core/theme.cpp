#include "theme.h"

namespace nc
{

namespace
{
Theme& active_theme()
{
	static Theme t = themes::necrum();
	return t;
}

ImVec4 f4(ImU32 c, float alpha_mul = 1.0f)
{
	ImVec4 v = ImGui::ColorConvertU32ToFloat4(c);
	v.w *= alpha_mul;
	return v;
}
} // namespace

void Theme::refresh()
{
	accent_col = accent.u32();
	accent_dim = hsv(accent.h, accent.s, accent.v * 0.8f, 0.78f * accent.a);
	accent_text = luminance(accent_col) > 0.6f ? IM_COL32(16, 16, 20, 255) : IM_COL32(250, 250, 252, 255);
}

Theme& theme()
{
	return active_theme();
}

void set_theme(const Theme& t)
{
	active_theme() = t;
	active_theme().refresh();
	if (ImGui::GetCurrentContext())
		apply_imgui_style(active_theme());
}

void apply_imgui_style(const Theme& t)
{
	ImGuiStyle& s = ImGui::GetStyle();

	s.WindowPadding = {0.0f, 0.0f};
	s.FramePadding = {8.0f, 5.0f};
	s.ItemSpacing = {10.0f, 10.0f};
	s.ItemInnerSpacing = {6.0f, 6.0f};
	s.ScrollbarSize = 10.0f;
	s.GrabMinSize = 8.0f;
	s.WindowBorderSize = 0.0f;
	s.FrameBorderSize = 0.0f;
	s.PopupBorderSize = 1.0f;
	s.TabBorderSize = 0.0f;
	s.ChildBorderSize = 0.0f;

	s.WindowRounding = t.window_rounding;
	s.FrameRounding = t.frame_rounding;
	s.PopupRounding = t.popup_rounding;
	s.ScrollbarRounding = 3.0f;
	s.GrabRounding = 2.0f;
	s.TabRounding = 3.0f;
	s.ChildRounding = 0.0f;
	s.WindowTitleAlign = {0.5f, 0.5f};

	const ImU32 accent = t.accent.u32();
	ImVec4* c = s.Colors;

	c[ImGuiCol_WindowBg] = f4(t.window_bg);
	c[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
	c[ImGuiCol_PopupBg] = f4(t.popup_bg, 0.98f);
	c[ImGuiCol_MenuBarBg] = f4(t.header_bg);
	c[ImGuiCol_Border] = f4(t.popup_border);
	c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

	c[ImGuiCol_Text] = f4(t.text);
	c[ImGuiCol_TextDisabled] = f4(t.text_disabled);

	c[ImGuiCol_FrameBg] = f4(t.field_bg);
	c[ImGuiCol_FrameBgHovered] = f4(lerp_color(t.field_bg, accent, 0.12f));
	c[ImGuiCol_FrameBgActive] = f4(lerp_color(t.field_bg, accent, 0.25f));

	c[ImGuiCol_TitleBg] = f4(t.header_bg);
	c[ImGuiCol_TitleBgActive] = f4(t.header_bg);
	c[ImGuiCol_TitleBgCollapsed] = f4(t.header_bg, 0.7f);

	c[ImGuiCol_Button] = f4(t.field_bg);
	c[ImGuiCol_ButtonHovered] = f4(accent, 0.35f);
	c[ImGuiCol_ButtonActive] = f4(accent, 0.5f);

	c[ImGuiCol_Header] = f4(t.field_bg, 0.5f);
	c[ImGuiCol_HeaderHovered] = f4(accent, 0.25f);
	c[ImGuiCol_HeaderActive] = f4(accent, 0.4f);

	c[ImGuiCol_Tab] = f4(t.header_bg, 0.9f);
	c[ImGuiCol_TabHovered] = f4(accent, 0.3f);
	c[ImGuiCol_TabSelected] = f4(accent, 0.5f);
	c[ImGuiCol_TabDimmed] = f4(t.header_bg, 0.7f);
	c[ImGuiCol_TabDimmedSelected] = f4(accent, 0.3f);

	c[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
	c[ImGuiCol_ScrollbarGrab] = f4(t.field_border);
	c[ImGuiCol_ScrollbarGrabHovered] = f4(lerp_color(t.field_border, accent, 0.4f));
	c[ImGuiCol_ScrollbarGrabActive] = f4(accent);

	c[ImGuiCol_SliderGrab] = f4(accent, 0.82f);
	c[ImGuiCol_SliderGrabActive] = f4(accent);
	c[ImGuiCol_CheckMark] = f4(accent);

	c[ImGuiCol_Separator] = f4(t.divider);
	c[ImGuiCol_SeparatorHovered] = f4(accent, 0.3f);
	c[ImGuiCol_SeparatorActive] = f4(accent, 0.5f);

	c[ImGuiCol_ResizeGrip] = f4(accent, 0.1f);
	c[ImGuiCol_ResizeGripHovered] = f4(accent, 0.3f);
	c[ImGuiCol_ResizeGripActive] = f4(accent, 0.5f);

	c[ImGuiCol_NavCursor] = f4(accent);
	c[ImGuiCol_TextSelectedBg] = f4(accent, 0.25f);
}

namespace themes
{

Theme necrum()
{
	Theme t;
	t.name = "Necrum";
	t.dark = true;
	t.accent = {265.91f, 0.69f, 1.0f, 1.0f};

	t.window_bg = IM_COL32(8, 8, 10, 255);
	t.window_border = IM_COL32(40, 38, 52, 50);
	t.header_bg = IM_COL32(14, 14, 14, 255);
	t.sidebar_bg = IM_COL32(10, 10, 10, 255);
	t.content_bg = IM_COL32(12, 12, 12, 255);
	t.footer_bg = IM_COL32(7, 7, 7, 255);
	t.panel_bg = IM_COL32(9, 9, 9, 255);
	t.panel_header_bg = IM_COL32(14, 14, 15, 255);
	t.panel_border = IM_COL32(45, 45, 48, 140);
	t.popup_bg = IM_COL32(10, 10, 10, 255);
	t.popup_border = IM_COL32(36, 36, 36, 200);
	t.field_bg = IM_COL32(12, 12, 13, 255);
	t.field_border = IM_COL32(36, 36, 36, 180);
	t.control_off = IM_COL32(6, 6, 6, 255);
	t.divider = IM_COL32(26, 26, 26, 255);
	t.hover = IM_COL32(255, 255, 255, 255);
	t.shadow = IM_COL32(0, 0, 0, 200);
	t.badge_bg = IM_COL32(35, 35, 38, 240);

	t.text = IM_COL32(188, 188, 188, 255);
	t.text_label = IM_COL32(130, 130, 130, 255);
	t.text_dim = IM_COL32(88, 88, 88, 255);
	t.text_disabled = IM_COL32(60, 60, 60, 255);

	t.refresh();
	return t;
}

Theme crimson()
{
	Theme t = necrum();
	t.name = "Crimson";
	t.accent = {0.0f, 0.675f, 0.784f, 1.0f};
	t.window_border = IM_COL32(52, 38, 38, 50);
	t.refresh();
	return t;
}

Theme graphite()
{
	Theme t = necrum();
	t.name = "Graphite";
	t.accent = {212.0f, 0.62f, 0.95f, 1.0f};

	t.window_bg = IM_COL32(22, 23, 26, 255);
	t.window_border = IM_COL32(58, 60, 68, 90);
	t.header_bg = IM_COL32(28, 29, 33, 255);
	t.sidebar_bg = IM_COL32(24, 25, 28, 255);
	t.content_bg = IM_COL32(26, 27, 31, 255);
	t.footer_bg = IM_COL32(20, 21, 24, 255);
	t.panel_bg = IM_COL32(22, 23, 26, 255);
	t.panel_header_bg = IM_COL32(30, 31, 36, 255);
	t.panel_border = IM_COL32(60, 62, 70, 150);
	t.popup_bg = IM_COL32(24, 25, 29, 255);
	t.popup_border = IM_COL32(60, 62, 70, 200);
	t.field_bg = IM_COL32(30, 31, 35, 255);
	t.field_border = IM_COL32(56, 58, 66, 200);
	t.control_off = IM_COL32(18, 19, 22, 255);
	t.divider = IM_COL32(44, 45, 52, 255);
	t.badge_bg = IM_COL32(48, 50, 58, 240);

	t.text = IM_COL32(214, 216, 222, 255);
	t.text_label = IM_COL32(150, 153, 162, 255);
	t.text_dim = IM_COL32(108, 111, 120, 255);
	t.text_disabled = IM_COL32(76, 78, 86, 255);

	t.refresh();
	return t;
}

Theme daylight()
{
	Theme t;
	t.name = "Daylight";
	t.dark = false;
	t.accent = {221.0f, 0.72f, 0.88f, 1.0f};
	t.glow = 0.35f;

	t.window_bg = IM_COL32(244, 245, 248, 255);
	t.window_border = IM_COL32(200, 203, 212, 200);
	t.header_bg = IM_COL32(252, 252, 254, 255);
	t.sidebar_bg = IM_COL32(240, 241, 245, 255);
	t.content_bg = IM_COL32(246, 247, 250, 255);
	t.footer_bg = IM_COL32(236, 238, 242, 255);
	t.panel_bg = IM_COL32(255, 255, 255, 255);
	t.panel_header_bg = IM_COL32(249, 250, 252, 255);
	t.panel_border = IM_COL32(214, 217, 225, 255);
	t.popup_bg = IM_COL32(255, 255, 255, 255);
	t.popup_border = IM_COL32(205, 208, 216, 255);
	t.field_bg = IM_COL32(238, 240, 244, 255);
	t.field_border = IM_COL32(206, 210, 218, 255);
	t.control_off = IM_COL32(228, 231, 236, 255);
	t.divider = IM_COL32(222, 225, 232, 255);
	t.hover = IM_COL32(0, 0, 0, 255);
	t.shadow = IM_COL32(40, 44, 60, 60);
	t.badge_bg = IM_COL32(228, 231, 238, 255);

	t.text = IM_COL32(34, 36, 44, 255);
	t.text_label = IM_COL32(88, 92, 104, 255);
	t.text_dim = IM_COL32(136, 140, 152, 255);
	t.text_disabled = IM_COL32(178, 182, 192, 255);

	t.refresh();
	return t;
}

const std::vector<Preset>& presets()
{
	static const std::vector<Preset> list = {
			{"Necrum", &necrum},
			{"Crimson", &crimson},
			{"Graphite", &graphite},
			{"Daylight", &daylight},
	};
	return list;
}

} // namespace themes

} // namespace nc
