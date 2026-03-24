#include "pch.h"
#include "render/hooks.h"
#include "core/console.h"

DWORD WINAPI MainThread(LPVOID lpParam)
{
	core::Console::Init();

	if (!renderer::Hooks::init())
	{
		core::Console::Shutdown();
		return 1;
	}

	while (true)
	{
		if (GetAsyncKeyState(VK_END) & 0x8000)
		{
			break;
		}
		Sleep(100);
	}

	renderer::Hooks::shutdown();
	core::Console::Shutdown();
	FreeLibraryAndExitThread((HMODULE)lpParam, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		if (auto hThread = CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr))
		{
			CloseHandle(hThread);
		}
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
