#!/bin/sh
# Builds GUI2D/Launch_Button_29frames.png, the film strip of the launch
# buttons, from the fourteen three-frame images in Image_Sources.
#
# Frame 0 is the unlit button. Colour c (1..14, in the order of CONTEXT.md)
# follows as frame 2c - 1, dim, and frame 2c, vibrant.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

SOURCES=Image_Sources
TARGET=GUI2D/Launch_Button_29frames.png
FRAME=205x135
COLOURS="White Red Orange Yellow Lime Green Turquoise Cyan Light_Blue Blue Dark_Blue Purple Fuchsia Pink"

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# In the sources the unlit frame comes first, then vibrant, then dim. The
# unlit frame is the same in all of them.
magick "$SOURCES/Launchnik_LButton_White_3frames.png" -crop "$FRAME+0+0" +repage "$WORK/00.png"
index=1
for colour in $COLOURS; do
	source="$SOURCES/Launchnik_LButton_${colour}_3frames.png"
	magick "$source" -crop "$FRAME+0+270" +repage "$WORK/$(printf '%02d' $index).png"
	magick "$source" -crop "$FRAME+0+135" +repage "$WORK/$(printf '%02d' $((index + 1))).png"
	index=$((index + 2))
done

magick "$WORK"/*.png -background none -append +repage "PNG32:$TARGET"
echo "Wrote $TARGET"
