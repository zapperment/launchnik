#include "Launcher.h"

#include <cmath>

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

// How steep the exponential curve of transitions B and D is.
const double kCurveSteepness = 4.0;

bool IsValidOutput(int iOutput) {
	return iOutput >= 0 && iOutput < CLauncher::kOutputCount;
}

bool IsValidValue(int iValue) {
	return iValue >= 0 && iValue < CLauncher::kValueCount;
}

bool IsValidTransition(int iTransition) {
	return iTransition >= 0 && iTransition < CLauncher::kTransitionCount;
}

// Which of the colours the labels of the editing steps are lit in: 0 for
// yellow in clip editing, 1 for orange in fill-in editing, 2 for blue in
// landing editing, 3 for pink in intro editing, 4 for turquoise in outro
// editing.
int EditingColourIndex(int iSetup) {
	if (CLauncher::IsFillInEditing(iSetup)) {
		return 1;
	}
	if (CLauncher::IsLandingEditing(iSetup)) {
		return 2;
	}
	if (CLauncher::IsIntroEditing(iSetup)) {
		return 3;
	}
	return CLauncher::IsOutroEditing(iSetup) ? 4 : 0;
}

} // namespace

const CLauncher::TMovement CLauncher::kNoMovement = { false, 0.0, false, 0.0, 0, 0, 0, 0 };

CLauncher::CLauncher()
	:
	fSetup(kSetupLaunchMode),
	fLength(kDefaultLength),
	fFlashLit(false),
	fStoppedTime(0.0),
	fSwitchPointInPreviousBatch(false),
	fReachedSegments(0),
	fFillInPressed(false),
	fFillInTriggered(false),
	fFillInPlaying(false),
	fFunction(kFunctionStop),
	fHasPreviousTransport(false)
{
	for (int track = 0; track < kTrackCount; ++track) {
		fRelaunchPressed[track] = false;
		fRelaunchTriggered[track] = false;
		fRelaunched[track] = false;
		fMutePressed[track] = false;
		fSoloPressed[track] = false;
		fMuteTriggered[track] = false;
		fSoloTriggered[track] = false;
		fMuted[track] = false;
		fSoloed[track] = false;
		fSelections[track] = kSelectionStop;
		fActive[track] = kSelectionStop;
		fLastActive[track] = kSelectionStop;
		for (int output = 0; output < kOutputCount; ++output) {
			fEmitted[track][output] = 0;
			fCVModes[track][output] = DefaultCVMode(output);
			fClipValues[track][output] = 0.0;
			fValues[track][output] = 0.0;
			fMovements[track][output] = kNoMovement;
			fFillInMovements[track][output] = kNoMovement;
		}
		fLandingRows[track] = kSelectionStop;
		fLandingStarts[track] = 0.0;
		fIntroStarts[track] = 0.0;
		fOutroStarts[track] = 0.0;
		for (int output = 0; output < kOutputCount; ++output) {
			fOutroFrom[track][output] = 0.0;
			fOutroFromModes[track][output] = DefaultCVMode(output);
			fFrozenValues[track][output] = 0.0;
		}
		ForgetIntroAndOutro(track);
		for (int row = 1; row <= kRowCount; ++row) {
			TClip& clip = fClips[track][row - 1];
			clip.fPresent = false;
			clip.fColour = DefaultColour(track);
			// A new clip plays the pattern of its row, with both values at 0.
			for (int output = 0; output < kOutputCount; ++output) {
				clip.fPatterns[output] = row - 1;
				clip.fUnipolar[output] = 0;
				clip.fBipolar[output] = kBipolarZero;
				clip.fTransitions[output] = kTransitionOff;
			}
			clip.fOneBarPattern = false;
			// A fill-in, a landing, an intro and an outro are set up as copies
			// of their clip, in white.
			clip.fFillIn = clip;
			clip.fFillIn.fPresent = false;
			clip.fFillIn.fColour = kColourWhite;
			clip.fLanding = clip.fFillIn;
			clip.fIntro = clip.fFillIn;
			clip.fOutro = clip.fFillIn;
		}
	}
}

int CLauncher::PackOutputSettings(const TOutputSettings& iSettings) {
	return iSettings.fPattern + kPatternCount * (iSettings.fUnipolar + kValueCount * (iSettings.fBipolar + kValueCount * iSettings.fTransition));
}

CLauncher::TOutputSettings CLauncher::UnpackOutputSettings(int iPacked) {
	if (iPacked < 0 || iPacked >= kOutputSettingsCount) {
		iPacked = 0;
	}
	TOutputSettings settings;
	settings.fPattern = iPacked % kPatternCount;
	iPacked /= kPatternCount;
	settings.fUnipolar = iPacked % kValueCount;
	iPacked /= kValueCount;
	settings.fBipolar = iPacked % kValueCount;
	settings.fTransition = iPacked / kValueCount;
	return settings;
}

int CLauncher::PackSwitches(const TSwitches& iSwitches) {
	return (iSwitches.fPresent ? 1 : 0) + 2 * iSwitches.fColour + (iSwitches.fOneBarPattern ? 2 * kColourCount : 0);
}

CLauncher::TSwitches CLauncher::UnpackSwitches(int iPacked) {
	if (iPacked < 0 || iPacked >= kSwitchesCount) {
		iPacked = 0;
	}
	TSwitches switches;
	switches.fPresent = iPacked % 2 == 1;
	switches.fColour = (iPacked / 2) % kColourCount;
	switches.fOneBarPattern = iPacked >= 2 * kColourCount;
	return switches;
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

int CLauncher::SetupForFillInEditing(int iTrack, int iRow) {
	return kSetupFirstFillInEditing + ButtonIndex(iTrack, iRow);
}

int CLauncher::SetupForLandingEditing(int iTrack, int iRow) {
	return kSetupFirstLandingEditing + ButtonIndex(iTrack, iRow);
}

int CLauncher::SetupForIntroEditing(int iTrack, int iRow) {
	return kSetupFirstIntroEditing + ButtonIndex(iTrack, iRow);
}

int CLauncher::SetupForOutroEditing(int iTrack, int iRow) {
	return kSetupFirstOutroEditing + ButtonIndex(iTrack, iRow);
}

bool CLauncher::IsClipEditing(int iSetup) {
	return iSetup >= kSetupFirstClipEditing && iSetup < kSetupFirstFillInEditing;
}

bool CLauncher::IsFillInEditing(int iSetup) {
	return iSetup >= kSetupFirstFillInEditing && iSetup < kSetupFirstLandingEditing;
}

bool CLauncher::IsLandingEditing(int iSetup) {
	return iSetup >= kSetupFirstLandingEditing && iSetup < kSetupFirstIntroEditing;
}

bool CLauncher::IsIntroEditing(int iSetup) {
	return iSetup >= kSetupFirstIntroEditing && iSetup < kSetupFirstOutroEditing;
}

bool CLauncher::IsOutroEditing(int iSetup) {
	return iSetup >= kSetupFirstOutroEditing && iSetup < kSetupClearMode;
}

// The editing steps follow one another, each with every launch button in turn.
int CLauncher::PickedButton(int iSetup) {
	if (iSetup >= kSetupFirstClipEditing && iSetup < kSetupClearMode) {
		return (iSetup - kSetupFirstClipEditing) % kButtonCount;
	}
	return -1;
}

bool CLauncher::IsSetupMode(int iSetup) {
	return iSetup >= kSetupClipPicking && iSetup < kSetupClearMode;
}

bool CLauncher::IsDuplicateMode(int iSetup) {
	return iSetup >= kSetupSourcePicking && iSetup < kSetupCount;
}

int CLauncher::SetupForClipDestinationPicking(int iTrack, int iRow) {
	return kSetupFirstClipDestinationPicking + ButtonIndex(iTrack, iRow);
}

int CLauncher::SetupForSceneDestinationPicking(int iScene) {
	return kSetupFirstSceneDestinationPicking + (iScene - 1);
}

int CLauncher::SourceButton(int iSetup) {
	if (iSetup >= kSetupFirstClipDestinationPicking && iSetup < kSetupFirstSceneDestinationPicking) {
		return iSetup - kSetupFirstClipDestinationPicking;
	}
	return -1;
}

int CLauncher::SourceScene(int iSetup) {
	if (iSetup >= kSetupFirstSceneDestinationPicking && iSetup < kSetupCount) {
		return iSetup - kSetupFirstSceneDestinationPicking + 1;
	}
	return 0;
}

bool CLauncher::IsValidButton(int iTrack, int iRow) {
	return iTrack >= 0 && iTrack < kTrackCount && iRow >= 1 && iRow <= kRowCount;
}

const CLauncher::TClip& CLauncher::Clip(int iTrack, int iRow) const {
	return fClips[iTrack][iRow - 1];
}

CLauncher::TSettings* CLauncher::EditableSettings(int iTrack, int iRow, ESettingsKind iKind) {
	if (!IsValidButton(iTrack, iRow)) {
		return nullptr;
	}
	TClip& clip = fClips[iTrack][iRow - 1];
	if (iKind == kFillInSettings) {
		return &clip.fFillIn;
	}
	if (iKind == kLandingSettings) {
		return &clip.fLanding;
	}
	if (iKind == kIntroSettings) {
		return &clip.fIntro;
	}
	if (iKind == kOutroSettings) {
		return &clip.fOutro;
	}
	return &clip;
}

CLauncher::TSettings* CLauncher::EditableSettings(int iTrack, int iRow, int iOutput, ESettingsKind iKind) {
	return IsValidOutput(iOutput) ? EditableSettings(iTrack, iRow, iKind) : nullptr;
}

void CLauncher::SetClipPresent(int iTrack, int iRow, bool iPresent) {
	if (!IsValidButton(iTrack, iRow)) {
		return;
	}
	fClips[iTrack][iRow - 1].fPresent = iPresent;
	if (!iPresent && fActive[iTrack] == iRow) {
		StopAtOnce(iTrack);
	}
}

// A track whose active launch button is emptied does not wait for the switch
// point: it stops there and then, whatever it was playing, fill-in and landing
// included, with no outro, and every CV output emits 0, unipolar and bipolar
// ones too.
void CLauncher::StopAtOnce(int iTrack) {
	fActive[iTrack] = kSelectionStop;
	fLastActive[iTrack] = kSelectionStop;
	fLandingRows[iTrack] = kSelectionStop;
	fRelaunched[iTrack] = false;
	ForgetIntroAndOutro(iTrack);
	for (int output = 0; output < kOutputCount; ++output) {
		fEmitted[iTrack][output] = 0;
		fClipValues[iTrack][output] = 0.0;
		fValues[iTrack][output] = 0.0;
		fMovements[iTrack][output] = kNoMovement;
		fFillInMovements[iTrack][output] = kNoMovement;
	}
}

void CLauncher::SetClipColour(int iTrack, int iRow, int iColour) {
	if (IsValidButton(iTrack, iRow) && iColour >= 0 && iColour < kColourCount) {
		fClips[iTrack][iRow - 1].fColour = iColour;
	}
}

void CLauncher::SetClipPattern(int iTrack, int iRow, int iOutput, int iPattern) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kClipSettings);
	if (settings && iPattern >= 0 && iPattern < kPatternCount) {
		settings->fPatterns[iOutput] = iPattern;
	}
}

void CLauncher::SetClipUnipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kClipSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fUnipolar[iOutput] = iValue;
	}
}

void CLauncher::SetClipBipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kClipSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fBipolar[iOutput] = iValue;
	}
}

void CLauncher::SetClipTransition(int iTrack, int iRow, int iOutput, int iTransition) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kClipSettings);
	if (settings && IsValidTransition(iTransition)) {
		settings->fTransitions[iOutput] = iTransition;
	}
}

void CLauncher::SetFillInPresent(int iTrack, int iRow, bool iPresent) {
	if (IsValidButton(iTrack, iRow)) {
		fClips[iTrack][iRow - 1].fFillIn.fPresent = iPresent;
	}
}

void CLauncher::SetFillInColour(int iTrack, int iRow, int iColour) {
	if (IsValidButton(iTrack, iRow) && iColour >= 0 && iColour < kColourCount) {
		fClips[iTrack][iRow - 1].fFillIn.fColour = iColour;
	}
}

void CLauncher::SetFillInPattern(int iTrack, int iRow, int iOutput, int iPattern) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kFillInSettings);
	if (settings && iPattern >= 0 && iPattern < kPatternCount) {
		settings->fPatterns[iOutput] = iPattern;
	}
}

void CLauncher::SetFillInUnipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kFillInSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fUnipolar[iOutput] = iValue;
	}
}

void CLauncher::SetFillInBipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kFillInSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fBipolar[iOutput] = iValue;
	}
}

void CLauncher::SetFillInTransition(int iTrack, int iRow, int iOutput, int iTransition) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kFillInSettings);
	if (settings && IsValidTransition(iTransition)) {
		settings->fTransitions[iOutput] = iTransition;
	}
}

void CLauncher::SetLandingPresent(int iTrack, int iRow, bool iPresent) {
	TSettings* settings = EditableSettings(iTrack, iRow, kLandingSettings);
	if (settings) {
		settings->fPresent = iPresent;
	}
}

void CLauncher::SetLandingColour(int iTrack, int iRow, int iColour) {
	TSettings* settings = EditableSettings(iTrack, iRow, kLandingSettings);
	if (settings && iColour >= 0 && iColour < kColourCount) {
		settings->fColour = iColour;
	}
}

void CLauncher::SetLandingPattern(int iTrack, int iRow, int iOutput, int iPattern) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kLandingSettings);
	if (settings && iPattern >= 0 && iPattern < kPatternCount) {
		settings->fPatterns[iOutput] = iPattern;
	}
}

void CLauncher::SetLandingUnipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kLandingSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fUnipolar[iOutput] = iValue;
	}
}

void CLauncher::SetLandingBipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kLandingSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fBipolar[iOutput] = iValue;
	}
}

void CLauncher::SetLandingTransition(int iTrack, int iRow, int iOutput, int iTransition) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kLandingSettings);
	if (settings && IsValidTransition(iTransition)) {
		settings->fTransitions[iOutput] = iTransition;
	}
}

void CLauncher::SetFillInOneBarPattern(int iTrack, int iRow, bool iOn) {
	TSettings* settings = EditableSettings(iTrack, iRow, kFillInSettings);
	if (settings) {
		settings->fOneBarPattern = iOn;
	}
}

void CLauncher::SetLandingOneBarPattern(int iTrack, int iRow, bool iOn) {
	TSettings* settings = EditableSettings(iTrack, iRow, kLandingSettings);
	if (settings) {
		settings->fOneBarPattern = iOn;
	}
}

void CLauncher::SetIntroPresent(int iTrack, int iRow, bool iPresent) {
	TSettings* settings = EditableSettings(iTrack, iRow, kIntroSettings);
	if (settings) {
		settings->fPresent = iPresent;
	}
}

void CLauncher::SetIntroColour(int iTrack, int iRow, int iColour) {
	TSettings* settings = EditableSettings(iTrack, iRow, kIntroSettings);
	if (settings && iColour >= 0 && iColour < kColourCount) {
		settings->fColour = iColour;
	}
}

void CLauncher::SetIntroPattern(int iTrack, int iRow, int iOutput, int iPattern) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kIntroSettings);
	if (settings && iPattern >= 0 && iPattern < kPatternCount) {
		settings->fPatterns[iOutput] = iPattern;
	}
}

void CLauncher::SetIntroUnipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kIntroSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fUnipolar[iOutput] = iValue;
	}
}

void CLauncher::SetIntroBipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kIntroSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fBipolar[iOutput] = iValue;
	}
}

void CLauncher::SetIntroTransition(int iTrack, int iRow, int iOutput, int iTransition) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kIntroSettings);
	if (settings && IsValidTransition(iTransition)) {
		settings->fTransitions[iOutput] = iTransition;
	}
}

void CLauncher::SetIntroOneBarPattern(int iTrack, int iRow, bool iOn) {
	TSettings* settings = EditableSettings(iTrack, iRow, kIntroSettings);
	if (settings) {
		settings->fOneBarPattern = iOn;
	}
}

void CLauncher::SetOutroPresent(int iTrack, int iRow, bool iPresent) {
	TSettings* settings = EditableSettings(iTrack, iRow, kOutroSettings);
	if (settings) {
		settings->fPresent = iPresent;
	}
}

void CLauncher::SetOutroColour(int iTrack, int iRow, int iColour) {
	TSettings* settings = EditableSettings(iTrack, iRow, kOutroSettings);
	if (settings && iColour >= 0 && iColour < kColourCount) {
		settings->fColour = iColour;
	}
}

void CLauncher::SetOutroPattern(int iTrack, int iRow, int iOutput, int iPattern) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kOutroSettings);
	if (settings && iPattern >= 0 && iPattern < kPatternCount) {
		settings->fPatterns[iOutput] = iPattern;
	}
}

void CLauncher::SetOutroUnipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kOutroSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fUnipolar[iOutput] = iValue;
	}
}

void CLauncher::SetOutroBipolar(int iTrack, int iRow, int iOutput, int iValue) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kOutroSettings);
	if (settings && IsValidValue(iValue)) {
		settings->fBipolar[iOutput] = iValue;
	}
}

void CLauncher::SetOutroTransition(int iTrack, int iRow, int iOutput, int iTransition) {
	TSettings* settings = EditableSettings(iTrack, iRow, iOutput, kOutroSettings);
	if (settings && IsValidTransition(iTransition)) {
		settings->fTransitions[iOutput] = iTransition;
	}
}

void CLauncher::SetOutroOneBarPattern(int iTrack, int iRow, bool iOn) {
	TSettings* settings = EditableSettings(iTrack, iRow, kOutroSettings);
	if (settings) {
		settings->fOneBarPattern = iOn;
	}
}

int CLauncher::DefaultCVMode(int iOutput) {
	return iOutput == 0 ? kCVModePattern : kCVModeUnipolar;
}

void CLauncher::SetCVMode(int iTrack, int iOutput, int iCVMode) {
	if (iTrack >= 0 && iTrack < kTrackCount && IsValidOutput(iOutput) && iCVMode >= 0 && iCVMode < kCVModeCount) {
		fCVModes[iTrack][iOutput] = iCVMode;
	}
}

int CLauncher::GetCVMode(int iTrack, int iOutput) const {
	return fCVModes[iTrack][iOutput];
}

void CLauncher::SetFillInPressed(bool iPressed) {
	if (iPressed && !fFillInPressed) {
		fFillInTriggered = true;
	}
	fFillInPressed = iPressed;
}

bool CLauncher::IsFillInPlaying() const {
	return fFillInPlaying;
}

void CLauncher::SetRelaunchPressed(int iTrack, bool iPressed) {
	if (iTrack < 0 || iTrack >= kTrackCount) {
		return;
	}
	if (iPressed && !fRelaunchPressed[iTrack]) {
		fRelaunchTriggered[iTrack] = true;
	}
	fRelaunchPressed[iTrack] = iPressed;
}

bool CLauncher::IsRelaunched(int iTrack) const {
	return fRelaunched[iTrack];
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

void CLauncher::SetFunction(int iFunction) {
	if (iFunction >= 0 && iFunction < kFunctionCount) {
		fFunction = iFunction;
	}
}

int CLauncher::GetFunction() const {
	return fFunction;
}

void CLauncher::SetMutePressed(int iTrack, bool iPressed) {
	if (iTrack < 0 || iTrack >= kTrackCount) {
		return;
	}
	if (iPressed && !fMutePressed[iTrack]) {
		fMuteTriggered[iTrack] = !fMuteTriggered[iTrack];
	}
	fMutePressed[iTrack] = iPressed;
}

void CLauncher::SetSoloPressed(int iTrack, bool iPressed) {
	if (iTrack < 0 || iTrack >= kTrackCount) {
		return;
	}
	if (iPressed && !fSoloPressed[iTrack]) {
		fSoloTriggered[iTrack] = !fSoloTriggered[iTrack];
	}
	fSoloPressed[iTrack] = iPressed;
}

bool CLauncher::IsMuted(int iTrack) const {
	return fMuted[iTrack];
}

bool CLauncher::IsSoloed(int iTrack) const {
	return fSoloed[iTrack];
}

bool CLauncher::IsSilenced(int iTrack) const {
	if (fSoloed[iTrack]) {
		return false;
	}
	if (fMuted[iTrack]) {
		return true;
	}
	for (int track = 0; track < kTrackCount; ++track) {
		if (fSoloed[track]) {
			return true;
		}
	}
	return false;
}

void CLauncher::EndMutesAndSolos() {
	for (int track = 0; track < kTrackCount; ++track) {
		fMuted[track] = false;
		fSoloed[track] = false;
	}
}

bool CLauncher::IsJump(const TTransport& iTransport) const {
	if (!fHasPreviousTransport) {
		return false;
	}
	double expected = fPreviousTransport.fPlayPos;
	if (fPreviousTransport.fPlaying) {
		expected += fPreviousTransport.fBatchLength;
	}
	const double distance = iTransport.fPlayPos - expected;
	return distance > kJumpTolerance || distance < -kJumpTolerance;
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
		if (fActive[track] != kSelectionStop) {
			fLastActive[track] = fActive[track];
		}
		for (int output = 0; output < kOutputCount; ++output) {
			fEmitted[track][output] = fActive[track] == kSelectionStop ? 0 : Clip(track, fActive[track]).fPatterns[output] + 1;
		}
	}
}

void CLauncher::Open() {
	Switch();
}

void CLauncher::Advance(const TTransport& iTransport) {
	bool switchNow = true;
	bool switchPointInBatch = false;
	const bool switchPointInPreviousBatch = fSwitchPointInPreviousBatch;

	if (iTransport.fPlaying) {
		switchPointInBatch = ContainsSwitchPoint(iTransport);
		// A change that arrives one batch after a switch point still counts as
		// landing on it, since the host does not say where in a batch it happened.
		// With the length immediate, nothing waits for a switch point.
		switchNow = switchPointInBatch || switchPointInPreviousBatch || fLength == kLengthImmediate;

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

	// A clip whose fill-in ends at a switch point lands, unless another clip
	// takes over there.
	bool fillInEnds[kTrackCount];
	for (int track = 0; track < kTrackCount; ++track) {
		fillInEnds[track] = switchPointInBatch && PlaysFillIn(track);
	}

	// A fill-in ends at the switch point itself, not one batch later, and only
	// while the song plays through it; with the length immediate there is none
	// to end at, so nothing fills in.
	if (switchPointInBatch || fLength == kLengthImmediate) {
		fFillInPlaying = false;
	}
	if (fFillInTriggered && fLength != kLengthImmediate) {
		fFillInPlaying = true;
	}
	fFillInTriggered = false;

	// Mutes and solos end at the switch point itself, like a fill-in, and when
	// the song position jumps, so that playing back part of a song does not
	// depend on presses made before it. A press in the same batch counts after
	// that, and so lasts until the next switch point.
	const bool jump = IsJump(iTransport);
	if (switchPointInBatch || jump) {
		EndMutesAndSolos();
	}
	fHasPreviousTransport = true;
	fPreviousTransport = iTransport;
	for (int track = 0; track < kTrackCount; ++track) {
		// A track without clips can be neither muted nor soloed.
		const bool hasClips = HasClips(track);
		if (fMuteTriggered[track] && hasClips) {
			fMuted[track] = !fMuted[track];
		}
		if (fSoloTriggered[track] && hasClips) {
			fSoloed[track] = !fSoloed[track];
		}
		fMuteTriggered[track] = false;
		fSoloTriggered[track] = false;
		if (!hasClips) {
			fMuted[track] = false;
			fSoloed[track] = false;
		}
	}

	// The interval meter follows the song position whether it moves or not.
	fReachedSegments = CountReachedSegments(iTransport);

	// A launch of the active clip relaunches it, before any selection takes
	// effect: a clip that becomes active at once, while the transport is
	// stopped, has not been relaunched. With the length immediate nothing
	// waits for a switch point, so nothing lands.
	for (int track = 0; track < kTrackCount; ++track) {
		if (fRelaunchTriggered[track] && fLength != kLengthImmediate && fActive[track] != kSelectionStop && Target(track) == fActive[track]) {
			fRelaunched[track] = true;
		}
		fRelaunchTriggered[track] = false;
	}

	// A landing ends at the switch point, like a fill-in, and when the song
	// position jumps; with the length immediate there are none. So does an
	// outro, whose values a stopped track goes on emitting after the switch
	// point; a jump or the length immediate drops it, and an intro under way
	// too, and the track holds what it emits.
	for (int track = 0; track < kTrackCount; ++track) {
		if (switchPointInBatch || jump || fLength == kLengthImmediate) {
			fLandingRows[track] = kSelectionStop;
			if (fOutroPlaying[track] && (jump || fLength == kLengthImmediate)) {
				Freeze(track);
				fOutroRows[track] = kSelectionStop;
			}
			fOutroPlaying[track] = false;
		}
		if (jump || fLength == kLengthImmediate) {
			if (PlaysIntro(track)) {
				Freeze(track);
			}
			fIntroPlaying[track] = false;
		}
	}

	// While the transport is stopped, and with the length immediate, selections
	// and changes to the active clip take effect at once.
	if (switchNow) {
		int previous[kTrackCount];
		for (int track = 0; track < kTrackCount; ++track) {
			previous[track] = fActive[track];
		}
		Switch();
		// A clip that becomes active at a switch point, while the song plays,
		// lands, and so does one that goes on there after its fill-in or
		// after it was relaunched; one that becomes active any other way does
		// not, and a track that changes otherwise stops landing.
		const bool atSwitchPoint = iTransport.fPlaying && (switchPointInBatch || switchPointInPreviousBatch) && fLength != kLengthImmediate;
		for (int track = 0; track < kTrackCount; ++track) {
			if (fActive[track] == previous[track]) {
				if (atSwitchPoint && (fillInEnds[track] || fRelaunched[track]) && fActive[track] != kSelectionStop) {
					fLandingRows[track] = fActive[track];
					fLandingStarts[track] = LastSwitchPoint(iTransport, switchPointInBatch);
				}
				if (atSwitchPoint) {
					fRelaunched[track] = false;
				}
				continue;
			}
			fLandingRows[track] = kSelectionStop;
			fRelaunched[track] = false;
			if (atSwitchPoint && fActive[track] != kSelectionStop) {
				fLandingRows[track] = fActive[track];
				fLandingStarts[track] = LastSwitchPoint(iTransport, switchPointInBatch);
			}
			// A clip that gives way to stop at a switch point plays its outro,
			// from what it emitted until then.
			if (atSwitchPoint && fActive[track] == kSelectionStop && previous[track] != kSelectionStop) {
				const TClip& clip = Clip(track, previous[track]);
				if (clip.fPresent && clip.fOutro.fPresent) {
					fOutroRows[track] = previous[track];
					fOutroStarts[track] = LastSwitchPoint(iTransport, switchPointInBatch);
					fOutroPlaying[track] = true;
					fFrozen[track] = false;
					for (int output = 0; output < kOutputCount; ++output) {
						fOutroFrom[track][output] = fValues[track][output];
						fOutroFromModes[track][output] = fCVModes[track][output];
					}
				}
			}
		}
	}
	// A clip that becomes active leaves nothing of an intro or an outro
	// behind. On a stopped track, an intro starts whenever another clip is
	// queued, while the song plays; taking it back drops it, and the track
	// holds what it emits. An intro cuts an outro short.
	for (int track = 0; track < kTrackCount; ++track) {
		if (fActive[track] != kSelectionStop) {
			ForgetIntroAndOutro(track);
			continue;
		}
		const int target = Target(track);
		if (target != fIntroRows[track]) {
			if (PlaysIntroOf(track, fIntroRows[track])) {
				Freeze(track);
			}
			fIntroRows[track] = target;
			fIntroStarts[track] = iTransport.fPlayPos;
			fIntroPlaying[track] = target != kSelectionStop && iTransport.fPlaying && fLength != kLengthImmediate;
		}
		if (PlaysIntro(track)) {
			fOutroRows[track] = kSelectionStop;
			fOutroPlaying[track] = false;
			fFrozen[track] = false;
		}
	}
	// Queueing another clip or stop takes a relaunch back, and so does the
	// length immediate.
	for (int track = 0; track < kTrackCount; ++track) {
		if (Target(track) != fActive[track] || fLength == kLengthImmediate) {
			fRelaunched[track] = false;
		}
	}
	UpdateValues(iTransport);
}

double CLauncher::UnipolarCV(int iValue) {
	return static_cast<double>(iValue) / (kValueCount - 1);
}

double CLauncher::BipolarCV(int iValue) {
	return static_cast<double>(iValue - kBipolarZero) / kBipolarZero;
}

double CLauncher::TransitionCurve(int iTransition, double iTime) {
	if (iTime <= 0.0) {
		return 0.0;
	}
	if (iTime >= 1.0) {
		return 1.0;
	}
	if (iTransition == kTransitionB || iTransition == kTransitionD) {
		return (std::exp(kCurveSteepness * iTime) - 1.0) / (std::exp(kCurveSteepness) - 1.0);
	}
	return iTime;
}

double CLauncher::LandingCurve(int iTransition, double iTime) {
	return 1.0 - TransitionCurve(iTransition, 1.0 - iTime);
}

int CLauncher::ValueIndex(const TSettings& iSettings, int iOutput, int iCVMode) {
	return iCVMode == kCVModeBipolar ? iSettings.fBipolar[iOutput] : iSettings.fUnipolar[iOutput];
}

double CLauncher::ValueCV(const TSettings& iSettings, int iOutput, int iCVMode) {
	const int value = ValueIndex(iSettings, iOutput, iCVMode);
	return iCVMode == kCVModeBipolar ? BipolarCV(value) : UnipolarCV(value);
}

double CLauncher::BarLength(const TTransport& iTransport) {
	int numerator = iTransport.fTimeSignatureNumerator;
	int denominator = iTransport.fTimeSignatureDenominator;
	if (numerator <= 0 || denominator <= 0) {
		numerator = 4;
		denominator = 4;
	}
	return static_cast<double>(numerator * (kPPQPerCrotchet * 4 / denominator));
}

double CLauncher::LastSwitchPoint(const TTransport& iTransport, bool iInBatch) const {
	const double intervalLength = static_cast<double>(ComputeSwitchIntervalLength(fLength, iTransport.fTimeSignatureNumerator, iTransport.fTimeSignatureDenominator));
	if (intervalLength <= 0.0) {
		return iTransport.fPlayPos;
	}
	const double intervals = iTransport.fPlayPos / intervalLength;
	return (iInBatch ? std::ceil(intervals) : std::floor(intervals)) * intervalLength;
}

double CLauncher::NextSwitchPoint(double iPosition, const TTransport& iTransport) const {
	const double intervalLength = static_cast<double>(ComputeSwitchIntervalLength(fLength, iTransport.fTimeSignatureNumerator, iTransport.fTimeSignatureDenominator));
	if (intervalLength <= 0.0) {
		return iPosition;
	}
	return (std::floor(iPosition / intervalLength) + 1.0) * intervalLength;
}

void CLauncher::StartOver(TMovement& ioMovement, double iEmitted, int iRow, int iTransition, int iValue, int iCVMode, double iPosition, const TTransport& iTransport) const {
	// A movement that had begun goes on from where it is. One that had not,
	// or whose CV mode has changed, starts from the active clip's value.
	const bool begun = ioMovement.fOn && (ioMovement.fFromFrozen || iPosition >= ioMovement.fStart);
	ioMovement.fFromFrozen = begun && ioMovement.fCVMode == iCVMode;
	ioMovement.fFrom = iEmitted;
	ioMovement.fOn = true;
	ioMovement.fRow = iRow;
	ioMovement.fTransition = iTransition;
	ioMovement.fValue = iValue;
	ioMovement.fCVMode = iCVMode;
	ioMovement.fStart = iPosition;
	// C and D move only within the last bar before the switch point.
	if (iTransition == kTransitionC || iTransition == kTransitionD) {
		const double lastBar = NextSwitchPoint(iPosition, iTransport) - BarLength(iTransport);
		if (lastBar > iPosition) {
			ioMovement.fStart = lastBar;
		}
	}
}

double CLauncher::MovementValue(const TMovement& iMovement, double iActiveValue, double iTo, double iPosition, const TTransport& iTransport) const {
	const double from = iMovement.fFromFrozen ? iMovement.fFrom : iActiveValue;
	// After a jump of the song position it lands on the switch point ahead.
	const double end = NextSwitchPoint(iPosition, iTransport);
	if (iPosition <= iMovement.fStart || end <= iMovement.fStart) {
		return from;
	}
	const double time = (iPosition - iMovement.fStart) / (end - iMovement.fStart);
	return from + (iTo - from) * TransitionCurve(iMovement.fTransition, time);
}

double CLauncher::ComputeClipValue(int iTrack, int iOutput, const TTransport& iTransport) {
	TMovement& movement = fMovements[iTrack][iOutput];
	const int cvMode = fCVModes[iTrack][iOutput];
	const int active = fActive[iTrack];
	if (active == kSelectionStop) {
		movement = kNoMovement;
		return ComputeStoppedValue(iTrack, iOutput, iTransport);
	}
	const TClip& clip = Clip(iTrack, active);
	// While the clip lands, the landing's movement stands in for its value.
	const bool landing = PlaysLanding(iTrack);
	const double activeValue = landing ? ComputeLandingValue(iTrack, iOutput, iTransport) : ValueCV(clip, iOutput, cvMode);
	// The active clip's transition moves towards a queued clip, never towards
	// stop; with the length immediate nothing is ever queued.
	const int target = Target(iTrack);
	const int transition = clip.fTransitions[iOutput];
	if (!iTransport.fPlaying || fLength == kLengthImmediate || target == kSelectionStop || target == active || transition == kTransitionOff) {
		movement = kNoMovement;
		return activeValue;
	}
	// It moves towards the queued clip's landing, if it holds one.
	const TSettings& targetSettings = ArrivalSettings(iTrack, target);
	const int value = ValueIndex(targetSettings, iOutput, cvMode);
	if (!movement.fOn || movement.fRow != target || movement.fTransition != transition || movement.fValue != value || movement.fCVMode != cvMode) {
		StartOver(movement, fClipValues[iTrack][iOutput], target, transition, value, cvMode, iTransport.fPlayPos, iTransport);
		// A landing under way is dropped: the transition starts over from
		// what is emitted now, even before it begins to move.
		if (landing) {
			movement.fFromFrozen = true;
		}
	}
	return MovementValue(movement, activeValue, ValueCV(targetSettings, iOutput, cvMode), iTransport.fPlayPos, iTransport);
}

const CLauncher::TSettings& CLauncher::ArrivalSettings(int iTrack, int iRow) const {
	const TClip& clip = Clip(iTrack, iRow);
	return clip.fLanding.fPresent ? clip.fLanding : clip;
}

double CLauncher::ComputeStoppedValue(int iTrack, int iOutput, const TTransport& iTransport) const {
	const int cvMode = fCVModes[iTrack][iOutput];
	// An intro moves from its own value towards what its clip arrives with,
	// the way a clip's transition does: A and B from the queue, C and D only
	// within the last bar before the switch point.
	if (PlaysIntro(iTrack)) {
		const int row = fIntroRows[iTrack];
		const TSettings& intro = Clip(iTrack, row).fIntro;
		const double from = ValueCV(intro, iOutput, cvMode);
		const int transition = intro.fTransitions[iOutput];
		if (transition == kTransitionOff) {
			return from;
		}
		const double end = NextSwitchPoint(iTransport.fPlayPos, iTransport);
		double start = fIntroStarts[iTrack];
		if ((transition == kTransitionC || transition == kTransitionD) && end - BarLength(iTransport) > start) {
			start = end - BarLength(iTransport);
		}
		if (iTransport.fPlayPos <= start || end <= start) {
			return from;
		}
		const double to = ValueCV(ArrivalSettings(iTrack, row), iOutput, cvMode);
		return from + (to - from) * TransitionCurve(transition, (iTransport.fPlayPos - start) / (end - start));
	}
	// An outro moves from what was emitted before it towards its own value,
	// the way a landing's transition runs: A and B across the whole switch
	// interval, C and D within its first bar only. Once it is over, the track
	// holds its value.
	const int outroRow = fOutroRows[iTrack];
	if (outroRow != kSelectionStop && Clip(iTrack, outroRow).fPresent && Clip(iTrack, outroRow).fOutro.fPresent) {
		const TSettings& outro = Clip(iTrack, outroRow).fOutro;
		const double to = ValueCV(outro, iOutput, cvMode);
		const int transition = outro.fTransitions[iOutput];
		// A change of the CV mode drops the movement.
		if (!fOutroPlaying[iTrack] || transition == kTransitionOff || fOutroFromModes[iTrack][iOutput] != cvMode) {
			return to;
		}
		double span = static_cast<double>(ComputeSwitchIntervalLength(fLength, iTransport.fTimeSignatureNumerator, iTransport.fTimeSignatureDenominator));
		if ((transition == kTransitionC || transition == kTransitionD) && BarLength(iTransport) < span) {
			span = BarLength(iTransport);
		}
		if (span <= 0.0) {
			return to;
		}
		const double from = fOutroFrom[iTrack][iOutput];
		return from + (to - from) * LandingCurve(transition, (iTransport.fPlayPos - fOutroStarts[iTrack]) / span);
	}
	if (fFrozen[iTrack]) {
		return fFrozenValues[iTrack][iOutput];
	}
	// Otherwise a stopped track goes on with the value of the clip it last
	// played.
	return fLastActive[iTrack] == kSelectionStop ? 0.0 : ValueCV(Clip(iTrack, fLastActive[iTrack]), iOutput, cvMode);
}

void CLauncher::Freeze(int iTrack) {
	fFrozen[iTrack] = true;
	for (int output = 0; output < kOutputCount; ++output) {
		fFrozenValues[iTrack][output] = fValues[iTrack][output];
	}
}

void CLauncher::ForgetIntroAndOutro(int iTrack) {
	fIntroRows[iTrack] = kSelectionStop;
	fIntroPlaying[iTrack] = false;
	fOutroRows[iTrack] = kSelectionStop;
	fOutroPlaying[iTrack] = false;
	fFrozen[iTrack] = false;
}

double CLauncher::ComputeLandingValue(int iTrack, int iOutput, const TTransport& iTransport) const {
	const int cvMode = fCVModes[iTrack][iOutput];
	const TClip& clip = Clip(iTrack, fActive[iTrack]);
	const double from = ValueCV(clip.fLanding, iOutput, cvMode);
	const int transition = clip.fLanding.fTransitions[iOutput];
	if (transition == kTransitionOff) {
		return from;
	}
	// A and B move across the whole switch interval, C and D within its first
	// bar only, so with a length of one bar or less they are alike.
	double span = static_cast<double>(ComputeSwitchIntervalLength(fLength, iTransport.fTimeSignatureNumerator, iTransport.fTimeSignatureDenominator));
	if ((transition == kTransitionC || transition == kTransitionD) && BarLength(iTransport) < span) {
		span = BarLength(iTransport);
	}
	if (span <= 0.0) {
		return from;
	}
	const double time = (iTransport.fPlayPos - fLandingStarts[iTrack]) / span;
	const double to = ValueCV(clip, iOutput, cvMode);
	return from + (to - from) * LandingCurve(transition, time);
}

double CLauncher::ComputeFillInValue(int iTrack, int iOutput, double iBelow, const TTransport& iTransport) {
	TMovement& movement = fFillInMovements[iTrack][iOutput];
	if (!PlaysFillIn(iTrack)) {
		movement = kNoMovement;
		return iBelow;
	}
	// A fill-in moves from whatever is emitted at the press to its own value,
	// and hides any transition of its clip meanwhile.
	const int cvMode = fCVModes[iTrack][iOutput];
	const int active = fActive[iTrack];
	const TSettings& fillIn = Clip(iTrack, active).fFillIn;
	const int transition = fillIn.fTransitions[iOutput];
	const int value = ValueIndex(fillIn, iOutput, cvMode);
	if (!movement.fOn || movement.fRow != active || movement.fTransition != transition || movement.fValue != value || movement.fCVMode != cvMode) {
		// Unlike a clip's, a fill-in's transition always starts from what is
		// emitted at that moment: what the clip emits, unless the fill-in was
		// already playing in the same CV mode.
		const double from = movement.fOn && movement.fCVMode == cvMode ? fValues[iTrack][iOutput] : iBelow;
		StartOver(movement, from, active, transition, value, cvMode, iTransport.fPlayPos, iTransport);
		movement.fFromFrozen = true;
	}
	const double fillInValue = ValueCV(fillIn, iOutput, cvMode);
	if (transition == kTransitionOff) {
		return fillInValue;
	}
	return MovementValue(movement, iBelow, fillInValue, iTransport.fPlayPos, iTransport);
}

void CLauncher::UpdateValues(const TTransport& iTransport) {
	for (int track = 0; track < kTrackCount; ++track) {
		for (int output = 0; output < kOutputCount; ++output) {
			if (fCVModes[track][output] == kCVModePattern) {
				fMovements[track][output] = kNoMovement;
				fFillInMovements[track][output] = kNoMovement;
				continue;
			}
			const double clipValue = ComputeClipValue(track, output, iTransport);
			fClipValues[track][output] = clipValue;
			fValues[track][output] = ComputeFillInValue(track, output, clipValue, iTransport);
		}
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
	if (active == kSelectionStop || !Clip(iTrack, active).fPresent) {
		return false;
	}
	for (int output = 0; output < kOutputCount; ++output) {
		if (fCVModes[iTrack][output] == kCVModePattern && Clip(iTrack, active).fPatterns[output] + 1 != fEmitted[iTrack][output]) {
			return true;
		}
	}
	return false;
}

bool CLauncher::PlaysLanding(int iTrack) const {
	const int active = fActive[iTrack];
	if (fLength == kLengthImmediate || active == kSelectionStop || fLandingRows[iTrack] != active) {
		return false;
	}
	const TClip& clip = Clip(iTrack, active);
	return clip.fPresent && clip.fLanding.fPresent;
}

bool CLauncher::PlaysIntroOf(int iTrack, int iRow) const {
	if (!fIntroPlaying[iTrack] || iRow == kSelectionStop || fActive[iTrack] != kSelectionStop || fLength == kLengthImmediate) {
		return false;
	}
	const TClip& clip = Clip(iTrack, iRow);
	return clip.fPresent && clip.fIntro.fPresent;
}

bool CLauncher::PlaysIntro(int iTrack) const {
	return Target(iTrack) == fIntroRows[iTrack] && PlaysIntroOf(iTrack, fIntroRows[iTrack]);
}

bool CLauncher::PlaysOutro(int iTrack) const {
	const int row = fOutroRows[iTrack];
	if (!fOutroPlaying[iTrack] || row == kSelectionStop || fActive[iTrack] != kSelectionStop) {
		return false;
	}
	const TClip& clip = Clip(iTrack, row);
	return clip.fPresent && clip.fOutro.fPresent;
}

bool CLauncher::PlaysIntroPatterns(int iTrack) const {
	if (!PlaysIntro(iTrack)) {
		return false;
	}
	if (!Clip(iTrack, fIntroRows[iTrack]).fIntro.fOneBarPattern) {
		return true;
	}
	const double position = fPreviousTransport.fPlayPos;
	return position >= NextSwitchPoint(position, fPreviousTransport) - BarLength(fPreviousTransport);
}

bool CLauncher::PlaysOutroPatterns(int iTrack) const {
	if (!PlaysOutro(iTrack)) {
		return false;
	}
	if (!Clip(iTrack, fOutroRows[iTrack]).fOutro.fOneBarPattern) {
		return true;
	}
	return fPreviousTransport.fPlayPos < fOutroStarts[iTrack] + BarLength(fPreviousTransport);
}

bool CLauncher::PlaysLandingPatterns(int iTrack) const {
	if (!PlaysLanding(iTrack)) {
		return false;
	}
	if (!Clip(iTrack, fActive[iTrack]).fLanding.fOneBarPattern) {
		return true;
	}
	return fPreviousTransport.fPlayPos < fLandingStarts[iTrack] + BarLength(fPreviousTransport);
}

bool CLauncher::PlaysFillInPatterns(int iTrack) const {
	if (!PlaysFillIn(iTrack)) {
		return false;
	}
	if (!Clip(iTrack, fActive[iTrack]).fFillIn.fOneBarPattern) {
		return true;
	}
	const double position = fPreviousTransport.fPlayPos;
	return position >= NextSwitchPoint(position, fPreviousTransport) - BarLength(fPreviousTransport);
}

bool CLauncher::PlaysFillIn(int iTrack) const {
	const int active = fActive[iTrack];
	if (!fFillInPlaying || active == kSelectionStop) {
		return false;
	}
	const TClip& clip = Clip(iTrack, active);
	return clip.fPresent && clip.fFillIn.fPresent;
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

int CLauncher::FunctionColour(int iFunction) {
	if (iFunction == kFunctionMute) {
		return kColourYellow;
	}
	if (iFunction == kFunctionSolo) {
		return kColourBlue;
	}
	return kColourRed;
}

CLauncher::TLight CLauncher::GetFunctionLight(int iTrack) const {
	if (fFunction == kFunctionStop) {
		const EBrightness brightness = GetStopLight(iTrack);
		return brightness == kUnlit ? kUnlitLight : MakeLight(brightness, kColourRed);
	}
	if (!HasClips(iTrack)) {
		return kUnlitLight;
	}
	// A mute button lights up while there is something to mute, a solo button
	// while its track is soloed.
	if (fFunction == kFunctionMute) {
		return MakeLight(fMuted[iTrack] ? kDim : kVibrant, kColourYellow);
	}
	return MakeLight(fSoloed[iTrack] ? kVibrant : kDim, kColourBlue);
}

int CLauncher::GetFunctionLightValue(int iTrack) const {
	return GetFunctionLight(iTrack).fBrightness;
}

int CLauncher::GetFunctionLightFrame(int iTrack) const {
	return LightFrame(GetFunctionLight(iTrack));
}

CLauncher::TLight CLauncher::GetFunctionSelectLight(int iFunction) const {
	if (iFunction == fFunction) {
		return MakeLight(kVibrant, FunctionColour(iFunction));
	}
	return MakeLight(kDim, kColourWhite);
}

int CLauncher::GetFunctionSelectLightFrame(int iFunction) const {
	return LightFrame(GetFunctionSelectLight(iFunction));
}

int CLauncher::GetFunctionSelectCaptionFrame(int iFunction) const {
	return iFunction == fFunction ? 1 : 0;
}

bool CLauncher::ShowsVibrant(const TLight& iLight) const {
	return iLight.fBrightness == kVibrant || (iLight.fBrightness == kFlashing && fFlashLit);
}

int CLauncher::LightFrame(const TLight& iLight) const {
	if (iLight.fBrightness == kUnlit) {
		return 0;
	}
	return 1 + 2 * iLight.fColour + (ShowsVibrant(iLight) ? 1 : 0);
}

CLauncher::TLight CLauncher::GetLight(int iTrack, int iRow) const {
	if (fSetup == kSetupLaunchMode) {
		return GetLaunchModeLight(iTrack, iRow);
	}
	if (fSetup == kSetupClipPicking) {
		return GetClipPickingLight(iTrack, iRow);
	}
	if (fSetup == kSetupClearMode) {
		return GetClearModeLight(iTrack, iRow);
	}
	if (IsDuplicateMode(fSetup)) {
		return GetDuplicateModeLight(iTrack, iRow);
	}
	// Clip editing, fill-in editing, landing editing, intro editing and outro
	// editing are laid out and lit the same: only the labels tell them apart.
	return GetEditingLight(iTrack, iRow, EditedSettings());
}

CLauncher::TLight CLauncher::GetLaunchModeLight(int iTrack, int iRow) const {
	const TClip& clip = Clip(iTrack, iRow);
	switch (GetButtonState(iTrack, iRow)) {
		// An active clip flashes while a change of its pattern waits for the switch point.
		// While it plays its fill-in, it flashes in the fill-in's colour, and
		// while it lands it shows the landing's colour. Relaunched, it flashes
		// in the landing's colour until the switch point.
		case kActive:
			if (PlaysFillIn(iTrack)) {
				return MakeLight(kFlashing, clip.fFillIn.fColour);
			}
			if (fRelaunched[iTrack] && clip.fPresent && clip.fLanding.fPresent) {
				return MakeLight(kFlashing, clip.fLanding.fColour);
			}
			return MakeLight(HasPendingPattern(iTrack) ? kFlashing : kVibrant, PlaysLanding(iTrack) ? clip.fLanding.fColour : clip.fColour);
		// A queued clip flashes in its intro's colour while it plays.
		case kQueued:
			return MakeLight(kFlashing, PlaysIntro(iTrack) ? clip.fIntro.fColour : clip.fColour);
		// The clip that was stopped shows its outro's colour while it plays.
		default:
			if (PlaysOutro(iTrack) && fOutroRows[iTrack] == iRow) {
				return MakeLight(kVibrant, clip.fOutro.fColour);
			}
			return clip.fPresent ? MakeLight(kDim, clip.fColour) : kUnlitLight;
	}
}

CLauncher::TLight CLauncher::GetClipPickingLight(int iTrack, int iRow) const {
	const TClip& clip = Clip(iTrack, iRow);
	return MakeLight(kFlashing, clip.fPresent ? clip.fColour : kColourWhite);
}

// Only what there is to clear flashes, in its own colour, so that the user
// can tell the clips apart.
CLauncher::TLight CLauncher::GetClearModeLight(int iTrack, int iRow) const {
	const TClip& clip = Clip(iTrack, iRow);
	return clip.fPresent ? MakeLight(kFlashing, clip.fColour) : kUnlitLight;
}

// In source picking what there is to copy flashes; in destination picking for
// a launch button the source stands still and everything else flashes, empty
// launch buttons in white. With a scene as the source, the launch buttons are
// unlit.
CLauncher::TLight CLauncher::GetDuplicateModeLight(int iTrack, int iRow) const {
	const TClip& clip = Clip(iTrack, iRow);
	if (fSetup == kSetupSourcePicking) {
		return clip.fPresent ? MakeLight(kFlashing, clip.fColour) : kUnlitLight;
	}
	const int source = SourceButton(fSetup);
	if (source < 0) {
		return kUnlitLight;
	}
	if (source == ButtonIndex(iTrack, iRow)) {
		return MakeLight(kVibrant, clip.fColour);
	}
	return MakeLight(kFlashing, clip.fPresent ? clip.fColour : kColourWhite);
}

bool CLauncher::SceneHasClips(int iScene) const {
	for (int track = 0; track < kTrackCount; ++track) {
		if (Clip(track, iScene).fPresent) {
			return true;
		}
	}
	return false;
}

const CLauncher::TClip& CLauncher::PickedClip() const {
	const int picked = PickedButton(fSetup);
	return fClips[picked / kRowCount][picked % kRowCount];
}

int CLauncher::PickedTrack() const {
	return PickedButton(fSetup) / kRowCount;
}

const CLauncher::TSettings& CLauncher::EditedSettings() const {
	const TClip& clip = PickedClip();
	if (IsFillInEditing(fSetup)) {
		return clip.fFillIn;
	}
	if (IsLandingEditing(fSetup)) {
		return clip.fLanding;
	}
	if (IsIntroEditing(fSetup)) {
		return clip.fIntro;
	}
	if (IsOutroEditing(fSetup)) {
		return clip.fOutro;
	}
	return clip;
}

CLauncher::TLight CLauncher::GetEditingLight(int iTrack, int iRow, const TSettings& iSettings) const {
	if (iTrack == kEditingToggleTrack && iRow == kEditingToggleRow) {
		return iSettings.fPresent ? MakeLight(kVibrant, kColourGreen) : MakeLight(kDim, kColourRed);
	}
	// The colour and CV controls only show while there is something to set
	// up.
	if (!iSettings.fPresent) {
		return kUnlitLight;
	}
	const bool isColourRow = iRow == kEditingFirstColourRow || iRow == kEditingFirstColourRow + 1;
	if (isColourRow && iTrack < kEditingColoursPerRow) {
		const int colour = (iRow - kEditingFirstColourRow) * kEditingColoursPerRow + iTrack;
		return MakeLight(colour == iSettings.fColour ? kVibrant : kDim, colour);
	}
	// The one-bar pattern switch is lit like SET/CLEAR.
	const bool overlay = !IsClipEditing(fSetup);
	if (overlay && iTrack == kEditingOneBarPatternTrack && iRow == kEditingOneBarPatternRow) {
		return iSettings.fOneBarPattern ? MakeLight(kVibrant, kColourGreen) : MakeLight(kDim, kColourRed);
	}
	if (iRow < kEditingFirstOutputRow) {
		return kUnlitLight;
	}
	const int output = iRow - kEditingFirstOutputRow;
	const int cvMode = fCVModes[PickedTrack()][output];
	// In pattern mode the chosen pattern is vibrant and the others dim, all in
	// orange.
	if (cvMode == kCVModePattern) {
		return MakeLight(iTrack == iSettings.fPatterns[output] ? kVibrant : kDim, kColourOrange);
	}
	// The transition button is dim red while off, like SET/CLEAR, and
	// otherwise vibrant in the colour of its transition in the diagram.
	if (iTrack == kEditingTransitionTrack) {
		static const int kTransitionColours[kTransitionCount] = { kColourRed, kColourLime, kColourLightBlue, kColourPurple, kColourPink };
		const int transition = iSettings.fTransitions[output];
		return MakeLight(transition == kTransitionOff ? kDim : kVibrant, kTransitionColours[transition]);
	}
	// A unipolar value is vibrant cyan, with the values below it dim. A
	// bipolar value is vibrant fuchsia, with the values between it and 0 dim,
	// 0 included.
	if (cvMode == kCVModeUnipolar) {
		const int value = iSettings.fUnipolar[output];
		if (iTrack == value) {
			return MakeLight(kVibrant, kColourCyan);
		}
		return iTrack < value ? MakeLight(kDim, kColourCyan) : kUnlitLight;
	}
	const int value = iSettings.fBipolar[output];
	if (iTrack == value) {
		return MakeLight(kVibrant, kColourFuchsia);
	}
	const bool between = (iTrack >= kBipolarZero && iTrack < value) || (iTrack <= kBipolarZero && iTrack > value);
	return between ? MakeLight(kDim, kColourFuchsia) : kUnlitLight;
}

int CLauncher::GetLightValue(int iTrack, int iRow) const {
	return LightValue(GetLight(iTrack, iRow));
}

int CLauncher::GetLightFrame(int iTrack, int iRow) const {
	return LightFrame(GetLight(iTrack, iRow));
}

CLauncher::TLight CLauncher::GetSceneLight(int iScene) const {
	if (fSetup == kSetupClipPicking) {
		return MakeLight(kFlashing, iScene == SceneForLength(fLength) ? kColourGreen : kColourWhite);
	}
	if (fSetup == kSetupClearMode) {
		const bool clearable = iScene >= 1 && iScene <= kRowCount && SceneHasClips(iScene);
		return clearable ? MakeLight(kFlashing, kColourRed) : kUnlitLight;
	}
	if (fSetup == kSetupSourcePicking) {
		const bool copyable = iScene >= 1 && iScene <= kRowCount && SceneHasClips(iScene);
		return copyable ? MakeLight(kFlashing, kColourFuchsia) : kUnlitLight;
	}
	if (IsDuplicateMode(fSetup)) {
		const int source = SourceScene(fSetup);
		if (source == 0 || iScene < 1 || iScene > kRowCount) {
			return kUnlitLight;
		}
		return MakeLight(iScene == source ? kVibrant : kFlashing, kColourFuchsia);
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
	return LightFrame(GetSceneLight(iScene));
}

CLauncher::EModeColour CLauncher::GetModeColour() const {
	if (fSetup == kSetupLaunchMode) {
		return kModeWhite;
	}
	if (fSetup == kSetupClipPicking) {
		return kModeGreen;
	}
	if (fSetup == kSetupClearMode) {
		return kModeRed;
	}
	if (IsDuplicateMode(fSetup)) {
		return kModeFuchsia;
	}
	if (IsFillInEditing(fSetup)) {
		return kModeOrange;
	}
	if (IsLandingEditing(fSetup)) {
		return kModeBlue;
	}
	if (IsIntroEditing(fSetup)) {
		return kModePink;
	}
	return IsOutroEditing(fSetup) ? kModeTurquoise : kModeYellow;
}

int CLauncher::GetLogoFrame() const {
	switch (GetModeColour()) {
		case kModeGreen: return kColourGreen;
		case kModeYellow: return kColourYellow;
		case kModeRed: return kColourRed;
		case kModeBlue: return kColourBlue;
		case kModeOrange: return kColourOrange;
		case kModeFuchsia: return kColourFuchsia;
		case kModePink: return kColourPink;
		case kModeTurquoise: return kColourTurquoise;
		default: return kColourWhite;
	}
}

// The back button shows the mode colour of the step it goes back to.
CLauncher::TLight CLauncher::GetBackButtonLight() const {
	if (fSetup == kSetupLaunchMode) {
		return kUnlitLight;
	}
	if (fSetup == kSetupClipPicking || fSetup == kSetupClearMode || IsDuplicateMode(fSetup)) {
		return MakeLight(kVibrant, kColourWhite);
	}
	if (IsFillInEditing(fSetup)) {
		return MakeLight(kVibrant, kColourYellow);
	}
	if (IsLandingEditing(fSetup)) {
		return MakeLight(kVibrant, kColourOrange);
	}
	if (IsIntroEditing(fSetup)) {
		return MakeLight(kVibrant, kColourBlue);
	}
	return MakeLight(kVibrant, IsOutroEditing(fSetup) ? kColourPink : kColourGreen);
}

int CLauncher::GetBackButtonFrame() const {
	return LightFrame(GetBackButtonLight());
}

// The setup button shows the mode colour of the step of setup mode it is in.
CLauncher::TLight CLauncher::GetSetupLight() const {
	if (fSetup == kSetupLaunchMode) {
		return MakeLight(kDim, kColourGreen);
	}
	if (fSetup == kSetupClearMode || IsDuplicateMode(fSetup)) {
		return kUnlitLight;
	}
	if (fSetup == kSetupClipPicking) {
		return MakeLight(kVibrant, kColourGreen);
	}
	if (IsFillInEditing(fSetup)) {
		return MakeLight(kVibrant, kColourOrange);
	}
	if (IsLandingEditing(fSetup)) {
		return MakeLight(kVibrant, kColourBlue);
	}
	if (IsIntroEditing(fSetup)) {
		return MakeLight(kVibrant, kColourPink);
	}
	return MakeLight(kVibrant, IsOutroEditing(fSetup) ? kColourTurquoise : kColourYellow);
}

int CLauncher::GetSetupLightFrame() const {
	return LightFrame(GetSetupLight());
}

int CLauncher::GetSetupCaptionFrame() const {
	if (fSetup == kSetupLaunchMode) {
		return 0;
	}
	const TLight light = GetSetupLight();
	if (light.fBrightness == kUnlit) {
		return 1;
	}
	switch (light.fColour) {
		case kColourGreen: return 2;
		case kColourYellow: return 3;
		case kColourOrange: return 4;
		case kColourBlue: return 5;
		case kColourPink: return 6;
		default: return 7;
	}
}

// The line of the step the back button goes back to: the one before the
// current step.
int CLauncher::GetBackLabelFrame() const {
	if (fSetup == kSetupLaunchMode) {
		return 0;
	}
	if (fSetup == kSetupClipPicking || fSetup == kSetupClearMode || IsDuplicateMode(fSetup)) {
		return 1;
	}
	if (IsFillInEditing(fSetup)) {
		return 3;
	}
	if (IsLandingEditing(fSetup)) {
		return 4;
	}
	if (IsIntroEditing(fSetup)) {
		return 5;
	}
	return IsOutroEditing(fSetup) ? 6 : 2;
}

int CLauncher::GetLaunchLabelsFrame() const {
	if (PickedButton(fSetup) < 0) {
		return 0;
	}
	return 1 + 2 * EditingColourIndex(fSetup) + (EditedSettings().fPresent ? 1 : 0);
}

int CLauncher::GetOutputLabelsFrame(int iOutput) const {
	if (!IsValidOutput(iOutput) || PickedButton(fSetup) < 0) {
		return 0;
	}
	const TSettings& settings = EditedSettings();
	if (!settings.fPresent) {
		return 0;
	}
	const int first = 1 + EditingColourIndex(fSetup) * kOutputLabelsFramesPerColour;
	const int cvMode = fCVModes[PickedTrack()][iOutput];
	if (cvMode == kCVModePattern) {
		return first;
	}
	const int transition = settings.fTransitions[iOutput];
	return first + 1 + (cvMode == kCVModeUnipolar ? 0 : kTransitionCount) + transition;
}

int CLauncher::GetTransitionDiagramsFrame() const {
	if (PickedButton(fSetup) < 0 || !EditedSettings().fPresent) {
		return 0;
	}
	return 1 + EditingColourIndex(fSetup);
}

CLauncher::TLight CLauncher::GetFillInLight() const {
	return fFillInPlaying ? MakeLight(kVibrant, kColourOrange) : MakeLight(kDim, kColourWhite);
}

int CLauncher::GetFillInLightValue() const {
	return LightValue(GetFillInLight());
}

int CLauncher::GetFillInLightFrame() const {
	return LightFrame(GetFillInLight());
}

int CLauncher::GetFillInLabelFrame() const {
	return fFillInPlaying ? 1 : 0;
}

CLauncher::TLight CLauncher::GetClearLight() const {
	if (fSetup == kSetupLaunchMode) {
		return MakeLight(kDim, kColourWhite);
	}
	return fSetup == kSetupClearMode ? MakeLight(kVibrant, kColourRed) : kUnlitLight;
}

int CLauncher::GetClearLightFrame() const {
	return LightFrame(GetClearLight());
}

int CLauncher::GetClearCaptionFrame() const {
	if (fSetup == kSetupLaunchMode) {
		return 0;
	}
	return fSetup == kSetupClearMode ? 2 : 1;
}

CLauncher::TLight CLauncher::GetDuplicateLight() const {
	if (fSetup == kSetupLaunchMode) {
		return MakeLight(kDim, kColourWhite);
	}
	return IsDuplicateMode(fSetup) ? MakeLight(kVibrant, kColourFuchsia) : kUnlitLight;
}

int CLauncher::GetDuplicateLightFrame() const {
	return LightFrame(GetDuplicateLight());
}

int CLauncher::GetDuplicateCaptionFrame() const {
	if (fSetup == kSetupLaunchMode) {
		return 0;
	}
	return IsDuplicateMode(fSetup) ? 2 : 1;
}

// It is lit while it leads somewhere, in the mode colour of the step it leads
// to: from clip editing of a clip to its fill-in, and on to its landing, its
// intro and its outro.
CLauncher::TLight CLauncher::GetForwardLight() const {
	if (PickedButton(fSetup) < 0 || !PickedClip().fPresent) {
		return kUnlitLight;
	}
	if (IsClipEditing(fSetup)) {
		return MakeLight(kVibrant, kColourOrange);
	}
	if (IsFillInEditing(fSetup)) {
		return MakeLight(kVibrant, kColourBlue);
	}
	if (IsLandingEditing(fSetup)) {
		return MakeLight(kVibrant, kColourPink);
	}
	if (IsIntroEditing(fSetup)) {
		return MakeLight(kVibrant, kColourTurquoise);
	}
	return kUnlitLight;
}

int CLauncher::GetForwardLightValue() const {
	return LightValue(GetForwardLight());
}

int CLauncher::GetForwardLightFrame() const {
	return LightFrame(GetForwardLight());
}

int CLauncher::GetForwardLabelFrame() const {
	const TLight light = GetForwardLight();
	if (light.fBrightness == kUnlit) {
		return 0;
	}
	switch (light.fColour) {
		case kColourOrange: return 1;
		case kColourBlue: return 2;
		case kColourPink: return 3;
		default: return 4;
	}
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

double CLauncher::GetCVValue(int iTrack, int iOutput) const {
	if (iTrack < 0 || iTrack >= kTrackCount || !IsValidOutput(iOutput)) {
		return 0.0;
	}
	// Mutes and solos only silence the Player: unipolar and bipolar values go
	// on as before.
	if (fCVModes[iTrack][iOutput] != kCVModePattern) {
		return fValues[iTrack][iOutput];
	}
	// The Combinator rounds its CV input to the nearest of nine evenly spaced
	// steps (Off, 1..8), so these values sit in the middle of each step. A
	// fill-in takes effect at once, and so does any change to it, to a
	// landing, an intro or an outro. A mute or a solo silences whatever the
	// track would emit, at once.
	if (IsSilenced(iTrack)) {
		return 0.0;
	}
	int emitted = fEmitted[iTrack][iOutput];
	if (PlaysFillInPatterns(iTrack)) {
		emitted = Clip(iTrack, fActive[iTrack]).fFillIn.fPatterns[iOutput] + 1;
	}
	else if (PlaysLandingPatterns(iTrack)) {
		emitted = Clip(iTrack, fActive[iTrack]).fLanding.fPatterns[iOutput] + 1;
	}
	else if (PlaysIntroPatterns(iTrack)) {
		emitted = Clip(iTrack, fIntroRows[iTrack]).fIntro.fPatterns[iOutput] + 1;
	}
	else if (PlaysOutroPatterns(iTrack)) {
		emitted = Clip(iTrack, fOutroRows[iTrack]).fOutro.fPatterns[iOutput] + 1;
	}
	return static_cast<double>(emitted) / kPatternCount;
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
