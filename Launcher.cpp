#include "Launcher.h"

namespace {

// Queued buttons are lit for the first quaver of every crotchet.
const long long kPPQPerQuaver = CLauncher::kPPQPerCrotchet / 2;

} // namespace

CLauncher::CLauncher()
	:
	fFlashLit(false),
	fSwitchPointInPreviousBatch(false)
{
	for (int track = 0; track < kTrackCount; ++track) {
		fSelections[track] = kSelectionStop;
		fActive[track] = kSelectionStop;
	}
}

void CLauncher::Reset(const int iSelections[kTrackCount]) {
	for (int track = 0; track < kTrackCount; ++track) {
		fSelections[track] = iSelections[track];
		fActive[track] = iSelections[track];
	}
}

void CLauncher::SetSelection(int iTrack, int iSelection) {
	if (iTrack < 0 || iTrack >= kTrackCount || iSelection < 0 || iSelection >= kSelectionCount) {
		return;
	}
	fSelections[iTrack] = iSelection;
}

void CLauncher::Advance(const TTransport& iTransport) {
	bool switchNow = true;
	bool switchPointInBatch = false;

	if (iTransport.fPlaying) {
		switchPointInBatch = ContainsSwitchPoint(iTransport);
		// A change that arrives one batch after a switch point still counts as
		// landing on it, since the host does not say where in a batch it happened.
		switchNow = switchPointInBatch || fSwitchPointInPreviousBatch;

		const long long position = static_cast<long long>(iTransport.fPlayPos);
		fFlashLit = position >= 0 && (position % kPPQPerCrotchet) < kPPQPerQuaver;
	}
	fSwitchPointInPreviousBatch = switchPointInBatch;

	// While the transport is stopped, selections take effect at once.
	if (switchNow) {
		for (int track = 0; track < kTrackCount; ++track) {
			fActive[track] = fSelections[track];
		}
	}
}

CLauncher::EButtonState CLauncher::GetButtonState(int iTrack, int iSelection) const {
	if (fActive[iTrack] == iSelection) {
		return kActive;
	}
	if (fSelections[iTrack] == iSelection) {
		return kQueued;
	}
	return kOff;
}

bool CLauncher::IsLit(int iTrack, int iSelection) const {
	switch (GetButtonState(iTrack, iSelection)) {
		case kActive:
			return true;
		case kQueued:
			return fFlashLit;
		default:
			return false;
	}
}

double CLauncher::GetCVValue(int iTrack) const {
	// The Combinator rounds its CV input to the nearest of nine evenly spaced
	// steps (Off, 1..8), so these values sit in the middle of each step.
	return static_cast<double>(fActive[iTrack]) / (kSelectionCount - 1);
}

long long CLauncher::ComputeSwitchIntervalLength(int iNumerator, int iDenominator) {
	if (iNumerator <= 0 || iDenominator <= 0) {
		iNumerator = 4;
		iDenominator = 4;
	}
	const long long barLength = iNumerator * (kPPQPerCrotchet * 4 / iDenominator);
	return barLength * kBarsPerSwitchInterval;
}

bool CLauncher::ContainsSwitchPoint(const TTransport& iTransport) {
	if (iTransport.fPlayPos < 0.0) {
		return false;
	}
	const long long intervalLength = ComputeSwitchIntervalLength(iTransport.fTimeSignatureNumerator, iTransport.fTimeSignatureDenominator);
	const double batchEnd = iTransport.fPlayPos + iTransport.fBatchLength;

	// Switch points are counted from the start of the song. Find the first one
	// at or after the start of the batch and see whether the batch reaches it.
	const long long intervalsBefore = static_cast<long long>(iTransport.fPlayPos / intervalLength);
	double nextSwitchPoint = static_cast<double>(intervalsBefore * intervalLength);
	if (nextSwitchPoint < iTransport.fPlayPos) {
		nextSwitchPoint += intervalLength;
	}
	return nextSwitchPoint < batchEnd;
}
