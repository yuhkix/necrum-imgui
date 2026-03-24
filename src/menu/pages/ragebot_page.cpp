#include "../menu.h"

namespace menu
{

using namespace ui;

void Menu::page_ragebot()
{
	const char* hitbox_items[] = {"Head", "Chest", "Body", "Arms", "Legs"};

	const char* mp_items[] = {"Center", "Edge", "Auto"};

	const char* ba_items[] = {"Lethal", "Safe", "Always"};

	const char* ds_items[] = {"Default", "Custom"};

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
		draw_panel(dl, lp, ImVec2(lp.x + pw, lp.y + ph), "Static");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 16));
	if (true)
	{
		ImGui::SetCursorScreenPos(ImVec2(lp.x, lp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::BeginChild("##rcc", ImVec2(pw, ph - th - 6), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
		ImGui::PushItemWidth(-1.0f);
	}

	{
		bool any_left = false;
		auto can = [&](std::initializer_list<const char*> tags) { return ui::search_match(tags); };

		if (can({"enabled", "ragebot"}))
		{
			flat_checkbox("Enabled", &enabled_);
			any_left = true;
		}

		if (can({"target hitbox", "hitbox"}))
		{
			ctrl_label("Target hitbox");
			flat_dropdown_multi("##th", hitbox_multi_, hitbox_items, 5);
			any_left = true;
		}

		if (can({"multi-point", "multipoint"}))
		{
			ctrl_label("Multi-point");
			flat_dropdown_multi("##mp", multipoint_multi_, mp_items, 3);
			any_left = true;
		}

		if (can({"point scale", "scale"}))
		{
			ctrl_label("Point scale");
			range_slider("##psr", &point_scale_lo_, &point_scale_hi_, 0, 100, "%d - %d%%");
			any_left = true;
		}

		if (can({"minimum damage", "min damage"}))
		{
			ctrl_label("Minimum damage");
			flat_slider("##md", &minimum_damage_, 0, 100, "%d");
			any_left = true;
		}

		if (!any_left && ui::has_search_query())
			draw_search_empty_hint("No matches in this section.");
	}

	if (true)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}

	ImGui::PopStyleVar(); // WindowPadding

	ImVec2 rp(lp.x + pw + gap, cur.y);
	if (true)
		draw_panel(dl, rp, ImVec2(rp.x + pw, rp.y + ph), "Accuracy");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 16));
	if (true)
	{
		ImGui::SetCursorScreenPos(ImVec2(rp.x, rp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::BeginChild("##rac", ImVec2(pw, ph - th - 6), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
		ImGui::PushItemWidth(-1.0f);
	}

	{
		bool any_right = false;
		auto can = [&](std::initializer_list<const char*> tags) { return ui::search_match(tags); };

		if (can({"hitchance", "accuracy"}))
		{
			ctrl_label("Hitchance");
			flat_slider("##hc", &hitchance_, 0, 100, "%d%%");
			any_right = true;
		}

		if (can({"double tap hitchance", "dt hitchance", "double tap"}))
		{
			ctrl_label("Double tap hitchance");
			flat_slider("##dthc", &dt_hitchance_, 0, 100, "%d%%");
			any_right = true;
		}

		if (can({"strict hitchance"}))
		{
			flat_checkbox("Strict hitchance", &strict_hitchance_);
			any_right = true;
		}

		if (can({"automatic stop", "auto stop"}))
		{
			flat_checkbox("Automatic stop", &auto_stop_);
			any_right = true;
		}

		if (can({"automatic scope", "auto scope"}))
		{
			flat_checkbox("Automatic scope", &auto_scope_);
			any_right = true;
		}

		if (can({"body-aim disablers", "body aim"}))
		{
			ctrl_label("Body-aim disablers");
			flat_dropdown_single("##baddd", &body_aim_sel_, ba_items, IM_ARRAYSIZE(ba_items));
			any_right = true;
		}

		if (can({"delay shot"}))
		{
			ctrl_label("Delay shot");
			flat_dropdown_single("##dsdd", &delay_shot_sel_, ds_items, IM_ARRAYSIZE(ds_items));
			any_right = true;
		}

		if (can({"force body-aim", "force body aim"}))
		{
			flat_checkbox("Force body-aim", &force_body_aim_);
			any_right = true;
		}

		if (!any_right && ui::has_search_query())
			draw_search_empty_hint("No matches in this section.");
	}

	if (true)
	{
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}

	ImGui::PopStyleVar(); // WindowPadding
}

} // namespace menu
