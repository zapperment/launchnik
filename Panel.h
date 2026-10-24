#pragma once

#include "Jukebox.h"

// The click handler of the front panel. The launch buttons, the scene buttons,
// the function buttons, the function select buttons, the setup button, the
// clear button, the back button, the forward button and the fill-in button
// are lamps with a
// transparent display on top; a click on a display
// ends up here, where the rules in CPress decide which property it changes.

// Identifiers of the displays, as given in hdgui_2D.lua.
const TJBox_DisplayID kDisplayID_Grid = 1;
const TJBox_DisplayID kDisplayID_SetupButton = 2;
const TJBox_DisplayID kDisplayID_SceneButtons = 3;
const TJBox_DisplayID kDisplayID_BackButton = 4;
const TJBox_DisplayID kDisplayID_FillInButton = 5;
const TJBox_DisplayID kDisplayID_ForwardButton = 6;
const TJBox_DisplayID kDisplayID_FunctionButtons = 7;
// One display for each function select button, numbered from here by
// function as in Launcher.h: stop, mute, solo.
const TJBox_DisplayID kDisplayID_FirstFunctionSelectButton = 8;
const TJBox_DisplayID kDisplayID_ClearButton = 11;
const TJBox_DisplayID kDisplayID_DuplicateButton = 12;

void HandlePanelGesture(TJBox_GestureArgs& iArgs);

// Tells the host which property each part of a display stands for, so that it
// offers automation, Remote Override and alt-click there: every launch button
// and every function button stands for its track's selection, in every mode
// and function, and the fill-in button for the fill-in property. The other
// displays stand for no automatable property.
void SetUpPanelDisplay(const TJBox_DisplayArgs& iArgs);
