#include "Panel.h"

#include "Launcher.h"
#include "Press.h"
#include "Tags.h"

namespace {

// Layout of the grid display, in its own pixels; matches GUI2D/device_2D.lua.
const TJBox_Float32 kTrackPitch = 252.0f;
const TJBox_Float32 kRowPitch = 235.0f;
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

int ClipPattern(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	return static_cast<int>(JBox_GetNumber(JBox_LoadMOMPropertyByTag(iCustomProps, ClipPatternTag(track, row))));
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
			JBox_StoreMOMPropertyByTag(iCustomProps, ClipPatternTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_clip_pattern";
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
			JBox_StoreMOMPropertyByTag(iCustomProps, FillInPatternTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_fill_in_pattern";
			break;
		case CPress::kFillInColour:
			JBox_StoreMOMPropertyByTag(iCustomProps, FillInColourTag(track, row), JBox_MakeNumber(iChange.fValue));
			iArgs.fGestureNameKey = "gesture_fill_in_colour";
			break;
		case CPress::kFillIn:
			JBox_StoreMOMPropertyByTag(iCustomProps, kCustomPropsTag_FillIn, JBox_MakeBoolean(iChange.fValue != 0));
			iArgs.fGestureNameKey = "gesture_fill_in";
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
	if (iArgs.fCall != kJBox_OnTap || iArgs.fTap != kJBox_TapRegular) {
		return;
	}
	const TJBox_ObjectRef customProps = JBox_GetMotherboardObjectRef("/custom_properties");
	const int setup = static_cast<int>(JBox_GetNumber(JBox_LoadMOMPropertyByTag(customProps, kCustomPropsTag_Setup)));

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
	if (iArgs.fDisplayInfo.fID == kDisplayID_SetFillInButton) {
		iArgs.fConsumeFlag = true;
		const int picked = CLauncher::PickedButton(setup);
		const bool pickedHoldsClip = picked >= 0 && HoldsClip(customProps, picked);
		Apply(CPress::SetFillInButton(setup, pickedHoldsClip), customProps, iArgs);
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
			Apply(changes.fChanges[i], customProps, iArgs);
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
	if (CLauncher::IsFillInEditing(setup)) {
		const CPress::TChanges changes = CPress::FillInEditingLaunchButton(setup, track, row, HoldsFillIn(customProps, picked), ClipPattern(customProps, picked));
		for (int i = 0; i < changes.fCount; ++i) {
			Apply(changes.fChanges[i], customProps, iArgs);
		}
		// Setting up a new fill-in is one step of the undo history, named after
		// switching it on.
		if (changes.fCount > 1) {
			iArgs.fGestureNameKey = "gesture_fill_in_present";
		}
		return;
	}
	const bool pressedHoldsClip = HoldsClip(customProps, CLauncher::ButtonIndex(track, row));
	const bool pickedHoldsClip = picked >= 0 && HoldsClip(customProps, picked);
	Apply(CPress::LaunchButton(setup, track, row, pressedHoldsClip, pickedHoldsClip), customProps, iArgs);
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
	if (iArgs.fDisplayInfo.fID == kDisplayID_FillInButton) {
		const TJBox_PropertyBox box = MakeBox(JBox_MakePropertyRef(customProps, "fill_in"), 0.0f, 0.0f, kFillInWidth, kFillInHeight);
		TJBox_PropertyInfo info;
		info.fBoxes = &box;
		info.fCount = 1;
		JBox_SetPropertyInfo(&info);
	}
}
