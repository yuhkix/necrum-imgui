// Standalone host window using GLFW + OpenGL 3. This is the Linux backend and
// also builds on Windows (xmake f --backend=glfw).

#include "standalone.h"

#include "necrum/extras/web_image.h"
#include "necrum/platform/host.h"

#include "ext/imgui/backends/imgui_impl_glfw.h"
#include "ext/imgui/backends/imgui_impl_opengl3.h"
#include "ext/imgui/imgui.h"

// xmake's glfw package defines GLFW_INCLUDE_NONE, which stops glfw3.h from
// pulling in the system OpenGL header. The texture helpers below call GL 1.1
// directly, so let GLFW include it (it also handles the Windows prerequisites).
#undef GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstdint>
#include <cstdio>

namespace renderer
{

namespace
{
void glfw_error(int code, const char* description)
{
	fprintf(stderr, "[necrum] GLFW error %d: %s\n", code, description);
}

ImTextureID create_texture(const unsigned char* pixels, int width, int height)
{
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	return (ImTextureID)(uintptr_t)tex;
}

void release_texture(ImTextureID tex)
{
	GLuint id = (GLuint)(uintptr_t)tex;
	glDeleteTextures(1, &id);
}
} // namespace

int run_standalone()
{
	glfwSetErrorCallback(glfw_error);
	if (!glfwInit())
		return 1;

	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	GLFWwindow* window = glfwCreateWindow(1280, 800, "necrum", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	nc::host::setup(io);

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	nc::web_image::set_texture_callbacks(create_texture, release_texture);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
		{
			glfwWaitEventsTimeout(0.1);
			continue;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		nc::host::frame();
		ImGui::Render();

		int w = 0, h = 0;
		glfwGetFramebufferSize(window, &w, &h);
		glViewport(0, 0, w, h);
		glClearColor(0.025f, 0.025f, 0.035f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	nc::host::shutdown();
	nc::web_image::shutdown();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

} // namespace renderer
