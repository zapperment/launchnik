#!/bin/sh
# Builds GUI2D/Logo_3frames.png, the film strip of the logo, from the three
# versions of the lettering in RE2D_Launchnik_Graphics, one for each mode
# colour: light grey (white) in launch mode, green in clip picking and yellow
# in clip editing. The red of fill-in editing is the yellow version recoloured.
#
# Each is trimmed to the lettering and scaled to 285x110, a size RE2DRender
# accepts and that fits the header of the panel.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

SOURCES=RE2D_Launchnik_Graphics
TARGET=GUI2D/Logo_4frames.png
RED="#ff2010"
FRAME=285x110

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

index=0
for version in bw green yellow; do
	magick "$SOURCES/Launchnik_Logo_$version.png" -trim +repage -resize "$FRAME" -background none -gravity center -extent "$FRAME" +repage "$WORK/$index.png"
	index=$((index + 1))
done

magick "$WORK/2.png" -fill "$RED" -colorize 100 "$WORK/3.png"

magick "$WORK"/[0-3].png -background none -append +repage "PNG32:$TARGET"
echo "Wrote $TARGET"
