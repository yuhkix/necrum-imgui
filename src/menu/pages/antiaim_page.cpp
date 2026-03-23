#include "menu/menu.h"

namespace menu
{

using namespace ui;

void Menu::page_antiaim()
{
	const char* pitch_items[] = {"None", "Down", "Up", "Zero"};

	const char* yaw_items[] = {"None", "Backward", "Static", "Spin"};

	const char* jitter_items[] = {"None", "Offset", "Center", "Random"};

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
		draw_panel(dl, lp, ImVec2(lp.x + pw, lp.y + ph), "Angles");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28, 28));
	if (!is_searching_all_)
	{
		ImGui::SetCursorScreenPos(ImVec2(lp.x + 10, lp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::BeginChild("##aa_ang", ImVec2(pw - 14, ph - th - 6), false, ImGuiWindowFlags_None);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.85f);
	}

	{
		bool any_left = false;
		auto can = [&](std::initializer_list<const char*> tags) { return ui::search_match(tags); };

		if (can({"enabled", "anti-aim"}))
		{
			flat_checkbox("Enabled", &aa_enabled_);
			any_left = true;
		}

		if (can({"pitch", "angles"}))
		{
			ctrl_label("Pitch");
			flat_dropdown_single("##aap", &pitch_sel_, pitch_items, 4);
			any_left = true;
		}

		if (can({"yaw", "angles"}))
		{
			ctrl_label("Yaw");
			flat_dropdown_single("##aay", &yaw_sel_, yaw_items, 4);
			any_left = true;
		}

		if (can({"yaw add", "yaw offset"}))
		{
			ctrl_label("Yaw add");
			flat_slider("##aaya", &yaw_add_, -180.0f, 180.0f, "%.0f°", 1.0f);
			any_left = true;
		}

		if (can({"yaw jitter", "jitter"}))
		{
			ctrl_label("Yaw jitter");
			flat_dropdown_single("##aayj", &yaw_jitter_sel_, jitter_items, 4);
			any_left = true;
		}

		if (can({"jitter range", "jitter"}))
		{
			ctrl_label("Jitter range");
			flat_slider("##aajr", &jitter_range_, 0.0f, 180.0f, "%.0f°", 1.0f);
			any_left = true;
		}

		if (!any_left && ui::has_search_query() && !is_searching_all_)
			draw_search_empty_hint("No matches in this section.");
	}

	if (!is_searching_all_)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}

	ImGui::PopStyleVar(); // WindowPadding

	ImVec2 rp(lp.x + pw + gap, cur.y);
	if (!is_searching_all_)
		draw_panel(dl, rp, ImVec2(rp.x + pw, rp.y + ph), "Fake-Lag & Desync");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28, 28));
	if (!is_searching_all_)
	{
		ImGui::SetCursorScreenPos(ImVec2(rp.x + 10, rp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::BeginChild("##aa_ds", ImVec2(pw - 14, ph - th - 6), false, ImGuiWindowFlags_None);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.85f);
	}

	{
		bool any_right = false;
		auto can = [&](std::initializer_list<const char*> tags) { return ui::search_match(tags); };

		if (can({"desync enabled", "desync"}))
		{
			flat_checkbox("Desync enabled", &desync_enabled_);
			any_right = true;
		}

		if (can({"desync range", "desync"}))
		{
			ctrl_label("Desync range");
			flat_slider("##aadsr", &desync_range_, 0.0f, 58.0f, "%.0f°", 1.0f);
			any_right = true;
		}

		if (!any_right && ui::has_search_query() && !is_searching_all_)
			draw_search_empty_hint("No matches in this section.");
	}

	if (!is_searching_all_)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}

	ImGui::PopStyleVar(); // WindowPadding
}

} // namespace menu
