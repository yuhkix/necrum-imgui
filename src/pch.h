#pragma once

// Precompiled header for the Windows render backends and hooks (not used by src/necrum).

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <dwmapi.h>
#ifdef USE_VULKAN
#include "ext/imgui/backends/vulkan/vulkan.h"
#endif

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <cstdint>
