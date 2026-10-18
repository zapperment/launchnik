#include "Press.h"

#include "Launcher.h"

namespace {

CPress::TChange MakeChange(CPress::EProperty iProperty, int iIndex, int iValue, int iOutput = 0) {
	CPress::TChange change;
	change.fProperty = iProperty;
	change.fIndex = iIndex;
	change.fOutput = iOutput;
	change.fValue = iValue;
	return change;
}

const CPress::TChange kNoChange = { CPress::kNothing, 0, 0, 0 };

// What a press changes among the colours and the CV outputs of the settings
// being edited, the clip's or the fill-in's, which must be there.
CPress::TChange EditSetting(bool iFillIn, int iPicked, int iTrack, int iRow, const CPress::TEdited& iEdited) {
	const bool isColourRow = iRow == CLauncher::kEditingFirstColourRow || iRow == CLauncher::kEditingFirstColourRow + 1;
	if (isColourRow && iTrack < CLauncher::kEditingColoursPerRow) {
		const int colour = (iRow - CLauncher::kEditingFirstColourRow) * CLauncher::kEditingColoursPerRow + iTrack;
		return MakeChange(iFillIn ? CPress::kFillInColour : CPress::kClipColour, iPicked, colour);
	}
	const int output = iRow - CLauncher::kEditingFirstOutputRow;
	if (output < 0 || output >= CLauncher::kOutputCount || iTrack < 0 || iTrack >= CLauncher::kTrackCount) {
		return kNoChange;
	}
	const int cvMode = iEdited.fCVModes[output];
	if (cvMode == CLauncher::kCVModePattern) {
		return MakeChange(iFillIn ? CPress::kFillInPattern : CPress::kClipPattern, iPicked, iTrack, output);
	}
	// The transition button goes round from off through A to D.
	if (iTrack == CLauncher::kEditingTransitionTrack) {
		const int transition = (iEdited.fTransitions[output] + 1) % CLauncher::kTransitionCount;
		return MakeChange(iFillIn ? CPress::kFillInTransition : CPress::kClipTransition, iPicked, transition, output);
	}
	if (cvMode == CLauncher::kCVModeUnipolar) {
		return MakeChange(iFillIn ? CPress::kFillInUnipolar : CPress::kClipUnipolar, iPicked, iTrack, output);
	}
	return MakeChange(iFillIn ? CPress::kFillInBipolar : CPress::kClipBipolar, iPicked, iTrack, output);
}

} // namespace

CPress::TEdited CPress::Edited(bool iPresent) {
	TEdited edited;
	edited.fPresent = iPresent;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		edited.fCVModes[output] = CLauncher::DefaultCVMode(output);
		edited.fTransitions[output] = CLauncher::kTransitionOff;
	}
	return edited;
}

CPress::TChange CPress::LaunchButton(int iSetup, int iTrack, int iRow, bool iPressedHoldsClip, const TEdited& iPicked) {
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
		return MakeChange(kClipPresent, picked, iPicked.fPresent ? 0 : 1);
	}
	// The colour and CV controls are not there without a clip.
	if (!iPicked.fPresent) {
		return kNoChange;
	}
	return EditSetting(false, picked, iTrack, iRow, iPicked);
}

CPress::TChanges CPress::FillInEditingLaunchButton(int iSetup, int iTrack, int iRow, const TEdited& iFillIn, const TClipValues& iClip) {
	TChanges changes;
	changes.fCount = 0;
	if (!CLauncher::IsFillInEditing(iSetup)) {
		return changes;
	}

	const int picked = CLauncher::PickedButton(iSetup);
	if (iTrack == CLauncher::kEditingToggleTrack && iRow == CLauncher::kEditingToggleRow) {
		if (iFillIn.fPresent) {
			changes.fChanges[changes.fCount++] = MakeChange(kFillInPresent, picked, 0);
			return changes;
		}
		// A new fill-in starts as a copy of its clip's values, with every
		// transition off, in white.
		changes.fChanges[changes.fCount++] = MakeChange(kFillInPresent, picked, 1);
		changes.fChanges[changes.fCount++] = MakeChange(kFillInColour, picked, CLauncher::kColourWhite);
		for (int output = 0; output < CLauncher::kOutputCount; ++output) {
			changes.fChanges[changes.fCount++] = MakeChange(kFillInPattern, picked, iClip.fPatterns[output], output);
			changes.fChanges[changes.fCount++] = MakeChange(kFillInUnipolar, picked, iClip.fUnipolar[output], output);
			changes.fChanges[changes.fCount++] = MakeChange(kFillInBipolar, picked, iClip.fBipolar[output], output);
			changes.fChanges[changes.fCount++] = MakeChange(kFillInTransition, picked, CLauncher::kTransitionOff, output);
		}
		return changes;
	}
	// The colour and CV controls are not there without a fill-in.
	if (!iFillIn.fPresent) {
		return changes;
	}
	const TChange change = EditSetting(true, picked, iTrack, iRow, iFillIn);
	if (change.fProperty != kNothing) {
		changes.fChanges[changes.fCount++] = change;
	}
	return changes;
}

CPress::TChange CPress::StopButton(int iTrack) {
	return MakeChange(kSelection, iTrack, CLauncher::kSelectionStop);
}

CPress::TChange CPress::FunctionButton(int iFunction, int iTrack, bool iTrackHasClips, bool iPressed) {
	if (iTrack < 0 || iTrack >= CLauncher::kTrackCount) {
		return kNoChange;
	}
	if (iPressed && !iTrackHasClips) {
		return kNoChange;
	}
	if (iFunction == CLauncher::kFunctionMute) {
		return MakeChange(kMute, iTrack, iPressed ? 1 : 0);
	}
	if (iFunction == CLauncher::kFunctionSolo) {
		return MakeChange(kSolo, iTrack, iPressed ? 1 : 0);
	}
	return iPressed ? StopButton(iTrack) : kNoChange;
}

CPress::TChange CPress::FunctionSelectButton(int iFunction) {
	if (iFunction < 0 || iFunction >= CLauncher::kFunctionCount) {
		return kNoChange;
	}
	return MakeChange(kFunction, 0, iFunction);
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
		changes.fChanges[changes.fCount++] = LaunchButton(iSetup, track, iScene, iHoldsClip[track], Edited(false));
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
