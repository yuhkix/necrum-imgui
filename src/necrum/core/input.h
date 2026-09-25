#pragma once

#include "base.h"

namespace nc
{

enum class KeybindMode : int
{
	Toggle = 0, // press flips the state
	Hold = 1,		// active while held
	Always = 2, // always active, key ignored
	Off = 3,		// never active
};

const char* keybind_mode_name(KeybindMode mode);

// A user-assignable key binding. Uses ImGuiKey so it works with every
// platform backend (keyboard, mouse buttons and gamepad keys).
struct Keybind
{
	ImGuiKey key = ImGuiKey_None;
	KeybindMode mode = KeybindMode::Toggle;
	bool toggled = false; // state for Toggle mode

	Keybind() = default;
	Keybind(ImGuiKey k, KeybindMode m = KeybindMode::Toggle) : key(k), mode(m) {}

	// Evaluates the binding for the current frame (see nc::keybinds::update).
	bool active() const;
	bool bound() const { return key != ImGuiKey_None || mode == KeybindMode::Always; }
};

// Human readable key name ("NONE", "M4", "F5", "INS", ...).
const char* key_name(ImGuiKey key);

namespace keybinds
{
// Registers a binding so overlays (e.g. nc::keybind_overlay) can list it.
// The pointer must stay valid until untrack() or program exit.
void track(Keybind* bind, const char* display_name);
void untrack(Keybind* bind);

struct Tracked
{
	Keybind* bind;
	std::string name;
};
const std::vector<Tracked>& tracked();

// Keys ignored while a keybind widget is listening (e.g. the menu toggle key).
void set_reserved_keys(std::initializer_list<ImGuiKey> keys);
bool is_reserved(ImGuiKey key);

// Updates toggle states of all tracked binds. Called by nc::new_frame().
void update();

// Updates a single (possibly untracked) bind. Idempotent within a frame.
void update(Keybind& bind);
} // namespace keybinds

} // namespace nc
