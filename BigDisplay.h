#pragma once

// What the big display says, free of any dependency on the Jukebox SDK so
// that it can be compiled and tested on the command line (see tests/).
//
// Vocabulary follows CONTEXT.md: big display, shown clip, value table, output
// graph, launch
// mode, clip picking, clip editing, fill-in editing, landing editing, intro
// editing, outro editing, clear mode, source picking, destination picking, CV
// output, CV output name, CV mode, transition, one-bar pattern.

#include "Launcher.h"

class CBigDisplay {
	// The line that names the way of working, for a setup state as in
	// Launcher.h: LAUNCH MODE, the step of setup mode, or what to pick in
	// clear mode and duplicate mode.
	public: static const char* ModeLine(int iSetup);

	// The name shown while a track or a clip has none of its own (see
	// docs/adr/0007): Track 1 to Track 8, counting tracks from 0, and Clip 1
	// to Clip 8 by row. oName must have room for kDefaultNameSize characters.
	public: static const int kDefaultNameSize = 16;
	public: static void DefaultTrackName(int iTrack, char oName[]);
	public: static void DefaultClipName(int iRow, char oName[]);
	// CV Out 1A to CV Out 8F, counting tracks and CV outputs from 0.
	public: static void DefaultOutputName(int iTrack, int iOutput, char oName[]);

	// The heading of a column of a value table, counted from 0: Clip, Fill,
	// Lnd, Int or Out, followed in unipolar and bipolar mode by the letter of
	// the transition, unless that is off, and in pattern mode by a 1 while
	// one-bar pattern is on. oText must have room for kHeadingSize characters.
	public: static const int kHeadingSize = 8;
	public: static void FormatHeading(int iColumn, int iCVMode, int iMark, char oText[]);

	// A value of a value table: the number of a pattern, or a unipolar or
	// bipolar value given in tenths, with a decimal only if it is not 0 and
	// with a plus sign for a bipolar value above 0. oText must have room for
	// kValueSize characters.
	public: static const int kValueSize = 8;
	public: static void FormatValue(int iCVMode, int iValue, char oText[]);

	// How wide the host draws a text in its small LCD font, in pixels of the
	// panel. Characters outside ASCII count as the widest.
	public: static int SmallTextWidth(const char* iText);

	// The first row of a CV output's block: its name, then the CV mode in
	// parentheses. A name too long for iMaxWidth, in pixels of the panel, is
	// cut off so that the CV mode stays in view. oText must have room for
	// iSize characters.
	public: static void FormatOutputTitle(const char* iName, int iCVMode, int iMaxWidth, char oText[], int iSize);

	// What the realtime code tells the display, as the bytes of one string
	// property (see docs/adr/0009): the value table and the output graph of
	// every CV output of the shown clip's track, a graph as one level for
	// each of its columns, the oldest first.
	public: struct TState {
		CLauncher::TValueTable fTables[CLauncher::kOutputCount];
		unsigned char fGraphs[CLauncher::kOutputCount][CLauncher::kGraphColumnCount];
	};
	public: static const int kBytesPerColumn = 4;
	public: static const int kBytesPerTable = 1 + CLauncher::kTableColumnCount * kBytesPerColumn;
	public: static const int kStateSize = CLauncher::kOutputCount * (kBytesPerTable + CLauncher::kGraphColumnCount);
	public: static void Encode(const TState& iState, unsigned char oBytes[]);
	// Returns false, leaving every column blank and every graph empty, for
	// bytes that are not a
	// state, such as those of a property that has not been written yet.
	public: static bool Decode(const unsigned char iBytes[], int iSize, TState& oState);
};
