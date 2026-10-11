#!/bin/sh
# Builds GUI2D/Stop_Button_3frames.png, the film strip of the stop buttons,
# from the stock button image: unlit, dim, vibrant.
#
# The stock image has two frames, a dark red and a bright one, which serve as
# dim and vibrant. The unlit frame is the dark one with most of its colour and
# brightness taken away.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

SOURCE=../../../RE2D_Stock_Graphics_1_1/Buttons/Button_21_2frames.png
TARGET=GUI2D/Stop_Button_3frames.png

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

magick "$SOURCE" -crop 70x75+0+0 +repage "$WORK/1.png"
magick "$SOURCE" -crop 70x75+0+75 +repage "$WORK/2.png"
magick "$WORK/1.png" -modulate 55,15 "$WORK/0.png"

magick "$WORK/0.png" "$WORK/1.png" "$WORK/2.png" -background none -append +repage "PNG32:$TARGET"
echo "Wrote $TARGET"
