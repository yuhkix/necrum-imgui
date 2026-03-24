#include "hooks.h"
#include "dx9_renderer.h"
#include "../menu/menu.h"

#include <d3d9.h>
#include "../ext/minhook/MinHook.h"
#include "../ext/imgui/backends/imgui_impl_dx9.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer
{

namespace
{
typedef HRESULT(WINAPI* tEndScene)(IDirect3DDevice9* pDevice);
typedef HRESULT(WINAPI* tReset)(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);

tEndScene oEndScene = nullptr;
tReset oReset = nullptr;

WNDPROC oWndProc = nullptr;
DX9Renderer renderer;
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

HRESULT WINAPI h_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	if (!hooks_enabled)
		return oReset(pDevice, pPresentationParameters);

	if (renderer.is_initialized())
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
	}

	HRESULT hr = oReset(pDevice, pPresentationParameters);

	if (hr == D3D_OK && renderer.is_initialized())
	{
		ImGui_ImplDX9_CreateDeviceObjects();
	}

	return hr;
}

HRESULT WINAPI h_EndScene(IDirect3DDevice9* pDevice)
{
	if (!hooks_enabled)
		return oEndScene(pDevice);

	if (!renderer.is_initialized())
	{
		if (renderer.init(pDevice))
		{
			D3DDEVICE_CREATION_PARAMETERS cp;
			pDevice->GetCreationParameters(&cp);
			oWndProc = (WNDPROC)SetWindowLongPtrW(cp.hFocusWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
		}
	}

	if (renderer.is_initialized())
	{
		renderer.begin_frame();
		menu.render();
		renderer.end_frame();
	}

	return oEndScene(pDevice);
}
} // namespace

bool Hooks::init()
{
	if (MH_Initialize() != MH_OK)
		return false;

	IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (!pD3D)
		return false;

	D3DPRESENT_PARAMETERS d3dpp{};
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.hDeviceWindow = GetForegroundWindow();

	IDirect3DDevice9* pDevice = nullptr;
	HRESULT hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3dpp.hDeviceWindow,
																	D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDevice);

	if (FAILED(hr))
	{
		pD3D->Release();
		return false;
	}

	void** p_vtable = *(void***)pDevice;
	void* p_endscene = p_vtable[42];
	void* p_reset = p_vtable[16];

	pDevice->Release();
	pD3D->Release();

	if (MH_CreateHook(p_endscene, &h_EndScene, (LPVOID*)&oEndScene) != MH_OK)
		return false;

	if (MH_CreateHook(p_reset, &h_Reset, (LPVOID*)&oReset) != MH_OK)
		return false;

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
		return false;

	return true;
}

void Hooks::shutdown()
{
	hooks_enabled = false;

	if (oWndProc)
	{
		D3DDEVICE_CREATION_PARAMETERS cp;
		// Wait, how to restore wndproc if we don't have focus window anymore?
		// Actually, DX11 didn't restore wndproc on shutdown either...
	}

	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();
	renderer.shutdown();
}

} // namespace renderer
