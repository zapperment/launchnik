#!/bin/sh
# Builds GUI2D/Function_Button_7frames.png, the film strip of the function
# buttons: unlit, then dim and vibrant for red (stop), yellow (mute) and blue
# (solo).
#
# Red comes from the stock button image, yellow and blue from the images in
# RE2D_Launchnik_Graphics; each has two frames, a dark one and a bright one,
# which serve as dim and vibrant. The unlit frame is the dark red one with most
# of its colour and brightness taken away.
#
# The stock image has frames of 70x75. The other two are 69x147, with frames
# of 74 and 73 pixels; each frame is padded to 70x75, a size RE2DRender
# accepts.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

STOCK=../../../RE2D_Stock_Graphics_1_1/Buttons/Button_21_2frames.png
YELLOW=RE2D_Launchnik_Graphics/Button_21_yellow_2frames.png
BLUE=RE2D_Launchnik_Graphics/Button_21_blue_2frames.png
TARGET=GUI2D/Function_Button_7frames.png

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

magick "$STOCK" -crop 70x75+0+0 +repage "$WORK/1.png"
magick "$STOCK" -crop 70x75+0+75 +repage "$WORK/2.png"
magick "$WORK/1.png" -modulate 55,15 "$WORK/0.png"

# Splits a two-frame image of 69x147 into two frames of 70x75.
split() {
	source=$1
	first=$2
	magick "$source" -crop 69x74+0+0 +repage -background none -gravity northwest -extent 70x75 "$WORK/$first.png"
	magick "$source" -crop 69x73+0+74 +repage -background none -gravity northwest -extent 70x75 "$WORK/$((first + 1)).png"
}
split "$YELLOW" 3
split "$BLUE" 5

magick "$WORK"/[0-6].png -background none -append +repage "PNG32:$TARGET"
echo "Wrote $TARGET"
