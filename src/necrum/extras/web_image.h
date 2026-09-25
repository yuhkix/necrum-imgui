#pragma once

#include "necrum/core/base.h"

// Asynchronous image loading from URLs (PNG/JPG/... via stb_image).
//
// Downloads run on a worker thread; textures are created on the render thread
// the first time get() is called after the download finished. The renderer
// backend provides texture creation/destruction through set_texture_callbacks().
// Downloading is implemented with WinINet on Windows; on other platforms
// get() always returns the placeholder.
namespace nc::web_image
{

using CreateTexture = std::function<ImTextureID(const unsigned char* rgba, int width, int height)>;
using ReleaseTexture = std::function<void(ImTextureID)>;

void set_texture_callbacks(CreateTexture create, ReleaseTexture release = nullptr);

// Texture for `url`, or a 1x1 placeholder (possibly 0) while loading / on failure.
ImTextureID get(const std::string& url);
bool is_loaded(const std::string& url);
bool has_failed(const std::string& url);

// Releases all textures (call before destroying the renderer device).
void clear();
void shutdown();

} // namespace nc::web_image
