#include "hooks.h"
#include "opengl_renderer.h"
#include "../menu/menu.h"

#include "../ext/minhook/MinHook.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer
{

namespace
{
typedef BOOL(WINAPI* twglSwapBuffers)(HDC hdc);
twglSwapBuffers owglSwapBuffers = nullptr;

WNDPROC oWndProc = nullptr;
OpenGLRenderer renderer;
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

BOOL WINAPI h_wglSwapBuffers(HDC hdc)
{
	if (!hooks_enabled)
		return owglSwapBuffers(hdc);

	static bool initialized = false;
	if (!initialized)
	{
		HWND hwnd = WindowFromDC(hdc);
		if (hwnd)
		{
			if (renderer.init(hwnd))
			{
				oWndProc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
				initialized = true;
			}
		}
	}

	if (initialized)
	{
		renderer.begin_frame();
		menu.render();
		renderer.end_frame();
	}

	return owglSwapBuffers(hdc);
}
} // namespace

bool Hooks::init()
{
	if (MH_Initialize() != MH_OK)
		return false;

	HMODULE h_opengl = GetModuleHandleW(L"opengl32.dll");
	if (!h_opengl)
		return false;

	void* p_wglSwapBuffers = (void*)GetProcAddress(h_opengl, "wglSwapBuffers");
	if (!p_wglSwapBuffers)
		return false;

	if (MH_CreateHook(p_wglSwapBuffers, &h_wglSwapBuffers, (LPVOID*)&owglSwapBuffers) != MH_OK)
		return false;

	if (MH_EnableHook(p_wglSwapBuffers) != MH_OK)
		return false;

	return true;
}

void Hooks::shutdown()
{
	hooks_enabled = false;

	// In a real DLL, we might want to restore WndProc and cleanup renderer
	// but for this implementation we'll keep it simple as a shutdown flag.
	MH_DisableHook(MH_ALL_HOOKS);
	MH_Uninitialize();

	renderer.shutdown();
}

} // namespace renderer
