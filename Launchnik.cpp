#include "Launchnik.h"

#include "Tags.h"

namespace {

const TJBox_Float64 kBatchSize = 64.0;

// Not a value any property can have, so the first batch writes everything.
const TJBox_Int32 kNothingWritten = -1;

// The tags of the launch buttons' properties are a base plus 10 for each track
// plus the row; a base of its own for each kind of property.
const TJBox_Tag kClipPresentBase = 3000;
const TJBox_Tag kClipColourBase = 5000;
const TJBox_Tag kFillInPresentBase = 6000;
const TJBox_Tag kLandingPresentBase = 7000;
const TJBox_Tag kFillInColourBase = 8000;
const TJBox_Tag kLandingColourBase = 9000;
const TJBox_Tag kClipTagSpan = 1000;

const TJBox_Tag kOutputSettingBases[] = {
	kClipPatternBase, kClipUnipolarBase, kClipBipolarBase, kClipTransitionBase,
	kFillInPatternBase, kFillInUnipolarBase, kFillInBipolarBase, kFillInTransitionBase,
	kLandingPatternBase, kLandingUnipolarBase, kLandingBipolarBase, kLandingTransitionBase,
};

// The names of the CV outputs in motherboard_def.lua, by track and output.
const char* const kCVOutputPaths[CLauncher::kTrackCount][CLauncher::kOutputCount] = {
	{ "/cv_outputs/track1_a", "/cv_outputs/track1_b", "/cv_outputs/track1_c", "/cv_outputs/track1_d", "/cv_outputs/track1_e", "/cv_outputs/track1_f" },
	{ "/cv_outputs/track2_a", "/cv_outputs/track2_b", "/cv_outputs/track2_c", "/cv_outputs/track2_d", "/cv_outputs/track2_e", "/cv_outputs/track2_f" },
	{ "/cv_outputs/track3_a", "/cv_outputs/track3_b", "/cv_outputs/track3_c", "/cv_outputs/track3_d", "/cv_outputs/track3_e", "/cv_outputs/track3_f" },
	{ "/cv_outputs/track4_a", "/cv_outputs/track4_b", "/cv_outputs/track4_c", "/cv_outputs/track4_d", "/cv_outputs/track4_e", "/cv_outputs/track4_f" },
	{ "/cv_outputs/track5_a", "/cv_outputs/track5_b", "/cv_outputs/track5_c", "/cv_outputs/track5_d", "/cv_outputs/track5_e", "/cv_outputs/track5_f" },
	{ "/cv_outputs/track6_a", "/cv_outputs/track6_b", "/cv_outputs/track6_c", "/cv_outputs/track6_d", "/cv_outputs/track6_e", "/cv_outputs/track6_f" },
	{ "/cv_outputs/track7_a", "/cv_outputs/track7_b", "/cv_outputs/track7_c", "/cv_outputs/track7_d", "/cv_outputs/track7_e", "/cv_outputs/track7_f" },
	{ "/cv_outputs/track8_a", "/cv_outputs/track8_b", "/cv_outputs/track8_c", "/cv_outputs/track8_d", "/cv_outputs/track8_e", "/cv_outputs/track8_f" },
};

} // namespace

CLaunchnik::CLaunchnik(TJBox_Float32 iSampleRate)
	:
	fCustomProps(JBox_GetMotherboardObjectRef("/custom_properties")),
	fTransport(JBox_GetMotherboardObjectRef("/transport")),
	fSampleRate(iSampleRate),
	fIsFirstBatch(true),
	fSetupLampLit(false),
	fWrittenSetupLamp(kNothingWritten),
	fWrittenLogoFrame(kNothingWritten),
	fWrittenLaunchLabelsFrame(kNothingWritten),
	fWrittenBackButtonFrame(kNothingWritten),
	fWrittenBackLabelFrame(kNothingWritten),
	fWrittenFillInLight(kNothingWritten),
	fWrittenFillInLightFrame(kNothingWritten),
	fWrittenFillInLabelFrame(kNothingWritten),
	fWrittenForwardLight(kNothingWritten),
	fWrittenForwardLightFrame(kNothingWritten),
	fWrittenForwardLabelFrame(kNothingWritten),
	fWrittenTransitionDiagramsFrame(kNothingWritten)
{
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		fWrittenOutputLabelsFrames[output] = kNothingWritten;
	}
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		for (int output = 0; output < CLauncher::kOutputCount; ++output) {
			fCVOutputs[track][output] = JBox_GetMotherboardObjectRef(kCVOutputPaths[track][output]);
			// Not a value any CV output can have, so the first batch writes them.
			fWrittenCVValues[track][output] = -2.0;
			fWrittenCVModes[track][output] = kNothingWritten;
		}
		fWrittenFunctionLights[track] = kNothingWritten;
		fWrittenFunctionLightFrames[track] = kNothingWritten;
		for (int row = 0; row < CLauncher::kRowCount; ++row) {
			fWrittenLights[track][row] = kNothingWritten;
			fWrittenLightFrames[track][row] = kNothingWritten;
		}
	}
	for (int scene = 0; scene < CLauncher::kRowCount; ++scene) {
		fWrittenSceneLights[scene] = kNothingWritten;
		fWrittenSceneLightFrames[scene] = kNothingWritten;
		fWrittenSceneLabelFrames[scene] = kNothingWritten;
	}
	for (int function = 0; function < CLauncher::kFunctionCount; ++function) {
		fWrittenFunctionSelectLightFrames[function] = kNothingWritten;
		fWrittenFunctionSelectCaptionFrames[function] = kNothingWritten;
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
	HandleChange(kCustomPropsTag_Length, JBox_LoadMOMPropertyByTag(fCustomProps, kCustomPropsTag_Length));
	HandleChange(kCustomPropsTag_Function, JBox_LoadMOMPropertyByTag(fCustomProps, kCustomPropsTag_Function));
	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		HandleChange(SelectionTag(track), JBox_LoadMOMPropertyByTag(fCustomProps, SelectionTag(track)));
		for (int output = 0; output < CLauncher::kOutputCount; ++output) {
			HandleChange(CVModeTag(track, output), JBox_LoadMOMPropertyByTag(fCustomProps, CVModeTag(track, output)));
		}
		for (int row = 1; row <= CLauncher::kRowCount; ++row) {
			const TJBox_Tag tags[] = {
				ClipPresentTag(track, row), ClipColourTag(track, row),
				FillInPresentTag(track, row), FillInColourTag(track, row),
				LandingPresentTag(track, row), LandingColourTag(track, row),
			};
			for (TJBox_Tag tag : tags) {
				HandleChange(tag, JBox_LoadMOMPropertyByTag(fCustomProps, tag));
			}
			for (TJBox_Tag base : kOutputSettingBases) {
				for (int output = 0; output < CLauncher::kOutputCount; ++output) {
					const TJBox_Tag tag = OutputSettingTag(base, track, row, output);
					HandleChange(tag, JBox_LoadMOMPropertyByTag(fCustomProps, tag));
				}
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
	if (iTag == kCustomPropsTag_Length) {
		fLauncher.SetLength(static_cast<int>(JBox_GetNumber(iValue)));
		return;
	}
	if (iTag == kCustomPropsTag_FillIn) {
		fLauncher.SetFillInPressed(JBox_GetBoolean(iValue));
		return;
	}
	if (iTag == kCustomPropsTag_Function) {
		fLauncher.SetFunction(static_cast<int>(JBox_GetNumber(iValue)));
		return;
	}
	if (iTag >= MuteTag(0) && iTag <= MuteTag(CLauncher::kTrackCount - 1)) {
		fLauncher.SetMutePressed(static_cast<int>(iTag - MuteTag(0)), JBox_GetBoolean(iValue));
		return;
	}
	if (iTag >= SoloTag(0) && iTag <= SoloTag(CLauncher::kTrackCount - 1)) {
		fLauncher.SetSoloPressed(static_cast<int>(iTag - SoloTag(0)), JBox_GetBoolean(iValue));
		return;
	}
	if (iTag >= SelectionTag(0) && iTag <= SelectionTag(CLauncher::kTrackCount - 1)) {
		const int track = static_cast<int>(iTag - SelectionTag(0));
		fLauncher.SetSelection(track, static_cast<int>(JBox_GetNumber(iValue)));
		return;
	}
	if (iTag >= CVModeTag(0, 0) && iTag <= CVModeTag(CLauncher::kTrackCount - 1, CLauncher::kOutputCount - 1)) {
		const int track = static_cast<int>((iTag - CVModeTag(0, 0)) / 10);
		const int output = static_cast<int>((iTag - CVModeTag(0, 0)) % 10);
		fLauncher.SetCVMode(track, output, static_cast<int>(JBox_GetNumber(iValue)));
		return;
	}
	if (iTag >= kClipPatternBase && iTag < kLandingTransitionBase + kOutputSettingTagSpan) {
		HandleOutputSettingChange(iTag, static_cast<int>(JBox_GetNumber(iValue)));
		return;
	}
	if (iTag < kClipPresentBase || iTag >= kLandingColourBase + kClipTagSpan) {
		return;
	}
	const TJBox_Tag base = iTag - (iTag % kClipTagSpan);
	const int track = static_cast<int>((iTag - base) / 10) - 1;
	const int row = static_cast<int>((iTag - base) % 10);
	if (base == kClipPresentBase) {
		fLauncher.SetClipPresent(track, row, JBox_GetBoolean(iValue));
	}
	else if (base == kClipColourBase) {
		fLauncher.SetClipColour(track, row, static_cast<int>(JBox_GetNumber(iValue)));
	}
	else if (base == kFillInPresentBase) {
		fLauncher.SetFillInPresent(track, row, JBox_GetBoolean(iValue));
	}
	else if (base == kFillInColourBase) {
		fLauncher.SetFillInColour(track, row, static_cast<int>(JBox_GetNumber(iValue)));
	}
	else if (base == kLandingPresentBase) {
		fLauncher.SetLandingPresent(track, row, JBox_GetBoolean(iValue));
	}
	else if (base == kLandingColourBase) {
		fLauncher.SetLandingColour(track, row, static_cast<int>(JBox_GetNumber(iValue)));
	}
}

void CLaunchnik::HandleOutputSettingChange(TJBox_Tag iTag, int iValue) {
	const TJBox_Tag base = iTag - (iTag % kOutputSettingTagSpan);
	const int offset = static_cast<int>(iTag - base);
	const int track = offset / 100 - 1;
	const int row = (offset / 10) % 10;
	const int output = offset % 10 - 1;
	switch (base) {
		case kClipPatternBase: fLauncher.SetClipPattern(track, row, output, iValue); break;
		case kClipUnipolarBase: fLauncher.SetClipUnipolar(track, row, output, iValue); break;
		case kClipBipolarBase: fLauncher.SetClipBipolar(track, row, output, iValue); break;
		case kClipTransitionBase: fLauncher.SetClipTransition(track, row, output, iValue); break;
		case kFillInPatternBase: fLauncher.SetFillInPattern(track, row, output, iValue); break;
		case kFillInUnipolarBase: fLauncher.SetFillInUnipolar(track, row, output, iValue); break;
		case kFillInBipolarBase: fLauncher.SetFillInBipolar(track, row, output, iValue); break;
		case kFillInTransitionBase: fLauncher.SetFillInTransition(track, row, output, iValue); break;
		case kLandingPatternBase: fLauncher.SetLandingPattern(track, row, output, iValue); break;
		case kLandingUnipolarBase: fLauncher.SetLandingUnipolar(track, row, output, iValue); break;
		case kLandingBipolarBase: fLauncher.SetLandingBipolar(track, row, output, iValue); break;
		case kLandingTransitionBase: fLauncher.SetLandingTransition(track, row, output, iValue); break;
		default: break;
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

void CLaunchnik::Write(TJBox_Tag iTag, TJBox_Int32 iValue, TJBox_Int32& ioWritten) {
	if (iValue != ioWritten) {
		ioWritten = iValue;
		JBox_StoreMOMPropertyByTag(fCustomProps, iTag, JBox_MakeNumber(iValue));
	}
}

void CLaunchnik::WriteOutputs() {
	const TJBox_Int32 setupLamp = fSetupLampLit ? 1 : 0;
	if (setupLamp != fWrittenSetupLamp) {
		fWrittenSetupLamp = setupLamp;
		JBox_StoreMOMPropertyByTag(fCustomProps, kCustomPropsTag_SetupLamp, JBox_MakeBoolean(fSetupLampLit));
	}

	const TJBox_Int32 logoFrame = fLauncher.GetModeColour();
	if (logoFrame != fWrittenLogoFrame) {
		fWrittenLogoFrame = logoFrame;
		JBox_StoreMOMPropertyByTag(fCustomProps, kCustomPropsTag_LogoFrame, JBox_MakeNumber(logoFrame));
	}

	const TJBox_Int32 launchLabelsFrame = fLauncher.GetLaunchLabelsFrame();
	if (launchLabelsFrame != fWrittenLaunchLabelsFrame) {
		fWrittenLaunchLabelsFrame = launchLabelsFrame;
		JBox_StoreMOMPropertyByTag(fCustomProps, kCustomPropsTag_LaunchLabelsFrame, JBox_MakeNumber(launchLabelsFrame));
	}

	const TJBox_Int32 backButtonFrame = fLauncher.GetBackButtonFrame();
	if (backButtonFrame != fWrittenBackButtonFrame) {
		fWrittenBackButtonFrame = backButtonFrame;
		JBox_StoreMOMPropertyByTag(fCustomProps, kCustomPropsTag_BackButtonFrame, JBox_MakeNumber(backButtonFrame));
	}

	const TJBox_Int32 backLabelFrame = fLauncher.GetBackLabelFrame();
	if (backLabelFrame != fWrittenBackLabelFrame) {
		fWrittenBackLabelFrame = backLabelFrame;
		JBox_StoreMOMPropertyByTag(fCustomProps, kCustomPropsTag_BackLabelFrame, JBox_MakeNumber(backLabelFrame));
	}

	Write(kCustomPropsTag_FillInLight, fLauncher.GetFillInLightValue(), fWrittenFillInLight);
	Write(kCustomPropsTag_FillInLightFrame, fLauncher.GetFillInLightFrame(), fWrittenFillInLightFrame);
	Write(kCustomPropsTag_FillInLabelFrame, fLauncher.GetFillInLabelFrame(), fWrittenFillInLabelFrame);
	Write(kCustomPropsTag_ForwardLight, fLauncher.GetForwardLightValue(), fWrittenForwardLight);
	Write(kCustomPropsTag_ForwardLightFrame, fLauncher.GetForwardLightFrame(), fWrittenForwardLightFrame);
	Write(kCustomPropsTag_ForwardLabelFrame, fLauncher.GetForwardLabelFrame(), fWrittenForwardLabelFrame);
	Write(kCustomPropsTag_TransitionDiagramsFrame, fLauncher.GetTransitionDiagramsFrame(), fWrittenTransitionDiagramsFrame);
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		Write(OutputLabelsFrameTag(output), fLauncher.GetOutputLabelsFrame(output), fWrittenOutputLabelsFrames[output]);
	}

	for (int function = 0; function < CLauncher::kFunctionCount; ++function) {
		Write(FunctionSelectLightFrameTag(function), fLauncher.GetFunctionSelectLightFrame(function), fWrittenFunctionSelectLightFrames[function]);
		Write(FunctionSelectCaptionFrameTag(function), fLauncher.GetFunctionSelectCaptionFrame(function), fWrittenFunctionSelectCaptionFrames[function]);
	}

	for (int track = 0; track < CLauncher::kTrackCount; ++track) {
		Write(FunctionLightTag(track), fLauncher.GetFunctionLightValue(track), fWrittenFunctionLights[track]);
		Write(FunctionLightFrameTag(track), fLauncher.GetFunctionLightFrame(track), fWrittenFunctionLightFrames[track]);

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

		for (int output = 0; output < CLauncher::kOutputCount; ++output) {
			Write(CVModeCopyTag(track, output), fLauncher.GetCVMode(track, output), fWrittenCVModes[track][output]);
			const TJBox_Float64 cvValue = fLauncher.GetCVValue(track, output);
			if (cvValue != fWrittenCVValues[track][output]) {
				fWrittenCVValues[track][output] = cvValue;
				JBox_StoreMOMPropertyAsNumber(fCVOutputs[track][output], kJBox_CVOutputValue, cvValue);
			}
		}
	}

	for (int scene = 1; scene <= CLauncher::kRowCount; ++scene) {
		const TJBox_Int32 light = fLauncher.GetSceneLightValue(scene);
		if (light != fWrittenSceneLights[scene - 1]) {
			fWrittenSceneLights[scene - 1] = light;
			JBox_StoreMOMPropertyByTag(fCustomProps, SceneLightTag(scene), JBox_MakeNumber(light));
		}

		const TJBox_Int32 frame = fLauncher.GetSceneLightFrame(scene);
		if (frame != fWrittenSceneLightFrames[scene - 1]) {
			fWrittenSceneLightFrames[scene - 1] = frame;
			JBox_StoreMOMPropertyByTag(fCustomProps, SceneLightFrameTag(scene), JBox_MakeNumber(frame));
		}

		const TJBox_Int32 labelFrame = fLauncher.GetSceneLabelFrame(scene);
		if (labelFrame != fWrittenSceneLabelFrames[scene - 1]) {
			fWrittenSceneLabelFrames[scene - 1] = labelFrame;
			JBox_StoreMOMPropertyByTag(fCustomProps, SceneLabelFrameTag(scene), JBox_MakeNumber(labelFrame));
		}
	}
}
