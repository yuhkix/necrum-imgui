#include "host.h"

#include "necrum/core/context.h"
#include "necrum/core/fonts.h"
#include "necrum/core/theme.h"

namespace nc
{

void App::setup(ImGuiIO& io)
{
	io.IniFilename = nullptr;
	fonts::load(io);
	apply_imgui_style(theme());
}

namespace host
{

namespace
{
std::unique_ptr<App>& instance()
{
	static std::unique_ptr<App> app;
	return app;
}
} // namespace

void install(std::unique_ptr<App> app)
{
	instance() = std::move(app);
}

App* app()
{
	auto& inst = instance();
	if (!inst)
		inst = create_app();
	return inst.get();
}

void setup(ImGuiIO& io)
{
	if (App* a = app())
		a->setup(io);
}

void frame()
{
	new_frame();
	if (App* a = app())
		a->frame();
	end_frame();
}

bool wants_input()
{
	App* a = instance().get();
	return a && a->wants_input();
}

void shutdown()
{
	if (App* a = instance().get())
		a->shutdown();
	instance().reset();
}

} // namespace host

} // namespace nc
