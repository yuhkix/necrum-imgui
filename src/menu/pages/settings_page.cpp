#include "../menu.h"

namespace menu

{

using namespace ui;

void Menu::page_settings()

{

	ImDrawList* dl = ImGui::GetWindowDrawList();

	ImVec2 cur = ImGui::GetCursorScreenPos();

	float aw = ImGui::GetContentRegionAvail().x;

	float ah = ImGui::GetContentRegionAvail().y;

	float pad = 16.0f, gap = 16.0f;

	float pw = (aw - pad * 2 - gap) * 0.5f;

	float ph = ah - pad;

	float th = 30.0f;
	static const char* config_items[] = {"Legit", "Rage", "HvH", "Scout", "Auto"};

	ImVec2 lp(cur.x + pad, cur.y);

	if (true)
		draw_panel(dl, lp, ImVec2(lp.x + pw, lp.y + ph), "UI Settings");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 16));
	if (true)
	{
		ImGui::SetCursorScreenPos(ImVec2(lp.x, lp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, styled(col_groove_brd));
		ImGui::BeginChild("##sui", ImVec2(pw, ph - th - 6), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
		ImGui::PushItemWidth(-1.0f);
	}

	{

		bool any_left = false;

		auto can = [&](std::initializer_list<const char*> tags) { return ui::search_match(tags); };

		if (can({"animation speed", "ui speed"}))

		{

			ctrl_label("Animation speed");

			flat_slider("##aspd", &anim_speed_, 2.0f, 30.0f, "%.0f", 1.0f);

			any_left = true;
		}

		if (can({"preview", "animation graph"}))

		{

			ctrl_label("Preview");

			anim_graph("##ag", ImGui::GetContentRegionAvail().x, 50, anim_speed_);

			any_left = true;
		}

		if (can({"discord rpc", "rpc"}))

		{

			flat_checkbox("Discord RPC", &discord_rpc_);

			any_left = true;
		}

		if (can({"watermark"}))

		{

			flat_checkbox("Watermark", &water_mark_enabled_);

			any_left = true;
		}

		if (can({"accent color", "accent", "theme"}))

		{

			flat_color_picker("Accent Color##accent", &accent_hue_, &accent_sat_, &accent_val_, 265.91f, 0.690f, 1.0f);

			any_left = true;
		}

		bool show_hotkey_w = can({"hotkey window", "hotkey list", "keybind"});

		bool show_spec_w = can({"spectator list", "spectators"});

		bool show_bomb_w = can({"bomb timer", "c4", "bomb"});

		if (show_hotkey_w || show_spec_w || show_bomb_w)

		{

			ctrl_label("Overlay widgets");

			if (show_hotkey_w)

				flat_checkbox("Hotkey list", &show_hotkey_overlay_);

			if (show_spec_w)

				flat_checkbox("Spectator list", &show_spectator_overlay_);

			if (show_bomb_w)

				flat_checkbox("Bomb timer", &show_bomb_overlay_);

			any_left = true;
		}

		bool show_replay = can({"replay intro", "intro"});

		bool show_error_n =

				can({"error notification", "notification", "toast", "error"});

		bool show_info_n =

				can({"info notification", "notification", "toast", "info"});

		bool show_success_n =

				can({"success notification", "notification", "toast", "success"});

		bool show_warn_n =

				can({"warning notification", "notification", "toast", "warning"});

		if (show_replay || show_error_n || show_info_n || show_success_n ||

				show_warn_n)

		{

			ImVec4 ac_btn = ImGui::ColorConvertU32ToFloat4(col_accent);

			ImGui::PushStyleColor(

					ImGuiCol_Button,

					ImVec4(ac_btn.x * 0.3f, ac_btn.y * 0.3f, ac_btn.z * 0.3f, 0.7f));

			ImGui::PushStyleColor(

					ImGuiCol_ButtonHovered,

					ImVec4(ac_btn.x * 0.5f, ac_btn.y * 0.5f, ac_btn.z * 0.5f, 0.85f));

			ImGui::PushStyleColor(

					ImGuiCol_ButtonActive,

					ImVec4(ac_btn.x * 0.7f, ac_btn.y * 0.7f, ac_btn.z * 0.7f, 1.0f));

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

			if (show_replay && ImGui::Button("Replay Intro", ImVec2(ImGui::GetContentRegionAvail().x, 24)))

			{

				intro_playing_ = true;

				intro_start_time_ = -1.0f;
			}

			if (show_error_n &&

					ImGui::Button("Error Notification", ImVec2(ImGui::GetContentRegionAvail().x, 24)))

			{

				char buf[128];

				sprintf(buf, "Error test");

				add_notification(buf, NotifyType::_ERROR);
			}

			if (show_info_n &&

					ImGui::Button("Info Notification", ImVec2(ImGui::GetContentRegionAvail().x, 24)))

			{

				char buf[128];

				sprintf(buf, "Info test");

				add_notification(buf, NotifyType::_INFO);
			}

			if (show_success_n &&

					ImGui::Button("Success Notification", ImVec2(ImGui::GetContentRegionAvail().x, 24)))

			{

				char buf[128];

				sprintf(buf, "Success test");

				add_notification(buf, NotifyType::_SUCCESS);
			}

			if (show_warn_n &&

					ImGui::Button("Warning Notification", ImVec2(ImGui::GetContentRegionAvail().x, 24)))

			{

				char buf[128];

				sprintf(buf, "Warning test");

				add_notification(buf, NotifyType::_WARNING);
			}

			ImGui::PopStyleVar();

			ImGui::PopStyleColor(3);

			any_left = true;
		}

		if (!any_left && ui::has_search_query())
			draw_search_empty_hint("No matches in this section.");
	}

	if (true)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(2);
	}
	else
	{
		ImGui::PopStyleVar(); // WindowPadding
	}

	ImVec2 rp(lp.x + pw + gap, cur.y);
	if (true)
		draw_panel(dl, rp, ImVec2(rp.x + pw, rp.y + ph), "Config System");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(28, 12));
	if (true)
	{
		ImGui::SetCursorScreenPos(ImVec2(rp.x, rp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::BeginChild("##scfg", ImVec2(pw, ph - th - 6), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_None);
		ImGui::PushItemWidth(-1.0f);
	}

	{

		bool any_right = false;

		auto can = [&](std::initializer_list<const char*> tags) { return ui::search_match(tags); };

		float btn_h = 24.0f;

		if (can({"presets", "config", "config system"}))

		{

			ctrl_label("Presets");

			float spacing = ImGui::GetStyle().ItemSpacing.y;

			float remaining = ImGui::GetContentRegionAvail().y - btn_h * 2 - spacing * 12;

			if (remaining < 60.0f)

				remaining = 60.0f;

			flat_listbox("##cfgsel", &config_sel_, config_items, 5, remaining, ImGui::GetContentRegionAvail().x);

			any_right = true;
		}

		bool show_save = can({"save config", "save", "config"});

		bool show_load = can({"load config", "load", "config"});

		if (show_save || show_load)

		{

			ImVec4 ac_btn = ImGui::ColorConvertU32ToFloat4(col_accent);

			ImGui::PushStyleColor(

					ImGuiCol_Button,

					ImVec4(ac_btn.x * 0.3f, ac_btn.y * 0.3f, ac_btn.z * 0.3f, 0.7f));

			ImGui::PushStyleColor(

					ImGuiCol_ButtonHovered,

					ImVec4(ac_btn.x * 0.5f, ac_btn.y * 0.5f, ac_btn.z * 0.5f, 0.85f));

			ImGui::PushStyleColor(

					ImGuiCol_ButtonActive,

					ImVec4(ac_btn.x * 0.7f, ac_btn.y * 0.7f, ac_btn.z * 0.7f, 1.0f));

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

			if (show_save && ImGui::Button("Save Config", ImVec2(ImGui::GetContentRegionAvail().x, btn_h)))

			{

				char buf[128];

				sprintf(buf, "Saved configuration: %s", config_items[config_sel_]);

				add_notification(buf, NotifyType::_SUCCESS);
			}

			if (show_load && ImGui::Button("Load Config", ImVec2(ImGui::GetContentRegionAvail().x, btn_h)))

			{

				char buf[128];

				sprintf(buf, "Loaded configuration: %s", config_items[config_sel_]);

				add_notification(buf, NotifyType::_SUCCESS);
			}

			ImGui::PopStyleVar();

			ImGui::PopStyleColor(3);

			any_right = true;
		}

		if (!any_right && ui::has_search_query())
			draw_search_empty_hint("No matches in this section.");
	}

	if (true)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
	}
	else
	{
		ImGui::PopStyleVar(); // WindowPadding
	}
}

} // namespace menu
