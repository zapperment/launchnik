#include "BigDisplay.h"

namespace {

// Writes a word, a space and a number below 10; the Rack Extension build has
// no snprintf.
void WriteNumberedName(const char* iWord, int iNumber, char oName[]) {
	int length = 0;
	while (iWord[length] != '\0') {
		oName[length] = iWord[length];
		++length;
	}
	oName[length++] = ' ';
	oName[length++] = static_cast<char>('0' + iNumber);
	oName[length] = '\0';
}

// How far each character from the space to the tilde moves on in the host's
// small LCD font, in the font's own pixels, of which each is five pixels of
// the panel wide. Taken from ProCombiProg.otf in the host's Fonts folder.
const int kFirstCharacter = 32;
const int kLastCharacter = 126;
const int kSmallCells[kLastCharacter - kFirstCharacter + 1] = {
	2, 2, 4, 6, 5, 6, 6, 3, 4, 4, 6, 6, 3, 5, 3, 6,
	5, 4, 5, 5, 5, 5, 5, 5, 5, 5, 2, 3, 4, 6, 4, 5,
	6, 6, 6, 5, 6, 5, 5, 6, 6, 2, 4, 6, 5, 6, 6, 6,
	5, 6, 6, 5, 6, 6, 6, 6, 6, 6, 5, 4, 6, 4, 6, 6,
	3, 5, 5, 4, 5, 5, 4, 5, 5, 2, 4, 5, 2, 6, 5, 5,
	5, 5, 4, 5, 4, 5, 6, 6, 6, 5, 5, 4, 2, 4, 5,
};
const int kWidestCells = 6;
const int kPixelsPerCell = 5;

// Appends iText to oText at ioLength, keeping it shorter than iSize.
void Append(const char* iText, char oText[], int& ioLength, int iSize) {
	for (int index = 0; iText[index] != '\0' && ioLength < iSize - 1; ++index) {
		oText[ioLength++] = iText[index];
	}
	oText[ioLength] = '\0';
}

bool IsContinuationByte(char iByte) {
	return (static_cast<unsigned char>(iByte) & 0xC0) == 0x80;
}

const char* CVModeSuffix(int iCVMode) {
	if (iCVMode == CLauncher::kCVModePattern) {
		return " (pattern)";
	}
	return iCVMode == CLauncher::kCVModeBipolar ? " (bipolar)" : " (unipolar)";
}

// Values are kept as two bytes, moved up so that none is below 0.
const int kValueOffset = 1000;
const int kMaxValue = 1000;

} // namespace

const char* CBigDisplay::ModeLine(int iSetup) {
	if (CLauncher::IsClipEditing(iSetup)) {
		return "Setup step 2: EDIT CLIP";
	}
	if (CLauncher::IsFillInEditing(iSetup)) {
		return "Setup step 3: SET FILL-IN";
	}
	if (CLauncher::IsLandingEditing(iSetup)) {
		return "Setup step 4: SET LANDING";
	}
	if (CLauncher::IsIntroEditing(iSetup)) {
		return "Setup step 5: SET INTRO";
	}
	if (CLauncher::IsOutroEditing(iSetup)) {
		return "Setup step 6: SET OUTRO";
	}
	if (iSetup == CLauncher::kSetupClipPicking) {
		return "Setup step 1: PICK CLIP";
	}
	if (iSetup == CLauncher::kSetupClearMode) {
		return "Clear: PICK CLIP";
	}
	if (iSetup == CLauncher::kSetupSourcePicking) {
		return "Duplicate: PICK CLIP";
	}
	if (CLauncher::IsDuplicateMode(iSetup)) {
		return "Duplicate: PICK DEST.";
	}
	return "LAUNCH MODE";
}

void CBigDisplay::DefaultTrackName(int iTrack, char oName[]) {
	WriteNumberedName("Track", iTrack + 1, oName);
}

void CBigDisplay::DefaultClipName(int iRow, char oName[]) {
	WriteNumberedName("Clip", iRow, oName);
}

void CBigDisplay::DefaultOutputName(int iTrack, int iOutput, char oName[]) {
	int length = 0;
	Append("CV Out ", oName, length, kDefaultNameSize);
	oName[length++] = static_cast<char>('1' + iTrack);
	oName[length++] = static_cast<char>('A' + iOutput);
	oName[length] = '\0';
}

void CBigDisplay::FormatHeading(int iColumn, int iCVMode, int iMark, char oText[]) {
	static const char* const kHeadings[CLauncher::kTableColumnCount] = { "Clip", "Fill", "Lnd", "Int", "Out" };
	int length = 0;
	oText[0] = '\0';
	if (iColumn < 0 || iColumn >= CLauncher::kTableColumnCount) {
		return;
	}
	Append(kHeadings[iColumn], oText, length, kHeadingSize);
	char mark = '\0';
	if (iCVMode == CLauncher::kCVModePattern) {
		mark = iMark != 0 ? '1' : '\0';
	}
	else if (iMark > CLauncher::kTransitionOff && iMark < CLauncher::kTransitionCount) {
		mark = static_cast<char>('A' + iMark - CLauncher::kTransitionA);
	}
	if (mark != '\0') {
		oText[length++] = ' ';
		oText[length++] = mark;
		oText[length] = '\0';
	}
}

void CBigDisplay::FormatValue(int iCVMode, int iValue, char oText[]) {
	int length = 0;
	int whole = iValue;
	int tenth = 0;
	if (iCVMode != CLauncher::kCVModePattern) {
		if (iValue < 0) {
			oText[length++] = '-';
		}
		else if (iValue > 0 && iCVMode == CLauncher::kCVModeBipolar) {
			oText[length++] = '+';
		}
		const int tenths = iValue < 0 ? -iValue : iValue;
		whole = tenths / 10;
		tenth = tenths % 10;
	}
	if (whole < 0) {
		whole = 0;
	}
	char digits[8];
	int digitCount = 0;
	do {
		digits[digitCount++] = static_cast<char>('0' + whole % 10);
		whole /= 10;
	} while (whole > 0 && digitCount < 4);
	while (digitCount > 0) {
		oText[length++] = digits[--digitCount];
	}
	if (tenth != 0) {
		oText[length++] = '.';
		oText[length++] = static_cast<char>('0' + tenth);
	}
	oText[length] = '\0';
}

int CBigDisplay::SmallTextWidth(const char* iText) {
	int cells = 0;
	for (int index = 0; iText[index] != '\0'; ++index) {
		const char character = iText[index];
		// The bytes that follow the first of a character outside ASCII.
		if (IsContinuationByte(character)) {
			continue;
		}
		const int code = static_cast<unsigned char>(character);
		cells += code >= kFirstCharacter && code <= kLastCharacter ? kSmallCells[code - kFirstCharacter] : kWidestCells;
	}
	return cells * kPixelsPerCell;
}

void CBigDisplay::FormatOutputTitle(const char* iName, int iCVMode, int iMaxWidth, char oText[], int iSize) {
	const char* const suffix = CVModeSuffix(iCVMode);
	int length = 0;
	Append(iName, oText, length, iSize);
	// Leave room for the CV mode, and take whole characters off the name
	// until both fit.
	int suffixLength = 0;
	while (suffix[suffixLength] != '\0') {
		++suffixLength;
	}
	if (length > iSize - 1 - suffixLength) {
		length = iSize - 1 - suffixLength;
		if (length < 0) {
			length = 0;
		}
		while (length > 0 && IsContinuationByte(oText[length])) {
			--length;
		}
		oText[length] = '\0';
	}
	const int suffixWidth = SmallTextWidth(suffix);
	while (length > 0 && SmallTextWidth(oText) + suffixWidth > iMaxWidth) {
		--length;
		while (length > 0 && IsContinuationByte(oText[length])) {
			--length;
		}
		oText[length] = '\0';
	}
	Append(suffix, oText, length, iSize);
}

void CBigDisplay::Encode(const TState& iState, unsigned char oBytes[]) {
	int index = 0;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		const CLauncher::TValueTable& table = iState.fTables[output];
		oBytes[index++] = static_cast<unsigned char>(table.fCVMode);
		for (int column = 0; column < CLauncher::kTableColumnCount; ++column) {
			const CLauncher::TTableColumn& tableColumn = table.fColumns[column];
			const int value = tableColumn.fValue + kValueOffset;
			oBytes[index++] = static_cast<unsigned char>((tableColumn.fPresent ? 1 : 0) + (tableColumn.fInverted ? 2 : 0));
			oBytes[index++] = static_cast<unsigned char>(tableColumn.fMark);
			oBytes[index++] = static_cast<unsigned char>(value / 256);
			oBytes[index++] = static_cast<unsigned char>(value % 256);
		}
	}
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		for (int column = 0; column < CLauncher::kGraphColumnCount; ++column) {
			oBytes[index++] = iState.fGraphs[output][column];
		}
	}
}

bool CBigDisplay::Decode(const unsigned char iBytes[], int iSize, TState& oState) {
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		CLauncher::TValueTable& table = oState.fTables[output];
		table.fCVMode = CLauncher::DefaultCVMode(output);
		for (int column = 0; column < CLauncher::kTableColumnCount; ++column) {
			table.fColumns[column].fPresent = false;
			table.fColumns[column].fInverted = false;
			table.fColumns[column].fMark = 0;
			table.fColumns[column].fValue = 0;
		}
		for (int column = 0; column < CLauncher::kGraphColumnCount; ++column) {
			oState.fGraphs[output][column] = CLauncher::kGraphEmpty;
		}
	}
	if (iSize != kStateSize) {
		return false;
	}
	int index = 0;
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		CLauncher::TValueTable& table = oState.fTables[output];
		const int cvMode = iBytes[index++];
		if (cvMode < CLauncher::kCVModeCount) {
			table.fCVMode = cvMode;
		}
		for (int column = 0; column < CLauncher::kTableColumnCount; ++column) {
			CLauncher::TTableColumn& tableColumn = table.fColumns[column];
			const int flags = iBytes[index++];
			tableColumn.fPresent = (flags & 1) != 0;
			tableColumn.fInverted = (flags & 2) != 0;
			tableColumn.fMark = iBytes[index++];
			const int high = iBytes[index++];
			const int low = iBytes[index++];
			int value = high * 256 + low - kValueOffset;
			if (value > kMaxValue) {
				value = kMaxValue;
			}
			tableColumn.fValue = value;
		}
	}
	for (int output = 0; output < CLauncher::kOutputCount; ++output) {
		for (int column = 0; column < CLauncher::kGraphColumnCount; ++column) {
			const unsigned char level = iBytes[index++];
			oState.fGraphs[output][column] = level < CLauncher::kGraphRowCount ? level : CLauncher::kGraphEmpty;
		}
	}
	return true;
}
