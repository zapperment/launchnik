#!/bin/sh
# Builds GUI2D/Scene_Button_9frames.png, the film strip of the scene buttons,
# from the nine-frame image in RE2D_Launchnik_Graphics.
#
# Frame 0 is the unlit button. Red, green, yellow and white follow in that
# order, each as a dim frame and then a vibrant one.
#
# The source is 126x1211 pixels with its frames 135 pixels apart. It is padded
# to frames of 130x135, a size RE2DRender accepts and the height of a launch
# button, with the button moved so that it sits level with the launch buttons.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

SOURCE=RE2D_Launchnik_Graphics/Launchnik_Scene_Button_9frames.png
TARGET=GUI2D/Scene_Button_9frames.png
FRAME=130x135

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

magick "$SOURCE" -background none -gravity northwest -splice 2x1 -extent 130x1215 +repage "$WORK/padded.png"

# In the source the unlit frame comes first, then vibrant and dim for red,
# green, yellow and white.
index=0
for source_frame in 0 2 1 4 3 6 5 8 7; do
	magick "$WORK/padded.png" -crop "$FRAME+0+$((source_frame * 135))" +repage "$WORK/$index.png"
	index=$((index + 1))
done

magick "$WORK"/[0-8].png -background none -append +repage "PNG32:$TARGET"
echo "Wrote $TARGET"
