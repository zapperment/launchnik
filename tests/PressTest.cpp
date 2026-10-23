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
}

bool IsOutputChange(const CPress::TChange& iChange, CPress::EProperty iProperty, int iIndex, int iOutput, int iValue) {
	return IsChange(iChange, iProperty, iIndex, iValue) && iChange.fOutput == iOutput;
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
		CHECK(IsOutputChange(CPress::LaunchButton(editing, column, 3, false, MixedModes(true)), CPress::kClipPattern, picked, 0, column));
	}
}

void TestClipEditingUnipolarAndBipolarValues() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	for (int column = 0; column < 7; ++column) {
		CHECK(IsOutputChange(CPress::LaunchButton(editing, column, 4, false, MixedModes(true)), CPress::kClipUnipolar, picked, 1, column));
		CHECK(IsOutputChange(CPress::LaunchButton(editing, column, 5, false, MixedModes(true)), CPress::kClipBipolar, picked, 2, column));
		CHECK(IsOutputChange(CPress::LaunchButton(editing, column, 8, false, MixedModes(true)), CPress::kClipUnipolar, picked, 5, column));
	}
}

void TestClipEditingTransitionButtonGoesRound() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	CHECK(IsOutputChange(CPress::LaunchButton(editing, 7, 4, false, MixedModes(true)), CPress::kClipTransition, picked, 1, CLauncher::kTransitionA));
	CHECK(IsOutputChange(CPress::LaunchButton(editing, 7, 5, false, MixedModes(true)), CPress::kClipTransition, picked, 2, CLauncher::kTransitionC));
	// After D it is off again.
	CHECK(IsOutputChange(CPress::LaunchButton(editing, 7, 6, false, MixedModes(true)), CPress::kClipTransition, picked, 3, CLauncher::kTransitionOff));
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

void TestSceneButtonSelectsItsRowOnEveryTrack() {
	const bool holdsClip[8] = { true, true, true, true, true, true, true, true };
	const CPress::TChanges changes = CPress::SceneButton(kLaunchMode, 3, holdsClip);
	CHECK(changes.fCount == 8);
	for (int track = 0; track < 8; ++track) {
		CHECK(IsChange(changes.fChanges[track], CPress::kSelection, track, 3));
	}
}

void TestSceneButtonStopsTracksWithAnEmptyLaunchButton() {
	const bool holdsClip[8] = { true, false, false, true, false, false, false, true };
	const CPress::TChanges changes = CPress::SceneButton(kLaunchMode, 8, holdsClip);
	CHECK(changes.fCount == 8);
	for (int track = 0; track < 8; ++track) {
		CHECK(IsChange(changes.fChanges[track], CPress::kSelection, track, holdsClip[track] ? 8 : CLauncher::kSelectionStop));
	}
}

void TestSceneButtonOfAnEmptySceneStopsEveryTrack() {
	const bool holdsClip[8] = { false, false, false, false, false, false, false, false };
	const CPress::TChanges changes = CPress::SceneButton(kLaunchMode, 1, holdsClip);
	CHECK(changes.fCount == 8);
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
	// From fill-in editing on to landing editing, whether or not there is a
	// fill-in, but not beyond.
	CHECK(IsChange(CPress::ForwardButton(CLauncher::SetupForFillInEditing(3, 6), true), CPress::kSetup, 0, CLauncher::SetupForLandingEditing(3, 6)));
	CHECK(IsNoChange(CPress::ForwardButton(CLauncher::SetupForFillInEditing(3, 6), false)));
	CHECK(IsNoChange(CPress::ForwardButton(CLauncher::SetupForLandingEditing(3, 6), true)));
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
	CHECK(IsChanges(set, 2 + 4 * CLauncher::kOutputCount));
	CHECK(IsChange(set.fChanges[0], CPress::kFillInPresent, picked, 1));
	CHECK(IsChange(set.fChanges[1], CPress::kFillInColour, picked, CLauncher::kColourWhite));
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		const CPress::TChange* changes = set.fChanges + 2 + 4 * output;
		CHECK(IsOutputChange(changes[0], CPress::kFillInPattern, picked, output, output));
		CHECK(IsOutputChange(changes[1], CPress::kFillInUnipolar, picked, output, 6 - output));
		CHECK(IsOutputChange(changes[2], CPress::kFillInBipolar, picked, output, output % 7));
		CHECK(IsOutputChange(changes[3], CPress::kFillInTransition, picked, output, CLauncher::kTransitionOff));
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
		CHECK(IsChanges(unipolar, 1) && IsOutputChange(unipolar.fChanges[0], CPress::kFillInUnipolar, picked, 1, column));
		const CPress::TChanges bipolar = CPress::FillInEditingLaunchButton(editing, column, 5, MixedModes(true), SomeClipValues());
		CHECK(IsChanges(bipolar, 1) && IsOutputChange(bipolar.fChanges[0], CPress::kFillInBipolar, picked, 2, column));
	}
	for (int column = 0; column < 8; ++column) {
		const CPress::TChanges pattern = CPress::FillInEditingLaunchButton(editing, column, 3, MixedModes(true), SomeClipValues());
		CHECK(IsChanges(pattern, 1) && IsOutputChange(pattern.fChanges[0], CPress::kFillInPattern, picked, 0, column));
	}
	const CPress::TChanges transition = CPress::FillInEditingLaunchButton(editing, 7, 5, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(transition, 1) && IsOutputChange(transition.fChanges[0], CPress::kFillInTransition, picked, 2, CLauncher::kTransitionC));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(editing, 7, 2, MixedModes(true), SomeClipValues()), 0));
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
	CHECK(IsChanges(set, 3 + 4 * CLauncher::kOutputCount));
	CHECK(IsChange(set.fChanges[2 + 4 * CLauncher::kOutputCount], CPress::kLandingOneBarPattern, picked, 0));
	CHECK(IsChange(set.fChanges[0], CPress::kLandingPresent, picked, 1));
	CHECK(IsChange(set.fChanges[1], CPress::kLandingColour, picked, CLauncher::kColourWhite));
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		const CPress::TChange* changes = set.fChanges + 2 + 4 * output;
		CHECK(IsOutputChange(changes[0], CPress::kLandingPattern, picked, output, output));
		CHECK(IsOutputChange(changes[1], CPress::kLandingUnipolar, picked, output, 6 - output));
		CHECK(IsOutputChange(changes[2], CPress::kLandingBipolar, picked, output, output % 7));
		CHECK(IsOutputChange(changes[3], CPress::kLandingTransition, picked, output, CLauncher::kTransitionOff));
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
	CHECK(IsChanges(pattern, 1) && IsOutputChange(pattern.fChanges[0], CPress::kLandingPattern, picked, 0, 5));
	const CPress::TChanges unipolar = CPress::LandingEditingLaunchButton(editing, 4, 4, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(unipolar, 1) && IsOutputChange(unipolar.fChanges[0], CPress::kLandingUnipolar, picked, 1, 4));
	const CPress::TChanges bipolar = CPress::LandingEditingLaunchButton(editing, 0, 5, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(bipolar, 1) && IsOutputChange(bipolar.fChanges[0], CPress::kLandingBipolar, picked, 2, 0));
	const CPress::TChanges transition = CPress::LandingEditingLaunchButton(editing, 7, 6, MixedModes(true), SomeClipValues());
	CHECK(IsChanges(transition, 1) && IsOutputChange(transition.fChanges[0], CPress::kLandingTransition, picked, 3, CLauncher::kTransitionOff));
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
	// Not there without a landing, nor in fill-in editing.
	CHECK(IsChanges(CPress::LandingEditingLaunchButton(editing, 7, 2, MixedModes(false), SomeClipValues()), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(CLauncher::SetupForFillInEditing(3, 6), 7, 2, MixedModes(true), SomeClipValues()), 0));
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
	TestSceneButtonSelectsItsRowOnEveryTrack();
	TestSceneButtonStopsTracksWithAnEmptyLaunchButton();
	TestSceneButtonOfAnEmptySceneStopsEveryTrack();
	TestSceneButtonDoesNothingInClipEditing();
	TestSceneButtonInClipPickingChoosesItsLengthAndLeavesSetupMode();

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

	return ReportChecks("Press");
}
