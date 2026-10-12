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

void TestClipEditingBackReturnsToClipPicking() {
	const int editing = CLauncher::SetupForClipEditing(3, 6);
	CHECK(IsChange(CPress::LaunchButton(editing, 7, 8, false, true), CPress::kSetup, 0, kClipPicking));
	CHECK(IsChange(CPress::LaunchButton(editing, 7, 8, false, false), CPress::kSetup, 0, kClipPicking));
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

int main() {
	TestLaunchModeSelectsTheClip();
	TestLaunchModeEmptyLaunchButtonSelectsStop();
	TestStopButtonSelectsStopInEveryMode();
	TestSetupButtonSwitchesBetweenLaunchModeAndClipPicking();
	TestClipPickingOpensThePressedLaunchButton();
	TestClipEditingToggleSwitchesThePickedClip();
	TestClipEditingBackReturnsToClipPicking();
	TestClipEditingColourRows();
	TestClipEditingPatternRow();
	TestClipEditingUnusedLaunchButtonsDoNothing();
	TestClipEditingWithoutClipHasNoColourOrPatternControls();
	TestSceneButtonSelectsItsRowOnEveryTrack();
	TestSceneButtonStopsTracksWithAnEmptyLaunchButton();
	TestSceneButtonOfAnEmptySceneStopsEveryTrack();
	TestSceneButtonDoesNothingInClipEditing();
	TestSceneButtonInClipPickingChoosesItsLengthAndLeavesSetupMode();

	return ReportChecks("Press");
}
