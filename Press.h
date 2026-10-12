#pragma once

// What pressing a button changes, free of any dependency on the Jukebox SDK.
// The front panel's click handler follows these rules; a codec for a control
// surface has to follow the same ones (see docs/adr/0002).
//
// Tracks are numbered from 0 and rows from 1, as in Launcher.h.

class CPress {
	public: enum EProperty {
		kNothing,
		// fIndex is the track, fValue the selection.
		kSelection,
		// fValue is the setup state.
		kSetup,
		// fIndex is the number of the launch button, see CLauncher::ButtonIndex.
		kClipPresent,
		kClipPattern,
		kClipColour
	};

	// One property and its new value. A press of a launch button, a stop
	// button or the setup button never changes more than one.
	public: struct TChange {
		EProperty fProperty;
		int fIndex;
		int fValue;
	};

	// What a press of a scene button changes: the selection of every track, or
	// nothing at all (see docs/adr/0004).
	public: static const int kMaxChangeCount = 8;
	public: struct TChanges {
		int fCount;
		TChange fChanges[kMaxChangeCount];
	};

	// iPressedHoldsClip says whether the pressed launch button holds a clip.
	// iPickedHoldsClip says the same of the launch button that is being edited
	// and only matters in clip editing.
	public: static TChange LaunchButton(int iSetup, int iTrack, int iRow, bool iPressedHoldsClip, bool iPickedHoldsClip);

	public: static TChange StopButton(int iTrack);

	public: static TChange SetupButton(int iSetup);

	// In launch mode, a scene button means what the eight launch buttons of
	// its scene mean, track by track; in setup mode it does nothing.
	// iHoldsClip says of each track whether its launch button in the scene
	// holds a clip.
	public: static TChanges SceneButton(int iSetup, int iScene, const bool iHoldsClip[]);
};
