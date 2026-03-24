#include "menu/menu.h"

namespace menu

{

using namespace ui;

void Menu::page_misc()
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 cur = ImGui::GetCursorScreenPos();
	float aw = ImGui::GetContentRegionAvail().x;
	float ah = ImGui::GetContentRegionAvail().y;
	float pad = 16.0f, gap = 16.0f;
	float pw = (aw - pad * 2 - gap) * 0.5f;
	float ph = ah - pad;
	float th = 30.0f;

	ImVec2 lp(cur.x + pad, cur.y);
	if (true)
		draw_panel(dl, lp, ImVec2(lp.x + pw, lp.y + ph), "General");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 16));
	if (true)
	{
		ImGui::SetCursorScreenPos(ImVec2(lp.x, lp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::BeginChild("##misc_general", ImVec2(pw, ph - th - 6), ImGuiChildFlags_AlwaysUseWindowPadding,
											ImGuiWindowFlags_None);
		ImGui::PushItemWidth(-1.0f);
	}

	{
		if (!ui::has_search_query() || ui::search_match({"misc", "general"}))
		{
			ImGui::TextDisabled("General options are not implemented yet.");
		}
		else
		{
			draw_search_empty_hint("No matches in this section.");
		}
	}

	if (true)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleVar(); // ScrollbarSize
	}
	ImGui::PopStyleVar(); // WindowPadding

	ImVec2 rp(lp.x + pw + gap, cur.y);
	if (true)
		draw_panel(dl, rp, ImVec2(rp.x + pw, rp.y + ph), "Movement");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 16));
	if (true)
	{
		ImGui::SetCursorScreenPos(ImVec2(rp.x, rp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::BeginChild("##misc_move", ImVec2(pw, ph - th - 6), ImGuiChildFlags_AlwaysUseWindowPadding,
											ImGuiWindowFlags_None);
		ImGui::PushItemWidth(-1.0f);
	}

	{
		if (!ui::has_search_query() || ui::search_match({"movement", "misc"}))
		{
			ImGui::TextDisabled("Movement options are not implemented yet.");
		}
		else if (ui::has_search_query())
		{
			draw_search_empty_hint("No matches in this section.");
		}
	}

	if (true)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleVar(); // ScrollbarSize
	}
	ImGui::PopStyleVar(); // WindowPadding
}

} // namespace menu
