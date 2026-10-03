#pragma once

// The switching rules of Launchnik, free of any dependency on the Jukebox SDK
// so that they can be compiled and tested on the command line (see tests/).
//
// Vocabulary follows CONTEXT.md: track, selection, off / queued / active,
// switch interval, switch point.

class CLauncher {
	public: static const int kTrackCount = 8;

	// A selection is 0 for the stop button or 1..8 for the launch button of that
	// pattern, so a track has nine buttons.
	public: static const int kSelectionStop = 0;
	public: static const int kSelectionCount = 9;

	public: static const int kBarsPerSwitchInterval = 4;

	// Musical time is counted in PPQ, the host's unit for song position.
	public: static const long long kPPQPerCrotchet = 15360;

	public: enum EButtonState {
		kOff = 0,
		kQueued = 1,
		kActive = 2
	};

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

	// Makes every track emit the given selections at once, with nothing queued.
	// Used when a song is opened.
	public: void Reset(const int iSelections[kTrackCount]);

	// Records what the user chose on a track. Call before Advance() for changes
	// that arrived in the same batch.
	public: void SetSelection(int iTrack, int iSelection);

	// Moves on by one batch.
	public: void Advance(const TTransport& iTransport);

	public: EButtonState GetButtonState(int iTrack, int iSelection) const;

	// Whether the light of a button is shining right now; a queued button flashes.
	public: bool IsLit(int iTrack, int iSelection) const;

	// The CV value a track emits, in the range 0..1.
	public: double GetCVValue(int iTrack) const;

	public: static long long ComputeSwitchIntervalLength(int iNumerator, int iDenominator);

	private: static bool ContainsSwitchPoint(const TTransport& iTransport);

	private: int fSelections[kTrackCount];
	private: int fActive[kTrackCount];
	private: bool fFlashLit;
	private: bool fSwitchPointInPreviousBatch;
};
