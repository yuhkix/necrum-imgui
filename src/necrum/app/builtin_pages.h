#pragma once

#include "necrum/core/base.h"

// Ready-made building blocks for settings pages.
namespace nc
{

// Config file manager bound to nc::config(): list of "<dir>/*.cfg" files with
// name field and Save / Load / Delete / Reset buttons (with toasts).
void config_manager(const char* directory);

// Theme editor: preset picker, accent color, animation speed, glow and rounding.
void theme_editor();

} // namespace nc
