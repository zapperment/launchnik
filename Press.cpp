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

// The properties of one kind of settings: a clip's, a fill-in's, a
// landing's, an intro's or an outro's.
struct TSettingsProperties {
	CPress::EProperty fPresent;
	CPress::EProperty fColour;
	CPress::EProperty fOutput;
	// kNothing for settings without a one-bar pattern switch.
	CPress::EProperty fOneBarPattern;
};

const TSettingsProperties kClipProperties = {
	CPress::kClipPresent, CPress::kClipColour, CPress::kClipOutput,
	CPress::kNothing,
};
const TSettingsProperties kFillInProperties = {
	CPress::kFillInPresent, CPress::kFillInColour, CPress::kFillInOutput,
	CPress::kFillInOneBarPattern,
};
const TSettingsProperties kLandingProperties = {
	CPress::kLandingPresent, CPress::kLandingColour, CPress::kLandingOutput,
	CPress::kLandingOneBarPattern,
};
const TSettingsProperties kIntroProperties = {
	CPress::kIntroPresent, CPress::kIntroColour, CPress::kIntroOutput,
	CPress::kIntroOneBarPattern,
};
const TSettingsProperties kOutroProperties = {
	CPress::kOutroPresent, CPress::kOutroColour, CPress::kOutroOutput,
	CPress::kOutroOneBarPattern,
};

// The settings of a CV output as one number.
int Pack(int iPattern, int iUnipolar, int iBipolar, int iTransition) {
	CLauncher::TOutputSettings settings;
	settings.fPattern = iPattern;
	settings.fUnipolar = iUnipolar;
	settings.fBipolar = iBipolar;
	settings.fTransition = iTransition;
	return CLauncher::PackOutputSettings(settings);
}

// What a press changes among the colours and the CV outputs of the settings
// being edited, the clip's, the fill-in's, the landing's, the intro's or the
// outro's, which must be there.
CPress::TChange EditSetting(const TSettingsProperties& iProperties, int iPicked, int iTrack, int iRow, const CPress::TEdited& iEdited) {
	const bool isColourRow = iRow == CLauncher::kEditingFirstColourRow || iRow == CLauncher::kEditingFirstColourRow + 1;
	if (isColourRow && iTrack < CLauncher::kEditingColoursPerRow) {
		const int colour = (iRow - CLauncher::kEditingFirstColourRow) * CLauncher::kEditingColoursPerRow + iTrack;
		return MakeChange(iProperties.fColour, iPicked, colour);
	}
	if (iProperties.fOneBarPattern != CPress::kNothing && iTrack == CLauncher::kEditingOneBarPatternTrack && iRow == CLauncher::kEditingOneBarPatternRow) {
		return MakeChange(iProperties.fOneBarPattern, iPicked, iEdited.fOneBarPattern ? 0 : 1);
	}
	const int output = iRow - CLauncher::kEditingFirstOutputRow;
	if (output < 0 || output >= CLauncher::kOutputCount || iTrack < 0 || iTrack >= CLauncher::kTrackCount) {
		return kNoChange;
	}
	// A press changes one of the output's settings and keeps the others.
	int pattern = iEdited.fPatterns[output];
	int unipolar = iEdited.fUnipolar[output];
	int bipolar = iEdited.fBipolar[output];
	int transition = iEdited.fTransitions[output];
	const int cvMode = iEdited.fCVModes[output];
	if (cvMode == CLauncher::kCVModePattern) {
		pattern = iTrack;
	}
	// The transition button goes round from off through A to D.
	else if (iTrack == CLauncher::kEditingTransitionTrack) {
		transition = (transition + 1) % CLauncher::kTransitionCount;
	}
	else if (cvMode == CLauncher::kCVModeUnipolar) {
		unipolar = iTrack;
	}
	else {
		bipolar = iTrack;
	}
	return MakeChange(iProperties.fOutput, iPicked, Pack(pattern, unipolar, bipolar, transition), output);
}

// A press of a launch button in fill-in editing, landing editing, intro
// editing or outro editing, where the grid stands for the fill-in, the
// landing, the intro or the outro as it stands for the clip in clip editing.
CPress::TChanges EditOverlay(const TSettingsProperties& iProperties, int iSetup, int iTrack, int iRow, const CPress::TEdited& iEdited, const CPress::TClipValues& iClip) {
	CPress::TChanges changes;
	changes.fCount = 0;
	const int picked = CLauncher::PickedButton(iSetup);
	if (iTrack == CLauncher::kEditingToggleTrack && iRow == CLauncher::kEditingToggleRow) {
		if (iEdited.fPresent) {
			changes.fChanges[changes.fCount++] = MakeChange(iProperties.fPresent, picked, 0);
			return changes;
		}
		// A new fill-in, landing, intro or outro starts as a copy of its clip's
		// values, with every transition off and one-bar pattern off, in white.
		changes.fChanges[changes.fCount++] = MakeChange(iProperties.fPresent, picked, 1);
		changes.fChanges[changes.fCount++] = MakeChange(iProperties.fColour, picked, CLauncher::kColourWhite);
		for (int output = 0; output < CLauncher::kOutputCount; ++output) {
			const int settings = Pack(iClip.fPatterns[output], iClip.fUnipolar[output], iClip.fBipolar[output], CLauncher::kTransitionOff);
			changes.fChanges[changes.fCount++] = MakeChange(iProperties.fOutput, picked, settings, output);
		}
		if (iProperties.fOneBarPattern != CPress::kNothing) {
			changes.fChanges[changes.fCount++] = MakeChange(iProperties.fOneBarPattern, picked, 0);
		}
		return changes;
	}
	// The colour and CV controls are not there without a fill-in, landing,
	// intro or outro.
	if (!iEdited.fPresent) {
		return changes;
	}
	const CPress::TChange change = EditSetting(iProperties, picked, iTrack, iRow, iEdited);
	if (change.fProperty != CPress::kNothing) {
		changes.fChanges[changes.fCount++] = change;
	}
	return changes;
}

// Appends what makes one kind of settings of a launch button exactly like
// those of another.
void AppendCopy(CPress::TChanges& ioChanges, const TSettingsProperties& iProperties, const CPress::TSettings& iSource, int iDestination) {
	ioChanges.fChanges[ioChanges.fCount++] = MakeChange(iProperties.fPresent, iDestination, iSource.fPresent ? 1 : 0);
	ioChanges.fChanges[ioChanges.fCount++] = MakeChange(iProperties.fColour, iDestination, iSource.fColour);
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		const int settings = Pack(iSource.fPatterns[output], iSource.fUnipolar[output], iSource.fBipolar[output], iSource.fTransitions[output]);
		ioChanges.fChanges[ioChanges.fCount++] = MakeChange(iProperties.fOutput, iDestination, settings, output);
	}
	if (iProperties.fOneBarPattern != CPress::kNothing) {
		ioChanges.fChanges[ioChanges.fCount++] = MakeChange(iProperties.fOneBarPattern, iDestination, iSource.fOneBarPattern ? 1 : 0);
	}
}

} // namespace

CPress::TEdited CPress::Edited(bool iPresent) {
	TEdited edited;
	edited.fPresent = iPresent;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		edited.fCVModes[output] = CLauncher::DefaultCVMode(output);
		edited.fPatterns[output] = 0;
		edited.fUnipolar[output] = 0;
		edited.fBipolar[output] = CLauncher::kBipolarZero;
		edited.fTransitions[output] = CLauncher::kTransitionOff;
	}
	edited.fOneBarPattern = false;
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
	return EditSetting(kClipProperties, picked, iTrack, iRow, iPicked);
}

CPress::TChanges CPress::FillInEditingLaunchButton(int iSetup, int iTrack, int iRow, const TEdited& iFillIn, const TClipValues& iClip) {
	if (!CLauncher::IsFillInEditing(iSetup)) {
		TChanges changes;
		changes.fCount = 0;
		return changes;
	}
	return EditOverlay(kFillInProperties, iSetup, iTrack, iRow, iFillIn, iClip);
}

CPress::TChanges CPress::LandingEditingLaunchButton(int iSetup, int iTrack, int iRow, const TEdited& iLanding, const TClipValues& iClip) {
	if (!CLauncher::IsLandingEditing(iSetup)) {
		TChanges changes;
		changes.fCount = 0;
		return changes;
	}
	return EditOverlay(kLandingProperties, iSetup, iTrack, iRow, iLanding, iClip);
}

CPress::TChanges CPress::IntroEditingLaunchButton(int iSetup, int iTrack, int iRow, const TEdited& iIntro, const TClipValues& iClip) {
	if (!CLauncher::IsIntroEditing(iSetup)) {
		TChanges changes;
		changes.fCount = 0;
		return changes;
	}
	return EditOverlay(kIntroProperties, iSetup, iTrack, iRow, iIntro, iClip);
}

CPress::TChanges CPress::OutroEditingLaunchButton(int iSetup, int iTrack, int iRow, const TEdited& iOutro, const TClipValues& iClip) {
	if (!CLauncher::IsOutroEditing(iSetup)) {
		TChanges changes;
		changes.fCount = 0;
		return changes;
	}
	return EditOverlay(kOutroProperties, iSetup, iTrack, iRow, iOutro, iClip);
}

CPress::TChanges CPress::ClearModeLaunchButton(int iSetup, int iTrack, int iRow, bool iPressedHoldsClip) {
	TChanges changes;
	changes.fCount = 0;
	if (iSetup != CLauncher::kSetupClearMode) {
		return changes;
	}
	if (iPressedHoldsClip) {
		changes.fChanges[changes.fCount++] = MakeChange(kClipPresent, CLauncher::ButtonIndex(iTrack, iRow), 0);
	}
	changes.fChanges[changes.fCount++] = MakeChange(kSetup, 0, CLauncher::kSetupLaunchMode);
	return changes;
}

CPress::TChanges CPress::DuplicateModeLaunchButton(int iSetup, int iTrack, int iRow, bool iPressedHoldsClip, const TButton& iSource) {
	TChanges changes;
	changes.fCount = 0;
	if (iSetup == CLauncher::kSetupSourcePicking) {
		if (iPressedHoldsClip) {
			changes.fChanges[changes.fCount++] = MakeChange(kSetup, 0, CLauncher::SetupForClipDestinationPicking(iTrack, iRow));
		}
		return changes;
	}
	const int source = CLauncher::SourceButton(iSetup);
	const int pressed = CLauncher::ButtonIndex(iTrack, iRow);
	if (source < 0 || pressed == source) {
		return changes;
	}
	changes = CopyButton(iSource, pressed);
	changes.fChanges[changes.fCount++] = MakeChange(kSetup, 0, CLauncher::kSetupLaunchMode);
	return changes;
}

CPress::TChange CPress::DuplicateModeSceneButton(int iSetup, int iScene, bool iSceneHoldsClip) {
	if (iSetup == CLauncher::kSetupSourcePicking) {
		return iSceneHoldsClip ? MakeChange(kSetup, 0, CLauncher::SetupForSceneDestinationPicking(iScene)) : kNoChange;
	}
	const int source = CLauncher::SourceScene(iSetup);
	if (source == 0 || iScene == source) {
		return kNoChange;
	}
	return MakeChange(kSetup, 0, CLauncher::kSetupLaunchMode);
}

CPress::TChanges CPress::CopyButton(const TButton& iSource, int iDestination) {
	TChanges changes;
	changes.fCount = 0;
	AppendCopy(changes, kClipProperties, iSource.fClip, iDestination);
	AppendCopy(changes, kFillInProperties, iSource.fFillIn, iDestination);
	AppendCopy(changes, kLandingProperties, iSource.fLanding, iDestination);
	AppendCopy(changes, kIntroProperties, iSource.fIntro, iDestination);
	AppendCopy(changes, kOutroProperties, iSource.fOutro, iDestination);
	return changes;
}

CPress::TChange CPress::RelaunchButton(int iSetup, int iTrack, bool iPressed) {
	if (iTrack < 0 || iTrack >= CLauncher::kTrackCount || (iPressed && iSetup != CLauncher::kSetupLaunchMode)) {
		return kNoChange;
	}
	return MakeChange(kRelaunch, iTrack, iPressed ? 1 : 0);
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
	if (iSetup == CLauncher::kSetupClearMode) {
		for (int track = 0; track < CLauncher::kTrackCount; ++track) {
			if (iHoldsClip[track]) {
				changes.fChanges[changes.fCount++] = MakeChange(kClipPresent, CLauncher::ButtonIndex(track, iScene), 0);
			}
		}
		changes.fChanges[changes.fCount++] = MakeChange(kSetup, 0, CLauncher::kSetupLaunchMode);
		return changes;
	}
	if (iSetup != CLauncher::kSetupLaunchMode) {
		return changes;
	}
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		changes.fChanges[changes.fCount++] = LaunchButton(iSetup, track, iScene, iHoldsClip[track], Edited(false));
	}
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		changes.fChanges[changes.fCount++] = RelaunchButton(iSetup, track, true);
	}
	return changes;
}

CPress::TChange CPress::SetupButton(int iSetup) {
	if (iSetup == CLauncher::kSetupClearMode || CLauncher::IsDuplicateMode(iSetup)) {
		return kNoChange;
	}
	const bool inSetupMode = iSetup != CLauncher::kSetupLaunchMode;
	return MakeChange(kSetup, 0, inSetupMode ? CLauncher::kSetupLaunchMode : CLauncher::kSetupClipPicking);
}

CPress::TChange CPress::ClearButton(int iSetup) {
	if (iSetup == CLauncher::kSetupLaunchMode) {
		return MakeChange(kSetup, 0, CLauncher::kSetupClearMode);
	}
	if (iSetup == CLauncher::kSetupClearMode) {
		return MakeChange(kSetup, 0, CLauncher::kSetupLaunchMode);
	}
	return kNoChange;
}

CPress::TChange CPress::DuplicateButton(int iSetup) {
	if (iSetup == CLauncher::kSetupLaunchMode) {
		return MakeChange(kSetup, 0, CLauncher::kSetupSourcePicking);
	}
	if (CLauncher::IsDuplicateMode(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupLaunchMode);
	}
	return kNoChange;
}

CPress::TChange CPress::BackButton(int iSetup) {
	if (iSetup == CLauncher::kSetupLaunchMode) {
		return kNoChange;
	}
	if (iSetup == CLauncher::kSetupClearMode || CLauncher::IsDuplicateMode(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupLaunchMode);
	}
	if (CLauncher::IsOutroEditing(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupFirstIntroEditing + CLauncher::PickedButton(iSetup));
	}
	if (CLauncher::IsIntroEditing(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupFirstLandingEditing + CLauncher::PickedButton(iSetup));
	}
	if (CLauncher::IsLandingEditing(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupFirstFillInEditing + CLauncher::PickedButton(iSetup));
	}
	if (CLauncher::IsFillInEditing(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupFirstClipEditing + CLauncher::PickedButton(iSetup));
	}
	const bool inClipPicking = iSetup == CLauncher::kSetupClipPicking;
	return MakeChange(kSetup, 0, inClipPicking ? CLauncher::kSetupLaunchMode : CLauncher::kSetupClipPicking);
}

CPress::TChange CPress::ForwardButton(int iSetup, bool iPickedHoldsClip) {
	if (!iPickedHoldsClip) {
		return kNoChange;
	}
	if (CLauncher::IsClipEditing(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupFirstFillInEditing + CLauncher::PickedButton(iSetup));
	}
	if (CLauncher::IsFillInEditing(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupFirstLandingEditing + CLauncher::PickedButton(iSetup));
	}
	if (CLauncher::IsLandingEditing(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupFirstIntroEditing + CLauncher::PickedButton(iSetup));
	}
	if (CLauncher::IsIntroEditing(iSetup)) {
		return MakeChange(kSetup, 0, CLauncher::kSetupFirstOutroEditing + CLauncher::PickedButton(iSetup));
	}
	return kNoChange;
}

CPress::TChange CPress::FillInButton(bool iPressed) {
	return MakeChange(kFillIn, 0, iPressed ? 1 : 0);
}
