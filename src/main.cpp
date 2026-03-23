#include "menu/menu.h"
#include "menu/theme.h"
#include "pch.h"
#include "render/renderer.h"

int main()
{
	renderer::Renderer renderer;
	if (!renderer.init())
	{
		return 1;
	}

	menu::Menu menu;
	while (renderer.begin_frame())
	{
		menu.render();
		renderer.end_frame();
	}

	renderer.shutdown();
	return 0;
}
