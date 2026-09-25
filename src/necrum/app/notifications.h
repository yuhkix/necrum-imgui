#pragma once

#include "necrum/core/base.h"

namespace nc
{

enum class Notify
{
	Success,
	Info,
	Warning,
	Error,
};

// Queues a toast. Rendered on the foreground draw list by nc::end_frame().
void notify(Notify type, const char* fmt, ...) IM_FMTARGS(2);
void notify_for(Notify type, float seconds, const char* fmt, ...) IM_FMTARGS(3);

struct NotifyStyle
{
	Corner corner = Corner::BottomRight;
	float width = 280.0f;
	float height = 45.0f;
	float margin = 20.0f;
	float spacing = 10.0f;
	int max_visible = 6;
};
NotifyStyle& notify_style();

void clear_notifications();

} // namespace nc
