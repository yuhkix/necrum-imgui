#pragma once

#include "base.h"

// Frame-rate independent animation helpers.
//
// Animated values are keyed by ImGuiID and live in a single store owned by the
// framework. Entries that are not touched for a while are garbage collected,
// so widgets can freely create per-item animation state.
namespace nc::anim
{

// Global speed multiplier (higher = snappier). Default 20.
float speed();
void set_speed(float s);

// Delta time of the current frame, clamped to a sane range.
float dt();

// Exponential approach of `cur` towards `target`. speed < 0 uses the global speed.
float approach(float cur, float target, float speed = -1.0f);

// Persistent float for `id`. `init` is used only when the entry is created.
float& value(ImGuiID id, float init = 0.0f);

// Fetches the value for `id` and moves it towards `target` (initialised to target on first use).
float animate(ImGuiID id, float target, float speed = -1.0f);
inline float animate(ImGuiID id, bool on, float speed = -1.0f)
{
	return animate(id, on ? 1.0f : 0.0f, speed);
}

// Derives a stable sub-key, e.g. key(id, "hover").
ImGuiID key(ImGuiID base, const char* salt);

// Easing curves, t in [0, 1].
float smoothstep(float t);
float smoothstep(float edge0, float edge1, float x);
float ease_out_cubic(float t);
float ease_in_out_cubic(float t);
float ease_out_back(float t);

// Called by nc::new_frame(). Advances time and garbage-collects stale entries.
void new_frame(float delta_time);

} // namespace nc::anim
