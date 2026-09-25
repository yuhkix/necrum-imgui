// Entry point of the injectable Windows DLL targets (necrum_dx9 ... necrum_vk).
// The hook installs itself into the host process' Present/SwapBuffers and
// drives the registered nc::App every frame. Press END to unload.

#include "pch.h"

#include "necrum/platform/console.h"
#include "render/hooks.h"

namespace
{
DWORD WINAPI main_thread(LPVOID module)
{
	nc::console::open();

	if (!renderer::Hooks::init())
	{
		nc::console::close();
		FreeLibraryAndExitThread((HMODULE)module, 1);
		return 1;
	}

	while (!(GetAsyncKeyState(VK_END) & 0x8000))
		Sleep(100);

	renderer::Hooks::shutdown();
	nc::console::close();
	FreeLibraryAndExitThread((HMODULE)module, 0);
	return 0;
}
} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(module);
		if (HANDLE thread = CreateThread(nullptr, 0, main_thread, module, 0, nullptr))
			CloseHandle(thread);
	}
	return TRUE;
}
