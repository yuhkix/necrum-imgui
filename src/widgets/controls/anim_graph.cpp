#include "anim_graph.h"
#include "../ui_core.h"

namespace ui
{
std::unordered_map<ImGuiID, float[120]> s_history;

void anim_graph(const char* id, float width, float height, float speed)
{
	ImGui::PushID(id);
	ImGuiID uid = ImGui::GetID("##ag");
	float& val = ganim(uid, 0.0f);
	float time = (float)ImGui::GetTime();
	float target = (fmodf(time, 2.5f) < 1.25f) ? 1.0f : 0.0f;
	val = val + (target - val) * std::min(speed * g_dt, 1.0f);

	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##ag", ImVec2(width, height));
	ImDrawList* dl = ImGui::GetWindowDrawList();
	ImVec2 gmin = pos, gmax(pos.x + width, pos.y + height);

	dl->AddRectFilled(gmin, gmax, styled(col_groove), 1.0f);
	dl->AddRect(gmin, gmax, styled(col_groove_brd), 1.0f);

	auto& hist = s_history[uid];
	for (int i = 0; i < 119; i++)
		hist[i] = hist[i + 1];
	hist[119] = val;

	int segs = (int)std::min(width, 119.0f);
	float seg_w = width / (float)segs;
	for (int i = 0; i < segs - 1; i++)
	{
		int idx1 = 120 - segs + i;
		int idx2 = idx1 + 1;
		if (idx1 < 0)
			continue;
		float y1 = gmax.y - 3 - hist[idx1] * (height - 6);
		float y2 = gmax.y - 3 - hist[idx2] * (height - 6);
		dl->AddLine(ImVec2(gmin.x + i * seg_w, y1), ImVec2(gmin.x + (i + 1) * seg_w, y2), styled(col_accent), 1.5f);
	}
	float ty = gmax.y - 3 - target * (height - 6);
	for (float x = gmin.x; x < gmax.x; x += 6)
		dl->AddLine(ImVec2(x, ty), ImVec2(std::min(x + 3, gmax.x), ty), styled(col_accent, 0.24f), 1.0f);
	ImGui::PopID();
}
} // namespace ui
