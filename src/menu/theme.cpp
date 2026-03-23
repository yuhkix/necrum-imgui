#include "theme.h"
#include "core/fonts/FontAwesome.h"
#include "core/fonts/Icons.h"
#include "pch.h"
#include <imgui.h>
#include <string>
#include <windows.h>

namespace theme
{

// Helpers
static ImVec4 hex(uint32_t col, float a = 1.0f)
{
	return ImVec4(((col >> 16) & 0xFF) / 255.0f, ((col >> 8) & 0xFF) / 255.0f, ((col >> 0) & 0xFF) / 255.0f, a);
}

// Font Loading
void LoadFonts(ImGuiIO& io)
{
	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.GlyphMinAdvanceX = 13.0f;
	// Necessary so ImGui doesn't try to free the static array.
	icons_config.FontDataOwnedByAtlas = false;

	static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};

	font_regular = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 14.0f);
	io.Fonts->AddFontFromMemoryTTF((void*)font_awesome_bin, sizeof(font_awesome_bin), 13.0f, &icons_config, icons_ranges);

	font_bold = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisb.ttf", 14.0f);
	io.Fonts->AddFontFromMemoryTTF((void*)font_awesome_bin, sizeof(font_awesome_bin), 13.0f, &icons_config, icons_ranges);

	font_logo = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisb.ttf", 22.0f);

	icons_config.MergeMode = false;
	font_icons = io.Fonts->AddFontFromMemoryTTF((void*)font_awesome_bin, sizeof(font_awesome_bin), 14.0f, &icons_config,
																							icons_ranges);

	if (!font_regular)
		font_regular = io.Fonts->AddFontDefault();
	if (!font_bold)
		font_bold = font_regular;
	if (!font_logo)
		font_logo = font_regular;
	if (!font_icons)
		font_icons = font_regular;

	io.Fonts->Build();
}

// Theme
void Apply()
{
	ImGuiStyle& s = ImGui::GetStyle();

	// Geometry
	s.WindowPadding = {0.0f, 0.0f};
	s.FramePadding = {6.0f, 3.0f};
	s.ItemSpacing = {8.0f, 3.5f};
	s.ItemInnerSpacing = {4.0f, 4.0f};
	s.ScrollbarSize = 10.0f;
	s.GrabMinSize = 8.0f;
	s.WindowBorderSize = 0.0f;
	s.FrameBorderSize = 0.0f;
	s.PopupBorderSize = 1.0f;
	s.TabBorderSize = 0.0f;
	s.ChildBorderSize = 0.0f;

	// Rounding
	s.WindowRounding = 4.0f;
	s.FrameRounding = 3.0f;
	s.PopupRounding = 4.0f;
	s.ScrollbarRounding = 3.0f;
	s.GrabRounding = 2.0f;
	s.TabRounding = 3.0f;
	s.ChildRounding = 0.0f;

	s.WindowTitleAlign = {0.5f, 0.5f};

	// Dark palette with red accent
	constexpr uint32_t bg_deep = 0x060606;
	constexpr uint32_t bg_window = 0x080808;
	constexpr uint32_t bg_header = 0x0c0c0c;
	constexpr uint32_t bg_widget = 0x161616;
	constexpr uint32_t bg_hover = 0x1e1e1e;
	constexpr uint32_t accent = 0xc84141;
	constexpr uint32_t accent_lit = 0xe06060;
	constexpr uint32_t accent_dim = 0xa03232;
	constexpr uint32_t text_white = 0xe8e8e8;
	constexpr uint32_t text_dim = 0x6e6e6e;
	constexpr uint32_t border_col = 0x1a1a1a;

	ImVec4* c = s.Colors;

	c[ImGuiCol_WindowBg] = hex(bg_window);
	c[ImGuiCol_ChildBg] = hex(bg_window, 0.0f);
	c[ImGuiCol_PopupBg] = hex(bg_header, 0.96f);
	c[ImGuiCol_MenuBarBg] = hex(bg_header);

	c[ImGuiCol_Border] = hex(border_col, 0.5f);
	c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

	c[ImGuiCol_Text] = hex(text_white);
	c[ImGuiCol_TextDisabled] = hex(text_dim);

	c[ImGuiCol_FrameBg] = hex(bg_widget, 0.6f);
	c[ImGuiCol_FrameBgHovered] = hex(bg_hover, 0.7f);
	c[ImGuiCol_FrameBgActive] = hex(accent_dim, 0.3f);

	c[ImGuiCol_TitleBg] = hex(bg_deep);
	c[ImGuiCol_TitleBgActive] = hex(bg_header);
	c[ImGuiCol_TitleBgCollapsed] = hex(bg_deep, 0.7f);

	c[ImGuiCol_Button] = hex(bg_widget, 0.7f);
	c[ImGuiCol_ButtonHovered] = hex(accent, 0.35f);
	c[ImGuiCol_ButtonActive] = hex(accent_dim, 0.5f);

	c[ImGuiCol_Header] = hex(bg_widget, 0.5f);
	c[ImGuiCol_HeaderHovered] = hex(accent, 0.25f);
	c[ImGuiCol_HeaderActive] = hex(accent, 0.4f);

	c[ImGuiCol_Tab] = hex(bg_header, 0.9f);
	c[ImGuiCol_TabHovered] = hex(accent, 0.3f);
	c[ImGuiCol_TabSelected] = hex(accent, 0.5f);
	c[ImGuiCol_TabDimmed] = hex(bg_header, 0.7f);
	c[ImGuiCol_TabDimmedSelected] = hex(accent_dim, 0.3f);

	c[ImGuiCol_ScrollbarBg] = hex(bg_deep, 0.5f);
	c[ImGuiCol_ScrollbarGrab] = hex(bg_widget);
	c[ImGuiCol_ScrollbarGrabHovered] = hex(bg_hover);
	c[ImGuiCol_ScrollbarGrabActive] = hex(accent_dim);

	c[ImGuiCol_SliderGrab] = hex(accent, 0.82f);
	c[ImGuiCol_SliderGrabActive] = hex(accent_lit);
	c[ImGuiCol_CheckMark] = hex(accent_lit);

	c[ImGuiCol_Separator] = hex(border_col, 0.4f);
	c[ImGuiCol_SeparatorHovered] = hex(accent, 0.3f);
	c[ImGuiCol_SeparatorActive] = hex(accent_lit, 0.5f);

	c[ImGuiCol_ResizeGrip] = hex(accent, 0.1f);
	c[ImGuiCol_ResizeGripHovered] = hex(accent, 0.3f);
	c[ImGuiCol_ResizeGripActive] = hex(accent_lit, 0.5f);

	c[ImGuiCol_NavHighlight] = hex(accent);
	c[ImGuiCol_TextSelectedBg] = hex(accent, 0.2f);
}

} // namespace theme
