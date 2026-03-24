#include "menu/esp_preview_helper.h"
#include "menu/menu.h"

namespace menu

{

using namespace ui;
using namespace esp_preview;

void Menu::page_visuals()

{

	ImDrawList* dl = ImGui::GetWindowDrawList();

	ImVec2 cur = ImGui::GetCursorScreenPos();

	float aw = ImGui::GetContentRegionAvail().x;

	float ah = ImGui::GetContentRegionAvail().y;

	float pad = 16.0f, gap = 16.0f;

	float pw = (aw - pad * 2 - gap) * 0.5f;

	float ph = ah - pad;

	float th = 30.0f;

	float& a_box = ganim(ImGui::GetID("##esp_box_a"), esp_box_ ? 1.0f : 0.0f);

	float& a_name = ganim(ImGui::GetID("##esp_name_a"), esp_name_ ? 1.0f : 0.0f);

	float& a_weapon =

			ganim(ImGui::GetID("##esp_wpn_a"), esp_weapon_ ? 1.0f : 0.0f);

	float& a_health =

			ganim(ImGui::GetID("##esp_hp_a"), esp_health_bar_ ? 1.0f : 0.0f);

	float& a_skel =

			ganim(ImGui::GetID("##esp_sk_a"), esp_skeleton_ ? 1.0f : 0.0f);

	float& a_snap =

			ganim(ImGui::GetID("##esp_sn_a"), esp_snaplines_ ? 1.0f : 0.0f);

	float& a_flags = ganim(ImGui::GetID("##esp_fl_a"), esp_flags_ ? 1.0f : 0.0f);

	float& a_corner =

			ganim(ImGui::GetID("##esp_cn_a"), (esp_box_style_ == 1) ? 1.0f : 0.0f);

	a_box = alerp(a_box, esp_box_ ? 1.0f : 0.0f);

	a_name = alerp(a_name, esp_name_ ? 1.0f : 0.0f);

	a_weapon = alerp(a_weapon, esp_weapon_ ? 1.0f : 0.0f);

	a_health = alerp(a_health, esp_health_bar_ ? 1.0f : 0.0f);

	a_skel = alerp(a_skel, esp_skeleton_ ? 1.0f : 0.0f);

	a_snap = alerp(a_snap, esp_snaplines_ ? 1.0f : 0.0f);

	a_flags = alerp(a_flags, esp_flags_ ? 1.0f : 0.0f);

	a_corner = alerp(a_corner, (esp_box_style_ == 1) ? 1.0f : 0.0f);

	ImVec2 lp(cur.x + pad, cur.y);

	ImVec2 lp_max(lp.x + pw, lp.y + ph);

	if (true)
	{
		draw_panel(dl, lp, lp_max, "ESP Preview");

		float preview_x = lp.x + 2;

		float preview_y = lp.y + 27;

		float preview_w = pw - 4;

		float preview_h = ph - 29;

		ImGui::SetCursorScreenPos(ImVec2(preview_x, preview_y));

		ImGui::InvisibleButton("##esp_preview_hit", ImVec2(preview_w, preview_h),

													 ImGuiButtonFlags_MouseButtonLeft);

		bool preview_active = ImGui::IsItemActive();

		esp_preview_hold_ = preview_active;

		dl->PushClipRect(ImVec2(preview_x, preview_y),

										 ImVec2(preview_x + preview_w, preview_y + preview_h), true);

		float ground_h = preview_h * 0.18f;

		ImVec2 ground_min(preview_x, preview_y + preview_h - ground_h);

		ImVec2 ground_max(preview_x + preview_w, preview_y + preview_h);

		dl->AddRectFilledMultiColor(

				ground_min, ground_max, styled(IM_COL32(0, 0, 0, 0)),

				styled(IM_COL32(0, 0, 0, 0)), styled(IM_COL32(14, 14, 18, 60)),

				styled(IM_COL32(14, 14, 18, 60)));

		// Subtle grid lines

		float grid_step = 24.0f;

		for (float gx = preview_x; gx < preview_x + preview_w; gx += grid_step)

			dl->AddLine(ImVec2(gx, preview_y), ImVec2(gx, preview_y + preview_h),

									styled(IM_COL32(255, 255, 255, 4)));

		for (float gy = preview_y; gy < preview_y + preview_h; gy += grid_step)

			dl->AddLine(ImVec2(preview_x, gy), ImVec2(preview_x + preview_w, gy),

									styled(IM_COL32(255, 255, 255, 4)));

		float time = (float)ImGui::GetTime();

		float bob = sinf(time * 1.8f) * 2.5f; // Gentle idle bob

		float body_h = preview_h * 0.54f;

		float body_w = body_h * 0.30f;

		float cx = preview_x + preview_w * 0.5f;

		float feet_y = preview_y + preview_h * 0.72f + bob;

		float top_y = feet_y - body_h;

		float head_r = body_w * 0.42f;
		float head_cy = top_y + head_r;

		dl->AddCircleFilled(ImVec2(cx, head_cy), head_r, styled(IM_COL32(48, 48, 54, 200)), 16);
		dl->AddCircle(ImVec2(cx, head_cy), head_r, styled(IM_COL32(70, 70, 80, 160)), 16, 1.5f);

		dl->AddCircle(ImVec2(cx - head_r * 0.3f, head_cy - head_r * 0.2f), head_r * 0.15f,
									styled(IM_COL32(100, 100, 110, 60)), 12, 1.0f);

		// Neck

		float neck_top = head_cy + head_r;

		float neck_bot = neck_top + body_h * 0.04f;

		dl->AddRectFilled(ImVec2(cx - body_w * 0.12f, neck_top),

											ImVec2(cx + body_w * 0.12f, neck_bot),

											styled(IM_COL32(42, 42, 48, 200)));

		// Torso with improved shading
		float torso_top = neck_bot;
		float torso_bot = torso_top + body_h * 0.38f;
		float torso_w = body_w * 0.75f;
		ImVec2 torso_tl(cx - torso_w, torso_top);
		ImVec2 torso_br(cx + torso_w, torso_bot);

		// Main torso
		dl->AddRectFilled(torso_tl, torso_br, styled(IM_COL32(46, 46, 52, 210)), 2.0f);
		dl->AddRect(torso_tl, torso_br, styled(IM_COL32(65, 65, 75, 150)), 2.0f, 0, 1.5f);

		// Subtle center line for depth
		float center_line_x = cx;
		dl->AddLine(ImVec2(center_line_x, torso_top + 2), ImVec2(center_line_x, torso_bot - 2),
								styled(IM_COL32(35, 35, 40, 80)), 1.0f);

		// Hips / pelvis

		float hip_top = torso_bot;

		float hip_bot = hip_top + body_h * 0.08f;

		float hip_w = torso_w * 0.85f;

		dl->AddRectFilled(ImVec2(cx - hip_w, hip_top), ImVec2(cx + hip_w, hip_bot),

											styled(IM_COL32(38, 38, 44, 200)), 1.0f);

		// Arms with improved joints
		float arm_w = body_w * 0.22f;
		float upper_arm_len = body_h * 0.22f;
		float forearm_len = body_h * 0.20f;
		float shoulder_y = torso_top + body_h * 0.04f;

		// Left arm
		float l_shoulder_x = cx - torso_w;
		float l_elbow_x = l_shoulder_x - arm_w * 1.0f;
		float l_elbow_y = shoulder_y + upper_arm_len;
		float l_hand_x = l_elbow_x + arm_w * 0.3f;
		float l_hand_y = l_elbow_y + forearm_len;

		dl->AddLine(ImVec2(l_shoulder_x, shoulder_y), ImVec2(l_elbow_x, l_elbow_y), styled(IM_COL32(48, 48, 54, 230)),
								4.5f);
		dl->AddLine(ImVec2(l_elbow_x, l_elbow_y), ImVec2(l_hand_x, l_hand_y), styled(IM_COL32(46, 46, 52, 220)), 4.0f);

		// Joint highlights
		dl->AddCircleFilled(ImVec2(l_elbow_x, l_elbow_y), 2.0f, styled(IM_COL32(60, 60, 70, 120)), 8);

		// Right arm
		float r_shoulder_x = cx + torso_w;
		float r_elbow_x = r_shoulder_x + arm_w * 1.0f;
		float r_elbow_y = shoulder_y + upper_arm_len;
		float r_hand_x = r_elbow_x - arm_w * 0.3f;
		float r_hand_y = r_elbow_y + forearm_len;

		dl->AddLine(ImVec2(r_shoulder_x, shoulder_y), ImVec2(r_elbow_x, r_elbow_y), styled(IM_COL32(48, 48, 54, 230)),
								4.5f);
		dl->AddLine(ImVec2(r_elbow_x, r_elbow_y), ImVec2(r_hand_x, r_hand_y), styled(IM_COL32(46, 46, 52, 220)), 4.0f);

		// Joint highlights
		dl->AddCircleFilled(ImVec2(r_elbow_x, r_elbow_y), 2.0f, styled(IM_COL32(60, 60, 70, 120)), 8);

		// Legs with improved joints
		float leg_gap = hip_w * 0.35f;
		float upper_leg_len = body_h * 0.24f;
		float lower_leg_len = body_h * 0.22f;

		// Left leg
		float ll_hip_x = cx - leg_gap;
		float ll_knee_y = hip_bot + upper_leg_len;
		float ll_knee_x = ll_hip_x - body_w * 0.06f;
		float ll_foot_x = ll_knee_x + body_w * 0.04f;
		float ll_foot_y = ll_knee_y + lower_leg_len;

		dl->AddLine(ImVec2(ll_hip_x, hip_bot), ImVec2(ll_knee_x, ll_knee_y), styled(IM_COL32(46, 46, 52, 230)), 5.0f);
		dl->AddLine(ImVec2(ll_knee_x, ll_knee_y), ImVec2(ll_foot_x, ll_foot_y), styled(IM_COL32(44, 44, 50, 220)), 4.5f);

		// Knee joint
		dl->AddCircleFilled(ImVec2(ll_knee_x, ll_knee_y), 2.2f, styled(IM_COL32(60, 60, 70, 120)), 8);

		// Right leg
		float rl_hip_x = cx + leg_gap;
		float rl_knee_y = hip_bot + upper_leg_len;
		float rl_knee_x = rl_hip_x + body_w * 0.06f;
		float rl_foot_x = rl_knee_x - body_w * 0.04f;
		float rl_foot_y = rl_knee_y + lower_leg_len;

		dl->AddLine(ImVec2(rl_hip_x, hip_bot), ImVec2(rl_knee_x, rl_knee_y), styled(IM_COL32(46, 46, 52, 230)), 5.0f);
		dl->AddLine(ImVec2(rl_knee_x, rl_knee_y), ImVec2(rl_foot_x, rl_foot_y), styled(IM_COL32(44, 44, 50, 220)), 4.5f);

		// Knee joint
		dl->AddCircleFilled(ImVec2(rl_knee_x, rl_knee_y), 2.2f, styled(IM_COL32(60, 60, 70, 120)), 8);

		// Bounding box extents (padded around the silhouette)

		float box_pad_x = 10.0f;

		float box_pad_top = 6.0f;

		float box_pad_bot = 4.0f;

		float box_left = cx - torso_w - arm_w * 1.2f - box_pad_x;

		float box_right = cx + torso_w + arm_w * 1.2f + box_pad_x;

		float box_top = top_y - box_pad_top;

		float box_bot = std::max({ll_foot_y, rl_foot_y}) + box_pad_bot;

		float box_cx = (box_left + box_right) * 0.5f;

		float box_cy = (box_top + box_bot) * 0.5f;

		ImU32 box_col = alpha_mul(

				styled(hsv_to_u32(esp_box_hue_, esp_box_sat_, esp_box_val_)), a_box);

		if (a_box > 0.005f)

		{

			float corner_t = a_corner;

			float full_alpha = 1.0f - corner_t;

			float corner_alpha = corner_t;

			if (full_alpha > 0.005f)

			{

				ImU32 box_2d = alpha_mul(box_col, full_alpha);

				dl->AddRect(ImVec2(box_left, box_top), ImVec2(box_right, box_bot), box_2d,

										0.0f, 0, 1.4f);
			}

			if (corner_alpha > 0.005f)

			{

				ImU32 box_cn = alpha_mul(box_col, corner_alpha);

				float bw = box_right - box_left, bh = box_bot - box_top;

				float cl = bw * 0.22f, cl_v = bh * 0.14f;

				float lt = 1.4f;

				dl->AddLine({box_left, box_top}, {box_left + cl, box_top}, box_cn, lt);

				dl->AddLine({box_left, box_top}, {box_left, box_top + cl_v}, box_cn, lt);

				dl->AddLine({box_right, box_top}, {box_right - cl, box_top}, box_cn, lt);

				dl->AddLine({box_right, box_top}, {box_right, box_top + cl_v}, box_cn,

										lt);

				dl->AddLine({box_left, box_bot}, {box_left + cl, box_bot}, box_cn, lt);

				dl->AddLine({box_left, box_bot}, {box_left, box_bot - cl_v}, box_cn, lt);

				dl->AddLine({box_right, box_bot}, {box_right - cl, box_bot}, box_cn, lt);

				dl->AddLine({box_right, box_bot}, {box_right, box_bot - cl_v}, box_cn,

										lt);
			}
		}

		// Dock anchors: 0=TOP, 1=BOTTOM, 2=LEFT, 3=RIGHT

		// Each anchor has a center point where elements are placed

		float dock_margin = 5.0f;

		struct DockAnchor

		{

			float x, y;
		};

		DockAnchor anchors[4] = {

				{box_cx, box_top - dock_margin}, // TOP

				{box_cx, box_bot + dock_margin}, // BOTTOM

				{box_left - dock_margin, box_cy}, // LEFT

				{box_right + dock_margin, box_cy} // RIGHT

		};

		// Dock zone rects for snap detection (larger hit area)

		float dz = 22.0f;

		ImVec2 dock_zones_min[4], dock_zones_max[4];

		float bw_half = (box_right - box_left) * 0.5f;

		float bh_half = (box_bot - box_top) * 0.5f;

		dock_zones_min[0] = {box_cx - bw_half, box_top - dz - dock_margin};

		dock_zones_max[0] = {box_cx + bw_half, box_top};

		dock_zones_min[1] = {box_cx - bw_half, box_bot};

		dock_zones_max[1] = {box_cx + bw_half, box_bot + dz + dock_margin};

		dock_zones_min[2] = {box_left - dz - dock_margin, box_top};

		dock_zones_max[2] = {box_left, box_bot};

		dock_zones_min[3] = {box_right, box_top};

		dock_zones_max[3] = {box_right + dz + dock_margin, box_bot};

		ImVec2 mouse = ImGui::GetIO().MousePos;

		bool mouse_down = ImGui::GetIO().MouseDown[0];

		bool mouse_released = ImGui::IsMouseReleased(0);

		// Keep drag positions clamped within the preview box (with small padding).

		auto clamp_to_preview = [&](ImVec2 pos)

		{
			float pad = 4.0f;

			pos.x = ui::clamp(pos.x, preview_x + pad, preview_x + preview_w - pad);

			pos.y = ui::clamp(pos.y, preview_y + pad, preview_y + preview_h - pad);

			return pos;
		};

		// Helper: find nearest dock zone to mouse

		auto nearest_dock = [&]() -> int

		{
			float best_d = FLT_MAX;

			int best = 0;

			for (int d = 0; d < 4; d++)

			{

				float dx = mouse.x - anchors[d].x;

				float dy = mouse.y - anchors[d].y;

				float dist = dx * dx + dy * dy;

				if (dist < best_d)

				{

					best_d = dist;

					best = d;
				}
			}

			return best;
		};

		// Helper: check if mouse is within a rect (for initiating drag)

		auto in_rect = [](ImVec2 p, ImVec2 mn, ImVec2 mx)

		{ return p.x >= mn.x && p.x <= mx.x && p.y >= mn.y && p.y <= mx.y; };

		// Element info for drag system: {dock_ptr, alpha, enabled, label}

		struct EspElem

		{

			int* dock;

			float alpha;

			bool enabled;

			const char* label;
		};

		EspElem elems[4] = {

				{&esp_name_dock_, a_name, esp_name_, "Name"},

				{&esp_weapon_dock_, a_weapon, esp_weapon_, "Weapon"},

				{&esp_health_dock_, a_health, esp_health_bar_, "Health"},

				{&esp_flags_dock_, a_flags, esp_flags_, "Flags"},

		};

		auto element_bounds =
				[&](int idx, ImVec2& mn, ImVec2& mx, float* anim_xs_local[], float* anim_ys_local[], float mouse_y)

		{
			float box_cy = (box_top + box_bot) * 0.5f;

			switch (idx)

			{

			case 0: // Name

			{

				ImVec2 sz = ImGui::CalcTextSize("Player");

				float nx = *anim_xs_local[0], ny = *anim_ys_local[0];

				int nd = *elems[0].dock;

				float tx, ty;

				if (nd == 0) // top

				{

					tx = box_left + (box_right - box_left - sz.x) * 0.5f;

					ty = ny - sz.y;
				}

				else if (nd == 1) // bottom

				{

					tx = box_left + (box_right - box_left - sz.x) * 0.5f;

					ty = ny + 2.0f;
				}

				else if (nd == 2) // left

				{

					tx = nx - sz.x - 2.0f;

					ty = (mouse_y < box_cy) ? box_top + 2.0f : box_bot - sz.y - 2.0f;
				}

				else // right

				{

					tx = box_right + 4.0f;

					ty = (mouse_y < box_cy) ? box_top + 2.0f : box_bot - sz.y - 2.0f;
				}

				mn = {tx - 2.0f, ty - 1.0f};

				mx = {tx + sz.x + 2.0f, ty + sz.y + 1.0f};

				return;
			}

			case 1: // Weapon

			{

				ImVec2 sz = ImGui::CalcTextSize("AK-47");

				float wx = *anim_xs_local[1], wy = *anim_ys_local[1];

				int wd = *elems[1].dock;

				float tx, ty;

				if (wd == 0)

				{

					tx = box_left + (box_right - box_left - sz.x) * 0.5f;

					ty = wy - sz.y;
				}

				else if (wd == 1)

				{

					tx = box_left + (box_right - box_left - sz.x) * 0.5f;

					ty = wy + 2.0f;
				}

				else if (wd == 2)

				{

					tx = wx - sz.x - 2.0f;

					ty = (mouse_y < box_cy) ? box_top + 18.0f : box_bot - sz.y - 6.0f;
				}

				else

				{

					tx = box_right + 4.0f;

					ty = (mouse_y < box_cy) ? box_top + 18.0f : box_bot - sz.y - 6.0f;
				}

				mn = {tx - 2.0f, ty - 1.0f};

				mx = {tx + sz.x + 2.0f, ty + sz.y + 1.0f};

				return;
			}

			case 2: // Health

			{

				int hd = *elems[2].dock;

				if (hd == 2 || hd == 3)

				{

					float bar_w = 3.0f;

					float bx = (hd == 2) ? (*anim_xs_local[2] - bar_w - 1.0f) : (*anim_xs_local[2] + 2.0f);

					mn = {bx - 1.0f, box_top - 1.0f};

					mx = {bx + bar_w + 1.0f, box_bot + 1.0f};
				}

				else

				{

					float bar_h = 3.0f;

					float by = (hd == 0) ? (*anim_ys_local[2] - bar_h - 1.0f) : (*anim_ys_local[2] + 2.0f);

					mn = {box_left - 1.0f, by - 1.0f};

					mx = {box_right + 1.0f, by + bar_h + 1.0f};
				}

				return;
			}

			case 3: // Flags

			{

				const char* flag_texts[] = {"HK", "ZOOM", "ARMOR"};

				int fd = *elems[3].dock;

				float line_h = 12.0f;

				if (fd == 2 || fd == 3)

				{

					float max_w = 0.0f;

					for (auto t : flag_texts)

						max_w = std::max(max_w, ImGui::CalcTextSize(t).x);

					float fx = (fd == 3) ? *anim_xs_local[3] + 4.0f : *anim_xs_local[3];

					float fy = box_top;

					mn = {(fd == 2) ? fx - max_w - 4.0f : fx - 2.0f, fy - 2.0f};

					mx = {(fd == 2) ? fx + 2.0f : fx + max_w + 6.0f,

								fy + line_h * 3.0f + 2.0f};
				}

				else

				{

					float start_x = box_left;

					float fty = (fd == 0) ? *anim_ys_local[3] - line_h - 1.0f : *anim_ys_local[3] + 3.0f;

					float total_w = 0.0f;

					for (int i = 0; i < 3; i++)

						total_w += ImGui::CalcTextSize(flag_texts[i]).x + (i == 2 ? 0.0f : 6.0f);

					mn = {start_x - 2.0f, fty - 2.0f};

					mx = {start_x + total_w + 2.0f, fty + line_h + 2.0f};
				}

				return;
			}

			default:

				mn = mx = ImVec2(0, 0);
			}
		};

		// Animated dock positions (smooth transition between docks)

		float& anim_name_x = ganim(ImGui::GetID("##dn_x"), anchors[esp_name_dock_].x);

		float& anim_name_y = ganim(ImGui::GetID("##dn_y"), anchors[esp_name_dock_].y);

		float& anim_wpn_x =

				ganim(ImGui::GetID("##dw_x"), anchors[esp_weapon_dock_].x);

		float& anim_wpn_y =

				ganim(ImGui::GetID("##dw_y"), anchors[esp_weapon_dock_].y);

		float& anim_hp_x = ganim(ImGui::GetID("##dh_x"), anchors[esp_health_dock_].x);

		float& anim_hp_y = ganim(ImGui::GetID("##dh_y"), anchors[esp_health_dock_].y);

		float& anim_fl_x = ganim(ImGui::GetID("##df_x"), anchors[esp_flags_dock_].x);

		float& anim_fl_y = ganim(ImGui::GetID("##df_y"), anchors[esp_flags_dock_].y);

		float* anim_xs[4] = {&anim_name_x, &anim_wpn_x, &anim_hp_x, &anim_fl_x};

		float* anim_ys[4] = {&anim_name_y, &anim_wpn_y, &anim_hp_y, &anim_fl_y};

		// Update animated positions (lerp toward target dock)

		for (int i = 0; i < 4; i++)

		{

			if (esp_dragging_ != i)

			{

				*anim_xs[i] = alerp(*anim_xs[i], anchors[*elems[i].dock].x);

				*anim_ys[i] = alerp(*anim_ys[i], anchors[*elems[i].dock].y);
			}
		}

		if (esp_dragging_ >= 0)

		{

			int hover_dock = nearest_dock();

			for (int d = 0; d < 4; d++)

			{

				float highlight = (d == hover_dock) ? 0.35f : 0.08f;

				ImU32 zc = styled(col_accent, highlight);

				dl->AddRectFilled(dock_zones_min[d], dock_zones_max[d], zc, 3.0f);

				dl->AddRect(dock_zones_min[d], dock_zones_max[d],

										styled(col_accent, highlight + 0.15f), 3.0f, 0, 1.0f);
			}
		}

		// We compute hit rects per element for drag initiation

		// Helper: compute element hit rect at its animated position

		auto elem_hit_rect = [&](int idx, ImVec2* out_min, ImVec2* out_max)

		{
			float ex = *anim_xs[idx], ey = *anim_ys[idx];

			int dock = *elems[idx].dock;

			float hw, hh;

			if (dock <= 1)

			{

				hw = 28.0f;

				hh = 10.0f;

			} // TOP/BOTTOM: wide

			else

			{

				hw = 8.0f;

				hh = 30.0f;

			} // LEFT/RIGHT: tall

			*out_min = {ex - hw, ey - hh};

			*out_max = {ex + hw, ey + hh};
		};

		// Hover detection for elements

		int esp_hovered = -1;

		for (int i = 0; i < 4; i++)

		{

			if (elems[i].alpha < 0.01f)

				continue;

			ImVec2 hmn, hmx;

			element_bounds(i, hmn, hmx, anim_xs, anim_ys, mouse.y);

			if (in_rect(mouse, hmn, hmx) && esp_dragging_ < 0)

			{

				esp_hovered = i;

				break;
			}
		}

		// Animated hover states per element

		float& hov_name = ganim(ImGui::GetID("##eh0"), 0.0f);

		float& hov_wpn = ganim(ImGui::GetID("##eh1"), 0.0f);

		float& hov_hp = ganim(ImGui::GetID("##eh2"), 0.0f);

		float& hov_fl = ganim(ImGui::GetID("##eh3"), 0.0f);

		hov_name = alerp(hov_name, (esp_hovered == 0) ? 1.0f : 0.0f);

		hov_wpn = alerp(hov_wpn, (esp_hovered == 1) ? 1.0f : 0.0f);

		hov_hp = alerp(hov_hp, (esp_hovered == 2) ? 1.0f : 0.0f);

		hov_fl = alerp(hov_fl, (esp_hovered == 3) ? 1.0f : 0.0f);

		float* hov_anims[4] = {&hov_name, &hov_wpn, &hov_hp, &hov_fl};

		// Handle drag initiation (check clicks on element hit rects)

		if (esp_dragging_ < 0 && ImGui::IsMouseClicked(0))

		{

			for (int i = 0; i < 4; i++)

			{

				if (elems[i].alpha < 0.01f)

					continue;

				ImVec2 hmn, hmx;

				element_bounds(i, hmn, hmx, anim_xs, anim_ys, mouse.y);

				if (in_rect(mouse, hmn, hmx))

				{

					esp_dragging_ = i;

					esp_drag_ox_ = mouse.x - *anim_xs[i];

					esp_drag_oy_ = mouse.y - *anim_ys[i];

					break;
				}
			}
		}

		// Update dragged element position

		if (esp_dragging_ >= 0 && mouse_down)

		{

			ImVec2 p = clamp_to_preview(ImVec2(mouse.x - esp_drag_ox_, mouse.y - esp_drag_oy_));

			*anim_xs[esp_dragging_] = p.x;

			*anim_ys[esp_dragging_] = p.y;

			esp_preview_hold_ = true;
		}

		// Snap to dock on release

		if (esp_dragging_ >= 0 && mouse_released)

		{

			int new_dock = nearest_dock();

			// If another element already occupies this dock, swap them

			for (int i = 0; i < 4; i++)

			{

				if (i != esp_dragging_ && *elems[i].dock == new_dock)

				{

					*elems[i].dock = *elems[esp_dragging_].dock; // swap

					break;
				}
			}

			*elems[esp_dragging_].dock = new_dock;

			esp_dragging_ = -1;
		}

		if (a_name > 0.005f)

		{

			const char* name_text = "Player";

			ImVec2 name_sz = ImGui::CalcTextSize(name_text);

			float nx = *anim_xs[0], ny = *anim_ys[0];

			int nd = *elems[0].dock;

			float tx, ty;

			if (nd == 0)

			{

				tx = box_left + (box_right - box_left - name_sz.x) * 0.5f;

				ty = ny - name_sz.y;
			}

			else if (nd == 1)

			{

				tx = box_left + (box_right - box_left - name_sz.x) * 0.5f;

				ty = ny + 2.0f;
			}

			else if (nd == 2)

			{

				tx = nx - name_sz.x - 2.0f;

				ty = (mouse.y < (box_top + box_bot) * 0.5f) ? box_top + 2.0f

																										: box_bot - name_sz.y - 2.0f;
			}

			else

			{

				tx = box_right + 4.0f;

				ty = (mouse.y < (box_top + box_bot) * 0.5f) ? box_top + 2.0f

																										: box_bot - name_sz.y - 2.0f;
			}

			ImU32 name_col = alpha_mul(

					styled(hsv_to_u32(esp_name_hue_, esp_name_sat_, esp_name_val_)),

					a_name);

			dl->AddText({tx + 1, ty + 1},

									alpha_mul(styled(IM_COL32(0, 0, 0, 200)), a_name), name_text);

			dl->AddText({tx, ty}, name_col, name_text);

			if (esp_dragging_ == 0 || *hov_anims[0] > 0.01f)

			{

				float ha = (esp_dragging_ == 0) ? 0.5f : *hov_anims[0] * 0.3f;

				dl->AddRect({tx - 2, ty - 1}, {tx + name_sz.x + 2, ty + name_sz.y + 1},

										styled(col_accent, ha), 2.0f);
			}
		}

		if (a_weapon > 0.005f)

		{

			const char* wpn_text = "AK-47";

			ImVec2 wpn_sz = ImGui::CalcTextSize(wpn_text);

			float wx = *anim_xs[1], wy = *anim_ys[1];

			int wd = *elems[1].dock;

			float tx, ty;

			if (wd == 0)

			{

				tx = box_left + (box_right - box_left - wpn_sz.x) * 0.5f;

				ty = wy - wpn_sz.y;
			}

			else if (wd == 1)

			{

				tx = box_left + (box_right - box_left - wpn_sz.x) * 0.5f;

				ty = wy + 2.0f;
			}

			else if (wd == 2)

			{

				tx = wx - wpn_sz.x - 2.0f;

				ty = (mouse.y < (box_top + box_bot) * 0.5f) ? box_top + 18.0f

																										: box_bot - wpn_sz.y - 6.0f;
			}

			else

			{

				tx = box_right + 4.0f;

				ty = (mouse.y < (box_top + box_bot) * 0.5f) ? box_top + 18.0f

																										: box_bot - wpn_sz.y - 6.0f;
			}

			ImU32 wpn_col = alpha_mul(styled(IM_COL32(160, 160, 160, 255)), a_weapon);

			dl->AddText({tx + 1, ty + 1},

									alpha_mul(styled(IM_COL32(0, 0, 0, 180)), a_weapon), wpn_text);

			dl->AddText({tx, ty}, wpn_col, wpn_text);

			if (esp_dragging_ == 1 || *hov_anims[1] > 0.01f)

			{

				float ha = (esp_dragging_ == 1) ? 0.5f : *hov_anims[1] * 0.3f;

				dl->AddRect({tx - 2, ty - 1}, {tx + wpn_sz.x + 2, ty + wpn_sz.y + 1},

										styled(col_accent, ha), 2.0f);
			}
		}

		if (a_health > 0.005f)

		{

			float hx = *anim_xs[2], hy = *anim_ys[2];

			int hd = *elems[2].dock;

			float hb_h = box_bot - box_top;

			float health_pct = 0.72f + sinf(time * 0.4f) * 0.03f;

			ImU32 htop =

					alpha_mul(styled(hsv_to_u32(esp_health_top_hue_, esp_health_top_sat_,

																			esp_health_top_val_)),

										a_health);

			ImU32 hbot =

					alpha_mul(styled(hsv_to_u32(esp_health_bot_hue_, esp_health_bot_sat_,

																			esp_health_bot_val_)),

										a_health);

			if (hd == 2 || hd == 3)

			{

				// Vertical bar

				float bar_w = 3.0f;

				float bx = (hd == 2) ? hx - bar_w - 1.0f : hx + 2.0f;

				float bt = box_top, bb = box_bot;

				dl->AddRectFilled({bx - 1, bt - 1}, {bx + bar_w + 1, bb + 1},

													alpha_mul(styled(IM_COL32(0, 0, 0, 180)), a_health));

				float ft = bb - hb_h * health_pct;

				dl->AddRectFilledMultiColor({bx, ft}, {bx + bar_w, bb}, htop, htop, hbot,

																		hbot);
			}

			else

			{

				// Horizontal bar (when docked top/bottom)

				float bar_h = 3.0f;

				float bar_w = box_right - box_left;

				float by = (hd == 0) ? hy - bar_h - 1.0f : hy + 2.0f;

				dl->AddRectFilled({box_left - 1, by - 1}, {box_right + 1, by + bar_h + 1},

													alpha_mul(styled(IM_COL32(0, 0, 0, 180)), a_health));

				float fw = bar_w * health_pct;

				dl->AddRectFilledMultiColor({box_left, by}, {box_left + fw, by + bar_h},

																		hbot, htop, htop, hbot);
			}

			if (esp_dragging_ == 2 || *hov_anims[2] > 0.01f)

			{

				float ha = (esp_dragging_ == 2) ? 0.5f : *hov_anims[2] * 0.3f;

				ImVec2 hmn, hmx;

				element_bounds(2, hmn, hmx, anim_xs, anim_ys, mouse.y);

				dl->AddRect(hmn, hmx, styled(col_accent, ha), 2.0f);
			}
		}

		if (a_flags > 0.005f)

		{

			const char* flag_texts[] = {"HK", "ZOOM", "ARMOR"};

			float fx = *anim_xs[3], fy = *anim_ys[3];

			int fd = *elems[3].dock;

			float flag_line_h = 12.0f;

			if (fd == 2 || fd == 3)

			{

				// Vertical stack

				float start_x = (fd == 3) ? fx + 4.0f : fx;

				float start_y = box_top;

				for (int fi = 0; fi < 3; fi++)

				{

					ImVec2 fsz = ImGui::CalcTextSize(flag_texts[fi]);

					float ftx = (fd == 2) ? start_x - fsz.x - 2.0f : start_x;

					dl->AddText({ftx + 1, start_y + 1},

											alpha_mul(styled(IM_COL32(0, 0, 0, 160)), a_flags),

											flag_texts[fi]);

					dl->AddText({ftx, start_y},

											alpha_mul(styled(IM_COL32(170, 170, 170, 255)), a_flags),

											flag_texts[fi]);

					start_y += flag_line_h;
				}
			}

			else

			{

				// Horizontal layout

				float start_x = box_left;

				float fty = (fd == 0) ? fy - flag_line_h - 1.0f : fy + 3.0f;

				for (int fi = 0; fi < 3; fi++)

				{

					ImVec2 fsz = ImGui::CalcTextSize(flag_texts[fi]);

					dl->AddText({start_x + 1, fty + 1},

											alpha_mul(styled(IM_COL32(0, 0, 0, 160)), a_flags),

											flag_texts[fi]);

					dl->AddText({start_x, fty},

											alpha_mul(styled(IM_COL32(170, 170, 170, 255)), a_flags),

											flag_texts[fi]);

					start_x += fsz.x + 6.0f;
				}
			}

			if (esp_dragging_ == 3 || *hov_anims[3] > 0.01f)

			{

				float ha = (esp_dragging_ == 3) ? 0.5f : *hov_anims[3] * 0.3f;

				ImVec2 hmn, hmx;

				element_bounds(3, hmn, hmx, anim_xs, anim_ys, mouse.y);

				dl->AddRect(hmn, hmx, styled(col_accent, ha), 2.0f);
			}
		}

		// Skeleton overlay (not dockable, stays on body)

		if (a_skel > 0.005f)

		{

			ImU32 skel_col =

					alpha_mul(styled(hsv_to_u32(esp_skeleton_hue_, esp_skeleton_sat_,

																			esp_skeleton_val_)),

										a_skel);

			float st = 1.2f;

			dl->AddLine({cx, head_cy}, {cx, neck_bot}, skel_col, st);

			dl->AddLine({cx, neck_bot}, {cx, (torso_top + torso_bot) * 0.5f}, skel_col,

									st);

			dl->AddLine({cx, (torso_top + torso_bot) * 0.5f}, {cx, hip_bot}, skel_col,

									st);

			dl->AddLine({l_shoulder_x, shoulder_y}, {l_elbow_x, l_elbow_y}, skel_col,

									st);

			dl->AddLine({l_elbow_x, l_elbow_y}, {l_hand_x, l_hand_y}, skel_col, st);

			dl->AddLine({r_shoulder_x, shoulder_y}, {r_elbow_x, r_elbow_y}, skel_col,

									st);

			dl->AddLine({r_elbow_x, r_elbow_y}, {r_hand_x, r_hand_y}, skel_col, st);

			dl->AddLine({ll_hip_x, hip_bot}, {ll_knee_x, ll_knee_y}, skel_col, st);

			dl->AddLine({ll_knee_x, ll_knee_y}, {ll_foot_x, ll_foot_y}, skel_col, st);

			dl->AddLine({rl_hip_x, hip_bot}, {rl_knee_x, rl_knee_y}, skel_col, st);

			dl->AddLine({rl_knee_x, rl_knee_y}, {rl_foot_x, rl_foot_y}, skel_col, st);

			ImU32 jc = alpha_mul(styled(col_accent), a_skel * 0.7f);

			float jr = 2.0f;

			dl->AddCircleFilled({cx, head_cy}, jr, jc, 8);

			dl->AddCircleFilled({cx, neck_bot}, jr, jc, 8);

			dl->AddCircleFilled({l_shoulder_x, shoulder_y}, jr, jc, 8);

			dl->AddCircleFilled({r_shoulder_x, shoulder_y}, jr, jc, 8);

			dl->AddCircleFilled({l_elbow_x, l_elbow_y}, jr, jc, 8);

			dl->AddCircleFilled({r_elbow_x, r_elbow_y}, jr, jc, 8);

			dl->AddCircleFilled({l_hand_x, l_hand_y}, jr, jc, 8);

			dl->AddCircleFilled({r_hand_x, r_hand_y}, jr, jc, 8);

			dl->AddCircleFilled({cx, hip_bot}, jr, jc, 8);

			dl->AddCircleFilled({ll_hip_x, hip_bot}, jr, jc, 8);

			dl->AddCircleFilled({rl_hip_x, hip_bot}, jr, jc, 8);

			dl->AddCircleFilled({ll_knee_x, ll_knee_y}, jr, jc, 8);

			dl->AddCircleFilled({rl_knee_x, rl_knee_y}, jr, jc, 8);

			dl->AddCircleFilled({ll_foot_x, ll_foot_y}, jr, jc, 8);

			dl->AddCircleFilled({rl_foot_x, rl_foot_y}, jr, jc, 8);
		}

		// Snap lines (not dockable)

		if (a_snap > 0.005f)

		{

			ImU32 snap_col = alpha_mul(styled(col_accent, 0.6f), a_snap);

			dl->AddLine({preview_x + preview_w * 0.5f, preview_y + preview_h},

									{cx, box_bot}, snap_col, 1.0f);
		}

		// Dragging tooltip

		if (esp_dragging_ >= 0)

		{

			const char* drag_label = elems[esp_dragging_].label;

			ImVec2 tsz = ImGui::CalcTextSize(drag_label);

			dl->AddRectFilled({mouse.x + 12, mouse.y - 2},

												{mouse.x + 16 + tsz.x, mouse.y + tsz.y + 2},

												styled(IM_COL32(0, 0, 0, 200)), 3.0f);

			dl->AddText({mouse.x + 14, mouse.y}, styled(col_accent), drag_label);
		}

		dl->PopClipRect();
	}

	ImVec2 rp(lp.x + pw + gap, cur.y);
	if (true)
		draw_panel(dl, rp, ImVec2(rp.x + pw, rp.y + ph), "ESP Settings");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24, 16));
	if (true)
	{
		ImGui::SetCursorScreenPos(ImVec2(rp.x, rp.y + th));
		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
		ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, styled(col_groove_brd));
		ImGui::BeginChild("##esp_settings", ImVec2(pw, ph - th - 6), ImGuiChildFlags_AlwaysUseWindowPadding,
											ImGuiWindowFlags_None);
		ImGui::PushItemWidth(-1.0f);
	}

	{
		static const char* box_style_items[] = {"2D", "Corner"};
		bool any_right = false;
		auto can = [&](std::initializer_list<const char*> tags) { return ui::search_match(tags); };
		bool filtering = ui::has_search_query();

		bool show_box =
				!filtering || can({"bounding box", "box"}) || can({"box style", "style"}) || can({"box color", "color"});
		bool show_box_style = !filtering || can({"box style", "style"});
		bool show_box_color = !filtering || can({"box color", "color"});

		if (show_box)
		{
			flat_checkbox("Bounding Box", &esp_box_);
			any_right = true;
			if (esp_box_)
			{

				{
					ctrl_label("Box Style");
					flat_dropdown_single("##bstyle", &esp_box_style_, box_style_items, 2);
				}
				if (show_box_color)
				{
					flat_color_picker("Box Color##boxc", &esp_box_hue_, &esp_box_sat_, &esp_box_val_, 0.0f, 0.74f, 0.78f);
				}
			}
		}

		bool show_health = !filtering || can({"health bar", "health"}) || can({"health top"}) || can({"health bottom"});
		bool show_health_top = !filtering || can({"health top", "health color"});
		bool show_health_bottom = !filtering || can({"health bottom", "health color"});

		if (show_health)
		{

			flat_checkbox("Health Bar", &esp_health_bar_);
			any_right = true;
			if (esp_health_bar_)
			{
				if (show_health_top)
				{
					flat_color_picker("Health Top##htop", &esp_health_top_hue_, &esp_health_top_sat_, &esp_health_top_val_,
														120.0f, 0.7f, 0.8f);
				}
				if (show_health_bottom)
				{
					flat_color_picker("Health Bottom##hbot", &esp_health_bot_hue_, &esp_health_bot_sat_, &esp_health_bot_val_,
														0.0f, 0.7f, 0.8f);
				}
			}
		}

		bool show_name = !filtering || can({"name", "esp name"}) || can({"name color", "name"});
		bool show_name_color = !filtering || can({"name color", "color"});

		if (show_name)
		{

			flat_checkbox("Name", &esp_name_);
			any_right = true;
			if (esp_name_ && show_name_color)
			{
				flat_color_picker("Name Color##namc", &esp_name_hue_, &esp_name_sat_, &esp_name_val_, 0.0f, 0.0f, 0.91f);
			}
		}

		if (!filtering || can({"weapon", "esp weapon"}))
		{

			flat_checkbox("Weapon", &esp_weapon_);
			any_right = true;
		}

		bool show_skel = !filtering || can({"skeleton"}) || can({"skeleton color", "skeleton"});
		bool show_skel_color = !filtering || can({"skeleton color", "color"});

		if (show_skel)
		{

			flat_checkbox("Skeleton", &esp_skeleton_);
			any_right = true;
			if (esp_skeleton_ && show_skel_color)
			{
				flat_color_picker("Skeleton Color##skelc", &esp_skeleton_hue_, &esp_skeleton_sat_, &esp_skeleton_val_, 0.0f,
													0.0f, 0.65f);
			}
		}

		if (!filtering || can({"snap lines", "snapline"}))
		{

			flat_checkbox("Snap Lines", &esp_snaplines_);
			any_right = true;
		}

		if (!filtering || can({"flags", "esp flags"}))
		{

			flat_checkbox("Flags", &esp_flags_);
			any_right = true;
		}

		if (!any_right && ui::has_search_query())
			draw_search_empty_hint("No matches in this section.");
	}

	if (true)
	{
		ImGui::PopItemWidth();
		ImGui::EndChild();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
	}
	ImGui::PopStyleVar(); // WindowPadding
}
} // namespace menu
