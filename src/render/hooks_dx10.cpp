#include "hooks.h"
#include "dx10_renderer.h"
#include "../menu/menu.h"

#include "../ext/minhook/MinHook.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer
{

namespace
{
typedef HRESULT(WINAPI* tPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
tPresent oPresent = nullptr;

WNDPROC oWndProc = nullptr;
DX10Renderer renderer;
menu::Menu menu;
bool hooks_enabled = true;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!hooks_enabled)
		return CallWindowProcW(oWndProc, hWnd, msg, wParam, lParam);

	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

	if (menu.is_visible())
	{
		ImGuiIO& io = ImGui::GetIO();

		if (msg == WM_SETCURSOR && LOWORD(lParam) == HTCLIENT)
		{
			return TRUE;
		}

		switch (msg)
		{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_MOUSEHWHEEL:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_CHAR:
			return TRUE;
		}
	}

	return CallWindowProcW(oWndProc, hWnd, msg, wParam, lParam);
}

HRESULT WINAPI h_Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!hooks_enabled)
		return oPresent(pSwapChain, SyncInterval, Flags);

	if (!renderer.is_initialized())
	{
		if (renderer.init(pSwapChain))
		{
			DXGI_SWAP_CHAIN_DESC desc;
			pSwapChain->GetDesc(&desc);
			oWndProc = (WNDPROC)SetWindowLongPtrW(desc.OutputWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
		}
	}

	if (renderer.is_initialized())
	{
		renderer.begin_frame();
		menu.render();
		renderer.end_frame();
	}

	return oPresent(pSwapChain, SyncInterval, Flags);
}
} // namespace

bool Hooks::init()
{
	if (MH_Initialize() != MH_OK)
		return false;

	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = GetForegroundWindow();
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;

	ID3D10Device* p_device = nullptr;
	IDXGISwapChain* p_swap_chain = nullptr;

	HRESULT hr = D3D10CreateDeviceAndSwapChain(nullptr, D3D10_DRIVER_TYPE_HARDWARE, nullptr, 0, D3D10_SDK_VERSION, &sd, &p_swap_chain, &p_device);
	if (FAILED(hr))
		return false;

	void** p_vtable = *(void***)p_swap_chain;
	void* p_present = p_vtable[8];

	p_swap_chain->Release();
	p_device->Release();

	if (MH_CreateHook(p_present, &h_Present, (LPVOID*)&oPresent) != MH_OK)
		return false;

	if (MH_EnableHook(p_present) != MH_OK)
		return false;

	return true;
}

void Hooks::shutdown()
{
	hooks_enabled = false;
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
	renderer.shutdown();
}

} // namespace renderer
