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

// What a press in clip editing, fill-in editing or landing editing needs to
// know of the settings being edited, the clip's, its fill-in's or its
// landing's.
CPress::TEdited LoadEdited(TJBox_ObjectRef iCustomProps, int iButtonIndex, int iSetup) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	CPress::TEdited edited = CPress::Edited(false);
	TJBox_Tag transitionBase = kClipTransitionBase;
	edited.fPresent = HoldsClip(iCustomProps, iButtonIndex);
	if (CLauncher::IsFillInEditing(iSetup)) {
		transitionBase = kFillInTransitionBase;
		edited.fPresent = HoldsFillIn(iCustomProps, iButtonIndex);
		edited.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, FillInOneBarPatternTag(track, row)));
	}
	else if (CLauncher::IsLandingEditing(iSetup)) {
		transitionBase = kLandingTransitionBase;
		edited.fPresent = HoldsLanding(iCustomProps, iButtonIndex);
		edited.fOneBarPattern = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, LandingOneBarPatternTag(track, row)));
	}
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		edited.fCVModes[output] = LoadNumber(iCustomProps, CVModeTag(track, output));
		edited.fTransitions[output] = LoadNumber(iCustomProps, OutputSettingTag(transitionBase, track, row, output));
	}
	return edited;
}

CPress::TClipValues LoadClipValues(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	CPress::TClipValues values;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		values.fPatterns[output] = LoadNumber(iCustomProps, OutputSettingTag(kClipPatternBase, track, row, output));
		values.fUnipolar[output] = LoadNumber(iCustomProps, OutputSettingTag(kClipUnipolarBase, track, row, output));
		values.fBipolar[output] = LoadNumber(iCustomProps, OutputSettingTag(kClipBipolarBase, track, row, output));
	}
	return values;
}

void StoreNumber(TJBox_ObjectRef iCustomProps, TJBox_Tag iTag, int iValue) {
	JBox_StoreMOMPropertyByTag(iCustomProps, iTag, JBox_MakeNumber(iValue));
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
		case CPress::kClipPattern:
			StoreNumber(iCustomProps, OutputSettingTag(kClipPatternBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_clip_pattern";
			break;
		case CPress::kClipUnipolar:
			StoreNumber(iCustomProps, OutputSettingTag(kClipUnipolarBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_clip_unipolar";
			break;
		case CPress::kClipBipolar:
			StoreNumber(iCustomProps, OutputSettingTag(kClipBipolarBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_clip_bipolar";
			break;
		case CPress::kClipTransition:
			StoreNumber(iCustomProps, OutputSettingTag(kClipTransitionBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_clip_transition";
			break;
		case CPress::kClipColour:
			JBox_StoreMOMPropertyByTag(iCustomProps, ClipColourTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_clip_colour";
			break;
		case CPress::kFillInPresent:
			JBox_StoreMOMPropertyByTag(iCustomProps, FillInPresentTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_fill_in_present";
			break;
		case CPress::kFillInPattern:
			StoreNumber(iCustomProps, OutputSettingTag(kFillInPatternBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_fill_in_pattern";
			break;
		case CPress::kFillInUnipolar:
			StoreNumber(iCustomProps, OutputSettingTag(kFillInUnipolarBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_fill_in_unipolar";
			break;
		case CPress::kFillInBipolar:
			StoreNumber(iCustomProps, OutputSettingTag(kFillInBipolarBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_fill_in_bipolar";
			break;
		case CPress::kFillInTransition:
			StoreNumber(iCustomProps, OutputSettingTag(kFillInTransitionBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_fill_in_transition";
			break;
		case CPress::kFillInColour:
			JBox_StoreMOMPropertyByTag(iCustomProps, FillInColourTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_fill_in_colour";
			break;
		case CPress::kLandingPresent:
			JBox_StoreMOMPropertyByTag(iCustomProps, LandingPresentTag(track, row), JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_landing_present";
			break;
		case CPress::kLandingPattern:
			StoreNumber(iCustomProps, OutputSettingTag(kLandingPatternBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_landing_pattern";
			break;
		case CPress::kLandingUnipolar:
			StoreNumber(iCustomProps, OutputSettingTag(kLandingUnipolarBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_landing_unipolar";
			break;
		case CPress::kLandingBipolar:
			StoreNumber(iCustomProps, OutputSettingTag(kLandingBipolarBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_landing_bipolar";
			break;
		case CPress::kLandingTransition:
			StoreNumber(iCustomProps, OutputSettingTag(kLandingTransitionBase, track, row, iChange.fOutput), iChange.fValue);
			iArgs.fGestureNameKey = "gesture_landing_transition";
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
		for (int track = 0; track < CLauncher::kTrackCount; ++track) {
			holdsClip[track] = HoldsClip(customProps, CLauncher::ButtonIndex(track, scene));
		}
		const CPress::TChanges changes = CPress::SceneButton(setup, scene, holdsClip);
		for (int i = 0; i < changes.fCount; ++i) {
			if (changes.fChanges[i].fProperty != CPress::kNothing) {
				Apply(changes.fChanges[i], customProps, iArgs);
			}
		}
		// All the changes of a press are one step of the undo history, named
		// after what the press was for.
		if (changes.fCount > 0) {
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

	const int picked = CLauncher::PickedButton(setup);
	const bool fillInEditing = CLauncher::IsFillInEditing(setup);
	if (fillInEditing || CLauncher::IsLandingEditing(setup)) {
		const CPress::TEdited edited = LoadEdited(customProps, picked, setup);
		const CPress::TClipValues clip = LoadClipValues(customProps, picked);
		const CPress::TChanges changes = fillInEditing
			? CPress::FillInEditingLaunchButton(setup, track, row, edited, clip)
			: CPress::LandingEditingLaunchButton(setup, track, row, edited, clip);
		for (int i = 0; i < changes.fCount; ++i) {
			Apply(changes.fChanges[i], customProps, iArgs);
		}
		// Setting up a new fill-in or landing is one step of the undo history,
		// named after switching it on.
		if (changes.fCount > 1) {
			iArgs.fGestureNameKey = fillInEditing ? "gesture_fill_in_present" : "gesture_landing_present";
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
