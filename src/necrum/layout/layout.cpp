#include "layout.h"

#include "necrum/core/draw.h"
#include "necrum/core/search.h"
#include "necrum/core/theme.h"

namespace nc
{

namespace
{
struct PanelState
{
	ImVec2 pos;
	float width = 0.0f;
	float header = 0.0f;
	PanelFlags flags = 0;
	bool dry = false;
	int visible_before = 0;
	const char* title = nullptr;
	ImVec2 body_min, body_max;
};

struct ColumnsState
{
	int count = 1;
	int index = 0;
	float gap = 0.0f;
	float col_w = 0.0f;
	ImVec2 origin;
	float max_y = 0.0f;
};

std::vector<PanelState> g_panels;
std::vector<ColumnsState> g_columns;
} // namespace

float available_width()
{
	if (!g_columns.empty())
	{
		// Only use the column width while we are still in the window that opened the columns.
		return g_columns.back().col_w;
	}
	return ImGui::GetContentRegionAvail().x;
}

bool begin_panel(const char* title, ImVec2 size, PanelFlags flags)
{
	const Theme& t = theme();
	PanelState st;
	st.flags = flags;
	st.title = title;
	st.dry = search::dry_run();
	st.visible_before = search::visible_count();
	if (!has_visible_label(title))
		st.flags |= PanelFlags_NoTitle;

	search::push_scope(title);

	if (st.dry)
	{
		g_panels.push_back(st);
		return true;
	}

	ImGui::PushID(title);
	st.pos = ImGui::GetCursorScreenPos();
	st.width = size.x > 0.0f ? size.x : available_width();
	st.header = (st.flags & PanelFlags_NoTitle) ? 0.0f : t.panel_header_height;

	float height = 0.0f;
	ImGuiChildFlags child_flags = ImGuiChildFlags_None;
	if (!(st.flags & PanelFlags_NoPadding))
		child_flags |= ImGuiChildFlags_AlwaysUseWindowPadding;
	if (size.y == 0.0f)
		child_flags |= ImGuiChildFlags_AutoResizeY;
	else if (size.y < 0.0f)
		height = std::max(st.header + 20.0f, ImGui::GetContentRegionAvail().y + size.y + 1.0f) - st.header;
	else
		height = std::max(1.0f, size.y - st.header);

	ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoBackground;
	if (st.flags & PanelFlags_NoScroll)
		win_flags |= ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	ImGui::SetCursorScreenPos(ImVec2(st.pos.x, st.pos.y + st.header));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, (st.flags & PanelFlags_NoPadding) ? ImVec2(0, 0) : t.panel_padding);
	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 3.0f);
	ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, IM_COL32(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, styled(t.field_border));
	ImGui::BeginChild("##panel", ImVec2(st.width, height), child_flags, win_flags);
	// Padding and scrollbar style are consumed by BeginChild; don't leak them into nested windows.
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));

	ImGui::PushItemWidth(-FLT_MIN);
	st.body_min = ImGui::GetWindowPos();
	st.body_max = ImVec2(st.body_min.x + ImGui::GetWindowWidth(), st.body_min.y + ImGui::GetWindowHeight());
	g_panels.push_back(st);

	// Columns opened outside the panel must not leak their width into it.
	g_columns.push_back(ColumnsState{1, 0, 0.0f, ImGui::GetContentRegionAvail().x, ImGui::GetCursorScreenPos(), 0.0f});
	return true;
}

void end_panel()
{
	IM_ASSERT(!g_panels.empty() && "end_panel() without begin_panel()");
	if (g_panels.empty())
		return;
	PanelState st = g_panels.back();
	g_panels.pop_back();

	if (st.dry)
	{
		search::pop_scope();
		return;
	}

	g_columns.pop_back();

	const Theme& t = theme();
	if (search::active() && !(st.flags & PanelFlags_NoSearchHint) && search::visible_count() == st.visible_before)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, styled(t.text_dim));
		ImGui::TextUnformatted("No matches in this section.");
		ImGui::PopStyleColor();
	}

	ImGui::PopItemWidth();
	ImGui::PopStyleVar(); // ItemSpacing
	ImGui::EndChild();
	search::pop_scope();

	ImVec2 child_max = ImGui::GetItemRectMax();
	ImVec2 min = st.pos;
	ImVec2 max(st.pos.x + st.width, child_max.y);
	draw::card(ImGui::GetWindowDrawList(), min, max, (st.flags & PanelFlags_NoTitle) ? nullptr : st.title);
	ImGui::PopID();
}

ImVec2 panel_body_min()
{
	return g_panels.empty() ? ImGui::GetWindowPos() : g_panels.back().body_min;
}

ImVec2 panel_body_max()
{
	if (g_panels.empty())
		return ImVec2(ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight());
	return g_panels.back().body_max;
}

// Columns are pure layout: during a search dry run nothing is drawn, so they
// only track state (moving the cursor there would trip ImGui's boundary checks).
void begin_columns(int count, float gap)
{
	count = std::max(1, count);
	if (search::dry_run())
	{
		g_columns.push_back(ColumnsState{count, 0, gap, 0.0f, ImVec2(0, 0), 0.0f});
		return;
	}
	ColumnsState st;
	st.count = count;
	st.gap = gap;
	st.origin = ImGui::GetCursorScreenPos();
	st.col_w = std::floor((available_width() - gap * (count - 1)) / count);
	st.max_y = st.origin.y;
	g_columns.push_back(st);
	ImGui::BeginGroup();
}

void next_column()
{
	IM_ASSERT(!g_columns.empty() && "next_column() without begin_columns()");
	ColumnsState& st = g_columns.back();
	if (search::dry_run())
		return;
	ImGui::EndGroup();
	st.max_y = std::max(st.max_y, ImGui::GetItemRectMax().y);
	st.index = std::min(st.index + 1, st.count - 1);
	ImGui::SetCursorScreenPos(ImVec2(st.origin.x + (st.col_w + st.gap) * st.index, st.origin.y));
	ImGui::BeginGroup();
}

void end_columns()
{
	IM_ASSERT(!g_columns.empty() && "end_columns() without begin_columns()");
	ColumnsState st = g_columns.back();
	g_columns.pop_back();
	if (search::dry_run())
		return;
	ImGui::EndGroup();
	st.max_y = std::max(st.max_y, ImGui::GetItemRectMax().y);
	ImGui::SetCursorScreenPos(ImVec2(st.origin.x, st.max_y));
	ImGui::Dummy(ImVec2(0.0f, 0.0f));
}

void spacing(float pixels)
{
	if (search::dry_run())
		return;
	ImGui::Dummy(ImVec2(0.0f, pixels));
}

} // namespace nc
