#include "panel.h"

namespace ui
{
void draw_panel(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* title, float rounding)
{
	if (is_search_pass_)
		return;

	float header_h = 28.0f;

	dl->AddRectFilled(min, ImVec2(max.x, min.y + header_h), styled(col_panel_title), rounding,
										ImDrawFlags_RoundCornersTop);
	dl->AddRectFilled(ImVec2(min.x, min.y + header_h), max, styled(col_panel), rounding, ImDrawFlags_RoundCornersBottom);

	dl->AddRectFilledMultiColor(min, ImVec2(max.x, min.y + 12), styled(IM_COL32(255, 255, 255, 5)),
															styled(IM_COL32(255, 255, 255, 5)), styled(IM_COL32(255, 255, 255, 0)),
															styled(IM_COL32(255, 255, 255, 0)));

	dl->AddLine(ImVec2(min.x + 1, min.y + header_h), ImVec2(max.x - 1, min.y + header_h),
							styled(IM_COL32(45, 45, 48, 110)));

	dl->AddRect(min, max, styled(IM_COL32(45, 45, 48, 140)), rounding);

	dl->PathClear();
	dl->PathArcTo(ImVec2(min.x + rounding, min.y + rounding + 1.0f), rounding, IM_PI, IM_PI * 1.5f);
	dl->PathArcTo(ImVec2(max.x - rounding, min.y + rounding + 1.0f), rounding, IM_PI * 1.5f, IM_PI * 2.0f);
	dl->PathStroke(styled(alpha_mul(col_accent, 0.45f)), false, 1.2f);

	ImGui::PushFont(theme::font_bold);
	dl->AddText(ImVec2(min.x + 12, min.y + 6), styled(col_text), title);
	ImGui::PopFont();
}
} // namespace ui
