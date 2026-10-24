#pragma once

#include "Jukebox.h"
#include "Launcher.h"

// Connects the rules in CLauncher to the host: feeds it selections, clips, CV
// modes, the setup state, the function, mute and solo presses and the
// transport, and writes states, lights, lamps, labels and CV back.
class CLaunchnik {
	public: explicit CLaunchnik(TJBox_Float32 iSampleRate);

	public: void RenderBatch(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount);

	private: void LoadEverything();
	private: void HandleChanges(const TJBox_PropertyDiff iPropertyDiffs[], TJBox_UInt32 iDiffCount);
	private: void HandleChange(TJBox_Tag iTag, const TJBox_Value& iValue);
	// A change of a setting of a clip's, a fill-in's or a landing's CV output.
	private: void HandleOutputSettingChange(TJBox_Tag iTag, int iValue);
	private: CLauncher::TTransport ReadTransport() const;
	private: void WriteOutputs();
	// Stores a number unless it is what was last stored.
	private: void Write(TJBox_Tag iTag, TJBox_Int32 iValue, TJBox_Int32& ioWritten);

	private: CLauncher fLauncher;
	private: TJBox_ObjectRef fCustomProps;
	private: TJBox_ObjectRef fTransport;
	private: TJBox_ObjectRef fCVOutputs[CLauncher::kTrackCount][CLauncher::kOutputCount];
	private: TJBox_Float32 fSampleRate;
	private: bool fIsFirstBatch;
	private: bool fSetupLampLit;

	// What was last written to the host, to avoid storing unchanged values.
	private: TJBox_Int32 fWrittenSetupLamp;
	private: TJBox_Int32 fWrittenLogoFrame;
	private: TJBox_Int32 fWrittenLaunchLabelsFrame;
	private: TJBox_Int32 fWrittenBackButtonFrame;
	private: TJBox_Int32 fWrittenBackLabelFrame;
	private: TJBox_Int32 fWrittenFillInLight;
	private: TJBox_Int32 fWrittenFillInLightFrame;
	private: TJBox_Int32 fWrittenFillInLabelFrame;
	private: TJBox_Int32 fWrittenClearLightFrame;
	private: TJBox_Int32 fWrittenClearCaptionFrame;
	private: TJBox_Int32 fWrittenForwardLight;
	private: TJBox_Int32 fWrittenForwardLightFrame;
	private: TJBox_Int32 fWrittenForwardLabelFrame;
	private: TJBox_Int32 fWrittenTransitionDiagramsFrame;
	private: TJBox_Int32 fWrittenOutputLabelsFrames[CLauncher::kOutputCount];
	private: TJBox_Int32 fWrittenSceneLabelFrames[CLauncher::kRowCount];
	private: TJBox_Int32 fWrittenFunctionLights[CLauncher::kTrackCount];
	private: TJBox_Int32 fWrittenFunctionLightFrames[CLauncher::kTrackCount];
	private: TJBox_Int32 fWrittenFunctionSelectLightFrames[CLauncher::kFunctionCount];
	private: TJBox_Int32 fWrittenFunctionSelectCaptionFrames[CLauncher::kFunctionCount];
	private: TJBox_Int32 fWrittenLights[CLauncher::kTrackCount][CLauncher::kRowCount];
	private: TJBox_Int32 fWrittenLightFrames[CLauncher::kTrackCount][CLauncher::kRowCount];
	private: TJBox_Int32 fWrittenSceneLights[CLauncher::kRowCount];
	private: TJBox_Int32 fWrittenSceneLightFrames[CLauncher::kRowCount];
	private: TJBox_Float64 fWrittenCVValues[CLauncher::kTrackCount][CLauncher::kOutputCount];
	private: TJBox_Int32 fWrittenCVModes[CLauncher::kTrackCount][CLauncher::kOutputCount];
};
