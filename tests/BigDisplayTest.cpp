// Command-line tests for what the big display says. Run with tests/run.sh.

#include "../BigDisplay.h"
#include "../Launcher.h"

#include "Check.h"

#include <cstring>

namespace {

bool Same(const char* iText, const char* iExpected) {
	return std::strcmp(iText, iExpected) == 0;
}

void TestModeLineNamesTheWayOfWorking() {
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::kSetupLaunchMode), "LAUNCH MODE"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::kSetupClipPicking), "Setup step 1: PICK CLIP"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::SetupForClipEditing(0, 1)), "Setup step 2: EDIT CLIP"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::SetupForClipEditing(7, 8)), "Setup step 2: EDIT CLIP"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::SetupForFillInEditing(3, 4)), "Setup step 3: SET FILL-IN"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::SetupForLandingEditing(3, 4)), "Setup step 4: SET LANDING"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::SetupForIntroEditing(3, 4)), "Setup step 5: SET INTRO"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::SetupForOutroEditing(7, 8)), "Setup step 6: SET OUTRO"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::kSetupClearMode), "Clear: PICK CLIP"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::kSetupSourcePicking), "Duplicate: PICK CLIP"));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::SetupForClipDestinationPicking(0, 1)), "Duplicate: PICK DEST."));
	CHECK(Same(CBigDisplay::ModeLine(CLauncher::SetupForSceneDestinationPicking(8)), "Duplicate: PICK DEST."));
}

void TestDefaultNames() {
	char name[CBigDisplay::kDefaultNameSize];
	CBigDisplay::DefaultTrackName(0, name);
	CHECK(Same(name, "Track 1"));
	CBigDisplay::DefaultTrackName(7, name);
	CHECK(Same(name, "Track 8"));
	CBigDisplay::DefaultClipName(1, name);
	CHECK(Same(name, "Clip 1"));
	CBigDisplay::DefaultClipName(8, name);
	CHECK(Same(name, "Clip 8"));
}

void TestDefaultOutputNames() {
	char name[CBigDisplay::kDefaultNameSize];
	CBigDisplay::DefaultOutputName(0, 0, name);
	CHECK(Same(name, "CV Out 1A"));
	CBigDisplay::DefaultOutputName(7, 5, name);
	CHECK(Same(name, "CV Out 8F"));
}

bool HeadingIs(int iColumn, int iCVMode, int iMark, const char* iExpected) {
	char text[CBigDisplay::kHeadingSize];
	CBigDisplay::FormatHeading(iColumn, iCVMode, iMark, text);
	return Same(text, iExpected);
}

void TestHeadings() {
	CHECK(HeadingIs(0, CLauncher::kCVModeUnipolar, CLauncher::kTransitionOff, "Clip"));
	CHECK(HeadingIs(0, CLauncher::kCVModeUnipolar, CLauncher::kTransitionA, "Clip A"));
	CHECK(HeadingIs(1, CLauncher::kCVModeBipolar, CLauncher::kTransitionC, "Fill C"));
	CHECK(HeadingIs(2, CLauncher::kCVModeBipolar, CLauncher::kTransitionD, "Lnd D"));
	CHECK(HeadingIs(3, CLauncher::kCVModeUnipolar, CLauncher::kTransitionB, "Int B"));
	CHECK(HeadingIs(4, CLauncher::kCVModeUnipolar, CLauncher::kTransitionOff, "Out"));
	// In pattern mode a 1 says that one-bar pattern is on.
	CHECK(HeadingIs(1, CLauncher::kCVModePattern, 1, "Fill 1"));
	CHECK(HeadingIs(4, CLauncher::kCVModePattern, 0, "Out"));
}

bool ValueIs(int iCVMode, int iValue, const char* iExpected) {
	char text[CBigDisplay::kValueSize];
	CBigDisplay::FormatValue(iCVMode, iValue, text);
	return Same(text, iExpected);
}

void TestValues() {
	CHECK(ValueIs(CLauncher::kCVModePattern, 1, "1"));
	CHECK(ValueIs(CLauncher::kCVModePattern, 8, "8"));
	CHECK(ValueIs(CLauncher::kCVModePattern, 0, "0"));
	// No decimal for whole numbers, and no plus sign in unipolar mode.
	CHECK(ValueIs(CLauncher::kCVModeUnipolar, 0, "0"));
	CHECK(ValueIs(CLauncher::kCVModeUnipolar, 166, "16.6"));
	CHECK(ValueIs(CLauncher::kCVModeUnipolar, 500, "50"));
	CHECK(ValueIs(CLauncher::kCVModeUnipolar, 1000, "100"));
	CHECK(ValueIs(CLauncher::kCVModeUnipolar, 5, "0.5"));
	CHECK(ValueIs(CLauncher::kCVModeBipolar, 0, "0"));
	CHECK(ValueIs(CLauncher::kCVModeBipolar, 333, "+33.3"));
	CHECK(ValueIs(CLauncher::kCVModeBipolar, -666, "-66.6"));
	CHECK(ValueIs(CLauncher::kCVModeBipolar, 1000, "+100"));
	CHECK(ValueIs(CLauncher::kCVModeBipolar, -1000, "-100"));
	CHECK(ValueIs(CLauncher::kCVModeBipolar, -4, "-0.4"));
}

void TestSmallTextWidth() {
	CHECK(CBigDisplay::SmallTextWidth("") == 0);
	// As measured in the screen design: P and o are 25 pixels, l is 10.
	CHECK(CBigDisplay::SmallTextWidth("Pol") == 60);
	// A character outside ASCII counts once, as the widest.
	CHECK(CBigDisplay::SmallTextWidth("\xC3\xA4") == 30);
}

bool TitleIs(const char* iName, int iCVMode, int iMaxWidth, const char* iExpected) {
	char text[64];
	CBigDisplay::FormatOutputTitle(iName, iCVMode, iMaxWidth, text, sizeof(text));
	return Same(text, iExpected);
}

void TestOutputTitleKeepsTheCVModeInView() {
	CHECK(TitleIs("Echo Pan", CLauncher::kCVModeBipolar, 584, "Echo Pan (bipolar)"));
	CHECK(TitleIs("CV Out 1A", CLauncher::kCVModePattern, 584, "CV Out 1A (pattern)"));
	CHECK(TitleIs("Filter Cutoff Freq", CLauncher::kCVModeUnipolar, 584, "Filter Cutoff Freq (unipolar)"));
	// A name that is too long loses characters from its end.
	char text[64];
	CBigDisplay::FormatOutputTitle("Filter Cutoff Frequency Of The Second Oscillator", CLauncher::kCVModeUnipolar, 584, text, sizeof(text));
	CHECK(CBigDisplay::SmallTextWidth(text) <= 584);
	CHECK(std::strstr(text, "Filter Cutoff Freq") == text);
	CHECK(std::strstr(text, " (unipolar)") != nullptr);
	// Whole characters only.
	CHECK(TitleIs("\xC3\xA4\xC3\xA4\xC3\xA4", CLauncher::kCVModePattern, CBigDisplay::SmallTextWidth(" (pattern)") + 60, "\xC3\xA4\xC3\xA4 (pattern)"));
	// However long, it fits its buffer.
	char small[16];
	CBigDisplay::FormatOutputTitle("A very long name indeed", CLauncher::kCVModePattern, 10000, small, sizeof(small));
	CHECK(Same(small, "A ver (pattern)"));
}

void TestStateSurvivesEncoding() {
	CBigDisplay::TState state;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		state.fTables[output].fCVMode = output % CLauncher::kCVModeCount;
		for (int column = 0; column < CLauncher::kTableColumnCount; ++column) {
			CLauncher::TTableColumn& tableColumn = state.fTables[output].fColumns[column];
			tableColumn.fPresent = (output + column) % 2 == 0;
			tableColumn.fInverted = column == output % CLauncher::kTableColumnCount;
			tableColumn.fMark = column;
			tableColumn.fValue = -1000 + 68 * (output * CLauncher::kTableColumnCount + column);
		}
		for (int column = 0; column < CLauncher::kGraphColumnCount; ++column) {
			state.fGraphs[output][column] = column == output ? CLauncher::kGraphEmpty : static_cast<unsigned char>((column + output) % CLauncher::kGraphRowCount);
		}
	}
	unsigned char bytes[CBigDisplay::kStateSize];
	CBigDisplay::Encode(state, bytes);
	CBigDisplay::TState decoded;
	CHECK(CBigDisplay::Decode(bytes, CBigDisplay::kStateSize, decoded));
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		CHECK(decoded.fTables[output].fCVMode == state.fTables[output].fCVMode);
		for (int column = 0; column < CLauncher::kTableColumnCount; ++column) {
			const CLauncher::TTableColumn& expected = state.fTables[output].fColumns[column];
			const CLauncher::TTableColumn& actual = decoded.fTables[output].fColumns[column];
			CHECK(actual.fPresent == expected.fPresent && actual.fInverted == expected.fInverted);
			CHECK(actual.fMark == expected.fMark && actual.fValue == expected.fValue);
		}
		bool sameGraph = true;
		for (int column = 0; column < CLauncher::kGraphColumnCount; ++column) {
			sameGraph = sameGraph && decoded.fGraphs[output][column] == state.fGraphs[output][column];
		}
		CHECK(sameGraph);
	}

	// A property that has not been written yet is empty: every column blank.
	CHECK(!CBigDisplay::Decode(bytes, 0, decoded));
	CHECK(!decoded.fTables[0].fColumns[0].fPresent);
	CHECK(decoded.fTables[0].fCVMode == CLauncher::kCVModePattern);
	CHECK(decoded.fGraphs[0][5] == CLauncher::kGraphEmpty);
}

} // namespace

int main() {
	TestModeLineNamesTheWayOfWorking();
	TestDefaultNames();
	TestDefaultOutputNames();
	TestHeadings();
	TestValues();
	TestSmallTextWidth();
	TestOutputTitleKeepsTheCVModeInView();
	TestStateSurvivesEncoding();
	return ReportChecks("BigDisplay");
}
