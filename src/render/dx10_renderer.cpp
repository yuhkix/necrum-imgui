#include "dx10_renderer.h"
#include "../menu/theme.h"
#include "../core/web_image.h"

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/backends/imgui_impl_dx10.h"
#include "../ext/imgui/backends/imgui_impl_win32.h"

namespace renderer
{

bool DX10Renderer::init(IDXGISwapChain* swap_chain)
{
	if (initialized)
		return true;

	DXGI_SWAP_CHAIN_DESC desc;
	if (FAILED(swap_chain->GetDesc(&desc)))
		return false;

	h_hwnd = desc.OutputWindow;

	if (FAILED(swap_chain->GetDevice(__uuidof(ID3D10Device), (void**)&p_device)))
		return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;

	theme::LoadFonts(io);

	if (!ImGui_ImplWin32_Init(h_hwnd))
		return false;

	if (!ImGui_ImplDX10_Init(p_device))
		return false;

	create_render_target(swap_chain);
	theme::Apply();

	web_image::set_texture_create_callback(
			[this](unsigned char* pixels, int width, int height) -> ImTextureID
			{
				if (!this->p_device)
					return 0;
				D3D10_TEXTURE2D_DESC desc{};
				desc.Width = width;
				desc.Height = height;
				desc.MipLevels = 1;
				desc.ArraySize = 1;
				desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				desc.SampleDesc.Count = 1;
				desc.Usage = D3D10_USAGE_DEFAULT;
				desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;

				ID3D10Texture2D* texture = nullptr;
				D3D10_SUBRESOURCE_DATA subResource{};
				subResource.pSysMem = pixels;
				subResource.SysMemPitch = width * 4;

				if (FAILED(this->p_device->CreateTexture2D(&desc, &subResource, &texture)))
					return 0;

				ID3D10ShaderResourceView* srv = nullptr;
				if (FAILED(this->p_device->CreateShaderResourceView(texture, nullptr, &srv)))
				{
					texture->Release();
					return 0;
				}

				texture->Release();
				return (ImTextureID)srv;
			});

	initialized = true;
	return true;
}

void DX10Renderer::create_render_target(IDXGISwapChain* swap_chain)
{
	ID3D10Texture2D* back_buffer = nullptr;
	swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
	if (back_buffer)
	{
		p_device->CreateRenderTargetView(back_buffer, nullptr, &p_rtv);
		back_buffer->Release();
	}
}

void DX10Renderer::cleanup_render_target()
{
	if (p_rtv)
	{
		p_rtv->Release();
		p_rtv = nullptr;
	}
}

void DX10Renderer::begin_frame()
{
	if (!initialized)
		return;

	ImGui_ImplDX10_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DX10Renderer::end_frame()
{
	if (!initialized)
		return;

	ImGui::Render();
	p_device->OMSetRenderTargets(1, &p_rtv, nullptr);
	ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());
}

void DX10Renderer::shutdown()
{
	if (!initialized)
		return;

	ImGui_ImplDX10_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	cleanup_render_target();

	if (p_device)
	{
		p_device->Release();
		p_device = nullptr;
	}

	initialized = false;
}

} // namespace renderer
