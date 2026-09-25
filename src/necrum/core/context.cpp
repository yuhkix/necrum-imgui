#include "context.h"

#include "anim.h"
#include "input.h"
#include "theme.h"

namespace nc
{

void render_notifications(); // app/notifications.cpp

namespace
{
struct Callback
{
	int handle;
	bool end;
	FrameCallback fn;
};

std::vector<Callback>& callbacks()
{
	static std::vector<Callback> list;
	return list;
}

int g_next_handle = 1;

void run(bool end)
{
	// Copy: callbacks may register/remove callbacks.
	auto list = callbacks();
	for (auto& cb : list)
		if (cb.end == end && cb.fn)
			cb.fn();
}
} // namespace

void new_frame()
{
	anim::new_frame(ImGui::GetIO().DeltaTime);
	theme().refresh();
	keybinds::update();
	run(false);
}

void end_frame()
{
	run(true);
	render_notifications();
}

int on_new_frame(FrameCallback cb)
{
	int h = g_next_handle++;
	callbacks().push_back({h, false, std::move(cb)});
	return h;
}

int on_end_frame(FrameCallback cb)
{
	int h = g_next_handle++;
	callbacks().push_back({h, true, std::move(cb)});
	return h;
}

void remove_callback(int handle)
{
	auto& list = callbacks();
	list.erase(std::remove_if(list.begin(), list.end(), [handle](const Callback& c) { return c.handle == handle; }),
						 list.end());
}

} // namespace nc
