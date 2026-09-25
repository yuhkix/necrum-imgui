#pragma once

// necrum - an animated, themeable UI framework on top of Dear ImGui.
//
// Include this single header to get the whole public API:
//
//   core      theme tokens & presets, colors, animation, fonts, keybinds,
//             search, config persistence, draw helpers
//   widgets   checkbox, toggle, sliders, combos, keybind, color picker,
//             inputs, buttons, plots, popups ...
//   layout    panels (cards) and columns
//   app       Shell (complete main window), notifications, overlays,
//             watermark, splash screen, config manager, theme editor
//   retained  object-tree API on top of the immediate-mode widgets
//   platform  nc::App + nc::host, the bridge to render backends/hooks

#include "necrum/core/anim.h"
#include "necrum/core/color.h"
#include "necrum/core/config.h"
#include "necrum/core/context.h"
#include "necrum/core/draw.h"
#include "necrum/core/fonts.h"
#include "necrum/core/input.h"
#include "necrum/core/search.h"
#include "necrum/core/theme.h"

#include "necrum/widgets/widgets.h"

#include "necrum/layout/layout.h"

#include "necrum/app/builtin_pages.h"
#include "necrum/app/notifications.h"
#include "necrum/app/overlay.h"
#include "necrum/app/shell.h"
#include "necrum/app/splash.h"
#include "necrum/app/watermark.h"

#include "necrum/retained/retained.h"

#include "necrum/extras/web_image.h"
#include "necrum/extras/web_image_imgui.h"

#include "necrum/platform/host.h"

#include "necrum/fonts/icons_fa.h"
