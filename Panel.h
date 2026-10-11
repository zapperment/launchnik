#pragma once

#include "Jukebox.h"

// The click handler of the front panel. The launch buttons and the setup
// button are lamps with a transparent display on top; a click on a display
// ends up here, where the rules in CPress decide which property it changes.

// Identifiers of the displays, as given in hdgui_2D.lua.
const TJBox_DisplayID kDisplayID_Grid = 1;
const TJBox_DisplayID kDisplayID_SetupButton = 2;

void HandlePanelGesture(TJBox_GestureArgs& iArgs);
