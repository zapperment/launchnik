#!/bin/sh
# Builds GUI2D/Back_Button_9frames.png, the film strip of the back button,
# from the nine-frame image in RE2D_Launchnik_Graphics.
#
# The frames keep the order of the source: dark, then vibrant and dim for red,
# green, yellow and white. The panel uses dark, vibrant red (frame 1), vibrant
# green (frame 3), vibrant yellow (frame 5) and vibrant white (frame 7), see
# CLauncher::GetBackButtonFrame.
#
# The source is 126x1211 pixels with its frames 135 pixels apart. It is padded
# to frames of 130x135, a size RE2DRender accepts, in the same way as the
# scene buttons (see make-scene-button-strip.sh).
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

SOURCE=RE2D_Launchnik_Graphics/Launchnik_Back_Button_9frames.png
TARGET=GUI2D/Back_Button_9frames.png

magick "$SOURCE" -background none -gravity northwest -splice 2x1 -extent 130x1215 +repage "PNG32:$TARGET"
echo "Wrote $TARGET"
