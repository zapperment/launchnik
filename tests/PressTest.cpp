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
	CHECK(IsChange(CPress::LaunchButton(kLaunchMode, 2, 5, true, false), CPress::kSelection, 2, 5));
	CHECK(IsChange(CPress::LaunchButton(kLaunchMode, 7, 8, true, false), CPress::kSelection, 7, 8));
}

void TestLaunchModeEmptyLaunchButtonSelectsStop() {
	CHECK(IsChange(CPress::LaunchButton(kLaunchMode, 2, 5, false, false), CPress::kSelection, 2, CLauncher::kSelectionStop));
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
	CHECK(IsChange(CPress::LaunchButton(kClipPicking, 0, 1, false, false), CPress::kSetup, 0, 2));
	CHECK(IsChange(CPress::LaunchButton(kClipPicking, 7, 8, true, false), CPress::kSetup, 0, 65));
	CHECK(IsChange(CPress::LaunchButton(kClipPicking, 3, 6, true, false), CPress::kSetup, 0, CLauncher::SetupForClipEditing(3, 6)));
}

void TestClipEditingToggleSwitchesThePickedClip() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	CHECK(IsChange(CPress::LaunchButton(editing, 7, 1, false, false), CPress::kClipPresent, picked, 1));
	CHECK(IsChange(CPress::LaunchButton(editing, 7, 1, false, true), CPress::kClipPresent, picked, 0));
}

void TestBackButtonGoesBackOneStep() {
	CHECK(IsNoChange(CPress::BackButton(kLaunchMode)));
	CHECK(IsChange(CPress::BackButton(kClipPicking), CPress::kSetup, 0, kLaunchMode));
	CHECK(IsChange(CPress::BackButton(CLauncher::SetupForClipEditing(3, 6)), CPress::kSetup, 0, kClipPicking));
	// From fill-in editing it returns to clip editing of the same launch button.
	CHECK(IsChange(CPress::BackButton(CLauncher::SetupForFillInEditing(3, 6)), CPress::kSetup, 0, CLauncher::SetupForClipEditing(3, 6)));
}

void TestClipEditingLastLaunchButtonDoesNothing() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	CHECK(IsNoChange(CPress::LaunchButton(editing, 7, 8, false, true)));
	CHECK(IsNoChange(CPress::LaunchButton(editing, 7, 8, false, false)));
}

void TestClipEditingColourRows() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	for (int column = 0; column < 7; ++column) {
		CHECK(IsChange(CPress::LaunchButton(editing, column, 2, false, true), CPress::kClipColour, picked, column));
		CHECK(IsChange(CPress::LaunchButton(editing, column, 3, false, true), CPress::kClipColour, picked, 7 + column));
	}
	// Column 8 of the colour rows is not a control.
	CHECK(IsNoChange(CPress::LaunchButton(editing, 7, 2, false, true)));
	CHECK(IsNoChange(CPress::LaunchButton(editing, 7, 3, false, true)));
}

void TestClipEditingPatternRow() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	for (int column = 0; column < 8; ++column) {
		CHECK(IsChange(CPress::LaunchButton(editing, column, 5, false, true), CPress::kClipPattern, picked, column));
	}
}

void TestClipEditingUnusedLaunchButtonsDoNothing() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	CHECK(IsNoChange(CPress::LaunchButton(editing, 0, 1, false, true)));
	CHECK(IsNoChange(CPress::LaunchButton(editing, 3, 4, false, true)));
	// Not even the picked launch button itself, whatever it holds.
	CHECK(IsNoChange(CPress::LaunchButton(editing, 3, 6, true, true)));
	CHECK(IsNoChange(CPress::LaunchButton(editing, 0, 8, false, true)));
}

void TestClipEditingWithoutClipHasNoColourOrPatternControls() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	CHECK(IsNoChange(CPress::LaunchButton(editing, 2, 2, false, false)));
	CHECK(IsNoChange(CPress::LaunchButton(editing, 2, 3, false, false)));
	CHECK(IsNoChange(CPress::LaunchButton(editing, 2, 5, false, false)));
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

void TestSetFillInButtonGoesOnFromClipEditingOfAClip() {
	CHECK(IsChange(CPress::SetFillInButton(CLauncher::SetupForClipEditing(3, 6), true), CPress::kSetup, 0, CLauncher::SetupForFillInEditing(3, 6)));
	CHECK(IsNoChange(CPress::SetFillInButton(CLauncher::SetupForClipEditing(3, 6), false)));
	CHECK(IsNoChange(CPress::SetFillInButton(kLaunchMode, true)));
	CHECK(IsNoChange(CPress::SetFillInButton(kClipPicking, true)));
	CHECK(IsNoChange(CPress::SetFillInButton(CLauncher::SetupForFillInEditing(3, 6), true)));
}

void TestFillInButtonFollowsTheButton() {
	CHECK(IsChange(CPress::FillInButton(true), CPress::kFillIn, 0, 1));
	CHECK(IsChange(CPress::FillInButton(false), CPress::kFillIn, 0, 0));
}

void TestLaunchButtonDoesNothingInFillInEditing() {
	const int editing = CLauncher::SetupForFillInEditing(3, 6);
	CHECK(IsNoChange(CPress::LaunchButton(editing, 7, 1, false, true)));
	CHECK(IsNoChange(CPress::LaunchButton(editing, 0, 2, false, true)));
}

void TestFillInEditingDoesNothingElsewhere() {
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(kLaunchMode, 7, 1, false, 2), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(kClipPicking, 7, 1, false, 2), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(CLauncher::SetupForClipEditing(3, 6), 7, 1, false, 2), 0));
}

void TestFillInEditingToggleSetsUpANewFillIn() {
	const int editing = CLauncher::SetupForFillInEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);

	// A new fill-in has the pattern of its clip, in white.
	const CPress::TChanges set = CPress::FillInEditingLaunchButton(editing, 7, 1, false, 4);
	CHECK(IsChanges(set, 3));
	CHECK(IsChange(set.fChanges[0], CPress::kFillInPresent, picked, 1));
	CHECK(IsChange(set.fChanges[1], CPress::kFillInPattern, picked, 4));
	CHECK(IsChange(set.fChanges[2], CPress::kFillInColour, picked, CLauncher::kColourWhite));

	const CPress::TChanges clear = CPress::FillInEditingLaunchButton(editing, 7, 1, true, 4);
	CHECK(IsChanges(clear, 1));
	CHECK(IsChange(clear.fChanges[0], CPress::kFillInPresent, picked, 0));
}

void TestFillInEditingColourAndPatternRows() {
	const int editing = CLauncher::SetupForFillInEditing(3, 6);
	const int picked = CLauncher::ButtonIndex(3, 6);
	for (int column = 0; column < 7; ++column) {
		const CPress::TChanges first = CPress::FillInEditingLaunchButton(editing, column, 2, true, 0);
		CHECK(IsChanges(first, 1) && IsChange(first.fChanges[0], CPress::kFillInColour, picked, column));
		const CPress::TChanges second = CPress::FillInEditingLaunchButton(editing, column, 3, true, 0);
		CHECK(IsChanges(second, 1) && IsChange(second.fChanges[0], CPress::kFillInColour, picked, 7 + column));
	}
	for (int column = 0; column < 8; ++column) {
		const CPress::TChanges pattern = CPress::FillInEditingLaunchButton(editing, column, 5, true, 0);
		CHECK(IsChanges(pattern, 1) && IsChange(pattern.fChanges[0], CPress::kFillInPattern, picked, column));
	}
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(editing, 7, 2, true, 0), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(editing, 0, 1, true, 0), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(editing, 3, 6, true, 0), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(editing, 0, 8, true, 0), 0));
}

void TestFillInEditingWithoutFillInHasNoColourOrPatternControls() {
	const int editing = CLauncher::SetupForFillInEditing(3, 6);
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(editing, 2, 2, false, 0), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(editing, 2, 3, false, 0), 0));
	CHECK(IsChanges(CPress::FillInEditingLaunchButton(editing, 2, 5, false, 0), 0));
}

void TestSceneButtonDoesNothingInFillInEditing() {
	const bool holdsClip[CLauncher::kTrackCount] = { true, true, true, true, true, true, true, true };
	CHECK(CPress::SceneButton(CLauncher::SetupForFillInEditing(0, 1), 3, holdsClip).fCount == 0);
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
	TestClipEditingLastLaunchButtonDoesNothing();
	TestClipEditingColourRows();
	TestClipEditingPatternRow();
	TestClipEditingUnusedLaunchButtonsDoNothing();
	TestClipEditingWithoutClipHasNoColourOrPatternControls();
	TestSceneButtonSelectsItsRowOnEveryTrack();
	TestSceneButtonStopsTracksWithAnEmptyLaunchButton();
	TestSceneButtonOfAnEmptySceneStopsEveryTrack();
	TestSceneButtonDoesNothingInClipEditing();
	TestSceneButtonInClipPickingChoosesItsLengthAndLeavesSetupMode();

	TestSetupButtonLeavesFillInEditing();
	TestSetFillInButtonGoesOnFromClipEditingOfAClip();
	TestFillInButtonFollowsTheButton();
	TestLaunchButtonDoesNothingInFillInEditing();
	TestFillInEditingDoesNothingElsewhere();
	TestFillInEditingToggleSetsUpANewFillIn();
	TestFillInEditingColourAndPatternRows();
	TestFillInEditingWithoutFillInHasNoColourOrPatternControls();
	TestSceneButtonDoesNothingInFillInEditing();

	TestFunctionButtonInStopFunctionSelectsStop();
	TestFunctionButtonIsHeldDownAsMuteOrSoloButton();
	TestFunctionButtonOfATrackWithoutClipsDoesNothing();
	TestFunctionSelectButtonChoosesItsFunction();

	return ReportChecks("Press");
}
