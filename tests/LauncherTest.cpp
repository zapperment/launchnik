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
		// With no clips to stop, the stop button is unlit.
		CHECK(launcher.GetStopLight(track) == CLauncher::kUnlit);
		CHECK(launcher.GetStopLightFrame(track) == 0);
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
	CHECK(launcher.GetStopLight(0) == CLauncher::kDim);
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
	CHECK(launcher.GetStopLightFrame(1) == 2);
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver - kBatch));
	CHECK(launcher.GetLightFrame(0, 3) == vibrantFrame);
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver));
	CHECK(launcher.GetLightFrame(0, 3) == dimFrame);
	CHECK(launcher.GetStopLightFrame(1) == 1);
	launcher.Advance(Playing(2 * CLauncher::kPPQPerCrotchet));
	CHECK(launcher.GetLightFrame(0, 3) == vibrantFrame);

	// To a control surface the lights are steadily "flashing".
	CHECK(IsLight(launcher.GetLight(0, 3), CLauncher::kFlashing, 4));
	CHECK(launcher.GetStopLight(1) == CLauncher::kFlashing);

	// The active buttons do not flash. Track 2's clips are orange.
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver));
	CHECK(launcher.GetStopLightFrame(0) == 1);
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
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::DefaultColour(0)));
	launcher.SetClipPattern(0, 1, 4);
	launcher.Advance(Playing(120));
	CHECK(launcher.GetCVValue(0) == 1.0 / 8.0);
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
	// It flashes while the new pattern waits, as a queued clip would.
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kFlashing, CLauncher::DefaultColour(0)));
	launcher.Advance(Playing(kInterval - 1));
	CHECK(launcher.GetCVValue(0) == 5.0 / 8.0);
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::DefaultColour(0)));

	// Changing it back before the switch point leaves nothing to wait for.
	launcher.Advance(Playing(kInterval + 2 * kBatch));
	launcher.SetClipPattern(0, 1, 6);
	launcher.Advance(Playing(kInterval + 3 * kBatch));
	CHECK(launcher.GetLight(0, 1).fBrightness == CLauncher::kFlashing);
	launcher.SetClipPattern(0, 1, 4);
	launcher.Advance(Playing(kInterval + 4 * kBatch));
	CHECK(launcher.GetLight(0, 1).fBrightness == CLauncher::kVibrant);

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
	CHECK(launcher.GetStopLightFrame(0) == 1);
	CHECK(launcher.GetStopLightFrame(1) == 2);
	CHECK(launcher.GetStopLightFrame(2) == 0);

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
	CHECK(launcher.GetCVValue(2) == 4.0 / 8.0);
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
	launcher.SetClipPattern(3, 6, 2);
	launcher.SetSetup(CLauncher::SetupForClipEditing(3, 6));

	CHECK(IsLight(launcher.GetLight(7, 1), CLauncher::kVibrant, CLauncher::kColourGreen));
	// The back button has taken over from the launch button in row 8, column 8.
	CHECK(IsUnlit(launcher.GetLight(7, 8)));

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
	CHECK(IsLight(launcher.GetLight(0, 1), CLauncher::kVibrant, CLauncher::DefaultColour(0)));
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
	CHECK(launcher.GetCVValue(0) == 1.0 / 8);
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
}

void TestBackButtonShowsTheStepItGoesBackFrom() {
	CLauncher launcher;
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonDark);
	launcher.SetSetup(CLauncher::kSetupClipPicking);
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonVibrantWhite);
	launcher.SetSetup(CLauncher::SetupForClipEditing(2, 4));
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonVibrantGreen);
	launcher.SetSetup(CLauncher::kSetupLaunchMode);
	CHECK(launcher.GetBackButtonFrame() == CLauncher::kBackButtonDark);
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
	TestSceneLabelsShowTheLength();

	return ReportChecks("Launcher");
}
