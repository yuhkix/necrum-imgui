#include "shell.h"

#include "necrum/app/overlay.h"
#include "necrum/core/anim.h"
#include "necrum/core/draw.h"
#include "necrum/core/fonts.h"
#include "necrum/core/search.h"
#include "necrum/core/theme.h"
#include "necrum/extras/web_image.h"
#include "necrum/fonts/icons_fa.h"
#include "necrum/widgets/widgets.h"

namespace nc
{

namespace
{
bool g_block_move = false;
constexpr float k_tab_button = 30.0f;
} // namespace

void block_window_move()
{
	g_block_move = true;
}

Page& Tab::add_page(std::string page_title, const char* page_icon, std::function<void()> draw)
{
	Page p;
	p.title = std::move(page_title);
	p.icon = page_icon;
	p.draw = std::move(draw);
	pages.push_back(std::move(p));
	return pages.back();
}

Shell::Shell(ShellConfig config) : config_(std::move(config))
{
	open_ = config_.start_open;
	id_ = ImHashStr(config_.title.c_str());
	if (!config_.logo_icon)
		config_.logo_icon = ICON_FA_STAR_AND_CRESCENT;
}

Tab& Shell::add_tab(std::string title, const char* icon)
{
	Tab t;
	t.title = std::move(title);
	t.icon = icon;
	tabs_.push_back(std::move(t));
	return tabs_.back();
}

Tab* Shell::tab(int index)
{
	return index >= 0 && index < (int)tabs_.size() ? &tabs_[index] : nullptr;
}

void Shell::select(int tab, int page)
{
	if (tab < 0 || tab >= (int)tabs_.size())
		return;
	tab_ = tab;
	page_ = clamp(page, 0, std::max(0, (int)tabs_[tab].pages.size() - 1));
}

void Shell::run_search()
{
	// Dry run every page in an off-screen window: widgets only count matches.
	int best_tab = -1, best_page = 0, best_hits = 0;
	const bool query = search::active();

	if (query)
	{
		ImGui::SetNextWindowPos(ImVec2(-10000.0f, -10000.0f));
		ImGui::SetNextWindowSize(config_.size);
		ImGui::Begin("##nc_search_dry_run", nullptr,
								 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
										 ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoFocusOnAppearing |
										 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav);
	}

	for (int ti = 0; ti < (int)tabs_.size(); ++ti)
	{
		Tab& tab = tabs_[ti];
		int tab_hits = 0;
		for (int pi = 0; pi < (int)tab.pages.size(); ++pi)
		{
			Page& page = tab.pages[pi];
			if (!query)
			{
				page.has_hits = true;
				continue;
			}
			ImGui::PushID(ti * 1000 + pi);
			search::push_scope(page.title.c_str());
			search::begin_dry_run();
			if (page.draw)
				page.draw();
			int hits = search::end_dry_run();
			search::pop_scope();
			ImGui::PopID();

			if (search::matches(page.title) || search::matches(tab.title))
				hits = std::max(hits, 1);
			page.has_hits = hits > 0;
			tab_hits += hits;
			if (hits > best_hits)
			{
				best_hits = hits;
				best_tab = ti;
				best_page = pi;
			}
		}
		tab.has_hits = !query || tab_hits > 0;
	}

	if (query)
	{
		ImGui::End();
		bool current_ok = tab_ < (int)tabs_.size() && page_ < (int)tabs_[tab_].pages.size() &&
											tabs_[tab_].pages[page_].has_hits;
		if (!current_ok && best_tab >= 0)
			select(best_tab, best_page);
	}
}

void Shell::render()
{
	const Theme& t = theme();
	ImGuiIO& io = ImGui::GetIO();

	if (config_.toggle_key != ImGuiKey_None && ImGui::IsKeyPressed(config_.toggle_key, false))
		toggle();
	if (config_.search && open_ && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_F, false))
		focus_search_ = true;

	alpha_ = anim::approach(alpha_, open_ ? 1.0f : 0.0f);
	set_overlays_editable(open_ && alpha_ > 0.5f);

	if (config_.search && search::sync())
		run_search();

	bool block_move = g_block_move;
	g_block_move = false;
	if (alpha_ <= 0.005f || tabs_.empty())
		return;

	if (tab_ >= (int)tabs_.size())
		tab_ = 0;

	const ImVec2 size = config_.size;
	ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - size.x) * 0.5f, (io.DisplaySize.y - size.y) * 0.5f),
													ImGuiCond_Once);
	ImGui::SetNextWindowSize(size, ImGuiCond_Always);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
													 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
													 ImGuiWindowFlags_NoSavedSettings;
	if (block_move)
		flags |= ImGuiWindowFlags_NoMove;
	if (!open_)
		flags |= ImGuiWindowFlags_NoInputs;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, t.window_rounding);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha_);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, with_alpha(t.window_bg, alpha_));

	char window_id[160];
	snprintf(window_id, sizeof(window_id), "%s##nc_shell", config_.title.c_str());
	if (ImGui::Begin(window_id, nullptr, flags))
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		win_pos_ = ImGui::GetWindowPos();
		const ImVec2 ws = ImGui::GetWindowSize();
		const float footer_h = config_.footer_height;
		const float body_h = ws.y - config_.header_height - footer_h;

		dl->AddRect(win_pos_, ImVec2(win_pos_.x + ws.x, win_pos_.y + ws.y), styled(t.window_border), t.window_rounding);
		draw_header(dl, win_pos_, ws.x);

		ImGui::SetCursorPos(ImVec2(0.0f, config_.header_height));
		ImGui::BeginChild("##nc_sidebar", ImVec2(config_.sidebar_width, body_h), ImGuiChildFlags_None,
											ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
		draw_sidebar(body_h);
		ImGui::EndChild();

		ImGui::SameLine(0.0f, 0.0f);
		const float content_w = ws.x - config_.sidebar_width;
		ImGui::BeginChild("##nc_content", ImVec2(content_w, body_h), ImGuiChildFlags_None,
											ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
		draw_content(content_w, body_h);
		ImGui::EndChild();

		if (footer_h > 0.0f)
			draw_footer(dl, ImVec2(win_pos_.x, win_pos_.y + ws.y - footer_h), ws.x);
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(4);
}

void Shell::draw_header(ImDrawList* dl, ImVec2 wp, float w)
{
	const Theme& t = theme();
	const float hh = config_.header_height;
	dl->AddRectFilled(wp, ImVec2(wp.x + w, wp.y + hh), styled(t.header_bg), t.window_rounding,
										ImDrawFlags_RoundCornersTop);

	// Logo
	const float logo = 32.0f;
	ImVec2 lp(wp.x + 9.0f, wp.y + (hh - logo) * 0.5f);
	ImTextureID tex = 0;
	if (!config_.logo_url.empty())
	{
		tex = web_image::get(config_.logo_url);
		if (!web_image::is_loaded(config_.logo_url))
			tex = 0;
	}
	if (tex)
	{
		dl->AddImageRounded(tex, lp, ImVec2(lp.x + logo, lp.y + logo), ImVec2(0, 0), ImVec2(1, 1), styled(IM_COL32_WHITE),
												9.0f);
	}
	else if (config_.logo_icon)
	{
		ImFont* f = fonts::icons();
		const float fs = 22.0f;
		ImVec2 is = draw::text_size(f, fs, config_.logo_icon);
		ImVec2 ip(lp.x + (logo - is.x) * 0.5f, lp.y + (logo - is.y) * 0.5f);
		for (int i = 1; i <= 3; ++i)
		{
			float spread = i * 1.5f;
			ImU32 gc = styled(t.accent_col, (0.055f - i * 0.012f) * t.glow);
			for (float dx = -spread; dx <= spread; dx += spread)
				for (float dy = -spread; dy <= spread; dy += spread)
					dl->AddText(f, fs, ImVec2(ip.x + dx, ip.y + dy), gc, config_.logo_icon);
		}
		dl->AddText(f, fs, ip, styled(t.accent_col), config_.logo_icon);
	}

	// Tab buttons (right aligned)
	float tabs_w = 0.0f;
	std::vector<float> widths;
	for (const Tab& tab : tabs_)
	{
		float bw = tab.icon ? k_tab_button : ImGui::CalcTextSize(tab.title.c_str()).x + 20.0f;
		widths.push_back(bw);
		tabs_w += bw;
	}
	float x = wp.x + w - tabs_w - 14.0f;
	const float cy = wp.y + hh * 0.5f;
	const float search_x_max = x - 10.0f;

	for (int i = 0; i < (int)tabs_.size(); ++i)
	{
		const Tab& tab = tabs_[i];
		const float bw = widths[i];
		ImGui::SetCursorScreenPos(ImVec2(x + 2.0f, cy - 12.0f));
		ImGui::PushID(i);
		ImGuiID bid = ImGui::GetID("##tab");
		if (ImGui::InvisibleButton("##tab", ImVec2(bw - 4.0f, 24.0f)) && tab_ != i)
			select(i, 0);
		float hov = anim::animate(anim::key(bid, "hov"), ImGui::IsItemHovered());
		float act = anim::animate(anim::key(bid, "act"), ImGui::IsItemActive());
		float sel = anim::animate(anim::key(bid, "sel"), tab_ == i);
		if (tab.icon)
			tooltip("%s", tab.title.c_str());
		ImGui::PopID();

		if (hov > 0.01f || act > 0.01f)
			dl->AddRectFilled(ImVec2(x + 1.0f, cy - 14.0f), ImVec2(x + bw - 1.0f, cy + 14.0f),
												styled(t.hover, hov * 0.05f + act * 0.05f), 4.0f);

		float dim = tab.has_hits ? 1.0f : 0.35f;
		ImU32 col = lerp_color(lerp_color(t.text_dim, t.text, hov), t.accent_col, sel);
		if (tab.icon)
		{
			ImFont* f = fonts::icons();
			ImVec2 is = draw::text_size(f, f->LegacySize, tab.icon);
			dl->AddText(f, f->LegacySize, ImVec2(x + (bw - is.x) * 0.5f, cy - is.y * 0.5f), styled(col, dim), tab.icon);
		}
		else
		{
			ImVec2 ts = ImGui::CalcTextSize(tab.title.c_str());
			dl->AddText(ImVec2(x + (bw - ts.x) * 0.5f, cy - ts.y * 0.5f), styled(col, dim), tab.title.c_str());
		}
		x += bw;
	}

	if (config_.search)
		draw_search(dl, ImVec2(wp.x + 56.0f, wp.y), search_x_max);
	else
	{
		ImFont* f = fonts::bold();
		dl->AddText(f, f->LegacySize, ImVec2(wp.x + 52.0f, cy - f->LegacySize * 0.5f), styled(t.text),
								config_.title.c_str());
	}

	dl->AddLine(ImVec2(wp.x, wp.y + hh - 1.0f), ImVec2(wp.x + w, wp.y + hh - 1.0f), styled(t.accent_col, 0.31f), 1.0f);
}

void Shell::draw_search(ImDrawList* dl, ImVec2 origin, float x_max)
{
	const Theme& t = theme();
	const float max_w = x_max - origin.x;
	if (max_w < 120.0f)
		return;

	const float h = 22.0f;
	const float w = std::min(224.0f, max_w);
	ImVec2 bmin(origin.x, origin.y + (config_.header_height - h) * 0.5f);
	ImVec2 bmax(bmin.x + w, bmin.y + h);
	ImGuiID fid = ImGui::GetID("##nc_search");
	const bool has_text = search::buffer()[0] != '\0';

	float& hov = anim::value(anim::key(fid, "hov"));
	float& foc = anim::value(anim::key(fid, "foc"));
	float filled = anim::animate(anim::key(fid, "fill"), has_text, 7.0f);
	float interact = std::max(anim::smoothstep(foc), anim::smoothstep(hov));
	float mix = saturate(interact + anim::smoothstep(filled) * 0.36f);

	dl->AddRectFilled(bmin, bmax, styled(t.field_bg), 4.0f);
	dl->AddRect(bmin, bmax, styled(lerp_color(t.field_border, t.accent_col, 0.1f + mix * 0.5f)), 4.0f);
	if (mix > 0.001f)
	{
		float cx = (bmin.x + bmax.x) * 0.5f;
		float half = std::max(0.0f, (w * 0.5f - 1.5f) * mix);
		draw::underline_glow(dl, ImVec2(cx - half, bmin.y), ImVec2(cx + half, bmax.y), t.accent_col, mix);
	}

	const float clear_w = has_text ? 20.0f : 0.0f;
	ImGui::SetCursorScreenPos(bmin);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, (h - ImGui::GetFontSize()) * 0.5f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, styled(t.text));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, styled(t.text_dim));
	ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, with_alpha(t.accent_col, 0.35f));
	ImGui::SetNextItemWidth(w - clear_w);
	if (focus_search_)
	{
		ImGui::SetKeyboardFocusHere();
		focus_search_ = false;
	}
	ImGui::InputTextWithHint("##nc_search", config_.search_hint, search::buffer(), search::buffer_size(),
													 ImGuiInputTextFlags_EscapeClearsAll);
	const bool active = ImGui::IsItemActive();
	ImGui::PopStyleColor(6);
	ImGui::PopStyleVar();

	hov = anim::approach(hov, ImGui::IsMouseHoveringRect(bmin, bmax) ? 1.0f : 0.0f);
	foc = anim::approach(foc, active ? 1.0f : 0.0f);

	ImFont* icons = fonts::icons();
	dl->AddText(icons, 12.0f, ImVec2(bmin.x + 8.0f, bmin.y + (h - 12.0f) * 0.5f),
							styled(lerp_color(t.text_dim, t.text_label, interact)), ICON_FA_MAGNIFYING_GLASS);

	if (has_text)
	{
		ImVec2 cmin(bmax.x - 18.0f, bmin.y + 4.0f);
		ImGui::SetCursorScreenPos(cmin);
		if (ImGui::InvisibleButton("##nc_search_clear", ImVec2(14.0f, 14.0f)))
			search::set_query("");
		float ch = anim::animate(anim::key(fid, "clear"), ImGui::IsItemHovered());
		dl->AddText(icons, 11.0f, ImVec2(cmin.x + 1.5f, cmin.y + 1.0f), styled(lerp_color(t.text_dim, t.text, ch)),
								ICON_FA_XMARK);
	}
}

void Shell::draw_sidebar(float h)
{
	const Theme& t = theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 sp = ImGui::GetCursorScreenPos();
	const float sw = config_.sidebar_width;

	ImU32 base = t.sidebar_bg;
	ImU32 lighter = lerp_color(base, t.hover, 0.004f), darker = lerp_color(base, t.shadow, 0.08f);
	dl->AddRectFilledMultiColor(sp, ImVec2(sp.x + sw, sp.y + h), styled(darker), styled(lighter), styled(base),
															styled(darker));
	dl->AddLine(ImVec2(sp.x + sw - 1.0f, sp.y), ImVec2(sp.x + sw - 1.0f, sp.y + h), styled(t.divider));

	Tab& tab = tabs_[tab_];
	constexpr float row_h = 30.0f, ind_h = 18.0f;
	const float base_y = sp.y + 24.0f;

	std::vector<int> rows;
	for (int i = 0; i < (int)tab.pages.size(); ++i)
		if (!search::active() || tab.pages[i].has_hits)
			rows.push_back(i);

	if (rows.empty())
	{
		dl->AddText(ImVec2(sp.x + 11.0f, base_y + 3.0f), styled(t.text_dim), "No matches");
		return;
	}
	if (std::find(rows.begin(), rows.end(), page_) == rows.end())
		page_ = rows.front();

	fonts::Scope bold(fonts::bold());
	int selected_row = 0;
	for (int r = 0; r < (int)rows.size(); ++r)
	{
		const int i = rows[r];
		const Page& page = tab.pages[i];
		const bool sel = page_ == i;
		if (sel)
			selected_row = r;
		const float ly = base_y + r * row_h;

		ImGui::SetCursorScreenPos(ImVec2(sp.x, ly));
		ImGui::PushID(i);
		ImGuiID rid = ImGui::GetID("##page");
		if (ImGui::InvisibleButton("##page", ImVec2(sw - 2.0f, row_h)))
			page_ = i;
		float hov = anim::animate(anim::key(rid, "hov"), ImGui::IsItemHovered());
		float act = anim::animate(anim::key(rid, "act"), ImGui::IsItemActive());
		ImGui::PopID();

		if (hov > 0.01f || act > 0.01f)
			dl->AddRectFilled(ImVec2(sp.x + 10.0f, ly), ImVec2(sp.x + sw - 10.0f, ly + row_h),
												styled(t.hover, hov * 0.03f + act * 0.03f), 4.0f);

		const float ty = ly + (row_h - ImGui::GetFontSize()) * 0.5f;
		float x = sp.x + 22.0f;
		ImU32 text_col = sel ? t.text : lerp_color(t.text_dim, t.text, hov);
		if (page.icon)
		{
			dl->AddText(ImVec2(x, ty), styled(sel ? t.accent_col : text_col), page.icon);
			x += ImGui::CalcTextSize(page.icon).x + 8.0f;
		}
		dl->PushClipRect(ImVec2(sp.x, ly), ImVec2(sp.x + sw - 8.0f, ly + row_h), true);
		dl->AddText(ImVec2(x, ty), styled(text_col), page.title.c_str());
		dl->PopClipRect();
	}

	float& ind = anim::value(anim::key(id_, "sidebar_ind"), selected_row * row_h);
	ind = anim::approach(ind, selected_row * row_h);
	float iy = base_y + ind + (row_h - ind_h) * 0.5f;
	dl->AddRectFilled(ImVec2(sp.x + 3.0f, iy), ImVec2(sp.x + 5.0f, iy + ind_h), styled(t.accent_col), 1.0f);
}

void Shell::draw_content(float w, float h)
{
	const Theme& t = theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 cp = ImGui::GetCursorScreenPos();
	dl->AddRectFilled(cp, ImVec2(cp.x + w, cp.y + h), styled(t.content_bg));

	if (tab_ != prev_tab_ || page_ != prev_page_)
	{
		content_fade_ = 0.0f;
		prev_tab_ = tab_;
		prev_page_ = page_;
	}
	content_fade_ = anim::approach(content_fade_, 1.0f);

	Tab& tab = tabs_[tab_];
	if (tab.pages.empty())
		return;
	page_ = clamp(page_, 0, (int)tab.pages.size() - 1);
	Page& page = tab.pages[page_];

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 12.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * content_fade_);
	ImGui::BeginChild("##nc_page", ImVec2(w, h), ImGuiChildFlags_AlwaysUseWindowPadding,
										ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PopStyleVar(); // padding is consumed by BeginChild; keep Alpha until EndChild
	ImGui::PushID(tab_ * 1000 + page_);

	if (tab.toolbar)
	{
		tab.toolbar();
		ImGui::Dummy(ImVec2(0.0f, 2.0f));
	}
	search::push_scope(page.title.c_str());
	if (page.draw)
		page.draw();
	search::pop_scope();

	ImGui::PopID();
	ImGui::EndChild();
	ImGui::PopStyleVar(); // Alpha
}

void Shell::draw_footer(ImDrawList* dl, ImVec2 pos, float w)
{
	const Theme& t = theme();
	const float h = config_.footer_height;
	dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + h), styled(t.footer_bg), t.window_rounding,
										ImDrawFlags_RoundCornersBottom);
	dl->AddLine(pos, ImVec2(pos.x + w, pos.y), styled(t.divider));

	const float ty = pos.y + (h - ImGui::GetFontSize()) * 0.5f;
	auto item_width = [](const FooterItem& it)
	{ return ImGui::CalcTextSize(it.label.c_str()).x + ImGui::CalcTextSize(" ").x + ImGui::CalcTextSize(it.value.c_str()).x; };
	auto draw_item = [&](float x, const FooterItem& it)
	{
		dl->AddText(ImVec2(x, ty), styled(t.text_dim), it.label.c_str());
		x += ImGui::CalcTextSize(it.label.c_str()).x + ImGui::CalcTextSize(" ").x;
		dl->AddText(ImVec2(x, ty), styled(t.accent_col), it.value.c_str());
	};

	float x = pos.x + 10.0f;
	for (const auto& it : config_.footer_left)
	{
		draw_item(x, it);
		x += item_width(it) + 16.0f;
	}
	x = pos.x + w - 10.0f;
	for (auto it = config_.footer_right.rbegin(); it != config_.footer_right.rend(); ++it)
	{
		x -= item_width(*it);
		draw_item(x, *it);
		x -= 16.0f;
	}
}

} // namespace nc
