#pragma once

namespace nc::console
{
// Opens a debug console window (Windows, debug builds). No-op elsewhere:
// on Linux stdout/stderr already go to the launching terminal.
void open(const char* title = "necrum - debug console");
void close();
} // namespace nc::console
