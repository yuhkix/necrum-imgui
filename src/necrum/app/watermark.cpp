#include "watermark.h"

#include "necrum/core/draw.h"
#include "necrum/core/fonts.h"
#include "necrum/core/theme.h"

#include <chrono>
#include <ctime>
#include <unordered_map>

namespace nc
{

std::string fps_text()
{
	char buf[32];
	snprintf(buf, sizeof(buf), "%03.0f fps", ImGui::GetIO().Framerate);
	return buf;
}

std::string clock_text()
{
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::tm tm_info{};
#ifdef _WIN32
	localtime_s(&tm_info, &now);
#else
	localtime_r(&now, &tm_info);
#endif
	char buf[16];
	std::strftime(buf, sizeof(buf), "%H:%M", &tm_info);
	return buf;
}

void watermark(const char* title, const std::function<void(std::vector<std::string>&)>& segments,
							 const WatermarkOptions& o)
{
	struct Cache
	{
		std::string text;
		double updated = -1.0;
	};
	static std::unordered_map<ImGuiID, Cache> caches;

	const Theme& t = theme();
	Cache& c = caches[ImHashStr(title)];
	double now = ImGui::GetTime();
	if (c.updated < 0.0 || now - c.updated >= o.refresh_interval)
	{
		std::vector<std::string> parts;
		if (segments)
			segments(parts);
		c.text.clear();
		for (const auto& p : parts)
		{
			if (!c.text.empty())
				c.text += " | ";
			c.text += p;
		}
		c.updated = now;
	}

	ImFont* bold = fonts::bold();
	const float fs = ImGui::GetFontSize();
	const char* sep = c.text.empty() ? "" : " | ";
	float w_title = draw::text_size(bold, bold->LegacySize, title).x;
	float w_sep = ImGui::CalcTextSize(sep).x;
	float w_text = ImGui::CalcTextSize(c.text.c_str()).x;

	const ImVec2 pad(8.0f, 4.0f);
	ImVec2 size(w_title + w_sep + w_text + pad.x * 2.0f, fs + pad.y * 2.0f);
	ImVec2 pos = anchor_position(o.corner, size, o.margin);
	ImVec2 max(pos.x + size.x, pos.y + size.y);

	ImDrawList* dl = ImGui::GetForegroundDrawList();
	dl->AddRectFilled(pos, max, styled(t.header_bg), 4.0f, ImDrawFlags_RoundCornersTop);
	dl->AddRect(pos, max, styled(t.field_border), 4.0f, ImDrawFlags_RoundCornersTop);
	draw::underline_glow(dl, ImVec2(pos.x + 1.0f, pos.y), ImVec2(max.x - 1.0f, max.y), t.accent_col, 1.0f);

	float x = pos.x + pad.x, y = pos.y + pad.y - 1.0f;
	dl->AddText(bold, bold->LegacySize, ImVec2(x, y), styled(t.accent_col), title);
	x += w_title;
	dl->AddText(ImVec2(x, y), styled(t.text_disabled), sep);
	x += w_sep;
	dl->AddText(ImVec2(x, y), styled(t.text_dim), c.text.c_str());
}

} // namespace nc
