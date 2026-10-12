#pragma once

#include "Jukebox.h"
#include "Launcher.h"

// Connects the rules in CLauncher to the host: feeds it selections, clips, the
// setup state and the transport, and writes states, lights, lamps and CV back.
class CLaunchnik {
	public: explicit CLaunchnik(TJBox_Float32 iSampleRate);

	public: void RenderBatch(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount);

	private: void LoadEverything();
	private: void HandleChanges(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount);
	private: void HandleChange(TJBox_Tag iTag, const TJBox_Value& iValue);
	private: CLauncher::TTransport ReadTransport() const;
	private: void WriteOutputs();

	private: CLauncher fLauncher;
	private: TJBox_ObjectRef fCustomProps;
	private: TJBox_ObjectRef fTransport;
	private: TJBox_ObjectRef fCVOutputs[CLauncher::kTrackCount];
	private: TJBox_Float32 fSampleRate;
	private: bool fIsFirstBatch;
	private: bool fSetupLampLit;

	// What was last written to the host, to avoid storing unchanged values.
	private: TJBox_Int32 fWrittenSetupLamp;
	private: TJBox_Int32 fWrittenStopLights[CLauncher::kTrackCount];
	private: TJBox_Int32 fWrittenStopLightFrames[CLauncher::kTrackCount];
	private: TJBox_Int32 fWrittenLights[CLauncher::kTrackCount][CLauncher::kRowCount];
	private: TJBox_Int32 fWrittenLightFrames[CLauncher::kTrackCount][CLauncher::kRowCount];
	private: TJBox_Int32 fWrittenSceneLights[CLauncher::kRowCount];
	private: TJBox_Int32 fWrittenSceneLightFrames[CLauncher::kRowCount];
	private: TJBox_Float64 fWrittenCVValues[CLauncher::kTrackCount];
};
