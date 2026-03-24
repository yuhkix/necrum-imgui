#include "opengl_renderer.h"
#include "menu/theme.h"
#include "pch.h"
#include "core/web_image.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <GL/gl.h>
#include <imgui_impl_win32.h>

namespace renderer
{

bool OpenGLRenderer::init(HWND hwnd)
{
	if (initialized)
		return true;

	h_hwnd = hwnd;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.IniFilename = nullptr;

	theme::LoadFonts(io);

	if (!ImGui_ImplWin32_Init(h_hwnd))
		return false;

	if (!ImGui_ImplOpenGL3_Init("#version 130"))
		return false;

	theme::Apply();

	web_image::set_texture_create_callback([](unsigned char* pixels, int width, int height) -> ImTextureID {
		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		return (ImTextureID)(intptr_t)tex;
	});

	initialized = true;
	return true;
}

void OpenGLRenderer::begin_frame()
{
	if (!initialized)
		return;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void OpenGLRenderer::end_frame()
{
	if (!initialized)
		return;

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void OpenGLRenderer::shutdown()
{
	if (!initialized)
		return;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	initialized = false;
}

} // namespace renderer
