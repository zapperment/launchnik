#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.10"
# dependencies = ["pillow", "psd-tools>=1.9"]
# ///
"""Builds Photoshop templates of the four panels in ui-template/.

Reads the layout from GUI2D/device_2D.lua (through LuaJIT) and places every
image at its offset, at original size, one layer per node. Film strips show
their first frame. Bg and Labels come first, then the rest by name; click
areas are kept as hidden layers. Each panel is written as a layered .psd and
as a flattened .png.

Needs uv and LuaJIT. Run: tools/make-ui-template.py
"""
import os
import re
import subprocess
from pathlib import Path

from PIL import Image
from psd_tools import PSDImage
from psd_tools.constants import Compression
from psd_tools.api.layers import PixelLayer

ROOT = Path(__file__).resolve().parent.parent
GUI = ROOT / "GUI2D"
TARGET = ROOT / "ui-template"
PANELS = ["front", "back", "folded_front", "folded_back"]

DUMP = r"""
dofile(arg[1])
for _, panel in ipairs({ "front", "back", "folded_front", "folded_back" }) do
	for name, node in pairs(_G[panel]) do
		local image = node[1]
		if image and image.path then
			local offset = node.offset or { 0, 0 }
			print(table.concat({ panel, name, offset[1], offset[2], image.path, image.frames or 1 }, "\t"))
		end
	end
end
"""


def natural_key(name):
	return [int(part) if part.isdigit() else part for part in re.split(r"(\d+)", name)]


def layer_order(name):
	return (0 if name == "Bg" else 1 if name == "Labels" else 2, natural_key(name))


def read_layout():
	output = subprocess.run(
		["luajit", "-", str(GUI / "device_2D.lua")],
		input=DUMP, check=True, capture_output=True, text=True,
	).stdout
	layout = {panel: [] for panel in PANELS}
	for line in output.splitlines():
		panel, name, x, y, path, frames = line.split("\t")
		layout[panel].append((name, round(float(x)), round(float(y)), path, int(frames)))
	for nodes in layout.values():
		nodes.sort(key=lambda node: layer_order(node[0]))
	return layout


def first_frame(path, frames):
	image = Image.open(GUI / f"{path}.png").convert("RGBA")
	return image.crop((0, 0, image.width, image.height // frames))


def build(panel, nodes):
	background = first_frame(nodes[0][3], nodes[0][4])
	psd = PSDImage.new("RGBA", background.size)
	flat = Image.new("RGBA", background.size)
	for name, x, y, path, frames in nodes:
		image = first_frame(path, frames)
		layer = PixelLayer.frompil(image, psd, name, y, x, Compression.RLE)
		if name.startswith("click_"):
			layer.visible = False
		else:
			flat.alpha_composite(image, (max(x, 0), max(y, 0)), (max(-x, 0), max(-y, 0)))
		psd.append(layer)
	psd.save(TARGET / f"Launchnik_{panel}.psd")
	flat.save(TARGET / f"Launchnik_{panel}.png")
	print(f"Wrote ui-template/Launchnik_{panel}.psd and .png ({background.width}x{background.height})")


def main():
	os.makedirs(TARGET, exist_ok=True)
	for panel, nodes in read_layout().items():
		build(panel, nodes)


if __name__ == "__main__":
	main()
