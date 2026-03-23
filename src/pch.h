#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <dwmapi.h>
#ifdef USE_VULKAN
#include <vulkan/vulkan.h>
#endif

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <cstdint>
