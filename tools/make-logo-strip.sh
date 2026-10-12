#!/bin/sh
# Builds GUI2D/Logo_2frames.png, the film strip of the logo, from the two
# versions of the lettering in RE2D_Launchnik_Graphics: light grey, shown in
# launch mode, and green, shown in setup mode.
#
# Each is trimmed to the lettering and scaled to 285x110, a size RE2DRender
# accepts and that fits the header of the panel.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

SOURCES=RE2D_Launchnik_Graphics
TARGET=GUI2D/Logo_2frames.png
FRAME=285x110

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

index=0
for version in bw green; do
	magick "$SOURCES/Launchnik_Logo_$version.png" -trim +repage -resize "$FRAME" -background none -gravity center -extent "$FRAME" +repage "$WORK/$index.png"
	index=$((index + 1))
done

magick "$WORK/0.png" "$WORK/1.png" -background none -append +repage "PNG32:$TARGET"
echo "Wrote $TARGET"
