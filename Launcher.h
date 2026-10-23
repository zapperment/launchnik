#pragma once

// The rules of Launchnik, free of any dependency on the Jukebox SDK so that
// they can be compiled and tested on the command line (see tests/): when a
// track switches, what it emits, and what every button shows in launch mode
// and in setup mode.
//
// Vocabulary follows CONTEXT.md: track, clip, empty, selection, off / queued /
// active, light, label, mode colour, switch interval, switch point, length,
// immediate, clip picking, clip editing, fill-in, fill-in editing, landing,
// landing editing, forward button, scene,
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

	public: static const int kColourCount = 14;
	public: static const int kColourWhite = 0;
	public: static const int kColourRed = 1;
	public: static const int kColourOrange = 2;
	public: static const int kColourYellow = 3;
	public: static const int kColourLime = 4;
	public: static const int kColourGreen = 5;
	public: static const int kColourCyan = 7;
	public: static const int kColourLightBlue = 8;
	public: static const int kColourBlue = 9;
	public: static const int kColourPurple = 11;
	public: static const int kColourFuchsia = 12;
	public: static const int kColourPink = 13;

	// The setup state is one number: launch mode, clip picking, or clip
	// editing, fill-in editing or landing editing for one launch button.
	public: static const int kSetupLaunchMode = 0;
	public: static const int kSetupClipPicking = 1;
	public: static const int kSetupFirstClipEditing = 2;
	public: static const int kSetupFirstFillInEditing = kSetupFirstClipEditing + kButtonCount;
	public: static const int kSetupFirstLandingEditing = kSetupFirstFillInEditing + kButtonCount;
	public: static const int kSetupCount = kSetupFirstLandingEditing + kButtonCount;

	// Where the controls of clip editing sit on the grid; fill-in editing and
	// landing editing have them in the same places. The colours fill the first seven tracks of rows
	// 1 and 2, and each of rows 3 to 8 stands for one CV output, A to F: eight
	// patterns in pattern mode, otherwise seven values and, in the last track,
	// the transition.
	public: static const int kEditingToggleTrack = 7;
	public: static const int kEditingToggleRow = 1;
	public: static const int kEditingFirstColourRow = 1;
	public: static const int kEditingColoursPerRow = 7;
	public: static const int kEditingFirstOutputRow = 3;
	public: static const int kEditingTransitionTrack = 7;

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

	// The film strip of the function buttons: frame 0 is unlit, then dim and
	// vibrant for red (stop), yellow (mute) and blue (solo).
	public: static const int kFunctionLightFrameCount = 7;

	// The film strip of the fill-in button and the function select buttons:
	// frame 0 is unlit, then dim and vibrant for red, green, yellow, blue and
	// white.
	public: static const int kSideLightFrameCount = 11;

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

	// The film strip of the launch buttons: frame 0 is unlit, then dim and
	// vibrant for each colour.
	public: static const int kLightFrameCount = 1 + 2 * kColourCount;

	// The film strip of the scene buttons and the forward button: frame 0 is
	// unlit, then dim and vibrant for red, green, yellow, white and blue.
	public: static const int kSceneLightFrameCount = 11;

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
	public: static bool IsClipEditing(int iSetup);
	public: static bool IsFillInEditing(int iSetup);
	public: static bool IsLandingEditing(int iSetup);
	// The number of the launch button being edited in clip editing, fill-in
	// editing or landing editing, or -1 in launch mode and clip picking.
	public: static int PickedButton(int iSetup);

	// Record what a launch button holds. A launch button keeps its settings
	// while it is empty, so that a clip comes back as it was. A new clip has,
	// on every CV output, the pattern of its row, unipolar value 0, bipolar
	// value 0 and transition off.
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

	// Record the landing a launch button holds, which it keeps while it is
	// empty, like its clip's settings.
	public: void SetLandingPresent(int iTrack, int iRow, bool iPresent);
	public: void SetLandingColour(int iTrack, int iRow, int iColour);
	public: void SetLandingPattern(int iTrack, int iRow, int iOutput, int iPattern);
	public: void SetLandingUnipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetLandingBipolar(int iTrack, int iRow, int iOutput, int iValue);
	public: void SetLandingTransition(int iTrack, int iRow, int iOutput, int iTransition);
	// Whether a track is playing the landing of its active clip: during the
	// switch interval that began when the clip became active at a switch
	// point, or went on there after playing its fill-in, unless the length is
	// immediate.
	public: bool PlaysLanding(int iTrack) const;

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
	// function is chosen, dim white otherwise. The frame is one of the side
	// buttons' film strip.
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
	// current length and in white otherwise. Scenes are numbered like rows,
	// 1..8 from the top, so the meter fills from scene 8 upwards. The value is
	// encoded like the light of a launch button.
	public: TLight GetSceneLight(int iScene) const;
	public: int GetSceneLightValue(int iScene) const;
	public: int GetSceneLightFrame(int iScene) const;

	// The mode colour, worn by the logo and the lit labels: white in launch
	// mode, green in clip picking, yellow in clip editing, red in fill-in
	// editing, blue in landing editing. Also the frame of the logo's film
	// strip.
	public: enum EModeColour {
		kModeWhite = 0,
		kModeGreen = 1,
		kModeYellow = 2,
		kModeRed = 3,
		kModeBlue = 4
	};
	public: EModeColour GetModeColour() const;

	// The frame of the back button's film strip to show: dark in launch mode,
	// vibrant white in clip picking, vibrant green in clip editing, vibrant
	// yellow in fill-in editing, vibrant red in landing editing. The strip has
	// the frames of the scene button's source image, in its order.
	public: static const int kBackButtonFrameCount = 9;
	public: static const int kBackButtonDark = 0;
	public: static const int kBackButtonVibrantRed = 1;
	public: static const int kBackButtonVibrantGreen = 3;
	public: static const int kBackButtonVibrantYellow = 5;
	public: static const int kBackButtonVibrantWhite = 7;
	public: int GetBackButtonFrame() const;

	// The frame of the back button's labels to show, one line for each step
	// it can go back to, lit in the colour of the button: 0 all unlit, 1
	// LAUNCH in white, 2 PICK CLIP in green, 3 EDIT CLIP in yellow, 4 SET
	// FILL-IN in red.
	public: static const int kBackLabelFrameCount = 5;
	public: int GetBackLabelFrame() const;

	// The frame of the launch labels' film strip to show, for rows 1 and 2: 0
	// unlit, then SET/CLEAR lit and all lit, in yellow (1, 2), red (3, 4) and
	// blue (5, 6). They are lit only in clip editing, in yellow, in fill-in
	// editing, in red, and in landing editing, in blue; the labels of colours
	// only while there is something to set up: the picked launch button's
	// clip, fill-in or landing.
	public: static const int kLaunchLabelsFrameCount = 7;
	public: int GetLaunchLabelsFrame() const;

	// The frame of the film strip of the labels of a CV output's row, lit like
	// the colours': 0 unlit, then for yellow, red and blue in turn the
	// patterns, the unipolar values with each of the five transitions, and the
	// bipolar values with each of the five transitions. Only the labels that
	// go with the output's CV mode are lit, and of the transitions only the
	// one chosen.
	public: static const int kOutputLabelsFramesPerColour = 1 + 2 * kTransitionCount;
	public: static const int kOutputLabelsFrameCount = 1 + 3 * kOutputLabelsFramesPerColour;
	public: int GetOutputLabelsFrame(int iOutput) const;

	// The three sets of transition diagrams to the right of the scene
	// buttons, TRANSITION, FILL-IN and LANDING from the top, lit like the
	// colours: 0 all grey, 1 TRANSITION lit in yellow, 2 FILL-IN lit in red,
	// 3 LANDING lit in blue.
	public: static const int kTransitionDiagramsFrameCount = 4;
	public: int GetTransitionDiagramsFrame() const;

	// What the fill-in button shows: dim white, or vibrant red while a fill-in
	// is playing. The value is encoded like the light of a launch button, the
	// frame is one of the side buttons' film strip.
	public: TLight GetFillInLight() const;
	public: int GetFillInLightValue() const;
	public: int GetFillInLightFrame() const;
	// The frame of the FILL-IN caption: 0 white, 1 red while a fill-in is
	// playing.
	public: int GetFillInLabelFrame() const;

	// What the forward button shows: vibrant red in clip editing and vibrant
	// blue in fill-in editing of a launch button that holds a clip, unlit
	// otherwise. Encoded like the fill-in button's light, framed like a scene
	// button's.
	public: TLight GetForwardLight() const;
	public: int GetForwardLightValue() const;
	public: int GetForwardLightFrame() const;
	// The frame of its labels, one line for each step it can go forward to:
	// 0 both unlit, 1 SET FILL-IN in red, 2 SET LANDING in blue.
	public: static const int kForwardLabelFrameCount = 3;
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
	public: static double LandingCurve(int iTransition, double iTime);

	// A length in PPQ; 0 for immediate, which has no switch intervals.
	public: static long long ComputeSwitchIntervalLength(int iLength, int iNumerator, int iDenominator);

	// What a clip holds, and also what its fill-in and its landing hold.
	private: struct TSettings {
		bool fPresent;
		int fColour;
		int fPatterns[kOutputCount];
		int fUnipolar[kOutputCount];
		int fBipolar[kOutputCount];
		int fTransitions[kOutputCount];
	};

	private: struct TClip : TSettings {
		TSettings fFillIn;
		TSettings fLanding;
	};

	// Which of a clip's settings a setter changes.
	private: enum ESettingsKind {
		kClipSettings,
		kFillInSettings,
		kLandingSettings
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
	private: bool HasClips(int iTrack) const;
	// Whether a pattern of the active clip on a CV output in pattern mode has
	// been changed and the track has not switched to it yet.
	private: bool HasPendingPattern(int iTrack) const;
	private: TLight GetLaunchModeLight(int iTrack, int iRow) const;
	private: TLight GetClipPickingLight(int iTrack, int iRow) const;
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
	// The frame of a film strip laid out like the scene buttons': unlit, then
	// dim and vibrant for red, green, yellow, white and blue.
	private: int SceneStripFrame(const TLight& iLight) const;
	// The frame of the side buttons' film strip: unlit, then dim and vibrant
	// for red, green, yellow, blue and white.
	private: int SideStripFrame(const TLight& iLight) const;
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
