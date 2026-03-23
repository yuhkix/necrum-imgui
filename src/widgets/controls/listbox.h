#pragma once
#include "dropdown.h"

namespace ui
{
inline bool flat_listbox(const char* id, int* sel, const char** items, int count, float height, float width = -1.0f)
{
	ImGui::PushID(id);
	float w = (width >= 0.0f) ? width : ImGui::GetContentRegionAvail().x;
	ImVec2 pos = ImGui::GetCursorScreenPos();

	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(pos, ImVec2(pos.x + w, pos.y + height), styled(IM_COL32(6, 6, 6, 255)), 3.0f);
	dl->AddRect(pos, ImVec2(pos.x + w, pos.y + height), styled(col_groove_brd), 3.0f);

	ImGui::SetCursorScreenPos(ImVec2(pos.x, pos.y + 2));
	bool changed = false;
	if (ImGui::BeginChild("##lb", ImVec2(w, height - 4), false, ImGuiWindowFlags_NoBackground))
	{
		for (int i = 0; i < count; i++)
		{
			if (dd_item(items[i], *sel == i, false, w))
			{
				*sel = i;
				changed = true;
			}
		}
	}
	ImGui::EndChild();
	ImGui::PopID();
	return changed;
}
} // namespace ui
