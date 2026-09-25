#pragma once

#include "necrum/core/base.h"

namespace nc
{

struct WatermarkOptions
{
	Corner corner = Corner::TopRight;
	float margin = 15.0f;
	float refresh_interval = 0.5f; // seconds between segment refreshes (they may be costly)
};

// "title | segment | segment" strip drawn on the foreground layer.
// `segments` is invoked at most every refresh_interval seconds.
void watermark(const char* title, const std::function<void(std::vector<std::string>&)>& segments,
							 const WatermarkOptions& options = {});

// Common segment helpers.
std::string fps_text();		// "144 fps"
std::string clock_text();	// "21:37"

} // namespace nc
