#include "Launchnik.h"

#include "Tags.h"

namespace {

const TJBox_Float64 kBatchSize = 64.0;

// Not a value any property can have, so the first batch writes everything.
const TJBox_Int32 kNothingWritten = -1;

// The tags of the launch buttons' properties are a base plus 10 for each track
// plus the row; a base of its own for each kind of property.
const TJBox_Tag kClipPresentBase = 3000;
const TJBox_Tag kClipPatternBase = 4000;
const TJBox_Tag kClipColourBase = 5000;
const TJBox_Tag kClipTagSpan = 1000;

} // namespace

CLaunchnik::CLaunchnik(TJBox_Float32 iSampleRate)
	:
	fCustomProps(JBox_GetMotherboardObjectRef("/custom_properties")),
	fTransport(JBox_GetMotherboardObjectRef("/transport")),
	fSampleRate(iSampleRate),
	fIsFirstBatch(true),
	fSetupLampLit(false),
	fWrittenSetupLamp(kNothingWritten)
{
	const char* const cvOutputPaths[CLauncher::kTrackCount] = {
		"/cv_outputs/track1",
		"/cv_outputs/track2",
		"/cv_outputs/track3",
		"/cv_outputs/track4",
		"/cv_outputs/track5",
		"/cv_outputs/track6",
		"/cv_outputs/track7",
		"/cv_outputs/track8",
	};
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		fCVOutputs[track] = JBox_GetMotherboardObjectRef(cvOutputPaths[track]);
		fWrittenCVValues[track] = -1.0;
		fWrittenStopStates[track] = kNothingWritten;
		fWrittenStopLamps[track] = kNothingWritten;
		for (int row = 0; row < CLauncher::kRowCount; ++row) {
			fWrittenLights[track][row] = kNothingWritten;
			fWrittenLightFrames[track][row] = kNothingWritten;
		}
	}
}

void CLaunchnik::RenderBatch(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount) {
	if (fIsFirstBatch) {
		// A song that has just been opened starts with its saved selections active.
		LoadEverything();
		fLauncher.Open();
		fIsFirstBatch = false;
	}
	HandleChanges(iPropertyDiffs, iDiffCount);
	fLauncher.Advance(ReadTransport());
	WriteOutputs();
}

void CLaunchnik::LoadEverything() {
	HandleChange(kCustomPropsTag_Setup, JBox_LoadMOMPropertyByTag(fCustomProps, kCustomPropsTag_Setup));
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		HandleChange(SelectionTag(track), JBox_LoadMOMPropertyByTag(fCustomProps, SelectionTag(track)));
		for (int row = 1; row <= CLauncher::kRowCount; ++row) {
			const TJBox_Tag tags[] = { ClipPresentTag(track, row), ClipPatternTag(track, row), ClipColourTag(track, row) };
			for (TJBox_Tag tag : tags) {
				HandleChange(tag, JBox_LoadMOMPropertyByTag(fCustomProps, tag));
			}
		}
	}
}

void CLaunchnik::HandleChanges(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount) {
	for (TJBox_UInt32 i = 0; i < iDiffCount; ++i) {
		const TJBox_PropertyDiff& diff = iPropertyDiffs[i];
		if (diff.fObjectRef == fCustomProps) {
			HandleChange(diff.fPropertyTag, diff.fCurrentValue);
		}
	}
}

void CLaunchnik::HandleChange(TJBox_Tag iTag, const TJBox_Value& iValue) {
	if (iTag == kCustomPropsTag_Setup) {
		const int setup = static_cast<int>(JBox_GetNumber(iValue));
		fLauncher.SetSetup(setup);
		fSetupLampLit = setup != CLauncher::kSetupLaunchMode;
		return;
	}
	if (iTag >= SelectionTag(0) && iTag <= SelectionTag(CLauncher::kTrackCount - 1)) {
		const int track = static_cast<int>(iTag - SelectionTag(0));
		fLauncher.SetSelection(track, static_cast<int>(JBox_GetNumber(iValue)));
		return;
	}
	if (iTag < kClipPresentBase || iTag >= kClipColourBase + kClipTagSpan) {
		return;
	}
	const TJBox_Tag base = iTag - (iTag % kClipTagSpan);
	const int track = static_cast<int>((iTag - base) / 10) - 1;
	const int row = static_cast<int>((iTag - base) % 10);
	if (base == kClipPresentBase) {
		fLauncher.SetClipPresent(track, row, JBox_GetBoolean(iValue));
	}
	else if (base == kClipPatternBase) {
		fLauncher.SetClipPattern(track, row, static_cast<int>(JBox_GetNumber(iValue)));
	}
	else if (base == kClipColourBase) {
		fLauncher.SetClipColour(track, row, static_cast<int>(JBox_GetNumber(iValue)));
	}
}

CLauncher::TTransport CLaunchnik::ReadTransport() const {
	const TJBox_Float64 tempo = JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportTempo);

	CLauncher::TTransport transport;
	transport.fPlaying = JBox_GetBoolean(JBox_LoadMOMPropertyByTag(fTransport, kJBox_TransportPlaying));
	transport.fPlayPos = JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportPlayPos);
	transport.fBatchLength = (kBatchSize / fSampleRate) * (tempo / 60.0) * CLauncher::kPPQPerCrotchet;
	transport.fTimeSignatureNumerator = static_cast<int>(JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportTimeSignatureNumerator));
	transport.fTimeSignatureDenominator = static_cast<int>(JBox_LoadMOMPropertyAsNumber(fTransport, kJBox_TransportTimeSignatureDenominator));
	return transport;
}

void CLaunchnik::WriteOutputs() {
	const TJBox_Int32 setupLamp = fSetupLampLit ? 1 : 0;
	if (setupLamp != fWrittenSetupLamp) {
		fWrittenSetupLamp = setupLamp;
		JBox_StoreMOMPropertyByTag(fCustomProps, kCustomPropsTag_SetupLamp, JBox_MakeBoolean(fSetupLampLit));
	}

	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		const TJBox_Int32 stopState = fLauncher.GetButtonState(track, CLauncher::kSelectionStop);
		if (stopState != fWrittenStopStates[track]) {
			fWrittenStopStates[track] = stopState;
			JBox_StoreMOMPropertyByTag(fCustomProps, StopStateTag(track), JBox_MakeNumber(stopState));
		}

		const TJBox_Int32 stopLamp = fLauncher.IsStopLit(track) ? 1 : 0;
		if (stopLamp != fWrittenStopLamps[track]) {
			fWrittenStopLamps[track] = stopLamp;
			JBox_StoreMOMPropertyByTag(fCustomProps, StopLampTag(track), JBox_MakeBoolean(stopLamp == 1));
		}

		for (int row = 1; row <= CLauncher::kRowCount; ++row) {
			const TJBox_Int32 light = fLauncher.GetLightValue(track, row);
			if (light != fWrittenLights[track][row - 1]) {
				fWrittenLights[track][row - 1] = light;
				JBox_StoreMOMPropertyByTag(fCustomProps, LightTag(track, row), JBox_MakeNumber(light));
			}

			const TJBox_Int32 frame = fLauncher.GetLightFrame(track, row);
			if (frame != fWrittenLightFrames[track][row - 1]) {
				fWrittenLightFrames[track][row - 1] = frame;
				JBox_StoreMOMPropertyByTag(fCustomProps, LightFrameTag(track, row), JBox_MakeNumber(frame));
			}
		}

		const TJBox_Float64 cvValue = fLauncher.GetCVValue(track);
		if (cvValue != fWrittenCVValues[track]) {
			fWrittenCVValues[track] = cvValue;
			JBox_StoreMOMPropertyAsNumber(fCVOutputs[track], kJBox_CVOutputValue, cvValue);
		}
	}
}
