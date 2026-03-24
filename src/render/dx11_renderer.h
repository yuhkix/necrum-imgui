#pragma once
#include "../pch.h"
#include <d3d11.h>

namespace renderer
{

class DX11Renderer
{
public:
	bool init(IDXGISwapChain* swap_chain);
	void begin_frame();
	void end_frame();
	void shutdown();

	bool is_initialized() const { return initialized; }

private:
	HWND h_hwnd = nullptr;
	ID3D11Device* p_device = nullptr;
	ID3D11DeviceContext* p_context = nullptr;
	ID3D11RenderTargetView* p_rtv = nullptr;
	bool initialized = false;

	void create_render_target(IDXGISwapChain* swap_chain);
	void cleanup_render_target();
};

} // namespace renderer
