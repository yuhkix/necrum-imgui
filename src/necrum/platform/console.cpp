#include "console.h"

#include <cstdio>

#if defined(_WIN32) && !defined(NDEBUG)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#define NC_WIN_CONSOLE 1
#endif

namespace nc::console
{

#ifdef NC_WIN_CONSOLE
namespace
{
FILE* g_out = nullptr;
FILE* g_err = nullptr;
FILE* g_in = nullptr;
} // namespace

void open(const char* title)
{
	if (!AllocConsole())
		return;
	SetConsoleTitleA(title);
	freopen_s(&g_out, "CONOUT$", "w", stdout);
	freopen_s(&g_err, "CONOUT$", "w", stderr);
	freopen_s(&g_in, "CONIN$", "r", stdin);
	printf("[necrum] debug console attached\n");
}

void close()
{
	if (g_out)
		fclose(g_out);
	if (g_err)
		fclose(g_err);
	if (g_in)
		fclose(g_in);
	g_out = g_err = g_in = nullptr;
	FreeConsole();
}
#else
void open(const char*) {}
void close() {}
#endif

} // namespace nc::console
