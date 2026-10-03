#pragma once

#include "Jukebox.h"
#include "Launcher.h"

// Connects the switching rules in CLauncher to the host: feeds it selections
// and the transport, and writes button states, lamps and CV back.
class CLaunchnik {
	public: explicit CLaunchnik(TJBox_Float32 iSampleRate);

	public: void RenderBatch(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount);

	private: void LoadSelections();
	private: void HandleSelectionChanges(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount);
	private: CLauncher::TTransport ReadTransport() const;
	private: void WriteOutputs();

	private: CLauncher fLauncher;
	private: TJBox_ObjectRef fCustomProps;
	private: TJBox_ObjectRef fTransport;
	private: TJBox_ObjectRef fCVOutputs[CLauncher::kTrackCount];
	private: TJBox_Float32 fSampleRate;
	private: bool fIsFirstBatch;

	// What was last written to the host, to avoid storing unchanged values.
	private: TJBox_Int32 fWrittenStates[CLauncher::kTrackCount][CLauncher::kSelectionCount];
	private: TJBox_Int32 fWrittenLamps[CLauncher::kTrackCount][CLauncher::kSelectionCount];
	private: TJBox_Float64 fWrittenCVValues[CLauncher::kTrackCount];
};
