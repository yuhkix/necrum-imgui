#include "splash.h"

#include "necrum/core/anim.h"
#include "necrum/core/draw.h"
#include "necrum/core/fonts.h"
#include "necrum/core/theme.h"
#include "necrum/extras/web_image.h"

namespace nc
{

namespace
{
double g_start = -1.0;
bool g_done = false;
} // namespace

void reset_splash()
{
	g_start = -1.0;
	g_done = false;
}

bool splash(const SplashOptions& o)
{
	if (g_done)
		return false;

	const Theme& t = theme();
	const double time = ImGui::GetTime();
	if (g_start < 0.0)
		g_start = time;
	const float elapsed = (float)(time - g_start);

	// Timeline, scaled to the requested duration.
	const float k = o.duration / 3.25f;
	const float appear_end = 0.65f * k, reveal_start = 0.22f * k, reveal_end = 1.45f * k;
	const float hold_end = 2.40f * k, out_end = 3.15f * k;

	if (elapsed >= o.duration)
	{
		g_done = true;
		return false;
	}

	const float appear = anim::smoothstep(0.0f, appear_end, elapsed);
	const float reveal = anim::smoothstep(reveal_start, reveal_end, elapsed);
	const float out = anim::smoothstep(hold_end, out_end, elapsed);
	const float alpha = 1.0f - out * out;
	if (alpha <= 0.001f)
	{
		g_done = true;
		return false;
	}

	ImDrawList* dl = ImGui::GetForegroundDrawList();
	const ImVec2 display = ImGui::GetIO().DisplaySize;
	const ImVec2 center(display.x * 0.5f, display.y * 0.5f);

	if (o.dim_background)
	{
		ImU32 top = with_alpha(t.window_bg, alpha), bottom = with_alpha(lerp_color(t.window_bg, IM_COL32_BLACK, 0.4f), alpha);
		dl->AddRectFilledMultiColor(ImVec2(0, 0), display, top, top, bottom, bottom);
		ImVec2 ambient(display.x * 0.72f, display.y * 0.28f);
		for (int i = 0; i < 3; ++i)
			dl->AddCircleFilled(ambient, 230.0f + 90.0f * i, with_alpha(t.accent_col, 0.030f * (1.0f - i / 2.0f) * alpha), 72);
	}

	const float scale = 0.975f + 0.025f * appear;
	const float panel_alpha = appear * alpha;
	const float y_off = (1.0f - appear) * 10.0f - out * 6.0f;
	const float pw = 324.0f * scale, ph = 146.0f * scale, header = 30.0f * scale;
	ImVec2 pmin(center.x - pw * 0.5f, center.y - ph * 0.5f + y_off);
	ImVec2 pmax(pmin.x + pw, pmin.y + ph);

	dl->AddRectFilled(ImVec2(pmin.x - 8.0f, pmin.y - 8.0f), ImVec2(pmax.x + 8.0f, pmax.y + 8.0f),
										with_alpha(t.shadow, 0.14f * panel_alpha), 8.0f);
	dl->AddRectFilled(pmin, ImVec2(pmax.x, pmin.y + header), alpha_mul(t.header_bg, panel_alpha), 4.0f,
										ImDrawFlags_RoundCornersTop);
	dl->AddRectFilled(ImVec2(pmin.x, pmin.y + header), pmax, alpha_mul(t.panel_bg, panel_alpha), 4.0f,
										ImDrawFlags_RoundCornersBottom);
	dl->AddRect(pmin, pmax, alpha_mul(t.field_border, panel_alpha), 4.0f);
	dl->AddLine(ImVec2(pmin.x + 1.0f, pmin.y + header - 1.0f), ImVec2(pmax.x - 1.0f, pmin.y + header - 1.0f),
							alpha_mul(t.panel_border, panel_alpha * 0.6f), 1.0f);

	const float logo_alpha = reveal * alpha;
	const float logo_y = pmin.y + 66.0f * scale + (1.0f - reveal) * 5.0f - out * 2.0f;
	if (logo_alpha > 0.01f)
	{
		ImTextureID tex = o.logo_texture;
		if (!tex && !o.logo_url.empty())
		{
			tex = web_image::get(o.logo_url); // starts the download on first use
			if (!web_image::is_loaded(o.logo_url))
				tex = 0;
		}
		const float ls = 56.0f * scale;
		if (tex)
		{
			ImVec2 lp(center.x - ls * 0.5f, logo_y - ls * 0.5f);
			dl->AddImageRounded(tex, lp, ImVec2(lp.x + ls, lp.y + ls), ImVec2(0, 0), ImVec2(1, 1),
													IM_COL32(255, 255, 255, (int)(255.0f * logo_alpha)), 12.0f * scale);
		}
		else if (o.logo_icon)
		{
			ImFont* f = fonts::icons();
			ImVec2 is = draw::text_size(f, ls * 0.6f, o.logo_icon);
			dl->AddText(f, ls * 0.6f, ImVec2(center.x - is.x * 0.5f, logo_y - is.y * 0.5f),
									with_alpha(t.accent_col, logo_alpha), o.logo_icon);
		}

		ImFont* f = fonts::title();
		const float fs = 22.0f * scale;
		ImVec2 ts = draw::text_size(f, fs, o.title.c_str());
		ImVec2 tp(center.x - ts.x * 0.5f, logo_y + 36.0f * scale);
		dl->AddText(f, fs, ImVec2(tp.x, tp.y + 1.0f), with_alpha(t.shadow, 0.42f * logo_alpha), o.title.c_str());
		float clip_w = ts.x * reveal;
		if (clip_w > 0.5f)
		{
			dl->PushClipRect(tp, ImVec2(tp.x + clip_w, tp.y + ts.y + 2.0f), true);
			dl->AddText(f, fs, tp, alpha_mul(t.text, logo_alpha), o.title.c_str());
			dl->PopClipRect();
		}
	}

	// Accent line + glow along the bottom edge.
	const float line_t = anim::smoothstep(0.48f * k, reveal_end, elapsed);
	const float lw = (pw - 72.0f * scale) * line_t;
	const float ly = pmax.y - 14.0f * scale;
	if (lw > 0.5f)
	{
		const float breathe = 0.86f + 0.14f * std::sin((float)time * 2.1f);
		ImU32 g = with_alpha(t.accent_col, 0.14f * alpha), clear = with_alpha(t.accent_col, 0.0f);
		dl->AddRectFilledMultiColor(ImVec2(center.x - lw * 0.5f, ly - 10.0f), ImVec2(center.x + lw * 0.5f, ly - 1.0f), clear,
																clear, g, g);
		dl->AddLine(ImVec2(center.x - lw * 0.5f, ly - 1.0f), ImVec2(center.x + lw * 0.5f, ly - 1.0f),
								with_alpha(t.accent_col, 0.9f * alpha * breathe), 1.0f);
	}
	return true;
}

} // namespace nc
