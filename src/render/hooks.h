#pragma once
#include "pch.h"

namespace renderer
{

class Hooks
{
public:
	static bool init();
	static void shutdown();

private:
	static bool hook_wglSwapBuffers();
};

} // namespace renderer
