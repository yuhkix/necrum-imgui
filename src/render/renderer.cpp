#include "renderer.h"
#include "menu/theme.h"
#include "pch.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer
{

LRESULT CALLBACK Renderer::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (auto* self = reinterpret_cast<Renderer*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA)))
		{
			if (self->p_device && wParam != SIZE_MINIMIZED)
			{
				self->cleanup_render_target();
				self->p_swap_chain->ResizeBuffers(0, static_cast<UINT>(LOWORD(lParam)), static_cast<UINT>(HIWORD(lParam)),
																					DXGI_FORMAT_UNKNOWN, 0);
				self->create_render_target();
			}
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool Renderer::create_app_window()
{
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandleW(nullptr);
	wc.lpszClassName = L"RubyWindow";
	wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));

	RegisterClassExW(&wc);

	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);

	h_hwnd = CreateWindowExW(WS_EX_TOPMOST, wc.lpszClassName, L"necrum", WS_POPUP, 0, 0, sw, sh, nullptr,
													 nullptr, wc.hInstance, nullptr);
	if (!h_hwnd)
		return false;

	SetWindowLongPtrW(h_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	BOOL dark = TRUE;
	DwmSetWindowAttribute(h_hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &dark, sizeof(dark));

	ShowWindow(h_hwnd, SW_SHOWMAXIMIZED);
	UpdateWindow(h_hwnd);
	return true;
}

bool Renderer::create_device()
{
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 2;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate = {60, 1};
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = h_hwnd;
	sd.SampleDesc = {1, 0};
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	constexpr D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, levels, 1,
																						 D3D11_SDK_VERSION, &sd, &p_swap_chain, &p_device, nullptr, &p_context);
	if (FAILED(hr))
		return false;

	p_device_static = p_device;

	create_render_target();
	return true;
}

void Renderer::create_render_target()
{
	ID3D11Texture2D* back = nullptr;
	p_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back));
	p_device->CreateRenderTargetView(back, nullptr, &p_rtv);
	back->Release();
}

void Renderer::cleanup_render_target()
{
	if (p_rtv)
	{
		p_rtv->Release();
		p_rtv = nullptr;
	}
}

bool Renderer::init()
{
	if (!create_app_window())
		return false;
	if (!create_device())
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;

	theme::LoadFonts(io);

	ImGui_ImplWin32_Init(h_hwnd);
	ImGui_ImplDX11_Init(p_device, p_context);

	theme::Apply();

	return true;
}

bool Renderer::begin_frame()
{
	MSG msg;
	while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
		if (msg.message == WM_QUIT)
			return false;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	return true;
}

void Renderer::end_frame()
{
	ImGui::Render();

	constexpr float clear[4] = {0.025f, 0.025f, 0.035f, 1.0f};
	p_context->OMSetRenderTargets(1, &p_rtv, nullptr);
	p_context->ClearRenderTargetView(p_rtv, clear);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	p_swap_chain->Present(1, 0); // vsync on
}

void Renderer::shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	cleanup_render_target();
	if (p_swap_chain)
	{
		p_swap_chain->Release();
		p_swap_chain = nullptr;
	}
	if (p_context)
	{
		p_context->Release();
		p_context = nullptr;
	}
	if (p_device)
	{
		p_device->Release();
		p_device = nullptr;
	}

	DestroyWindow(h_hwnd);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

} // namespace renderer
