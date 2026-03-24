set_project("necrum")
set_version("1.0.0")
set_languages("c++20")

add_rules("mode.debug", "mode.release")

add_requires("vulkan-headers", "volk")

target("necrum")
    set_kind("binary")
    set_pcheader("src/pch.h")
    add_includedirs("src")

    add_files("src/**.cpp")
    remove_files("src/dllmain.cpp", "src/render/opengl_renderer.cpp", "src/render/hooks_gl.cpp")
    remove_files("src/render/dx9_renderer.cpp", "src/render/hooks_dx9.cpp")
    remove_files("src/render/dx11_renderer.cpp", "src/render/hooks_dx11.cpp")
    remove_files("src/render/vulkan_renderer.cpp", "src/render/hooks_vk.cpp")
    remove_files("src/ext/imgui/backends/imgui_impl_vulkan.cpp", "src/ext/imgui/backends/imgui_impl_opengl3.cpp", "src/ext/imgui/backends/imgui_impl_dx9.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src/ext/imgui", "src/ext/imgui/backends")
    add_files("src/ext/imgui/imgui*.cpp")
    add_files("src/ext/imgui/backends/imgui_impl_win32.cpp", "src/ext/imgui/backends/imgui_impl_dx11.cpp")
    
    add_syslinks("d3d11", "dxgi", "d3dcompiler", "dwmapi", "advapi32", "wininet")
    add_ldflags("/SUBSYSTEM:WINDOWS", "/ENTRY:mainCRTStartup", { force = true })

if is_plat("windows") and is_mode("release") then
    set_runtimes("MD")
    set_targetdir("build/release/")
    set_objectdir("build/release/intermediates/")
    set_dependir("build/release/intermediates/")
    set_optimize("smallest")
    add_cxflags("/bigobj")
else
    set_runtimes("MDd")
    set_targetdir("build/debug/")
    set_objectdir("build/debug/intermediates/")
    set_dependir("build/debug/intermediates/")
end

target("necrum_gl")
    set_kind("shared")
    set_pcheader("src/pch.h")
    
    add_files("src/menu/**.cpp", "src/widgets/**.cpp")
    add_files("src/render/opengl_renderer.cpp", "src/render/hooks_gl.cpp", "src/dllmain.cpp")
    add_files("src/core/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    add_includedirs("src/ext/imgui", "src/ext/imgui/backends", "src/ext/minhook")
    add_files("src/ext/imgui/imgui*.cpp")
    add_files("src/ext/imgui/backends/imgui_impl_win32.cpp", "src/ext/imgui/backends/imgui_impl_opengl3.cpp")
    add_files("src/ext/minhook/*.c", "src/ext/minhook/hde/*.c")
    
    add_syslinks("opengl32", "glu32", "dwmapi", "advapi32", "user32", "gdi32", "wininet")
    
if is_plat("windows") and is_mode("release") then
    set_runtimes("MD")
    set_targetdir("build/release/")
    set_objectdir("build/release/intermediates/")
    set_dependir("build/release/intermediates/")
    set_optimize("smallest")
    add_cxflags("/bigobj")
else
    set_runtimes("MDd")
    set_targetdir("build/debug/")
    set_objectdir("build/debug/intermediates/")
    set_dependir("build/debug/intermediates/")
end

target("necrum_dx11")
    set_kind("shared")
    set_pcheader("src/pch.h")
    
    add_files("src/menu/**.cpp", "src/widgets/**.cpp")
    add_files("src/render/dx11_renderer.cpp", "src/render/hooks_dx11.cpp", "src/dllmain.cpp")
    add_files("src/core/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    add_includedirs("src/ext/imgui", "src/ext/imgui/backends", "src/ext/minhook")
    add_files("src/ext/imgui/imgui*.cpp")
    add_files("src/ext/imgui/backends/imgui_impl_win32.cpp", "src/ext/imgui/backends/imgui_impl_dx11.cpp")
    add_files("src/ext/minhook/*.c", "src/ext/minhook/hde/*.c")
    
    add_syslinks("d3d11", "dxgi", "d3dcompiler", "dwmapi", "advapi32", "user32", "gdi32", "wininet")

if is_plat("windows") and is_mode("release") then
    set_runtimes("MD")
    set_targetdir("build/release/")
    set_objectdir("build/release/intermediates/")
    set_dependir("build/release/intermediates/")
    set_optimize("smallest")
    add_cxflags("/bigobj")
else
    set_runtimes("MDd")
    set_targetdir("build/debug/")
    set_objectdir("build/debug/intermediates/")
    set_dependir("build/debug/intermediates/")
end

target("necrum_vk")
    set_kind("shared")
    set_pcheader("src/pch.h")
    add_includedirs("src")
    
    add_files("src/menu/**.cpp", "src/widgets/**.cpp")
    add_files("src/render/vulkan_renderer.cpp", "src/render/hooks_vk.cpp", "src/dllmain.cpp")
    add_files("src/core/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src/ext/imgui", "src/ext/imgui/backends", "src/ext/minhook")
    add_files("src/ext/imgui/imgui*.cpp")
    add_files("src/ext/imgui/backends/imgui_impl_win32.cpp", "src/ext/imgui/backends/imgui_impl_vulkan.cpp")
    add_files("src/ext/minhook/*.c", "src/ext/minhook/hde/*.c")
    
    add_packages("vulkan-headers", "volk")
    add_syslinks("dwmapi", "advapi32", "user32", "gdi32", "wininet")
    add_defines("USE_VULKAN", "IMGUI_IMPL_VULKAN_USE_VOLK", "VK_NO_PROTOTYPES", "IMGUI_IMPL_VULKAN_NO_PROTOTYPES")

if is_plat("windows") and is_mode("release") then
    set_runtimes("MD")
    set_targetdir("build/release/")
    set_objectdir("build/release/intermediates/")
    set_dependir("build/release/intermediates/")
    set_optimize("smallest")
    add_cxflags("/bigobj")
else
    set_runtimes("MDd")
    set_targetdir("build/debug/")
    set_objectdir("build/debug/intermediates/")
    set_dependir("build/debug/intermediates/")
end

target("necrum_dx9")
    set_kind("shared")
    set_pcheader("src/pch.h")
    
    add_files("src/menu/**.cpp", "src/widgets/**.cpp")
    add_files("src/render/dx9_renderer.cpp", "src/render/hooks_dx9.cpp", "src/dllmain.cpp")
    add_files("src/core/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    add_includedirs("src/ext/imgui", "src/ext/imgui/backends", "src/ext/minhook")
    add_files("src/ext/imgui/imgui*.cpp")
    add_files("src/ext/imgui/backends/imgui_impl_win32.cpp", "src/ext/imgui/backends/imgui_impl_dx9.cpp")
    add_files("src/ext/minhook/*.c", "src/ext/minhook/hde/*.c")
    
    add_syslinks("d3d9", "dwmapi", "advapi32", "user32", "gdi32", "wininet")

if is_plat("windows") and is_mode("release") then
    set_runtimes("MD")
    set_targetdir("build/release/")
    set_objectdir("build/release/intermediates/")
    set_dependir("build/release/intermediates/")
    set_optimize("smallest")
    add_cxflags("/bigobj")
else
    set_runtimes("MDd")
    set_targetdir("build/debug/")
    set_objectdir("build/debug/intermediates/")
    set_dependir("build/debug/intermediates/")
end
