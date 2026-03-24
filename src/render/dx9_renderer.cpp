#include "dx9_renderer.h"
#include "menu/theme.h"
#include "pch.h"
#include "core/web_image.h"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

namespace renderer
{

bool DX9Renderer::init(IDirect3DDevice9* device)
{
	if (initialized)
		return true;

	p_device = device;
	
    D3DDEVICE_CREATION_PARAMETERS cp;
    if (FAILED(device->GetCreationParameters(&cp)))
        return false;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;

	theme::LoadFonts(io);

	if (!ImGui_ImplWin32_Init(cp.hFocusWindow))
		return false;

	if (!ImGui_ImplDX9_Init(p_device))
		return false;

	theme::Apply();

	web_image::set_texture_create_callback([this](unsigned char* pixels, int width, int height) -> ImTextureID {
		if (!this->p_device) return 0;

		IDirect3DTexture9* texture = nullptr;
		if (FAILED(this->p_device->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr)))
			return 0;

		D3DLOCKED_RECT rect;
		if (FAILED(texture->LockRect(0, &rect, nullptr, 0)))
		{
			texture->Release();
			return 0;
		}

		unsigned char* dest = static_cast<unsigned char*>(rect.pBits);
		unsigned char* src = pixels;
		for (int y = 0; y < height; ++y)
		{
            for (int x = 0; x < width; ++x)
            {
                dest[x * 4 + 0] = src[x * 4 + 2]; // B
                dest[x * 4 + 1] = src[x * 4 + 1]; // G
                dest[x * 4 + 2] = src[x * 4 + 0]; // R
                dest[x * 4 + 3] = src[x * 4 + 3]; // A
            }
			dest += rect.Pitch;
			src += width * 4;
		}

		texture->UnlockRect(0);
		return (ImTextureID)texture;
	});

	initialized = true;
	return true;
}

void DX9Renderer::begin_frame()
{
	if (!initialized)
		return;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void DX9Renderer::end_frame()
{
	if (!initialized)
		return;

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void DX9Renderer::shutdown()
{
	if (!initialized)
		return;

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	p_device = nullptr;
	initialized = false;
}

} // namespace renderer
