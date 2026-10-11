// Command-line tests for the switching rules and the lights. Run with
// tests/run.sh.

#include "../Launcher.h"

#include "Check.h"

namespace {

const double kBatch = 20.0; // about one batch at 120 BPM and 44.1 kHz
const long long kBar = CLauncher::kPPQPerCrotchet * 4;
const long long kInterval = kBar * 4;

CLauncher::TTransport Playing(double iPlayPos, int iNumerator = 4, int iDenominator = 4) {
	CLauncher::TTransport transport;
	transport.fPlaying = true;
	transport.fPlayPos = iPlayPos;
	transport.fBatchLength = kBatch;
	transport.fTimeSignatureNumerator = iNumerator;
	transport.fTimeSignatureDenominator = iDenominator;
	return transport;
}

CLauncher::TTransport Stopped(double iPlayPos = 0.0) {
	CLauncher::TTransport transport = Playing(iPlayPos);
	transport.fPlaying = false;
	return transport;
}

// Puts a clip on every launch button, each playing the pattern of its row, so
// that the grid behaves as it did before clips could be set up.
void FillGrid(CLauncher& iLauncher) {
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		for (int row = 1; row <= CLauncher::kRowCount; ++row) {
			iLauncher.SetClipPresent(track, row, true);
		}
	}
}

// Makes a track emit a selection at once, as if the song had been saved with it.
void OpenWith(CLauncher& iLauncher, int iTrack, int iSelection) {
	iLauncher.SetSelection(iTrack, iSelection);
	iLauncher.Open();
}

bool IsLight(const CLauncher::TLight& iLight, CLauncher::EBrightness iBrightness, int iColour) {
	return iLight.fBrightness == iBrightness && iLight.fColour == iColour;
}

bool IsUnlit(const CLauncher::TLight& iLight) {
	return iLight.fBrightness == CLauncher::kUnlit;
}

// Plays batch by batch from iFrom up to (but not including) iTo.
void PlayThrough(CLauncher& iLauncher, double iFrom, double iTo) {
	for (double position = iFrom; position < iTo; position += kBatch) {
		iLauncher.Advance(Playing(position));
	}
}

void TestInitialState() {
	CLauncher launcher;
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		CHECK(launcher.GetButtonState(track, CLauncher::kSelectionStop) == CLauncher::kActive);
		CHECK(launcher.IsStopLit(track));
		CHECK(launcher.GetCVValue(track) == 0.0);
		// A new device has only empty launch buttons.
		for (int row = 1; row <= 8; ++row) {
			CHECK(launcher.GetButtonState(track, row) == CLauncher::kOff);
			CHECK(IsUnlit(launcher.GetLight(track, row)));
			CHECK(launcher.GetLightValue(track, row) == 0);
			CHECK(launcher.GetLightFrame(track, row) == 0);
		}
	}
}

void TestClickWhileStoppedTakesEffectAtOnce() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Stopped());
	launcher.SetSelection(2, 5);
	launcher.Advance(Stopped());
	CHECK(launcher.GetButtonState(2, 5) == CLauncher::kActive);
	CHECK(launcher.GetButtonState(2, CLauncher::kSelectionStop) == CLauncher::kOff);
	CHECK(launcher.GetCVValue(2) == 5.0 / 8.0);
	// Other tracks are untouched.
	CHECK(launcher.GetButtonState(3, CLauncher::kSelectionStop) == CLauncher::kActive);
}

void TestClickWhilePlayingQueuesUntilSwitchPoint() {
	CLauncher launcher;
	FillGrid(launcher);
	PlayThrough(launcher, kBar, kBar + 100);
	launcher.SetSelection(0, 3);
	launcher.Advance(Playing(kBar + 100));

	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kQueued);
	// The outgoing button stays active and the CV keeps its value.
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kActive);
	CHECK(launcher.IsStopLit(0));
	CHECK(launcher.GetCVValue(0) == 0.0);

	// Still queued just before the switch point.
	PlayThrough(launcher, kBar + 120, kInterval - 2 * kBatch);
	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kQueued);

	// The batch that reaches the switch point makes the change.
	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kActive);
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kOff);
	CHECK(launcher.GetCVValue(0) == 3.0 / 8.0);
}

void TestStopButtonQueuesLikeALaunchButton() {
	CLauncher launcher;
	FillGrid(launcher);
	OpenWith(launcher, 0, 4);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, CLauncher::kSelectionStop);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kQueued);
	CHECK(launcher.GetButtonState(0, 4) == CLauncher::kActive);
	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kActive);
	CHECK(launcher.GetButtonState(0, 4) == CLauncher::kOff);
	CHECK(launcher.GetCVValue(0) == 0.0);
}

void TestLastClickWins() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 3);
	launcher.Advance(Playing(120));
	launcher.SetSelection(0, 5);
	launcher.Advance(Playing(140));
	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kOff);
	CHECK(launcher.GetButtonState(0, 5) == CLauncher::kQueued);
	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetButtonState(0, 5) == CLauncher::kActive);
}

void TestClickingQueuedButtonAgainIsIgnored() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 3);
	launcher.Advance(Playing(120));
	launcher.SetSelection(0, 3);
	launcher.Advance(Playing(140));
	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kQueued);
}

void TestClickingActiveButtonCancelsQueue() {
	CLauncher launcher;
	FillGrid(launcher);
	OpenWith(launcher, 0, 3);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 5);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetButtonState(0, 5) == CLauncher::kQueued);
	launcher.SetSelection(0, 3);
	launcher.Advance(Playing(140));
	CHECK(launcher.GetButtonState(0, 5) == CLauncher::kOff);
	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kActive);
	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kActive);
}

void TestStoppingTransportPromotesQueued() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(100));
	launcher.SetSelection(1, 7);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetButtonState(1, 7) == CLauncher::kQueued);
	launcher.Advance(Stopped(140));
	CHECK(launcher.GetButtonState(1, 7) == CLauncher::kActive);
	CHECK(launcher.GetCVValue(1) == 7.0 / 8.0);
	// Starting again keeps the same scene.
	launcher.Advance(Playing(140));
	CHECK(launcher.GetButtonState(1, 7) == CLauncher::kActive);
}

void TestJumpOverSwitchPointDoesNotFire() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(120));
	// The user drags the song position past the switch point at bar 5.
	launcher.Advance(Playing(kInterval + kBar));
	CHECK(launcher.GetButtonState(0, 2) == CLauncher::kQueued);
	// It fires at the next switch point that is played through.
	launcher.Advance(Playing(2 * kInterval - 1));
	CHECK(launcher.GetButtonState(0, 2) == CLauncher::kActive);
}

void TestLoopWithoutSwitchPointKeepsQueue() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	// Loop over bars 2 and 3, several times round.
	for (int round = 0; round < 3; ++round) {
		PlayThrough(launcher, kBar, 3 * kBar);
	}
	CHECK(launcher.GetButtonState(0, 2) == CLauncher::kQueued);
}

void TestSelectionChangeOnSwitchPointIsImmediate() {
	// Automation drawn exactly on a switch point.
	CLauncher launcher;
	FillGrid(launcher);
	PlayThrough(launcher, kInterval - 100, kInterval);
	launcher.SetSelection(0, 6);
	launcher.Advance(Playing(kInterval));
	CHECK(launcher.GetButtonState(0, 6) == CLauncher::kActive);

	// The same change arriving one batch late is still taken at once.
	CLauncher late;
	FillGrid(late);
	PlayThrough(late, kInterval - 100, kInterval + kBatch);
	late.SetSelection(0, 6);
	late.Advance(Playing(kInterval + kBatch));
	CHECK(late.GetButtonState(0, 6) == CLauncher::kActive);

	// Two batches late is an ordinary click and waits for the next switch point.
	CLauncher later;
	FillGrid(later);
	PlayThrough(later, kInterval - 100, kInterval + 2 * kBatch);
	later.SetSelection(0, 6);
	later.Advance(Playing(kInterval + 2 * kBatch));
	CHECK(later.GetButtonState(0, 6) == CLauncher::kQueued);
}

void TestSwitchPointsFollowTimeSignature() {
	CHECK(CLauncher::ComputeSwitchIntervalLength(4, 4) == 16 * CLauncher::kPPQPerCrotchet);
	CHECK(CLauncher::ComputeSwitchIntervalLength(3, 4) == 12 * CLauncher::kPPQPerCrotchet);
	CHECK(CLauncher::ComputeSwitchIntervalLength(6, 8) == 12 * CLauncher::kPPQPerCrotchet);
	CHECK(CLauncher::ComputeSwitchIntervalLength(7, 8) == 14 * CLauncher::kPPQPerCrotchet);

	// In 3/4 the first switch point after the start is twelve crotchets in.
	const long long interval = 12 * CLauncher::kPPQPerCrotchet;
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(100, 3, 4));
	launcher.SetSelection(0, 1);
	launcher.Advance(Playing(interval - 100, 3, 4));
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kQueued);
	launcher.Advance(Playing(interval - 1, 3, 4));
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
}

void TestQueuedButtonFlashesOnTheBeat() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.SetClipColour(0, 3, 4);
	OpenWith(launcher, 1, 2);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 3);
	launcher.SetSelection(1, CLauncher::kSelectionStop);
	const long long quaver = CLauncher::kPPQPerCrotchet / 2;
	const int dimFrame = 1 + 2 * 4;
	const int vibrantFrame = dimFrame + 1;

	// Vibrant for the first quaver of a crotchet, dim for the second.
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet));
	CHECK(launcher.GetLightFrame(0, 3) == vibrantFrame);
	CHECK(launcher.IsStopLit(1));
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver - kBatch));
	CHECK(launcher.GetLightFrame(0, 3) == vibrantFrame);
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver));
	CHECK(launcher.GetLightFrame(0, 3) == dimFrame);
	CHECK(!launcher.IsStopLit(1));
	launcher.Advance(Playing(2 * CLauncher::kPPQPerCrotchet));
	CHECK(launcher.GetLightFrame(0, 3) == vibrantFrame);

	// To a control surface the light is steadily "flashing".
	CHECK(IsLight(launcher.GetLight(0, 3), CLauncher::kFlashing, 4));

	// The active buttons do not flash.
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver));
	CHECK(launcher.IsStopLit(0));
	CHECK(launcher.GetLightFrame(1, 2) == 2);
}

void TestOpenMakesSelectionsActive() {
	CLauncher launcher;
	FillGrid(launcher);
	const int selections[CLauncher::kTrackCount] = { 0, 1, 2, 3, 4, 5, 6, 8 };
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		launcher.SetSelection(track, selections[track]);
	}
	launcher.Open();
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		CHECK(launcher.GetButtonState(track, selections[track]) == CLauncher::kActive);
		CHECK(launcher.GetCVValue(track) == selections[track] / 8.0);
	}
}

void TestClipEmitsItsPatternNotItsRow() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 2, true);
	launcher.SetClipPattern(0, 2, 6);
	launcher.SetClipPresent(0, 7, true);
	launcher.SetClipPattern(0, 7, 6);
	launcher.Advance(Stopped());
	launcher.SetSelection(0, 2);
	launcher.Advance(Stopped());
	CHECK(launcher.GetCVValue(0) == 7.0 / 8.0);
	// Two clips of a track may use the same pattern.
	launcher.SetSelection(0, 7);
	launcher.Advance(Stopped());
	CHECK(launcher.GetButtonState(0, 7) == CLauncher::kActive);
	CHECK(launcher.GetCVValue(0) == 7.0 / 8.0);
}

void TestNewClipPlaysThePatternOfItsRow() {
	CLauncher launcher;
	launcher.SetClipPresent(4, 6, true);
	OpenWith(launcher, 4, 6);
	CHECK(launcher.GetCVValue(4) == 6.0 / 8.0);
	CHECK(IsLight(launcher.GetLight(4, 6), CLauncher::kVibrant, CLauncher::kColourWhite));
}

void TestSelectingEmptyLaunchButtonStopsTheTrack() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 3, true);
	OpenWith(launcher, 0, 3);
	launcher.Advance(Playing(100));
	// Automation can still point at a launch button that has lost its clip.
	launcher.SetSelection(0, 5);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kQueued);
	CHECK(launcher.GetButtonState(0, 5) == CLauncher::kOff);
	CHECK(IsUnlit(launcher.GetLight(0, 5)));
	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kActive);
	CHECK(launcher.GetCVValue(0) == 0.0);
}

void TestLaunchModeLights() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 1, true);
	launcher.SetClipColour(0, 1, 2);
	launcher.SetClipPresent(0, 2, true);
	launcher.SetClipColour(0, 2, 9);
	launcher.SetClipPresent(0, 3, true);
	launcher.SetClipColour(0, 3, 13);
	OpenWith(launcher, 0, 1);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(120));

	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, 2));
	CHECK(IsLight(launcher.GetLight(0, 2), CLauncher::kFlashing, 9));
	CHECK(IsLight(launcher.GetLight(0, 3), CLauncher::kDim, 13));
	CHECK(IsUnlit(launcher.GetLight(0, 4)));

	// As numbers for control surfaces: dim, vibrant, flashing per colour.
	CHECK(launcher.GetLightValue(0, 1) == 1 + 3 * 2 + 1);
	CHECK(launcher.GetLightValue(0, 2) == 1 + 3 * 9 + 2);
	CHECK(launcher.GetLightValue(0, 3) == 1 + 3 * 13);
	CHECK(launcher.GetLightValue(0, 4) == 0);
	CHECK(launcher.GetLightValue(0, 3) == CLauncher::kLightValueCount - 3);

	// As frames of the film strip: dim, vibrant per colour.
	CHECK(launcher.GetLightFrame(0, 1) == 2 + 2 * 2);
	CHECK(launcher.GetLightFrame(0, 3) == 1 + 2 * 13);
	CHECK(launcher.GetLightFrame(0, 4) == 0);
}

void TestEditingActiveClipWaitsForSwitchPoint() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 1, true);
	OpenWith(launcher, 0, 1);
	launcher.Advance(Playing(100));
	launcher.SetClipPattern(0, 1, 4);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetCVValue(0) == 1.0 / 8.0);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetCVValue(0) == 5.0 / 8.0);

	// While the transport is stopped the change is heard at once.
	launcher.Advance(Stopped());
	launcher.SetClipPattern(0, 1, 7);
	launcher.Advance(Stopped());
	CHECK(launcher.GetCVValue(0) == 8.0 / 8.0);
}

void TestSwitchingActiveClipOffStopsAtSwitchPoint() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 1, true);
	launcher.SetClipColour(0, 1, 5);
	OpenWith(launcher, 0, 1);
	launcher.Advance(Playing(100));
	launcher.SetClipPresent(0, 1, false);
	launcher.Advance(Playing(120));

	// It plays on, in its colour, with the stop button queued.
	CHECK(launcher.GetCVValue(0) == 1.0 / 8.0);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, 5));
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kQueued);

	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetCVValue(0) == 0.0);
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kActive);
	CHECK(IsUnlit(launcher.GetLight(0, 1)));

	// The track's selection still names it, so switched on again it is queued,
	// as it was.
	launcher.Advance(Playing(kInterval + 2 * kBatch));
	launcher.SetClipPresent(0, 1, true);
	launcher.Advance(Playing(kInterval + 3 * kBatch));
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kQueued);
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kFlashing, 5));
}

void TestClipPickingFlashesEveryLaunchButton() {
	CLauncher launcher;
	launcher.SetClipPresent(2, 4, true);
	launcher.SetClipColour(2, 4, 7);
	OpenWith(launcher, 2, 4);
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	launcher.Advance(Stopped());

	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		for (int row = 1; row <= CLauncher::kRowCount; ++row) {
			const bool isTheClip = track == 2 && row == 4;
			CHECK(IsLight(launcher.GetLight(track, row), CLauncher::kFlashing, isTheClip ? 7 : CLauncher::kColourWhite));
		}
	}
	// Tracks go on playing and stop buttons go on showing their state.
	CHECK(launcher.GetCVValue(2) == 4.0 / 8.0);
	CHECK(!launcher.IsStopLit(2));
	CHECK(launcher.IsStopLit(3));
}

void TestFlashingGoesOnWhileStopped() {
	CLauncher launcher;
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	const double batch = CLauncher::kPPQPerCrotchet / 8.0;
	CLauncher::TTransport stopped = Stopped();
	stopped.fBatchLength = batch;

	// Eight batches make a crotchet: four vibrant, four dim, and round again.
	for (int round = 0; round < 2; ++round) {
		for (int i = 0; i < 8; ++i) {
			launcher.Advance(stopped);
			CHECK(launcher.GetLightFrame(0, 1) == (i < 4 ? 2 : 1));
		}
	}
}

void TestClipEditingOfEmptyLaunchButtonShowsOnlyToggleAndBack() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.SetClipPresent(3, 6, false);
	launcher.SetSetup(CLauncher::SetupForClipEditing(3, 6));

	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		for (int row = 1; row <= CLauncher::kRowCount; ++row) {
			const bool isToggle = track == 7 && row == 1;
			const bool isBack = track == 7 && row == 8;
			if (isToggle || isBack) {
				CHECK(IsLight(launcher.GetLight(track, row), CLauncher::kDim, CLauncher::kColourWhite));
			}
			else {
				CHECK(IsUnlit(launcher.GetLight(track, row)));
			}
		}
	}
}

void TestClipEditingShowsColoursAndPatterns() {
	CLauncher launcher;
	launcher.SetClipPresent(3, 6, true);
	launcher.SetClipColour(3, 6, 9);
	launcher.SetClipPattern(3, 6, 2);
	launcher.SetSetup(CLauncher::SetupForClipEditing(3, 6));

	CHECK(IsLight(launcher.GetLight(7, 1), CLauncher::kVibrant, CLauncher::kColourWhite));
	CHECK(IsLight(launcher.GetLight(7, 8), CLauncher::kDim, CLauncher::kColourWhite));

	// Row 2 holds white to turquoise, row 3 cyan to pink, in columns 1 to 7.
	for (int column = 0; column < 7; ++column) {
		CHECK(IsLight(launcher.GetLight(column, 2), CLauncher::kDim, column));
		const int colour = 7 + column;
		CHECK(IsLight(launcher.GetLight(column, 3), colour == 9 ? CLauncher::kVibrant : CLauncher::kDim, colour));
	}
	CHECK(IsUnlit(launcher.GetLight(7, 2)));
	CHECK(IsUnlit(launcher.GetLight(7, 3)));

	// Row 5 holds the eight patterns, in pink.
	for (int column = 0; column < 8; ++column) {
		CHECK(IsLight(launcher.GetLight(column, 5), column == 2 ? CLauncher::kVibrant : CLauncher::kDim, CLauncher::kColourPink));
	}

	// Everything else is unlit, including the picked launch button itself.
	const int unlitRows[] = { 4, 6, 7 };
	for (int row : unlitRows) {
		for (int column = 0; column < 8; ++column) {
			CHECK(IsUnlit(launcher.GetLight(column, row)));
		}
	}
	for (int column = 0; column < 7; ++column) {
		CHECK(IsUnlit(launcher.GetLight(column, 1)));
		CHECK(IsUnlit(launcher.GetLight(column, 8)));
	}
}

void TestLeavingSetupModeShowsTheEditedClip() {
	CLauncher launcher;
	launcher.SetSetup(CLauncher::SetupForClipEditing(0, 1));
	launcher.SetClipPresent(0, 1, true);
	launcher.SetClipColour(0, 1, 3);
	launcher.SetSetup(CLauncher::kSetupLaunchMode);
	launcher.Advance(Stopped());
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kDim, 3));
}

void TestButtonIndexAndSetupNumbering() {
	CHECK(CLauncher::ButtonIndex(0, 1) == 0);
	CHECK(CLauncher::ButtonIndex(0, 8) == 7);
	CHECK(CLauncher::ButtonIndex(1, 1) == 8);
	CHECK(CLauncher::ButtonIndex(7, 8) == 63);
	CHECK(CLauncher::SetupForClipEditing(0, 1) == 2);
	CHECK(CLauncher::SetupForClipEditing(7, 8) == CLauncher::kSetupCount - 1);
}

void TestOutOfRangeClipSettingsAreIgnored() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 1, true);
	launcher.SetClipColour(0, 1, 14);
	launcher.SetClipPattern(0, 1, 8);
	launcher.SetClipPresent(8, 1, true);
	launcher.SetClipPresent(0, 0, true);
	launcher.SetClipPresent(0, 9, true);
	launcher.SetSetup(CLauncher::kSetupCount);
	OpenWith(launcher, 0, 1);
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::kColourWhite));
	CHECK(launcher.GetCVValue(0) == 1.0 / 8.0);
}

void TestOneActiveAndAtMostOneQueuedPerTrack() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(100));
	const int clicks[] = { 3, 5, 0, 5, 8, 8, 1 };
	double position = 120;
	for (int click : clicks) {
		launcher.SetSelection(0, click);
		launcher.Advance(Playing(position));
		position += kBatch;
		int active = 0;
		int queued = 0;
		for (int selection = 0; selection < CLauncher::kSelectionCount; ++selection) {
			const CLauncher::EButtonState state = launcher.GetButtonState(0, selection);
			active += state == CLauncher::kActive ? 1 : 0;
			queued += state == CLauncher::kQueued ? 1 : 0;
		}
		CHECK(active == 1);
		CHECK(queued <= 1);
	}
}

void TestOutOfRangeSelectionIsIgnored() {
	CLauncher launcher;
	launcher.SetSelection(0, 9);
	launcher.SetSelection(8, 1);
	launcher.SetSelection(-1, 1);
	launcher.Advance(Stopped());
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kActive);
}

} // namespace

int main() {
	TestInitialState();
	TestClickWhileStoppedTakesEffectAtOnce();
	TestClickWhilePlayingQueuesUntilSwitchPoint();
	TestStopButtonQueuesLikeALaunchButton();
	TestLastClickWins();
	TestClickingQueuedButtonAgainIsIgnored();
	TestClickingActiveButtonCancelsQueue();
	TestStoppingTransportPromotesQueued();
	TestJumpOverSwitchPointDoesNotFire();
	TestLoopWithoutSwitchPointKeepsQueue();
	TestSelectionChangeOnSwitchPointIsImmediate();
	TestSwitchPointsFollowTimeSignature();
	TestQueuedButtonFlashesOnTheBeat();
	TestOpenMakesSelectionsActive();
	TestClipEmitsItsPatternNotItsRow();
	TestNewClipPlaysThePatternOfItsRow();
	TestSelectingEmptyLaunchButtonStopsTheTrack();
	TestLaunchModeLights();
	TestEditingActiveClipWaitsForSwitchPoint();
	TestSwitchingActiveClipOffStopsAtSwitchPoint();
	TestClipPickingFlashesEveryLaunchButton();
	TestFlashingGoesOnWhileStopped();
	TestClipEditingOfEmptyLaunchButtonShowsOnlyToggleAndBack();
	TestClipEditingShowsColoursAndPatterns();
	TestLeavingSetupModeShowsTheEditedClip();
	TestButtonIndexAndSetupNumbering();
	TestOutOfRangeClipSettingsAreIgnored();
	TestOneActiveAndAtMostOneQueuedPerTrack();
	TestOutOfRangeSelectionIsIgnored();

	return ReportChecks("Launcher");
}
