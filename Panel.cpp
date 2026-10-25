#include "Panel.h"

#include "Launcher.h"
#include "Press.h"
#include "Tags.h"

namespace {

// Layout of the grid display, in its own pixels; matches GUI2D/device_2D.lua.
const TJBox_Float32 kTrackPitch = 283.0f;
const TJBox_Float32 kRowPitch = 250.0f;
const TJBox_Float32 kButtonWidth = 205.0f;
const TJBox_Float32 kButtonHeight = 135.0f;

// The names of the tracks' selections in motherboard_def.lua.
const char* const kSelectionKeys[CLauncher::kTrackCount] = {
	"track1_selection", "track2_selection", "track3_selection", "track4_selection",
	"track5_selection", "track6_selection", "track7_selection", "track8_selection",
};

// The size of the fill-in button's display.
const TJBox_Float32 kFillInWidth = 130.0f;
const TJBox_Float32 kFillInHeight = 135.0f;

// The size of a function button; the display of the function buttons is as
// wide as the eight of them, a track apart.
const TJBox_Float32 kFunctionButtonWidth = 70.0f;
const TJBox_Float32 kFunctionButtonHeight = 75.0f;

// Finds the launch button under a point of the grid display. Returns false for
// a point in the gaps between launch buttons.
bool FindLaunchButton(const TJBox_Point& iPoint, int& oTrack, int& oRow) {
	if (iPoint.fX < 0.0f || iPoint.fY < 0.0f) {
		return false;
	}
	const int column = static_cast<int>(iPoint.fX / kTrackPitch);
	const int line = static_cast<int>(iPoint.fY / kRowPitch);
	if (column >= CLauncher::kTrackCount || line >= CLauncher::kRowCount) {
		return false;
	}
	if (iPoint.fX - column * kTrackPitch >= kButtonWidth || iPoint.fY - line * kRowPitch >= kButtonHeight) {
		return false;
	}
	oTrack = column;
	oRow = line + 1;
	return true;
}

// Finds the scene button under a point of the scene buttons' display. Returns
// false for a point in the gaps between scene buttons.
bool FindSceneButton(const TJBox_Point& iPoint, int& oScene) {
	if (iPoint.fX < 0.0f || iPoint.fY < 0.0f) {
		return false;
	}
	const int line = static_cast<int>(iPoint.fY / kRowPitch);
	if (line >= CLauncher::kRowCount || iPoint.fY - line * kRowPitch >= kButtonHeight) {
		return false;
	}
	oScene = line + 1;
	return true;
}

// Finds the function button under a point of the function buttons' display.
// Returns false for a point in the gaps between function buttons.
bool FindFunctionButton(const TJBox_Point& iPoint, int& oTrack) {
	if (iPoint.fX < 0.0f || iPoint.fY < 0.0f || iPoint.fY >= kFunctionButtonHeight) {
		return false;
	}
	const int column = static_cast<int>(iPoint.fX / kTrackPitch);
	if (column >= CLauncher::kTrackCount || iPoint.fX - column * kTrackPitch >= kFunctionButtonWidth) {
		return false;
	}
	oTrack = column;
	return true;
}

bool HoldsClip(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	return JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, ClipPresentTag(track, row)));
}

bool HoldsFillIn(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	return JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, FillInPresentTag(track, row)));
}

bool HoldsLanding(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	return JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, LandingPresentTag(track, row)));
}

bool HoldsIntro(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	return JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, IntroPresentTag(track, row)));
}

bool HoldsOutro(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	return JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, OutroPresentTag(track, row)));
}

bool TrackHoldsClips(TJBox_ObjectRef iCustomProps, int iTrack) {
	for (int row = 1; row <= CLauncher::kRowCount; ++row) {
		if (HoldsClip(iCustomProps, CLauncher::ButtonIndex(iTrack, row))) {
			return true;
		}
	}
	return false;
}

int LoadNumber(TJBox_ObjectRef iCustomProps, TJBox_Tag iTag) {
	return static_cast<int>(JBox_GetNumber(JBox_LoadMOMPropertyByTag(iCustomProps, iTag)));
}

// The settings of a CV output of one kind: a clip's, a fill-in's, a
// landing's, an intro's or an outro's, as iBase says.
CLauncher::TOutputSettings LoadOutputSettings(TJBox_ObjectRef iCustomProps, TJBox_Tag iBase, int iTrack, int iRow, int iOutput) {
	return CLauncher::UnpackOutputSettings(LoadNumber(iCustomProps, OutputSettingTag(iBase, iTrack, iRow, iOutput)));
}

// What a press in clip editing, fill-in editing, landing editing, intro
// editing or outro editing needs to know of the settings being edited, the
// clip's, its fill-in's, its landing's, its intro's or its outro's.
CPress::TEdited LoadEdited(TJBox_ObjectRef iCustomProps, int iButtonIndex, int iSetup) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	CPress::TEdited edited = CPress::Edited(false);
	TJBox_Tag outputBase = kClipOutputBase;
	edited.fPresent = HoldsClip(iCustomProps, iButtonIndex);
	if (CLauncher::IsFillInEditing(iSetup)) {
		outputBase = kFillInOutputBase;
		edited.fPresent = HoldsFillIn(iCustomProps, iButtonIndex);
		edited.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, FillInOneBarPatternTag(track, row)));
	}
	else if (CLauncher::IsLandingEditing(iSetup)) {
		outputBase = kLandingOutputBase;
		edited.fPresent = HoldsLanding(iCustomProps, iButtonIndex);
		edited.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, LandingOneBarPatternTag(track, row)));
	}
	else if (CLauncher::IsIntroEditing(iSetup)) {
		outputBase = kIntroOutputBase;
		edited.fPresent = HoldsIntro(iCustomProps, iButtonIndex);
		edited.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, IntroOneBarPatternTag(track, row)));
	}
	else if (CLauncher::IsOutroEditing(iSetup)) {
		outputBase = kOutroOutputBase;
		edited.fPresent = HoldsOutro(iCustomProps, iButtonIndex);
		edited.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, OutroOneBarPatternTag(track, row)));
	}
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		edited.fCVModes[output] = LoadNumber(iCustomProps, CVModeTag(track, output));
		const CLauncher::TOutputSettings settings = LoadOutputSettings(iCustomProps, outputBase, track, row, output);
		edited.fPatterns[output] = settings.fPattern;
		edited.fUnipolar[output] = settings.fUnipolar;
		edited.fBipolar[output] = settings.fBipolar;
		edited.fTransitions[output] = settings.fTransition;
	}
	return edited;
}

CPress::TClipValues LoadClipValues(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	CPress::TClipValues values;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		const CLauncher::TOutputSettings settings = LoadOutputSettings(iCustomProps, kClipOutputBase, track, row, output);
		values.fPatterns[output] = settings.fPattern;
		values.fUnipolar[output] = settings.fUnipolar;
		values.fBipolar[output] = settings.fBipolar;
	}
	return values;
}

// One kind of settings of a launch button, whose properties have these tags.
CPress::TSettings LoadSettings(TJBox_ObjectRef iCustomProps, int iTrack, int iRow, TJBox_Tag iPresentTag, TJBox_Tag iColourTag, TJBox_Tag iOutputBase) {
	CPress::TSettings settings;
	settings.fPresent = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, iPresentTag));
	settings.fColour = LoadNumber(iCustomProps, iColourTag);
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		const CLauncher::TOutputSettings outputSettings = LoadOutputSettings(iCustomProps, iOutputBase, iTrack, iRow, output);
		settings.fPatterns[output] = outputSettings.fPattern;
		settings.fUnipolar[output] = outputSettings.fUnipolar;
		settings.fBipolar[output] = outputSettings.fBipolar;
		settings.fTransitions[output] = outputSettings.fTransition;
	}
	settings.fOneBarPattern = false;
	return settings;
}

// Everything a launch button holds, which a duplicate copies.
CPress::TButton LoadButton(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	CPress::TButton button;
	button.fClip = LoadSettings(iCustomProps, track, row, ClipPresentTag(track, row), ClipColourTag(track, row),
		kClipOutputBase);
	button.fFillIn = LoadSettings(iCustomProps, track, row, FillInPresentTag(track, row), FillInColourTag(track, row),
		kFillInOutputBase);
	button.fFillIn.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, FillInOneBarPatternTag(track, row)));
	button.fLanding = LoadSettings(iCustomProps, track, row, LandingPresentTag(track, row), LandingColourTag(track, row),
		kLandingOutputBase);
	button.fLanding.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, LandingOneBarPatternTag(track, row)));
	button.fIntro = LoadSettings(iCustomProps, track, row, IntroPresentTag(track, row), IntroColourTag(track, row),
		kIntroOutputBase);
	button.fIntro.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, IntroOneBarPatternTag(track, row)));
	button.fOutro = LoadSettings(iCustomProps, track, row, OutroPresentTag(track, row), OutroColourTag(track, row),
		kOutroOutputBase);
	button.fOutro.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, OutroOneBarPatternTag(track, row)));
	return button;
}

// The kinds of settings a launch button holds, in the order of
// kOutputSettingKeys.
enum ESettingsKind { kClipKind, kFillInKind, kLandingKind, kIntroKind, kOutroKind };

// The names of the changes of a CV output's settings, by kind and by the
// setting changed: pattern, unipolar value, bipolar value, transition.
const char* const kOutputSettingKeys[5][4] = {
	{ "gesture_clip_pattern", "gesture_clip_unipolar", "gesture_clip_bipolar", "gesture_clip_transition" },
	{ "gesture_fill_in_pattern", "gesture_fill_in_unipolar", "gesture_fill_in_bipolar", "gesture_fill_in_transition" },
	{ "gesture_landing_pattern", "gesture_landing_unipolar", "gesture_landing_bipolar", "gesture_landing_transition" },
	{ "gesture_intro_pattern", "gesture_intro_unipolar", "gesture_intro_bipolar", "gesture_intro_transition" },
	{ "gesture_outro_pattern", "gesture_outro_unipolar", "gesture_outro_bipolar", "gesture_outro_transition" },
};

// Writes the settings of a CV output, all four in one property (see
// docs/adr/0008), and names the change after the setting it changes; a
// change of several at once, such as a duplicate, has its name from the
// caller.
void StoreOutputSettings(TJBox_ObjectRef iCustomProps, TJBox_Tag iBase, ESettingsKind iKind, int iTrack, int iRow, const CPress::TChange& iChange, TJBox_GestureArgs& ioArgs) {
	const CLauncher::TOutputSettings before = LoadOutputSettings(iCustomProps, iBase, iTrack, iRow, iChange.fOutput);
	const CLauncher::TOutputSettings after = CLauncher::UnpackOutputSettings(iChange.fValue);
	JBox_StoreMOMPropertyByTag(iCustomProps, OutputSettingTag(iBase, iTrack, iRow, iChange.fOutput), JBox_MakeNumber(iChange.fValue));
	int setting = 3;
	if (after.fPattern != before.fPattern) {
		setting = 0;
	}
	else if (after.fUnipolar != before.fUnipolar) {
		setting = 1;
	}
	else if (after.fBipolar != before.fBipolar) {
		setting = 2;
	}
	ioArgs.fGestureNameKey = kOutputSettingKeys[iKind][setting];
}

// Writes the one property a press changes. The names of the changes are keys
// of texts.lua; a name that is not a key makes the host refuse the change.
void Apply(const CPress::TChange& iChange, TJBox_ObjectRef iCustomProps, TJBox_GestureArgs& iArgs) {
	const int track = iChange.fIndex / CLauncher::kRowCount;
	const int row = iChange.fIndex % CLauncher::kRowCount + 1;

	switch (iChange.fProperty) {
		case CPress::kSelection:
			JBox_StoreMOMPropertyByTag(iCustomProps, SelectionTag(iChange.fIndex), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_selection";
			break;
		case CPress::kSetup:
			JBox_StoreMOMPropertyByTag(iCustomProps, kCustomPropsTag_Setup, JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_setup";
			break;
		case CPress::kLength:
			JBox_StoreMOMPropertyByTag(iCustomProps, kCustomPropsTag_Length, JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_length";
			break;
		case CPress::kClipPresent:
			JBox_StoreMOMPropertyByTag(iCustomProps, ClipPresentTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_clip_present";
			break;
		case CPress::kClipOutput:
			StoreOutputSettings(iCustomProps, kClipOutputBase, kClipKind, track, row, iChange, iArgs);
			break;
		case CPress::kClipColour:
			JBox_StoreMOMPropertyByTag(iCustomProps, ClipColourTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_clip_colour";
			break;
		case CPress::kFillInPresent:
			JBox_StoreMOMPropertyByTag(iCustomProps, FillInPresentTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_fill_in_present";
			break;
		case CPress::kFillInOutput:
			StoreOutputSettings(iCustomProps, kFillInOutputBase, kFillInKind, track, row, iChange, iArgs);
			break;
		case CPress::kFillInColour:
			JBox_StoreMOMPropertyByTag(iCustomProps, FillInColourTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_fill_in_colour";
			break;
		case CPress::kLandingPresent:
			JBox_StoreMOMPropertyByTag(iCustomProps, LandingPresentTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_landing_present";
			break;
		case CPress::kLandingOutput:
			StoreOutputSettings(iCustomProps, kLandingOutputBase, kLandingKind, track, row, iChange, iArgs);
			break;
		case CPress::kFillInOneBarPattern:
			JBox_StoreMOMPropertyByTag(iCustomProps, FillInOneBarPatternTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_one_bar_pattern";
			break;
		case CPress::kLandingOneBarPattern:
			JBox_StoreMOMPropertyByTag(iCustomProps, LandingOneBarPatternTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_one_bar_pattern";
			break;
		case CPress::kLandingColour:
			JBox_StoreMOMPropertyByTag(iCustomProps, LandingColourTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_landing_colour";
			break;
		case CPress::kIntroPresent:
			JBox_StoreMOMPropertyByTag(iCustomProps, IntroPresentTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_intro_present";
			break;
		case CPress::kIntroColour:
			JBox_StoreMOMPropertyByTag(iCustomProps, IntroColourTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_intro_colour";
			break;
		case CPress::kIntroOutput:
			StoreOutputSettings(iCustomProps, kIntroOutputBase, kIntroKind, track, row, iChange, iArgs);
			break;
		case CPress::kIntroOneBarPattern:
			JBox_StoreMOMPropertyByTag(iCustomProps, IntroOneBarPatternTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_one_bar_pattern";
			break;
		case CPress::kOutroPresent:
			JBox_StoreMOMPropertyByTag(iCustomProps, OutroPresentTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_outro_present";
			break;
		case CPress::kOutroColour:
			JBox_StoreMOMPropertyByTag(iCustomProps, OutroColourTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_outro_colour";
			break;
		case CPress::kOutroOutput:
			StoreOutputSettings(iCustomProps, kOutroOutputBase, kOutroKind, track, row, iChange, iArgs);
			break;
		case CPress::kOutroOneBarPattern:
			JBox_StoreMOMPropertyByTag(iCustomProps, OutroOneBarPatternTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_one_bar_pattern";
			break;
		case CPress::kFillIn:
			JBox_StoreMOMPropertyByTag(iCustomProps, kCustomPropsTag_FillIn, JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_fill_in";
			break;
		case CPress::kFunction:
			JBox_StoreMOMPropertyByTag(iCustomProps, kCustomPropsTag_Function, JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_function";
			break;
		case CPress::kMute:
			JBox_StoreMOMPropertyByTag(iCustomProps, MuteTag(iChange.fIndex), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_mute";
			break;
		case CPress::kSolo:
			JBox_StoreMOMPropertyByTag(iCustomProps, SoloTag(iChange.fIndex), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_solo";
			break;
		case CPress::kRelaunch:
			JBox_StoreMOMPropertyByTag(iCustomProps, RelaunchTag(iChange.fIndex), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_relaunch";
			break;
		case CPress::kNothing:
			break;
	}
}

// The fill-in button is held down from the tap until it is let go, so that
// automation records both.
void HandleFillInGesture(TJBox_GestureArgs& iArgs) {
	const TJBox_ObjectRef customProps = JBox_GetMotherboardObjectRef("/custom_properties");
	if (iArgs.fCall == kJBox_OnTap && iArgs.fTap == kJBox_TapRegular) {
		iArgs.fConsumeFlag = true;
		Apply(CPress::FillInButton(true), customProps, iArgs);
	}
	// Only let go of a button that is held, not after a context menu.
	else if (iArgs.fCall == kJBox_OnRelease || iArgs.fCall == kJBox_OnCancel) {
		if (JBox_GetBoolean(JBox_LoadMOMPropertyByTag(customProps, kCustomPropsTag_FillIn))) {
			Apply(CPress::FillInButton(false), customProps, iArgs);
		}
	}
}

// In mute and solo function a function button is held down from the tap until
// it is let go, so that automation records both, like the fill-in button. In
// stop function the tap chooses stop.
void HandleFunctionButtonsGesture(TJBox_GestureArgs& iArgs) {
	const TJBox_ObjectRef customProps = JBox_GetMotherboardObjectRef("/custom_properties");
	const int function = static_cast<int>(JBox_GetNumber(JBox_LoadMOMPropertyByTag(customProps, kCustomPropsTag_Function)));
	if (iArgs.fCall == kJBox_OnTap && iArgs.fTap == kJBox_TapRegular) {
		int track = 0;
		if (!FindFunctionButton(iArgs.fStartState.fPoint, track)) {
			return;
		}
		iArgs.fConsumeFlag = true;
		Apply(CPress::FunctionButton(function, track, TrackHoldsClips(customProps, track), true), customProps, iArgs);
	}
	// Let go of whichever mute or solo button is held, whatever the function
	// is by now, but not after a context menu.
	else if (iArgs.fCall == kJBox_OnRelease || iArgs.fCall == kJBox_OnCancel) {
		for (int track = 0; track < CLauncher::kTrackCount; ++track) {
			if (JBox_GetBoolean(JBox_LoadMOMPropertyByTag(customProps, MuteTag(track)))) {
				Apply(CPress::FunctionButton(CLauncher::kFunctionMute, track, true, false), customProps, iArgs);
			}
			if (JBox_GetBoolean(JBox_LoadMOMPropertyByTag(customProps, SoloTag(track)))) {
				Apply(CPress::FunctionButton(CLauncher::kFunctionSolo, track, true, false), customProps, iArgs);
			}
		}
	}
}

// Lets go of every relaunch press that a launch button or a scene button
// holds down, once the mouse button is let go, but not after a context menu.
void ReleaseRelaunches(TJBox_GestureArgs& iArgs) {
	if (iArgs.fCall != kJBox_OnRelease && iArgs.fCall != kJBox_OnCancel) {
		return;
	}
	const TJBox_ObjectRef customProps = JBox_GetMotherboardObjectRef("/custom_properties");
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		if (JBox_GetBoolean(JBox_LoadMOMPropertyByTag(customProps, RelaunchTag(track)))) {
			Apply(CPress::RelaunchButton(CLauncher::kSetupLaunchMode, track, false), customProps, iArgs);
		}
	}
}

TJBox_PropertyBox MakeBox(TJBox_PropertyRef iProperty, TJBox_Coord iLeft, TJBox_Coord iTop, TJBox_Coord iWidth, TJBox_Coord iHeight) {
	TJBox_PropertyBox box;
	box.fPropertyIndex = 0;
	box.fPropertyRef = iProperty;
	box.fRect.fLeft = iLeft;
	box.fRect.fTop = iTop;
	box.fRect.fRight = iLeft + iWidth;
	box.fRect.fBottom = iTop + iHeight;
	box.fFlags = 0;
	return box;
}

} // namespace

void HandlePanelGesture(TJBox_GestureArgs& iArgs) {
	if (iArgs.fDisplayInfo.fID == kDisplayID_FillInButton) {
		HandleFillInGesture(iArgs);
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_FunctionButtons) {
		HandleFunctionButtonsGesture(iArgs);
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_Grid || iArgs.fDisplayInfo.fID == kDisplayID_SceneButtons) {
		ReleaseRelaunches(iArgs);
	}
	if (iArgs.fCall != kJBox_OnTap || iArgs.fTap != kJBox_TapRegular) {
		return;
	}
	const TJBox_ObjectRef customProps = JBox_GetMotherboardObjectRef("/custom_properties");
	const int setup = static_cast<int>(JBox_GetNumber(JBox_LoadMOMPropertyByTag(customProps, kCustomPropsTag_Setup)));

	const int functionSelect = static_cast<int>(iArgs.fDisplayInfo.fID) - static_cast<int>(kDisplayID_FirstFunctionSelectButton);
	if (functionSelect >= 0 && functionSelect < CLauncher::kFunctionCount) {
		iArgs.fConsumeFlag = true;
		Apply(CPress::FunctionSelectButton(functionSelect), customProps, iArgs);
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_SetupButton) {
		iArgs.fConsumeFlag = true;
		Apply(CPress::SetupButton(setup), customProps, iArgs);
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_ClearButton) {
		iArgs.fConsumeFlag = true;
		Apply(CPress::ClearButton(setup), customProps, iArgs);
		iArgs.fGestureNameKey = "gesture_clear_mode";
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_DuplicateButton) {
		iArgs.fConsumeFlag = true;
		Apply(CPress::DuplicateButton(setup), customProps, iArgs);
		iArgs.fGestureNameKey = "gesture_duplicate_mode";
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_BackButton) {
		iArgs.fConsumeFlag = true;
		Apply(CPress::BackButton(setup), customProps, iArgs);
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_ForwardButton) {
		iArgs.fConsumeFlag = true;
		const int picked = CLauncher::PickedButton(setup);
		const bool pickedHoldsClip = picked >= 0 && HoldsClip(customProps, picked);
		Apply(CPress::ForwardButton(setup, pickedHoldsClip), customProps, iArgs);
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_SceneButtons) {
		int scene = 0;
		if (!FindSceneButton(iArgs.fStartState.fPoint, scene)) {
			return;
		}
		iArgs.fConsumeFlag = true;

		bool holdsClip[CLauncher::kTrackCount];
		bool sceneHoldsClip = false;
		for (int track = 0; track < CLauncher::kTrackCount; ++track) {
			holdsClip[track] = HoldsClip(customProps, CLauncher::ButtonIndex(track, scene));
			sceneHoldsClip = sceneHoldsClip || holdsClip[track];
		}
		// Duplicating a scene and returning to launch mode are one step of the
		// undo history. The scene is copied track by track, then the setup
		// state changes.
		if (CLauncher::IsDuplicateMode(setup)) {
			const CPress::TChange change = CPress::DuplicateModeSceneButton(setup, scene, sceneHoldsClip);
			const int source = CLauncher::SourceScene(setup);
			if (change.fProperty != CPress::kNothing && source > 0) {
				for (int track = 0; track < CLauncher::kTrackCount; ++track) {
					const CPress::TButton button = LoadButton(customProps, CLauncher::ButtonIndex(track, source));
					const CPress::TChanges copy = CPress::CopyButton(button, CLauncher::ButtonIndex(track, scene));
					for (int i = 0; i < copy.fCount; ++i) {
						Apply(copy.fChanges[i], customProps, iArgs);
					}
				}
			}
			Apply(change, customProps, iArgs);
			if (change.fProperty != CPress::kNothing && source > 0) {
				iArgs.fGestureNameKey = "gesture_duplicate_scene";
			}
			return;
		}
		const CPress::TChanges changes = CPress::SceneButton(setup, scene, holdsClip);
		for (int i = 0; i < changes.fCount; ++i) {
			if (changes.fChanges[i].fProperty != CPress::kNothing) {
				Apply(changes.fChanges[i], customProps, iArgs);
			}
		}
		// All the changes of a press are one step of the undo history, named
		// after what the press was for.
		if (setup == CLauncher::kSetupClearMode) {
			iArgs.fGestureNameKey = "gesture_clear_scene";
		}
		else if (changes.fCount > 0) {
			iArgs.fGestureNameKey = changes.fChanges[0].fProperty == CPress::kLength ? "gesture_length" : "gesture_scene";
		}
		return;
	}
	if (iArgs.fDisplayInfo.fID != kDisplayID_Grid) {
		return;
	}

	int track = 0;
	int row = 0;
	if (!FindLaunchButton(iArgs.fStartState.fPoint, track, row)) {
		return;
	}
	iArgs.fConsumeFlag = true;

	// Clearing a launch button and returning to launch mode are one step of
	// the undo history.
	if (setup == CLauncher::kSetupClearMode) {
		const CPress::TChanges changes = CPress::ClearModeLaunchButton(setup, track, row, HoldsClip(customProps, CLauncher::ButtonIndex(track, row)));
		for (int i = 0; i < changes.fCount; ++i) {
			Apply(changes.fChanges[i], customProps, iArgs);
		}
		iArgs.fGestureNameKey = "gesture_clear_clip";
		return;
	}

	// Duplicating a launch button and returning to launch mode are one step
	// of the undo history.
	if (CLauncher::IsDuplicateMode(setup)) {
		const int source = CLauncher::SourceButton(setup);
		const int pressed = CLauncher::ButtonIndex(track, row);
		const CPress::TButton sourceButton = LoadButton(customProps, source >= 0 ? source : pressed);
		const CPress::TChanges changes = CPress::DuplicateModeLaunchButton(setup, track, row, HoldsClip(customProps, pressed), sourceButton);
		for (int i = 0; i < changes.fCount; ++i) {
			Apply(changes.fChanges[i], customProps, iArgs);
		}
		if (changes.fCount > 1) {
			iArgs.fGestureNameKey = "gesture_duplicate_clip";
		}
		return;
	}

	const int picked = CLauncher::PickedButton(setup);
	if (picked >= 0 && !CLauncher::IsClipEditing(setup)) {
		const CPress::TEdited edited = LoadEdited(customProps, picked, setup);
		const CPress::TClipValues clip = LoadClipValues(customProps, picked);
		CPress::TChanges changes;
		const char* presentKey;
		if (CLauncher::IsFillInEditing(setup)) {
			changes = CPress::FillInEditingLaunchButton(setup, track, row, edited, clip);
			presentKey = "gesture_fill_in_present";
		}
		else if (CLauncher::IsLandingEditing(setup)) {
			changes = CPress::LandingEditingLaunchButton(setup, track, row, edited, clip);
			presentKey = "gesture_landing_present";
		}
		else if (CLauncher::IsIntroEditing(setup)) {
			changes = CPress::IntroEditingLaunchButton(setup, track, row, edited, clip);
			presentKey = "gesture_intro_present";
		}
		else {
			changes = CPress::OutroEditingLaunchButton(setup, track, row, edited, clip);
			presentKey = "gesture_outro_present";
		}
		for (int i = 0; i < changes.fCount; ++i) {
			Apply(changes.fChanges[i], customProps, iArgs);
		}
		// Setting up a new fill-in, landing, intro or outro is one step of the
		// undo history, named after switching it on.
		if (changes.fCount > 1) {
			iArgs.fGestureNameKey = presentKey;
		}
		return;
	}
	const bool pressedHoldsClip = HoldsClip(customProps, CLauncher::ButtonIndex(track, row));
	const CPress::TEdited pickedClip = picked >= 0 ? LoadEdited(customProps, picked, setup) : CPress::Edited(false);
	// In launch mode the press holds the track's relaunch press down as well,
	// after the selection, which the realtime code compares it with; the undo
	// history names it after the selection.
	Apply(CPress::LaunchButton(setup, track, row, pressedHoldsClip, pickedClip), customProps, iArgs);
	const char* gestureNameKey = iArgs.fGestureNameKey;
	Apply(CPress::RelaunchButton(setup, track, true), customProps, iArgs);
	iArgs.fGestureNameKey = gestureNameKey;
}

void SetUpPanelDisplay(const TJBox_DisplayArgs& iArgs) {
	const TJBox_ObjectRef customProps = JBox_GetMotherboardObjectRef("/custom_properties");

	if (iArgs.fDisplayInfo.fID == kDisplayID_Grid) {
		TJBox_PropertyBox boxes[CLauncher::kButtonCount];
		for (int track = 0; track < CLauncher::kTrackCount; ++track) {
			const TJBox_PropertyRef selection = JBox_MakePropertyRef(customProps, kSelectionKeys[track]);
			for (int row = 1; row <= CLauncher::kRowCount; ++row) {
				boxes[CLauncher::ButtonIndex(track, row)] = MakeBox(selection, track * kTrackPitch, (row - 1) * kRowPitch, kButtonWidth, kButtonHeight);
			}
		}
		TJBox_PropertyInfo info;
		info.fBoxes = boxes;
		info.fCount = CLauncher::kButtonCount;
		JBox_SetPropertyInfo(&info);
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_FunctionButtons) {
		TJBox_PropertyBox boxes[CLauncher::kTrackCount];
		for (int track = 0; track < CLauncher::kTrackCount; ++track) {
			const TJBox_PropertyRef selection = JBox_MakePropertyRef(customProps, kSelectionKeys[track]);
			boxes[track] = MakeBox(selection, track * kTrackPitch, 0.0f, kFunctionButtonWidth, kFunctionButtonHeight);
		}
		TJBox_PropertyInfo info;
		info.fBoxes = boxes;
		info.fCount = CLauncher::kTrackCount;
		JBox_SetPropertyInfo(&info);
		return;
	}
	if (iArgs.fDisplayInfo.fID == kDisplayID_FillInButton) {
		const TJBox_PropertyBox box = MakeBox(JBox_MakePropertyRef(customProps, "fill_in"), 0.0f, 0.0f, kFillInWidth, kFillInHeight);
		TJBox_PropertyInfo info;
		info.fBoxes = &box;
		info.fCount = 1;
		JBox_SetPropertyInfo(&info);
	}
}
