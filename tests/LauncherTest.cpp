// Command-line tests for the switching rules and the lights. Run with
// tests/run.sh.

#include "../Launcher.h"

#include "Check.h"

#include <cmath>

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
		// With no clips to stop, the stop button is unlit.
		CHECK(launcher.GetStopLight(track) == CLauncher::kUnlit);
		CHECK(launcher.GetFunctionLightFrame(track) == 0);
		CHECK(launcher.GetCVValue(track, 0) == 0.0);
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
	CHECK(launcher.GetCVValue(2, 0) == 5.0 / 8.0);
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
	CHECK(launcher.GetStopLight(0) == CLauncher::kDim);
	CHECK(launcher.GetCVValue(0, 0) == 0.0);

	// Still queued just before the switch point.
	PlayThrough(launcher, kBar + 120, kInterval - 2 * kBatch);
	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kQueued);

	// The batch that reaches the switch point makes the change.
	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kActive);
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kOff);
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8.0);
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
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
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
	CHECK(launcher.GetCVValue(1, 0) == 7.0 / 8.0);
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
	const int fourBars = CLauncher::kLengthFourBars;
	CHECK(CLauncher::ComputeSwitchIntervalLength(fourBars, 4, 4) == 16 * CLauncher::kPPQPerCrotchet);
	CHECK(CLauncher::ComputeSwitchIntervalLength(fourBars, 3, 4) == 12 * CLauncher::kPPQPerCrotchet);
	CHECK(CLauncher::ComputeSwitchIntervalLength(fourBars, 6, 8) == 12 * CLauncher::kPPQPerCrotchet);
	CHECK(CLauncher::ComputeSwitchIntervalLength(fourBars, 7, 8) == 14 * CLauncher::kPPQPerCrotchet);

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
	CHECK(launcher.GetFunctionLightFrame(1) == 2);
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver - kBatch));
	CHECK(launcher.GetLightFrame(0, 3) == vibrantFrame);
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver));
	CHECK(launcher.GetLightFrame(0, 3) == dimFrame);
	CHECK(launcher.GetFunctionLightFrame(1) == 1);
	launcher.Advance(Playing(2 * CLauncher::kPPQPerCrotchet));
	CHECK(launcher.GetLightFrame(0, 3) == vibrantFrame);

	// To a control surface the lights are steadily "flashing".
	CHECK(IsLight(launcher.GetLight(0, 3), CLauncher::kFlashing, 4));
	CHECK(launcher.GetStopLight(1) == CLauncher::kFlashing);

	// The active buttons do not flash. Track 2's clips are orange.
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver));
	CHECK(launcher.GetFunctionLightFrame(0) == 1);
	CHECK(launcher.GetLightFrame(1, 2) == 2 + 2 * 2);
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
		CHECK(launcher.GetCVValue(track, 0) == selections[track] / 8.0);
	}
}

void TestClipEmitsItsPatternNotItsRow() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 2, true);
	launcher.SetClipPattern(0, 2, 0, 6);
	launcher.SetClipPresent(0, 7, true);
	launcher.SetClipPattern(0, 7, 0, 6);
	launcher.Advance(Stopped());
	launcher.SetSelection(0, 2);
	launcher.Advance(Stopped());
	CHECK(launcher.GetCVValue(0, 0) == 7.0 / 8.0);
	// Two clips of a track may use the same pattern.
	launcher.SetSelection(0, 7);
	launcher.Advance(Stopped());
	CHECK(launcher.GetButtonState(0, 7) == CLauncher::kActive);
	CHECK(launcher.GetCVValue(0, 0) == 7.0 / 8.0);
}

void TestNewClipPlaysThePatternOfItsRow() {
	CLauncher launcher;
	launcher.SetClipPresent(4, 6, true);
	OpenWith(launcher, 4, 6);
	CHECK(launcher.GetCVValue(4, 0) == 6.0 / 8.0);
	// ...and has the colour of its track.
	CHECK(IsLight(launcher.GetLight(4, 6), CLauncher::kVibrant, CLauncher::DefaultColour(4)));
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
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
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
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::DefaultColour(0)));
	launcher.SetClipPattern(0, 1, 0, 4);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetCVValue(0, 0) == 1.0 / 8.0);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
	// It flashes while the new pattern waits, as a queued clip would.
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kFlashing, CLauncher::DefaultColour(0)));
	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetCVValue(0, 0) == 5.0 / 8.0);
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::DefaultColour(0)));

	// Changing it back before the switch point leaves nothing to wait for.
	launcher.Advance(Playing(kInterval + 2 * kBatch));
	launcher.SetClipPattern(0, 1, 0, 6);
	launcher.Advance(Playing(kInterval + 3 * kBatch));
	CHECK(launcher.GetLight(0, 1).fBrightness == CLauncher::kFlashing);
	launcher.SetClipPattern(0, 1, 0, 4);
	launcher.Advance(Playing(kInterval + 4 * kBatch));
	CHECK(launcher.GetLight(0, 1).fBrightness == CLauncher::kVibrant);

	// While the transport is stopped the change is heard at once.
	launcher.Advance(Stopped());
	launcher.SetClipPattern(0, 1, 0, 7);
	launcher.Advance(Stopped());
	CHECK(launcher.GetCVValue(0, 0) == 8.0 / 8.0);
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
	CHECK(launcher.GetCVValue(0, 0) == 1.0 / 8.0);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, 5));
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kQueued);

	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
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

void TestStopLights() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 1, true);
	launcher.SetClipPresent(1, 1, true);
	OpenWith(launcher, 1, 1);
	launcher.Advance(Playing(100));

	// A track with clips, none playing: dim. One playing: vibrant. No clips: unlit.
	CHECK(launcher.GetStopLight(0) == CLauncher::kDim);
	CHECK(launcher.GetStopLight(1) == CLauncher::kVibrant);
	CHECK(launcher.GetStopLight(2) == CLauncher::kUnlit);
	CHECK(launcher.GetFunctionLightFrame(0) == 1);
	CHECK(launcher.GetFunctionLightFrame(1) == 2);
	CHECK(launcher.GetFunctionLightFrame(2) == 0);

	// A queued clip does not change the stop button...
	launcher.SetSelection(0, 1);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetStopLight(0) == CLauncher::kDim);
	// ...a queued stop makes it flash...
	launcher.SetSelection(1, CLauncher::kSelectionStop);
	launcher.Advance(Playing(140));
	CHECK(launcher.GetStopLight(1) == CLauncher::kFlashing);
	// ...until the track has stopped.
	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetStopLight(0) == CLauncher::kVibrant);
	CHECK(launcher.GetStopLight(1) == CLauncher::kDim);

	// Losing its last clip while it plays, a track still has something to stop.
	launcher.Advance(Playing(kInterval + 2 * kBatch));
	launcher.SetClipPresent(0, 1, false);
	launcher.Advance(Playing(kInterval + 3 * kBatch));
	CHECK(launcher.GetStopLight(0) == CLauncher::kFlashing);
	launcher.Advance(Playing(2 * kInterval - 1));
	CHECK(launcher.GetStopLight(0) == CLauncher::kUnlit);
}

void TestDefaultColourFollowsTheTrack() {
	// Red, orange, yellow, green, cyan, blue, purple, pink.
	const int expected[CLauncher::kTrackCount] = { 1, 2, 3, 5, 7, 9, 11, 13 };
	CLauncher launcher;
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		CHECK(CLauncher::DefaultColour(track) == expected[track]);
		launcher.SetClipPresent(track, 3, true);
		CHECK(IsLight(launcher.GetLight(track, 3), CLauncher::kDim, expected[track]));
	}
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
	// Tracks go on playing and stop buttons go on showing their light.
	CHECK(launcher.GetCVValue(2, 0) == 4.0 / 8.0);
	CHECK(launcher.GetStopLight(2) == CLauncher::kVibrant);
	CHECK(launcher.GetStopLight(3) == CLauncher::kUnlit);
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

void TestClipEditingOfEmptyLaunchButtonShowsOnlyToggle() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.SetClipPresent(3, 6, false);
	launcher.SetSetup(CLauncher::SetupForClipEditing(3, 6));

	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		for (int row = 1; row <= CLauncher::kRowCount; ++row) {
			const bool isToggle = track == 7 && row == 1;
			if (isToggle) {
				CHECK(IsLight(launcher.GetLight(track, row), CLauncher::kDim, CLauncher::kColourRed));
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
	launcher.SetClipPattern(3, 6, 0, 2);
	launcher.SetSetup(CLauncher::SetupForClipEditing(3, 6));

	CHECK(IsLight(launcher.GetLight(7, 1), CLauncher::kVibrant, CLauncher::kColourGreen));

	// Row 1 holds white to turquoise, row 2 cyan to pink, in columns 1 to 7.
	for (int column = 0; column < 7; ++column) {
		CHECK(IsLight(launcher.GetLight(column, 1), CLauncher::kDim, column));
		const int colour = 7 + column;
		CHECK(IsLight(launcher.GetLight(column, 2), colour == 9 ? CLauncher::kVibrant : CLauncher::kDim, colour));
	}
	CHECK(IsUnlit(launcher.GetLight(7, 2)));

	// Row 3 stands for output A, in pattern mode: eight patterns, in orange.
	for (int column = 0; column < 8; ++column) {
		CHECK(IsLight(launcher.GetLight(column, 3), column == 2 ? CLauncher::kVibrant : CLauncher::kDim, CLauncher::kColourOrange));
	}

	// Rows 4 to 8 stand for outputs B to F, in unipolar mode at 0, with the
	// transition off.
	for (int row = 4; row <= 8; ++row) {
		CHECK(IsLight(launcher.GetLight(0, row), CLauncher::kVibrant, CLauncher::kColourCyan));
		for (int column = 1; column < 7; ++column) {
			CHECK(IsUnlit(launcher.GetLight(column, row)));
		}
		CHECK(IsLight(launcher.GetLight(7, row), CLauncher::kDim, CLauncher::kColourRed));
	}
}

void TestPatternsAreOrangeInEveryStep() {
	CLauncher launcher;
	launcher.SetClipPresent(3, 6, true);
	launcher.SetFillInPresent(3, 6, true);
	launcher.SetFillInPattern(3, 6, 0, 4);
	launcher.SetSetup(CLauncher::SetupForFillInEditing(3, 6));
	for (int column = 0; column < 8; ++column) {
		CHECK(IsLight(launcher.GetLight(column, 3), column == 4 ? CLauncher::kVibrant : CLauncher::kDim, CLauncher::kColourOrange));
	}
}

void TestUnipolarValueLightsTheValuesBelowIt() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 3, true);
	launcher.SetClipUnipolar(0, 3, 1, 5);
	launcher.SetSetup(CLauncher::SetupForClipEditing(0, 3));
	for (int column = 0; column < 5; ++column) {
		CHECK(IsLight(launcher.GetLight(column, 4), CLauncher::kDim, CLauncher::kColourCyan));
	}
	CHECK(IsLight(launcher.GetLight(5, 4), CLauncher::kVibrant, CLauncher::kColourCyan));
	CHECK(IsUnlit(launcher.GetLight(6, 4)));
}

void TestBipolarValueLightsTheValuesBackToZero() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 3, true);
	launcher.SetCVMode(0, 2, CLauncher::kCVModeBipolar);
	launcher.SetSetup(CLauncher::SetupForClipEditing(0, 3));

	// At 0 only the middle is lit.
	for (int column = 0; column < 7; ++column) {
		if (column == 3) {
			CHECK(IsLight(launcher.GetLight(column, 5), CLauncher::kVibrant, CLauncher::kColourFuchsia));
		}
		else {
			CHECK(IsUnlit(launcher.GetLight(column, 5)));
		}
	}

	launcher.SetClipBipolar(0, 3, 2, 5);
	CHECK(IsUnlit(launcher.GetLight(2, 5)));
	CHECK(IsLight(launcher.GetLight(3, 5), CLauncher::kDim, CLauncher::kColourFuchsia));
	CHECK(IsLight(launcher.GetLight(4, 5), CLauncher::kDim, CLauncher::kColourFuchsia));
	CHECK(IsLight(launcher.GetLight(5, 5), CLauncher::kVibrant, CLauncher::kColourFuchsia));
	CHECK(IsUnlit(launcher.GetLight(6, 5)));

	launcher.SetClipBipolar(0, 3, 2, 0);
	CHECK(IsLight(launcher.GetLight(0, 5), CLauncher::kVibrant, CLauncher::kColourFuchsia));
	CHECK(IsLight(launcher.GetLight(1, 5), CLauncher::kDim, CLauncher::kColourFuchsia));
	CHECK(IsLight(launcher.GetLight(2, 5), CLauncher::kDim, CLauncher::kColourFuchsia));
	CHECK(IsLight(launcher.GetLight(3, 5), CLauncher::kDim, CLauncher::kColourFuchsia));
	CHECK(IsUnlit(launcher.GetLight(4, 5)));
}

void TestTransitionButtonShowsTheColourOfItsTransition() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 3, true);
	launcher.SetSetup(CLauncher::SetupForClipEditing(0, 3));
	const int colours[] = { CLauncher::kColourRed, CLauncher::kColourLime, CLauncher::kColourLightBlue, CLauncher::kColourPurple, CLauncher::kColourPink };
	for (int transition = 0; transition < CLauncher::kTransitionCount; ++transition) {
		launcher.SetClipTransition(0, 3, 3, transition);
		CHECK(IsLight(launcher.GetLight(7, 6), transition == 0 ? CLauncher::kDim : CLauncher::kVibrant, colours[transition]));
	}
	// A CV output in pattern mode has no transition button.
	launcher.SetCVMode(0, 3, CLauncher::kCVModePattern);
	CHECK(IsLight(launcher.GetLight(7, 6), CLauncher::kDim, CLauncher::kColourOrange));
}

void TestEditingFollowsTheCVModesOfThePickedTrack() {
	CLauncher launcher;
	launcher.SetClipPresent(2, 1, true);
	launcher.SetCVMode(2, 1, CLauncher::kCVModePattern);
	launcher.SetSetup(CLauncher::SetupForClipEditing(2, 1));
	CHECK(IsLight(launcher.GetLight(7, 4), CLauncher::kDim, CLauncher::kColourOrange));
	// Another track's CV modes make no difference.
	launcher.SetSetup(CLauncher::SetupForClipEditing(1, 1));
	launcher.SetClipPresent(1, 1, true);
	CHECK(IsLight(launcher.GetLight(7, 4), CLauncher::kDim, CLauncher::kColourRed));
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
	CHECK(CLauncher::SetupForClipEditing(7, 8) == 65);
	CHECK(CLauncher::SetupForFillInEditing(0, 1) == 66);
	CHECK(CLauncher::SetupForFillInEditing(7, 8) == 129);
	CHECK(CLauncher::SetupForLandingEditing(0, 1) == 130);
	CHECK(CLauncher::SetupForLandingEditing(7, 8) == CLauncher::kSetupCount - 1);

	CHECK(!CLauncher::IsClipEditing(CLauncher::kSetupClipPicking));
	CHECK(CLauncher::IsClipEditing(2));
	CHECK(CLauncher::IsClipEditing(65));
	CHECK(!CLauncher::IsClipEditing(66));
	CHECK(!CLauncher::IsFillInEditing(65));
	CHECK(CLauncher::IsFillInEditing(66));
	CHECK(CLauncher::IsFillInEditing(129));
	CHECK(!CLauncher::IsFillInEditing(130));
	CHECK(!CLauncher::IsLandingEditing(129));
	CHECK(CLauncher::IsLandingEditing(130));
	CHECK(CLauncher::IsLandingEditing(193));
	CHECK(!CLauncher::IsLandingEditing(194));

	CHECK(CLauncher::PickedButton(CLauncher::kSetupLaunchMode) == -1);
	CHECK(CLauncher::PickedButton(CLauncher::kSetupClipPicking) == -1);
	CHECK(CLauncher::PickedButton(CLauncher::SetupForClipEditing(3, 6)) == CLauncher::ButtonIndex(3, 6));
	CHECK(CLauncher::PickedButton(CLauncher::SetupForFillInEditing(3, 6)) == CLauncher::ButtonIndex(3, 6));
	CHECK(CLauncher::PickedButton(CLauncher::SetupForLandingEditing(3, 6)) == CLauncher::ButtonIndex(3, 6));
}

void TestOutOfRangeClipSettingsAreIgnored() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 1, true);
	launcher.SetClipColour(0, 1, 14);
	launcher.SetClipPattern(0, 1, 0, 8);
	launcher.SetClipPresent(8, 1, true);
	launcher.SetClipPresent(0, 0, true);
	launcher.SetClipPresent(0, 9, true);
	launcher.SetSetup(CLauncher::kSetupCount);
	OpenWith(launcher, 0, 1);
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::DefaultColour(0)));
	CHECK(launcher.GetCVValue(0, 0) == 1.0 / 8.0);
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

// How many scene buttons the interval meter has lit, checking on the way that
// they are the lowest ones.
int CountLitSceneButtons(const CLauncher& iLauncher) {
	int count = 0;
	bool unlitSeen = false;
	for (int scene = CLauncher::kRowCount; scene >= 1; --scene) {
		if (IsUnlit(iLauncher.GetSceneLight(scene))) {
			unlitSeen = true;
		}
		else {
			CHECK(!unlitSeen);
			++count;
		}
	}
	return count;
}

void TestIntervalMeterFillsFromTheBottom() {
	CLauncher launcher;
	const long long kMinim = CLauncher::kPPQPerCrotchet * 2;

	// One more scene button for every two beats in 4/4.
	for (int segment = 0; segment < 8; ++segment) {
		launcher.Advance(Playing(static_cast<double>(segment * kMinim)));
		CHECK(CountLitSceneButtons(launcher) == segment + 1);
		launcher.Advance(Playing(static_cast<double>((segment + 1) * kMinim - 1)));
		CHECK(CountLitSceneButtons(launcher) == segment + 1);
	}

	// It starts again at the switch point, in every switch interval.
	launcher.Advance(Playing(static_cast<double>(kInterval)));
	CHECK(CountLitSceneButtons(launcher) == 1);
	launcher.Advance(Playing(static_cast<double>(5 * kInterval + 3 * kMinim)));
	CHECK(CountLitSceneButtons(launcher) == 4);
}

void TestIntervalMeterColours() {
	CLauncher launcher;
	launcher.Advance(Playing(static_cast<double>(kInterval - 1)));

	// Scene 8 is the bottom scene button: five green, two yellow, one red, all
	// dim.
	for (int scene = 8; scene >= 4; --scene) {
		CHECK(IsLight(launcher.GetSceneLight(scene), CLauncher::kDim, CLauncher::kColourGreen));
	}
	CHECK(IsLight(launcher.GetSceneLight(3), CLauncher::kDim, CLauncher::kColourYellow));
	CHECK(IsLight(launcher.GetSceneLight(2), CLauncher::kDim, CLauncher::kColourYellow));
	CHECK(IsLight(launcher.GetSceneLight(1), CLauncher::kDim, CLauncher::kColourRed));

	// Encoded like the light of a launch button: dim green, dim yellow, dim red.
	CHECK(launcher.GetSceneLightValue(8) == 1 + 3 * CLauncher::kColourGreen);
	CHECK(launcher.GetSceneLightValue(3) == 1 + 3 * CLauncher::kColourYellow);
	CHECK(launcher.GetSceneLightValue(1) == 1 + 3 * CLauncher::kColourRed);

	// The film strip: unlit, then dim and vibrant for red, green and yellow.
	CHECK(launcher.GetSceneLightFrame(8) == 3);
	CHECK(launcher.GetSceneLightFrame(3) == 5);
	CHECK(launcher.GetSceneLightFrame(1) == 1);

	launcher.Advance(Playing(0.0));
	CHECK(launcher.GetSceneLightValue(7) == 0);
	CHECK(launcher.GetSceneLightFrame(7) == 0);
}

void TestIntervalMeterFollowsTheSongPositionWhileStopped() {
	CLauncher launcher;
	const long long kMinim = CLauncher::kPPQPerCrotchet * 2;

	launcher.Advance(Stopped(0.0));
	CHECK(CountLitSceneButtons(launcher) == 1);
	launcher.Advance(Stopped(static_cast<double>(2 * kInterval + 5 * kMinim + 100)));
	CHECK(CountLitSceneButtons(launcher) == 6);
	launcher.Advance(Stopped(static_cast<double>(kMinim)));
	CHECK(CountLitSceneButtons(launcher) == 2);
}

void TestIntervalMeterIsUnlitBeforeTheSongStarts() {
	CLauncher launcher;
	CHECK(CountLitSceneButtons(launcher) == 0);
	launcher.Advance(Playing(-100.0));
	CHECK(CountLitSceneButtons(launcher) == 0);
	launcher.Advance(Stopped(-1.0));
	CHECK(CountLitSceneButtons(launcher) == 0);
}

void TestIntervalMeterDividesAnyTimeSignatureIntoEight() {
	CLauncher launcher;
	// Four bars of 3/4 are twelve crotchets, so a segment is one and a half.
	const long long kSegment = CLauncher::kPPQPerCrotchet * 3 / 2;
	launcher.Advance(Playing(static_cast<double>(kSegment - 1), 3, 4));
	CHECK(CountLitSceneButtons(launcher) == 1);
	launcher.Advance(Playing(static_cast<double>(kSegment), 3, 4));
	CHECK(CountLitSceneButtons(launcher) == 2);
	launcher.Advance(Playing(static_cast<double>(8 * kSegment - 1), 3, 4));
	CHECK(CountLitSceneButtons(launcher) == 8);
	launcher.Advance(Playing(static_cast<double>(8 * kSegment), 3, 4));
	CHECK(CountLitSceneButtons(launcher) == 1);
}

void TestIntervalMeterRunsInClipEditing() {
	CLauncher launcher;
	launcher.SetSetup(CLauncher::SetupForClipEditing(0, 1));
	launcher.Advance(Playing(static_cast<double>(2 * kBar)));
	CHECK(CountLitSceneButtons(launcher) == 5);
}

void TestLengthsInFourFour() {
	const long long crotchet = CLauncher::kPPQPerCrotchet;
	const long long expected[CLauncher::kLengthCount] = { 0, crotchet, 2 * crotchet, kBar, 2 * kBar, 4 * kBar, 8 * kBar, 16 * kBar };
	for (int length = 0; length < CLauncher::kLengthCount; ++length) {
		CHECK(CLauncher::ComputeSwitchIntervalLength(length, 4, 4) == expected[length]);
	}
}

void TestBeatAndHalfBarFollowTheTimeSignature() {
	const long long crotchet = CLauncher::kPPQPerCrotchet;
	// In 6/8 a beat is a quaver and half a bar three of them.
	CHECK(CLauncher::ComputeSwitchIntervalLength(CLauncher::kLengthBeat, 6, 8) == crotchet / 2);
	CHECK(CLauncher::ComputeSwitchIntervalLength(CLauncher::kLengthHalfBar, 6, 8) == 3 * crotchet / 2);
	// In 3/4 half a bar is one and a half crotchets.
	CHECK(CLauncher::ComputeSwitchIntervalLength(CLauncher::kLengthBeat, 3, 4) == crotchet);
	CHECK(CLauncher::ComputeSwitchIntervalLength(CLauncher::kLengthHalfBar, 3, 4) == 3 * crotchet / 2);
	CHECK(CLauncher::ComputeSwitchIntervalLength(CLauncher::kLengthFirstBars, 3, 4) == 3 * crotchet);
	// In any signature a bar is a whole number of beats and two half bars, and
	// each longer length is twice the one before, so all of them line up with
	// the bars. Half a bar need not be a whole number of beats.
	const int signatures[][2] = { { 4, 4 }, { 3, 4 }, { 6, 8 }, { 7, 8 }, { 5, 4 }, { 9, 16 } };
	for (const auto& signature : signatures) {
		const long long beat = CLauncher::ComputeSwitchIntervalLength(CLauncher::kLengthBeat, signature[0], signature[1]);
		const long long halfBar = CLauncher::ComputeSwitchIntervalLength(CLauncher::kLengthHalfBar, signature[0], signature[1]);
		const long long bar = CLauncher::ComputeSwitchIntervalLength(CLauncher::kLengthFirstBars, signature[0], signature[1]);
		CHECK(beat > 0 && bar == signature[0] * beat);
		CHECK(bar == 2 * halfBar);
		for (int length = CLauncher::kLengthFirstBars + 1; length < CLauncher::kLengthCount; ++length) {
			const long long longer = CLauncher::ComputeSwitchIntervalLength(length, signature[0], signature[1]);
			const long long shorter = CLauncher::ComputeSwitchIntervalLength(length - 1, signature[0], signature[1]);
			CHECK(longer == 2 * shorter);
		}
	}
}

void TestNewDeviceHasFourBars() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 1);
	PlayThrough(launcher, 120, static_cast<double>(kInterval) - 40);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kQueued);
	PlayThrough(launcher, static_cast<double>(kInterval) - 40, static_cast<double>(kInterval) + 40);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
}

void TestShorterLengthSwitchesSooner() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.SetLength(CLauncher::kLengthFirstBars);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 1);
	PlayThrough(launcher, 120, static_cast<double>(kBar) - 40);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kQueued);
	PlayThrough(launcher, static_cast<double>(kBar) - 40, static_cast<double>(kBar) + 40);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
}

void TestImmediateNeverQueues() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.SetLength(CLauncher::kLengthImmediate);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 1);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
	CHECK(launcher.GetCVValue(0, 0) == 1.0 / 8);
	// There is no interval to measure.
	CHECK(CountLitSceneButtons(launcher) == 0);
}

void TestChangingLengthMovesTheNextSwitchPoint() {
	// From four bars to sixteen in bar 6: the next switch point is bar 17, not 9.
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(static_cast<double>(5 * kBar)));
	launcher.SetSelection(0, 1);
	launcher.SetLength(7);
	PlayThrough(launcher, static_cast<double>(8 * kBar) - 40, static_cast<double>(8 * kBar) + 40);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kQueued);
	PlayThrough(launcher, static_cast<double>(16 * kBar) - 40, static_cast<double>(16 * kBar) + 40);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
}

void TestChangingToImmediateFiresWhatIsQueued() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 1);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kQueued);
	launcher.SetLength(CLauncher::kLengthImmediate);
	launcher.Advance(Playing(140));
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
}

void TestIntervalMeterFollowsTheLength() {
	CLauncher launcher;
	// Sixteen bars: a segment is two bars.
	launcher.SetLength(7);
	launcher.Advance(Playing(static_cast<double>(2 * kBar - 1)));
	CHECK(CountLitSceneButtons(launcher) == 1);
	launcher.Advance(Playing(static_cast<double>(2 * kBar)));
	CHECK(CountLitSceneButtons(launcher) == 2);
	// One bar: a segment is a quaver.
	launcher.SetLength(CLauncher::kLengthFirstBars);
	launcher.Advance(Playing(static_cast<double>(3 * kBar + 3 * CLauncher::kPPQPerCrotchet / 2)));
	CHECK(CountLitSceneButtons(launcher) == 4);
	// One beat: a segment is a demisemiquaver.
	launcher.SetLength(CLauncher::kLengthBeat);
	launcher.Advance(Playing(static_cast<double>(CLauncher::kPPQPerCrotchet - 1)));
	CHECK(CountLitSceneButtons(launcher) == 8);
}

void TestClipPickingShowsTheLengthOnTheSceneButtons() {
	CLauncher launcher;
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	launcher.Advance(Playing(static_cast<double>(kBar)));

	// A new device has four bars, the third scene button from the top.
	for (int scene = 1; scene <= 8; ++scene) {
		CHECK(IsLight(launcher.GetSceneLight(scene), CLauncher::kFlashing, scene == 3 ? CLauncher::kColourGreen : CLauncher::kColourWhite));
	}
	// Flashing white is light value 3, flashing green 18.
	CHECK(launcher.GetSceneLightValue(1) == 3);
	CHECK(launcher.GetSceneLightValue(3) == 18);

	// Immediate is the bottom scene button, sixteen bars the top one.
	launcher.SetLength(CLauncher::kLengthImmediate);
	CHECK(IsLight(launcher.GetSceneLight(8), CLauncher::kFlashing, CLauncher::kColourGreen));
	CHECK(IsLight(launcher.GetSceneLight(3), CLauncher::kFlashing, CLauncher::kColourWhite));
	launcher.SetLength(7);
	CHECK(IsLight(launcher.GetSceneLight(1), CLauncher::kFlashing, CLauncher::kColourGreen));
	for (int length = 0; length < CLauncher::kLengthCount; ++length) {
		CHECK(CLauncher::LengthForScene(CLauncher::SceneForLength(length)) == length);
	}
}

void TestSceneButtonsFlashWithTheLaunchButtons() {
	CLauncher launcher;
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	launcher.SetLength(7);

	// The film strip: 3 and 4 are dim and vibrant green, 7 and 8 white.
	launcher.Advance(Playing(100));
	CHECK(launcher.GetSceneLightFrame(1) == 4);
	CHECK(launcher.GetSceneLightFrame(2) == 8);
	CHECK(launcher.GetLightFrame(0, 1) == 2);
	launcher.Advance(Playing(static_cast<double>(CLauncher::kPPQPerCrotchet / 2 + 100)));
	CHECK(launcher.GetSceneLightFrame(1) == 3);
	CHECK(launcher.GetSceneLightFrame(2) == 7);
	CHECK(launcher.GetLightFrame(0, 1) == 1);
}

void TestOutOfRangeLengthIsIgnored() {
	CLauncher launcher;
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	launcher.SetLength(8);
	launcher.SetLength(-1);
	CHECK(IsLight(launcher.GetSceneLight(3), CLauncher::kFlashing, CLauncher::kColourGreen));
}

} // namespace

void TestModeColourFollowsTheSetupStep() {
	CLauncher launcher;
	CHECK(launcher.GetModeColour() == CLauncher::kModeWhite);
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	CHECK(launcher.GetModeColour() == CLauncher::kModeGreen);
	launcher.SetSetup(CLauncher::SetupForClipEditing(3, 5));
	CHECK(launcher.GetModeColour() == CLauncher::kModeYellow);
	launcher.SetSetup(CLauncher::SetupForFillInEditing(3, 5));
	CHECK(launcher.GetModeColour() == CLauncher::kModeRed);
	launcher.SetSetup(CLauncher::SetupForLandingEditing(3, 5));
	CHECK(launcher.GetModeColour() == CLauncher::kModeBlue);
	launcher.SetSetup(CLauncher::kSetupLaunchMode);
	CHECK(launcher.GetModeColour() == CLauncher::kModeWhite);
}

// Frames of the launch labels' film strip: unlit, SET/CLEAR lit, all lit.
void TestLaunchLabelsAreLitOnlyInClipEditing() {
	CLauncher launcher;
	launcher.SetClipPresent(2, 4, true);
	CHECK(launcher.GetLaunchLabelsFrame() == 0);
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	CHECK(launcher.GetLaunchLabelsFrame() == 0);

	// The labels of colours and patterns stay unlit while the picked launch
	// button is empty, as do their lights.
	launcher.SetSetup(CLauncher::SetupForClipEditing(0, 1));
	CHECK(launcher.GetLaunchLabelsFrame() == 1);
	launcher.SetSetup(CLauncher::SetupForClipEditing(2, 4));
	CHECK(launcher.GetLaunchLabelsFrame() == 2);
	launcher.SetClipPresent(2, 4, false);
	CHECK(launcher.GetLaunchLabelsFrame() == 1);

	// In fill-in editing they are red: SET/CLEAR alone until there is a
	// fill-in.
	launcher.SetClipPresent(2, 4, true);
	launcher.SetSetup(CLauncher::SetupForFillInEditing(2, 4));
	CHECK(launcher.GetLaunchLabelsFrame() == 3);
	launcher.SetFillInPresent(2, 4, true);
	CHECK(launcher.GetLaunchLabelsFrame() == 4);

	// In landing editing they are blue, likewise.
	launcher.SetSetup(CLauncher::SetupForLandingEditing(2, 4));
	CHECK(launcher.GetLaunchLabelsFrame() == 5);
	launcher.SetLandingPresent(2, 4, true);
	CHECK(launcher.GetLaunchLabelsFrame() == 6);
}

void TestBackButtonShowsTheStepItGoesBackFrom() {
	CLauncher launcher;
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonDark);
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonVibrantWhite);
	launcher.SetSetup(CLauncher::SetupForClipEditing(2, 4));
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonVibrantGreen);
	launcher.SetSetup(CLauncher::SetupForFillInEditing(2, 4));
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonVibrantYellow);
	launcher.SetSetup(CLauncher::SetupForLandingEditing(2, 4));
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonVibrantRed);
	launcher.SetSetup(CLauncher::kSetupLaunchMode);
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonDark);
}

// Frames of the back labels' film strip: all unlit, then LAUNCH, PICK CLIP,
// EDIT CLIP or SET FILL-IN lit.
void TestBackLabelHasTheColourOfTheBackButton() {
	CLauncher launcher;
	CHECK(launcher.GetBackLabelFrame() == 0);
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	CHECK(launcher.GetBackLabelFrame() == 1);
	launcher.SetSetup(CLauncher::SetupForClipEditing(2, 4));
	CHECK(launcher.GetBackLabelFrame() == 2);
	launcher.SetSetup(CLauncher::SetupForFillInEditing(2, 4));
	CHECK(launcher.GetBackLabelFrame() == 3);
	launcher.SetSetup(CLauncher::SetupForLandingEditing(2, 4));
	CHECK(launcher.GetBackLabelFrame() == 4);
	launcher.SetSetup(CLauncher::kSetupLaunchMode);
	CHECK(launcher.GetBackLabelFrame() == 0);
}

// Frames of the scene labels' film strip: unlit, white, green.
void TestSceneLabelsShowTheLength() {
	CLauncher launcher;

	// In launch mode only the label of the current length is lit, in white; a
	// new device has four bars, the third scene button from the top.
	for (int scene = 1; scene <= 8; ++scene) {
		CHECK(launcher.GetSceneLabelFrame(scene) == (scene == 3 ? 1 : 0));
	}
	launcher.SetLength(CLauncher::kLengthImmediate);
	CHECK(launcher.GetSceneLabelFrame(8) == 1);
	CHECK(launcher.GetSceneLabelFrame(3) == 0);

	// In clip picking every label is lit in green, the current length included.
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	for (int scene = 1; scene <= 8; ++scene) {
		CHECK(launcher.GetSceneLabelFrame(scene) == 2);
	}

	// In clip editing it is as in launch mode.
	launcher.SetSetup(CLauncher::SetupForClipEditing(7, 8));
	for (int scene = 1; scene <= 8; ++scene) {
		CHECK(launcher.GetSceneLabelFrame(scene) == (scene == 8 ? 1 : 0));
	}
	CHECK(launcher.GetSceneLabelFrame(0) == 0);
	CHECK(launcher.GetSceneLabelFrame(9) == 0);
}

// Track 0 plays the clip in row 1, pattern 3, which holds a fill-in with
// pattern 6 in cyan; track 1 plays a clip without a fill-in; track 2 is stopped.
void SetUpFillIns(CLauncher& iLauncher) {
	iLauncher.SetClipPresent(0, 1, true);
	iLauncher.SetClipPattern(0, 1, 0, 2);
	iLauncher.SetFillInPresent(0, 1, true);
	iLauncher.SetFillInPattern(0, 1, 0, 5);
	iLauncher.SetFillInColour(0, 1, 7);
	iLauncher.SetClipPresent(1, 1, true);
	iLauncher.SetClipPattern(1, 1, 0, 0);
	iLauncher.SetClipPresent(2, 1, true);
	iLauncher.SetSelection(0, 1);
	iLauncher.SetSelection(1, 1);
	iLauncher.Open();
}

void Press(CLauncher& iLauncher) {
	iLauncher.SetFillInPressed(true);
	iLauncher.SetFillInPressed(false);
}

void TestNewFillInHasThePatternOfItsClipInWhite() {
	CLauncher launcher;
	launcher.SetClipPresent(3, 6, true);
	launcher.SetSetup(CLauncher::SetupForFillInEditing(3, 6));
	launcher.SetFillInPresent(3, 6, true);
	// The pattern of a new clip in row 6 is 6.
	CHECK(IsLight(launcher.GetLight(5, 3), CLauncher::kVibrant, CLauncher::kColourOrange));
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::kColourWhite));
}

void TestFillInPlaysUntilTheNextSwitchPoint() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	PlayThrough(launcher, 0.0, kBar);
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);
	CHECK(!launcher.IsFillInPlaying());

	Press(launcher);
	launcher.Advance(Playing(kBar));
	CHECK(launcher.IsFillInPlaying());
	// It takes effect at once, not at the switch point.
	CHECK(launcher.GetCVValue(0, 0) == 6.0 / 8);
	// A track whose active clip has no fill-in, and a stopped track, go on as
	// before.
	CHECK(launcher.GetCVValue(1, 0) == 1.0 / 8);
	CHECK(launcher.GetCVValue(2, 0) == 0.0);

	PlayThrough(launcher, kBar + kBatch, kInterval);
	CHECK(launcher.IsFillInPlaying());
	CHECK(launcher.GetCVValue(0, 0) == 6.0 / 8);

	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(!launcher.IsFillInPlaying());
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);
}

void TestFillInGivesWayToWhatIsQueued() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	launcher.SetClipPresent(0, 2, true);
	launcher.SetClipPattern(0, 2, 0, 7);
	PlayThrough(launcher, 0.0, kBar);

	Press(launcher);
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	// The queued clip waits for the switch point; the fill-in plays meanwhile.
	CHECK(launcher.GetCVValue(0, 0) == 6.0 / 8);
	CHECK(launcher.GetButtonState(0, 2) == CLauncher::kQueued);

	PlayThrough(launcher, kBar + kBatch, kInterval + kBatch);
	CHECK(!launcher.IsFillInPlaying());
	CHECK(launcher.GetCVValue(0, 0) == 8.0 / 8);
	CHECK(launcher.GetButtonState(0, 2) == CLauncher::kActive);
}

void TestFillInDoesNotEndOneBatchAfterAPress() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	// A press in the batch after a switch point fills in the whole interval,
	// even though a selection made then still counts as landing on the switch
	// point.
	PlayThrough(launcher, kInterval - kBatch, kInterval + kBatch);
	Press(launcher);
	launcher.Advance(Playing(kInterval + kBatch));
	CHECK(launcher.IsFillInPlaying());
	launcher.Advance(Playing(kInterval + 2 * kBatch));
	CHECK(launcher.IsFillInPlaying());
}

void TestPressingDuringAFillInDoesNothing() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	Press(launcher);
	launcher.Advance(Playing(kBar));
	Press(launcher);
	PlayThrough(launcher, kBar + kBatch, kInterval + kBatch);
	// It neither cancels the fill-in nor makes another follow it.
	CHECK(!launcher.IsFillInPlaying());
}

void TestHoldingTheButtonTriggersOnlyOnce() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	launcher.SetFillInPressed(true);
	launcher.Advance(Playing(kBar));
	PlayThrough(launcher, kBar + kBatch, kInterval + kBatch);
	CHECK(!launcher.IsFillInPlaying());
	launcher.SetFillInPressed(true);
	launcher.Advance(Playing(kInterval + kBatch));
	CHECK(!launcher.IsFillInPlaying());
	launcher.SetFillInPressed(false);
	launcher.SetFillInPressed(true);
	launcher.Advance(Playing(kInterval + 2 * kBatch));
	CHECK(launcher.IsFillInPlaying());
}

void TestNoFillInWithTheLengthImmediate() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	launcher.SetLength(CLauncher::kLengthImmediate);
	Press(launcher);
	launcher.Advance(Playing(kBar));
	CHECK(!launcher.IsFillInPlaying());
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);

	// Changing to immediate ends a fill-in, as there is no switch point left to
	// end at.
	launcher.SetLength(CLauncher::kDefaultLength);
	Press(launcher);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(launcher.IsFillInPlaying());
	launcher.SetLength(CLauncher::kLengthImmediate);
	launcher.Advance(Playing(kBar + 2 * kBatch));
	CHECK(!launcher.IsFillInPlaying());
}

void TestFillInWaitsWhileTheTransportIsStopped() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	Press(launcher);
	launcher.Advance(Stopped(kBar));
	CHECK(launcher.IsFillInPlaying());
	CHECK(launcher.GetCVValue(0, 0) == 6.0 / 8);
	for (int batch = 0; batch < 1000; ++batch) {
		launcher.Advance(Stopped(kBar));
	}
	CHECK(launcher.IsFillInPlaying());
	PlayThrough(launcher, kBar, kInterval + kBatch);
	CHECK(!launcher.IsFillInPlaying());
}

void TestChangesToTheFillInTakeEffectAtOnce() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	Press(launcher);
	launcher.Advance(Playing(kBar));
	launcher.SetFillInPattern(0, 1, 0, 1);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(launcher.GetCVValue(0, 0) == 2.0 / 8);

	// Track 1 gets a fill-in while one is playing, and plays it straight away.
	launcher.SetFillInPresent(1, 1, true);
	launcher.SetFillInPattern(1, 1, 0, 4);
	launcher.Advance(Playing(kBar + 2 * kBatch));
	CHECK(launcher.GetCVValue(1, 0) == 5.0 / 8);

	launcher.SetFillInPresent(0, 1, false);
	launcher.Advance(Playing(kBar + 3 * kBatch));
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);
}

void TestEmptyLaunchButtonHasNoFillInToPlay() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	PlayThrough(launcher, 0.0, kBar);
	// Switching the active clip off stops the track at the switch point; until
	// then it plays its pattern, but no longer its fill-in.
	launcher.SetClipPresent(0, 1, false);
	Press(launcher);
	launcher.Advance(Playing(kBar));
	CHECK(launcher.IsFillInPlaying());
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);
}

void TestFillInLights() {
	CLauncher launcher;
	SetUpFillIns(launcher);
	PlayThrough(launcher, 0.0, kBar);
	CHECK(IsLight(launcher.GetFillInLight(), CLauncher::kDim, CLauncher::kColourWhite));
	CHECK(launcher.GetFillInLightValue() == 1);
	CHECK(launcher.GetFillInLightFrame() == 9);
	CHECK(launcher.GetFillInLabelFrame() == 0);
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::DefaultColour(0)));

	Press(launcher);
	launcher.Advance(Playing(kBar));
	CHECK(IsLight(launcher.GetFillInLight(), CLauncher::kVibrant, CLauncher::kColourRed));
	CHECK(launcher.GetFillInLightValue() == 5);
	CHECK(launcher.GetFillInLightFrame() == 2);
	CHECK(launcher.GetFillInLabelFrame() == 1);
	// The launch button of a track that plays its fill-in flashes in the
	// fill-in's colour; the others are as they were.
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kFlashing, 7));
	CHECK(IsLight(launcher.GetLight(1, 1), CLauncher::kVibrant, CLauncher::DefaultColour(1)));
	CHECK(IsLight(launcher.GetLight(2, 1), CLauncher::kDim, CLauncher::DefaultColour(2)));

	// The button is red whether or not any track has a fill-in, and in every
	// mode.
	launcher.SetFillInPresent(0, 1, false);
	launcher.SetSetup(CLauncher::SetupForClipEditing(4, 4));
	CHECK(IsLight(launcher.GetFillInLight(), CLauncher::kVibrant, CLauncher::kColourRed));

	PlayThrough(launcher, kBar, kInterval + kBatch);
	CHECK(IsLight(launcher.GetFillInLight(), CLauncher::kDim, CLauncher::kColourWhite));
	CHECK(launcher.GetFillInLabelFrame() == 0);
}

void TestForwardButtonIsLitWhileItLeadsSomewhere() {
	CLauncher launcher;
	launcher.SetClipPresent(2, 4, true);
	CHECK(IsUnlit(launcher.GetForwardLight()));
	CHECK(launcher.GetForwardLightValue() == 0);
	CHECK(launcher.GetForwardLightFrame() == 0);
	CHECK(launcher.GetForwardLabelFrame() == 0);
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	CHECK(IsUnlit(launcher.GetForwardLight()));
	launcher.SetSetup(CLauncher::SetupForClipEditing(0, 1));
	CHECK(IsUnlit(launcher.GetForwardLight()));

	// Red towards fill-in editing.
	launcher.SetSetup(CLauncher::SetupForClipEditing(2, 4));
	CHECK(IsLight(launcher.GetForwardLight(), CLauncher::kVibrant, CLauncher::kColourRed));
	CHECK(launcher.GetForwardLightValue() == 5);
	CHECK(launcher.GetForwardLightFrame() == 2);
	CHECK(launcher.GetForwardLabelFrame() == 1);

	// Blue towards landing editing, whether or not there is a fill-in.
	launcher.SetSetup(CLauncher::SetupForFillInEditing(2, 4));
	CHECK(IsLight(launcher.GetForwardLight(), CLauncher::kVibrant, CLauncher::kColourBlue));
	CHECK(launcher.GetForwardLightValue() == 29);
	CHECK(launcher.GetForwardLightFrame() == 10);
	CHECK(launcher.GetForwardLabelFrame() == 2);
	launcher.SetSetup(CLauncher::SetupForFillInEditing(0, 1));
	CHECK(IsUnlit(launcher.GetForwardLight()));

	// Landing editing is the last step.
	launcher.SetSetup(CLauncher::SetupForLandingEditing(2, 4));
	CHECK(IsUnlit(launcher.GetForwardLight()));
	CHECK(launcher.GetForwardLabelFrame() == 0);
}

void TestFillInEditingWithoutFillInShowsOnlyToggle() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.SetSetup(CLauncher::SetupForFillInEditing(3, 6));
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		for (int row = 1; row <= CLauncher::kRowCount; ++row) {
			if (track == 7 && row == 1) {
				CHECK(IsLight(launcher.GetLight(track, row), CLauncher::kDim, CLauncher::kColourRed));
			}
			else {
				CHECK(IsUnlit(launcher.GetLight(track, row)));
			}
		}
	}
}

void TestFillInEditingShowsColoursAndPatterns() {
	CLauncher launcher;
	launcher.SetClipPresent(3, 6, true);
	launcher.SetClipColour(3, 6, 2);
	launcher.SetClipPattern(3, 6, 0, 4);
	launcher.SetClipUnipolar(3, 6, 1, 6);
	launcher.SetFillInPresent(3, 6, true);
	launcher.SetFillInColour(3, 6, 9);
	launcher.SetFillInPattern(3, 6, 0, 2);
	launcher.SetFillInUnipolar(3, 6, 1, 1);
	launcher.SetSetup(CLauncher::SetupForFillInEditing(3, 6));

	CHECK(IsLight(launcher.GetLight(7, 1), CLauncher::kVibrant, CLauncher::kColourGreen));
	for (int column = 0; column < 7; ++column) {
		CHECK(IsLight(launcher.GetLight(column, 1), CLauncher::kDim, column));
		const int colour = 7 + column;
		CHECK(IsLight(launcher.GetLight(column, 2), colour == 9 ? CLauncher::kVibrant : CLauncher::kDim, colour));
	}
	// The fill-in's settings, not the clip's.
	for (int column = 0; column < 8; ++column) {
		CHECK(IsLight(launcher.GetLight(column, 3), column == 2 ? CLauncher::kVibrant : CLauncher::kDim, CLauncher::kColourOrange));
	}
	CHECK(IsLight(launcher.GetLight(0, 4), CLauncher::kDim, CLauncher::kColourCyan));
	CHECK(IsLight(launcher.GetLight(1, 4), CLauncher::kVibrant, CLauncher::kColourCyan));
	CHECK(IsUnlit(launcher.GetLight(2, 4)));
}

void TestIntervalMeterRunsInFillInEditing() {
	CLauncher launcher;
	launcher.SetSetup(CLauncher::SetupForFillInEditing(1, 1));
	launcher.Advance(Playing(0.0));
	CHECK(IsLight(launcher.GetSceneLight(8), CLauncher::kDim, CLauncher::kColourGreen));
	CHECK(launcher.GetSceneLabelFrame(3) == 1);
}

// Tracks 0, 1 and 2 play clips with patterns 3, 1 and 5; track 3 holds a
// clip but is stopped; track 4 has no clips. The song plays up to bar 2.
void SetUpMutesAndSolos(CLauncher& iLauncher) {
	iLauncher.SetClipPresent(0, 1, true);
	iLauncher.SetClipPattern(0, 1, 0, 2);
	iLauncher.SetClipPresent(1, 1, true);
	iLauncher.SetClipPattern(1, 1, 0, 0);
	iLauncher.SetClipPresent(2, 1, true);
	iLauncher.SetClipPattern(2, 1, 0, 4);
	iLauncher.SetClipPresent(3, 1, true);
	for (int track = 0; track < 3; ++track) {
		iLauncher.SetSelection(track, 1);
	}
	iLauncher.Open();
	PlayThrough(iLauncher, 0.0, kBar);
}

void PressMute(CLauncher& iLauncher, int iTrack) {
	iLauncher.SetMutePressed(iTrack, true);
	iLauncher.SetMutePressed(iTrack, false);
}

void PressSolo(CLauncher& iLauncher, int iTrack) {
	iLauncher.SetSoloPressed(iTrack, true);
	iLauncher.SetSoloPressed(iTrack, false);
}

void TestNewDeviceHasTheStopFunction() {
	CLauncher launcher;
	CHECK(launcher.GetFunction() == CLauncher::kFunctionStop);
	CHECK(IsLight(launcher.GetFunctionSelectLight(CLauncher::kFunctionStop), CLauncher::kVibrant, CLauncher::kColourRed));
	CHECK(IsLight(launcher.GetFunctionSelectLight(CLauncher::kFunctionMute), CLauncher::kDim, CLauncher::kColourWhite));
	CHECK(IsLight(launcher.GetFunctionSelectLight(CLauncher::kFunctionSolo), CLauncher::kDim, CLauncher::kColourWhite));
	// Vibrant red, and dim white, in the side buttons' film strip.
	CHECK(launcher.GetFunctionSelectLightFrame(CLauncher::kFunctionStop) == 2);
	CHECK(launcher.GetFunctionSelectLightFrame(CLauncher::kFunctionMute) == 9);
	CHECK(launcher.GetFunctionSelectCaptionFrame(CLauncher::kFunctionStop) == 1);
	CHECK(launcher.GetFunctionSelectCaptionFrame(CLauncher::kFunctionMute) == 0);
	CHECK(launcher.GetFunctionSelectCaptionFrame(CLauncher::kFunctionSolo) == 0);
}

void TestFunctionSelectButtonsWorkLikeRadioButtons() {
	CLauncher launcher;
	launcher.SetFunction(CLauncher::kFunctionMute);
	CHECK(launcher.GetFunction() == CLauncher::kFunctionMute);
	CHECK(launcher.GetFunctionSelectLightFrame(CLauncher::kFunctionMute) == 6);
	CHECK(launcher.GetFunctionSelectLightFrame(CLauncher::kFunctionStop) == 9);
	CHECK(launcher.GetFunctionSelectCaptionFrame(CLauncher::kFunctionMute) == 1);
	CHECK(launcher.GetFunctionSelectCaptionFrame(CLauncher::kFunctionStop) == 0);

	launcher.SetFunction(CLauncher::kFunctionSolo);
	CHECK(IsLight(launcher.GetFunctionSelectLight(CLauncher::kFunctionSolo), CLauncher::kVibrant, CLauncher::kColourBlue));
	CHECK(launcher.GetFunctionSelectLightFrame(CLauncher::kFunctionSolo) == 8);
	CHECK(launcher.GetFunctionSelectLightFrame(CLauncher::kFunctionMute) == 9);

	launcher.SetFunction(3);
	launcher.SetFunction(-1);
	CHECK(launcher.GetFunction() == CLauncher::kFunctionSolo);
}

void TestMuteSilencesATrackAtOnceUntilTheNextSwitchPoint() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	PressMute(launcher, 0);
	launcher.Advance(Playing(kBar));
	CHECK(launcher.IsMuted(0));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
	CHECK(launcher.GetCVValue(1, 0) == 1.0 / 8);
	// The launch buttons go on as before.
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::DefaultColour(0)));

	PlayThrough(launcher, kBar + kBatch, kInterval - kBatch / 2);
	CHECK(launcher.IsMuted(0));
	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(!launcher.IsMuted(0));
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);
}

void TestSecondPressUnmutes() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	PressMute(launcher, 0);
	launcher.Advance(Playing(kBar));
	PressMute(launcher, 0);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(!launcher.IsMuted(0));
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);
}

void TestSeveralTracksCanBeMuted() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	PressMute(launcher, 0);
	PressMute(launcher, 2);
	launcher.Advance(Playing(kBar));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
	CHECK(launcher.GetCVValue(1, 0) == 1.0 / 8);
	CHECK(launcher.GetCVValue(2, 0) == 0.0);
}

void TestHoldingAMuteButtonTogglesOnlyOnce() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	launcher.SetMutePressed(0, true);
	launcher.Advance(Playing(kBar));
	launcher.SetMutePressed(0, true);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(launcher.IsMuted(0));
	launcher.SetMutePressed(0, false);
	launcher.Advance(Playing(kBar + 2 * kBatch));
	CHECK(launcher.IsMuted(0));
}

void TestMuteInTheBatchAfterASwitchPointLastsTheWholeInterval() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	PlayThrough(launcher, kBar, kInterval + kBatch);
	PressMute(launcher, 0);
	launcher.Advance(Playing(kInterval + kBatch));
	launcher.Advance(Playing(kInterval + 2 * kBatch));
	CHECK(launcher.IsMuted(0));
}

void TestMuteLastsWithTheLengthImmediate() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	launcher.SetLength(CLauncher::kLengthImmediate);
	PressMute(launcher, 0);
	launcher.Advance(Playing(kBar));
	PlayThrough(launcher, kBar + kBatch, 3 * kInterval);
	CHECK(launcher.IsMuted(0));
	// A clip launched meanwhile is muted as well.
	launcher.SetSelection(0, CLauncher::kSelectionStop);
	launcher.Advance(Playing(3 * kInterval));
	launcher.SetSelection(0, 1);
	launcher.Advance(Playing(3 * kInterval + kBatch));
	CHECK(launcher.IsMuted(0));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
}

void TestMuteWaitsWhileTheTransportIsStopped() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	PressMute(launcher, 0);
	launcher.Advance(Stopped(kBar));
	for (int batch = 0; batch < 1000; ++batch) {
		launcher.Advance(Stopped(kBar));
	}
	CHECK(launcher.IsMuted(0));
	// Playing on from where the transport stopped is not a jump.
	PlayThrough(launcher, kBar, kInterval - kBatch);
	CHECK(launcher.IsMuted(0));
	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(!launcher.IsMuted(0));
}

void TestMutesAndSolosEndWhenTheSongPositionJumps() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	launcher.SetLength(CLauncher::kLengthImmediate);
	PressMute(launcher, 0);
	PressSolo(launcher, 1);
	launcher.Advance(Playing(kBar));
	PlayThrough(launcher, kBar + kBatch, 2 * kBar);
	CHECK(launcher.IsMuted(0));
	CHECK(launcher.IsSoloed(1));
	// A loop goes back to bar 2.
	launcher.Advance(Playing(kBar));
	CHECK(!launcher.IsMuted(0));
	CHECK(!launcher.IsSoloed(1));

	// So does a locate while the transport is stopped.
	PressMute(launcher, 0);
	launcher.Advance(Stopped(kBar + kBatch));
	launcher.Advance(Stopped(kBar + kBatch));
	CHECK(launcher.IsMuted(0));
	launcher.Advance(Stopped(3 * kBar));
	CHECK(!launcher.IsMuted(0));
}

void TestMuteAndSoloNeedClips() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	PressMute(launcher, 4);
	PressSolo(launcher, 4);
	launcher.Advance(Playing(kBar));
	CHECK(!launcher.IsMuted(4));
	CHECK(!launcher.IsSoloed(4));
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);

	// A stopped track that holds a clip can be soloed, and silences the others.
	PressSolo(launcher, 3);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(launcher.IsSoloed(3));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);

	// A track that loses its last clip is neither muted nor soloed any more.
	PressMute(launcher, 3);
	launcher.Advance(Playing(kBar + 2 * kBatch));
	CHECK(launcher.IsMuted(3));
	launcher.SetClipPresent(3, 1, false);
	launcher.Advance(Playing(kBar + 3 * kBatch));
	CHECK(!launcher.IsMuted(3));
	CHECK(!launcher.IsSoloed(3));
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);
}

void TestSoloSilencesTheOtherTracksUntilTheNextSwitchPoint() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	PressSolo(launcher, 1);
	launcher.Advance(Playing(kBar));
	CHECK(launcher.IsSoloed(1));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
	CHECK(launcher.GetCVValue(1, 0) == 1.0 / 8);
	CHECK(launcher.GetCVValue(2, 0) == 0.0);
	CHECK(launcher.IsSilenced(0));
	CHECK(!launcher.IsSilenced(1));

	// Several tracks can be soloed.
	PressSolo(launcher, 2);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
	CHECK(launcher.GetCVValue(2, 0) == 5.0 / 8);

	PlayThrough(launcher, kBar + 2 * kBatch, kInterval + kBatch);
	CHECK(!launcher.IsSoloed(1));
	CHECK(!launcher.IsSoloed(2));
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);
}

void TestSoloAndMuteCombine() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	PressMute(launcher, 0);
	PressMute(launcher, 1);
	launcher.Advance(Playing(kBar));

	// A muted track that is soloed plays at once.
	PressSolo(launcher, 0);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(launcher.IsMuted(0));
	CHECK(launcher.GetCVValue(0, 0) == 3.0 / 8);
	CHECK(launcher.GetCVValue(2, 0) == 0.0);

	// Switching the solo off leaves every track as it was: tracks 0 and 1
	// muted, track 2 playing.
	PressSolo(launcher, 0);
	launcher.Advance(Playing(kBar + 2 * kBatch));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
	CHECK(launcher.GetCVValue(1, 0) == 0.0);
	CHECK(launcher.GetCVValue(2, 0) == 5.0 / 8);
}

void TestMutedTrackPlaysNoFillIn() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);
	launcher.SetFillInPresent(0, 1, true);
	launcher.SetFillInPattern(0, 1, 0, 5);
	PressMute(launcher, 0);
	launcher.SetFillInPressed(true);
	launcher.Advance(Playing(kBar));
	CHECK(launcher.IsFillInPlaying());
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
}

void TestFunctionLights() {
	CLauncher launcher;
	SetUpMutesAndSolos(launcher);

	// In stop function, the stop buttons' lights in red.
	CHECK(IsLight(launcher.GetFunctionLight(0), CLauncher::kVibrant, CLauncher::kColourRed));
	CHECK(IsLight(launcher.GetFunctionLight(3), CLauncher::kDim, CLauncher::kColourRed));
	CHECK(IsUnlit(launcher.GetFunctionLight(4)));
	CHECK(launcher.GetFunctionLightValue(0) == 2);

	// In mute function, vibrant yellow, dim while muted, unlit without clips.
	PressMute(launcher, 1);
	launcher.SetFunction(CLauncher::kFunctionMute);
	launcher.Advance(Playing(kBar));
	CHECK(IsLight(launcher.GetFunctionLight(0), CLauncher::kVibrant, CLauncher::kColourYellow));
	CHECK(IsLight(launcher.GetFunctionLight(1), CLauncher::kDim, CLauncher::kColourYellow));
	CHECK(IsLight(launcher.GetFunctionLight(3), CLauncher::kVibrant, CLauncher::kColourYellow));
	CHECK(IsUnlit(launcher.GetFunctionLight(4)));
	CHECK(launcher.GetFunctionLightFrame(0) == 4);
	CHECK(launcher.GetFunctionLightFrame(1) == 3);
	CHECK(launcher.GetFunctionLightFrame(4) == 0);
	CHECK(launcher.GetFunctionLightValue(0) == 2);
	CHECK(launcher.GetFunctionLightValue(1) == 1);

	// In solo function, dim blue, vibrant while soloed. A muted track that is
	// soloed still shows dim yellow in mute function.
	PressSolo(launcher, 1);
	launcher.SetFunction(CLauncher::kFunctionSolo);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(IsLight(launcher.GetFunctionLight(0), CLauncher::kDim, CLauncher::kColourBlue));
	CHECK(IsLight(launcher.GetFunctionLight(1), CLauncher::kVibrant, CLauncher::kColourBlue));
	CHECK(IsUnlit(launcher.GetFunctionLight(4)));
	CHECK(launcher.GetFunctionLightFrame(0) == 5);
	CHECK(launcher.GetFunctionLightFrame(1) == 6);
	launcher.SetFunction(CLauncher::kFunctionMute);
	CHECK(IsLight(launcher.GetFunctionLight(1), CLauncher::kDim, CLauncher::kColourYellow));
}

// Tests of the CV outputs in unipolar and bipolar mode, and their
// transitions. Output B, the second, is in unipolar mode on a new device.

bool Near(double iActual, double iExpected) {
	return std::fabs(iActual - iExpected) < 1e-9;
}

const int kOutputB = 1;

// Track 1 plays the clip in row 1 at unipolar value iFrom, with transition
// iTransition on output B; the clips in rows 2 and 3 have the values 6 and 0.
void SetUpTransition(CLauncher& iLauncher, int iFrom, int iTransition) {
	FillGrid(iLauncher);
	iLauncher.SetClipUnipolar(0, 1, kOutputB, iFrom);
	iLauncher.SetClipTransition(0, 1, kOutputB, iTransition);
	iLauncher.SetClipUnipolar(0, 2, kOutputB, 6);
	OpenWith(iLauncher, 0, 1);
}

void TestCVModesOfANewDevice() {
	CLauncher launcher;
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		CHECK(launcher.GetCVMode(track, 0) == CLauncher::kCVModePattern);
		for (int output = 1; output < CLauncher::kOutputCount; ++output) {
			CHECK(launcher.GetCVMode(track, output) == CLauncher::kCVModeUnipolar);
			CHECK(launcher.GetCVValue(track, output) == 0.0);
		}
	}
}

void TestUnipolarAndBipolarSteps() {
	CHECK(CLauncher::UnipolarCV(0) == 0.0);
	CHECK(Near(CLauncher::UnipolarCV(1), 1.0 / 6));
	CHECK(CLauncher::UnipolarCV(6) == 1.0);
	CHECK(CLauncher::BipolarCV(0) == -1.0);
	CHECK(CLauncher::BipolarCV(3) == 0.0);
	CHECK(Near(CLauncher::BipolarCV(4), 1.0 / 3));
	CHECK(CLauncher::BipolarCV(6) == 1.0);
}

void TestOutputsEmitTheValueOfTheirCVMode() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.SetClipPattern(0, 1, kOutputB, 4);
	launcher.SetClipUnipolar(0, 1, kOutputB, 3);
	launcher.SetClipBipolar(0, 1, kOutputB, 1);
	OpenWith(launcher, 0, 1);
	launcher.Advance(Stopped());
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	// Switching the CV mode takes effect at once, and loses nothing.
	launcher.SetCVMode(0, kOutputB, CLauncher::kCVModeBipolar);
	launcher.Advance(Stopped());
	CHECK(Near(launcher.GetCVValue(0, kOutputB), -2.0 / 3));
	launcher.SetCVMode(0, kOutputB, CLauncher::kCVModePattern);
	launcher.Advance(Stopped());
	CHECK(launcher.GetCVValue(0, kOutputB) == 5.0 / 8);
	launcher.SetCVMode(0, kOutputB, CLauncher::kCVModeUnipolar);
	launcher.Advance(Stopped());
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
}

void TestEditingAValueTakesEffectAtOnce() {
	CLauncher launcher;
	SetUpTransition(launcher, 2, CLauncher::kTransitionOff);
	launcher.Advance(Playing(kBar));
	launcher.SetClipUnipolar(0, 1, kOutputB, 5);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 5.0 / 6));
}

void TestStoppedTrackHoldsItsValue() {
	CLauncher launcher;
	SetUpTransition(launcher, 4, CLauncher::kTransitionA);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, CLauncher::kSelectionStop);
	// A queued stop never moves.
	launcher.Advance(Playing(kBar * 3));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 4.0 / 6));
	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kActive);
	// Output A, in pattern mode, emits 0; output B goes on with its value.
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 4.0 / 6));
}

void TestMuteAndSoloLeaveUnipolarAndBipolarOutputsAlone() {
	CLauncher launcher;
	SetUpTransition(launcher, 4, CLauncher::kTransitionOff);
	launcher.SetCVMode(0, 2, CLauncher::kCVModeBipolar);
	launcher.SetMutePressed(0, true);
	launcher.Advance(Playing(kBar));
	CHECK(launcher.IsMuted(0));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 4.0 / 6));
	CHECK(launcher.GetCVValue(0, 2) == 0.0);
	launcher.SetClipBipolar(0, 1, 2, 6);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(launcher.GetCVValue(0, 2) == 1.0);
}

void TestWithoutTransitionTheValueJumpsAtTheSwitchPoint() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionOff);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar * 3));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0);
}

void TestTransitionAMovesInAStraightLine() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionA);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	// From the moment of queueing to the switch point, three bars later.
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	launcher.Advance(Playing(kBar * 3.25));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.75));
	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0);
}

void TestTheIncomingClipsTransitionDoesNotCount() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionOff);
	launcher.SetClipTransition(0, 2, kOutputB, CLauncher::kTransitionA);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
}

void TestTransitionBFollowsAnExponentialCurve() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionB);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), (std::exp(2.0) - 1.0) / (std::exp(4.0) - 1.0)));
	CHECK(Near(CLauncher::TransitionCurve(CLauncher::kTransitionD, 1.0), 1.0));
	CHECK(CLauncher::TransitionCurve(CLauncher::kTransitionB, 0.5) < 0.5);
}

void TestTransitionCMovesOnlyInTheLastBar() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionC);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	launcher.Advance(Playing(kBar * 3.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	// Queued within the last bar, it moves from then on.
	CLauncher late;
	SetUpTransition(late, 0, CLauncher::kTransitionC);
	late.Advance(Playing(kBar * 3.5));
	late.SetSelection(0, 2);
	late.Advance(Playing(kBar * 3.5));
	late.Advance(Playing(kBar * 3.75));
	CHECK(Near(late.GetCVValue(0, kOutputB), 0.5));
}

void TestTransitionCWithOneBarIsLikeA() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionC);
	launcher.SetLength(CLauncher::kLengthFirstBars);
	launcher.Advance(Playing(kBar + kBar / 4));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar + kBar / 4));
	launcher.Advance(Playing(kBar + kBar * 5 / 8));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
}

void TestNoTransitionWithTheLengthImmediate() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionA);
	launcher.SetLength(CLauncher::kLengthImmediate);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0);
}

void TestQueueingAnotherClipStartsOverFromWhereItIs() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionA);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	// Row 3 is at 0: from 0.5 down to 0 over the remaining bar and a half.
	launcher.SetSelection(0, 3);
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	launcher.Advance(Playing(kBar * 3.25));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.25));
}

void TestEditingTheQueuedValueStartsOver() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionA);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	launcher.Advance(Playing(kBar * 2.5));
	launcher.SetClipUnipolar(0, 2, kOutputB, 0);
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	launcher.Advance(Playing(kBar * 3.25));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.25));
}

void TestCancellingJumpsBack() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionA);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	launcher.Advance(Playing(kBar * 2.5));
	// Choosing the active clip again.
	launcher.SetSelection(0, 1);
	launcher.Advance(Playing(kBar * 2.5 + kBatch));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	// Queueing stop.
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar * 3));
	launcher.Advance(Playing(kBar * 3.5));
	CHECK(launcher.GetCVValue(0, kOutputB) > 0.0);
	launcher.SetSelection(0, CLauncher::kSelectionStop);
	launcher.Advance(Playing(kBar * 3.5 + kBatch));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	// Switching the transition off.
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar * 3.5 + kBatch * 2));
	launcher.Advance(Playing(kBar * 3.75));
	CHECK(launcher.GetCVValue(0, kOutputB) > 0.0);
	launcher.SetClipTransition(0, 1, kOutputB, CLauncher::kTransitionOff);
	launcher.Advance(Playing(kBar * 3.75 + kBatch));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
}

void TestChangingTheCVModeDropsTheTransition() {
	CLauncher launcher;
	SetUpTransition(launcher, 0, CLauncher::kTransitionA);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	launcher.Advance(Playing(kBar * 2.5));
	// In bipolar mode both clips start at 0; the movement starts again from
	// the active clip's value.
	launcher.SetClipBipolar(0, 2, kOutputB, 6);
	launcher.SetCVMode(0, kOutputB, CLauncher::kCVModeBipolar);
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	launcher.Advance(Playing(kBar * 3.25));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
}

// The clip in row 1 of track 1 is at unipolar 0 on output B, its fill-in at
// 6 with transition iTransition.
void SetUpRiser(CLauncher& iLauncher, int iTransition) {
	SetUpTransition(iLauncher, 0, CLauncher::kTransitionOff);
	iLauncher.SetFillInPresent(0, 1, true);
	iLauncher.SetFillInUnipolar(0, 1, kOutputB, 6);
	iLauncher.SetFillInTransition(0, 1, kOutputB, iTransition);
}

void TestFillInWithoutTransitionJumpsAtThePress() {
	CLauncher launcher;
	SetUpRiser(launcher, CLauncher::kTransitionOff);
	launcher.Advance(Playing(kBar));
	Press(launcher);
	launcher.Advance(Playing(kBar * 2));
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0);
	// At the switch point it jumps back to the clip.
	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
}

void TestFillInTransitionRisesToTheSwitchPoint() {
	CLauncher launcher;
	SetUpRiser(launcher, CLauncher::kTransitionA);
	launcher.Advance(Playing(kBar));
	Press(launcher);
	launcher.Advance(Playing(kBar * 2));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	launcher.Advance(Playing(kBar * 3));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	// It jumps to whatever was queued meanwhile, without a transition.
	launcher.SetSelection(0, 3);
	launcher.SetClipUnipolar(0, 3, kOutputB, 3);
	launcher.Advance(Playing(kBar * 3.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.75));
	launcher.Advance(Playing(kInterval - kBatch / 2));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
}

void TestFillInTransitionDWaitsForTheLastBar() {
	CLauncher launcher;
	SetUpRiser(launcher, CLauncher::kTransitionD);
	launcher.Advance(Playing(kBar));
	Press(launcher);
	launcher.Advance(Playing(kBar * 2));
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	launcher.Advance(Playing(kBar * 3.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), CLauncher::TransitionCurve(CLauncher::kTransitionD, 0.5)));
}

void TestFillInHidesTheClipsTransition() {
	CLauncher launcher;
	SetUpRiser(launcher, CLauncher::kTransitionA);
	launcher.SetClipTransition(0, 1, kOutputB, CLauncher::kTransitionA);
	launcher.SetFillInUnipolar(0, 1, kOutputB, 0);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	Press(launcher);
	launcher.Advance(Playing(kBar * 2));
	launcher.Advance(Playing(kBar * 3));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	// Cleared, the fill-in shows the clip's transition underneath, half way
	// from the second bar, when the queue was seen, to the switch point.
	launcher.SetFillInPresent(0, 1, false);
	launcher.Advance(Playing(kBar * 3));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
}

void TestFillInStartsFromWhatIsEmitted() {
	CLauncher launcher;
	SetUpRiser(launcher, CLauncher::kTransitionA);
	launcher.SetClipTransition(0, 1, kOutputB, CLauncher::kTransitionA);
	launcher.SetFillInUnipolar(0, 1, kOutputB, 0);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	Press(launcher);
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	launcher.Advance(Playing(kBar * 3.25));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.25));
}

void TestChangedPatternOnAnyOutputFlashes() {
	CLauncher launcher;
	FillGrid(launcher);
	launcher.SetCVMode(0, kOutputB, CLauncher::kCVModePattern);
	OpenWith(launcher, 0, 1);
	launcher.Advance(Playing(kBar));
	CHECK(launcher.GetLight(0, 1).fBrightness == CLauncher::kVibrant);
	launcher.SetClipPattern(0, 1, kOutputB, 5);
	launcher.Advance(Playing(kBar + kBatch));
	CHECK(launcher.GetLight(0, 1).fBrightness == CLauncher::kFlashing);
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0 / 8);
	// A pattern on an output in another CV mode makes no difference.
	launcher.SetClipPattern(0, 1, kOutputB, 0);
	launcher.SetClipPattern(0, 1, 2, 5);
	launcher.Advance(Playing(kBar + kBatch * 2));
	CHECK(launcher.GetLight(0, 1).fBrightness == CLauncher::kVibrant);
}

void TestOutputLabelsFollowTheCVModeAndTransition() {
	CLauncher launcher;
	launcher.SetClipPresent(0, 1, true);
	launcher.SetCVMode(0, 2, CLauncher::kCVModeBipolar);
	launcher.SetClipTransition(0, 1, kOutputB, CLauncher::kTransitionC);
	launcher.SetClipTransition(0, 1, 2, CLauncher::kTransitionA);
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		CHECK(launcher.GetOutputLabelsFrame(output) == 0);
	}
	CHECK(launcher.GetTransitionDiagramsFrame() == 0);

	// Patterns, then unipolar values with each transition, then bipolar ones.
	launcher.SetSetup(CLauncher::SetupForClipEditing(0, 1));
	CHECK(launcher.GetOutputLabelsFrame(0) == 1);
	CHECK(launcher.GetOutputLabelsFrame(kOutputB) == 2 + CLauncher::kTransitionC);
	CHECK(launcher.GetOutputLabelsFrame(2) == 7 + CLauncher::kTransitionA);
	CHECK(launcher.GetOutputLabelsFrame(3) == 2);
	CHECK(launcher.GetTransitionDiagramsFrame() == 1);

	// In fill-in editing in red, and only while there is a fill-in.
	launcher.SetSetup(CLauncher::SetupForFillInEditing(0, 1));
	CHECK(launcher.GetOutputLabelsFrame(0) == 0);
	CHECK(launcher.GetTransitionDiagramsFrame() == 0);
	launcher.SetFillInPresent(0, 1, true);
	CHECK(launcher.GetOutputLabelsFrame(0) == 12);
	CHECK(launcher.GetOutputLabelsFrame(2) == 18);
	CHECK(launcher.GetOutputLabelsFrame(5) == 13);
	CHECK(launcher.GetTransitionDiagramsFrame() == 2);

	// In landing editing in blue, and only while there is a landing.
	launcher.SetSetup(CLauncher::SetupForLandingEditing(0, 1));
	CHECK(launcher.GetOutputLabelsFrame(0) == 0);
	CHECK(launcher.GetTransitionDiagramsFrame() == 0);
	launcher.SetLandingPresent(0, 1, true);
	launcher.SetLandingTransition(0, 1, kOutputB, CLauncher::kTransitionD);
	CHECK(launcher.GetOutputLabelsFrame(0) == 23);
	CHECK(launcher.GetOutputLabelsFrame(kOutputB) == 24 + CLauncher::kTransitionD);
	CHECK(launcher.GetOutputLabelsFrame(2) == 29);
	CHECK(launcher.GetTransitionDiagramsFrame() == 3);
	CHECK(CLauncher::kOutputLabelsFrameCount == 34);

	// Without a clip they are unlit.
	launcher.SetSetup(CLauncher::SetupForClipEditing(1, 1));
	CHECK(launcher.GetOutputLabelsFrame(0) == 0);
	CHECK(launcher.GetTransitionDiagramsFrame() == 0);
}

// Tests of landings. Track 1 plays the clip in row 1 at unipolar 0 on output
// B; the clip in row 2 is at 6 and holds a landing at 0 with transition
// iTransition, pattern 8 on output A and colour 7.
void SetUpLanding(CLauncher& iLauncher, int iTransition) {
	SetUpTransition(iLauncher, 0, CLauncher::kTransitionOff);
	iLauncher.SetLandingPresent(0, 2, true);
	iLauncher.SetLandingUnipolar(0, 2, kOutputB, 0);
	iLauncher.SetLandingTransition(0, 2, kOutputB, iTransition);
	iLauncher.SetLandingPattern(0, 2, 0, 7);
	iLauncher.SetLandingColour(0, 2, 7);
}

// Queues row 2 in the first bar and plays on up to the switch point, at which
// it becomes active; a landing starts there.
void LandOnRow2(CLauncher& iLauncher) {
	PlayThrough(iLauncher, 0.0, kBar);
	iLauncher.SetSelection(0, 2);
	PlayThrough(iLauncher, kBar, kInterval + kBatch);
}

// Plays on from the batch after iFrom up to iTo, which is the last batch.
void PlayOnTo(CLauncher& iLauncher, double iFrom, double iTo) {
	PlayThrough(iLauncher, iFrom + kBatch, iTo);
	iLauncher.Advance(Playing(iTo));
}

void TestNewLandingHasThePatternOfItsClipInWhite() {
	CLauncher launcher;
	launcher.SetClipPresent(3, 6, true);
	launcher.SetSetup(CLauncher::SetupForLandingEditing(3, 6));
	launcher.SetLandingPresent(3, 6, true);
	CHECK(IsLight(launcher.GetLight(5, 3), CLauncher::kVibrant, CLauncher::kColourOrange));
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::kColourWhite));
}

void TestLandingPlaysForTheFirstSwitchIntervalOnly() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	PlayThrough(launcher, 0.0, kBar);
	launcher.SetSelection(0, 2);
	PlayThrough(launcher, kBar, kInterval);
	CHECK(!launcher.PlaysLanding(0));

	// At the switch point the landing's pattern, value and colour take over.
	launcher.Advance(Playing(kInterval));
	CHECK(launcher.PlaysLanding(0));
	CHECK(launcher.GetCVValue(0, 0) == 8.0 / 8);
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	CHECK(IsLight(launcher.GetLight(0, 2), CLauncher::kVibrant, 7));

	// Without a transition it holds the landing's value to the next switch
	// point, then jumps to the clip's.
	PlayThrough(launcher, kInterval + kBatch, 2 * kInterval);
	CHECK(launcher.PlaysLanding(0));
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	launcher.Advance(Playing(2 * kInterval));
	CHECK(!launcher.PlaysLanding(0));
	CHECK(launcher.GetCVValue(0, 0) == 2.0 / 8);
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0);
	CHECK(IsLight(launcher.GetLight(0, 2), CLauncher::kVibrant, CLauncher::DefaultColour(0)));
}

void TestLandingPlaysAfterStop() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	OpenWith(launcher, 0, CLauncher::kSelectionStop);
	LandOnRow2(launcher);
	CHECK(launcher.PlaysLanding(0));
	CHECK(launcher.GetCVValue(0, 0) == 8.0 / 8);
}

void TestEveryTrackOfASceneLandsOnItsOwn() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	PlayThrough(launcher, 0.0, kBar);
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		launcher.SetSelection(track, 2);
	}
	PlayThrough(launcher, kBar, kInterval + kBatch);
	CHECK(launcher.PlaysLanding(0));
	CHECK(!launcher.PlaysLanding(1));
	CHECK(launcher.GetButtonState(1, 2) == CLauncher::kActive);
}

void TestNoLandingWithoutASwitchPoint() {
	// Opening a song.
	CLauncher opened;
	SetUpLanding(opened, CLauncher::kTransitionOff);
	OpenWith(opened, 0, 2);
	opened.Advance(Playing(0.0));
	CHECK(!opened.PlaysLanding(0));

	// Choosing while the transport is stopped, even if it starts again.
	CLauncher stopped;
	SetUpLanding(stopped, CLauncher::kTransitionOff);
	stopped.SetSelection(0, 2);
	stopped.Advance(Stopped(kBar));
	CHECK(stopped.GetButtonState(0, 2) == CLauncher::kActive);
	CHECK(!stopped.PlaysLanding(0));
	PlayOnTo(stopped, kBar, kBar * 2);
	CHECK(!stopped.PlaysLanding(0));

	// With the length immediate.
	CLauncher immediate;
	SetUpLanding(immediate, CLauncher::kTransitionOff);
	immediate.SetLength(CLauncher::kLengthImmediate);
	immediate.Advance(Playing(kBar));
	immediate.SetSelection(0, 2);
	immediate.Advance(Playing(kBar + kBatch));
	CHECK(immediate.GetButtonState(0, 2) == CLauncher::kActive);
	CHECK(!immediate.PlaysLanding(0));
	CHECK(immediate.GetCVValue(0, kOutputB) == 1.0);
}

void TestChoosingTheActiveClipAgainDoesNotLand() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	LandOnRow2(launcher);
	PlayThrough(launcher, kInterval + kBatch, kInterval + kBar);
	// Another clip queued and the active one chosen again before the switch
	// point: the clip never stops, so it does not land again.
	launcher.SetSelection(0, 1);
	PlayThrough(launcher, kInterval + kBar, kInterval + 2 * kBar);
	launcher.SetSelection(0, 2);
	PlayThrough(launcher, kInterval + 2 * kBar, 2 * kInterval + kBatch);
	CHECK(launcher.GetButtonState(0, 2) == CLauncher::kActive);
	CHECK(!launcher.PlaysLanding(0));
}

void TestJumpingEndsALanding() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	LandOnRow2(launcher);
	launcher.Advance(Playing(kInterval + 2 * kBar));
	CHECK(!launcher.PlaysLanding(0));
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0);
}

void TestLandingTransitionAMovesInAStraightLine() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionA);
	LandOnRow2(launcher);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.0));
	// From the start of the switch interval to its end.
	PlayOnTo(launcher, kInterval, kInterval + kBar);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.25));
	PlayOnTo(launcher, kInterval + kBar, kInterval + 2 * kBar);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
}

void TestLandingTransitionBSettlesSlowly() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionB);
	LandOnRow2(launcher);
	PlayOnTo(launcher, kInterval, kInterval + 2 * kBar);
	// A clip's curve mirrored in time: quick at first, slow at the end.
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 1.0 - (std::exp(2.0) - 1.0) / (std::exp(4.0) - 1.0)));
	CHECK(CLauncher::LandingCurve(CLauncher::kTransitionB, 0.5) > 0.5);
	CHECK(Near(CLauncher::LandingCurve(CLauncher::kTransitionD, 0.0), 0.0));
	CHECK(Near(CLauncher::LandingCurve(CLauncher::kTransitionD, 1.0), 1.0));
	CHECK(Near(CLauncher::LandingCurve(CLauncher::kTransitionA, 0.25), 0.25));
}

void TestLandingTransitionsCAndDMoveOnlyInTheFirstBar() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionC);
	LandOnRow2(launcher);
	PlayOnTo(launcher, kInterval, kInterval + kBar / 2);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	// For the rest of the interval, the clip's value.
	PlayOnTo(launcher, kInterval + kBar / 2, kInterval + 2 * kBar);
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0);
	CHECK(launcher.PlaysLanding(0));

	CLauncher curved;
	SetUpLanding(curved, CLauncher::kTransitionD);
	LandOnRow2(curved);
	PlayOnTo(curved, kInterval, kInterval + kBar / 2);
	CHECK(Near(curved.GetCVValue(0, kOutputB), CLauncher::LandingCurve(CLauncher::kTransitionD, 0.5)));
}

void TestLandingTransitionCWithLessThanTwoBarsIsLikeA() {
	// With half a bar, C moves across the whole interval.
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionC);
	launcher.SetLength(CLauncher::kLengthHalfBar);
	PlayThrough(launcher, 0.0, kBar / 8);
	launcher.SetSelection(0, 2);
	PlayThrough(launcher, kBar / 8, kBar / 2 + kBatch);
	CHECK(launcher.PlaysLanding(0));
	PlayOnTo(launcher, kBar / 2, kBar / 2 + kBar / 4);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
}

void TestTransitionMovesTowardsTheQueuedClipsLanding() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	launcher.SetClipUnipolar(0, 1, kOutputB, 3);
	launcher.SetClipTransition(0, 1, kOutputB, CLauncher::kTransitionA);
	launcher.Advance(Playing(kBar));
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kBar));
	// From 0.5 towards the landing's 0 rather than the clip's 1.
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.25));
	// Without the landing it starts over towards the clip.
	launcher.SetLandingPresent(0, 2, false);
	launcher.Advance(Playing(kBar * 2.5));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.25));
	launcher.Advance(Playing(kBar * 3.25));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.625));
}

void TestQueueingDuringALandingStartsOverFromWhatIsEmitted() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionA);
	launcher.SetClipTransition(0, 2, kOutputB, CLauncher::kTransitionA);
	LandOnRow2(launcher);
	PlayOnTo(launcher, kInterval, kInterval + kBar - kBatch);
	const double emitted = launcher.GetCVValue(0, kOutputB);
	CHECK(Near(emitted, 0.25 - kBatch / kInterval));

	// The landing's movement is dropped; the clip's own transition moves from
	// there towards row 3, at 0, over the remaining three bars.
	launcher.SetSelection(0, 3);
	PlayOnTo(launcher, kInterval + kBar - kBatch, kInterval + 2 * kBar);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), emitted * 2.0 / 3.0));
	CHECK(launcher.PlaysLanding(0));

	// Choosing the active clip again leaves nothing to move towards: back to
	// where the landing has got to.
	launcher.SetSelection(0, 2);
	launcher.Advance(Playing(kInterval + 2 * kBar + kBatch));
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5 + kBatch / kInterval));
}

void TestFillInLiesOverALanding() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	launcher.SetFillInPresent(0, 2, true);
	launcher.SetFillInUnipolar(0, 2, kOutputB, 3);
	launcher.SetFillInPattern(0, 2, 0, 4);
	launcher.SetFillInColour(0, 2, 12);
	LandOnRow2(launcher);
	Press(launcher);
	launcher.Advance(Playing(kInterval + kBatch * 2));
	CHECK(launcher.GetCVValue(0, 0) == 5.0 / 8);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	CHECK(IsLight(launcher.GetLight(0, 2), CLauncher::kFlashing, 12));
	// Underneath, the landing goes on.
	launcher.SetFillInPresent(0, 2, false);
	launcher.Advance(Playing(kInterval + kBatch * 3));
	CHECK(launcher.GetCVValue(0, 0) == 8.0 / 8);
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
}

void TestChangesToTheLandingTakeEffectAtOnce() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	LandOnRow2(launcher);
	launcher.SetLandingPattern(0, 2, 0, 3);
	launcher.SetLandingUnipolar(0, 2, kOutputB, 3);
	launcher.SetLandingColour(0, 2, 9);
	launcher.Advance(Playing(kInterval + kBatch * 2));
	CHECK(launcher.GetCVValue(0, 0) == 4.0 / 8);
	CHECK(Near(launcher.GetCVValue(0, kOutputB), 0.5));
	CHECK(IsLight(launcher.GetLight(0, 2), CLauncher::kVibrant, 9));

	launcher.SetLandingPresent(0, 2, false);
	launcher.Advance(Playing(kInterval + kBatch * 3));
	CHECK(!launcher.PlaysLanding(0));
	CHECK(launcher.GetCVValue(0, 0) == 2.0 / 8);
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0);
}

void TestMutedTrackPlaysNoLandingPattern() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	LandOnRow2(launcher);
	launcher.SetMutePressed(0, true);
	launcher.Advance(Playing(kInterval + kBatch * 2));
	CHECK(launcher.GetCVValue(0, 0) == 0.0);
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
}

void TestLandingEditingShowsTheLanding() {
	CLauncher launcher;
	launcher.SetClipPresent(3, 6, true);
	launcher.SetClipPattern(3, 6, 0, 4);
	launcher.SetSetup(CLauncher::SetupForLandingEditing(3, 6));
	// Only the toggle without a landing.
	CHECK(IsLight(launcher.GetLight(7, 1), CLauncher::kDim, CLauncher::kColourRed));
	CHECK(IsUnlit(launcher.GetLight(4, 3)));

	launcher.SetLandingPresent(3, 6, true);
	launcher.SetLandingColour(3, 6, 9);
	launcher.SetLandingPattern(3, 6, 0, 2);
	CHECK(IsLight(launcher.GetLight(7, 1), CLauncher::kVibrant, CLauncher::kColourGreen));
	CHECK(IsLight(launcher.GetLight(2, 2), CLauncher::kVibrant, 9));
	CHECK(IsLight(launcher.GetLight(2, 3), CLauncher::kVibrant, CLauncher::kColourOrange));
	CHECK(IsLight(launcher.GetLight(4, 3), CLauncher::kDim, CLauncher::kColourOrange));
}

void TestClipLandsAgainAfterItsFillIn() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	launcher.SetFillInPresent(0, 2, true);
	launcher.SetFillInPattern(0, 2, 0, 4);
	OpenWith(launcher, 0, 2);
	PlayThrough(launcher, 0.0, kBar);
	CHECK(!launcher.PlaysLanding(0));
	Press(launcher);
	PlayThrough(launcher, kBar, kInterval);
	CHECK(launcher.GetCVValue(0, 0) == 5.0 / 8);

	// Nothing is queued: at the switch point the clip goes on, and lands.
	launcher.Advance(Playing(kInterval));
	CHECK(launcher.PlaysLanding(0));
	CHECK(launcher.GetCVValue(0, 0) == 8.0 / 8);
	CHECK(launcher.GetCVValue(0, kOutputB) == 0.0);
	PlayThrough(launcher, kInterval + kBatch, 2 * kInterval + kBatch);
	CHECK(!launcher.PlaysLanding(0));
	CHECK(launcher.GetCVValue(0, kOutputB) == 1.0);
}

void TestNoLandingAfterAFillInWhenSomethingIsQueued() {
	// Row 1, queued during the fill-in, has no landing.
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	launcher.SetFillInPresent(0, 2, true);
	OpenWith(launcher, 0, 2);
	PlayThrough(launcher, 0.0, kBar);
	Press(launcher);
	launcher.SetSelection(0, 1);
	PlayThrough(launcher, kBar, kInterval + kBatch);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
	CHECK(!launcher.PlaysLanding(0));

	// Stop queued during the fill-in.
	CLauncher stopping;
	SetUpLanding(stopping, CLauncher::kTransitionOff);
	stopping.SetFillInPresent(0, 2, true);
	OpenWith(stopping, 0, 2);
	PlayThrough(stopping, 0.0, kBar);
	Press(stopping);
	stopping.SetSelection(0, CLauncher::kSelectionStop);
	PlayThrough(stopping, kBar, kInterval + kBatch);
	CHECK(!stopping.PlaysLanding(0));
	CHECK(stopping.GetCVValue(0, 0) == 0.0);
}

void TestClipWithoutAFillInDoesNotLandAfterThePress() {
	CLauncher launcher;
	SetUpLanding(launcher, CLauncher::kTransitionOff);
	OpenWith(launcher, 0, 2);
	PlayThrough(launcher, 0.0, kBar);
	Press(launcher);
	PlayThrough(launcher, kBar, kInterval + kBatch);
	CHECK(!launcher.IsFillInPlaying());
	CHECK(!launcher.PlaysLanding(0));
}

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
	TestStopLights();
	TestDefaultColourFollowsTheTrack();
	TestClipPickingFlashesEveryLaunchButton();
	TestFlashingGoesOnWhileStopped();
	TestClipEditingOfEmptyLaunchButtonShowsOnlyToggle();
	TestClipEditingShowsColoursAndPatterns();
	TestPatternsAreOrangeInEveryStep();
	TestUnipolarValueLightsTheValuesBelowIt();
	TestBipolarValueLightsTheValuesBackToZero();
	TestTransitionButtonShowsTheColourOfItsTransition();
	TestEditingFollowsTheCVModesOfThePickedTrack();
	TestLeavingSetupModeShowsTheEditedClip();
	TestButtonIndexAndSetupNumbering();
	TestOutOfRangeClipSettingsAreIgnored();
	TestOneActiveAndAtMostOneQueuedPerTrack();
	TestOutOfRangeSelectionIsIgnored();
	TestIntervalMeterFillsFromTheBottom();
	TestIntervalMeterColours();
	TestIntervalMeterFollowsTheSongPositionWhileStopped();
	TestIntervalMeterIsUnlitBeforeTheSongStarts();
	TestIntervalMeterDividesAnyTimeSignatureIntoEight();
	TestIntervalMeterRunsInClipEditing();
	TestLengthsInFourFour();
	TestBeatAndHalfBarFollowTheTimeSignature();
	TestNewDeviceHasFourBars();
	TestShorterLengthSwitchesSooner();
	TestImmediateNeverQueues();
	TestChangingLengthMovesTheNextSwitchPoint();
	TestChangingToImmediateFiresWhatIsQueued();
	TestIntervalMeterFollowsTheLength();
	TestClipPickingShowsTheLengthOnTheSceneButtons();
	TestSceneButtonsFlashWithTheLaunchButtons();
	TestOutOfRangeLengthIsIgnored();
	TestModeColourFollowsTheSetupStep();
	TestLaunchLabelsAreLitOnlyInClipEditing();
	TestBackButtonShowsTheStepItGoesBackFrom();
	TestBackLabelHasTheColourOfTheBackButton();
	TestSceneLabelsShowTheLength();

	TestNewFillInHasThePatternOfItsClipInWhite();
	TestFillInPlaysUntilTheNextSwitchPoint();
	TestFillInGivesWayToWhatIsQueued();
	TestFillInDoesNotEndOneBatchAfterAPress();
	TestPressingDuringAFillInDoesNothing();
	TestHoldingTheButtonTriggersOnlyOnce();
	TestNoFillInWithTheLengthImmediate();
	TestFillInWaitsWhileTheTransportIsStopped();
	TestChangesToTheFillInTakeEffectAtOnce();
	TestEmptyLaunchButtonHasNoFillInToPlay();
	TestFillInLights();
	TestForwardButtonIsLitWhileItLeadsSomewhere();
	TestFillInEditingWithoutFillInShowsOnlyToggle();
	TestFillInEditingShowsColoursAndPatterns();
	TestIntervalMeterRunsInFillInEditing();

	TestNewDeviceHasTheStopFunction();
	TestFunctionSelectButtonsWorkLikeRadioButtons();
	TestMuteSilencesATrackAtOnceUntilTheNextSwitchPoint();
	TestSecondPressUnmutes();
	TestSeveralTracksCanBeMuted();
	TestHoldingAMuteButtonTogglesOnlyOnce();
	TestMuteInTheBatchAfterASwitchPointLastsTheWholeInterval();
	TestMuteLastsWithTheLengthImmediate();
	TestMuteWaitsWhileTheTransportIsStopped();
	TestMutesAndSolosEndWhenTheSongPositionJumps();
	TestMuteAndSoloNeedClips();
	TestSoloSilencesTheOtherTracksUntilTheNextSwitchPoint();
	TestSoloAndMuteCombine();
	TestMutedTrackPlaysNoFillIn();
	TestFunctionLights();

	TestCVModesOfANewDevice();
	TestUnipolarAndBipolarSteps();
	TestOutputsEmitTheValueOfTheirCVMode();
	TestEditingAValueTakesEffectAtOnce();
	TestStoppedTrackHoldsItsValue();
	TestMuteAndSoloLeaveUnipolarAndBipolarOutputsAlone();
	TestWithoutTransitionTheValueJumpsAtTheSwitchPoint();
	TestTransitionAMovesInAStraightLine();
	TestTheIncomingClipsTransitionDoesNotCount();
	TestTransitionBFollowsAnExponentialCurve();
	TestTransitionCMovesOnlyInTheLastBar();
	TestTransitionCWithOneBarIsLikeA();
	TestNoTransitionWithTheLengthImmediate();
	TestQueueingAnotherClipStartsOverFromWhereItIs();
	TestEditingTheQueuedValueStartsOver();
	TestCancellingJumpsBack();
	TestChangingTheCVModeDropsTheTransition();
	TestFillInWithoutTransitionJumpsAtThePress();
	TestFillInTransitionRisesToTheSwitchPoint();
	TestFillInTransitionDWaitsForTheLastBar();
	TestFillInHidesTheClipsTransition();
	TestFillInStartsFromWhatIsEmitted();
	TestChangedPatternOnAnyOutputFlashes();
	TestOutputLabelsFollowTheCVModeAndTransition();

	TestNewLandingHasThePatternOfItsClipInWhite();
	TestLandingPlaysForTheFirstSwitchIntervalOnly();
	TestLandingPlaysAfterStop();
	TestEveryTrackOfASceneLandsOnItsOwn();
	TestNoLandingWithoutASwitchPoint();
	TestChoosingTheActiveClipAgainDoesNotLand();
	TestJumpingEndsALanding();
	TestLandingTransitionAMovesInAStraightLine();
	TestLandingTransitionBSettlesSlowly();
	TestLandingTransitionsCAndDMoveOnlyInTheFirstBar();
	TestLandingTransitionCWithLessThanTwoBarsIsLikeA();
	TestTransitionMovesTowardsTheQueuedClipsLanding();
	TestQueueingDuringALandingStartsOverFromWhatIsEmitted();
	TestFillInLiesOverALanding();
	TestChangesToTheLandingTakeEffectAtOnce();
	TestMutedTrackPlaysNoLandingPattern();
	TestLandingEditingShowsTheLanding();
	TestClipLandsAgainAfterItsFillIn();
	TestNoLandingAfterAFillInWhenSomethingIsQueued();
	TestClipWithoutAFillInDoesNotLandAfterThePress();

	return ReportChecks("Launcher");
}
