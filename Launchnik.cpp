#include "Launchnik.h"

namespace {

// Tags configured in motherboard_def.lua; track is 1..8, selection is 0..8
const TJBox_Tag kCustomPropsTag_FirstSelection = 101;

TJBox_Tag SelectionTag(int iTrack) {
	return kCustomPropsTag_FirstSelection + iTrack;
}

TJBox_Tag StateTag(int iTrack, int iSelection) {
	return 1000 + 10 * (iTrack + 1) + iSelection;
}

TJBox_Tag LampTag(int iTrack, int iSelection) {
	return 2000 + 10 * (iTrack + 1) + iSelection;
}

const TJBox_Float64 kBatchSize = 64.0;

// Not a value any property can have, so the first batch writes everything.
const TJBox_Int32 kNothingWritten = -1;

} // namespace

CLaunchnik::CLaunchnik(TJBox_Float32 iSampleRate)
	:
	fCustomProps(JBox_GetMotherboardObjectRef("/custom_properties")),
	fTransport(JBox_GetMotherboardObjectRef("/transport")),
	fSampleRate(iSampleRate),
	fIsFirstBatch(true)
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
		for (int selection = 0; selection < CLauncher::kSelectionCount; ++selection) {
			fWrittenStates[track][selection] = kNothingWritten;
			fWrittenLamps[track][selection] = kNothingWritten;
		}
	}
}

void CLaunchnik::RenderBatch(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount) {
	if (fIsFirstBatch) {
		// A song that has just been opened starts with its saved selections active.
		LoadSelections();
		fIsFirstBatch = false;
	}
	HandleSelectionChanges(iPropertyDiffs, iDiffCount);
	fLauncher.Advance(ReadTransport());
	WriteOutputs();
}

void CLaunchnik::LoadSelections() {
	int selections[CLauncher::kTrackCount];
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		selections[track] = static_cast<int>(JBox_LoadMOMPropertyAsNumber(fCustomProps, SelectionTag(track)));
	}
	fLauncher.Reset(selections);
}

void CLaunchnik::HandleSelectionChanges(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount) {
	for (TJBox_UInt32 i = 0; i < iDiffCount; ++i) {
		const TJBox_PropertyDiff& diff = iPropertyDiffs[i];
		if (diff.fObjectRef != fCustomProps) {
			continue;
		}
		const TJBox_Tag lastSelectionTag = SelectionTag(CLauncher::kTrackCount - 1);
		if (diff.fPropertyTag >= kCustomPropsTag_FirstSelection && diff.fPropertyTag <= lastSelectionTag) {
			const int track = static_cast<int>(diff.fPropertyTag - kCustomPropsTag_FirstSelection);
			fLauncher.SetSelection(track, static_cast<int>(JBox_GetNumber(diff.fCurrentValue)));
		}
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
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		for (int selection = 0; selection < CLauncher::kSelectionCount; ++selection) {
			const TJBox_Int32 state = fLauncher.GetButtonState(track, selection);
			if (state != fWrittenStates[track][selection]) {
				fWrittenStates[track][selection] = state;
				JBox_StoreMOMPropertyByTag(fCustomProps, StateTag(track, selection), JBox_MakeNumber(state));
			}

			const TJBox_Int32 lamp = fLauncher.IsLit(track, selection) ? 1 : 0;
			if (lamp != fWrittenLamps[track][selection]) {
				fWrittenLamps[track][selection] = lamp;
				JBox_StoreMOMPropertyByTag(fCustomProps, LampTag(track, selection), JBox_MakeBoolean(lamp == 1));
			}
		}

		const TJBox_Float64 cvValue = fLauncher.GetCVValue(track);
		if (cvValue != fWrittenCVValues[track]) {
			fWrittenCVValues[track] = cvValue;
			JBox_StoreMOMPropertyAsNumber(fCVOutputs[track], kJBox_CVOutputValue, cvValue);
		}
	}
}
