#pragma once

#include "necrum/core/base.h"

#include <memory>

// Glue between a rendering backend (standalone window or a hooked Present /
// SwapBuffers) and the user's application. Backends only talk to nc::host;
// the application only implements nc::App. Neither knows about the other.
namespace nc
{

class App
{
public:
	virtual ~App() = default;

	// Called once after ImGui::CreateContext(), before the backend is initialised.
	// The default loads the framework fonts and applies the active theme.
	virtual void setup(ImGuiIO& io);

	// Called every frame between ImGui::NewFrame() and ImGui::Render().
	virtual void frame() = 0;

	// While true, hook backends swallow mouse/keyboard input so it does not reach
	// the host application (e.g. while the main window is open).
	virtual bool wants_input() const { return false; }

	// Called before the ImGui context is destroyed.
	virtual void shutdown() {}
};

// Implemented by the application; see NC_REGISTER_APP.
std::unique_ptr<App> create_app();

namespace host
{
void install(std::unique_ptr<App> app); // optional: create_app() is used when nothing is installed
App* app();

void setup(ImGuiIO& io);	// backend: after CreateContext
void frame();							// backend: between NewFrame and Render
bool wants_input();				// backend: input blocking decision
void shutdown();					// backend: before DestroyContext (destroys the app)
} // namespace host

} // namespace nc

// Registers the application type used by every backend target:
//     NC_REGISTER_APP(MyApp)
#define NC_REGISTER_APP(Type)                                                                                          \
	std::unique_ptr<nc::App> nc::create_app()                                                                            \
	{                                                                                                                    \
		return std::make_unique<Type>();                                                                                   \
	}
