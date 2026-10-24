#!/bin/sh
# Builds GUI2D/Scene_Button_15frames.png, the film strip of the scene buttons
# and the forward button, from the thirteen-frame image in
# RE2D_Launchnik_Graphics, and GUI2D/Side_Button_15frames.png, that of the
# fill-in button, the clear button, the duplicate button and the function
# select buttons, from the thirteen-frame image of a button without an arrow.
#
# Frame 0 is the unlit button. The colours follow, each as a dim frame and
# then a vibrant one: red, green, yellow, white, blue, orange and fuchsia for
# the scene buttons; red, green, yellow, blue, white, orange and fuchsia for
# the side buttons. The sources have no orange, so its two frames are the red
# ones with their hue turned towards orange (see ORANGE); fuchsia comes last
# in the sources, after the five other colours.
#
# The sources are 126 pixels wide with their frames 135 pixels apart. They are
# padded to frames of 130x135, a size RE2DRender accepts and the height of a
# launch button, with the button moved so that it sits level with the launch
# buttons.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

FRAME=130x135
# Brightness, saturation and hue for -modulate that turn the sources' red into
# an orange close to that of the launch buttons.
ORANGE=100,160,111

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# In the sources the unlit frame comes first, then vibrant and dim for each
# colour, so the two frames of each colour swap places. The orange frames
# follow the first eleven, made from the red ones (frames 1 and 2 of the
# source), and the fuchsia ones come last, from the last two of the source.
build_strip() {
	source=$1
	target=$2
	frames=11
	magick "$source" -background none -gravity northwest -splice 2x1 -extent "130x$((13 * 135))" +repage "$WORK/padded.png"

	index=0
	list=""
	while [ "$index" -lt "$frames" ]; do
		if [ "$index" -eq 0 ]; then
			source_frame=0
		elif [ $((index % 2)) -eq 1 ]; then
			source_frame=$((index + 1))
		else
			source_frame=$((index - 1))
		fi
		magick "$WORK/padded.png" -crop "$FRAME+0+$((source_frame * 135))" +repage "$WORK/frame$index.png"
		list="$list $WORK/frame$index.png"
		index=$((index + 1))
	done
	magick "$WORK/frame1.png" -modulate "$ORANGE" "$WORK/frame$index.png"
	list="$list $WORK/frame$index.png"
	index=$((index + 1))
	magick "$WORK/frame2.png" -modulate "$ORANGE" "$WORK/frame$index.png"
	list="$list $WORK/frame$index.png"
	for source_frame in 12 11; do
		index=$((index + 1))
		magick "$WORK/padded.png" -crop "$FRAME+0+$((source_frame * 135))" +repage "$WORK/frame$index.png"
		list="$list $WORK/frame$index.png"
	done

	# shellcheck disable=SC2086
	magick $list -background none -append +repage "PNG32:$target"
	rm "$WORK"/frame*.png
	echo "Wrote $target"
}

build_strip RE2D_Launchnik_Graphics/Launchnik_Scene_Button_13frames.png GUI2D/Scene_Button_15frames.png
build_strip RE2D_Launchnik_Graphics/Launchnik_Side_Button_13frames.png GUI2D/Side_Button_15frames.png
