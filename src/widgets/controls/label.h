#pragma once
#include "../ui_core.h"

namespace ui
{
inline void ctrl_label(const char* text)
{
	ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4(0.50f, 0.50f, 0.50f, 1)));
	ImGui::TextUnformatted(text);
	ImGui::PopStyleColor();
}
} // namespace ui
