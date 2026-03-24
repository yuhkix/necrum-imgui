#pragma once
#include "../core/fonts/FontAwesome.h"
#include "../widgets/ui_framework.h"
#include <imgui.h>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

namespace menu
{
class Menu
{
public:
	void render();
	bool is_visible() const { return alpha_ > 0.01f; }

private:
	void draw_header(float pw);
	void draw_search_bar(float pw);
	void draw_sidebar(float ph);
	void draw_weapon_tabs(float pw);
	void draw_content(float pw, float ph);
	void draw_footer(float pw);
	void draw_watermark();
	void draw_intro();
	void render_main();
	void render_overlays();
	float draw_hotkey_overlay(const ImVec2& pos, float width);
	float draw_spectator_overlay(const ImVec2& pos, float width);
	float draw_bomb_timer_overlay(const ImVec2& pos, float width);
	std::string format_key_name(int key) const;
	ImU32 avatar_color_for_name(const std::string& name);
	std::string get_spectator_avatar_url(const std::string& name);

	void page_ragebot();
	void page_exploits();
	void page_antiaim();
	void page_visuals();
	void page_misc();
	void page_legitbot();
	void page_settings();
	void run_search_pass();
	void sync_search_query();
	void page_search_results();

	float alpha_ = 1.0f;
	float target_alpha_ = 1.0f;
	float anim_speed_ = 14.0f;

	float hotkey_alpha_ = 0.0f;
	float spec_alpha_ = 0.0f;
	float bomb_alpha_ = 0.0f;

	int top_tab_ = 0;
	int side_tab_ = 0;
	int weapon_tab_ = 0;

	float anim_side_y_ = -1.0f;

	int prev_side_tab_ = 0;
	int prev_top_tab_ = 0;
	float tab_fade_ = 1.0f;

	bool intro_playing_ = true;
	bool intro_centered_ = false;
	float intro_start_time_ = -1.0f;

	bool enabled_ = false;
	bool silent_aim_ = false;
	bool auto_fire_ = false;
	bool auto_knife_ = false;
	bool dormant_aimbot_ = false;
	int targets_per_tick_ = 1;
	int roll_resolver_key_ = 0;
	int roll_resolver_mode_ = 0;
	bool roll_resolver_toggle_state_ = false;
	int roll_resolver_key_prev_ = 0;
	bool hitbox_multi_[5] = {};
	bool multipoint_multi_[3] = {};
	int head_scale_ = 0;
	int point_scale_lo_ = 25;
	int point_scale_hi_ = 75;
	int minimum_damage_ = 0;
	int hitchance_ = 47;
	int dt_hitchance_ = 50;
	bool strict_hitchance_ = true;
	bool auto_stop_ = false;
	bool auto_scope_ = false;
	int body_aim_sel_ = -1;
	int delay_shot_sel_ = -1;
	bool force_body_aim_ = false;

	bool double_tap_ = false;
	bool hide_shots_ = false;
	bool rapid_fire_ = false;
	int double_tap_key_ = 0;
	int double_tap_mode_ = 0;
	bool double_tap_toggle_state_ = false;
	int double_tap_key_prev_ = 0;

	bool aa_enabled_ = false;
	int pitch_sel_ = 0;
	int yaw_sel_ = 0;
	int yaw_jitter_sel_ = 0;
	float yaw_add_ = 0.0f;
	float jitter_range_ = 0.0f;
	bool desync_enabled_ = false;
	float desync_range_ = 0.0f;

	bool discord_rpc_ = true;
	bool water_mark_enabled_ = true;
	int config_sel_ = 0;

	bool show_hotkey_overlay_ = true;
	bool show_spectator_overlay_ = true;
	bool show_bomb_overlay_ = true;

	float accent_hue_ = 265.91f; // 0 = red (default)
	float accent_sat_ = 0.690f;
	float accent_val_ = 1.0f;

	bool esp_box_ = true;
	bool esp_name_ = true;
	bool esp_weapon_ = false;
	bool esp_health_bar_ = true;
	bool esp_skeleton_ = false;
	bool esp_snaplines_ = false;
	bool esp_flags_ = false;
	int esp_box_style_ = 0; // 0=2D, 1=Corner

	float esp_box_hue_ = 0.0f, esp_box_sat_ = 0.74f, esp_box_val_ = 0.78f;
	float esp_name_hue_ = 0.0f, esp_name_sat_ = 0.0f, esp_name_val_ = 0.91f;
	float esp_health_top_hue_ = 120.0f, esp_health_top_sat_ = 0.7f, esp_health_top_val_ = 0.8f;
	float esp_health_bot_hue_ = 0.0f, esp_health_bot_sat_ = 0.7f, esp_health_bot_val_ = 0.8f;
	float esp_skeleton_hue_ = 0.0f, esp_skeleton_sat_ = 0.0f, esp_skeleton_val_ = 0.65f;

	int esp_name_dock_ = 0;		// default: top
	int esp_weapon_dock_ = 1; // default: bottom
	int esp_health_dock_ = 2; // default: left
	int esp_flags_dock_ = 3;	// default: right

	int esp_dragging_ = -1;													// -1=none, 0=name, 1=weapon, 2=health, 3=flags
	float esp_drag_ox_ = 0.0f, esp_drag_oy_ = 0.0f; // offset from element center
	bool esp_preview_hold_ = false;									// true while mouse is held inside preview

	enum class NotifyType
	{
		_SUCCESS,
		_INFO,
		_WARNING,
		_ERROR
	};
	struct Notification
	{
		std::string message;
		NotifyType type;
		float time;				// Time when created
		float expiration; // Remaining life time
		float anim;				// Animation progress (0.0 -> 1.0)
	};
	std::vector<Notification> notifications_;

	std::vector<std::string> spectator_names_;
	std::unordered_map<std::string, std::string> spectator_avatar_urls_; // Maps spectator name to avatar URL
	float bomb_time_total_ = 40.0f;
	float bomb_time_left_ = -1.0f; // negative = demo loop
	float bomb_predicted_damage_ = 75.0f;
	int player_health_ = 100;

	struct SideItem
	{
		const char* icon;
		const char* label;
	};

	void add_notification(const char* text, NotifyType type);
	void render_notifications();
};
} // namespace menu
