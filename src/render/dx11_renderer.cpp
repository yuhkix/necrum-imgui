#include "dx11_renderer.h"
#include "menu/theme.h"
#include "pch.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

namespace renderer
{

bool DX11Renderer::init(IDXGISwapChain* swap_chain)
{
	if (initialized)
		return true;

	DXGI_SWAP_CHAIN_DESC desc;
	if (FAILED(swap_chain->GetDesc(&desc)))
		return false;

	h_hwnd = desc.OutputWindow;

	if (FAILED(swap_chain->GetDevice(__uuidof(ID3D11Device), (void**)&p_device)))
		return false;

	p_device->GetImmediateContext(&p_context);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;

	theme::LoadFonts(io);

	if (!ImGui_ImplWin32_Init(h_hwnd))
		return false;

	if (!ImGui_ImplDX11_Init(p_device, p_context))
		return false;

	create_render_target(swap_chain);
	theme::Apply();

	initialized = true;
	return true;
}

void DX11Renderer::create_render_target(IDXGISwapChain* swap_chain)
{
	ID3D11Texture2D* back_buffer = nullptr;
	swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
	if (back_buffer)
	{
		p_device->CreateRenderTargetView(back_buffer, nullptr, &p_rtv);
		back_buffer->Release();
	}
}

void DX11Renderer::cleanup_render_target()
{
	if (p_rtv)
	{
		p_rtv->Release();
		p_rtv = nullptr;
	}
}

void DX11Renderer::begin_frame()
{
	if (!initialized)
		return;

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DX11Renderer::end_frame()
{
	if (!initialized)
		return;

	ImGui::Render();
	p_context->OMSetRenderTargets(1, &p_rtv, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void DX11Renderer::shutdown()
{
	if (!initialized)
		return;

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	cleanup_render_target();

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

	initialized = false;
}

} // namespace renderer
