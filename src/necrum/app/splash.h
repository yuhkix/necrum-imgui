#pragma once

#include "necrum/core/base.h"

namespace nc
{

struct SplashOptions
{
	std::string title = "necrum";
	std::string logo_url;					// optional, loaded through nc::web_image
	ImTextureID logo_texture = 0; // takes precedence over logo_url
	const char* logo_icon = nullptr; // Font Awesome icon used when no texture is available
	float duration = 3.25f;				// seconds, including fade out
	bool dim_background = true;
};

// Animated intro card drawn on the foreground layer. Returns true while playing.
bool splash(const SplashOptions& options);

// Restarts the splash on the next call.
void reset_splash();

} // namespace nc
