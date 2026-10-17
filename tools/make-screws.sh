#!/bin/sh
# Splits the film strip RE2D_Launchnik_Graphics/Screw_17_4frames.png into one
# image per frame, GUI2D/Screw_17_<n>.png, so that each of the four screws on
# the front panel can show a different one: frame 1 top left, 2 top right,
# 3 bottom left, 4 bottom right. A static decoration always shows the first
# frame of a strip, hence the split.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

SOURCE=RE2D_Launchnik_Graphics/Screw_17_4frames.png

for frame in 1 2 3 4; do
	target=GUI2D/Screw_17_$frame.png
	magick "$SOURCE" -crop "95x95+0+$((95 * (frame - 1)))" +repage "PNG32:$target"
	echo "Wrote $target"
done
