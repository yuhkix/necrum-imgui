#include "menu/menu.h"

namespace menu
{
using namespace ui;

void Menu::page_legitbot()
{
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 cur = ImGui::GetCursorScreenPos();
	float aw = ImGui::GetContentRegionAvail().x;
	float ah = ImGui::GetContentRegionAvail().y;
	float pad = 6.0f, gap = 6.0f;
	float pw = (aw - pad * 2 - gap) * 0.5f;
	float ph = ah - pad;
	float th = 30.0f;

	ImVec2 lp(cur.x + pad, cur.y);
	if (!is_searching_all_)
		draw_panel(dl, lp, ImVec2(lp.x + pw, lp.y + ph), "Legitbot");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28, 28));
	if (!is_searching_all_)
	{
		ImGui::SetCursorScreenPos(ImVec2(lp.x + 10, lp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::BeginChild("##legit_main", ImVec2(pw - 14, ph - th - 6), false, ImGuiWindowFlags_None);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.85f);
	}
	{
		if (!ui::has_search_query() || ui::search_match({"aimbot", "legitbot"}))
		{
			ImGui::TextDisabled("Legitbot options are not implemented yet.");
		}
		else if (!is_searching_all_)
		{
			draw_search_empty_hint("No matches in this section.");
		}
	}

	if (!is_searching_all_)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleVar(); // ScrollbarSize
	}
	ImGui::PopStyleVar(); // WindowPadding

	ImVec2 rp(lp.x + pw + gap, cur.y);
	if (!is_searching_all_)
		draw_panel(dl, rp, ImVec2(rp.x + pw, rp.y + ph), "Triggerbot");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28, 28));
	if (!is_searching_all_)
	{
		ImGui::SetCursorScreenPos(ImVec2(rp.x + 10, rp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::BeginChild("##legit_trigger", ImVec2(pw - 14, ph - th - 6), false, ImGuiWindowFlags_None);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.85f);
	}
	{
		if (!ui::has_search_query() || ui::search_match({"trigger", "triggerbot"}))
		{
			ImGui::TextDisabled("Triggerbot options are not implemented yet.");
		}
		else if (!is_searching_all_)
		{
			draw_search_empty_hint("No matches in this section.");
		}
	}

	if (!is_searching_all_)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleVar(); // ScrollbarSize
	}
	ImGui::PopStyleVar(); // WindowPadding
}
} // namespace menu
