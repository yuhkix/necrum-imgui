#pragma once

// The original necrum menu, rebuilt on top of the framework. Everything that
// used to be hard-wired into the framework (tabs, sidebar entries, overlays,
// ESP preview) now lives here as ordinary application code.

#include "necrum/necrum.h"

namespace game_menu
{

// State of the visuals page; draw_preview() is a custom interactive widget
// (drag the name/weapon/health/flags labels around the bounding box).
struct Visuals
{
	bool esp_box_ = true;
	bool esp_name_ = true;
	bool esp_weapon_ = false;
	bool esp_health_bar_ = true;
	bool esp_skeleton_ = false;
	bool esp_snaplines_ = false;
	bool esp_flags_ = false;
	int esp_box_style_ = 0; // 0 = 2D, 1 = corners

	nc::HSV esp_box_col_{0.0f, 0.74f, 0.78f, 1.0f};
	nc::HSV esp_name_col_{0.0f, 0.0f, 0.91f, 1.0f};
	nc::HSV esp_health_top_col_{120.0f, 0.7f, 0.8f, 1.0f};
	nc::HSV esp_health_bot_col_{0.0f, 0.7f, 0.8f, 1.0f};
	nc::HSV esp_skeleton_col_{0.0f, 0.0f, 0.65f, 1.0f};

	int esp_name_dock_ = 0;		// top
	int esp_weapon_dock_ = 1; // bottom
	int esp_health_dock_ = 2; // left
	int esp_flags_dock_ = 3;	// right

	int esp_dragging_ = -1;
	float esp_drag_ox_ = 0.0f, esp_drag_oy_ = 0.0f;
	bool esp_preview_hold_ = false;

	void draw_preview(ImVec2 body_min, ImVec2 body_max);
};

struct State
{
	// Ragebot
	bool enabled = false;
	bool hitbox[5] = {};
	bool multipoint[3] = {};
	int point_scale_lo = 25, point_scale_hi = 75;
	int minimum_damage = 0;
	int hitchance = 47;
	int dt_hitchance = 50;
	bool strict_hitchance = true;
	bool auto_stop = false;
	bool auto_scope = false;
	int body_aim = -1;
	int delay_shot = -1;
	bool force_body_aim = false;
	int weapon_group = 0;

	// Exploits
	bool double_tap = false;
	bool hide_shots = false;
	bool rapid_fire = false;
	nc::Keybind double_tap_key{ImGuiKey_None, nc::KeybindMode::Toggle};
	nc::Keybind roll_resolver_key{ImGuiKey_None, nc::KeybindMode::Hold};

	// Anti-aim
	bool aa_enabled = false;
	int pitch = 0, yaw = 0, yaw_jitter = 0;
	float yaw_add = 0.0f, jitter_range = 0.0f;
	bool desync = false;
	float desync_range = 0.0f;

	// Legitbot / misc
	bool legit_enabled = false;
	float legit_fov = 3.0f, legit_smooth = 8.0f;
	bool trigger_enabled = false;
	int trigger_delay = 40;
	bool bunny_hop = false, auto_strafe = false, fast_stop = false;
	bool discord_rpc = true;

	// HUD
	bool watermark = true;
	bool show_keybinds = true;
	bool show_spectators = true;
	bool show_bomb = true;

	Visuals visuals;
};

class GameMenu final : public nc::App
{
public:
	GameMenu();
	void frame() override;
	bool wants_input() const override { return shell_.is_open(); }

private:
	void bind_config();
	void page_ragebot();
	void page_exploits();
	void page_antiaim();
	void page_visuals(int section);
	void page_misc(int section);
	void page_legitbot(int section);
	void page_settings();
	void page_configs();
	void draw_spectators();
	void draw_bomb_timer();

	State s_;
	nc::Shell shell_;
	nc::SplashOptions splash_;
	bool greeted_ = false;
};

} // namespace game_menu
