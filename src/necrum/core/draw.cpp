#include "draw.h"

#include "anim.h"
#include "fonts.h"

#include <unordered_map>

namespace nc::draw
{

void glow(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float strength, ImU32 tint)
{
	const Theme& t = theme();
	strength = saturate(strength) * t.glow;
	if (strength <= 0.001f)
		return;

	const ImU32 base = tint ? tint : t.accent_col;
	constexpr int steps = 5;
	const float base_alpha = 0.14f * strength;
	for (int i = 0; i < steps; ++i)
	{
		float k = (float)i / (float)(steps - 1);
		float fade = (1.0f - k) * (1.0f - k);
		float spread = 1.0f + (float)i * 1.7f;
		dl->AddRectFilled(ImVec2(a.x - spread, a.y - spread), ImVec2(b.x + spread, b.y + spread),
											styled(with_alpha(base, base_alpha * fade)), rounding + spread);
	}
}

void accent_fill(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float alpha)
{
	glow(dl, a, b, rounding, alpha);
	dl->AddRectFilled(a, b, styled(theme().accent_col, alpha), rounding);
}

void shadow(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding, float alpha)
{
	const ImU32 base = theme().shadow;
	for (int i = 1; i <= 4; ++i)
	{
		float spread = (float)i * 2.0f;
		dl->AddRectFilled(ImVec2(a.x - spread, a.y - spread + 2.0f), ImVec2(b.x + spread, b.y + spread + 2.0f),
											styled(base, alpha * 0.10f / (float)i), rounding + spread);
	}
}

void underline_glow(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 col, float alpha, float height)
{
	ImVec2 gmin(a.x, b.y - height);
	ImU32 solid = styled(col, 0.14f * alpha);
	ImU32 clear = styled(col, 0.0f);
	dl->AddRectFilledMultiColor(gmin, b, clear, clear, solid, solid);
	dl->AddLine(ImVec2(a.x, b.y - 1.0f), ImVec2(b.x, b.y - 1.0f), styled(col, 0.9f * alpha), 1.0f);
}

float card(ImDrawList* dl, ImVec2 min, ImVec2 max, const char* title, float rounding, float header_height)
{
	const Theme& t = theme();
	if (rounding < 0.0f)
		rounding = t.panel_rounding;
	if (header_height < 0.0f)
		header_height = t.panel_header_height;
	const bool has_header = title && *title && header_height > 0.0f;

	if (has_header)
	{
		dl->AddRectFilled(min, ImVec2(max.x, min.y + header_height), styled(t.panel_header_bg), rounding,
											ImDrawFlags_RoundCornersTop);
		dl->AddRectFilled(ImVec2(min.x, min.y + header_height), max, styled(t.panel_bg), rounding,
											ImDrawFlags_RoundCornersBottom);
		ImU32 sheen = with_alpha(t.hover, t.dark ? 0.02f : 0.0f);
		dl->AddRectFilledMultiColor(min, ImVec2(max.x, min.y + 12.0f), styled(sheen), styled(sheen), 0, 0);
		dl->AddLine(ImVec2(min.x + 1.0f, min.y + header_height), ImVec2(max.x - 1.0f, min.y + header_height),
								styled(t.panel_border, 0.8f));
	}
	else
	{
		dl->AddRectFilled(min, max, styled(t.panel_bg), rounding);
	}

	dl->AddRect(min, max, styled(t.panel_border), rounding);

	// Accent arc hugging the top edge.
	dl->PathClear();
	dl->PathArcTo(ImVec2(min.x + rounding, min.y + rounding + 1.0f), rounding, IM_PI, IM_PI * 1.5f);
	dl->PathArcTo(ImVec2(max.x - rounding, min.y + rounding + 1.0f), rounding, IM_PI * 1.5f, IM_PI * 2.0f);
	dl->PathStroke(styled(t.accent_col, 0.45f), 0, 1.2f);

	if (has_header)
	{
		ImFont* f = fonts::bold();
		float size = f->LegacySize;
		dl->AddText(f, size, ImVec2(min.x + 12.0f, min.y + (header_height - size) * 0.5f), styled(t.text), title,
								label_end(title));
		return min.y + header_height;
	}
	return min.y;
}

void chevron(ImDrawList* dl, ImVec2 center, float size, float open, ImU32 col)
{
	const float rot = open * IM_PI;
	const float s = std::sin(rot), c = std::cos(rot);
	auto rotate = [&](float x, float y) { return ImVec2(center.x + x * c - y * s, center.y + x * s + y * c); };
	dl->AddTriangleFilled(rotate(-size, -size * 0.5f), rotate(size, -size * 0.5f), rotate(0.0f, size * 0.8f), col);
}

void check_mark(ImDrawList* dl, ImVec2 min, float size, ImU32 col, float t)
{
	t = saturate(t);
	if (t <= 0.001f)
		return;
	float thickness = std::max(size / 5.0f, 1.0f);
	float sz = size - thickness * 0.5f;
	ImVec2 pos(min.x + thickness * 0.25f, min.y + thickness * 0.25f);
	float third = sz / 3.0f;
	float bx = pos.x + third;
	float by = pos.y + sz - third * 0.5f;
	ImVec2 p0(bx - third, by - third);
	ImVec2 p1(bx, by);
	ImVec2 p2(bx + third * 2.0f, by - third * 2.0f);

	// Animate the stroke: first leg, then second leg.
	float first = saturate(t * 2.0f);
	float second = saturate(t * 2.0f - 1.0f);
	dl->PathLineTo(p0);
	dl->PathLineTo(lerp(p0, p1, first));
	if (second > 0.0f)
		dl->PathLineTo(lerp(p1, p2, second));
	dl->PathStroke(col, 0, thickness);
}

void text(ImDrawList* dl, ImVec2 pos, ImU32 col, const char* txt, const char* txt_end)
{
	dl->AddText(pos, col, txt, txt_end);
}

void text_aligned(ImDrawList* dl, ImVec2 anchor, Align align, float center_y, ImU32 col, const char* txt,
									const char* txt_end)
{
	ImVec2 sz = ImGui::CalcTextSize(txt, txt_end);
	float x = anchor.x;
	if (align == Align::Center)
		x -= sz.x * 0.5f;
	else if (align == Align::Right)
		x -= sz.x;
	dl->AddText(ImVec2(x, center_y - sz.y * 0.5f), col, txt, txt_end);
}

void text_font(ImDrawList* dl, ImFont* font, float size, ImVec2 pos, ImU32 col, const char* txt, const char* txt_end)
{
	dl->AddText(font, size, pos, col, txt, txt_end);
}

ImVec2 text_size(ImFont* font, float size, const char* txt, const char* txt_end)
{
	return font->CalcTextSizeA(size, FLT_MAX, 0.0f, txt, txt_end);
}

namespace
{
struct TextSwap
{
	std::string current;
	std::string previous;
	float blend = 1.0f;
	int last_frame = 0;
};

std::unordered_map<ImGuiID, TextSwap>& swap_store()
{
	static std::unordered_map<ImGuiID, TextSwap> store;
	return store;
}
} // namespace

void text_swap(ImDrawList* dl, ImGuiID id, float x, float center_y, Align align, ImU32 col, const char* txt)
{
	auto& store = swap_store();
	const int frame = ImGui::GetFrameCount();

	// Occasional sweep so the store does not grow unbounded.
	if (frame % 600 == 0)
	{
		for (auto it = store.begin(); it != store.end();)
			it = (frame - it->second.last_frame > 600) ? store.erase(it) : std::next(it);
	}

	TextSwap& st = store[id];
	st.last_frame = frame;
	const char* next = txt ? txt : "";
	if (st.current.empty() && st.previous.empty())
	{
		st.current = next;
		st.blend = 1.0f;
	}
	else if (st.current != next)
	{
		st.previous = st.current;
		st.current = next;
		st.blend = 0.0f;
	}
	st.blend = anim::approach(st.blend, 1.0f);
	if (st.blend > 0.995f)
		st.previous.clear();

	const float in_t = saturate(st.blend);
	const float out_t = st.previous.empty() ? 0.0f : 1.0f - in_t;

	auto place = [&](const std::string& s, float alpha, float dy)
	{
		ImVec2 sz = ImGui::CalcTextSize(s.c_str());
		float px = x;
		if (align == Align::Center)
			px -= sz.x * 0.5f;
		else if (align == Align::Right)
			px -= sz.x;
		dl->AddText(ImVec2(px, center_y - sz.y * 0.5f + dy), alpha_mul(col, alpha), s.c_str());
	};

	if (out_t > 0.001f)
		place(st.previous, out_t, -(1.0f - in_t) * 2.0f);
	if (in_t > 0.001f)
		place(st.current, in_t, (1.0f - in_t) * 2.0f);
}

} // namespace nc::draw
