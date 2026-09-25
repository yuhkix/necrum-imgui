#include "internal.h"

namespace nc
{

using namespace detail;

namespace
{
ImGuiID g_listening = 0;

bool capturable(ImGuiKey key)
{
	if (key == ImGuiKey_MouseLeft || key == ImGuiKey_MouseWheelX || key == ImGuiKey_MouseWheelY)
		return false;
	if (key >= ImGuiKey_ReservedForModCtrl && key <= ImGuiKey_ReservedForModSuper)
		return false;
	return !keybinds::is_reserved(key);
}

// Draws "[ KEY ]" right-aligned on the current line and handles listening,
// the mode popup and key capture. `x`/`w` describe the clickable region.
bool keybind_body(ImGuiID id, Keybind* bind, ImVec2 pos, float w, float h, const char* caption)
{
	const Theme& t = theme();
	ImGui::SetCursorScreenPos(pos);
	ImGui::InvisibleButton("##kb", ImVec2(w, h), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	const bool hovered = ImGui::IsItemHovered();
	bool listening = g_listening == id;

	if (!listening && ImGui::IsItemClicked(ImGuiMouseButton_Left) && bind->mode != KeybindMode::Always)
		g_listening = id, listening = true;
	else if (!listening && ImGui::IsItemClicked(ImGuiMouseButton_Right))
		ImGui::OpenPopup("##kb_mode");

	bool changed = false;
	if (listening)
	{
		// Clicking elsewhere cancels.
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !hovered)
			g_listening = 0, listening = false;
		for (int k = ImGuiKey_NamedKey_BEGIN; listening && k < ImGuiKey_NamedKey_END; ++k)
		{
			ImGuiKey key = (ImGuiKey)k;
			if (!capturable(key) || !ImGui::IsKeyPressed(key, false))
				continue;
			bind->key = key == ImGuiKey_Escape ? ImGuiKey_None : key;
			bind->toggled = false;
			g_listening = 0;
			listening = false;
			changed = true;
		}
		if (listening)
			ImGui::SetKeyOwner(ImGuiKey_Escape, id);
	}

	float lst = anim::animate(anim::key(id, "lst"), listening);
	float hov = anim::animate(anim::key(id, "hov"), hovered);

	ImDrawList* dl = ImGui::GetWindowDrawList();
	if (caption)
	{
		ImVec2 cs = ImGui::CalcTextSize(caption, label_end(caption));
		dl->AddText(ImVec2(pos.x, pos.y + (h - cs.y) * 0.5f), styled(t.text_label), caption, label_end(caption));
	}

	const char* key_text = bind->mode == KeybindMode::Always ? "ON"
												 : listening															? "..."
																																	: key_name(bind->key);
	char bracket[48];
	snprintf(bracket, sizeof(bracket), "[ %s ]", key_text);
	ImU32 col = lerp_color(t.text_label, t.accent_col, std::max(lst, hov * 0.7f));
	if (bind->mode == KeybindMode::Always)
		col = t.accent_col;
	draw::text_swap(dl, anim::key(id, "txt"), pos.x + w - 2.0f, pos.y + h * 0.5f + 0.5f, draw::Align::Right, styled(col),
									bracket);

	if (begin_popup("##kb_mode", 96.0f))
	{
		for (int m = 0; m < 3; ++m)
		{
			KeybindMode mode = (KeybindMode)m;
			if (list_row(keybind_mode_name(mode), bind->mode == mode, ImGui::GetContentRegionAvail().x, RowStyle::Radio))
			{
				if (bind->mode != mode)
					changed = true;
				bind->mode = mode;
				bind->toggled = false;
				ImGui::CloseCurrentPopup();
			}
		}
		end_popup();
	}
	return changed;
}
} // namespace

bool keybind(const char* label, Keybind* bind)
{
	if (!search::filter(label))
		return false;
	ImGui::PushID(label);
	ImGuiID id = ImGui::GetID("##kb");
	ImVec2 pos = ImGui::GetCursorScreenPos();
	float w = ImGui::GetContentRegionAvail().x;
	float h = ImGui::GetFontSize() + 2.0f;
	bool changed = keybind_body(id, bind, pos, w, h, has_visible_label(label) ? label : nullptr);
	ImGui::PopID();
	return changed;
}

bool inline_keybind(const char* id_str, Keybind* bind)
{
	if (search::dry_run() || !search::filter(id_str))
		return false;
	ImGui::PushID(id_str);
	ImGuiID id = ImGui::GetID("##kb");

	ImGui::SameLine();
	float h = ImGui::GetFontSize() + 2.0f;
	float w = ImGui::CalcTextSize("[ WWWWWW ]").x;
	float right = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
	ImVec2 pos(right - w, ImGui::GetCursorScreenPos().y);
	bool changed = keybind_body(id, bind, pos, w, h, nullptr);
	ImGui::PopID();
	return changed;
}

} // namespace nc
