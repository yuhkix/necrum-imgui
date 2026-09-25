#pragma once

#include "necrum/core/base.h"

// HUD overlays: small floating cards (keybind list, status panels, timers...)
// that stay on screen while the main window is closed. They can be dragged
// while overlays are editable (the shell enables this while it is open).
namespace nc
{

struct OverlayOptions
{
	Corner anchor = Corner::TopLeft; // initial placement when `position` is unset
	ImVec2 position{-1.0f, -1.0f};	 // explicit initial position (screen space)
	ImVec2 pivot{0.0f, 0.0f};				 // pivot for `position`
	float margin = 18.0f;
	float width = 270.0f;
	const char* badge = nullptr; // small pill in the header, e.g. "2 active"
	ImU32 badge_color = 0;			 // 0 = theme badge color
	bool draggable = true;
};

// Returns true when the overlay is (at least partially) visible; only then call end_overlay().
// `visible` drives a fade in/out animation.
bool begin_overlay(const char* title, bool visible, const OverlayOptions& options = {});
void end_overlay();

// Content helpers
void overlay_row(const char* label, const char* value = nullptr, bool active = true);
void overlay_text(const char* text);

// Overlays can be moved only while editable.
void set_overlays_editable(bool editable);
bool overlays_editable();

// Built-in overlay listing every tracked nc::Keybind (see nc::keybinds::track).
void keybind_overlay(bool visible, const OverlayOptions& options = {});

} // namespace nc
