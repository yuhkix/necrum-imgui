#pragma once
#include "../pch.h"

namespace renderer
{

class Renderer
{
public:
	bool init();
	bool begin_frame();
	void end_frame();
	void shutdown();

	static ID3D11Device* get_device() { return p_device_static; }

private:
	static inline ID3D11Device* p_device_static = nullptr;
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool create_app_window();
	bool create_device();
	void create_render_target();
	void cleanup_render_target();

	HWND h_hwnd = nullptr;
	WNDCLASSEXW wc = {};
	ID3D11Device* p_device = nullptr;
	ID3D11DeviceContext* p_context = nullptr;
	IDXGISwapChain* p_swap_chain = nullptr;
	ID3D11RenderTargetView* p_rtv = nullptr;
};

} // namespace renderer
