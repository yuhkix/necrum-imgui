// necrum showcase: a tour of the framework that doubles as a template for new
// applications. Every page below only uses the public API from necrum.h.

#include "necrum/necrum.h"

namespace
{

struct Settings
{
	// Controls
	bool checkbox = true;
	bool toggle = false;
	int radio = 1;
	float opacity = 0.75f;
	int quality = 60;
	float range_lo = 0.25f, range_hi = 0.75f;
	int segment = 1;

	// Selection
	int combo = 0;
	bool multi[5] = {true, false, true, false, false};
	int list = 2;
	nc::HSV color{200.0f, 0.7f, 0.95f, 1.0f};
	nc::HSV color_alpha{330.0f, 0.6f, 1.0f, 0.6f};
	nc::Keybind action_bind{ImGuiKey_F, nc::KeybindMode::Toggle};
	nc::Keybind hold_bind{ImGuiKey_MouseX1, nc::KeybindMode::Hold};

	// Input
	char name[64] = "necrum";
	std::string notes = "Multi-purpose UI framework";
	int count = 3;
	float scale = 1.25f;

	// Overlays
	bool show_keybinds = true;
	bool show_status = true;
	bool show_watermark = true;
	int toast_type = 1;
	int sub_tab = 0;
};

class Showcase final : public nc::App
{
public:
	Showcase() : shell_(make_config())
	{
		bind_config();
		nc::keybinds::track(&s_.action_bind, "Action");
		nc::keybinds::track(&s_.hold_bind, "Hold action");
		build_retained_page();

		auto& widgets = shell_.add_tab("Widgets", ICON_FA_SHAPES);
		widgets.add_page("Controls", ICON_FA_TOGGLE_ON, [this] { page_controls(); });
		widgets.add_page("Selection", ICON_FA_LIST, [this] { page_selection(); });
		widgets.add_page("Input", ICON_FA_KEYBOARD, [this] { page_input(); });

		auto& layout = shell_.add_tab("Layout", ICON_FA_TABLE_COLUMNS);
		layout.add_page("Panels", ICON_FA_TABLE_CELLS, [this] { page_panels(); });
		layout.add_page("Retained", ICON_FA_SITEMAP, [this] { retained_root_.draw(); });
		layout.add_page("Display", ICON_FA_CHART_LINE, [this] { page_display(); });

		auto& hud = shell_.add_tab("Overlays", ICON_FA_LAYER_GROUP);
		hud.add_page("HUD", ICON_FA_WINDOW_RESTORE, [this] { page_overlays(); });

		auto& settings = shell_.add_tab("Settings", ICON_FA_GEAR);
		settings.add_page("Appearance", ICON_FA_PALETTE, [this] { page_appearance(); });
		settings.add_page("Configs", ICON_FA_FILE_LINES, [this] { page_configs(); });
	}

	void frame() override
	{
		if (nc::splash(splash_))
			return;
		if (!greeted_)
		{
			nc::notify(nc::Notify::Success, "Welcome! Press INSERT to toggle the menu.");
			greeted_ = true;
		}

		if (s_.show_watermark)
			nc::watermark("necrum", [](std::vector<std::string>& out) {
				out.push_back("showcase");
				out.push_back(nc::fps_text());
				out.push_back(nc::clock_text());
			});

		nc::keybind_overlay(s_.show_keybinds, {.anchor = nc::Corner::BottomLeft});
		draw_status_overlay();
		shell_.render();
	}

	bool wants_input() const override { return shell_.is_open(); }

private:
	static nc::ShellConfig make_config()
	{
		nc::ShellConfig cfg;
		cfg.title = "necrum";
		cfg.logo_icon = ICON_FA_STAR_AND_CRESCENT;
		cfg.footer_left = {{"framework:", "necrum"}};
		cfg.footer_right = {{"build:", "showcase"}};
		return cfg;
	}

	void bind_config()
	{
		auto& c = nc::config();
		c.bind("controls.checkbox", &s_.checkbox);
		c.bind("controls.toggle", &s_.toggle);
		c.bind("controls.radio", &s_.radio);
		c.bind("controls.opacity", &s_.opacity);
		c.bind("controls.quality", &s_.quality);
		c.bind("controls.range_lo", &s_.range_lo);
		c.bind("controls.range_hi", &s_.range_hi);
		c.bind("selection.combo", &s_.combo);
		c.bind("selection.multi", s_.multi, 5);
		c.bind("selection.color", &s_.color);
		c.bind("binds.action", &s_.action_bind);
		c.bind("binds.hold", &s_.hold_bind);
		c.bind("input.notes", &s_.notes);
		c.bind("hud.keybinds", &s_.show_keybinds);
		c.bind("hud.status", &s_.show_status);
		c.bind("hud.watermark", &s_.show_watermark);
		c.bind("theme.accent", &nc::theme().accent);
	}

	void page_controls()
	{
		nc::begin_columns(2);
		if (nc::begin_panel("Toggles", {0, -1}))
		{
			nc::checkbox("Checkbox", &s_.checkbox);
			nc::toggle("Toggle switch", &s_.toggle);
			nc::separator("Radio");
			nc::radio("Low", &s_.radio, 0);
			nc::radio("Medium", &s_.radio, 1);
			nc::radio("High", &s_.radio, 2);
			nc::separator();
			nc::checkbox("With inline color", &s_.checkbox);
			nc::inline_color("##inline_col", &s_.color);
			nc::checkbox("With inline keybind", &s_.toggle);
			nc::inline_keybind("##inline_kb", &s_.action_bind);
		}
		nc::end_panel();
		nc::next_column();
		if (nc::begin_panel("Sliders", {0, -1}))
		{
			nc::slider("Opacity", &s_.opacity, 0.0f, 1.0f, "%.0f%%", 100.0f);
			nc::slider("Quality", &s_.quality, 0, 100, "%d");
			nc::range_slider("Range", &s_.range_lo, &s_.range_hi, 0.0f, 1.0f, "%.0f - %.0f%%", 100.0f);
			static const char* modes[] = {"Off", "Auto", "Always"};
			nc::segmented("Segmented", &s_.segment, modes, 3);
			nc::progress_bar("Progress", s_.opacity);
			nc::separator("Buttons");
			float bw = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;
			if (nc::button("Default", {bw, 0}))
				nc::notify(nc::Notify::Info, "Default button clicked");
			ImGui::SameLine(0, 8);
			if (nc::button("Primary", {bw, 0}, nc::ButtonStyle::Primary))
				nc::notify(nc::Notify::Success, "Primary button clicked");
			if (nc::button("Ghost", {bw, 0}, nc::ButtonStyle::Ghost))
				nc::notify(nc::Notify::Info, "Ghost button clicked");
			ImGui::SameLine(0, 8);
			if (nc::button("Danger", {bw, 0}, nc::ButtonStyle::Danger))
				ImGui::OpenPopup("##danger_confirm");
			if (nc::confirm_dialog("##danger_confirm", "Are you sure?", "This is a themed modal dialog.") == 1)
				nc::notify(nc::Notify::Warning, "Confirmed the dangerous thing");
		}
		nc::end_panel();
		nc::end_columns();
	}

	void page_selection()
	{
		static const char* items[] = {"Alpha", "Bravo", "Charlie", "Delta", "Echo"};
		nc::begin_columns(2);
		if (nc::begin_panel("Lists", {0, -1}))
		{
			nc::combo("Combo", &s_.combo, items, 5);
			nc::multi_combo("Multi combo", s_.multi, items, 5);
			nc::listbox("List box", &s_.list, items, 5, 100.0f);
		}
		nc::end_panel();
		nc::next_column();
		if (nc::begin_panel("Colors & binds", {0, -1}))
		{
			static const nc::HSV color_default{200.0f, 0.7f, 0.95f, 1.0f};
			nc::color_edit("Color", &s_.color, &color_default);
			nc::color_edit("Color with alpha", &s_.color_alpha, nullptr, true);
			nc::separator("Keybinds");
			nc::keybind("Action key", &s_.action_bind);
			nc::keybind("Hold key", &s_.hold_bind);
			nc::text_dim("Action is %s", s_.action_bind.active() ? "ON" : "off");
			nc::help_marker("Right click a keybind to change its mode.");
		}
		nc::end_panel();
		nc::end_columns();
	}

	void page_input()
	{
		nc::begin_columns(2);
		if (nc::begin_panel("Text", {0, -1}))
		{
			nc::input_text("Name", s_.name, sizeof(s_.name), "your name");
			nc::input_text("Notes (std::string)", &s_.notes, "anything");
			nc::text_wrapped("Hello, %s! Notes are %d characters long.", s_.name, (int)s_.notes.size());
		}
		nc::end_panel();
		nc::next_column();
		if (nc::begin_panel("Numbers", {0, -1}))
		{
			nc::input_int("Count", &s_.count, 1, 0, 99);
			nc::input_float("Scale", &s_.scale, 0.05f, 0.25f, 4.0f);
			nc::text("count x scale = %.2f", s_.count * s_.scale);
		}
		nc::end_panel();
		nc::end_columns();
	}

	void page_panels()
	{
		static const char* sub[] = {"Auto height", "Three columns"};
		nc::tab_strip("##panels_sub", &s_.sub_tab, sub, 2, false);
		nc::spacing(4.0f);
		if (s_.sub_tab == 0)
		{
			nc::begin_columns(2);
			if (nc::begin_panel("Grows with content"))
			{
				nc::text_wrapped("Panels with size.y == 0 fit their content.");
				nc::checkbox("Show more", &s_.toggle);
				if (s_.toggle)
				{
					nc::slider("Extra A", &s_.opacity, 0.0f, 1.0f);
					nc::slider("Extra B", &s_.quality, 0, 100);
				}
			}
			nc::end_panel();
			nc::spacing(8.0f);
			if (nc::begin_panel("##untitled", {0, 0}, nc::PanelFlags_NoTitle))
				nc::text_dim("Panels can also be untitled.");
			nc::end_panel();
			nc::next_column();
			if (nc::begin_panel("Fixed 180px", {0, 180}))
			{
				for (int i = 0; i < 12; ++i)
					nc::text_dim("Scrollable line %d", i + 1);
			}
			nc::end_panel();
			nc::end_columns();
		}
		else
		{
			nc::begin_columns(3, 12.0f);
			for (int i = 0; i < 3; ++i)
			{
				if (i > 0)
					nc::next_column();
				char title[32];
				snprintf(title, sizeof(title), "Column %d", i + 1);
				if (nc::begin_panel(title, {0, -1}))
				{
					ImGui::PushID(i);
					nc::checkbox("Option", &s_.multi[i]);
					nc::slider("Value", &s_.opacity, 0.0f, 1.0f);
					ImGui::PopID();
				}
				nc::end_panel();
			}
			nc::end_columns();
		}
	}

	void build_retained_page()
	{
		using namespace nc::retained;
		auto& cols = retained_root_.add<Columns>(2);

		auto& left = cols.column(0).add<Panel>("Built once", ImVec2(0, -1));
		left.add<Label>("This page is a retained node tree.");
		auto& enabled = left.add<Checkbox>("Enabled (bound)", &s_.checkbox);
		enabled.on_change([] { nc::notify(nc::Notify::Info, "Checkbox changed"); });
		left.add<Slider>("Owned value", 0.0f, 10.0f, 5.0f, "%.1f").visible_if([this] { return s_.checkbox; });
		left.add<Combo>("Owned combo", std::vector<std::string>{"One", "Two", "Three"});
		left.add<Button>("Say hi", [] { nc::notify(nc::Notify::Success, "Hi from a retained button"); });

		auto& right = cols.column(1).add<Panel>("More nodes", ImVec2(0, -1));
		right.add<Toggle>("Toggle node", false);
		right.add<ColorPicker>("Color node", nc::HSV{120.0f, 0.6f, 0.9f, 1.0f});
		right.add<KeybindControl>("Keybind node", nc::Keybind{ImGuiKey_G});
		right.add<TextInput>("Text node", "", "type here");
		right.add<Custom>([] {
			nc::text_dim("Custom nodes run any immediate code.");
			return false;
		});
	}

	void page_display()
	{
		static float samples[64];
		static float histogram[16];
		float time = (float)ImGui::GetTime();
		for (int i = 0; i < 64; ++i)
			samples[i] = std::sin(time * 2.0f + i * 0.2f) * 0.5f + 0.5f + std::sin(i * 0.7f) * 0.1f;
		for (int i = 0; i < 16; ++i)
			histogram[i] = std::fabs(std::sin(time * 0.8f + i * 0.45f));

		nc::begin_columns(2);
		if (nc::begin_panel("Plots", {0, -1}))
		{
			nc::plot_lines("Signal", samples, 64, 0.0f, 1.2f, 60.0f);
			nc::plot_histogram("Histogram", histogram, 16, 0.0f, 1.0f, 60.0f);
		}
		nc::end_panel();
		nc::next_column();
		if (nc::begin_panel("Misc", {0, -1}))
		{
			nc::label("Animation curve");
			nc::animation_preview("##curve", 50.0f);
			nc::label("Spinner & badges");
			nc::spinner("##spin");
			ImGui::SameLine(0, 10);
			nc::badge("default");
			ImGui::SameLine(0, 6);
			nc::badge("success", nc::theme().success);
			ImGui::SameLine(0, 6);
			nc::badge("error", nc::theme().error);
		}
		nc::end_panel();
		nc::end_columns();
	}

	void page_overlays()
	{
		nc::begin_columns(2);
		if (nc::begin_panel("Overlays", {0, -1}))
		{
			nc::checkbox("Keybind list", &s_.show_keybinds);
			nc::checkbox("Status card", &s_.show_status);
			nc::checkbox("Watermark", &s_.show_watermark);
			nc::text_wrapped("Overlays can be dragged while this window is open.");
		}
		nc::end_panel();
		nc::next_column();
		if (nc::begin_panel("Notifications", {0, -1}))
		{
			static const char* types[] = {"Success", "Info", "Warning", "Error"};
			nc::segmented("Type", &s_.toast_type, types, 4);
			static const char* corners[] = {"Top left", "Top center", "Top right", "Bottom left", "Bottom center",
																			"Bottom right"};
			int corner = (int)nc::notify_style().corner;
			if (nc::combo("Corner", &corner, corners, 6))
				nc::notify_style().corner = (nc::Corner)corner;
			if (nc::button("Show notification", {-1, 0}, nc::ButtonStyle::Primary))
				nc::notify((nc::Notify)s_.toast_type, "This is a %s notification", types[s_.toast_type]);
		}
		nc::end_panel();
		nc::end_columns();
	}

	void page_appearance()
	{
		nc::begin_columns(2);
		if (nc::begin_panel("Theme", {0, -1}))
			nc::theme_editor();
		nc::end_panel();
		nc::next_column();
		if (nc::begin_panel("Preview", {0, -1}))
		{
			nc::label("Animation curve");
			nc::animation_preview("##preview_curve");
			nc::checkbox("Sample checkbox", &s_.checkbox);
			nc::slider("Sample slider", &s_.opacity, 0.0f, 1.0f);
		}
		nc::end_panel();
		nc::end_columns();
	}

	void page_configs()
	{
		nc::begin_columns(2);
		if (nc::begin_panel("Configs", {0, -1}))
			nc::config_manager("configs");
		nc::end_panel();
		nc::next_column();
		if (nc::begin_panel("About", {0, -1}))
		{
			nc::text_wrapped("Bound values are saved as plain text key/value pairs.");
			nc::text_dim("%d values bound", (int)nc::config().size());
		}
		nc::end_panel();
		nc::end_columns();
	}

	void draw_status_overlay()
	{
		char badge[16];
		snprintf(badge, sizeof(badge), "%.0f fps", ImGui::GetIO().Framerate);
		nc::OverlayOptions o;
		o.anchor = nc::Corner::TopLeft;
		o.margin = 18.0f;
		o.width = 230.0f;
		o.badge = badge;
		if (!nc::begin_overlay("Status", s_.show_status, o))
			return;
		nc::overlay_row("Menu", shell_.is_open() ? "open" : "closed", shell_.is_open());
		nc::overlay_row("Search", nc::search::active() ? nc::search::query().c_str() : "-", nc::search::active());
		nc::overlay_row("Theme", nc::theme().name.c_str(), true);
		nc::end_overlay();
	}

	Settings s_;
	nc::SplashOptions splash_{.title = "necrum", .logo_url = {}, .logo_texture = 0, .logo_icon = ICON_FA_STAR_AND_CRESCENT,
													 .duration = 2.4f};
	nc::Shell shell_;
	nc::retained::Group retained_root_;
	bool greeted_ = false;
};

} // namespace

NC_REGISTER_APP(Showcase)
