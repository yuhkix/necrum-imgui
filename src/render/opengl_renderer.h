#pragma once
#include "../pch.h"

namespace renderer
{

class OpenGLRenderer
{
public:
	bool init(HWND hwnd);
	void begin_frame();
	void end_frame();
	void shutdown();

	bool is_initialized() const { return initialized; }

private:
	HWND h_hwnd = nullptr;
	bool initialized = false;
};

} // namespace renderer
