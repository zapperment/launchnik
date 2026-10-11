#include "Launcher.h"

namespace {

// Flashing lights are vibrant for the first quaver of every crotchet.
const long long kPPQPerQuaver = CLauncher::kPPQPerCrotchet / 2;

bool IsFirstQuaver(double iPosition) {
	const long long position = static_cast<long long>(iPosition);
	return position >= 0 && (position % CLauncher::kPPQPerCrotchet) < kPPQPerQuaver;
}

CLauncher::TLight MakeLight(CLauncher::EBrightness iBrightness, int iColour) {
	CLauncher::TLight light;
	light.fBrightness = iBrightness;
	light.fColour = iColour;
	return light;
}

const CLauncher::TLight kUnlitLight = { CLauncher::kUnlit, CLauncher::kColourWhite };

} // namespace

CLauncher::CLauncher()
	:
	fSetup(kSetupLaunchMode),
	fFlashLit(false),
	fStoppedTime(0.0),
	fSwitchPointInPreviousBatch(false)
{
	for (int track = 0; track < kTrackCount; ++track) {
		fSelections[track] = kSelectionStop;
		fActive[track] = kSelectionStop;
		fEmitted[track] = 0;
		for (int row = 1; row <= kRowCount; ++row) {
			TClip& clip = fClips[track][row - 1];
			clip.fPresent = false;
			// A new clip plays the pattern of its row.
			clip.fPattern = row - 1;
			clip.fColour = kColourWhite;
		}
	}
}

int CLauncher::ButtonIndex(int iTrack, int iRow) {
	return iTrack * kRowCount + (iRow - 1);
}

int CLauncher::SetupForClipEditing(int iTrack, int iRow) {
	return kSetupFirstClipEditing + ButtonIndex(iTrack, iRow);
}

bool CLauncher::IsValidButton(int iTrack, int iRow) {
	return iTrack >= 0 && iTrack < kTrackCount && iRow >= 1 && iRow <= kRowCount;
}

const CLauncher::TClip& CLauncher::Clip(int iTrack, int iRow) const {
	return fClips[iTrack][iRow - 1];
}

void CLauncher::SetClipPresent(int iTrack, int iRow, bool iPresent) {
	if (IsValidButton(iTrack, iRow)) {
		fClips[iTrack][iRow - 1].fPresent = iPresent;
	}
}

void CLauncher::SetClipPattern(int iTrack, int iRow, int iPattern) {
	if (IsValidButton(iTrack, iRow) && iPattern >= 0 && iPattern < kPatternCount) {
		fClips[iTrack][iRow - 1].fPattern = iPattern;
	}
}

void CLauncher::SetClipColour(int iTrack, int iRow, int iColour) {
	if (IsValidButton(iTrack, iRow) && iColour >= 0 && iColour < kColourCount) {
		fClips[iTrack][iRow - 1].fColour = iColour;
	}
}

void CLauncher::SetSetup(int iSetup) {
	if (iSetup >= 0 && iSetup < kSetupCount) {
		fSetup = iSetup;
	}
}

void CLauncher::SetSelection(int iTrack, int iSelection) {
	if (iTrack < 0 || iTrack >= kTrackCount || iSelection < 0 || iSelection >= kSelectionCount) {
		return;
	}
	fSelections[iTrack] = iSelection;
}

int CLauncher::Target(int iTrack) const {
	const int selection = fSelections[iTrack];
	if (selection != kSelectionStop && !Clip(iTrack, selection).fPresent) {
		return kSelectionStop;
	}
	return selection;
}

void CLauncher::Switch() {
	for (int track = 0; track < kTrackCount; ++track) {
		fActive[track] = Target(track);
		fEmitted[track] = fActive[track] == kSelectionStop ? 0 : Clip(track, fActive[track]).fPattern + 1;
	}
}

void CLauncher::Open() {
	Switch();
}

void CLauncher::Advance(const TTransport& iTransport) {
	bool switchNow = true;
	bool switchPointInBatch = false;

	if (iTransport.fPlaying) {
		switchPointInBatch = ContainsSwitchPoint(iTransport);
		// A change that arrives one batch after a switch point still counts as
		// landing on it, since the host does not say where in a batch it happened.
		switchNow = switchPointInBatch || fSwitchPointInPreviousBatch;

		fFlashLit = IsFirstQuaver(iTransport.fPlayPos);
		fStoppedTime = 0.0;
	}
	else {
		fFlashLit = IsFirstQuaver(fStoppedTime);
		fStoppedTime += iTransport.fBatchLength;
		// Keep the count small; only the place within a crotchet matters.
		if (fStoppedTime >= kPPQPerCrotchet) {
			fStoppedTime -= kPPQPerCrotchet;
		}
	}
	fSwitchPointInPreviousBatch = switchPointInBatch;

	// While the transport is stopped, selections and changes to the active clip
	// take effect at once.
	if (switchNow) {
		Switch();
	}
}

CLauncher::EButtonState CLauncher::GetButtonState(int iTrack, int iSelection) const {
	if (fActive[iTrack] == iSelection) {
		return kActive;
	}
	if (Target(iTrack) == iSelection) {
		return kQueued;
	}
	return kOff;
}

bool CLauncher::IsStopLit(int iTrack) const {
	switch (GetButtonState(iTrack, kSelectionStop)) {
		case kActive:
			return true;
		case kQueued:
			return fFlashLit;
		default:
			return false;
	}
}

CLauncher::TLight CLauncher::GetLight(int iTrack, int iRow) const {
	if (fSetup == kSetupLaunchMode) {
		return GetLaunchModeLight(iTrack, iRow);
	}
	if (fSetup == kSetupClipPicking) {
		return GetClipPickingLight(iTrack, iRow);
	}
	return GetClipEditingLight(iTrack, iRow);
}

CLauncher::TLight CLauncher::GetLaunchModeLight(int iTrack, int iRow) const {
	const TClip& clip = Clip(iTrack, iRow);
	switch (GetButtonState(iTrack, iRow)) {
		// A clip that was switched off while active goes on showing its colour
		// until its track has switched.
		case kActive:
			return MakeLight(kVibrant, clip.fColour);
		case kQueued:
			return MakeLight(kFlashing, clip.fColour);
		default:
			return clip.fPresent ? MakeLight(kDim, clip.fColour) : kUnlitLight;
	}
}

CLauncher::TLight CLauncher::GetClipPickingLight(int iTrack, int iRow) const {
	const TClip& clip = Clip(iTrack, iRow);
	return MakeLight(kFlashing, clip.fPresent ? clip.fColour : kColourWhite);
}

CLauncher::TLight CLauncher::GetClipEditingLight(int iTrack, int iRow) const {
	const int picked = fSetup - kSetupFirstClipEditing;
	const TClip& clip = fClips[picked / kRowCount][picked % kRowCount];

	if (iTrack == kEditingToggleTrack && iRow == kEditingToggleRow) {
		return MakeLight(clip.fPresent ? kVibrant : kDim, kColourWhite);
	}
	if (iTrack == kEditingBackTrack && iRow == kEditingBackRow) {
		return MakeLight(kDim, kColourWhite);
	}
	// The colour and pattern controls only show while there is a clip to set up.
	if (!clip.fPresent) {
		return kUnlitLight;
	}
	const bool isColourRow = iRow == kEditingFirstColourRow || iRow == kEditingFirstColourRow + 1;
	if (isColourRow && iTrack < kEditingColoursPerRow) {
		const int colour = (iRow - kEditingFirstColourRow) * kEditingColoursPerRow + iTrack;
		return MakeLight(colour == clip.fColour ? kVibrant : kDim, colour);
	}
	if (iRow == kEditingPatternRow) {
		return MakeLight(iTrack == clip.fPattern ? kVibrant : kDim, kColourPink);
	}
	return kUnlitLight;
}

int CLauncher::GetLightValue(int iTrack, int iRow) const {
	const TLight light = GetLight(iTrack, iRow);
	if (light.fBrightness == kUnlit) {
		return 0;
	}
	return 1 + 3 * light.fColour + (light.fBrightness - kDim);
}

int CLauncher::GetLightFrame(int iTrack, int iRow) const {
	const TLight light = GetLight(iTrack, iRow);
	if (light.fBrightness == kUnlit) {
		return 0;
	}
	const bool vibrant = light.fBrightness == kVibrant || (light.fBrightness == kFlashing && fFlashLit);
	return 1 + 2 * light.fColour + (vibrant ? 1 : 0);
}

double CLauncher::GetCVValue(int iTrack) const {
	// The Combinator rounds its CV input to the nearest of nine evenly spaced
	// steps (Off, 1..8), so these values sit in the middle of each step.
	return static_cast<double>(fEmitted[iTrack]) / kPatternCount;
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
