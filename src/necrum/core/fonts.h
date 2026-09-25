#pragma once

#include "base.h"

namespace nc::fonts
{

struct Config
{
	// TTF/OTF paths. Missing files silently fall back to ImGui's built-in font.
	std::string regular_path;
	std::string bold_path;
	std::string title_path; // defaults to bold_path when empty

	float size = 14.0f;
	float title_size = 22.0f;
	float icon_size = 14.0f;

	// Merge the bundled Font Awesome glyphs into the text fonts so ICON_FA_*
	// strings can be mixed with regular text.
	bool merge_icons = true;
	float merged_icon_size = 13.0f;
};

// Platform defaults (Segoe UI on Windows, DejaVu/Liberation on Linux when present).
Config default_config();

// Loads all framework fonts into io.Fonts. Call once after ImGui::CreateContext().
void load(ImGuiIO& io, const Config& cfg = default_config());

ImFont* regular();
ImFont* bold();
ImFont* title();
ImFont* icons();

// PushFont using the font's own size (ImGui 1.92 PushFont takes an explicit size).
void push(ImFont* font, float size = 0.0f);
void pop();

struct Scope
{
	explicit Scope(ImFont* font, float size = 0.0f) { push(font, size); }
	~Scope() { pop(); }
	Scope(const Scope&) = delete;
	Scope& operator=(const Scope&) = delete;
};

} // namespace nc::fonts
