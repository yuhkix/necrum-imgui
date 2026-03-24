#pragma once
#include "pch.h"
#include <d3d9.h>

namespace renderer
{

class DX9Renderer
{
public:
	bool init(IDirect3DDevice9* device);
	void begin_frame();
	void end_frame();
	void shutdown();

	bool is_initialized() const { return initialized; }

private:
	IDirect3DDevice9* p_device = nullptr;
	bool initialized = false;
};

} // namespace renderer
