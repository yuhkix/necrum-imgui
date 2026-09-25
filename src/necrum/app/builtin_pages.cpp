#include "builtin_pages.h"

#include "necrum/app/notifications.h"
#include "necrum/core/anim.h"
#include "necrum/core/config.h"
#include "necrum/core/search.h"
#include "necrum/core/theme.h"
#include "necrum/widgets/widgets.h"

namespace nc
{

void config_manager(const char* directory)
{
	static std::vector<std::string> files;
	static double last_scan = -10.0;
	static int selected = -1;
	static char name[64] = "default";

	const bool dry = search::dry_run();
	double now = ImGui::GetTime();
	if (!dry && now - last_scan > 2.0)
	{
		files = config_files::list(directory);
		last_scan = now;
		if (selected >= (int)files.size())
			selected = -1;
	}

	search::tags({"config", "profile", "preset"});
	if (listbox("Configs", &selected, files, 110.0f) && selected >= 0)
		snprintf(name, sizeof(name), "%s", files[selected].c_str());

	search::tags({"config", "profile"});
	input_text("Name", name, sizeof(name), "config name");

	const float bw = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
	search::tags({"config", "save"});
	if (button("Save", ImVec2(bw, 0.0f), ButtonStyle::Primary))
	{
		if (!name[0])
			notify(Notify::Warning, "Enter a config name first");
		else if (config_files::ensure_dir(directory) && config().save(config_files::path(directory, name)))
		{
			notify(Notify::Success, "Saved config '%s'", name);
			last_scan = -10.0;
		}
		else
			notify(Notify::Error, "Could not save '%s'", name);
	}
	ImGui::SameLine(0.0f, 8.0f);
	search::tags({"config", "load"});
	if (button("Load", ImVec2(bw, 0.0f)))
	{
		if (config().load(config_files::path(directory, name)))
			notify(Notify::Success, "Loaded config '%s'", name);
		else
			notify(Notify::Error, "Config '%s' not found", name);
	}

	search::tags({"config", "delete"});
	if (button("Delete", ImVec2(bw, 0.0f), ButtonStyle::Danger) && name[0])
		ImGui::OpenPopup("##nc_cfg_delete");
	ImGui::SameLine(0.0f, 8.0f);
	search::tags({"config", "reset", "defaults"});
	if (button("Reset", ImVec2(bw, 0.0f), ButtonStyle::Ghost))
	{
		config().reset();
		notify(Notify::Info, "Restored default settings");
	}

	if (!dry)
	{
		char message[128];
		snprintf(message, sizeof(message), "Delete '%s'? This cannot be undone.", name);
		if (confirm_dialog("##nc_cfg_delete", "Delete config", message, "Delete") == 1)
		{
			if (config_files::remove(config_files::path(directory, name)))
				notify(Notify::Success, "Deleted '%s'", name);
			else
				notify(Notify::Error, "Could not delete '%s'", name);
			selected = -1;
			last_scan = -10.0;
		}
	}
}

void theme_editor()
{
	Theme& t = theme();
	const auto& presets = themes::presets();

	int current = -1;
	std::vector<std::string> names;
	for (int i = 0; i < (int)presets.size(); ++i)
	{
		names.emplace_back(presets[i].name);
		if (t.name == presets[i].name)
			current = i;
	}
	search::tags({"theme", "preset", "style"});
	if (combo("Theme", &current, names) && current >= 0)
		set_theme(presets[current].make());

	static const HSV default_accent = themes::necrum().accent;
	search::tags({"theme", "accent", "color"});
	color_edit("Accent color", &t.accent, &default_accent);

	float speed = anim::speed();
	search::tags({"theme", "animation", "speed"});
	if (slider("Animation speed", &speed, 2.0f, 30.0f, "%.0f"))
		anim::set_speed(speed);

	search::tags({"theme", "glow"});
	slider("Glow", &t.glow, 0.0f, 1.5f, "%.0f%%", 100.0f);

	search::tags({"theme", "rounding"});
	if (slider("Rounding", &t.frame_rounding, 0.0f, 8.0f, "%.0f px"))
	{
		t.panel_rounding = t.frame_rounding + 1.0f;
		t.window_rounding = t.frame_rounding + 2.0f;
		t.popup_rounding = t.frame_rounding + 1.0f;
		apply_imgui_style(t);
	}
}

} // namespace nc
