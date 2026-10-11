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

bool HoldsClip(TJBox_ObjectRef iCustomProps, int iButtonIndex) {
	const int track = iButtonIndex / CLauncher::kRowCount;
	const int row = iButtonIndex % CLauncher::kRowCount + 1;
	return JBox_GetBoolean(JBox_LoadMOMPropertyByTag(iCustomProps, ClipPresentTag(track, row)));
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
		case CPress::kNothing:
			break;
	}
}

} // namespace

void HandlePanelGesture(TJBox_GestureArgs& iArgs) {
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
	if (iArgs.fDisplayInfo.fID != kDisplayID_Grid) {
		return;
	}

	int track = 0;
	int row = 0;
	if (!FindLaunchButton(iArgs.fStartState.fPoint, track, row)) {
		return;
	}
	iArgs.fConsumeFlag = true;

	const bool pressedHoldsClip = HoldsClip(customProps, CLauncher::ButtonIndex(track, row));
	const bool isClipEditing = setup >= CLauncher::kSetupFirstClipEditing;
	const bool pickedHoldsClip = isClipEditing && HoldsClip(customProps, setup - CLauncher::kSetupFirstClipEditing);
	Apply(CPress::LaunchButton(setup, track, row, pressedHoldsClip, pickedHoldsClip), customProps, iArgs);
}
