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
		// fValue is the length of the switch interval.
		kLength,
		// fIndex is the number of the launch button, see CLauncher::ButtonIndex.
		kClipPresent,
		kClipPattern,
		kClipColour,
		kFillInPresent,
		kFillInPattern,
		kFillInColour,
		// fValue is 1 while the fill-in button is held down, 0 once it is let go.
		kFillIn,
		// fValue is the function.
		kFunction,
		// fIndex is the track, fValue 1 while its mute or solo button is held
		// down, 0 once it is let go.
		kMute,
		kSolo
	};

	// One property and its new value. A press of a launch button outside
	// fill-in editing, a function button, a function select button, the setup
	// button, the back button, the set fill-in button or the fill-in button
	// never changes more than one.
	public: struct TChange {
		EProperty fProperty;
		int fIndex;
		int fValue;
	};

	// What a press of a scene button changes: the selection of every track,
	// the length and the setup state, or nothing at all (see docs/adr/0004).
	// What a press of a launch button in fill-in editing changes: up to three
	// settings of the fill-in.
	public: static const int kMaxChangeCount = 8;
	public: struct TChanges {
		int fCount;
		TChange fChanges[kMaxChangeCount];
	};

	// iPressedHoldsClip says whether the pressed launch button holds a clip.
	// iPickedHoldsClip says the same of the launch button that is being edited
	// and only matters in clip editing. In fill-in editing a press changes
	// nothing here; see FillInEditingLaunchButton().
	public: static TChange LaunchButton(int iSetup, int iTrack, int iRow, bool iPressedHoldsClip, bool iPickedHoldsClip);
	// A press of a launch button in fill-in editing. iPickedHoldsFillIn says
	// whether the launch button being edited holds a fill-in, iPickedPattern is
	// the pattern of its clip, which a new fill-in starts with, in white.
	// Outside fill-in editing a press changes nothing here.
	public: static TChanges FillInEditingLaunchButton(int iSetup, int iTrack, int iRow, bool iPickedHoldsFillIn, int iPickedPattern);

	public: static TChange StopButton(int iTrack);

	// Holding down a function button and letting it go. In stop function a
	// press chooses stop and letting go does nothing; in mute and solo function
	// the button is held down, as the mute or solo button of its track, for as
	// long as it is, and whether a press mutes or solos is up to the realtime
	// code. iTrackHasClips says whether the track holds any clip: without one,
	// a press does nothing. Letting go is always passed on, so that a button
	// that is held down can be let go of.
	public: static TChange FunctionButton(int iFunction, int iTrack, bool iTrackHasClips, bool iPressed);

	// Chooses the function of a function select button.
	public: static TChange FunctionSelectButton(int iFunction);

	public: static TChange SetupButton(int iSetup);

	// Goes back one step of setup mode: from fill-in editing to clip editing of
	// the same launch button, from clip editing to clip picking, from clip
	// picking to launch mode. In launch mode it does nothing.
	public: static TChange BackButton(int iSetup);
	// Goes on from clip editing to fill-in editing of the same launch button,
	// but only if it holds a clip. Anywhere else it does nothing.
	public: static TChange SetFillInButton(int iSetup, bool iPickedHoldsClip);
	// Holding down the fill-in button and letting it go; whether a press
	// triggers a fill-in is up to the realtime code.
	public: static TChange FillInButton(bool iPressed);

	// In launch mode, a scene button means what the eight launch buttons of
	// its scene mean, track by track. In clip picking it chooses the length it
	// stands for and returns to launch mode. In clip editing it does nothing.
	// iHoldsClip says of each track whether its launch button in the scene
	// holds a clip.
	public: static TChanges SceneButton(int iSetup, int iScene, const bool iHoldsClip[]);
};
