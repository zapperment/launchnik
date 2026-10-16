#pragma once

#include "Jukebox.h"

// Property tags configured in motherboard_def.lua. Tracks are numbered from 0
// here and rows from 1, as in Launcher.h; in the tags both count from 1.

const TJBox_Tag kCustomPropsTag_Setup = 50;
const TJBox_Tag kCustomPropsTag_SetupLamp = 51;
const TJBox_Tag kCustomPropsTag_Length = 52;
const TJBox_Tag kCustomPropsTag_LogoFrame = 53;
const TJBox_Tag kCustomPropsTag_LaunchLabelsFrame = 54;
const TJBox_Tag kCustomPropsTag_BackButtonFrame = 55;
const TJBox_Tag kCustomPropsTag_BackLabelFrame = 56;
const TJBox_Tag kCustomPropsTag_FillIn = 57;
const TJBox_Tag kCustomPropsTag_FillInLight = 58;
const TJBox_Tag kCustomPropsTag_SetFillInLight = 59;
const TJBox_Tag kCustomPropsTag_FillInLightFrame = 70;
const TJBox_Tag kCustomPropsTag_FillInLabelFrame = 71;
const TJBox_Tag kCustomPropsTag_SetFillInLightFrame = 72;
const TJBox_Tag kCustomPropsTag_SetFillInLabelFrame = 73;
const TJBox_Tag kCustomPropsTag_Function = 74;

// Functions are numbered as in Launcher.h.
inline TJBox_Tag FunctionSelectLightFrameTag(int iFunction) {
	return 75 + iFunction;
}

inline TJBox_Tag FunctionSelectCaptionFrameTag(int iFunction) {
	return 80 + iFunction;
}

// Scenes are numbered from 1, like rows.
inline TJBox_Tag SceneLabelFrameTag(int iScene) {
	return 60 + iScene;
}

inline TJBox_Tag SelectionTag(int iTrack) {
	return 100 + (iTrack + 1);
}

inline TJBox_Tag MuteTag(int iTrack) {
	return 110 + (iTrack + 1);
}

inline TJBox_Tag SoloTag(int iTrack) {
	return 120 + (iTrack + 1);
}

inline TJBox_Tag FunctionLightTag(int iTrack) {
	return 1000 + 10 * (iTrack + 1);
}

inline TJBox_Tag FunctionLightFrameTag(int iTrack) {
	return 2000 + 10 * (iTrack + 1);
}

inline TJBox_Tag LightTag(int iTrack, int iRow) {
	return 1000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag LightFrameTag(int iTrack, int iRow) {
	return 2000 + 10 * (iTrack + 1) + iRow;
}

// The scene buttons use the tags a ninth track's launch buttons would have.
// Scenes are numbered from 1, like rows.
inline TJBox_Tag SceneLightTag(int iScene) {
	return 1090 + iScene;
}

inline TJBox_Tag SceneLightFrameTag(int iScene) {
	return 2090 + iScene;
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

inline TJBox_Tag FillInPresentTag(int iTrack, int iRow) {
	return 6000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag FillInPatternTag(int iTrack, int iRow) {
	return 7000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag FillInColourTag(int iTrack, int iRow) {
	return 8000 + 10 * (iTrack + 1) + iRow;
}
