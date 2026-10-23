#!/bin/sh
# Builds GUI2D/Scene_Button_11frames.png, the film strip of the scene buttons
# and the forward button, from the eleven-frame image in
# RE2D_Launchnik_Graphics, and GUI2D/Side_Button_11frames.png, that of the
# fill-in button and the function select buttons, from the eleven-frame image
# of a button without an arrow.
#
# Frame 0 is the unlit button. The colours follow, each as a dim frame and
# then a vibrant one: red, green, yellow, white and blue for the scene
# buttons; red, green, yellow, blue and white for the side buttons.
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

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# In the sources the unlit frame comes first, then vibrant and dim for each
# colour, so the two frames of each colour swap places.
build_strip() {
	source=$1
	target=$2
	frames=$3
	magick "$source" -background none -gravity northwest -splice 2x1 -extent "130x$((frames * 135))" +repage "$WORK/padded.png"

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

	# shellcheck disable=SC2086
	magick $list -background none -append +repage "PNG32:$target"
	rm "$WORK"/frame*.png
	echo "Wrote $target"
}

build_strip RE2D_Launchnik_Graphics/Launchnik_Scene_Button_11frames.png GUI2D/Scene_Button_11frames.png 11
build_strip RE2D_Launchnik_Graphics/Launchnik_Side_Button_11frames.png GUI2D/Side_Button_11frames.png 11
