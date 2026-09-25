#include "input.h"

#include <unordered_map>

namespace nc
{

namespace
{
// Registries are intentionally leaked: binds may be untracked from static
// destructors that run after these would otherwise have been destroyed.
std::vector<keybinds::Tracked>& tracked_list()
{
	static auto* list = new std::vector<keybinds::Tracked>();
	return *list;
}

std::vector<ImGuiKey>& reserved_list()
{
	static auto* list = new std::vector<ImGuiKey>{ImGuiKey_Insert};
	return *list;
}

// Last frame each bind was updated, so update() is safe to call more than once.
std::unordered_map<const Keybind*, int>& update_frames()
{
	static auto* frames = new std::unordered_map<const Keybind*, int>();
	return *frames;
}
} // namespace

const char* keybind_mode_name(KeybindMode mode)
{
	switch (mode)
	{
	case KeybindMode::Toggle:
		return "Toggle";
	case KeybindMode::Hold:
		return "Hold";
	case KeybindMode::Always:
		return "Always";
	case KeybindMode::Off:
		return "Off";
	}
	return "?";
}

bool Keybind::active() const
{
	switch (mode)
	{
	case KeybindMode::Always:
		return true;
	case KeybindMode::Off:
		return false;
	case KeybindMode::Hold:
		return key != ImGuiKey_None && ImGui::GetCurrentContext() && ImGui::IsKeyDown(key);
	case KeybindMode::Toggle:
		return key != ImGuiKey_None && toggled;
	}
	return false;
}

const char* key_name(ImGuiKey key)
{
	switch (key)
	{
	case ImGuiKey_None:
		return "NONE";
	case ImGuiKey_MouseLeft:
		return "M1";
	case ImGuiKey_MouseRight:
		return "M2";
	case ImGuiKey_MouseMiddle:
		return "M3";
	case ImGuiKey_MouseX1:
		return "M4";
	case ImGuiKey_MouseX2:
		return "M5";
	case ImGuiKey_Insert:
		return "INS";
	case ImGuiKey_Delete:
		return "DEL";
	case ImGuiKey_PageUp:
		return "PGUP";
	case ImGuiKey_PageDown:
		return "PGDN";
	case ImGuiKey_Backspace:
		return "BACK";
	case ImGuiKey_Escape:
		return "ESC";
	case ImGuiKey_LeftCtrl:
		return "LCTRL";
	case ImGuiKey_RightCtrl:
		return "RCTRL";
	case ImGuiKey_LeftShift:
		return "LSHIFT";
	case ImGuiKey_RightShift:
		return "RSHIFT";
	case ImGuiKey_LeftAlt:
		return "LALT";
	case ImGuiKey_RightAlt:
		return "RALT";
	case ImGuiKey_CapsLock:
		return "CAPS";
	default:
		break;
	}
	if (key >= ImGuiKey_NamedKey_BEGIN && key < ImGuiKey_NamedKey_END)
		return ImGui::GetKeyName(key);
	return "?";
}

namespace keybinds
{

void track(Keybind* bind, const char* display_name)
{
	if (!bind)
		return;
	for (auto& t : tracked_list())
	{
		if (t.bind == bind)
		{
			t.name = display_name ? display_name : "";
			return;
		}
	}
	tracked_list().push_back({bind, display_name ? display_name : ""});
}

void untrack(Keybind* bind)
{
	auto& list = tracked_list();
	list.erase(std::remove_if(list.begin(), list.end(), [bind](const Tracked& t) { return t.bind == bind; }), list.end());
	update_frames().erase(bind);
}

const std::vector<Tracked>& tracked()
{
	return tracked_list();
}

void set_reserved_keys(std::initializer_list<ImGuiKey> keys)
{
	reserved_list().assign(keys.begin(), keys.end());
}

bool is_reserved(ImGuiKey key)
{
	const auto& list = reserved_list();
	return std::find(list.begin(), list.end(), key) != list.end();
}

void update(Keybind& bind)
{
	if (!ImGui::GetCurrentContext())
		return;
	int frame = ImGui::GetFrameCount();
	int& last = update_frames()[&bind];
	if (last == frame)
		return;
	last = frame;

	if (bind.mode == KeybindMode::Toggle && bind.key != ImGuiKey_None && ImGui::IsKeyPressed(bind.key, false))
		bind.toggled = !bind.toggled;
}

void update()
{
	for (auto& t : tracked_list())
		update(*t.bind);
}

} // namespace keybinds

} // namespace nc
