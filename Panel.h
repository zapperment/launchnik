#pragma once

#include "Jukebox.h"

// The click handler of the front panel. The launch buttons, the scene buttons,
// the setup button, the back button, the set fill-in button and the fill-in
// button are lamps with a transparent display on top; a click on a display
// ends up here, where the rules in CPress decide which property it changes.

// Identifiers of the displays, as given in hdgui_2D.lua.
const TJBox_DisplayID kDisplayID_Grid = 1;
const TJBox_DisplayID kDisplayID_SetupButton = 2;
const TJBox_DisplayID kDisplayID_SceneButtons = 3;
const TJBox_DisplayID kDisplayID_BackButton = 4;
const TJBox_DisplayID kDisplayID_FillInButton = 5;
const TJBox_DisplayID kDisplayID_SetFillInButton = 6;

void HandlePanelGesture(TJBox_GestureArgs& iArgs);
