#include "game_menu.h"

#include "necrum/extras/web_image_imgui.h"

namespace game_menu
{

namespace
{
constexpr const char* k_logo_url = "https://i.ibb.co/k2tJg9Gx/necrum.png";

// Two full-height panels side by side: the layout every page of this menu uses.
template <typename Left, typename Right>
void two_panels(const char* left_title, Left&& left, const char* right_title, Right&& right)
{
	nc::begin_columns(2);
	if (nc::begin_panel(left_title, {0, -1}))
		left();
	nc::end_panel();
	nc::next_column();
	if (nc::begin_panel(right_title, {0, -1}))
		right();
	nc::end_panel();
	nc::end_columns();
}
} // namespace

GameMenu::GameMenu()
		: shell_([] {
				nc::ShellConfig cfg;
				cfg.title = "necrum";
				cfg.logo_url = k_logo_url;
				cfg.footer_left = {{"build:", "dev"}};
				cfg.footer_right = {{"active user:", "Yuhki"}};
				return cfg;
			}())
{
	splash_.title = "necrum";
	splash_.logo_url = k_logo_url;

	nc::keybinds::track(&s_.double_tap_key, "Double tap");
	nc::keybinds::track(&s_.roll_resolver_key, "Roll resolver");
	bind_config();

	auto& rage = shell_.add_tab("Rage", ICON_FA_GUN);
	rage.toolbar = [this] {
		static const char* groups[] = {ICON_FA_CROSSHAIRS, ICON_FA_BOLT,				 ICON_FA_FIRE,
																	 ICON_FA_BULLSEYE,	 ICON_FA_SKULL_CROSSBONES, ICON_FA_SHIELD_HALVED};
		nc::tab_strip("##weapon_groups", &s_.weapon_group, groups, 6);
	};
	rage.add_page("RAGEBOT", ICON_FA_GUN, [this] { page_ragebot(); });
	rage.add_page("EXPLOITS", ICON_FA_BOLT, [this] { page_exploits(); });
	rage.add_page("ANTI-AIM", ICON_FA_GHOST, [this] { page_antiaim(); });

	auto& visuals = shell_.add_tab("Visuals", ICON_FA_EYE);
	visuals.add_page("ESP", ICON_FA_EYE, [this] { page_visuals(0); });
	visuals.add_page("CHAMS", ICON_FA_USER, [this] { page_visuals(1); });
	visuals.add_page("GLOW", ICON_FA_SHIELD_HALVED, [this] { page_visuals(2); });

	auto& misc = shell_.add_tab("Misc", ICON_FA_WRENCH);
	misc.add_page("GENERAL", ICON_FA_WRENCH, [this] { page_misc(0); });
	misc.add_page("MOVEMENT", ICON_FA_GEAR, [this] { page_misc(1); });
	misc.add_page("SKINS", ICON_FA_USER, [this] { page_misc(2); });

	auto& legit = shell_.add_tab("Legit", ICON_FA_BOLT);
	legit.add_page("AIMBOT", ICON_FA_SHIELD_HALVED, [this] { page_legitbot(0); });
	legit.add_page("TRIGGER", ICON_FA_HAND_POINTER, [this] { page_legitbot(1); });

	auto& settings = shell_.add_tab("Settings", ICON_FA_GEAR);
	settings.add_page("SETTINGS", ICON_FA_GEAR, [this] { page_settings(); });
	settings.add_page("CONFIGS", ICON_FA_FILE_LINES, [this] { page_configs(); });
}

void GameMenu::bind_config()
{
	auto& c = nc::config();
	c.bind("rage.enabled", &s_.enabled);
	c.bind("rage.hitbox", s_.hitbox, 5);
	c.bind("rage.multipoint", s_.multipoint, 3);
	c.bind("rage.point_scale_lo", &s_.point_scale_lo);
	c.bind("rage.point_scale_hi", &s_.point_scale_hi);
	c.bind("rage.min_damage", &s_.minimum_damage);
	c.bind("rage.hitchance", &s_.hitchance);
	c.bind("rage.dt_hitchance", &s_.dt_hitchance);
	c.bind("rage.strict_hitchance", &s_.strict_hitchance);
	c.bind("rage.auto_stop", &s_.auto_stop);
	c.bind("rage.auto_scope", &s_.auto_scope);
	c.bind("rage.body_aim", &s_.body_aim);
	c.bind("rage.delay_shot", &s_.delay_shot);
	c.bind("rage.force_body_aim", &s_.force_body_aim);
	c.bind("exploits.double_tap", &s_.double_tap);
	c.bind("exploits.double_tap_key", &s_.double_tap_key);
	c.bind("exploits.hide_shots", &s_.hide_shots);
	c.bind("exploits.rapid_fire", &s_.rapid_fire);
	c.bind("exploits.roll_resolver_key", &s_.roll_resolver_key);
	c.bind("aa.enabled", &s_.aa_enabled);
	c.bind("aa.pitch", &s_.pitch);
	c.bind("aa.yaw", &s_.yaw);
	c.bind("aa.yaw_add", &s_.yaw_add);
	c.bind("aa.yaw_jitter", &s_.yaw_jitter);
	c.bind("aa.jitter_range", &s_.jitter_range);
	c.bind("aa.desync", &s_.desync);
	c.bind("aa.desync_range", &s_.desync_range);

	Visuals& v = s_.visuals;
	c.bind("esp.box", &v.esp_box_);
	c.bind("esp.box_style", &v.esp_box_style_);
	c.bind("esp.box_color", &v.esp_box_col_);
	c.bind("esp.name", &v.esp_name_);
	c.bind("esp.name_color", &v.esp_name_col_);
	c.bind("esp.weapon", &v.esp_weapon_);
	c.bind("esp.health", &v.esp_health_bar_);
	c.bind("esp.health_top", &v.esp_health_top_col_);
	c.bind("esp.health_bottom", &v.esp_health_bot_col_);
	c.bind("esp.skeleton", &v.esp_skeleton_);
	c.bind("esp.skeleton_color", &v.esp_skeleton_col_);
	c.bind("esp.snaplines", &v.esp_snaplines_);
	c.bind("esp.flags", &v.esp_flags_);
	c.bind("esp.name_dock", &v.esp_name_dock_);
	c.bind("esp.weapon_dock", &v.esp_weapon_dock_);
	c.bind("esp.health_dock", &v.esp_health_dock_);
	c.bind("esp.flags_dock", &v.esp_flags_dock_);

	c.bind("hud.watermark", &s_.watermark);
	c.bind("hud.keybinds", &s_.show_keybinds);
	c.bind("hud.spectators", &s_.show_spectators);
	c.bind("hud.bomb", &s_.show_bomb);
	c.bind("ui.accent", &nc::theme().accent);
}

void GameMenu::frame()
{
	if (nc::splash(splash_))
		return;
	if (!greeted_)
	{
		nc::notify(nc::Notify::Success, "Successfully loaded menu!");
		greeted_ = true;
	}

	if (s_.watermark)
		nc::watermark("necrum", [](std::vector<std::string>& out) {
			out.push_back("uid: 1");
			out.push_back("user: Yuhki");
			out.push_back(nc::fps_text());
			out.push_back(nc::clock_text());
		});

	const float margin = 18.0f;
	const ImVec2 display = ImGui::GetIO().DisplaySize;
	nc::keybind_overlay(s_.show_keybinds, {.position = ImVec2(margin, display.y * 0.45f)});
	draw_spectators();
	draw_bomb_timer();

	shell_.render();
}

void GameMenu::page_ragebot()
{
	static const char* hitboxes[] = {"Head", "Chest", "Body", "Arms", "Legs"};
	static const char* multipoint[] = {"Center", "Edge", "Auto"};
	static const char* body_aim[] = {"Lethal", "Safe", "Always"};
	static const char* delay[] = {"Default", "Custom"};

	two_panels(
			"Static",
			[&] {
				nc::checkbox("Enabled", &s_.enabled);
				nc::multi_combo("Target hitbox", s_.hitbox, hitboxes, 5);
				nc::multi_combo("Multi-point", s_.multipoint, multipoint, 3);
				nc::range_slider("Point scale", &s_.point_scale_lo, &s_.point_scale_hi, 0, 100, "%d - %d%%");
				nc::slider("Minimum damage", &s_.minimum_damage, 0, 100);
			},
			"Accuracy",
			[&] {
				nc::slider("Hitchance", &s_.hitchance, 0, 100, "%d%%");
				nc::slider("Double tap hitchance", &s_.dt_hitchance, 0, 100, "%d%%");
				nc::checkbox("Strict hitchance", &s_.strict_hitchance);
				nc::checkbox("Automatic stop", &s_.auto_stop);
				nc::checkbox("Automatic scope", &s_.auto_scope);
				nc::combo("Body-aim disablers", &s_.body_aim, body_aim, 3, true);
				nc::combo("Delay shot", &s_.delay_shot, delay, 2, true);
				nc::checkbox("Force body-aim", &s_.force_body_aim);
			});
}

void GameMenu::page_exploits()
{
	two_panels(
			"Combat",
			[&] {
				nc::search::tags({"dt", "hotkey"});
				nc::checkbox("Double tap", &s_.double_tap);
				nc::inline_keybind("##dt_key", &s_.double_tap_key);
				nc::checkbox("Hide shots", &s_.hide_shots);
				nc::checkbox("Rapid fire", &s_.rapid_fire);
			},
			"Resolver",
			[&] { nc::keybind("Roll resolver", &s_.roll_resolver_key); });
}

void GameMenu::page_antiaim()
{
	static const char* pitch[] = {"None", "Down", "Up", "Zero"};
	static const char* yaw[] = {"None", "Backward", "Static", "Spin"};
	static const char* jitter[] = {"None", "Offset", "Center", "Random"};
	two_panels(
			"Angles",
			[&] {
				nc::checkbox("Enabled", &s_.aa_enabled);
				nc::combo("Pitch", &s_.pitch, pitch, 4);
				nc::combo("Yaw", &s_.yaw, yaw, 4);
				nc::slider("Yaw add", &s_.yaw_add, -180.0f, 180.0f, "%.0f deg");
				nc::combo("Yaw jitter", &s_.yaw_jitter, jitter, 4);
				nc::slider("Jitter range", &s_.jitter_range, 0.0f, 180.0f, "%.0f deg");
			},
			"Fake-Lag & Desync",
			[&] {
				nc::checkbox("Desync enabled", &s_.desync);
				nc::slider("Desync range", &s_.desync_range, 0.0f, 58.0f, "%.0f deg");
			});
}

void GameMenu::page_visuals(int section)
{
	Visuals& v = s_.visuals;
	if (section != 0)
	{
		two_panels(
				section == 1 ? "Chams" : "Glow", [] { nc::text_dim("Nothing here yet."); }, "Colors",
				[] { nc::text_dim("Nothing here yet."); });
		return;
	}

	static const char* box_styles[] = {"2D", "Corners"};
	static const nc::HSV box_default{0.0f, 0.74f, 0.78f, 1.0f};
	static const nc::HSV name_default{0.0f, 0.0f, 0.91f, 1.0f};

	nc::begin_columns(2);
	if (nc::begin_panel("ESP Preview", {0, -1}, nc::PanelFlags_NoPadding | nc::PanelFlags_NoScroll | nc::PanelFlags_NoSearchHint))
	{
		if (!nc::search::dry_run())
		{
			v.draw_preview(nc::panel_body_min(), nc::panel_body_max());
			if (v.esp_dragging_ >= 0 || v.esp_preview_hold_)
				nc::block_window_move();
		}
	}
	nc::end_panel();
	nc::next_column();
	if (nc::begin_panel("ESP Settings", {0, -1}))
	{
		nc::checkbox("Bounding box", &v.esp_box_);
		nc::inline_color("##box_col", &v.esp_box_col_, &box_default);
		if (v.esp_box_)
			nc::combo("Box style", &v.esp_box_style_, box_styles, 2);
		nc::checkbox("Health bar", &v.esp_health_bar_);
		if (v.esp_health_bar_)
		{
			nc::color_edit("Health top", &v.esp_health_top_col_);
			nc::color_edit("Health bottom", &v.esp_health_bot_col_);
		}
		nc::checkbox("Name", &v.esp_name_);
		nc::inline_color("##name_col", &v.esp_name_col_, &name_default);
		nc::checkbox("Weapon", &v.esp_weapon_);
		nc::checkbox("Skeleton", &v.esp_skeleton_);
		nc::inline_color("##skel_col", &v.esp_skeleton_col_);
		nc::checkbox("Snap lines", &v.esp_snaplines_);
		nc::checkbox("Flags", &v.esp_flags_);
	}
	nc::end_panel();
	nc::end_columns();
}

void GameMenu::page_misc(int section)
{
	if (section == 2)
	{
		two_panels("Skins", [] { nc::text_dim("Nothing here yet."); }, "Preview", [] { nc::text_dim("Nothing here yet."); });
		return;
	}
	two_panels(
			"General", [&] { nc::checkbox("Discord RPC", &s_.discord_rpc); }, "Movement",
			[&] {
				nc::checkbox("Bunny hop", &s_.bunny_hop);
				nc::checkbox("Auto strafe", &s_.auto_strafe);
				nc::checkbox("Fast stop", &s_.fast_stop);
			});
}

void GameMenu::page_legitbot(int section)
{
	if (section == 0)
		two_panels(
				"Legitbot",
				[&] {
					nc::checkbox("Enabled", &s_.legit_enabled);
					nc::slider("Field of view", &s_.legit_fov, 0.0f, 30.0f, "%.1f deg");
					nc::slider("Smoothing", &s_.legit_smooth, 1.0f, 30.0f, "%.1f");
				},
				"Targets", [] { nc::text_dim("Nothing here yet."); });
	else
		two_panels(
				"Triggerbot",
				[&] {
					nc::checkbox("Enabled", &s_.trigger_enabled);
					nc::slider("Delay", &s_.trigger_delay, 0, 250, "%d ms");
				},
				"Filters", [] { nc::text_dim("Nothing here yet."); });
}

void GameMenu::page_settings()
{
	two_panels(
			"UI Settings",
			[&] {
				nc::theme_editor();
				nc::separator("Overlay widgets");
				nc::checkbox("Watermark", &s_.watermark);
				nc::checkbox("Hotkey list", &s_.show_keybinds);
				nc::checkbox("Spectator list", &s_.show_spectators);
				nc::checkbox("Bomb timer", &s_.show_bomb);
			},
			"Notifications",
			[&] {
				if (nc::button("Replay intro", {-1, 0}))
					nc::reset_splash();
				if (nc::button("Error notification", {-1, 0}))
					nc::notify(nc::Notify::Error, "Test error notification");
				if (nc::button("Info notification", {-1, 0}))
					nc::notify(nc::Notify::Info, "Test info notification");
				if (nc::button("Success notification", {-1, 0}))
					nc::notify(nc::Notify::Success, "Test success notification");
				if (nc::button("Warning notification", {-1, 0}))
					nc::notify(nc::Notify::Warning, "Test warning notification");
			});
}

void GameMenu::page_configs()
{
	two_panels(
			"Config System", [] { nc::config_manager("configs"); }, "Info",
			[] { nc::text_wrapped("Configs are stored as plain text in ./configs next to the executable."); });
}

void GameMenu::draw_spectators()
{
	static const std::vector<std::string> names = {"Necrum Demo"};
	char badge[32];
	snprintf(badge, sizeof(badge), "%zu watching", names.size());

	const ImVec2 display = ImGui::GetIO().DisplaySize;
	nc::OverlayOptions o;
	o.position = ImVec2(display.x - 18.0f, display.y * 0.45f);
	o.pivot = ImVec2(1.0f, 0.0f);
	o.badge = badge;
	if (!nc::begin_overlay("Spectators", s_.show_spectators, o))
		return;

	const nc::Theme& t = nc::theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	for (const auto& name : names)
	{
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 28.0f));
		ImVec2 c(pos.x + 10.0f, pos.y + 14.0f);
		if (!nc::web_image::draw_circle(dl, k_logo_url, c, 10.0f, nc::styled(IM_COL32_WHITE)))
		{
			char initial[2] = {name.empty() ? '?' : (char)std::toupper((unsigned char)name[0]), 0};
			dl->AddCircleFilled(c, 9.0f, nc::styled(t.accent_col, 0.3f), 16);
			ImVec2 is = ImGui::CalcTextSize(initial);
			dl->AddText(ImVec2(c.x - is.x * 0.5f, c.y - is.y * 0.5f), nc::styled(t.text), initial);
		}
		dl->AddCircle(c, 10.5f, nc::styled(t.hover, 0.06f), 24);
		dl->AddText(ImVec2(c.x + 18.0f, c.y - ImGui::GetFontSize() * 0.5f), nc::styled(t.text), name.c_str());
	}
	nc::end_overlay();
}

void GameMenu::draw_bomb_timer()
{
	const float total = 40.0f;
	const float time_left = total - std::min(std::fmod((float)ImGui::GetTime(), total + 3.0f), total);
	const float damage = 75.0f;
	const int hp_after = std::max(0, 100 - (int)damage);
	const bool survives = hp_after > 0;
	const nc::Theme& t = nc::theme();

	nc::OverlayOptions o;
	o.anchor = nc::Corner::TopCenter;
	o.margin = 58.0f;
	o.badge = survives ? "SURVIVE" : "DEAD";
	o.badge_color = survives ? t.success : t.error;
	if (!nc::begin_overlay("Bomb Timer", s_.show_bomb, o))
		return;

	char timer[32];
	snprintf(timer, sizeof(timer), "%.1fs", time_left);
	{
		nc::fonts::Scope big(nc::fonts::title(), 28.0f);
		nc::text("%s", timer);
	}
	nc::progress_bar("##bomb_progress", time_left / total, " ");
	nc::text_colored(survives ? t.success : t.error, "Estimated HP: %d", hp_after);
	nc::text_dim("Damage: %.0f", damage);
	nc::end_overlay();
}

} // namespace game_menu

NC_REGISTER_APP(game_menu::GameMenu)
