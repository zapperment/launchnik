#!/usr/bin/env python3
"""Builds the film strips of the five kinds of button (see "Light" and
"Colour" in CONTEXT.md):

- GUI2D/Side_Button_29frames.png: the fill-in button, the clear button, the
  duplicate button and the function select buttons.
- GUI2D/Back_Button_29frames.png: the back button.
- GUI2D/Forward_Button_29frames.png: the scene buttons and the forward
  button, which look alike.
- GUI2D/Launch_Button_29frames.png: the launch buttons.
- GUI2D/Function_Button_29frames.png: the function buttons and the setup
  button.

Every strip has the same frames: 0 is unlit, then dim and vibrant for each of
the fourteen colours in their order, so colour c (0..13) is dim in frame
1 + 2c and vibrant in frame 2 + 2c.

Each source in Image_Sources has three frames, one above the other: unlit,
vibrant red and dim red. The other colours are made from the red frames with
ImageMagick's -modulate in HSB, by the ratios of their brightness and
saturation to the tint's and the difference of their hues; red, too, where
the tint is not quite red (see TINTS).

Needs ImageMagick. Run: tools/make-button-strips.py
"""
import colorsys
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
GUI = ROOT / "GUI2D"
SOURCES = ROOT / "Image_Sources"

BUTTONS = ["Side", "Back", "Forward", "Launch", "Function"]

# The colours in their order, each vibrant and dim.
COLOURS = [
	("White", "f3f3f3", "8c8c8c"),
	("Red", "fe0101", "b36061"),
	("Orange", "fe881c", "af5a0c"),
	("Yellow", "fafe07", "aca418"),
	("Lime", "acfe2a", "5a9c07"),
	("Green", "4bfd58", "379d47"),
	("Turquoise", "21ffc2", "259c79"),
	("Cyan", "1effff", "0c9090"),
	("Light blue", "18d7ff", "077c95"),
	("Blue", "0598fe", "0369af"),
	("Dark blue", "1f56da", "274aa0"),
	("Purple", "9635ff", "6f37a5"),
	("Fuchsia", "f225fe", "7339a0"),
	("Pink", "fd3aa9", "bf3d85"),
]
_, RED_VIBRANT, RED_DIM = COLOURS[1]

# What the red frames of a source are tinted with, vibrant and dim, if not
# the colour red: the vibrant function button is orange.
TINTS = {"Function": ("ff8000", RED_DIM)}

# The rows of the source frames.
UNLIT_FRAME = 0
VIBRANT_FRAME = 1
DIM_FRAME = 2


def hsb(hex_colour):
	return colorsys.rgb_to_hsv(*(int(hex_colour[i:i + 2], 16) / 255 for i in (0, 2, 4)))


def modulation(source_hex, target_hex):
	"""The arguments of -modulate that turn source_hex into target_hex."""
	source_hue, source_saturation, source_brightness = hsb(source_hex)
	target_hue, target_saturation, target_brightness = hsb(target_hex)
	# -modulate turns the hue by (h - 100) * 1.8 degrees.
	turn = (target_hue - source_hue + 0.5) % 1 - 0.5
	return "%.2f,%.2f,%.2f" % (
		100 * target_brightness / source_brightness,
		100 * target_saturation / source_saturation,
		100 + turn * 360 / 1.8,
	)


def make_strip(button, work):
	source = SOURCES / f"Launchnik_{button}_Button_3frames.png"
	width, height = (int(size) for size in subprocess.check_output(["magick", "identify", "-format", "%w %h", source], text=True).split())
	frame = f"{width}x{height // 3}"
	# RE2DRender wants sizes in multiples of 5 pixels, so each frame is
	# centred in the next larger one.
	padded = f"{-(-width // 5) * 5}x{-(-height // 3 // 5) * 5}"

	def crop(row):
		return [source, "-crop", f"{frame}+0+{row * height // 3}", "+repage", "-background", "none", "-gravity", "center", "-extent", padded]

	frames = []

	def add(path, *arguments):
		subprocess.run(["magick", *arguments, path], check=True)
		frames.append(path)

	tint_vibrant, tint_dim = TINTS.get(button, (RED_VIBRANT, RED_DIM))
	add(work / f"{button}_00.png", *crop(UNLIT_FRAME))
	for index, (_, vibrant, dim) in enumerate(COLOURS):
		for offset, row, red, target in ((1, DIM_FRAME, tint_dim, dim), (2, VIBRANT_FRAME, tint_vibrant, vibrant)):
			add(
				work / f"{button}_{2 * index + offset:02d}.png",
				*crop(row),
				"-define", "modulate:colorspace=HSB",
				"-modulate", modulation(red, target),
			)

	target = GUI / f"{button}_Button_{len(frames)}frames.png"
	subprocess.run(["magick", *frames, "-background", "none", "-append", "+repage", f"PNG32:{target}"], check=True)
	print(f"Wrote {target.relative_to(ROOT)}")


def main():
	with tempfile.TemporaryDirectory() as work:
		for button in BUTTONS:
			make_strip(button, Path(work))


if __name__ == "__main__":
	main()
