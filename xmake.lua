set_project("necrum")
set_version("2.0.0")
set_languages("c++20")
set_warnings("all")

add_rules("mode.debug", "mode.release")

-- Which example application is compiled into the executables / DLLs.
--   xmake f --app=showcase     (default, framework tour)
--   xmake f --app=game_menu    (the original necrum menu rebuilt on the framework)
option("app")
	set_default("showcase")
	set_showmenu(true)
	set_description("Application from examples/<app> linked into every target")
option_end()

local is_windows = is_plat("windows", "mingw")

if is_windows then
	add_requires("vulkan-headers", "volk")
else
	add_requires("glfw")
end

if is_mode("release") then
	set_optimize("fastest")
end

-- Settings shared by every target.
local function necrum_defaults()
	add_includedirs("src", "src/ext/imgui", "src/ext/imgui/backends")
	if is_plat("windows") then
		add_cxflags("/utf-8", "/bigobj")
		add_defines("NOMINMAX", "WIN32_LEAN_AND_MEAN", "_CRT_SECURE_NO_WARNINGS")
		set_runtimes(is_mode("debug") and "MDd" or "MD")
	end
end

-- The example app sources are added to final binaries (not to a library) so
-- that NC_REGISTER_APP's nc::create_app() is always linked in.
local function add_app()
	add_files(path.join("examples", "$(app)", "**.cpp"))
end

-- Framework: Dear ImGui core + src/necrum. Portable (Windows / Linux).
target("necrum_core")
	set_kind("static")
	necrum_defaults()
	add_files("src/ext/imgui/imgui.cpp", "src/ext/imgui/imgui_draw.cpp", "src/ext/imgui/imgui_widgets.cpp",
		"src/ext/imgui/imgui_tables.cpp")
	add_files("src/necrum/**.cpp")
	add_headerfiles("src/necrum/**.h")
	if is_windows then
		add_syslinks("wininet", {public = true})
	else
		add_syslinks("pthread", {public = true})
	end
target_end()

-- Standalone desktop app: DX11 window on Windows, GLFW + OpenGL 3 on Linux.
target("necrum")
	set_kind("binary")
	necrum_defaults()
	add_deps("necrum_core")
	add_files("src/main.cpp")
	add_app()
	if is_windows then
		add_files("src/render/standalone_dx11.cpp")
		add_files("src/ext/imgui/backends/imgui_impl_win32.cpp", "src/ext/imgui/backends/imgui_impl_dx11.cpp")
		add_syslinks("d3d11", "dxgi", "d3dcompiler", "dwmapi", "user32", "gdi32", "advapi32")
		if is_plat("windows") then
			add_ldflags("/SUBSYSTEM:WINDOWS", "/ENTRY:mainCRTStartup", {force = true})
		else
			add_ldflags("-mwindows", {force = true})
		end
	else
		add_packages("glfw")
		add_files("src/render/standalone_glfw.cpp")
		add_files("src/ext/imgui/backends/imgui_impl_glfw.cpp", "src/ext/imgui/backends/imgui_impl_opengl3.cpp")
		add_syslinks("GL", "dl")
	end
target_end()

-- Headless smoke test: fuzzes the selected app without any window or GPU.
--   xmake build necrum_headless && xmake run necrum_headless 3000
target("necrum_headless")
	set_kind("binary")
	set_default(false)
	necrum_defaults()
	add_deps("necrum_core")
	add_files("tests/headless.cpp")
	add_app()
target_end()

-- Injectable in-process overlays (Windows only): one DLL per graphics API.
if is_windows then
	local function hook_target(name, backend_files, renderer_file, hook_file, links)
		target(name)
			set_kind("shared")
			necrum_defaults()
			add_includedirs("src/ext/minhook")
			add_deps("necrum_core")
			add_files("src/dllmain.cpp", renderer_file, hook_file)
			add_files("src/ext/minhook/*.c", "src/ext/minhook/hde/*.c")
			add_files("src/ext/imgui/backends/imgui_impl_win32.cpp")
			add_files(table.unpack(backend_files))
			add_app()
			add_syslinks("dwmapi", "user32", "gdi32", "advapi32", table.unpack(links))
		target_end()
	end

	hook_target("necrum_dx9", {"src/ext/imgui/backends/imgui_impl_dx9.cpp"}, "src/render/dx9_renderer.cpp",
		"src/render/hooks_dx9.cpp", {"d3d9"})
	hook_target("necrum_dx10", {"src/ext/imgui/backends/imgui_impl_dx10.cpp"}, "src/render/dx10_renderer.cpp",
		"src/render/hooks_dx10.cpp", {"d3d10", "dxgi", "d3dcompiler"})
	hook_target("necrum_dx11", {"src/ext/imgui/backends/imgui_impl_dx11.cpp"}, "src/render/dx11_renderer.cpp",
		"src/render/hooks_dx11.cpp", {"d3d11", "dxgi", "d3dcompiler"})
	hook_target("necrum_dx12", {"src/ext/imgui/backends/imgui_impl_dx12.cpp"}, "src/render/dx12_renderer.cpp",
		"src/render/hooks_dx12.cpp", {"d3d12", "dxgi", "d3dcompiler"})
	hook_target("necrum_gl", {"src/ext/imgui/backends/imgui_impl_opengl3.cpp"}, "src/render/opengl_renderer.cpp",
		"src/render/hooks_gl.cpp", {"opengl32"})
	hook_target("necrum_vk", {"src/ext/imgui/backends/imgui_impl_vulkan.cpp"}, "src/render/vulkan_renderer.cpp",
		"src/render/hooks_vk.cpp", {})

	target("necrum_vk")
		add_packages("vulkan-headers", "volk")
		add_defines("USE_VULKAN", "IMGUI_IMPL_VULKAN_USE_VOLK", "VK_NO_PROTOTYPES", "IMGUI_IMPL_VULKAN_NO_PROTOTYPES")
	target_end()
end
