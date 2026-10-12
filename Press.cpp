#include "Press.h"

#include "Launcher.h"

namespace {

CPress::TChange MakeChange(CPress::EProperty iProperty, int iIndex, int iValue) {
	CPress::TChange change;
	change.fProperty = iProperty;
	change.fIndex = iIndex;
	change.fValue = iValue;
	return change;
}

const CPress::TChange kNoChange = { CPress::kNothing, 0, 0 };

} // namespace

CPress::TChange CPress::LaunchButton(int iSetup, int iTrack, int iRow, bool iPressedHoldsClip, bool iPickedHoldsClip) {
	if (iSetup == CLauncher::kSetupLaunchMode) {
		// An empty launch button stands in for the stop button.
		return MakeChange(kSelection, iTrack, iPressedHoldsClip ? iRow : CLauncher::kSelectionStop);
	}
	if (iSetup == CLauncher::kSetupClipPicking) {
		return MakeChange(kSetup, 0, CLauncher::SetupForClipEditing(iTrack, iRow));
	}

	const int picked = iSetup - CLauncher::kSetupFirstClipEditing;
	if (iTrack == CLauncher::kEditingToggleTrack && iRow == CLauncher::kEditingToggleRow) {
		return MakeChange(kClipPresent, picked, iPickedHoldsClip ? 0 : 1);
	}
	if (iTrack == CLauncher::kEditingBackTrack && iRow == CLauncher::kEditingBackRow) {
		return MakeChange(kSetup, 0, CLauncher::kSetupClipPicking);
	}
	// The colour and pattern controls are not there without a clip.
	if (!iPickedHoldsClip) {
		return kNoChange;
	}
	const bool isColourRow = iRow == CLauncher::kEditingFirstColourRow || iRow == CLauncher::kEditingFirstColourRow + 1;
	if (isColourRow && iTrack < CLauncher::kEditingColoursPerRow) {
		const int colour = (iRow - CLauncher::kEditingFirstColourRow) * CLauncher::kEditingColoursPerRow + iTrack;
		return MakeChange(kClipColour, picked, colour);
	}
	if (iRow == CLauncher::kEditingPatternRow) {
		return MakeChange(kClipPattern, picked, iTrack);
	}
	return kNoChange;
}

CPress::TChange CPress::StopButton(int iTrack) {
	return MakeChange(kSelection, iTrack, CLauncher::kSelectionStop);
}

CPress::TChanges CPress::SceneButton(int iSetup, int iScene, const bool iHoldsClip[]) {
	TChanges changes;
	changes.fCount = 0;
	if (iSetup != CLauncher::kSetupLaunchMode) {
		return changes;
	}
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		changes.fChanges[changes.fCount++] = LaunchButton(iSetup, track, iScene, iHoldsClip[track], false);
	}
	return changes;
}

CPress::TChange CPress::SetupButton(int iSetup) {
	const bool inSetupMode = iSetup != CLauncher::kSetupLaunchMode;
	return MakeChange(kSetup, 0, inSetupMode ? CLauncher::kSetupLaunchMode : CLauncher::kSetupClipPicking);
}
