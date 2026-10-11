#pragma once

#include "Jukebox.h"

// Property tags configured in motherboard_def.lua. Tracks are numbered from 0
// here and rows from 1, as in Launcher.h; in the tags both count from 1.

const TJBox_Tag kCustomPropsTag_Setup = 50;
const TJBox_Tag kCustomPropsTag_SetupLamp = 51;

inline TJBox_Tag SelectionTag(int iTrack) {
	return 100 + (iTrack + 1);
}

inline TJBox_Tag StopLightTag(int iTrack) {
	return 1000 + 10 * (iTrack + 1);
}

inline TJBox_Tag StopLightFrameTag(int iTrack) {
	return 2000 + 10 * (iTrack + 1);
}

inline TJBox_Tag LightTag(int iTrack, int iRow) {
	return 1000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag LightFrameTag(int iTrack, int iRow) {
	return 2000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag ClipPresentTag(int iTrack, int iRow) {
	return 3000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag ClipPatternTag(int iTrack, int iRow) {
	return 4000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag ClipColourTag(int iTrack, int iRow) {
	return 5000 + 10 * (iTrack + 1) + iRow;
}
