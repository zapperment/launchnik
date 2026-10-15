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
	if (!CLauncher::IsClipEditing(iSetup)) {
		return kNoChange;
	}

	const int picked = CLauncher::PickedButton(iSetup);
	if (iTrack == CLauncher::kEditingToggleTrack && iRow == CLauncher::kEditingToggleRow) {
		return MakeChange(kClipPresent, picked, iPickedHoldsClip ? 0 : 1);
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

CPress::TChanges CPress::FillInEditingLaunchButton(int iSetup, int iTrack, int iRow, bool iPickedHoldsFillIn, int iPickedPattern) {
	TChanges changes;
	changes.fCount = 0;
	if (!CLauncher::IsFillInEditing(iSetup)) {
		return changes;
	}

	const int picked = CLauncher::PickedButton(iSetup);
	if (iTrack == CLauncher::kEditingToggleTrack && iRow == CLauncher::kEditingToggleRow) {
		if (iPickedHoldsFillIn) {
			changes.fChanges[changes.fCount++] = MakeChange(kFillInPresent, picked, 0);
			return changes;
		}
		// A new fill-in starts with the pattern of its clip, in white.
		changes.fChanges[changes.fCount++] = MakeChange(kFillInPresent, picked, 1);
		changes.fChanges[changes.fCount++] = MakeChange(kFillInPattern, picked, iPickedPattern);
		changes.fChanges[changes.fCount++] = MakeChange(kFillInColour, picked, CLauncher::kColourWhite);
		return changes;
	}
	// The colour and pattern controls are not there without a fill-in.
	if (!iPickedHoldsFillIn) {
		return changes;
	}
	const bool isColourRow = iRow == CLauncher::kEditingFirstColourRow || iRow == CLauncher::kEditingFirstColourRow + 1;
	if (isColourRow && iTrack < CLauncher::kEditingColoursPerRow) {
		const int colour = (iRow - CLauncher::kEditingFirstColourRow) * CLauncher::kEditingColoursPerRow + iTrack;
		changes.fChanges[changes.fCount++] = MakeChange(kFillInColour, picked, colour);
	}
	else if (iRow == CLauncher::kEditingPatternRow) {
		changes.fChanges[changes.fCount++] = MakeChange(kFillInPattern, picked, iTrack);
	}
	return changes;
}

CPress::TChange CPress::StopButton(int iTrack) {
	return MakeChange(kSelection, iTrack, CLauncher::kSelectionStop);
}

CPress::TChanges CPress::SceneButton(int iSetup, int iScene, const bool iHoldsClip[]) {
	TChanges changes;
	changes.fCount = 0;
	if (iSetup == CLauncher::kSetupClipPicking) {
		changes.fChanges[changes.fCount++] = MakeChange(kLength, 0, CLauncher::LengthForScene(iScene));
		changes.fChanges[changes.fCount++] = MakeChange(kSetup, 0, CLauncher::kSetupLaunchMode);
		return changes;
	}
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

CPress::TChange CPress::BackButton(int iSetup) {
	if (iSetup == CLauncher::kSetupLaunchMode) {
		return kNoChange;
	}
	if (CLauncher::IsFillInEditing(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupFirstClipEditing + CLauncher::PickedButton(iSetup));
	}
	const bool inClipPicking = iSetup == CLauncher::kSetupClipPicking;
	return MakeChange(kSetup, 0, inClipPicking ? CLauncher::kSetupLaunchMode : CLauncher::kSetupClipPicking);
}

CPress::TChange CPress::SetFillInButton(int iSetup, bool iPickedHoldsClip) {
	if (!CLauncher::IsClipEditing(iSetup) || !iPickedHoldsClip) {
		return kNoChange;
	}
	return MakeChange(kSetup, 0, CLauncher::kSetupFirstFillInEditing + CLauncher::PickedButton(iSetup));
}

CPress::TChange CPress::FillInButton(bool iPressed) {
	return MakeChange(kFillIn, 0, iPressed ? 1 : 0);
}
