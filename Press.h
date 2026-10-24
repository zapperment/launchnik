#pragma once

#include "Launcher.h"

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
		// fIndex is the number of the launch button, see CLauncher::ButtonIndex;
		// for the settings of a CV output, fOutput is the output.
		kClipPresent,
		kClipColour,
		kClipPattern,
		kClipUnipolar,
		kClipBipolar,
		kClipTransition,
		kFillInPresent,
		kFillInColour,
		kFillInPattern,
		kFillInUnipolar,
		kFillInBipolar,
		kFillInTransition,
		kFillInOneBarPattern,
		kLandingPresent,
		kLandingColour,
		kLandingPattern,
		kLandingUnipolar,
		kLandingBipolar,
		kLandingTransition,
		kLandingOneBarPattern,
		// fValue is 1 while the fill-in button is held down, 0 once it is let go.
		kFillIn,
		// fValue is the function.
		kFunction,
		// fIndex is the track, fValue 1 while its mute or solo button is held
		// down, 0 once it is let go.
		kMute,
		kSolo,
		// fIndex is the track, fValue 1 while a launch on it is held down, 0
		// once it is let go.
		kRelaunch
	};

	// One property and its new value. A press of a launch button outside
	// fill-in editing, landing editing, clear mode and duplicate mode, a
	// function button, a function select button, the setup button, the clear
	// button, the duplicate button, the back button, the forward button or the
	// fill-in button never changes more than one.
	public: struct TChange {
		EProperty fProperty;
		int fIndex;
		int fOutput;
		int fValue;
	};

	// How many properties a launch button holds: of its clip, its fill-in and
	// its landing, whether there is one, the colour and the four settings of
	// every CV output, and of the fill-in and the landing one-bar pattern.
	public: static const int kButtonSettingCount = 3 * (2 + 4 * CLauncher::kOutputCount) + 2;

	// What a press of a scene button changes: the selection and the relaunch
	// press of every track, the length and the setup state, what every track
	// holds in its scene and the setup state, or nothing at all (see
	// docs/adr/0004).
	// What a press of a launch button in fill-in editing or landing editing
	// changes: up to every setting of the fill-in or the landing; in
	// destination picking, every setting of the destination and the setup
	// state.
	public: static const int kMaxChangeCount = kButtonSettingCount + 1;
	static_assert(kMaxChangeCount >= 2 * CLauncher::kTrackCount, "a scene press changes two properties of every track");
	static_assert(kMaxChangeCount >= 3 + 4 * CLauncher::kOutputCount, "a press in fill-in editing sets up a whole fill-in");
	public: struct TChanges {
		int fCount;
		TChange fChanges[kMaxChangeCount];
	};

	// What a press in clip editing, fill-in editing or landing editing needs
	// to know of the settings being edited: whether there are any (the picked
	// launch button holds a clip, or in fill-in editing a fill-in, or in
	// landing editing a landing), the CV modes of the picked launch button's
	// track, and the transitions, which a press moves on to the next; of a
	// fill-in or a landing also whether one-bar pattern is on, which a press
	// switches.
	public: struct TEdited {
		bool fPresent;
		int fCVModes[CLauncher::kOutputCount];
		int fTransitions[CLauncher::kOutputCount];
		bool fOneBarPattern;
	};
	// Settings with a new device's CV modes, every transition off and
	// one-bar pattern off.
	public: static TEdited Edited(bool iPresent);

	// A clip's values, which a new fill-in or landing starts as a copy of.
	public: struct TClipValues {
		int fPatterns[CLauncher::kOutputCount];
		int fUnipolar[CLauncher::kOutputCount];
		int fBipolar[CLauncher::kOutputCount];
	};

	// Everything a launch button holds, which a duplicate copies: the
	// settings of its clip, its fill-in and its landing, whether or not
	// they are there. Only a fill-in's and a landing's one-bar pattern mean
	// anything.
	public: struct TSettings {
		bool fPresent;
		int fColour;
		int fPatterns[CLauncher::kOutputCount];
		int fUnipolar[CLauncher::kOutputCount];
		int fBipolar[CLauncher::kOutputCount];
		int fTransitions[CLauncher::kOutputCount];
		bool fOneBarPattern;
	};
	public: struct TButton {
		TSettings fClip;
		TSettings fFillIn;
		TSettings fLanding;
	};

	// iPressedHoldsClip says whether the pressed launch button holds a clip.
	// iPicked describes the clip being edited and only matters in clip
	// editing. In fill-in editing, landing editing and clear mode a press
	// changes nothing here; see FillInEditingLaunchButton(),
	// LandingEditingLaunchButton() and ClearModeLaunchButton().
	public: static TChange LaunchButton(int iSetup, int iTrack, int iRow, bool iPressedHoldsClip, const TEdited& iPicked);
	// A press of a launch button in fill-in editing. iFillIn describes the
	// fill-in being edited, iClip the values of its clip, which a new fill-in
	// starts as a copy of, with every transition off and one-bar pattern
	// off, in white. Outside fill-in editing a press changes nothing here.
	public: static TChanges FillInEditingLaunchButton(int iSetup, int iTrack, int iRow, const TEdited& iFillIn, const TClipValues& iClip);
	// The same in landing editing, for the landing.
	public: static TChanges LandingEditingLaunchButton(int iSetup, int iTrack, int iRow, const TEdited& iLanding, const TClipValues& iClip);
	// A press of a launch button in clear mode empties it, if it holds a
	// clip, as SET/CLEAR does in clip editing, and returns to launch mode.
	// Outside clear mode a press changes nothing here.
	public: static TChanges ClearModeLaunchButton(int iSetup, int iTrack, int iRow, bool iPressedHoldsClip);
	// A press of a launch button in duplicate mode. In source picking it picks
	// the launch button as the source, if it holds a clip. In destination
	// picking for a launch button it makes the pressed one exactly like the
	// source, iSource, and returns to launch mode; a press of the source
	// itself does nothing. With a scene as the source, and outside duplicate
	// mode, a press changes nothing here.
	public: static TChanges DuplicateModeLaunchButton(int iSetup, int iTrack, int iRow, bool iPressedHoldsClip, const TButton& iSource);
	// A press of a scene button in duplicate mode. In source picking it picks
	// the scene as the source, if it holds a clip. In destination picking for
	// a scene it returns to launch mode, and the caller makes every launch
	// button of the pressed scene exactly like the one of the same track in
	// the source scene, empty ones included, with CopyButton(), before that
	// change; a press of the source itself does nothing. With a launch button
	// as the source, and outside duplicate mode, a press changes nothing here.
	// A whole scene's changes are too many to return at once.
	public: static TChange DuplicateModeSceneButton(int iSetup, int iScene, bool iSceneHoldsClip);
	// What makes a launch button, iDestination, exactly like iSource.
	public: static TChanges CopyButton(const TButton& iSource, int iDestination);

	// Holding down a launch button and letting it go, or a scene button for
	// each track: in launch mode a press holds the track's relaunch press down
	// as well as choosing the selection (see LaunchButton()); whether it
	// relaunches is up to the realtime code. Elsewhere a press changes nothing
	// here. Letting go is always passed on.
	public: static TChange RelaunchButton(int iSetup, int iTrack, bool iPressed);

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

	// From launch mode to clip picking, and from any step of setup mode, from
	// clear mode or from duplicate mode back to launch mode.
	public: static TChange SetupButton(int iSetup);
	// From launch mode to clear mode and back. In setup mode and duplicate
	// mode it does nothing.
	public: static TChange ClearButton(int iSetup);
	// From launch mode to duplicate mode, and from either step of it back. In
	// setup mode and clear mode it does nothing.
	public: static TChange DuplicateButton(int iSetup);

	// Goes back one step of setup mode: from landing editing to fill-in
	// editing and from fill-in editing to clip editing of the same launch
	// button, from clip editing to clip picking, from clip picking, from clear
	// mode and from duplicate mode to launch mode. In launch mode it does
	// nothing.
	public: static TChange BackButton(int iSetup);
	// Goes forward one step of setup mode: from clip editing to fill-in
	// editing and from fill-in editing to landing editing of the same launch
	// button, but only if it holds a clip. Anywhere else it does nothing.
	public: static TChange ForwardButton(int iSetup, bool iPickedHoldsClip);
	// Holding down the fill-in button and letting it go; whether a press
	// triggers a fill-in is up to the realtime code.
	public: static TChange FillInButton(bool iPressed);

	// In launch mode, a scene button means what the eight launch buttons of
	// its scene mean, track by track: the selections, then the relaunch
	// presses. In clip picking it chooses the length it
	// stands for and returns to launch mode. In clear mode it empties every
	// launch button of its scene that holds a clip and returns to launch mode.
	// In clip editing it does nothing, and in duplicate mode nothing here; see
	// DuplicateModeSceneButton().
	// iHoldsClip says of each track whether its launch button in the scene
	// holds a clip.
	public: static TChanges SceneButton(int iSetup, int iScene, const bool iHoldsClip[]);
};
