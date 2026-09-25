#pragma once

#include "base.h"

namespace nc
{

// Per-frame framework bookkeeping. Call new_frame() right after
// ImGui::NewFrame() and end_frame() right before ImGui::Render().
// nc::host::frame() does both for you.
void new_frame();
void end_frame();

// Hooks that run inside new_frame()/end_frame(); useful for modules that
// need per-frame updates (overlays, custom HUD layers, ...).
using FrameCallback = std::function<void()>;
int on_new_frame(FrameCallback cb);
int on_end_frame(FrameCallback cb);
void remove_callback(int handle);

} // namespace nc
