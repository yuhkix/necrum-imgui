#include "hooks.h"
#include "dx12_renderer.h"
#include "../menu/menu.h"

#include "../ext/minhook/MinHook.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace renderer
{

namespace
{
typedef HRESULT(WINAPI* tPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
tPresent oPresent = nullptr;

typedef void(WINAPI* tExecuteCommandLists)(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
tExecuteCommandLists oExecuteCommandLists = nullptr;

WNDPROC oWndProc = nullptr;
DX12Renderer renderer;
menu::Menu menu;
bool hooks_enabled = true;
ID3D12CommandQueue* p_captured_command_queue = nullptr;

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

void WINAPI h_ExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
	if (pCommandQueue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_DIRECT)
	{
		p_captured_command_queue = pCommandQueue;
	}

	return oExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

HRESULT WINAPI h_Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!hooks_enabled)
		return oPresent(pSwapChain, SyncInterval, Flags);

	if (!renderer.is_initialized() && p_captured_command_queue)
	{
		IDXGISwapChain3* pSwapChain3 = nullptr;
		if (SUCCEEDED(pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain3))))
		{
			if (renderer.init(pSwapChain3, p_captured_command_queue))
			{
				DXGI_SWAP_CHAIN_DESC desc;
				pSwapChain->GetDesc(&desc);
				oWndProc = (WNDPROC)SetWindowLongPtrW(desc.OutputWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);
			}
			pSwapChain3->Release();
		}
	}

	if (renderer.is_initialized())
	{
		IDXGISwapChain3* pSwapChain3 = nullptr;
		if (SUCCEEDED(pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain3))))
		{
			renderer.begin_frame();
			menu.render();
			renderer.end_frame(pSwapChain3);
			pSwapChain3->Release();
		}
	}

	return oPresent(pSwapChain, SyncInterval, Flags);
}
} // namespace

bool Hooks::init()
{
	if (MH_Initialize() != MH_OK)
		return false;

	// For DX12, we can't easily create a dummy device in a generic way without knowing the hardware.
	// We'll rely on finding the vtable from an existing device if possible, or use a known signature.
	// Alternatively, we can use a dummy swapchain to find Present, and hook ExecuteCommandLists dynamically.

	// Dummy swapchain to find Present (same as DX11)
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferCount = 2;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = GetForegroundWindow();
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	ID3D12Device* p_device = nullptr;
	ID3D12CommandQueue* p_queue = nullptr;
	IDXGISwapChain* p_swap_chain = nullptr;

	if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&p_device))))
		return false;

	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (FAILED(p_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&p_queue))))
	{
		p_device->Release();
		return false;
	}

	IDXGIFactory4* p_factory = nullptr;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&p_factory))))
	{
		p_device->Release();
		p_queue->Release();
		return false;
	}

	if (FAILED(p_factory->CreateSwapChain(p_queue, &sd, &p_swap_chain)))
	{
		p_factory->Release();
		p_device->Release();
		p_queue->Release();
		return false;
	}

	void** p_swap_vtable = *(void***)p_swap_chain;
	void* p_present = p_swap_vtable[8];

	void** p_queue_vtable = *(void***)p_queue;
	void* p_execute = p_queue_vtable[10];

	p_swap_chain->Release();
	p_queue->Release();
	p_device->Release();
	p_factory->Release();

	if (MH_CreateHook(p_present, &h_Present, (LPVOID*)&oPresent) != MH_OK)
		return false;

	if (MH_CreateHook(p_execute, &h_ExecuteCommandLists, (LPVOID*)&oExecuteCommandLists) != MH_OK)
		return false;

	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
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
