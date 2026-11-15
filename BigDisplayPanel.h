#pragma once

#include "Jukebox.h"

// Draws the big display of the front panel: what CBigDisplay says and the
// output graphs, at the places of the screen design. It keeps nothing between two calls; everything
// it shows comes from the values listed for the display in hdgui_2D.lua.
void DrawBigDisplay(const TJBox_DisplayArgs& iArgs);
