#pragma once

#include "Jukebox.h"

// Property tags configured in motherboard_def.lua. Tracks are numbered from 0
// here and rows from 1, as in Launcher.h; in the tags both count from 1.

const TJBox_Tag kCustomPropsTag_Setup = 50;
const TJBox_Tag kCustomPropsTag_SetupLightFrame = 51;
const TJBox_Tag kCustomPropsTag_Length = 52;
const TJBox_Tag kCustomPropsTag_LogoFrame = 53;
const TJBox_Tag kCustomPropsTag_LaunchLabelsFrame = 54;
const TJBox_Tag kCustomPropsTag_BackButtonFrame = 55;
const TJBox_Tag kCustomPropsTag_BackLabelFrame = 56;
const TJBox_Tag kCustomPropsTag_FillIn = 57;
const TJBox_Tag kCustomPropsTag_FillInLight = 58;
const TJBox_Tag kCustomPropsTag_ForwardLight = 59;
const TJBox_Tag kCustomPropsTag_FillInLightFrame = 70;
const TJBox_Tag kCustomPropsTag_FillInLabelFrame = 71;
const TJBox_Tag kCustomPropsTag_ForwardLightFrame = 72;
const TJBox_Tag kCustomPropsTag_ForwardLabelFrame = 73;
const TJBox_Tag kCustomPropsTag_Function = 74;
const TJBox_Tag kCustomPropsTag_ClearLightFrame = 84;
const TJBox_Tag kCustomPropsTag_ClearCaptionFrame = 85;
const TJBox_Tag kCustomPropsTag_DuplicateLightFrame = 86;
const TJBox_Tag kCustomPropsTag_DuplicateCaptionFrame = 87;
const TJBox_Tag kCustomPropsTag_SetupCaptionFrame = 88;
const TJBox_Tag kCustomPropsTag_BackPressedAt = 89;
const TJBox_Tag kCustomPropsTag_TransitionDiagramsFrame = 150;

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

// CV outputs are numbered from 0 here, as in Launcher.h, and from 1 in the
// tags.
inline TJBox_Tag OutputLabelsFrameTag(int iOutput) {
	return 140 + (iOutput + 1);
}

inline TJBox_Tag CVModeTag(int iTrack, int iOutput) {
	return 200 + 10 * (iTrack + 1) + (iOutput + 1);
}

// The copy of a CV mode that control surfaces read.
inline TJBox_Tag CVModeCopyTag(int iTrack, int iOutput) {
	return 300 + 10 * (iTrack + 1) + (iOutput + 1);
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

inline TJBox_Tag RelaunchTag(int iTrack) {
	return 130 + (iTrack + 1);
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

// Names, empty while they stand for the default (see docs/adr/0007). Scenes
// are numbered from 1, like rows; CV outputs from 0, as in Launcher.h.
inline TJBox_Tag TrackNameTag(int iTrack) {
	return 400 + (iTrack + 1);
}

inline TJBox_Tag SceneNameTag(int iScene) {
	return 410 + iScene;
}

inline TJBox_Tag OutputNameTag(int iTrack, int iOutput) {
	return 500 + 10 * (iTrack + 1) + (iOutput + 1);
}

inline TJBox_Tag ClipNameTag(int iTrack, int iRow) {
	return 4000 + 10 * (iTrack + 1) + iRow;
}

// Whether a launch button holds a clip, its colour and one-bar pattern, all
// in one property (see docs/adr/0008), and the same of its fill-in, landing,
// intro and outro.
inline TJBox_Tag ClipSwitchesTag(int iTrack, int iRow) {
	return 3000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag FillInSwitchesTag(int iTrack, int iRow) {
	return 6000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag LandingSwitchesTag(int iTrack, int iRow) {
	return 7000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag IntroSwitchesTag(int iTrack, int iRow) {
	return 24000 + 10 * (iTrack + 1) + iRow;
}

inline TJBox_Tag OutroSwitchesTag(int iTrack, int iRow) {
	return 27000 + 10 * (iTrack + 1) + iRow;
}

// The settings of a clip's CV outputs, and of its fill-in's, its landing's,
// its intro's and its outro's, all four of an output in one property (see
// docs/adr/0008): a base of its own for each kind, plus 100 for each track,
// 10 for each row and the output.
const TJBox_Tag kOutputSettingTagSpan = 1000;
const TJBox_Tag kClipOutputBase = 10000;
const TJBox_Tag kFillInOutputBase = 14000;
const TJBox_Tag kLandingOutputBase = 18000;
const TJBox_Tag kIntroOutputBase = 30000;
const TJBox_Tag kOutroOutputBase = 34000;

inline TJBox_Tag OutputSettingTag(TJBox_Tag iBase, int iTrack, int iRow, int iOutput) {
	return iBase + 100 * (iTrack + 1) + 10 * iRow + (iOutput + 1);
}
