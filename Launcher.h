#pragma once

// The rules of Launchnik, free of any dependency on the Jukebox SDK so that
// they can be compiled and tested on the command line (see tests/): when a
// track switches, what it emits, and what every button shows in launch mode
// and in setup mode.
//
// Vocabulary follows CONTEXT.md: track, clip, empty, selection, off / queued /
// active, light, label, mode colour, switch interval, switch point, length,
// immediate, clip picking, clip editing, fill-in, fill-in editing, landing,
// landing editing, intro, intro editing, outro, outro editing, one-bar
// pattern, relaunch, forward button, clear mode,
// clear button, duplicate mode, duplicate button, source, destination, source
// picking, destination picking, scene,
// segment, interval meter, function, function button, function select button,
// stop, muted, soloed, solo-muted, CV output, CV mode, pattern, unipolar value,
// bipolar value, transition.
//
// Tracks are numbered from 0 here. Rows are numbered from 1, so that a row is
// also the selection that chooses its launch button, and the number of its
// scene.

class CLauncher {
	public: static const int kTrackCount = 8;
	public: static const int kRowCount = 8;
	public: static const int kButtonCount = kTrackCount * kRowCount;

	// A selection is 0 for stop or 1..8 for the launch button in that row, so
	// a track has nine buttons.
	public: static const int kSelectionStop = 0;
	public: static const int kSelectionCount = 9;

	public: static const int kPatternCount = 8;

	// Each track has six CV outputs, A to F, numbered from 0 here.
	public: static const int kOutputCount = 6;

	// What a CV output emits, in the order of the frames of its switch on the
	// back panel, from the bottom up. A new device has output A in pattern
	// mode and the others in unipolar mode.
	public: enum ECVMode {
		kCVModeBipolar = 0,
		kCVModeUnipolar = 1,
		kCVModePattern = 2
	};
	public: static const int kCVModeCount = 3;

	// Unipolar and bipolar values are one of seven steps, the columns of the
	// first seven tracks in clip editing: 0..1 and -1..+1, with bipolar 0 in
	// the middle.
	public: static const int kValueCount = 7;
	public: static const int kBipolarZero = 3;

	// Off, then A, B, C and D. A and C move in a straight line, B and D along
	// an exponential curve; C and D only within the last bar.
	public: static const int kTransitionOff = 0;
	public: static const int kTransitionA = 1;
	public: static const int kTransitionB = 2;
	public: static const int kTransitionC = 3;
	public: static const int kTransitionD = 4;
	public: static const int kTransitionCount = 5;

	// The four settings of a CV output, as one number, the way the patch,
	// the panel and control surfaces keep them (see docs/adr/0008): the
	// pattern, plus 8 times the unipolar value, plus 56 times the bipolar
	// value, plus 392 times the transition.
	public: struct TOutputSettings {
		int fPattern;
		int fUnipolar;
		int fBipolar;
		int fTransition;
	};
	public: static const int kOutputSettingsCount = kPatternCount * kValueCount * kValueCount * kTransitionCount;
	public: static int PackOutputSettings(const TOutputSettings& iSettings);
	public: static TOutputSettings UnpackOutputSettings(int iPacked);

	public: static const int kColourCount = 14;

	// Whether a clip, fill-in, landing, intro or outro is there, its colour,
	// its one-bar pattern and whether it has been set up before, as one
	// number, the way the patch, the panel and control surfaces keep them
	// (see docs/adr/0008): 1 if it is there, plus 2 times the colour, plus 28
	// if one-bar pattern is on, plus 56 if it has been set up before. A
	// fill-in, landing, intro or outro that has been set up before keeps its
	// settings when it is switched on again; one that has not starts as a
	// copy of its clip. A clip's one-bar pattern is always off, and whether
	// it has been set up means nothing.
	public: struct TSwitches {
		bool fPresent;
		int fColour;
		bool fOneBarPattern;
		bool fSetUp;
	};
	public: static const int kSwitchesCount = 2 * kColourCount * 2 * 2;
	public: static int PackSwitches(const TSwitches& iSwitches);
	public: static TSwitches UnpackSwitches(int iPacked);
	public: static const int kColourWhite = 0;
	public: static const int kColourRed = 1;
	public: static const int kColourOrange = 2;
	public: static const int kColourYellow = 3;
	public: static const int kColourLime = 4;
	public: static const int kColourGreen = 5;
	public: static const int kColourTurquoise = 6;
	public: static const int kColourCyan = 7;
	public: static const int kColourLightBlue = 8;
	public: static const int kColourBlue = 9;
	public: static const int kColourPurple = 11;
	public: static const int kColourFuchsia = 12;
	public: static const int kColourPink = 13;

	// The setup state is one number: launch mode, clip picking, clip editing,
	// fill-in editing, landing editing, intro editing or outro editing for one
	// launch button, clear mode, or
	// duplicate mode: source picking, then destination picking for a launch
	// button or a scene as the source. Clear mode and duplicate mode are not
	// part of setup mode.
	public: static const int kSetupLaunchMode = 0;
	public: static const int kSetupClipPicking = 1;
	public: static const int kSetupFirstClipEditing = 2;
	public: static const int kSetupFirstFillInEditing = kSetupFirstClipEditing + kButtonCount;
	public: static const int kSetupFirstLandingEditing = kSetupFirstFillInEditing + kButtonCount;
	public: static const int kSetupFirstIntroEditing = kSetupFirstLandingEditing + kButtonCount;
	public: static const int kSetupFirstOutroEditing = kSetupFirstIntroEditing + kButtonCount;
	public: static const int kSetupClearMode = kSetupFirstOutroEditing + kButtonCount;
	public: static const int kSetupSourcePicking = kSetupClearMode + 1;
	public: static const int kSetupFirstClipDestinationPicking = kSetupSourcePicking + 1;
	public: static const int kSetupFirstSceneDestinationPicking = kSetupFirstClipDestinationPicking + kButtonCount;
	public: static const int kSetupCount = kSetupFirstSceneDestinationPicking + kRowCount;

	// Where the controls of clip editing sit on the grid; fill-in editing,
	// landing editing, intro editing and outro editing have them in the same
	// places. The colours fill the first seven tracks of rows
	// 1 and 2, and each of rows 3 to 8 stands for one CV output, A to F: eight
	// patterns in pattern mode, otherwise seven values and, in the last track,
	// the transition.
	public: static const int kEditingToggleTrack = 7;
	public: static const int kEditingToggleRow = 1;
	public: static const int kEditingFirstColourRow = 1;
	public: static const int kEditingColoursPerRow = 7;
	public: static const int kEditingFirstOutputRow = 3;
	public: static const int kEditingTransitionTrack = 7;
	// In fill-in editing, landing editing, intro editing and outro editing
	// only, the one-bar pattern switch sits in the last track of the second
	// colour row.
	public: static const int kEditingOneBarPatternTrack = 7;
	public: static const int kEditingOneBarPatternRow = 2;

	// The length of the switch interval is one of eight settings: immediate,
	// one beat, half a bar, then 1, 2, 4, 8 and 16 bars. In clip picking the
	// scene buttons stand for them, from the bottom.
	public: static const int kLengthImmediate = 0;
	public: static const int kLengthBeat = 1;
	public: static const int kLengthHalfBar = 2;
	public: static const int kLengthFirstBars = 3;
	public: static const int kLengthFourBars = 5;
	public: static const int kLengthCount = 8;
	public: static const int kDefaultLength = kLengthFourBars;

	// The interval meter has one scene button for each segment of the switch
	// interval. Counted from the bottom, the first five are green, the next two
	// yellow and the last one red.
	public: static const int kSegmentCount = kRowCount;
	public: static const int kGreenSegmentCount = 5;
	public: static const int kYellowSegmentCount = 2;

	// What the function buttons do: stop, mute or solo. A song opens with stop.
	public: static const int kFunctionStop = 0;
	public: static const int kFunctionMute = 1;
	public: static const int kFunctionSolo = 2;
	public: static const int kFunctionCount = 3;

	// A jump of the song position by more than this, between one batch and the
	// next, is a locate or a loop rather than the song playing on.
	public: static const long long kJumpTolerance = 3840;

	// Musical time is counted in PPQ, the host's unit for song position.
	public: static const long long kPPQPerCrotchet = 15360;

	public: enum EButtonState {
		kOff = 0,
		kQueued = 1,
		kActive = 2
	};

	public: enum EBrightness {
		kUnlit = 0,
		kDim = 1,
		kVibrant = 2,
		kFlashing = 3
	};

	// What a launch button is showing. The colour means nothing while unlit.
	public: struct TLight {
		EBrightness fBrightness;
		int fColour;
	};

	// A light as one number, the way control surfaces receive it: 0 is unlit,
	// then dim, vibrant and flashing for each colour.
	public: static const int kLightValueCount = 1 + 3 * kColourCount;

	// The film strips of every kind of button: frame 0 is unlit, then dim and
	// vibrant for each colour.
	public: static const int kLightFrameCount = 1 + 2 * kColourCount;

	// What the host's transport looks like for one batch of audio.
	public: struct TTransport {
		bool fPlaying;
		// Song position at the start of the batch.
		double fPlayPos;
		// Musical time covered by the batch.
		double fBatchLength;
		int fTimeSignatureNumerator;
		int fTimeSignatureDenominator;
	};

	public: CLauncher();

	// The colour a launch button's clip has until another one is chosen; each
	// track has its own.
	public: static int DefaultColour(int iTrack);

	// The number of the launch button of a track and row, 0..63, and the setup
	// state in which that launch button is being edited.
	public: static int ButtonIndex(int iTrack, int iRow);
	public: static int SetupForClipEditing(int iTrack, int iRow);
	public: static int SetupForFillInEditing(int iTrack, int iRow);
	public: static int SetupForLandingEditing(int iTrack, int iRow);
	public: static int SetupForIntroEditing(int iTrack, int iRow);
	public: static int SetupForOutroEditing(int iTrack, int iRow);
	public: static bool IsClipEditing(int iSetup);
	public: static bool IsFillInEditing(int iSetup);
	public: static bool IsLandingEditing(int iSetup);
	public: static bool IsIntroEditing(int iSetup);
	public: static bool IsOutroEditing(int iSetup);
	// The number of the launch button being edited in clip editing, fill-in
	// editing, landing editing, intro editing or outro editing, or -1 in
	// launch mode and clip picking.
	public: static int PickedButton(int iSetup);
	// Whether a setup state is one of setup mode: clip picking, clip editing,
	// fill-in editing, landing editing, intro editing or outro editing.
	public: static bool IsSetupMode(int iSetup);
	// Duplicate mode: source picking or destination picking.
	public: static bool IsDuplicateMode(int iSetup);
	// The setup state of destination picking with a launch button or a scene
	// as the source.
	public: static int SetupForClipDestinationPicking(int iTrack, int iRow);
	public: static int SetupForSceneDestinationPicking(int iScene);
	// The number of the launch button that is the source in destination
	// picking, or -1 if there is none.
	public: static int SourceButton(int iSetup);
	// The scene that is the source in destination picking, or 0 if there is
	// none.
	public: static int SourceScene(int iSetup);

	// Record what a launch button holds. A launch button keeps its settings
	// while it is empty, so that a clip comes back as it was. A new clip has,
	// on every CV output, the pattern of its row, unipolar value 0, bipolar
	// value 0 and transition off. Emptying the active launch button stops its
	// track at once, with 0 on every CV output, even while the song plays.
	public: void SetClipPresent(int iTrack, int iRow, bool iPresent);
	public: void SetClipColour(int iTrack, int iRow, int iColour);
	public: void SetClipPattern(int iTrack, int iRow, int iOutput, int iPattern);
	public: void SetClipUnipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetClipBipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetClipTransition(int iTrack, int iRow, int iOutput, int iTransition);

	// Record the fill-in a launch button holds, which it keeps while it is
	// empty, like its clip's settings.
	public: void SetFillInPresent(int iTrack, int iRow, bool iPresent);
	public: void SetFillInColour(int iTrack, int iRow, int iColour);
	public: void SetFillInPattern(int iTrack, int iRow, int iOutput, int iPattern);
	public: void SetFillInUnipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetFillInBipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetFillInTransition(int iTrack, int iRow, int iOutput, int iTransition);
	public: void SetFillInOneBarPattern(int iTrack, int iRow, bool iOn);

	// Record the landing a launch button holds, which it keeps while it is
	// empty, like its clip's settings.
	public: void SetLandingPresent(int iTrack, int iRow, bool iPresent);
	public: void SetLandingColour(int iTrack, int iRow, int iColour);
	public: void SetLandingPattern(int iTrack, int iRow, int iOutput, int iPattern);
	public: void SetLandingUnipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetLandingBipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetLandingTransition(int iTrack, int iRow, int iOutput, int iTransition);
	public: void SetLandingOneBarPattern(int iTrack, int iRow, bool iOn);
	// Whether a track is playing the landing of its active clip: during the
	// switch interval that began when the clip became active at a switch
	// point, or went on there after playing its fill-in, unless the length is
	// immediate.
	public: bool PlaysLanding(int iTrack) const;

	// Record the intro a launch button holds, which it keeps while it is
	// empty, like its clip's settings.
	public: void SetIntroPresent(int iTrack, int iRow, bool iPresent);
	public: void SetIntroColour(int iTrack, int iRow, int iColour);
	public: void SetIntroPattern(int iTrack, int iRow, int iOutput, int iPattern);
	public: void SetIntroUnipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetIntroBipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetIntroTransition(int iTrack, int iRow, int iOutput, int iTransition);
	public: void SetIntroOneBarPattern(int iTrack, int iRow, bool iOn);
	// Whether a stopped track is playing the intro of its queued clip: from
	// the moment the clip was queued until the switch point, while the song
	// plays and unless the length is immediate or the song position jumped.
	public: bool PlaysIntro(int iTrack) const;

	// Record the outro a launch button holds, which it keeps while it is
	// empty, like its clip's settings.
	public: void SetOutroPresent(int iTrack, int iRow, bool iPresent);
	public: void SetOutroColour(int iTrack, int iRow, int iColour);
	public: void SetOutroPattern(int iTrack, int iRow, int iOutput, int iPattern);
	public: void SetOutroUnipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetOutroBipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetOutroTransition(int iTrack, int iRow, int iOutput, int iTransition);
	public: void SetOutroOneBarPattern(int iTrack, int iRow, bool iOn);
	// Whether a track is playing the outro of the clip it stopped: during the
	// switch interval that began when stop took over from the clip at a
	// switch point, unless an intro cut it short.
	public: bool PlaysOutro(int iTrack) const;

	// The CV mode of a track's CV output. A change takes effect at once and
	// drops any transition under way.
	public: void SetCVMode(int iTrack, int iOutput, int iCVMode);
	public: int GetCVMode(int iTrack, int iOutput) const;
	public: static int DefaultCVMode(int iOutput);

	public: void SetSetup(int iSetup);

	public: void SetLength(int iLength);

	// The scene whose scene button stands for a length in clip picking, and
	// the other way round.
	public: static int SceneForLength(int iLength);
	public: static int LengthForScene(int iScene);

	// Records what the user chose on a track. Call before Advance() for changes
	// that arrived in the same batch.
	public: void SetSelection(int iTrack, int iSelection);

	public: void SetFunction(int iFunction);
	public: int GetFunction() const;

	// Whether a track's mute button or solo button is held down. Pressing it
	// switches the track between muted and not muted, or soloed and not
	// soloed, from the next call of Advance(). A press on a track that has no
	// clips does nothing. Call before Advance() for changes that arrived in the
	// same batch.
	public: void SetMutePressed(int iTrack, bool iPressed);
	public: void SetSoloPressed(int iTrack, bool iPressed);
	// Mutes and solos end at the next switch point, when the song position
	// jumps, and when their track has no clips left.
	public: bool IsMuted(int iTrack) const;
	public: bool IsSoloed(int iTrack) const;
	// Whether a track emits 0 because of a mute or a solo: it is not soloed,
	// and it is muted or another track is soloed.
	public: bool IsSilenced(int iTrack) const;

	// Whether the fill-in button is held down. Pressing it triggers a fill-in,
	// which plays from the next call of Advance() until the next switch point.
	// A press does nothing while a fill-in is playing or while the length is
	// immediate. Call before Advance() for changes that arrived in the same
	// batch.
	public: void SetFillInPressed(bool iPressed);
	public: bool IsFillInPlaying() const;

	// Whether a track's relaunch press is held down: every launch in launch
	// mode holds it, from a launch button or a scene button. Launching the
	// clip that is already active relaunches it, so that it lands at the next
	// switch point, if it holds a landing; any other launch is a selection
	// alone. Call before Advance() for changes that arrived in the same
	// batch, after the selection.
	public: void SetRelaunchPressed(int iTrack, bool iPressed);
	// Whether a track's active clip has been relaunched and waits for the
	// next switch point; queueing another clip or stop takes it back.
	public: bool IsRelaunched(int iTrack) const;

	// Makes every track emit its selection at once, with nothing queued. Used
	// when a song is opened.
	public: void Open();

	// Moves on by one batch.
	public: void Advance(const TTransport& iTransport);

	public: EButtonState GetButtonState(int iTrack, int iSelection) const;

	// What a stop button shows, in red: flashing while the track is about to
	// stop, vibrant while a clip is active and can be stopped, dim while the
	// track is stopped, unlit while the track has no clips at all.
	public: EBrightness GetStopLight(int iTrack) const;

	// What a function button shows, in the colour of the function: in stop
	// function the light of the stop button, in red. In mute function vibrant
	// yellow, or dim yellow while the track is muted; in solo function dim
	// blue, or vibrant blue while the track is soloed. Unlit in every function
	// while the track has no clips.
	public: TLight GetFunctionLight(int iTrack) const;
	// Its brightness alone, the way control surfaces receive it; the colour
	// follows from the function.
	public: int GetFunctionLightValue(int iTrack) const;
	// The frame of the function buttons' film strip to show right now; a
	// flashing light alternates between dim and vibrant.
	public: int GetFunctionLightFrame(int iTrack) const;

	// What a function select button shows: vibrant in the colour of its
	// function (red for stop, yellow for mute, blue for solo) while that
	// function is chosen, dim white otherwise.
	public: TLight GetFunctionSelectLight(int iFunction) const;
	public: int GetFunctionSelectLightFrame(int iFunction) const;
	// The frame of its caption: 0 white, 1 in the colour of its function while
	// that function is chosen.
	public: int GetFunctionSelectCaptionFrame(int iFunction) const;

	public: TLight GetLight(int iTrack, int iRow) const;
	public: int GetLightValue(int iTrack, int iRow) const;
	// The frame to show right now; a flashing light alternates.
	public: int GetLightFrame(int iTrack, int iRow) const;

	// What a scene button shows: its part of the interval meter, or in clip
	// picking the length it stands for, flashing in green if that is the
	// current length and in white otherwise, or in clear mode flashing red if
	// its scene holds a clip and unlit otherwise. In source picking it flashes
	// fuchsia if its scene holds a clip; in destination picking for a scene it
	// is vibrant fuchsia if its scene is the source and flashes fuchsia
	// otherwise, and for a launch button it is unlit. Scenes are numbered like rows,
	// 1..8 from the top, so the meter fills from scene 8 upwards. The value is
	// encoded like the light of a launch button.
	public: TLight GetSceneLight(int iScene) const;
	public: int GetSceneLightValue(int iScene) const;
	public: int GetSceneLightFrame(int iScene) const;

	// The mode colour, worn by the logo and the lit labels: white in launch
	// mode, green in clip picking, yellow in clip editing, orange in fill-in
	// editing, blue in landing editing, pink in intro editing, turquoise in
	// outro editing, red in clear mode, fuchsia in duplicate mode.
	public: enum EModeColour {
		kModeWhite = 0,
		kModeGreen = 1,
		kModeYellow = 2,
		kModeRed = 3,
		kModeBlue = 4,
		kModeOrange = 5,
		kModeFuchsia = 6,
		kModePink = 7,
		kModeTurquoise = 8
	};
	public: static const int kModeColourCount = 9;
	public: EModeColour GetModeColour() const;
	// The frame of the logo's film strip, which holds the lettering in each
	// colour: the mode colour's.
	public: int GetLogoFrame() const;

	// What the back button shows: unlit in launch mode, vibrant white in clip
	// picking, clear mode and duplicate mode, vibrant green in clip editing,
	// vibrant yellow in fill-in editing, vibrant orange in landing editing,
	// vibrant blue in intro editing, vibrant pink in outro editing.
	public: TLight GetBackButtonLight() const;
	public: int GetBackButtonFrame() const;

	// What the setup button shows: dim green in launch mode, unlit in clear
	// mode and duplicate mode, where it does nothing, and in setup mode vibrant
	// in the mode colour of the step.
	public: TLight GetSetupLight() const;
	public: int GetSetupLightFrame() const;
	// The frame of the SETUP caption: 0 white, 1 dark grey, then green,
	// yellow, orange, blue, pink and turquoise (2 to 7), coloured like the
	// light.
	public: static const int kSetupCaptionFrameCount = 8;
	public: int GetSetupCaptionFrame() const;

	// The frame of the back button's labels to show, one line for each step
	// it can go back to, lit in the colour of the button: 0 all unlit, 1
	// LAUNCH in white, 2 PICK CLIP in green, 3 EDIT CLIP in yellow, 4 SET
	// FILL-IN in orange, 5 SET LANDING in blue, 6 SET INTRO in pink.
	public: static const int kBackLabelFrameCount = 7;
	public: int GetBackLabelFrame() const;

	// The frame of the launch labels' film strip to show, for rows 1 and 2: 0
	// unlit, then SET/CLEAR lit and all lit, in yellow (1, 2), orange (3, 4),
	// blue (5, 6), pink (7, 8) and turquoise (9, 10). They are lit only in
	// clip editing, in yellow, in fill-in editing, in orange, in landing
	// editing, in blue, in intro editing, in pink, and in outro editing, in
	// turquoise; the labels of colours only while there is something to set
	// up: the picked launch button's clip, fill-in, landing, intro or outro.
	// 1 BAR PATT is lit only in the frames with all lit in orange, blue, pink
	// and turquoise.
	public: static const int kEditingColourCount = 5;
	public: static const int kLaunchLabelsFrameCount = 1 + 2 * kEditingColourCount;
	public: int GetLaunchLabelsFrame() const;

	// The frame of the film strip of the labels of a CV output's row, lit like
	// the colours': 0 unlit, then for yellow, orange, blue, pink and
	// turquoise in turn the
	// patterns, the unipolar values with each of the five transitions, and the
	// bipolar values with each of the five transitions. Only the labels that
	// go with the output's CV mode are lit, and of the transitions only the
	// one chosen.
	public: static const int kOutputLabelsFramesPerColour = 1 + 2 * kTransitionCount;
	public: static const int kOutputLabelsFrameCount = 1 + kEditingColourCount * kOutputLabelsFramesPerColour;
	public: int GetOutputLabelsFrame(int iOutput) const;

	// The five sets of transition diagrams to the right of the scene
	// buttons, TRANSITION, FILL-IN, LANDING, INTRO and OUTRO from the top, lit
	// like the colours: 0 all grey, 1 TRANSITION lit in yellow, 2 FILL-IN lit
	// in orange, 3 LANDING lit in blue, 4 INTRO lit in pink, 5 OUTRO lit in
	// turquoise.
	public: static const int kTransitionDiagramsFrameCount = 1 + kEditingColourCount;
	public: int GetTransitionDiagramsFrame() const;

	// What the fill-in button shows: dim white, or vibrant orange while a
	// fill-in is playing. The value is encoded like the light of a launch
	// button.
	public: TLight GetFillInLight() const;
	public: int GetFillInLightValue() const;
	public: int GetFillInLightFrame() const;
	// The frame of the FILL-IN caption: 0 white, 1 orange while a fill-in is
	// playing.
	public: int GetFillInLabelFrame() const;

	// What the clear button shows: dim white in launch mode, vibrant red in
	// clear mode, unlit in setup mode and duplicate mode, where it does
	// nothing.
	public: TLight GetClearLight() const;
	public: int GetClearLightFrame() const;
	// The frame of the CLEAR caption: 0 white, 1 dark grey in setup mode and
	// duplicate mode, 2 red in clear mode.
	public: int GetClearCaptionFrame() const;

	// What the duplicate button shows: dim white in launch mode, vibrant
	// fuchsia in duplicate mode, unlit in setup mode and clear mode, where it
	// does nothing.
	public: TLight GetDuplicateLight() const;
	public: int GetDuplicateLightFrame() const;
	// The frame of the DUPLICATE caption: 0 white, 1 dark grey in setup mode
	// and clear mode, 2 fuchsia in duplicate mode.
	public: int GetDuplicateCaptionFrame() const;

	// What the forward button shows: vibrant orange in clip editing, vibrant
	// blue in fill-in editing, vibrant pink in landing editing and vibrant
	// turquoise in intro editing of a launch button that holds a clip, unlit
	// otherwise. Encoded like the fill-in button's light.
	public: TLight GetForwardLight() const;
	public: int GetForwardLightValue() const;
	public: int GetForwardLightFrame() const;
	// The frame of its labels, one line for each step it can go forward to:
	// 0 all unlit, 1 SET FILL-IN in orange, 2 SET LANDING in blue, 3 SET INTRO
	// in pink, 4 SET OUTRO in turquoise.
	public: static const int kForwardLabelFrameCount = 5;
	public: int GetForwardLabelFrame() const;

	// The frame of the scene labels' film strip to show: 0 unlit, 1 white, 2
	// green. Every label is green in clip picking; otherwise only the label of
	// the current length is lit, in white.
	public: static const int kSceneLabelFrameCount = 3;
	public: int GetSceneLabelFrame(int iScene) const;

	// The CV value a track's CV output emits: in pattern mode 0..1 in the
	// steps a Combinator reads as pattern selection, in unipolar mode 0..1, in
	// bipolar mode -1..+1.
	public: double GetCVValue(int iTrack, int iOutput) const;

	// The CV a unipolar or bipolar value stands for.
	public: static double UnipolarCV(int iValue);
	public: static double BipolarCV(int iValue);
	// How far a transition has come, 0..1, after the share iTime of its time.
	public: static double TransitionCurve(int iTransition, double iTime);
	// The same for a landing's transition, which runs the other way round: B
	// and D leave quickly and settle slowly, a clip's curve mirrored in time.
	// An outro's transition follows it too.
	public: static double LandingCurve(int iTransition, double iTime);

	// A length in PPQ; 0 for immediate, which has no switch intervals.
	public: static long long ComputeSwitchIntervalLength(int iLength, int iNumerator, int iDenominator);

	// What a clip holds, and also what its fill-in, its landing, its intro and
	// its outro hold.
	private: struct TSettings {
		bool fPresent;
		int fColour;
		int fPatterns[kOutputCount];
		int fUnipolar[kOutputCount];
		int fBipolar[kOutputCount];
		int fTransitions[kOutputCount];
		// Only a fill-in's, a landing's, an intro's and an outro's mean
		// anything.
		bool fOneBarPattern;
	};

	private: struct TClip : TSettings {
		TSettings fFillIn;
		TSettings fLanding;
		TSettings fIntro;
		TSettings fOutro;
	};

	// Which of a clip's settings a setter changes.
	private: enum ESettingsKind {
		kClipSettings,
		kFillInSettings,
		kLandingSettings,
		kIntroSettings,
		kOutroSettings
	};

	// A transition of a CV output in unipolar or bipolar mode, under way or
	// waiting for its last bar. It starts over whenever what it was set up for
	// changes.
	private: struct TMovement {
		bool fOn;
		// The song position at which it begins to move.
		double fStart;
		// Where it moves from: the value it started over from, if it was moving
		// then, or otherwise the active clip's value, whatever that is by now.
		bool fFromFrozen;
		double fFrom;
		// What it was set up for: the row it moves towards, the transition, the
		// value it moves to and the CV mode.
		int fRow;
		int fTransition;
		int fValue;
		int fCVMode;
	};
	private: static const TMovement kNoMovement;

	private: bool ContainsSwitchPoint(const TTransport& iTransport) const;
	// How many segments of its switch interval a song position has reached,
	// 1..8, or 0 before the start of the song and with the length immediate.
	private: int CountReachedSegments(const TTransport& iTransport) const;
	private: static bool IsValidButton(int iTrack, int iRow);

	private: const TClip& Clip(int iTrack, int iRow) const;
	private: TSettings* EditableSettings(int iTrack, int iRow, int iOutput, ESettingsKind iKind);
	private: TSettings* EditableSettings(int iTrack, int iRow, ESettingsKind iKind);
	// The value that settings hold for a CV output in a CV mode other than
	// pattern: an index of the seven steps, and its CV.
	private: static int ValueIndex(const TSettings& iSettings, int iOutput, int iCVMode);
	private: static double ValueCV(const TSettings& iSettings, int iOutput, int iCVMode);
	// Works out what the CV outputs in unipolar and bipolar mode emit, with
	// their transitions, once a batch has switched.
	private: void UpdateValues(const TTransport& iTransport);
	private: double ComputeClipValue(int iTrack, int iOutput, const TTransport& iTransport);
	// What the active clip's landing emits on a CV output in unipolar or
	// bipolar mode: from the landing's value at the start of the switch
	// interval towards the clip's.
	private: double ComputeLandingValue(int iTrack, int iOutput, const TTransport& iTransport) const;
	private: double ComputeFillInValue(int iTrack, int iOutput, double iBelow, const TTransport& iTransport);
	// What a stopped track emits on a CV output in unipolar or bipolar mode:
	// its intro, moving from the intro's value towards the queued clip's (or
	// its landing's); its outro, moving from what was emitted before the
	// switch point towards the outro's value, and holding that once it is
	// over; the value frozen when an intro was dropped; or else the value of
	// the clip it last played.
	private: double ComputeStoppedValue(int iTrack, int iOutput, const TTransport& iTransport) const;
	// The settings a clip ends up emitting at the switch point: its landing,
	// if it holds one, or its own.
	private: const TSettings& ArrivalSettings(int iTrack, int iRow) const;
	// Sets a movement up anew, from what is emitted now unless it had not
	// begun to move.
	private: void StartOver(TMovement& ioMovement, double iEmitted, int iRow, int iTransition, int iValue, int iCVMode, double iPosition, const TTransport& iTransport) const;
	// Where a movement stands at a song position.
	private: double MovementValue(const TMovement& iMovement, double iActiveValue, double iTo, double iPosition, const TTransport& iTransport) const;
	// The next switch point after a song position, and the length of a bar.
	private: double NextSwitchPoint(double iPosition, const TTransport& iTransport) const;
	// The switch point the song has just reached: the one within a batch, or
	// with iInBatch false the last one before it.
	private: double LastSwitchPoint(const TTransport& iTransport, bool iInBatch) const;
	private: static double BarLength(const TTransport& iTransport);
	// The selection a track is heading for: an empty launch button counts as
	// the stop button.
	private: int Target(int iTrack) const;
	private: void Switch();
	private: void StopAtOnce(int iTrack);
	private: bool HasClips(int iTrack) const;
	// Whether a pattern of the active clip on a CV output in pattern mode has
	// been changed and the track has not switched to it yet.
	private: bool HasPendingPattern(int iTrack) const;
	private: TLight GetLaunchModeLight(int iTrack, int iRow) const;
	private: TLight GetClipPickingLight(int iTrack, int iRow) const;
	private: TLight GetClearModeLight(int iTrack, int iRow) const;
	private: TLight GetDuplicateModeLight(int iTrack, int iRow) const;
	private: bool SceneHasClips(int iScene) const;
	// The light of a launch button in clip editing, fill-in editing or landing
	// editing, given the settings being edited and the CV modes of the picked
	// track.
	private: TLight GetEditingLight(int iTrack, int iRow, const TSettings& iSettings) const;
	private: const TClip& PickedClip() const;
	// The settings being edited: the picked clip's, or in fill-in editing its
	// fill-in's, or in landing editing its landing's.
	private: const TSettings& EditedSettings() const;
	private: int PickedTrack() const;
	// Whether a track is playing the fill-in of its active clip.
	private: bool PlaysFillIn(int iTrack) const;
	// Whether a track's CV outputs in pattern mode emit its fill-in's
	// patterns: while it plays the fill-in, but with one-bar pattern only in
	// the last bar before the switch point.
	private: bool PlaysFillInPatterns(int iTrack) const;
	// Whether a track's CV outputs in pattern mode emit its landing's
	// patterns: while it plays the landing, but with one-bar pattern only for
	// the landing's first bar.
	private: bool PlaysLandingPatterns(int iTrack) const;
	// The same for an intro, with one-bar pattern only in the last bar before
	// the switch point, and for an outro, with one-bar pattern only for the
	// outro's first bar.
	private: bool PlaysIntroPatterns(int iTrack) const;
	private: bool PlaysOutroPatterns(int iTrack) const;
	// Whether a stopped track plays the intro of a clip it has queued.
	private: bool PlaysIntroOf(int iTrack, int iRow) const;
	// Stops emitting what an intro or an outro left behind, when a clip
	// becomes active or the track is stopped at once.
	private: void ForgetIntroAndOutro(int iTrack);
	// Makes a stopped track hold what it emits now, when an intro or an outro
	// is dropped.
	private: void Freeze(int iTrack);
	// The frame of a button's film strip that shows a light right now; a
	// flashing light alternates between dim and vibrant.
	private: int LightFrame(const TLight& iLight) const;
	// Whether a light shows vibrant right now, as opposed to dim.
	private: bool ShowsVibrant(const TLight& iLight) const;
	// Whether the song position has moved other than by playing on since the
	// last call of Advance().
	private: bool IsJump(const TTransport& iTransport) const;
	private: void EndMutesAndSolos();
	private: static int FunctionColour(int iFunction);

	private: TClip fClips[kTrackCount][kRowCount];
	private: int fSetup;
	private: int fLength;
	private: int fSelections[kTrackCount];
	private: int fActive[kTrackCount];
	// 0 for nothing, 1..8 for the pattern each CV output is emitting in
	// pattern mode. Changes only when the track switches, even if the active
	// clip is edited meanwhile.
	private: int fEmitted[kTrackCount][kOutputCount];
	private: int fCVModes[kTrackCount][kOutputCount];
	// The row of the clip a track last played, whose values a stopped track
	// goes on emitting in unipolar and bipolar mode; 0 before any.
	private: int fLastActive[kTrackCount];
	// What the CV outputs in unipolar and bipolar mode emit: underneath any
	// fill-in, and altogether.
	private: double fClipValues[kTrackCount][kOutputCount];
	private: double fValues[kTrackCount][kOutputCount];
	private: TMovement fMovements[kTrackCount][kOutputCount];
	// The transitions of the fill-ins being played, which lie over those of
	// the clips; fRow is the row of the clip that holds the fill-in.
	private: TMovement fFillInMovements[kTrackCount][kOutputCount];
	// The row of the clip whose landing a track plays, from the switch point
	// at which it became active until the next one; 0 for none. fLandingStart
	// is that switch point.
	private: int fLandingRows[kTrackCount];
	private: double fLandingStarts[kTrackCount];
	// The clip a stopped track has queued since fIntroStarts, which decides
	// whether its intro plays; 0 for none. An intro plays while
	// fIntroPlaying, which a jump of the song position switches off.
	private: int fIntroRows[kTrackCount];
	private: double fIntroStarts[kTrackCount];
	private: bool fIntroPlaying[kTrackCount];
	// The row of the clip whose outro a stopped track plays, or plays no more
	// but goes on emitting the values of, from the switch point fOutroStarts
	// on; 0 for none. fOutroFrom is what each CV output emitted before it, in
	// the CV mode fOutroFromModes.
	private: int fOutroRows[kTrackCount];
	private: double fOutroStarts[kTrackCount];
	private: bool fOutroPlaying[kTrackCount];
	private: double fOutroFrom[kTrackCount][kOutputCount];
	private: int fOutroFromModes[kTrackCount][kOutputCount];
	// The values a stopped track holds once its intro was dropped, while
	// fFrozen is set.
	private: bool fFrozen[kTrackCount];
	private: double fFrozenValues[kTrackCount][kOutputCount];
	private: bool fFlashLit;
	// Musical time that has passed while the transport was stopped, so that
	// flashing goes on at the song's tempo.
	private: double fStoppedTime;
	private: bool fSwitchPointInPreviousBatch;
	// How many scene buttons the interval meter has lit, from the bottom.
	private: int fReachedSegments;
	private: bool fFillInPressed;
	// Set by a press of the fill-in button, until the next call of Advance().
	private: bool fFillInTriggered;
	private: bool fFillInPlaying;
	private: int fFunction;
	private: bool fRelaunchPressed[kTrackCount];
	// Set by a relaunch press, until the next call of Advance().
	private: bool fRelaunchTriggered[kTrackCount];
	private: bool fRelaunched[kTrackCount];
	private: bool fMutePressed[kTrackCount];
	private: bool fSoloPressed[kTrackCount];
	// Set by a press of a mute or solo button, until the next call of
	// Advance().
	private: bool fMuteTriggered[kTrackCount];
	private: bool fSoloTriggered[kTrackCount];
	private: bool fMuted[kTrackCount];
	private: bool fSoloed[kTrackCount];
	// The transport of the last call of Advance(), to tell a jump of the song
	// position from playing on.
	private: bool fHasPreviousTransport;
	private: TTransport fPreviousTransport;
};
