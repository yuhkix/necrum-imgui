#pragma once
#include "widgets/ui_framework.h"
#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>

using namespace ui; // Support access to styled() and alpha_mul() functions

namespace esp_preview
{

// Enhanced ESP preview element structure with full customization
struct ESPElement
{
	const char* label;
	int* dock_pos;					// 0=TOP, 1=BOTTOM, 2=LEFT, 3=RIGHT
	float *anim_x, *anim_y; // Animated positions
	float* alpha;						// Element opacity animation
	float *color_hue, *color_sat, *color_val;
	bool enabled;
	ImU32 get_color() const;
};

// Dock anchor positions around a bounding box
struct DockAnchors
{
	ImVec2 top, bottom, left, right;
	ImVec2 center;
	float pad;
};

// Calculate dock anchors from bounding box
inline DockAnchors calc_dock_anchors(ImVec2 box_min, ImVec2 box_max, float pad = 6.0f)
{
	DockAnchors anchors;
	float cx = (box_min.x + box_max.x) * 0.5f;
	float cy = (box_min.y + box_max.y) * 0.5f;
	anchors.top = {cx, box_min.y - pad};
	anchors.bottom = {cx, box_max.y + pad};
	anchors.left = {box_min.x - pad, cy};
	anchors.right = {box_max.x + pad, cy};
	anchors.center = {cx, cy};
	anchors.pad = pad;
	return anchors;
}

// Get anchor point by index
inline ImVec2 get_anchor(const DockAnchors& anchors, int idx)
{
	switch (idx)
	{
	case 0:
		return anchors.top;
	case 1:
		return anchors.bottom;
	case 2:
		return anchors.left;
	case 3:
		return anchors.right;
	default:
		return anchors.center;
	}
}

// Draw enhanced dock indicator zones during drag
inline void draw_dock_zones(ImDrawList* dl, const DockAnchors& anchors, ImVec2 box_min, ImVec2 box_max, int hover_dock,
														ImU32 accent_col)
{
	float bw = box_max.x - box_min.x;
	float bh = box_max.y - box_min.y;
	float dz = 24.0f;

	struct Zone
	{
		ImVec2 min, max;
	};

	Zone zones[4] = {{{anchors.center.x - bw * 0.5f, anchors.top.y - dz}, {anchors.center.x + bw * 0.5f, box_min.y}},
									 {{anchors.center.x - bw * 0.5f, box_max.y}, {anchors.center.x + bw * 0.5f, anchors.bottom.y + dz}},
									 {{anchors.left.x - dz, box_min.y}, {box_min.x, box_max.y}},
									 {{box_max.x, box_min.y}, {anchors.right.x + dz, box_max.y}}};

	for (int i = 0; i < 4; i++)
	{
		float highlight = (i == hover_dock) ? 0.4f : 0.1f;
		ImU32 zc = styled(accent_col, highlight);

		dl->AddRectFilled(zones[i].min, zones[i].max, zc, 4.0f);
		dl->AddRect(zones[i].min, zones[i].max, styled(accent_col, highlight + 0.2f), 4.0f, 0, 1.5f);
	}
}

// Draw element with optional hover/drag highlight
inline void draw_element_text(ImDrawList* dl, const char* text, ImVec2 pos, ImU32 text_col, float alpha, int dock,
															bool highlighted = false, float highlight_alpha = 0.0f)
{
	ImU32 final_col = alpha_mul(text_col, alpha);
	ImU32 shadow_col = alpha_mul(styled(IM_COL32(0, 0, 0, 200)), alpha);

	ImVec2 text_sz = ImGui::CalcTextSize(text);

	// Shadow
	dl->AddText({pos.x + 1, pos.y + 1}, shadow_col, text);

	// Main text
	dl->AddText(pos, final_col, text);

	// Highlight border when dragging or hovering
	if (highlighted && highlight_alpha > 0.001f)
	{
		ImU32 outline = styled(IM_COL32(100, 180, 255, 255), std::min(highlight_alpha, alpha));
		ImVec2 rect_min(pos.x - 3, pos.y - 2);
		ImVec2 rect_max(pos.x + text_sz.x + 3, pos.y + text_sz.y + 2);
		dl->AddRect(rect_min, rect_max, outline, 2.5f, 0, 1.5f);
	}
}

// Draw a health bar element with better styling
inline void draw_health_bar(ImDrawList* dl, ImVec2 pos, int dock,
														float health_norm, // 0.0 - 1.0
														ImU32 healthy_col, ImU32 damaged_col, float alpha, bool highlighted = false,
														float highlight_alpha = 0.0f)
{
	const float bar_w = 3.0f;
	const float bar_h = 40.0f;

	ImVec2 bar_min, bar_max;

	if (dock == 2) // LEFT
	{
		bar_min = {pos.x - bar_w - 2.0f, pos.y - bar_h * 0.5f};
		bar_max = {pos.x - 2.0f, pos.y + bar_h * 0.5f};
	}
	else if (dock == 3) // RIGHT
	{
		bar_min = {pos.x + 2.0f, pos.y - bar_h * 0.5f};
		bar_max = {pos.x + bar_w + 2.0f, pos.y + bar_h * 0.5f};
	}
	else if (dock == 0) // TOP
	{
		bar_min = {pos.x - bar_h * 0.5f, pos.y - bar_w - 2.0f};
		bar_max = {pos.x + bar_h * 0.5f, pos.y - 2.0f};
	}
	else // BOTTOM
	{
		bar_min = {pos.x - bar_h * 0.5f, pos.y + 2.0f};
		bar_max = {pos.x + bar_h * 0.5f, pos.y + bar_w + 2.0f};
	}

	// Background
	dl->AddRectFilled(bar_min, bar_max, alpha_mul(styled(IM_COL32(30, 30, 35, 200)), alpha), 1.5f);

	// Health fill
	ImVec2 fill_max = bar_min;
	if (dock == 2 || dock == 3)
	{
		fill_max.y = bar_min.y + (bar_max.y - bar_min.y) * health_norm;
	}
	else
	{
		fill_max.x = bar_min.x + (bar_max.x - bar_min.x) * health_norm;
	}

	ImU32 fill_col = health_norm > 0.5f ? healthy_col : damaged_col;
	dl->AddRectFilled(bar_min, fill_max, alpha_mul(fill_col, alpha), 1.5f);

	// Border
	dl->AddRect(bar_min, bar_max, alpha_mul(styled(IM_COL32(100, 100, 100, 150)), alpha), 1.5f);

	if (highlighted && highlight_alpha > 0.001f)
	{
		ImU32 outline = styled(IM_COL32(100, 180, 255, 255), std::min(highlight_alpha, alpha));
		ImVec2 outline_min(bar_min.x - 1, bar_min.y - 1);
		ImVec2 outline_max(bar_max.x + 1, bar_max.y + 1);
		dl->AddRect(outline_min, outline_max, outline, 2.0f, 0, 2.0f);
	}
}

// Advanced drag state tracker
struct DragState
{
	int dragging_element = -1;
	float drag_offset_x = 0.0f;
	float drag_offset_y = 0.0f;
	ImVec2 last_pos = {0, 0};

	void start_drag(int elem_idx, ImVec2 current_pos, ImVec2 element_pos)
	{
		dragging_element = elem_idx;
		drag_offset_x = current_pos.x - element_pos.x;
		drag_offset_y = current_pos.y - element_pos.y;
		last_pos = current_pos;
	}

	void update_drag(ImVec2 new_pos, float& out_x, float& out_y)
	{
		if (dragging_element >= 0)
		{
			out_x = new_pos.x - drag_offset_x;
			out_y = new_pos.y - drag_offset_y;
			last_pos = new_pos;
		}
	}

	void end_drag() { dragging_element = -1; }

	bool is_dragging(int idx) const { return dragging_element == idx; }
};
} // namespace esp_preview
