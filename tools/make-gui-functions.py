#!/usr/bin/env python3
"""Writes GUI2D/gui_functions.lua: the functions that decide whether a name,
or the default name that stands in while it is empty, is shown (see "Track
name", "Clip name" and "Scene name" in CONTEXT.md, and docs/adr/0007).

Track names and clip names are shown in launch mode, clear mode and clip
picking, and in clip editing, fill-in editing and landing editing only for the
launch button being edited and its track. A visibility function only gets the values of properties, not the
widget it belongs to, so each track name and clip name has functions of its
own, which this script writes out. Each is self-contained, without helpers
or library calls, since the host runs them on its own.

The setup state is 0 in launch mode, 1 in clip picking, 2 + i in clip
editing, 66 + i in fill-in editing and 130 + i in landing editing of launch
button i, counted from 0 down track 1 first (see CLauncher::PickedButton), and
194 in clear mode, so (setup - 2) % 64 is the launch button being edited from
2 to 193.

Run: tools/make-gui-functions.py
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TRACK_COUNT = 8
ROW_COUNT = 8

HEADER = """\
-- Written by tools/make-gui-functions.py; change that instead.
--
-- Whether a name, or the default name that stands in while it is empty, is
-- shown (see docs/adr/0007). On the front panel values[1] is the setup state
-- (0 = launch mode, 1 = clip picking, 2 + i clip editing, 66 + i fill-in
-- editing and 130 + i landing editing of launch button i, counted from 0 down
-- track 1 first, 194 = clear mode) and
-- values[2] the name; on the back panel values[1] is the name.

-- Track names on the back panel are always shown.
function is_name_shown(values)
	return values[1] ~= ""
end

function is_default_name_shown(values)
	return values[1] == ""
end

-- Scene names are shown in launch mode and clear mode only, since in clip
-- picking the scene buttons stand for lengths.
function is_name_shown_in_launch_mode(values)
	return (values[1] == 0 or values[1] == 194) and values[2] ~= ""
end

function is_default_name_shown_in_launch_mode(values)
	return (values[1] == 0 or values[1] == 194) and values[2] == ""
end
"""


def pair(name, condition):
	return (
		f"function is_{name}_shown(values) return ({condition}) and values[2] ~= \"\" end\n"
		f"function is_{name}_default_shown(values) return ({condition}) and values[2] == \"\" end\n"
	)


def main():
	parts = [HEADER, "\n-- Track names: also in the editing of a launch button of the track.\n"]
	for track in range(1, TRACK_COUNT + 1):
		first = ROW_COUNT * (track - 1)
		last = first + ROW_COUNT - 1
		condition = f"values[1] <= 1 or values[1] == 194 or ((values[1] - 2) % 64 >= {first} and (values[1] - 2) % 64 <= {last})"
		parts.append(pair(f"track{track}_name", condition))
	parts.append("\n-- Clip names: also in the editing of their launch button.\n")
	for track in range(1, TRACK_COUNT + 1):
		for row in range(1, ROW_COUNT + 1):
			index = ROW_COUNT * (track - 1) + row - 1
			condition = f"values[1] <= 1 or values[1] == 194 or (values[1] - 2) % 64 == {index}"
			parts.append(pair(f"track{track}_clip{row}_name", condition))
	target = ROOT / "GUI2D" / "gui_functions.lua"
	target.write_text("".join(parts))
	print(f"Wrote {target.relative_to(ROOT)}")


if __name__ == "__main__":
	main()
