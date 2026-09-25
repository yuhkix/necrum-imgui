#pragma once

// Standalone (non-injected) host window. Implemented by exactly one backend per
// target: standalone_dx11 (Windows) or standalone_glfw (Linux, optionally Windows).
namespace renderer
{
int run_standalone();
} // namespace renderer
