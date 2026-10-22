#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.10"
# dependencies = ["pillow"]
# ///
"""Builds the labels of the CV outputs (see "Label" and "CV output" in
CONTEXT.md):

- GUI2D/Labels_Output_23frames.png holds the labels below the launch buttons
  of one scene from 3 to 8, each of which stands for a CV output in clip
  editing and fill-in editing. Every launch button has two lines: on top the
  unipolar value on the left and the bipolar value on the right, or in the
  last track the transitions OFF A B C D; below it the pattern. The frames
  are: all unlit, then in yellow and in red each the patterns lit, the
  unipolar values lit with each of the five transitions, and the bipolar
  values lit with each of the five transitions. The six scenes share it.
- GUI2D/Label_Transitions_3frames.png holds the labels of the transition
  diagram to the right of the scene buttons: TRANSITION and the letters A to
  D, unlit, yellow or red.
- GUI2D/Labels_Back.png holds every caption of the back panel, as large as
  the panel: CV OUT and, left of the first track, the names of the CV modes with lines to the
  positions of the switch.

It also copies the transition diagram and the CV mode switch into GUI2D.
Front labels are set in Arial Bold like those of tools/make-label-strips.sh,
back ones in Arial. The layout numbers must match GUI2D/device_2D.lua.

Needs uv. Run: tools/make-cv-labels.py
"""
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
GUI = ROOT / "GUI2D"
SOURCES = ROOT / "RE2D_Launchnik_Graphics"
STOCK = ROOT.parent.parent.parent / "RE2D_Stock_Graphics_1_1"

BOLD = "/System/Library/Fonts/Supplemental/Arial Bold.ttf"
REGULAR = "/System/Library/Fonts/Supplemental/Arial.ttf"

UNLIT = "#4d4d4d"
YELLOW = "#fafe07"
RED = "#ff5e5e"
BACK_TEXT = "#e0e0e0"

# The launch buttons, as in tools/make-label-strips.sh.
TRACK_PITCH = 283
BUTTON_WIDTH = 205
LAUNCH_MARGIN = 25
LAUNCH_WIDTH = 2240

# One frame of the labels of a scene: the values on top, the pattern 32
# pixels below them.
OUTPUT_FRAME_HEIGHT = 75
PATTERN_LINE_TOP = 32
VALUE_SIZE = 32
PATTERN_SIZE = 36

UNIPOLAR = ["0", "16.6", "33.3", "50", "66.6", "83.3", "100"]
BIPOLAR = ["-100", "-66.6", "-33.3", "0", "+33.3", "+66.6", "+100"]
TRANSITIONS = ["OFF", "A", "B", "C", "D"]

# The transition diagram's labels: where the strip sits on the front panel,
# its size, and where TRANSITION and the letters go on the panel. The diagram
# itself is at DIAGRAM_LEFT, DIAGRAM_TOP, with the middle of its four boxes
# DIAGRAM_ROWS pixels below its top.
TRANSITION_LABELS_LEFT = 3327
TRANSITION_LABELS_TOP = 1233
TRANSITION_LABELS_SIZE = (250, 300)
DIAGRAM_TOP = 1293
DIAGRAM_ROWS = [28, 92, 156, 219]
HEADING_LEFT = 3343
HEADING_TOP = 1242
LETTER_CENTRE = 3356

# The back panel: the jacks of track 1 start 840 pixels from the left, 300
# apart, the jacks of output A 1065 pixels from the top, 240 apart; a jack is
# 80 pixels wide and 90 high. The switches stand 90 pixels left of their
# jacks.
PANEL_SIZE = (3770, 2760)
JACK_LEFT = 840
JACK_TOP = 1065
JACK_PITCH = 300
OUTPUT_PITCH = 240
JACK_WIDTH = 80
JACK_HEIGHT = 90
SWITCH_LEFT = JACK_LEFT - 90
TRACK_COUNT = 8
OUTPUT_LETTERS = "ABCDEF"


def font(path, size):
	return ImageFont.truetype(path, size)


def draw_text(draw, xy, text, fill, typeface, anchor):
	draw.text(xy, text, fill=fill, font=typeface, anchor=anchor)


def output_frame(top_colour, bottom_colour, lit):
	"""One frame of a scene's labels. lit is "patterns", ("unipolar", t) or
	("bipolar", t); everything else is unlit."""
	image = Image.new("RGBA", (LAUNCH_WIDTH, OUTPUT_FRAME_HEIGHT))
	draw = ImageDraw.Draw(image)
	values = font(BOLD, VALUE_SIZE)
	patterns = font(BOLD, PATTERN_SIZE)
	kind, transition = lit if isinstance(lit, tuple) else (lit, None)
	for track in range(8):
		left = LAUNCH_MARGIN + TRACK_PITCH * track
		centre = left + BUTTON_WIDTH // 2
		if track < 7:
			draw_text(draw, (left, 0), UNIPOLAR[track], top_colour if kind == "unipolar" else UNLIT, values, "la")
			draw_text(draw, (left + BUTTON_WIDTH, 0), BIPOLAR[track], top_colour if kind == "bipolar" else UNLIT, values, "ra")
		else:
			# OFF A B C D, centred as a whole, the chosen transition lit.
			space = draw.textlength(" ", font=values)
			widths = [draw.textlength(word, font=values) for word in TRANSITIONS]
			x = centre - (sum(widths) + space * (len(TRANSITIONS) - 1)) / 2
			for index, word in enumerate(TRANSITIONS):
				colour = top_colour if transition == index else UNLIT
				draw_text(draw, (x, 0), word, colour, values, "la")
				x += widths[index] + space
		draw_text(draw, (centre, PATTERN_LINE_TOP), f"PATTERN {track + 1}", bottom_colour if kind == "patterns" else UNLIT, patterns, "ma")
	return image


def make_output_labels():
	frames = [output_frame(UNLIT, UNLIT, None)]
	for colour in (YELLOW, RED):
		frames.append(output_frame(colour, colour, "patterns"))
		for kind in ("unipolar", "bipolar"):
			for transition in range(len(TRANSITIONS)):
				frames.append(output_frame(colour, colour, (kind, transition)))
	strip = Image.new("RGBA", (LAUNCH_WIDTH, OUTPUT_FRAME_HEIGHT * len(frames)))
	for index, frame in enumerate(frames):
		strip.paste(frame, (0, OUTPUT_FRAME_HEIGHT * index))
	target = GUI / f"Labels_Output_{len(frames)}frames.png"
	strip.save(target)
	print(f"Wrote {target.relative_to(ROOT)}")


def make_transition_labels():
	typeface = font(BOLD, PATTERN_SIZE)
	width, height = TRANSITION_LABELS_SIZE
	# HEADING_TOP is where the capitals of the heading begin.
	cap_top = typeface.getbbox("T", anchor="ls")[1]
	strip = Image.new("RGBA", (width, height * 3))
	draw = ImageDraw.Draw(strip)
	for index, colour in enumerate((UNLIT, YELLOW, RED)):
		top = height * index
		baseline = top + HEADING_TOP - TRANSITION_LABELS_TOP - cap_top
		draw_text(draw, (HEADING_LEFT - TRANSITION_LABELS_LEFT, baseline), "TRANSITION", colour, typeface, "ls")
		for letter, row in zip("ABCD", DIAGRAM_ROWS):
			centre = (LETTER_CENTRE - TRANSITION_LABELS_LEFT, top + DIAGRAM_TOP + row - TRANSITION_LABELS_TOP)
			draw_text(draw, centre, letter, colour, typeface, "mm")
	target = GUI / "Label_Transitions_3frames.png"
	strip.save(target)
	print(f"Wrote {target.relative_to(ROOT)}")


def make_back_labels():
	panel = Image.new("RGBA", PANEL_SIZE)
	draw = ImageDraw.Draw(panel)
	jack_centre = lambda track: JACK_LEFT + JACK_WIDTH // 2 + JACK_PITCH * track
	row_centre = lambda output: JACK_TOP + JACK_HEIGHT // 2 + OUTPUT_PITCH * output

	# CV OUT centred over the jacks. The track names below it, and the names
	# of the CV outputs, are text widgets (see GUI2D/device_2D.lua).
	heading = font(REGULAR, 68)
	middle = (jack_centre(0) + jack_centre(TRACK_COUNT - 1)) // 2
	draw_text(draw, (middle, JACK_TOP - 251 - heading.getbbox("C", anchor="la")[1]), "CV OUT", BACK_TEXT, heading, "ma")

	# Left of the first track, the names of the CV modes, each pointing with a
	# line to its position of the switch. The lines' image has a margin of 16
	# pixels; its lines end 95 pixels from its left edge, at the switch.
	lines = Image.open(SOURCES / "Polarity_switch_lines.png").convert("RGBA")
	modes = font(REGULAR, 33)
	line_rows = [(17, 16), (62, 44), (105, 16)]
	for output in range(len(OUTPUT_LETTERS)):
		left = SWITCH_LEFT - 95
		top = row_centre(output) - lines.height // 2
		panel.alpha_composite(lines, (left, top))
		for name, (row, start) in zip(("PATTERN", "UNIPOLAR", "BIPOLAR"), line_rows):
			draw_text(draw, (left + start - 8, top + row), name, BACK_TEXT, modes, "rm")

	target = GUI / "Labels_Back.png"
	panel.save(target)
	print(f"Wrote {target.relative_to(ROOT)}")


def copy_graphics():
	shutil.copy(SOURCES / "Transition_diagram_2frames.png", GUI / "Transition_Diagram_2frames.png")
	shutil.copy(STOCK / "Special" / "Fader_Bypass_3frames.png", GUI / "CV_Mode_Switch_3frames.png")
	print("Copied GUI2D/Transition_Diagram_2frames.png and GUI2D/CV_Mode_Switch_3frames.png")


if __name__ == "__main__":
	make_output_labels()
	make_transition_labels()
	make_back_labels()
	copy_graphics()
