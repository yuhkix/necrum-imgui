#include "../pch.h"
#include "console.h"

namespace core
{
#ifdef _DEBUG
static FILE* fOut = nullptr;
static FILE* fIn = nullptr;
static FILE* fErr = nullptr;
#endif

void Console::Init()
{
#ifdef _DEBUG
	AllocConsole();
	SetConsoleTitleA("Necrum ImGui - Debug Console");

	freopen_s(&fOut, "CONOUT$", "w", stdout);
	freopen_s(&fErr, "CONOUT$", "w", stderr);
	freopen_s(&fIn, "CONIN$", "r", stdin);

	std::cout.clear();
	std::cerr.clear();
	std::cin.clear();

	std::cout << "[INFO] Debug console initialized." << std::endl;
#endif
}

void Console::Shutdown()
{
#ifdef _DEBUG
	std::cout << "[INFO] Shutting down debug console..." << std::endl;

	if (fOut)
		fclose(fOut);
	if (fErr)
		fclose(fErr);
	if (fIn)
		fclose(fIn);

	FreeConsole();
#endif
}
} // namespace core
