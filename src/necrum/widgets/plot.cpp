#include "internal.h"

#include <unordered_map>

namespace nc
{

using namespace detail;

namespace
{
struct PlotArea
{
	ImVec2 min, max;
};

PlotArea plot_frame(const char* label, float height)
{
	const Theme& t = theme();
	float w = stacked_label(label);
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::Dummy(ImVec2(w, height));
	ImVec2 max(pos.x + w, pos.y + height);
	ImDrawList* dl = ImGui::GetWindowDrawList();
	dl->AddRectFilled(pos, max, styled(t.field_bg), t.frame_rounding);
	dl->AddRect(pos, max, styled(t.field_border), t.frame_rounding);
	return {pos, max};
}

void value_range(const float* values, int count, float* mn, float* mx)
{
	if (*mn == FLT_MAX || *mx == FLT_MAX)
	{
		float lo = FLT_MAX, hi = -FLT_MAX;
		for (int i = 0; i < count; ++i)
		{
			lo = std::min(lo, values[i]);
			hi = std::max(hi, values[i]);
		}
		if (*mn == FLT_MAX)
			*mn = lo;
		if (*mx == FLT_MAX)
			*mx = hi;
	}
	if (*mx - *mn < 1e-6f)
		*mx = *mn + 1.0f;
}
} // namespace

void plot_lines(const char* label, const float* values, int count, float mn, float mx, float height)
{
	if (!search::filter(label) || count < 2)
		return;
	ImGui::PushID(label);
	PlotArea a = plot_frame(label, height);
	ImGui::PopID();
	value_range(values, count, &mn, &mx);

	const Theme& t = theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const float pad = 3.0f;
	const float w = a.max.x - a.min.x - pad * 2.0f;
	const float h = a.max.y - a.min.y - pad * 2.0f;
	auto point = [&](int i)
	{
		float x = a.min.x + pad + w * (float)i / (float)(count - 1);
		float y = a.max.y - pad - h * saturate((values[i] - mn) / (mx - mn));
		return ImVec2(x, y);
	};

	dl->PushClipRect(a.min, a.max, true);
	// Soft fill under the curve.
	ImU32 fill_top = styled(t.accent_col, 0.18f), fill_bot = styled(t.accent_col, 0.0f);
	for (int i = 0; i < count - 1; ++i)
	{
		ImVec2 p0 = point(i), p1 = point(i + 1);
		dl->AddRectFilledMultiColor(ImVec2(p0.x, std::min(p0.y, p1.y)), ImVec2(p1.x, a.max.y - pad), fill_top, fill_top,
																fill_bot, fill_bot);
	}
	for (int i = 0; i < count - 1; ++i)
		dl->AddLine(point(i), point(i + 1), styled(t.accent_col), 1.5f);
	dl->PopClipRect();
}

void plot_histogram(const char* label, const float* values, int count, float mn, float mx, float height)
{
	if (!search::filter(label) || count < 1)
		return;
	ImGui::PushID(label);
	PlotArea a = plot_frame(label, height);
	ImGui::PopID();
	if (mn == FLT_MAX)
		mn = std::min(0.0f, *std::min_element(values, values + count));
	value_range(values, count, &mn, &mx);

	const Theme& t = theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const float pad = 3.0f;
	const float w = a.max.x - a.min.x - pad * 2.0f;
	const float h = a.max.y - a.min.y - pad * 2.0f;
	const float bar = w / (float)count;
	for (int i = 0; i < count; ++i)
	{
		float k = saturate((values[i] - mn) / (mx - mn));
		ImVec2 bmin(a.min.x + pad + bar * i + 1.0f, a.max.y - pad - h * k);
		ImVec2 bmax(a.min.x + pad + bar * (i + 1) - 1.0f, a.max.y - pad);
		if (bmax.x > bmin.x)
			dl->AddRectFilled(bmin, bmax, styled(lerp_color(t.accent_dim, t.accent_col, k)), 1.0f);
	}
}

void animation_preview(const char* id_str, float height)
{
	if (search::dry_run())
		return;
	constexpr int k_samples = 120;
	static std::unordered_map<ImGuiID, std::vector<float>> history;

	ImGui::PushID(id_str);
	ImGuiID id = ImGui::GetID("##ap");
	float target = std::fmod((float)ImGui::GetTime(), 2.5f) < 1.25f ? 1.0f : 0.0f;
	float val = anim::animate(id, target);

	PlotArea a = plot_frame(nullptr, height);
	ImGui::PopID();

	auto& hist = history[id];
	if (hist.size() != k_samples)
		hist.assign(k_samples, 0.0f);
	std::rotate(hist.begin(), hist.begin() + 1, hist.end());
	hist.back() = val;

	const Theme& t = theme();
	ImDrawList* dl = ImGui::GetWindowDrawList();
	const float w = a.max.x - a.min.x;
	const float h = a.max.y - a.min.y;
	auto y_of = [&](float v) { return a.max.y - 3.0f - v * (h - 6.0f); };
	for (int i = 0; i < k_samples - 1; ++i)
	{
		float x0 = a.min.x + w * (float)i / (k_samples - 1);
		float x1 = a.min.x + w * (float)(i + 1) / (k_samples - 1);
		dl->AddLine(ImVec2(x0, y_of(hist[i])), ImVec2(x1, y_of(hist[i + 1])), styled(t.accent_col), 1.5f);
	}
	float ty = y_of(target);
	for (float x = a.min.x; x < a.max.x; x += 6.0f)
		dl->AddLine(ImVec2(x, ty), ImVec2(std::min(x + 3.0f, a.max.x), ty), styled(t.accent_col, 0.24f), 1.0f);
}

} // namespace nc
