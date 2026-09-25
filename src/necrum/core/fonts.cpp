#include "fonts.h"

#include "necrum/fonts/fa_solid_900.h"
#include "necrum/fonts/icons_fa.h"

namespace nc::fonts
{

namespace
{
ImFont* g_regular = nullptr;
ImFont* g_bold = nullptr;
ImFont* g_title = nullptr;
ImFont* g_icons = nullptr;

const ImWchar k_icon_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};

bool file_exists(const std::string& path)
{
	if (path.empty())
		return false;
	if (FILE* f = fopen(path.c_str(), "rb"))
	{
		fclose(f);
		return true;
	}
	return false;
}

ImFont* add_text_font(ImGuiIO& io, const std::string& path, float size)
{
	if (!file_exists(path))
		return nullptr;
	ImFontConfig cfg;
	cfg.Flags |= ImFontFlags_NoLoadError;
	return io.Fonts->AddFontFromFileTTF(path.c_str(), size, &cfg);
}

void merge_icons(ImGuiIO& io, float size)
{
	ImFontConfig cfg;
	cfg.MergeMode = true;
	cfg.PixelSnapH = true;
	cfg.GlyphMinAdvanceX = size;
	cfg.FontDataOwnedByAtlas = false;
	io.Fonts->AddFontFromMemoryTTF((void*)font_awesome_bin, (int)sizeof(font_awesome_bin), size, &cfg, k_icon_ranges);
}

std::string first_existing(std::initializer_list<const char*> candidates)
{
	for (const char* c : candidates)
		if (file_exists(c))
			return c;
	return {};
}
} // namespace

Config default_config()
{
	Config cfg;
#ifdef _WIN32
	cfg.regular_path = "C:\\Windows\\Fonts\\segoeui.ttf";
	cfg.bold_path = "C:\\Windows\\Fonts\\seguisb.ttf";
#else
	cfg.regular_path = first_existing({"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
																		 "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf"});
	cfg.bold_path = first_existing({"/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
																	"/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf"});
#endif
	return cfg;
}

void load(ImGuiIO& io, const Config& cfg)
{
	g_regular = add_text_font(io, cfg.regular_path, cfg.size);
	if (!g_regular)
	{
		ImFontConfig def;
		def.SizePixels = cfg.size;
		g_regular = io.Fonts->AddFontDefault(&def);
	}
	if (cfg.merge_icons)
		merge_icons(io, cfg.merged_icon_size);

	g_bold = add_text_font(io, cfg.bold_path, cfg.size);
	if (g_bold && cfg.merge_icons)
		merge_icons(io, cfg.merged_icon_size);

	const std::string& title_path = cfg.title_path.empty() ? cfg.bold_path : cfg.title_path;
	g_title = add_text_font(io, title_path, cfg.title_size);

	ImFontConfig icon_cfg;
	icon_cfg.PixelSnapH = true;
	icon_cfg.GlyphMinAdvanceX = cfg.icon_size;
	icon_cfg.FontDataOwnedByAtlas = false;
	g_icons = io.Fonts->AddFontFromMemoryTTF((void*)font_awesome_bin, (int)sizeof(font_awesome_bin), cfg.icon_size,
																					 &icon_cfg, k_icon_ranges);

	if (!g_bold)
		g_bold = g_regular;
	if (!g_title)
		g_title = g_bold;
	if (!g_icons)
		g_icons = g_regular;

	io.FontDefault = g_regular;
}

ImFont* regular()
{
	return g_regular ? g_regular : ImGui::GetFont();
}

ImFont* bold()
{
	return g_bold ? g_bold : regular();
}

ImFont* title()
{
	return g_title ? g_title : bold();
}

ImFont* icons()
{
	return g_icons ? g_icons : regular();
}

void push(ImFont* font, float size)
{
	if (size <= 0.0f)
		size = font ? font->LegacySize : 0.0f;
	ImGui::PushFont(font, size);
}

void pop()
{
	ImGui::PopFont();
}

} // namespace nc::fonts
