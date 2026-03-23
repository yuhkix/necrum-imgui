#pragma once

struct ImGuiIO;
struct ImFont;

namespace theme
{

void Apply();
void LoadFonts(ImGuiIO& io);

// Accessible font pointers (set by LoadFonts)
inline ImFont* font_regular = nullptr;
inline ImFont* font_bold = nullptr;
inline ImFont* font_logo = nullptr;
inline ImFont* font_icons = nullptr;

} // namespace theme
