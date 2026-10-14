#pragma once

// The rules of Launchnik, free of any dependency on the Jukebox SDK so that
// they can be compiled and tested on the command line (see tests/): when a
// track switches, what it emits, and what every button shows in launch mode
// and in setup mode.
//
// Vocabulary follows CONTEXT.md: track, clip, empty, selection, off / queued /
// active, light, label, mode colour, switch interval, switch point, length,
// immediate, clip picking, clip editing, scene, segment, interval meter.
//
// Tracks are numbered from 0 here. Rows are numbered from 1, so that a row is
// also the selection that chooses its launch button, and the number of its
// scene.

class CLauncher {
	public: static const int kTrackCount = 8;
	public: static const int kRowCount = 8;
	public: static const int kButtonCount = kTrackCount * kRowCount;

	// A selection is 0 for the stop button or 1..8 for the launch button in
	// that row, so a track has nine buttons.
	public: static const int kSelectionStop = 0;
	public: static const int kSelectionCount = 9;

	public: static const int kPatternCount = 8;

	public: static const int kColourCount = 14;
	public: static const int kColourWhite = 0;
	public: static const int kColourRed = 1;
	public: static const int kColourYellow = 3;
	public: static const int kColourGreen = 5;
	public: static const int kColourPink = 13;

	// The setup state is one number: launch mode, clip picking, or clip editing
	// for one launch button.
	public: static const int kSetupLaunchMode = 0;
	public: static const int kSetupClipPicking = 1;
	public: static const int kSetupFirstClipEditing = 2;
	public: static const int kSetupCount = kSetupFirstClipEditing + kButtonCount;

	// Where the controls of clip editing sit on the grid.
	public: static const int kEditingToggleTrack = 7;
	public: static const int kEditingToggleRow = 1;
	public: static const int kEditingFirstColourRow = 2;
	public: static const int kEditingColoursPerRow = 7;
	public: static const int kEditingPatternRow = 5;

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

	// The film strip of the scene buttons: frame 0 is unlit, then dim and
	// vibrant for red, green, yellow and white.
	public: static const int kSceneLightFrameCount = 9;

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

	// Record what a launch button holds. A launch button keeps its pattern and
	// colour while it is empty, so that a clip comes back as it was.
	public: void SetClipPresent(int iTrack, int iRow, bool iPresent);
	public: void SetClipPattern(int iTrack, int iRow, int iPattern);
	public: void SetClipColour(int iTrack, int iRow, int iColour);

	public: void SetSetup(int iSetup);

	public: void SetLength(int iLength);

	// The scene whose scene button stands for a length in clip picking, and
	// the other way round.
	public: static int SceneForLength(int iLength);
	public: static int LengthForScene(int iScene);

	// Records what the user chose on a track. Call before Advance() for changes
	// that arrived in the same batch.
	public: void SetSelection(int iTrack, int iSelection);

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
	// The frame of the stop buttons' film strip to show right now: unlit, dim
	// or vibrant; a flashing light alternates between the last two.
	public: int GetStopLightFrame(int iTrack) const;

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
	// mode, green in clip picking, yellow in clip editing. Also the frame of
	// the logo's film strip.
	public: enum EModeColour {
		kModeWhite = 0,
		kModeGreen = 1,
		kModeYellow = 2
	};
	public: EModeColour GetModeColour() const;

	// The frame of the back button's film strip to show: dark in launch mode,
	// vibrant white in clip picking, vibrant green in clip editing. The strip
	// has the frames of the scene button's source image, in its order.
	public: static const int kBackButtonFrameCount = 9;
	public: static const int kBackButtonDark = 0;
	public: static const int kBackButtonVibrantGreen = 3;
	public: static const int kBackButtonVibrantWhite = 7;
	public: int GetBackButtonFrame() const;

	// The frame of the launch labels' film strip to show: 0 unlit, 1 SET/CLEAR
	// lit, 2 all lit. They are lit, in yellow, only in clip editing,
	// and the labels of colours and patterns only while the picked launch
	// button holds a clip.
	public: static const int kLaunchLabelsFrameCount = 3;
	public: int GetLaunchLabelsFrame() const;

	// The frame of the scene labels' film strip to show: 0 unlit, 1 white, 2
	// green. Every label is green in clip picking; otherwise only the label of
	// the current length is lit, in white.
	public: static const int kSceneLabelFrameCount = 3;
	public: int GetSceneLabelFrame(int iScene) const;

	// The CV value a track emits, in the range 0..1.
	public: double GetCVValue(int iTrack) const;

	// A length in PPQ; 0 for immediate, which has no switch intervals.
	public: static long long ComputeSwitchIntervalLength(int iLength, int iNumerator, int iDenominator);

	private: struct TClip {
		bool fPresent;
		int fPattern;
		int fColour;
	};

	private: bool ContainsSwitchPoint(const TTransport& iTransport) const;
	// How many segments of its switch interval a song position has reached,
	// 1..8, or 0 before the start of the song and with the length immediate.
	private: int CountReachedSegments(const TTransport& iTransport) const;
	private: static bool IsValidButton(int iTrack, int iRow);

	private: const TClip& Clip(int iTrack, int iRow) const;
	// The selection a track is heading for: an empty launch button counts as
	// the stop button.
	private: int Target(int iTrack) const;
	private: void Switch();
	private: bool HasClips(int iTrack) const;
	// Whether the active clip's pattern has been changed and the track has not
	// switched to it yet.
	private: bool HasPendingPattern(int iTrack) const;
	private: TLight GetLaunchModeLight(int iTrack, int iRow) const;
	private: TLight GetClipPickingLight(int iTrack, int iRow) const;
	private: TLight GetClipEditingLight(int iTrack, int iRow) const;

	private: TClip fClips[kTrackCount][kRowCount];
	private: int fSetup;
	private: int fLength;
	private: int fSelections[kTrackCount];
	private: int fActive[kTrackCount];
	// 0 for nothing, 1..8 for the pattern each track is emitting. Changes only
	// when the track switches, even if the active clip is edited meanwhile.
	private: int fEmitted[kTrackCount];
	private: bool fFlashLit;
	// Musical time that has passed while the transport was stopped, so that
	// flashing goes on at the song's tempo.
	private: double fStoppedTime;
	private: bool fSwitchPointInPreviousBatch;
	// How many scene buttons the interval meter has lit, from the bottom.
	private: int fReachedSegments;
};
