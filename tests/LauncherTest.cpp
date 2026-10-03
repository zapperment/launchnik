// Command-line tests for the switching rules. Run with tests/run.sh.

#include "../Launcher.h"

#include <cstdio>

namespace {

int gFailures = 0;
int gChecks = 0;

#define CHECK(condition) \
	do { \
		++gChecks; \
		if (!(condition)) { \
			++gFailures; \
			std::printf("FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		} \
	} while (false)

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
		CHECK(launcher.IsLit(track, CLauncher::kSelectionStop));
		CHECK(launcher.GetCVValue(track) == 0.0);
		for (int pattern = 1; pattern <= 8; ++pattern) {
			CHECK(launcher.GetButtonState(track, pattern) == CLauncher::kOff);
			CHECK(!launcher.IsLit(track, pattern));
		}
	}
}

void TestClickWhileStoppedTakesEffectAtOnce() {
	CLauncher launcher;
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
	PlayThrough(launcher, kBar, kBar + 100);
	launcher.SetSelection(0, 3);
	launcher.Advance(Playing(kBar + 100));

	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kQueued);
	// The outgoing button stays active and the CV keeps its value.
	CHECK(launcher.GetButtonState(0, CLauncher::kSelectionStop) == CLauncher::kActive);
	CHECK(launcher.IsLit(0, CLauncher::kSelectionStop));
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
	const int selections[CLauncher::kTrackCount] = { 4, 0, 0, 0, 0, 0, 0, 0 };
	launcher.Reset(selections);
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
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 3);
	launcher.Advance(Playing(120));
	launcher.SetSelection(0, 3);
	launcher.Advance(Playing(140));
	CHECK(launcher.GetButtonState(0, 3) == CLauncher::kQueued);
}

void TestClickingActiveButtonCancelsQueue() {
	CLauncher launcher;
	const int selections[CLauncher::kTrackCount] = { 3, 0, 0, 0, 0, 0, 0, 0 };
	launcher.Reset(selections);
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
	PlayThrough(launcher, kInterval - 100, kInterval);
	launcher.SetSelection(0, 6);
	launcher.Advance(Playing(kInterval));
	CHECK(launcher.GetButtonState(0, 6) == CLauncher::kActive);

	// The same change arriving one batch late is still taken at once.
	CLauncher late;
	PlayThrough(late, kInterval - 100, kInterval + kBatch);
	late.SetSelection(0, 6);
	late.Advance(Playing(kInterval + kBatch));
	CHECK(late.GetButtonState(0, 6) == CLauncher::kActive);

	// Two batches late is an ordinary click and waits for the next switch point.
	CLauncher later;
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
	launcher.Advance(Playing(100, 3, 4));
	launcher.SetSelection(0, 1);
	launcher.Advance(Playing(interval - 100, 3, 4));
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kQueued);
	launcher.Advance(Playing(interval - 1, 3, 4));
	CHECK(launcher.GetButtonState(0, 1) == CLauncher::kActive);
}

void TestQueuedButtonFlashesOnTheBeat() {
	CLauncher launcher;
	launcher.Advance(Playing(100));
	launcher.SetSelection(0, 3);
	const long long quaver = CLauncher::kPPQPerCrotchet / 2;

	// Lit for the first quaver of a crotchet, dark for the second.
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet));
	CHECK(launcher.IsLit(0, 3));
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver - kBatch));
	CHECK(launcher.IsLit(0, 3));
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver));
	CHECK(!launcher.IsLit(0, 3));
	launcher.Advance(Playing(2 * CLauncher::kPPQPerCrotchet));
	CHECK(launcher.IsLit(0, 3));

	// The active button does not flash.
	launcher.Advance(Playing(CLauncher::kPPQPerCrotchet + quaver));
	CHECK(launcher.IsLit(0, CLauncher::kSelectionStop));
}

void TestResetMakesSelectionsActive() {
	CLauncher launcher;
	const int selections[CLauncher::kTrackCount] = { 0, 1, 2, 3, 4, 5, 6, 8 };
	launcher.Reset(selections);
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		CHECK(launcher.GetButtonState(track, selections[track]) == CLauncher::kActive);
		CHECK(launcher.GetCVValue(track) == selections[track] / 8.0);
	}
}

void TestOneActiveAndAtMostOneQueuedPerTrack() {
	CLauncher launcher;
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
	TestResetMakesSelectionsActive();
	TestOneActiveAndAtMostOneQueuedPerTrack();
	TestOutOfRangeSelectionIsIgnored();

	std::printf("%d checks, %d failed\n", gChecks, gFailures);
	return gFailures == 0 ? 0 : 1;
}
