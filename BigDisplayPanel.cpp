#include "BigDisplayPanel.h"

#include "BigDisplay.h"
#include "Launcher.h"

namespace {

// The places of the values in the display's list in hdgui_2D.lua.
const TJBox_UInt32 kParam_Setup = 0;
const TJBox_UInt32 kParam_ShownClip = 1;
const TJBox_UInt32 kParam_FirstTrackName = 2;
const TJBox_UInt32 kParam_FirstClipName = kParam_FirstTrackName + CLauncher::kTrackCount;
const TJBox_UInt32 kParam_State = kParam_FirstClipName + CLauncher::kButtonCount;
const TJBox_UInt32 kParam_FirstOutputName = kParam_State + 1;
const TJBox_UInt32 kParamCount = kParam_FirstOutputName + CLauncher::kTrackCount * CLauncher::kOutputCount;

// The layout, in pixels of the panel from the top left corner of the display,
// as measured in the screen design. The display counts in units of five
// pixels, which Rect() converts to.
const TJBox_Float32 kPixelsPerUnit = 5.0f;

// Everything is drawn in the colour of the patch name; inverted text is
// dark, like the display itself, on a bar of that colour.
const TJBox_Color kCyan = JBOX_COLOR_O(0, 179, 179);
const TJBox_Color kDark = JBOX_COLOR_O(10, 22, 24);

// Text and lines keep this far from the left and right edges.
const TJBox_Float32 kLeft = 20.0f;
const TJBox_Float32 kRight = 604.0f;

// A row of text is 50 pixels high, as the patch name's is; the host centres
// the text in it, which puts the top of a capital letter 5 pixels below the
// top of the row.
const TJBox_Float32 kRowHeight = 50.0f;
const TJBox_Float32 kTrackRowTop = 15.0f;
const TJBox_Float32 kClipRowTop = 61.0f;
const TJBox_Float32 kModeRowTop = 119.0f;
// The track name and the clip name start at the same place, right of
// "Track:" and "Clip:".
const TJBox_Float32 kNameLeft = 167.0f;

// The lines above and below the mode line.
const TJBox_Float32 kLineHeight = 2.0f;
const TJBox_Float32 kUpperLineTop = 112.0f;
const TJBox_Float32 kLowerLineTop = 171.0f;

// Each CV output has a block of three rows of text: its name and CV mode,
// then the headings and the values of its value table. The tops given here
// are those of the capital letters, which an inverted bar starts at.
const TJBox_Float32 kFirstBlockTop = 184.0f;
const TJBox_Float32 kBlockPitch = 215.0f;
const TJBox_Float32 kHeadingTop = 46.0f;
const TJBox_Float32 kValueTop = 92.0f;
const TJBox_Float32 kCapitalOffset = 5.0f;
const TJBox_Float32 kBarHeight = 36.0f;
// The five columns of a value table; headings start at the left of a
// column, values end at its right.
const TJBox_Float32 kColumnPitch = 118.0f;
const TJBox_Float32 kColumnWidth = 112.0f;

// The output graph lies below the value table. It is made of coarse pixels of
// five by five pixels of the panel, which the font's are too, and sits on
// that grid, so that no pixel is drawn blurred.
const TJBox_Float32 kGraphTop = 141.0f;
const TJBox_Float32 kGraphPixel = 5.0f;

// A name is cut off after this many bytes; the display has room for far fewer
// characters.
const TJBox_UInt32 kMaxNameLength = 96;

TJBox_Rect Rect(TJBox_Float32 iLeft, TJBox_Float32 iTop, TJBox_Float32 iRight, TJBox_Float32 iBottom) {
	TJBox_Rect rect;
	rect.fLeft = iLeft / kPixelsPerUnit;
	rect.fTop = iTop / kPixelsPerUnit;
	rect.fRight = iRight / kPixelsPerUnit;
	rect.fBottom = iBottom / kPixelsPerUnit;
	return rect;
}

// One row of text between iLeft and iRight; text that is too long is cut
// off there.
void DrawText(const char* iText, TJBox_Font iFont, TJBox_Color iColour, TJBox_HAlign iAlign, TJBox_Float32 iLeft, TJBox_Float32 iRight, TJBox_Float32 iTop) {
	const TJBox_Rect rect = Rect(iLeft, iTop, iRight, iTop + kRowHeight);
	TJBox_ClipRegion region;
	region.fRectangles = &rect;
	region.fRectangleCount = 1;
	region.fPaths = nullptr;
	region.fPathCount = 0;
	JBox_SetClip(&region, nullptr, true);

	TJBox_Text text;
	text.fRect = rect;
	text.fHAlign = iAlign;
	text.fText = iText;
	text.fFont = iFont;
	JBox_DrawText(&text, iColour, nullptr);

	JBox_SetClip(nullptr, nullptr, true);
}

// One row of text, from iLeft to the right edge.
void DrawRowText(const char* iText, TJBox_Font iFont, TJBox_Float32 iLeft, TJBox_Float32 iTop) {
	DrawText(iText, iFont, kCyan, kJBox_HAlign_Left, iLeft, kRight, iTop);
}

void FillRect(TJBox_Float32 iLeft, TJBox_Float32 iTop, TJBox_Float32 iRight, TJBox_Float32 iBottom) {
	const TJBox_Rect rect = Rect(iLeft, iTop, iRight, iBottom);
	TJBox_PathStyle fill;
	fill.fThickness = 0.0f;
	fill.fCap = kJBox_Cap_Butt;
	fill.fJoin = kJBox_Join_Miter;
	JBox_DrawRect(&rect, kCyan, &fill, nullptr);
}

void DrawLine(TJBox_Float32 iTop) {
	FillRect(kLeft, iTop, kRight, iTop + kLineHeight);
}

// Copies a name into oName, which must have room for kMaxNameLength + 1
// characters. Returns false for an empty name, which stands for the default.
bool LoadName(const TJBox_Value& iValue, char oName[]) {
	TJBox_UInt32 length = JBox_GetStringLength(iValue);
	if (length == 0) {
		return false;
	}
	if (length > kMaxNameLength) {
		length = kMaxNameLength;
	}
	JBox_GetSubstring(iValue, 0, length, oName);
	// Names are UTF-8: do not leave part of a character at the end.
	if (length == kMaxNameLength) {
		while (length > 0 && (static_cast<unsigned char>(oName[length - 1]) & 0xC0) == 0x80) {
			--length;
		}
		if (length > 0 && (static_cast<unsigned char>(oName[length - 1]) & 0x80) != 0) {
			--length;
		}
		oName[length] = '\0';
	}
	return true;
}

// What the realtime code has to say; every column blank and every graph
// empty until it has said anything.
CBigDisplay::TState LoadState(const TJBox_Value& iValue) {
	CBigDisplay::TState state;
	char bytes[CBigDisplay::kStateSize + 1];
	TJBox_UInt32 length = JBox_GetStringLength(iValue);
	if (length != static_cast<TJBox_UInt32>(CBigDisplay::kStateSize)) {
		length = 0;
	}
	JBox_GetSubstring(iValue, 0, length, bytes);
	CBigDisplay::Decode(reinterpret_cast<const unsigned char*>(bytes), static_cast<int>(length), state);
	return state;
}

// An output graph, with its top at iTop: the line for 0, and above it, or in
// bipolar mode above or below it, a bar for each run of columns of one level.
void DrawGraph(const unsigned char iLevels[], int iCVMode, TJBox_Float32 iTop) {
	const bool bipolar = iCVMode == CLauncher::kCVModeBipolar;
	// The row of the line for 0, counted from the top.
	const int zeroRow = bipolar ? CLauncher::kGraphRowCount - 1 - CLauncher::kGraphBipolarZero : CLauncher::kGraphRowCount - 1;
	const TJBox_Float32 zeroTop = iTop + kGraphPixel * zeroRow;
	const TJBox_Float32 zeroBottom = zeroTop + kGraphPixel;
	FillRect(kLeft, zeroTop, kLeft + kGraphPixel * CLauncher::kGraphColumnCount, zeroBottom);

	int runStart = 0;
	for (int column = 1; column <= CLauncher::kGraphColumnCount; ++column) {
		if (column < CLauncher::kGraphColumnCount && iLevels[column] == iLevels[runStart]) {
			continue;
		}
		const unsigned char level = iLevels[runStart];
		if (level != CLauncher::kGraphEmpty) {
			const int pixels = bipolar ? static_cast<int>(level) - CLauncher::kGraphBipolarZero : static_cast<int>(level);
			const TJBox_Float32 left = kLeft + kGraphPixel * runStart;
			const TJBox_Float32 right = kLeft + kGraphPixel * column;
			if (pixels > 0) {
				FillRect(left, zeroTop - kGraphPixel * pixels, right, zeroTop);
			}
			else if (pixels < 0) {
				FillRect(left, zeroBottom, right, zeroBottom - kGraphPixel * pixels);
			}
		}
		runStart = column;
	}
}

// The block of one CV output: its name and CV mode, and its value table.
void DrawBlock(const char* iOutputName, const CLauncher::TValueTable& iTable, TJBox_Float32 iTop) {
	char title[kMaxNameLength + 16];
	CBigDisplay::FormatOutputTitle(iOutputName, iTable.fCVMode, static_cast<int>(kRight - kLeft), title, sizeof(title));
	DrawRowText(title, kJBox_Font_SmallLCD, kLeft, iTop - kCapitalOffset);

	for (int column = 0; column < CLauncher::kTableColumnCount; ++column) {
		const CLauncher::TTableColumn& tableColumn = iTable.fColumns[column];
		if (!tableColumn.fPresent) {
			continue;
		}
		const TJBox_Float32 left = kLeft + kColumnPitch * column;
		const TJBox_Float32 right = left + kColumnWidth;
		const TJBox_Float32 headingTop = iTop + kHeadingTop;
		const TJBox_Float32 valueTop = iTop + kValueTop;
		if (tableColumn.fInverted) {
			FillRect(left, headingTop, right, headingTop + kBarHeight);
			FillRect(left, valueTop, right, valueTop + kBarHeight);
		}
		const TJBox_Color colour = tableColumn.fInverted ? kDark : kCyan;
		char heading[CBigDisplay::kHeadingSize];
		CBigDisplay::FormatHeading(column, iTable.fCVMode, tableColumn.fMark, heading);
		DrawText(heading, kJBox_Font_SmallLCD, colour, kJBox_HAlign_Left, left, right, headingTop - kCapitalOffset);
		char value[CBigDisplay::kValueSize];
		CBigDisplay::FormatValue(iTable.fCVMode, tableColumn.fValue, value);
		DrawText(value, kJBox_Font_SmallLCD, colour, kJBox_HAlign_Right, left, right, valueTop - kCapitalOffset);
	}
}

} // namespace

void DrawBigDisplay(const TJBox_DisplayArgs& iArgs) {
	if (iArgs.fDisplayInfo.fParamCount < kParamCount) {
		return;
	}
	const int setup = static_cast<int>(JBox_GetNumber(iArgs.fParams[kParam_Setup]));
	// 0 for none, otherwise the button index plus one.
	const int shownButton = static_cast<int>(JBox_GetNumber(iArgs.fParams[kParam_ShownClip])) - 1;

	DrawRowText("Track:", kJBox_Font_SmallLCD, kLeft, kTrackRowTop);
	DrawRowText("Clip:", kJBox_Font_SmallLCD, kLeft, kClipRowTop);
	if (shownButton >= 0 && shownButton < CLauncher::kButtonCount) {
		const int track = shownButton / CLauncher::kRowCount;
		const int row = shownButton % CLauncher::kRowCount + 1;
		char name[kMaxNameLength + 1];
		if (!LoadName(iArgs.fParams[kParam_FirstTrackName + track], name)) {
			CBigDisplay::DefaultTrackName(track, name);
		}
		DrawRowText(name, kJBox_Font_BoldLCD, kNameLeft, kTrackRowTop);
		if (!LoadName(iArgs.fParams[kParam_FirstClipName + shownButton], name)) {
			CBigDisplay::DefaultClipName(row, name);
		}
		DrawRowText(name, kJBox_Font_BoldLCD, kNameLeft, kClipRowTop);
	}

	DrawLine(kUpperLineTop);
	DrawRowText(CBigDisplay::ModeLine(setup), kJBox_Font_SmallLCD, kLeft, kModeRowTop);
	DrawLine(kLowerLineTop);

	// With no shown clip, the blocks are those of the first track.
	const int blockTrack = shownButton >= 0 && shownButton < CLauncher::kButtonCount ? shownButton / CLauncher::kRowCount : 0;
	const CBigDisplay::TState state = LoadState(iArgs.fParams[kParam_State]);
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		char name[kMaxNameLength + 1];
		if (!LoadName(iArgs.fParams[kParam_FirstOutputName + blockTrack * CLauncher::kOutputCount + output], name)) {
			CBigDisplay::DefaultOutputName(blockTrack, output, name);
		}
		const TJBox_Float32 blockTop = kFirstBlockTop + kBlockPitch * output;
		DrawBlock(name, state.fTables[output], blockTop);
		DrawGraph(state.fGraphs[output], state.fTables[output].fCVMode, blockTop + kGraphTop);
	}
}
