#pragma once

// The rules of Launchnik, free of any dependency on the Jukebox SDK so that
// they can be compiled and tested on the command line (see tests/): when a
// track switches, what it emits, and what every button shows in launch mode
// and in setup mode.
//
// Vocabulary follows CONTEXT.md: track, clip, empty, selection, off / queued /
// active, light, switch interval, switch point, clip picking, clip editing.
//
// Tracks are numbered from 0 here. Rows are numbered from 1, so that a row is
// also the selection that chooses its launch button.

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
	public: static const int kColourGreen = 5;
	public: static const int kColourBlue = 9;
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
	public: static const int kEditingBackTrack = 7;
	public: static const int kEditingBackRow = 8;

	public: static const int kBarsPerSwitchInterval = 4;

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

	// The CV value a track emits, in the range 0..1.
	public: double GetCVValue(int iTrack) const;

	public: static long long ComputeSwitchIntervalLength(int iNumerator, int iDenominator);

	private: struct TClip {
		bool fPresent;
		int fPattern;
		int fColour;
	};

	private: static bool ContainsSwitchPoint(const TTransport& iTransport);
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
};
