// Command-line tests for what a press changes. Run with tests/run.sh.
//
// A codec for a control surface has to pass the same cases; see docs/adr/0002.

#include "../Launcher.h"
#include "../Press.h"

#include "Check.h"

namespace {

bool IsChange(const CPress::TChange& iChange, CPress::EProperty iProperty, int iIndex, int iValue) {
	return iChange.fProperty == iProperty && iChange.fIndex == iIndex && iChange.fValue == iValue;
}

bool IsNoChange(const CPress::TChange& iChange) {
	return iChange.fProperty == CPress::kNothing;
}

const int kLaunchMode = CLauncher::kSetupLaunchMode;
const int kClipPicking = CLauncher::kSetupClipPicking;

void TestLaunchModeSelectsTheClip() {
	CHECK(IsChange(CPress::LaunchButton(kLaunchMode, 2, 5, true, CPress::Edited(false)), CPress::kSelection, 2, 5));
	CHECK(IsChange(CPress::LaunchButton(kLaunchMode, 7, 8, true, CPress::Edited(false)), CPress::kSelection, 7, 8));
}

void TestLaunchModeEmptyLaunchButtonSelectsStop() {
	CHECK(IsChange(CPress::LaunchButton(kLaunchMode, 2, 5, false, CPress::Edited(false)), CPress::kSelection, 2, CLauncher::kSelectionStop));
}

void TestStopButtonSelectsStopInEveryMode() {
	CHECK(IsChange(CPress::StopButton(4), CPress::kSelection, 4, CLauncher::kSelectionStop));
}

void TestSetupButtonSwitchesBetweenLaunchModeAndClipPicking() {
	CHECK(IsChange(CPress::SetupButton(kLaunchMode), CPress::kSetup, 0, kClipPicking));
	CHECK(IsChange(CPress::SetupButton(kClipPicking), CPress::kSetup, 0, kLaunchMode));
	// From clip editing it leaves setup mode altogether.
	CHECK(IsChange(CPress::SetupButton(CLauncher::SetupForClipEditing(3, 3)), CPress::kSetup, 0, kLaunchMode));
}

void TestClipPickingOpensThePressedLaunchButton() {
	CHECK(IsChange(CPress::LaunchButton(kClipPicking, 0, 1, false, CPress::Edited(false)), CPress::kSetup, 0, 2));
	CHECK(IsChange(CPress::LaunchButton(kClipPicking, 7, 8, true, CPress::Edited(false)), CPress::kSetup, 0, 65));
	CHECK(IsChange(CPress::LaunchButton(kClipPicking, 3, 6, true, CPress::Edited(false)), CPress::kSetup, 0, CLauncher::SetupForClipEditing(3, 6)));
}

void TestClipEditingToggleSwitchesThePickedClip() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	CHECK(IsChange(CPress::LaunchButton(editing, 7, 1, false, CPress::Edited(false)), CPress::kClipPresent, picked, 1));
	CHECK(IsChange(CPress::LaunchButton(editing, 7, 1, false, CPress::Edited(true)), CPress::kClipPresent, picked, 0));
}

void TestBackButtonGoesBackOneStep() {
	CHECK(IsNoChange(CPress::BackButton(kLaunchMode)));
	CHECK(IsChange(CPress::BackButton(kClipPicking), CPress::kSetup, 0, kLaunchMode));
	CHECK(IsChange(CPress::BackButton(CLauncher::SetupForClipEditing(3, 6)), CPress::kSetup, 0, kClipPicking));
	// From fill-in editing it returns to clip editing of the same launch
	// button, and from landing editing to fill-in editing.
	CHECK(IsChange(CPress::BackButton(CLauncher::SetupForFillInEditing(3, 6)), CPress::kSetup, 0, CLauncher::SetupForClipEditing(3, 6)));
	CHECK(IsChange(CPress::BackButton(CLauncher::SetupForLandingEditing(3, 6)), CPress::kSetup, 0, CLauncher::SetupForFillInEditing(3, 6)));
	// From intro editing to landing editing, and from outro editing to intro
	// editing.
	CHECK(IsChange(CPress::BackButton(CLauncher::SetupForIntroEditing(3, 6)), CPress::kSetup, 0, CLauncher::SetupForLandingEditing(3, 6)));
	CHECK(IsChange(CPress::BackButton(CLauncher::SetupForOutroEditing(3, 6)), CPress::kSetup, 0, CLauncher::SetupForIntroEditing(3, 6)));
}

// The four settings of a CV output that a change sets, all in one number.
enum ESetting { kPatternSetting, kUnipolarSetting, kBipolarSetting, kTransitionSetting };

int SettingOf(int iPacked, ESetting iSetting) {
	const CLauncher::TOutputSettings settings = CLauncher::UnpackOutputSettings(iPacked);
	switch (iSetting) {
		case kPatternSetting: return settings.fPattern;
		case kUnipolarSetting: return settings.fUnipolar;
		case kBipolarSetting: return settings.fBipolar;
		default: return settings.fTransition;
	}
}

// Whether a change sets a CV output's settings, with one of them iValue.
bool IsSettingChange(const CPress::TChange& iChange, CPress::EProperty iProperty, ESetting iSetting, int iIndex, int iOutput, int iValue) {
	return iChange.fProperty == iProperty && iChange.fIndex == iIndex && iChange.fOutput == iOutput && SettingOf(iChange.fValue, iSetting) == iValue;
}

// Output A in pattern mode, B unipolar, C bipolar, the others as on a new
// device; transitions off, except C at B and D at D.
CPress::TEdited MixedModes(bool iPresent) {
	CPress::TEdited edited = CPress::Edited(iPresent);
	edited.fCVModes[2] = CLauncher::kCVModeBipolar;
	edited.fTransitions[2] = CLauncher::kTransitionB;
	edited.fTransitions[3] = CLauncher::kTransitionD;
	return edited;
}

void TestClipEditingColourRows() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	for (int column = 0; column < 7; ++column) {
		CHECK(IsChange(CPress::LaunchButton(editing, column, 1, false, CPress::Edited(true)), CPress::kClipColour, picked, column));
		CHECK(IsChange(CPress::LaunchButton(editing, column, 2, false, CPress::Edited(true)), CPress::kClipColour, picked, 7 + column));
	}
	// Column 8 of the second colour row is not a control.
	CHECK(IsNoChange(CPress::LaunchButton(editing, 7, 2, false, CPress::Edited(true))));
}

void TestClipEditingPatternMode() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	// Output A, in row 3, is in pattern mode: all eight columns are patterns.
	for (int column = 0; column < 8; ++column) {
		CHECK(IsSettingChange(CPress::LaunchButton(editing, column, 3, false, MixedModes(true)), CPress::kClipOutput, kPatternSetting, picked, 0, column));
	}
}

void TestClipEditingUnipolarAndBipolarValues() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	for (int column = 0; column < 7; ++column) {
		CHECK(IsSettingChange(CPress::LaunchButton(editing, column, 4, false, MixedModes(true)), CPress::kClipOutput, kUnipolarSetting, picked, 1, column));
		CHECK(IsSettingChange(CPress::LaunchButton(editing, column, 5, false, MixedModes(true)), CPress::kClipOutput, kBipolarSetting, picked, 2, column));
		CHECK(IsSettingChange(CPress::LaunchButton(editing, column, 8, false, MixedModes(true)), CPress::kClipOutput, kUnipolarSetting, picked, 5, column));
	}
}

void TestClipEditingTransitionButtonGoesRound() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	CHECK(IsSettingChange(CPress::LaunchButton(editing, 7, 4, false, MixedModes(true)), CPress::kClipOutput, kTransitionSetting, picked, 1, CLauncher::kTransitionA));
	CHECK(IsSettingChange(CPress::LaunchButton(editing, 7, 5, false, MixedModes(true)), CPress::kClipOutput, kTransitionSetting, picked, 2, CLauncher::kTransitionC));
	// After D it is off again.
	CHECK(IsSettingChange(CPress::LaunchButton(editing, 7, 6, false, MixedModes(true)), CPress::kClipOutput, kTransitionSetting, picked, 3, CLauncher::kTransitionOff));
}

void TestClipEditingUnusedLaunchButtonsDoNothing() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	CHECK(IsNoChange(CPress::LaunchButton(editing, 7, 2, true, CPress::Edited(true))));
}

void TestClipEditingWithoutClipHasNoColourOrCVControls() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	for (int row = 1; row <= 8; ++row) {
		CHECK(IsNoChange(CPress::LaunchButton(editing, 2, row, false, MixedModes(false))));
	}
	CHECK(IsNoChange(CPress::LaunchButton(editing, 7, 4, false, MixedModes(false))));
}

void TestLaunchHoldsTheRelaunchPressInLaunchModeOnly() {
	CHECK(IsChange(CPress::RelaunchButton(kLaunchMode, 2, true), CPress::kRelaunch, 2, 1));
	CHECK(IsNoChange(CPress::RelaunchButton(kClipPicking, 2, true)));
	CHECK(IsNoChange(CPress::RelaunchButton(CLauncher::SetupForClipEditing(3, 6), 2, true)));
	// Letting go is passed on in every mode.
	CHECK(IsChange(CPress::RelaunchButton(kClipPicking, 2, false), CPress::kRelaunch, 2, 0));
	CHECK(IsNoChange(CPress::RelaunchButton(kLaunchMode, 8, true)));
}

void TestSceneButtonSelectsItsRowOnEveryTrack() {
	const bool holdsClip[8] = { true, true, true, true, true, true, true, true };
	const CPress::TChanges changes = CPress::SceneButton(kLaunchMode, 3, holdsClip);
	CHECK(changes.fCount == 16);
	for (int track = 0; track < 8; ++track) {
		CHECK(IsChange(changes.fChanges[track], CPress::kSelection, track, 3));
		// Each track's relaunch press is held down as well.
		CHECK(IsChange(changes.fChanges[8 + track], CPress::kRelaunch, track, 1));
	}
}

void TestSceneButtonStopsTracksWithAnEmptyLaunchButton() {
	const bool holdsClip[8] = { true, false, false, true, false, false, false, true };
	const CPress::TChanges changes = CPress::SceneButton(kLaunchMode, 8, holdsClip);
	CHECK(changes.fCount == 16);
	for (int track = 0; track < 8; ++track) {
		CHECK(IsChange(changes.fChanges[track], CPress::kSelection, track, holdsClip[track] ? 8 : CLauncher::kSelectionStop));
	}
}

void TestSceneButtonOfAnEmptySceneStopsEveryTrack() {
	const bool holdsClip[8] = { false, false, false, false, false, false, false, false };
	const CPress::TChanges changes = CPress::SceneButton(kLaunchMode, 1, holdsClip);
	CHECK(changes.fCount == 16);
	for (int track = 0; track < 8; ++track) {
		CHECK(IsChange(changes.fChanges[track], CPress::kSelection, track, CLauncher::kSelectionStop));
	}
}

void TestSceneButtonDoesNothingInClipEditing() {
	const bool holdsClip[8] = { true, true, true, true, true, true, true, true };
	CHECK(CPress::SceneButton(CLauncher::SetupForClipEditing(3, 6), 3, holdsClip).fCount == 0);
}

void TestSceneButtonInClipPickingChoosesItsLengthAndLeavesSetupMode() {
	const bool holdsClip[8] = { true, true, true, true, true, true, true, true };
	// From the bottom: immediate, one beat, half a bar, 1, 2, 4, 8 and 16 bars.
	for (int scene = 1; scene <= 8; ++scene) {
		const CPress::TChanges changes = CPress::SceneButton(kClipPicking, scene, holdsClip);
		CHECK(changes.fCount == 2);
		CHECK(IsChange(changes.fChanges[0], CPress::kLength, 0, 8 - scene));
		CHECK(IsChange(changes.fChanges[1], CPress::kSetup, 0, kLaunchMode));
	}
}

const int kClearMode = CLauncher::kSetupClearMode;

void TestClearButtonSwitchesBetweenLaunchModeAndClearMode() {
	CHECK(IsChange(CPress::ClearButton(kLaunchMode), CPress::kSetup, 0, kClearMode));
	CHECK(IsChange(CPress::ClearButton(kClearMode), CPress::kSetup, 0, kLaunchMode));
	// In setup mode it does nothing.
	CHECK(IsNoChange(CPress::ClearButton(kClipPicking)));
	CHECK(IsNoChange(CPress::ClearButton(CLauncher::SetupForClipEditing(3, 6))));
	CHECK(IsNoChange(CPress::ClearButton(CLauncher::SetupForFillInEditing(3, 6))));
	CHECK(IsNoChange(CPress::ClearButton(CLauncher::SetupForLandingEditing(3, 6))));
}

void TestBackButtonLeavesClearMode() {
	CHECK(IsNoChange(CPress::SetupButton(kClearMode)));
	CHECK(IsChange(CPress::BackButton(kClearMode), CPress::kSetup, 0, kLaunchMode));
	CHECK(IsNoChange(CPress::ForwardButton(kClearMode, true)));
}

void TestClearModeEmptiesThePressedLaunchButtonAndReturns() {
	const CPress::TChanges changes = CPress::ClearModeLaunchButton(kClearMode, 3, 6, true);
	CHECK(changes.fCount == 2);
	CHECK(IsChange(changes.fChanges[0], CPress::kClipPresent, CLauncher::ButtonIndex(3, 6), 0));
	CHECK(IsChange(changes.fChanges[1], CPress::kSetup, 0, kLaunchMode));
}

void TestClearModeLeavesAnEmptyLaunchButtonEmpty() {
	const CPress::TChanges changes = CPress::ClearModeLaunchButton(kClearMode, 3, 6, false);
	CHECK(changes.fCount == 1);
	CHECK(IsChange(changes.fChanges[0], CPress::kSetup, 0, kLaunchMode));
}

void TestClearModeLaunchButtonDoesNothingElsewhere() {
	CHECK(CPress::ClearModeLaunchButton(kLaunchMode, 3, 6, true).fCount == 0);
	CHECK(CPress::ClearModeLaunchButton(kClipPicking, 3, 6, true).fCount == 0);
	CHECK(CPress::ClearModeLaunchButton(CLauncher::SetupForClipEditing(3, 6), 3, 6, true).fCount == 0);
	// And the ordinary rules do nothing in clear mode, not even a relaunch.
	CHECK(IsNoChange(CPress::LaunchButton(kClearMode, 3, 6, true, CPress::Edited(false))));
	CHECK(IsNoChange(CPress::RelaunchButton(kClearMode, 3, true)));
}

void TestSceneButtonInClearModeEmptiesItsSceneAndReturns() {
	const bool holdsClip[8] = { true, false, true, false, false, false, false, true };
	const CPress::TChanges changes = CPress::SceneButton(kClearMode, 5, holdsClip);
	CHECK(changes.fCount == 4);
	CHECK(IsChange(changes.fChanges[0], CPress::kClipPresent, CLauncher::ButtonIndex(0, 5), 0));
	CHECK(IsChange(changes.fChanges[1], CPress::kClipPresent, CLauncher::ButtonIndex(2, 5), 0));
	CHECK(IsChange(changes.fChanges[2], CPress::kClipPresent, CLauncher::ButtonIndex(7, 5), 0));
	CHECK(IsChange(changes.fChanges[3], CPress::kSetup, 0, kLaunchMode));

	// A scene without clips only returns to launch mode.
	const bool none[8] = { false, false, false, false, false, false, false, false };
	const CPress::TChanges nothing = CPress::SceneButton(kClearMode, 5, none);
	CHECK(nothing.fCount == 1);
	CHECK(IsChange(nothing.fChanges[0], CPress::kSetup, 0, kLaunchMode));
}

const int kSourcePicking = CLauncher::kSetupSourcePicking;

void TestDuplicateButtonSwitchesBetweenLaunchModeAndDuplicateMode() {
	CHECK(IsChange(CPress::DuplicateButton(kLaunchMode), CPress::kSetup, 0, kSourcePicking));
	// From either step it returns to launch mode, copying nothing.
	CHECK(IsChange(CPress::DuplicateButton(kSourcePicking), CPress::kSetup, 0, kLaunchMode));
	CHECK(IsChange(CPress::DuplicateButton(CLauncher::SetupForClipDestinationPicking(3, 6)), CPress::kSetup, 0, kLaunchMode));
	CHECK(IsChange(CPress::DuplicateButton(CLauncher::SetupForSceneDestinationPicking(6)), CPress::kSetup, 0, kLaunchMode));
	// In setup mode and clear mode it does nothing.
	CHECK(IsNoChange(CPress::DuplicateButton(kClipPicking)));
	CHECK(IsNoChange(CPress::DuplicateButton(CLauncher::SetupForClipEditing(3, 6))));
	CHECK(IsNoChange(CPress::DuplicateButton(CLauncher::SetupForFillInEditing(3, 6))));
	CHECK(IsNoChange(CPress::DuplicateButton(CLauncher::SetupForLandingEditing(3, 6))));
	CHECK(IsNoChange(CPress::DuplicateButton(kClearMode)));
	// Nor does the clear button in duplicate mode.
	CHECK(IsNoChange(CPress::ClearButton(kSourcePicking)));
	CHECK(IsNoChange(CPress::ClearButton(CLauncher::SetupForClipDestinationPicking(3, 6))));
}

void TestBackButtonLeavesDuplicateMode() {
	const int steps[] = { kSourcePicking, CLauncher::SetupForClipDestinationPicking(3, 6), CLauncher::SetupForSceneDestinationPicking(6) };
	for (int step : steps) {
		CHECK(IsNoChange(CPress::SetupButton(step)));
		CHECK(IsChange(CPress::BackButton(step), CPress::kSetup, 0, kLaunchMode));
		CHECK(IsNoChange(CPress::ForwardButton(step, true)));
		// And the ordinary rules do nothing, not even a relaunch.
		CHECK(IsNoChange(CPress::LaunchButton(step, 3, 6, true, CPress::Edited(false))));
		CHECK(IsNoChange(CPress::RelaunchButton(step, 3, true)));
		const bool holdsClip[8] = { true, true, true, true, true, true, true, true };
		CHECK(CPress::SceneButton(step, 6, holdsClip).fCount == 0);
	}
}

// Settings that differ from one another and from a new clip's, so that a copy
// can be told from them.
CPress::TSettings SomeSettings(int iSeed) {
	CPress::TSettings settings;
	settings.fPresent = iSeed % 2 == 0;
	settings.fColour = iSeed % CLauncher::kColourCount;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		settings.fPatterns[output] = (iSeed + output) % CLauncher::kPatternCount;
		settings.fUnipolar[output] = (iSeed + output + 1) % CLauncher::kValueCount;
		settings.fBipolar[output] = (iSeed + output + 2) % CLauncher::kValueCount;
		settings.fTransitions[output] = (iSeed + output) % CLauncher::kTransitionCount;
	}
	settings.fOneBarPattern = iSeed % 3 == 0;
	return settings;
}

CPress::TButton SomeButton(int iSeed) {
	CPress::TButton button;
	button.fClip = SomeSettings(iSeed);
	button.fClip.fOneBarPattern = false;
	button.fFillIn = SomeSettings(iSeed + 5);
	button.fLanding = SomeSettings(iSeed + 10);
	button.fIntro = SomeSettings(iSeed + 15);
	button.fOutro = SomeSettings(iSeed + 20);
	return button;
}

// Sets a CV output's settings from the number a change holds.
void Unpack(int iPacked, CPress::TSettings& ioSettings, int iOutput) {
	const CLauncher::TOutputSettings settings = CLauncher::UnpackOutputSettings(iPacked);
	ioSettings.fPatterns[iOutput] = settings.fPattern;
	ioSettings.fUnipolar[iOutput] = settings.fUnipolar;
	ioSettings.fBipolar[iOutput] = settings.fBipolar;
	ioSettings.fTransitions[iOutput] = settings.fTransition;
}

// Plays the changes to one launch button back onto a launch button whose
// settings are all -1, and counts them.
void Replay(const CPress::TChange iChanges[], int iCount, int iButtonIndex, CPress::TButton& oButton, int& oCount) {
	CPress::TSettings blank;
	blank.fPresent = false;
	blank.fColour = -1;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		blank.fPatterns[output] = -1;
		blank.fUnipolar[output] = -1;
		blank.fBipolar[output] = -1;
		blank.fTransitions[output] = -1;
	}
	blank.fOneBarPattern = false;
	oButton.fClip = blank;
	oButton.fFillIn = blank;
	oButton.fLanding = blank;
	oButton.fIntro = blank;
	oButton.fOutro = blank;
	oCount = 0;
	for (int i = 0; i < iCount; ++i) {
		const CPress::TChange& change = iChanges[i];
		if (change.fProperty == CPress::kSetup || change.fIndex != iButtonIndex) {
			continue;
		}
		++oCount;
		const int output = change.fOutput;
		switch (change.fProperty) {
			case CPress::kClipPresent: oButton.fClip.fPresent = change.fValue != 0; break;
			case CPress::kClipColour: oButton.fClip.fColour = change.fValue; break;
			case CPress::kClipOutput: Unpack(change.fValue, oButton.fClip, output); break;
			case CPress::kFillInPresent: oButton.fFillIn.fPresent = change.fValue != 0; break;
			case CPress::kFillInColour: oButton.fFillIn.fColour = change.fValue; break;
			case CPress::kFillInOutput: Unpack(change.fValue, oButton.fFillIn, output); break;
			case CPress::kFillInOneBarPattern: oButton.fFillIn.fOneBarPattern = change.fValue != 0; break;
			case CPress::kLandingPresent: oButton.fLanding.fPresent = change.fValue != 0; break;
			case CPress::kLandingColour: oButton.fLanding.fColour = change.fValue; break;
			case CPress::kLandingOutput: Unpack(change.fValue, oButton.fLanding, output); break;
			case CPress::kLandingOneBarPattern: oButton.fLanding.fOneBarPattern = change.fValue != 0; break;
			case CPress::kIntroPresent: oButton.fIntro.fPresent = change.fValue != 0; break;
			case CPress::kIntroColour: oButton.fIntro.fColour = change.fValue; break;
			case CPress::kIntroOutput: Unpack(change.fValue, oButton.fIntro, output); break;
			case CPress::kIntroOneBarPattern: oButton.fIntro.fOneBarPattern = change.fValue != 0; break;
			case CPress::kOutroPresent: oButton.fOutro.fPresent = change.fValue != 0; break;
			case CPress::kOutroColour: oButton.fOutro.fColour = change.fValue; break;
			case CPress::kOutroOutput: Unpack(change.fValue, oButton.fOutro, output); break;
			case CPress::kOutroOneBarPattern: oButton.fOutro.fOneBarPattern = change.fValue != 0; break;
			default: --oCount; break;
		}
	}
}

bool IsSameSettings(const CPress::TSettings& iA, const CPress::TSettings& iB) {
	if (iA.fPresent != iB.fPresent || iA.fColour != iB.fColour || iA.fOneBarPattern != iB.fOneBarPattern) {
		return false;
	}
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		if (iA.fPatterns[output] != iB.fPatterns[output] || iA.fUnipolar[output] != iB.fUnipolar[output]
			|| iA.fBipolar[output] != iB.fBipolar[output] || iA.fTransitions[output] != iB.fTransitions[output]) {
			return false;
		}
	}
	return true;
}

bool IsSameButton(const CPress::TButton& iA, const CPress::TButton& iB) {
	return IsSameSettings(iA.fClip, iB.fClip) && IsSameSettings(iA.fFillIn, iB.fFillIn) && IsSameSettings(iA.fLanding, iB.fLanding)
		&& IsSameSettings(iA.fIntro, iB.fIntro) && IsSameSettings(iA.fOutro, iB.fOutro);
}

void TestSourcePickingPicksALaunchButtonWithAClip() {
	const CPress::TChanges changes = CPress::DuplicateModeLaunchButton(kSourcePicking, 3, 6, true, SomeButton(1));
	CHECK(changes.fCount == 1);
	CHECK(IsChange(changes.fChanges[0], CPress::kSetup, 0, CLauncher::SetupForClipDestinationPicking(3, 6)));
	// An empty launch button cannot be the source.
	CHECK(CPress::DuplicateModeLaunchButton(kSourcePicking, 3, 6, false, SomeButton(1)).fCount == 0);
}

void TestClipDestinationPickingCopiesEverythingAndReturns() {
	const CPress::TButton source = SomeButton(2);
	const int setup = CLauncher::SetupForClipDestinationPicking(3, 6);
	// The destination may be on another track, empty or not.
	for (int held = 0; held < 2; ++held) {
		const CPress::TChanges changes = CPress::DuplicateModeLaunchButton(setup, 5, 2, held == 1, source);
		CHECK(changes.fCount == CPress::kButtonSettingCount + 1);
		CHECK(IsChange(changes.fChanges[changes.fCount - 1], CPress::kSetup, 0, kLaunchMode));
		CPress::TButton copy;
		int count = 0;
		Replay(changes.fChanges, changes.fCount, CLauncher::ButtonIndex(5, 2), copy, count);
		CHECK(count == CPress::kButtonSettingCount);
		CHECK(IsSameButton(copy, source));
	}
	// A source without a fill-in, landing, intro or outro switched on still
	// hands on their
	// settings, and an empty one makes the destination empty.
	CPress::TButton bare = SomeButton(3);
	bare.fFillIn.fPresent = false;
	bare.fLanding.fPresent = false;
	bare.fIntro.fPresent = false;
	bare.fOutro.fPresent = false;
	const CPress::TChanges changes = CPress::DuplicateModeLaunchButton(setup, 0, 1, true, bare);
	CPress::TButton copy;
	int count = 0;
	Replay(changes.fChanges, changes.fCount, CLauncher::ButtonIndex(0, 1), copy, count);
	CHECK(IsSameButton(copy, bare));
}

void TestClipDestinationPickingIgnoresTheSource() {
	const int setup = CLauncher::SetupForClipDestinationPicking(3, 6);
	CHECK(CPress::DuplicateModeLaunchButton(setup, 3, 6, true, SomeButton(2)).fCount == 0);
}

void TestDuplicateModeLaunchButtonDoesNothingElsewhere() {
	const CPress::TButton source = SomeButton(2);
	CHECK(CPress::DuplicateModeLaunchButton(kLaunchMode, 3, 6, true, source).fCount == 0);
	CHECK(CPress::DuplicateModeLaunchButton(kClipPicking, 3, 6, true, source).fCount == 0);
	CHECK(CPress::DuplicateModeLaunchButton(kClearMode, 3, 6, true, source).fCount == 0);
	// With a scene as the source, launch buttons do nothing.
	CHECK(CPress::DuplicateModeLaunchButton(CLauncher::SetupForSceneDestinationPicking(6), 3, 2, true, source).fCount == 0);
}

void TestSourcePickingPicksASceneWithAClip() {
	CHECK(IsChange(CPress::DuplicateModeSceneButton(kSourcePicking, 6, true), CPress::kSetup, 0, CLauncher::SetupForSceneDestinationPicking(6)));
	// A scene without clips cannot be the source.
	CHECK(IsNoChange(CPress::DuplicateModeSceneButton(kSourcePicking, 6, false)));
}

void TestSceneDestinationPickingReturnsToLaunchMode() {
	// With clips or without, any scene but the source is a destination.
	CHECK(IsChange(CPress::DuplicateModeSceneButton(CLauncher::SetupForSceneDestinationPicking(6), 2, false), CPress::kSetup, 0, kLaunchMode));
	CHECK(IsChange(CPress::DuplicateModeSceneButton(CLauncher::SetupForSceneDestinationPicking(6), 2, true), CPress::kSetup, 0, kLaunchMode));
}

void TestCopyButtonCopiesEverything() {
	// Even seeds hold a clip, odd ones are empty: a copy of an empty launch
	// button empties the destination, as a scene copy needs.
	for (int seed = 0; seed < 2; ++seed) {
		const CPress::TButton source = SomeButton(seed);
		CHECK(source.fClip.fPresent == (seed == 0));
		const CPress::TChanges changes = CPress::CopyButton(source, CLauncher::ButtonIndex(4, 2));
		CHECK(changes.fCount == CPress::kButtonSettingCount);
		CPress::TButton copy;
		int count = 0;
		Replay(changes.fChanges, changes.fCount, CLauncher::ButtonIndex(4, 2), copy, count);
		CHECK(count == CPress::kButtonSettingCount);
		CHECK(IsSameButton(copy, source));
	}
}

void TestDuplicateModeSceneButtonDoesNothingElsewhere() {
	// The source itself.
	CHECK(IsNoChange(CPress::DuplicateModeSceneButton(CLauncher::SetupForSceneDestinationPicking(6), 6, true)));
	// With a launch button as the source, scene buttons do nothing.
	CHECK(IsNoChange(CPress::DuplicateModeSceneButton(CLauncher::SetupForClipDestinationPicking(3, 6), 2, true)));
	const int steps[] = { kLaunchMode, kClipPicking, kClearMode };
	for (int step : steps) {
		CHECK(IsNoChange(CPress::DuplicateModeSceneButton(step, 2, true)));
	}
}
} // namespace

bool IsChanges(const CPress::TChanges& iChanges, int iCount) {
	return iChanges.fCount == iCount;
}

void TestSetupButtonLeavesFillInEditing() {
	CHECK(IsChange(CPress::SetupButton(CLauncher::SetupForFillInEditing(3, 3)), CPress::kSetup, 0, kLaunchMode));
}

void TestForwardButtonGoesForwardOneStepWithAClip() {
	CHECK(IsChange(CPress::ForwardButton(CLauncher::SetupForClipEditing(3, 6), true), CPress::kSetup, 0, CLauncher::SetupForFillInEditing(3, 6)));
	CHECK(IsNoChange(CPress::ForwardButton(CLauncher::SetupForClipEditing(3, 6), false)));
	CHECK(IsNoChange(CPress::ForwardButton(kLaunchMode, true)));
	CHECK(IsNoChange(CPress::ForwardButton(kClipPicking, true)));
	// From fill-in editing on to landing editing, intro editing and outro
	// editing, whether or not there is a fill-in, landing or intro, but not
	// beyond.
	CHECK(IsChange(CPress::ForwardButton(CLauncher::SetupForFillInEditing(3, 6), true), CPress::kSetup, 0, CLauncher::SetupForLandingEditing(3, 6)));
	CHECK(IsNoChange(CPress::ForwardButton(CLauncher::SetupForFillInEditing(3, 6), false)));
	CHECK(IsChange(CPress::ForwardButton(CLauncher::SetupForLandingEditing(3, 6), true), CPress::kSetup, 0, CLauncher::SetupForIntroEditing(3, 6)));
	CHECK(IsChange(CPress::ForwardButton(CLauncher::SetupForIntroEditing(3, 6), true), CPress::kSetup, 0, CLauncher::SetupForOutroEditing(3, 6)));
	CHECK(IsNoChange(CPress::ForwardButton(CLauncher::SetupForIntroEditing(3, 6), false)));
	CHECK(IsNoChange(CPress::ForwardButton(CLauncher::SetupForOutroEditing(3, 6), true)));
}

void TestFillInButtonFollowsTheButton() {
	CHECK(IsChange(CPress::FillInButton(true), CPress::kFillIn, 0, 1));
	CHECK(IsChange(CPress::FillInButton(false), CPress::kFillIn, 0, 0));
}

void TestLaunchButtonDoesNothingInFillInEditing() {
	const int editing = CLauncher::SetupForFillInEditing(3, 6);
	CHECK(IsNoChange(CPress::LaunchButton(editing, 7, 1, false, CPress::Edited(true))));
	CHECK(IsNoChange(CPress::LaunchButton(editing, 0, 2, false, CPress::Edited(true))));
}

CPress::TClipValues SomeClipValues() {
	CPress::TClipValues values;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		values.fPatterns[output] = output;
		values.fUnipolar[output] = 6 - output;
		values.fBipolar[output] = output % 7;
	}
	return values;
}

void TestFillInEditingDoesNothingElsewhere() {
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(kLaunchMode, 7, 1, CPress::Edited(false), SomeClipValues()), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(kClipPicking, 7, 1, CPress::Edited(false), SomeClipValues()), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(CLauncher::SetupForClipEditing(3, 6), 7, 1, CPress::Edited(false), SomeClipValues()), 0));
}

void TestFillInEditingToggleSetsUpANewFillIn() {
	const int editing = CLauncher::SetupForFillInEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);

	// A new fill-in is a copy of its clip's values, with every transition
	// off, in white.
	const CPress::TChanges set = CPress::FillInEditingLaunchButton(editing, 7, 1, MixedModes(false), SomeClipValues());
	CHECK(IsChanges(set, 3 + CLauncher::kOutputCount));
	CHECK(IsChange(set.fChanges[2 + CLauncher::kOutputCount], CPress::kFillInOneBarPattern, picked, 0));
	CHECK(IsChange(set.fChanges[0], CPress::kFillInPresent, picked, 1));
	CHECK(IsChange(set.fChanges[1], CPress::kFillInColour, picked, CLauncher::kColourWhite));
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		const CPress::TChange* changes = set.fChanges + 2 + output;
		CHECK(IsSettingChange(changes[0], CPress::kFillInOutput, kPatternSetting, picked, output, output));
		CHECK(IsSettingChange(changes[0], CPress::kFillInOutput, kUnipolarSetting, picked, output, 6 - output));
		CHECK(IsSettingChange(changes[0], CPress::kFillInOutput, kBipolarSetting, picked, output, output % 7));
		CHECK(IsSettingChange(changes[0], CPress::kFillInOutput, kTransitionSetting, picked, output, CLauncher::kTransitionOff));
	}

	const CPress::TChanges clear = CPress::FillInEditingLaunchButton(editing, 7, 1, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(clear, 1));
	CHECK(IsChange(clear.fChanges[0], CPress::kFillInPresent, picked, 0));
}

void TestFillInEditingColourAndCVRows() {
	const int editing = CLauncher::SetupForFillInEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	for (int column = 0; column < 7; ++column) {
		const CPress::TChanges first = CPress::FillInEditingLaunchButton(editing, column, 1, MixedModes(true), SomeClipValues());
		CHECK(IsChanges(first, 1) && IsChange(first.fChanges[0], CPress::kFillInColour, picked, column));
		const CPress::TChanges second = CPress::FillInEditingLaunchButton(editing, column, 2, MixedModes(true), SomeClipValues());
		CHECK(IsChanges(second, 1) && IsChange(second.fChanges[0], CPress::kFillInColour, picked, 7 + column));
		const CPress::TChanges unipolar = CPress::FillInEditingLaunchButton(editing, column, 4, MixedModes(true), SomeClipValues());
		CHECK(IsChanges(unipolar, 1) && IsSettingChange(unipolar.fChanges[0], CPress::kFillInOutput, kUnipolarSetting, picked, 1, column));
		const CPress::TChanges bipolar = CPress::FillInEditingLaunchButton(editing, column, 5, MixedModes(true), SomeClipValues());
		CHECK(IsChanges(bipolar, 1) && IsSettingChange(bipolar.fChanges[0], CPress::kFillInOutput, kBipolarSetting, picked, 2, column));
	}
	for (int column = 0; column < 8; ++column) {
		const CPress::TChanges pattern = CPress::FillInEditingLaunchButton(editing, column, 3, MixedModes(true), SomeClipValues());
		CHECK(IsChanges(pattern, 1) && IsSettingChange(pattern.fChanges[0], CPress::kFillInOutput, kPatternSetting, picked, 0, column));
	}
	const CPress::TChanges transition = CPress::FillInEditingLaunchButton(editing, 7, 5, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(transition, 1) && IsSettingChange(transition.fChanges[0], CPress::kFillInOutput, kTransitionSetting, picked, 2, CLauncher::kTransitionC));
	// The last track of the second colour row is the one-bar pattern switch.
	const CPress::TChanges oneBar = CPress::FillInEditingLaunchButton(editing, 7, 2, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(oneBar, 1) && IsChange(oneBar.fChanges[0], CPress::kFillInOneBarPattern, picked, 1));
}

void TestFillInEditingWithoutFillInHasNoColourOrCVControls() {
	const int editing = CLauncher::SetupForFillInEditing(3, 6);
	for (int row = 1; row <= 8; ++row) {
		CHECK(IsChanges(CPress::FillInEditingLaunchButton(editing, 2, row, MixedModes(false), SomeClipValues()), 0));
	}
}

void TestLandingEditingDoesNothingElsewhere() {
	CHECK(IsChanges(CPress::LandingEditingLaunchButton(kClipPicking, 7, 1, CPress::Edited(false), SomeClipValues()), 0));
	CHECK(IsChanges(CPress::LandingEditingLaunchButton(CLauncher::SetupForFillInEditing(3, 6), 7, 1, CPress::Edited(false), SomeClipValues()), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(CLauncher::SetupForLandingEditing(3, 6), 7, 1, CPress::Edited(false), SomeClipValues()), 0));
	CHECK(IsNoChange(CPress::LaunchButton(CLauncher::SetupForLandingEditing(3, 6), 7, 1, false, CPress::Edited(true))));
}

void TestLandingEditingToggleSetsUpANewLanding() {
	const int editing = CLauncher::SetupForLandingEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);

	// A new landing is a copy of its clip's values, with every transition
	// off, in white, like a new fill-in.
	const CPress::TChanges set = CPress::LandingEditingLaunchButton(editing, 7, 1, MixedModes(false), SomeClipValues());
	CHECK(IsChanges(set, 3 + CLauncher::kOutputCount));
	CHECK(IsChange(set.fChanges[2 + CLauncher::kOutputCount], CPress::kLandingOneBarPattern, picked, 0));
	CHECK(IsChange(set.fChanges[0], CPress::kLandingPresent, picked, 1));
	CHECK(IsChange(set.fChanges[1], CPress::kLandingColour, picked, CLauncher::kColourWhite));
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		const CPress::TChange* changes = set.fChanges + 2 + output;
		CHECK(IsSettingChange(changes[0], CPress::kLandingOutput, kPatternSetting, picked, output, output));
		CHECK(IsSettingChange(changes[0], CPress::kLandingOutput, kUnipolarSetting, picked, output, 6 - output));
		CHECK(IsSettingChange(changes[0], CPress::kLandingOutput, kBipolarSetting, picked, output, output % 7));
		CHECK(IsSettingChange(changes[0], CPress::kLandingOutput, kTransitionSetting, picked, output, CLauncher::kTransitionOff));
	}

	const CPress::TChanges clear = CPress::LandingEditingLaunchButton(editing, 7, 1, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(clear, 1));
	CHECK(IsChange(clear.fChanges[0], CPress::kLandingPresent, picked, 0));
}

void TestLandingEditingColourAndCVRows() {
	const int editing = CLauncher::SetupForLandingEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	const CPress::TChanges colour = CPress::LandingEditingLaunchButton(editing, 2, 2, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(colour, 1) && IsChange(colour.fChanges[0], CPress::kLandingColour, picked, 9));
	const CPress::TChanges pattern = CPress::LandingEditingLaunchButton(editing, 5, 3, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(pattern, 1) && IsSettingChange(pattern.fChanges[0], CPress::kLandingOutput, kPatternSetting, picked, 0, 5));
	const CPress::TChanges unipolar = CPress::LandingEditingLaunchButton(editing, 4, 4, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(unipolar, 1) && IsSettingChange(unipolar.fChanges[0], CPress::kLandingOutput, kUnipolarSetting, picked, 1, 4));
	const CPress::TChanges bipolar = CPress::LandingEditingLaunchButton(editing, 0, 5, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(bipolar, 1) && IsSettingChange(bipolar.fChanges[0], CPress::kLandingOutput, kBipolarSetting, picked, 2, 0));
	const CPress::TChanges transition = CPress::LandingEditingLaunchButton(editing, 7, 6, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(transition, 1) && IsSettingChange(transition.fChanges[0], CPress::kLandingOutput, kTransitionSetting, picked, 3, CLauncher::kTransitionOff));
	// Without a landing there is nothing but the toggle.
	CHECK(IsChanges(CPress::LandingEditingLaunchButton(editing, 2, 2, MixedModes(false), SomeClipValues()), 0));
}

void TestOneBarPatternSwitchGoesOnAndOff() {
	const int editing = CLauncher::SetupForLandingEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	CPress::TEdited landing = MixedModes(true);
	const CPress::TChanges on = CPress::LandingEditingLaunchButton(editing, 7, 2, landing, SomeClipValues());
	CHECK(IsChanges(on, 1) && IsChange(on.fChanges[0], CPress::kLandingOneBarPattern, picked, 1));
	landing.fOneBarPattern = true;
	const CPress::TChanges off = CPress::LandingEditingLaunchButton(editing, 7, 2, landing, SomeClipValues());
	CHECK(IsChanges(off, 1) && IsChange(off.fChanges[0], CPress::kLandingOneBarPattern, picked, 0));
	// Not there without a landing.
	CHECK(IsChanges(CPress::LandingEditingLaunchButton(editing, 7, 2, MixedModes(false), SomeClipValues()), 0));
	// In fill-in editing it is the fill-in's.
	const int fillInEditing = CLauncher::SetupForFillInEditing(3, 6);
	const CPress::TChanges fillIn = CPress::FillInEditingLaunchButton(fillInEditing, 7, 2, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(fillIn, 1) && IsChange(fillIn.fChanges[0], CPress::kFillInOneBarPattern, picked, 1));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(fillInEditing, 7, 2, MixedModes(false), SomeClipValues()), 0));
}

void TestSceneButtonDoesNothingInFillInEditing() {
	const bool holdsClip[CLauncher::kTrackCount] = { true, true, true, true, true, true, true, true };
	CHECK(CPress::SceneButton(CLauncher::SetupForFillInEditing(0, 1), 3, holdsClip).fCount == 0);
	CHECK(CPress::SceneButton(CLauncher::SetupForLandingEditing(0, 1), 3, holdsClip).fCount == 0);
}

void TestFunctionButtonInStopFunctionSelectsStop() {
	CHECK(IsChange(CPress::FunctionButton(CLauncher::kFunctionStop, 4, true, true), CPress::kSelection, 4, CLauncher::kSelectionStop));
	CHECK(IsNoChange(CPress::FunctionButton(CLauncher::kFunctionStop, 4, true, false)));
}

void TestFunctionButtonIsHeldDownAsMuteOrSoloButton() {
	CHECK(IsChange(CPress::FunctionButton(CLauncher::kFunctionMute, 2, true, true), CPress::kMute, 2, 1));
	CHECK(IsChange(CPress::FunctionButton(CLauncher::kFunctionMute, 2, true, false), CPress::kMute, 2, 0));
	CHECK(IsChange(CPress::FunctionButton(CLauncher::kFunctionSolo, 7, true, true), CPress::kSolo, 7, 1));
	CHECK(IsChange(CPress::FunctionButton(CLauncher::kFunctionSolo, 7, true, false), CPress::kSolo, 7, 0));
}

void TestFunctionButtonOfATrackWithoutClipsDoesNothing() {
	CHECK(IsNoChange(CPress::FunctionButton(CLauncher::kFunctionStop, 4, false, true)));
	CHECK(IsNoChange(CPress::FunctionButton(CLauncher::kFunctionMute, 4, false, true)));
	CHECK(IsNoChange(CPress::FunctionButton(CLauncher::kFunctionSolo, 4, false, true)));
	// A button that is held down can still be let go of.
	CHECK(IsChange(CPress::FunctionButton(CLauncher::kFunctionMute, 4, false, false), CPress::kMute, 4, 0));
	CHECK(IsNoChange(CPress::FunctionButton(CLauncher::kFunctionMute, 8, true, true)));
}

void TestFunctionSelectButtonChoosesItsFunction() {
	CHECK(IsChange(CPress::FunctionSelectButton(CLauncher::kFunctionMute), CPress::kFunction, 0, CLauncher::kFunctionMute));
	CHECK(IsChange(CPress::FunctionSelectButton(CLauncher::kFunctionSolo), CPress::kFunction, 0, CLauncher::kFunctionSolo));
	CHECK(IsChange(CPress::FunctionSelectButton(CLauncher::kFunctionStop), CPress::kFunction, 0, CLauncher::kFunctionStop));
	CHECK(IsNoChange(CPress::FunctionSelectButton(3)));
}

void TestIntroAndOutroEditing() {
	const int picked = CLauncher::ButtonIndex(3, 6);
	const int intro = CLauncher::SetupForIntroEditing(3, 6);
	const int outro = CLauncher::SetupForOutroEditing(3, 6);
	// Nothing changes outside its own step.
	CHECK(IsChanges(CPress::IntroEditingLaunchButton(outro, 7, 1, CPress::Edited(false), SomeClipValues()), 0));
	CHECK(IsChanges(CPress::OutroEditingLaunchButton(intro, 7, 1, CPress::Edited(false), SomeClipValues()), 0));
	CHECK(IsChanges(CPress::LandingEditingLaunchButton(intro, 7, 1, CPress::Edited(false), SomeClipValues()), 0));
	CHECK(IsNoChange(CPress::LaunchButton(intro, 7, 1, false, CPress::Edited(true))));
	CHECK(IsNoChange(CPress::LaunchButton(outro, 7, 1, false, CPress::Edited(true))));

	// A new intro or outro is a copy of its clip's values, with every
	// transition off and one-bar pattern off, in white.
	const CPress::TChanges setIntro = CPress::IntroEditingLaunchButton(intro, 7, 1, MixedModes(false), SomeClipValues());
	CHECK(IsChanges(setIntro, 3 + CLauncher::kOutputCount));
	CHECK(IsChange(setIntro.fChanges[0], CPress::kIntroPresent, picked, 1));
	CHECK(IsChange(setIntro.fChanges[1], CPress::kIntroColour, picked, CLauncher::kColourWhite));
	CHECK(IsSettingChange(setIntro.fChanges[2], CPress::kIntroOutput, kPatternSetting, picked, 0, 0));
	CHECK(IsSettingChange(setIntro.fChanges[2], CPress::kIntroOutput, kTransitionSetting, picked, 0, CLauncher::kTransitionOff));
	CHECK(IsChange(setIntro.fChanges[2 + CLauncher::kOutputCount], CPress::kIntroOneBarPattern, picked, 0));
	const CPress::TChanges setOutro = CPress::OutroEditingLaunchButton(outro, 7, 1, MixedModes(false), SomeClipValues());
	CHECK(IsChanges(setOutro, 3 + CLauncher::kOutputCount));
	CHECK(IsChange(setOutro.fChanges[0], CPress::kOutroPresent, picked, 1));
	CHECK(IsChange(setOutro.fChanges[2 + CLauncher::kOutputCount], CPress::kOutroOneBarPattern, picked, 0));
	const CPress::TChanges clearOutro = CPress::OutroEditingLaunchButton(outro, 7, 1, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(clearOutro, 1) && IsChange(clearOutro.fChanges[0], CPress::kOutroPresent, picked, 0));

	// Colours, CV rows and the one-bar pattern switch.
	const CPress::TChanges colour = CPress::IntroEditingLaunchButton(intro, 2, 2, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(colour, 1) && IsChange(colour.fChanges[0], CPress::kIntroColour, picked, 9));
	const CPress::TChanges unipolar = CPress::OutroEditingLaunchButton(outro, 4, 4, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(unipolar, 1) && IsSettingChange(unipolar.fChanges[0], CPress::kOutroOutput, kUnipolarSetting, picked, 1, 4));
	const CPress::TChanges transition = CPress::IntroEditingLaunchButton(intro, 7, 6, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(transition, 1) && IsSettingChange(transition.fChanges[0], CPress::kIntroOutput, kTransitionSetting, picked, 3, CLauncher::kTransitionOff));
	const CPress::TChanges oneBar = CPress::OutroEditingLaunchButton(outro, 7, 2, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(oneBar, 1) && IsChange(oneBar.fChanges[0], CPress::kOutroOneBarPattern, picked, 1));
	CHECK(IsChanges(CPress::IntroEditingLaunchButton(intro, 2, 2, MixedModes(false), SomeClipValues()), 0));
}

// A press changes one setting of a CV output and hands on the other three,
// since all four are one property.
void TestEditingKeepsTheOtherSettingsOfTheOutput() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	CPress::TEdited edited = MixedModes(true);
	edited.fPatterns[1] = 5;
	edited.fUnipolar[1] = 2;
	edited.fBipolar[1] = 6;
	edited.fTransitions[1] = CLauncher::kTransitionB;
	// Output B is unipolar: column 4 sets the unipolar value 3.
	const CPress::TChange unipolar = CPress::LaunchButton(editing, 3, 4, false, edited);
	CHECK(IsSettingChange(unipolar, CPress::kClipOutput, kUnipolarSetting, picked, 1, 3));
	CHECK(SettingOf(unipolar.fValue, kPatternSetting) == 5);
	CHECK(SettingOf(unipolar.fValue, kBipolarSetting) == 6);
	CHECK(SettingOf(unipolar.fValue, kTransitionSetting) == CLauncher::kTransitionB);
	// The transition button moves the transition on and keeps the values.
	const CPress::TChange transition = CPress::LaunchButton(editing, 7, 4, false, edited);
	CHECK(IsSettingChange(transition, CPress::kClipOutput, kTransitionSetting, picked, 1, CLauncher::kTransitionC));
	CHECK(SettingOf(transition.fValue, kUnipolarSetting) == 2);
	CHECK(SettingOf(transition.fValue, kPatternSetting) == 5);
}

int main() {
	TestLaunchModeSelectsTheClip();
	TestLaunchModeEmptyLaunchButtonSelectsStop();
	TestStopButtonSelectsStopInEveryMode();
	TestSetupButtonSwitchesBetweenLaunchModeAndClipPicking();
	TestClipPickingOpensThePressedLaunchButton();
	TestClipEditingToggleSwitchesThePickedClip();
	TestBackButtonGoesBackOneStep();
	TestClipEditingColourRows();
	TestClipEditingPatternMode();
	TestClipEditingUnipolarAndBipolarValues();
	TestClipEditingTransitionButtonGoesRound();
	TestClipEditingUnusedLaunchButtonsDoNothing();
	TestClipEditingWithoutClipHasNoColourOrCVControls();
	TestLaunchHoldsTheRelaunchPressInLaunchModeOnly();
	TestSceneButtonSelectsItsRowOnEveryTrack();
	TestSceneButtonStopsTracksWithAnEmptyLaunchButton();
	TestSceneButtonOfAnEmptySceneStopsEveryTrack();
	TestSceneButtonDoesNothingInClipEditing();
	TestSceneButtonInClipPickingChoosesItsLengthAndLeavesSetupMode();
	TestClearButtonSwitchesBetweenLaunchModeAndClearMode();
	TestBackButtonLeavesClearMode();
	TestClearModeEmptiesThePressedLaunchButtonAndReturns();
	TestClearModeLeavesAnEmptyLaunchButtonEmpty();
	TestClearModeLaunchButtonDoesNothingElsewhere();
	TestSceneButtonInClearModeEmptiesItsSceneAndReturns();
	TestDuplicateButtonSwitchesBetweenLaunchModeAndDuplicateMode();
	TestBackButtonLeavesDuplicateMode();
	TestSourcePickingPicksALaunchButtonWithAClip();
	TestClipDestinationPickingCopiesEverythingAndReturns();
	TestClipDestinationPickingIgnoresTheSource();
	TestDuplicateModeLaunchButtonDoesNothingElsewhere();
	TestSourcePickingPicksASceneWithAClip();
	TestSceneDestinationPickingReturnsToLaunchMode();
	TestCopyButtonCopiesEverything();
	TestDuplicateModeSceneButtonDoesNothingElsewhere();

	TestSetupButtonLeavesFillInEditing();
	TestForwardButtonGoesForwardOneStepWithAClip();
	TestFillInButtonFollowsTheButton();
	TestLaunchButtonDoesNothingInFillInEditing();
	TestFillInEditingDoesNothingElsewhere();
	TestFillInEditingToggleSetsUpANewFillIn();
	TestFillInEditingColourAndCVRows();
	TestFillInEditingWithoutFillInHasNoColourOrCVControls();
	TestSceneButtonDoesNothingInFillInEditing();
	TestLandingEditingDoesNothingElsewhere();
	TestLandingEditingToggleSetsUpANewLanding();
	TestLandingEditingColourAndCVRows();
	TestOneBarPatternSwitchGoesOnAndOff();

	TestFunctionButtonInStopFunctionSelectsStop();
	TestFunctionButtonIsHeldDownAsMuteOrSoloButton();
	TestFunctionButtonOfATrackWithoutClipsDoesNothing();
	TestFunctionSelectButtonChoosesItsFunction();
	TestIntroAndOutroEditing();
	TestEditingKeepsTheOtherSettingsOfTheOutput();

	return ReportChecks("Press");
}
