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

// A light as one number: 0 is unlit, then dim, vibrant and flashing for each
// colour.
int LightValue(const CLauncher::TLight& iLight) {
	if (iLight.fBrightness == CLauncher::kUnlit) {
		return 0;
	}
	return 1 + 3 * iLight.fColour + (iLight.fBrightness - CLauncher::kDim);
}

// Red, orange, yellow, green, cyan, blue, purple and pink for tracks 1 to 8.
const int kDefaultColours[CLauncher::kTrackCount] = { 1, 2, 3, 5, 7, 9, 11, 13 };

} // namespace

CLauncher::CLauncher()
	:
	fSetup(kSetupLaunchMode),
	fLength(kDefaultLength),
	fFlashLit(false),
	fStoppedTime(0.0),
	fSwitchPointInPreviousBatch(false),
	fReachedSegments(0)
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
			clip.fColour = DefaultColour(track);
		}
	}
}

int CLauncher::DefaultColour(int iTrack) {
	return kDefaultColours[iTrack];
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

void CLauncher::SetLength(int iLength) {
	if (iLength >= 0 && iLength < kLengthCount) {
		fLength = iLength;
	}
}

int CLauncher::SceneForLength(int iLength) {
	return kRowCount - iLength;
}

int CLauncher::LengthForScene(int iScene) {
	return kRowCount - iScene;
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
		// With the length immediate, nothing waits for a switch point.
		switchNow = switchPointInBatch || fSwitchPointInPreviousBatch || fLength == kLengthImmediate;

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

	// The interval meter follows the song position whether it moves or not.
	fReachedSegments = CountReachedSegments(iTransport);

	// While the transport is stopped, and with the length immediate, selections
	// and changes to the active clip take effect at once.
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

bool CLauncher::HasClips(int iTrack) const {
	for (int row = 1; row <= kRowCount; ++row) {
		if (Clip(iTrack, row).fPresent) {
			return true;
		}
	}
	return false;
}

bool CLauncher::HasPendingPattern(int iTrack) const {
	const int active = fActive[iTrack];
	return active != kSelectionStop && Clip(iTrack, active).fPresent && Clip(iTrack, active).fPattern + 1 != fEmitted[iTrack];
}

CLauncher::EBrightness CLauncher::GetStopLight(int iTrack) const {
	switch (GetButtonState(iTrack, kSelectionStop)) {
		case kQueued:
			return kFlashing;
		case kActive:
			return HasClips(iTrack) ? kDim : kUnlit;
		default:
			return kVibrant;
	}
}

int CLauncher::GetStopLightFrame(int iTrack) const {
	const EBrightness light = GetStopLight(iTrack);
	if (light == kFlashing) {
		return fFlashLit ? kVibrant : kDim;
	}
	return light;
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
		// It flashes while a change of its pattern waits for the switch point.
		case kActive:
			return MakeLight(HasPendingPattern(iTrack) ? kFlashing : kVibrant, clip.fColour);
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
		return clip.fPresent ? MakeLight(kVibrant, kColourGreen) : MakeLight(kDim, kColourRed);
	}
	if (iTrack == kEditingBackTrack && iRow == kEditingBackRow) {
		return MakeLight(kDim, kColourBlue);
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
	return LightValue(GetLight(iTrack, iRow));
}

int CLauncher::GetLightFrame(int iTrack, int iRow) const {
	const TLight light = GetLight(iTrack, iRow);
	if (light.fBrightness == kUnlit) {
		return 0;
	}
	const bool vibrant = light.fBrightness == kVibrant || (light.fBrightness == kFlashing && fFlashLit);
	return 1 + 2 * light.fColour + (vibrant ? 1 : 0);
}

CLauncher::TLight CLauncher::GetSceneLight(int iScene) const {
	if (fSetup == kSetupClipPicking) {
		return MakeLight(kFlashing, iScene == SceneForLength(fLength) ? kColourGreen : kColourWhite);
	}
	// The bottom scene button stands for the first segment.
	const int segment = kSegmentCount + 1 - iScene;
	if (segment < 1 || segment > fReachedSegments) {
		return kUnlitLight;
	}
	if (segment <= kGreenSegmentCount) {
		return MakeLight(kDim, kColourGreen);
	}
	if (segment <= kGreenSegmentCount + kYellowSegmentCount) {
		return MakeLight(kDim, kColourYellow);
	}
	return MakeLight(kDim, kColourRed);
}

int CLauncher::GetSceneLightValue(int iScene) const {
	return LightValue(GetSceneLight(iScene));
}

int CLauncher::GetSceneLightFrame(int iScene) const {
	const TLight light = GetSceneLight(iScene);
	if (light.fBrightness == kUnlit) {
		return 0;
	}
	// The film strip holds red, green, yellow and white, in that order.
	int stripColour = 3;
	if (light.fColour == kColourRed) {
		stripColour = 0;
	}
	else if (light.fColour == kColourGreen) {
		stripColour = 1;
	}
	else if (light.fColour == kColourYellow) {
		stripColour = 2;
	}
	const bool vibrant = light.fBrightness == kVibrant || (light.fBrightness == kFlashing && fFlashLit);
	return 1 + 2 * stripColour + (vibrant ? 1 : 0);
}

CLauncher::EModeColour CLauncher::GetModeColour() const {
	if (fSetup == kSetupLaunchMode) {
		return kModeWhite;
	}
	return fSetup == kSetupClipPicking ? kModeGreen : kModeYellow;
}

int CLauncher::GetLaunchLabelsFrame() const {
	if (fSetup < kSetupFirstClipEditing) {
		return 0;
	}
	const int picked = fSetup - kSetupFirstClipEditing;
	return fClips[picked / kRowCount][picked % kRowCount].fPresent ? 2 : 1;
}

int CLauncher::GetSceneLabelFrame(int iScene) const {
	if (iScene < 1 || iScene > kRowCount) {
		return 0;
	}
	if (fSetup == kSetupClipPicking) {
		return 2;
	}
	return iScene == SceneForLength(fLength) ? 1 : 0;
}

double CLauncher::GetCVValue(int iTrack) const {
	// The Combinator rounds its CV input to the nearest of nine evenly spaced
	// steps (Off, 1..8), so these values sit in the middle of each step.
	return static_cast<double>(fEmitted[iTrack]) / kPatternCount;
}

long long CLauncher::ComputeSwitchIntervalLength(int iLength, int iNumerator, int iDenominator) {
	if (iNumerator <= 0 || iDenominator <= 0) {
		iNumerator = 4;
		iDenominator = 4;
	}
	// A beat is the beat of the time signature, so that every length divides
	// the bar or is a whole number of bars, and all of them line up with the
	// bars.
	const long long beatLength = kPPQPerCrotchet * 4 / iDenominator;
	const long long barLength = iNumerator * beatLength;
	if (iLength <= kLengthImmediate || iLength >= kLengthCount) {
		return 0;
	}
	if (iLength == kLengthBeat) {
		return beatLength;
	}
	if (iLength == kLengthHalfBar) {
		return barLength / 2;
	}
	// 1, 2, 4, 8 and 16 bars.
	return barLength << (iLength - kLengthFirstBars);
}

int CLauncher::CountReachedSegments(const TTransport& iTransport) const {
	const long long intervalLength = ComputeSwitchIntervalLength(fLength, iTransport.fTimeSignatureNumerator, iTransport.fTimeSignatureDenominator);
	if (iTransport.fPlayPos < 0.0 || intervalLength == 0) {
		return 0;
	}
	const long long position = static_cast<long long>(iTransport.fPlayPos) % intervalLength;
	// Multiplying first keeps this exact where a segment is not a whole number
	// of PPQ.
	return static_cast<int>(position * kSegmentCount / intervalLength) + 1;
}

bool CLauncher::ContainsSwitchPoint(const TTransport& iTransport) const {
	const long long intervalLength = ComputeSwitchIntervalLength(fLength, iTransport.fTimeSignatureNumerator, iTransport.fTimeSignatureDenominator);
	if (iTransport.fPlayPos < 0.0 || intervalLength == 0) {
		return false;
	}
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
