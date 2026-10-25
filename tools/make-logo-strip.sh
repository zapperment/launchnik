#!/bin/sh
# Builds GUI2D/Logo_14frames.png, the film strip of the logo, from the white
# lettering in Image_Sources: one frame for each of the fourteen colours, in
# their order, each the vibrant one of the palette (see "Colour" in
# CONTEXT.md). Red is the softer red of the captions, since the palette's is
# too harsh for text. The logo shows the mode colour.
#
# The lettering is trimmed and scaled to 285x110, a size RE2DRender accepts
# and that fits the header of the panel.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

SOURCE=Image_Sources/Launchnik_Logo_bw.png
TARGET=GUI2D/Logo_14frames.png
FRAME=285x110
COLOURS="#f3f3f3 #ff5e5e #fe881c #fafe07 #acfe2a #4bfd58 #21ffc2 #1effff #18d7ff #0598fe #1f56da #9635ff #f225fe #fd3aa9"

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

magick "$SOURCE" -trim +repage -resize "$FRAME" -background none -gravity center -extent "$FRAME" +repage "$WORK/white.png"
index=0
for colour in $COLOURS; do
	magick "$WORK/white.png" -fill "$colour" -colorize 100 "$WORK/$(printf '%02d' $index).png"
	index=$((index + 1))
done

magick "$WORK"/[0-9][0-9].png -background none -append +repage "PNG32:$TARGET"
echo "Wrote $TARGET"
