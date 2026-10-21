#!/bin/sh
# Builds the film strips of the labels below the launch buttons and the scene
# buttons (see "Label" in CONTEXT.md):
#
# - GUI2D/Labels_Launch_5frames.png holds the labels of the launch buttons in
#   scenes 1 and 2, placed as on the panel: all unlit, then SET/CLEAR lit in
#   yellow, all lit in yellow, SET/CLEAR lit in red, all lit in red. Yellow is
#   for clip editing, red for fill-in editing. The labels of colours stay
#   unlit while there is no clip, or no fill-in, to set up. The labels of the
#   CV outputs in scenes 3 to 8 are made by tools/make-cv-labels.py.
# - GUI2D/Label_Scene<n>_3frames.png holds the label of scene button n:
#   unlit, then lit in white, then lit in green.
# - GUI2D/Label_Back_4frames.png holds the label of the back button: unlit,
#   white, green, yellow.
# - GUI2D/Label_Set_Fill_In_2frames.png holds the label of the set fill-in
#   button: unlit, red.
# - GUI2D/Caption_Fill_In_2frames.png holds the caption of the fill-in button:
#   white, red.
# - GUI2D/Caption_Mute_2frames.png, Caption_Solo_2frames.png and
#   Caption_Stop_2frames.png hold the captions of the function select buttons:
#   white, then yellow, blue and red.
# - GUI2D/Labels_Front.png holds the caption SETUP, which is always white, as
#   large as the panel: to the left of the setup button, its right edge 8
#   pixels from the button and 2 pixels above its vertical centre.
#
# The labels are set in Arial Bold, capitals 26 pixels high, in white or
# coloured. Each one is centred below its button, its top 8 pixels above the
# bottom of the button's image, where the button itself ends. The layout
# numbers must match GUI2D/device_2D.lua.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

FONT="/System/Library/Fonts/Supplemental/Arial Bold.ttf"
POINTSIZE=36

UNLIT="#4d4d4d"
WHITE="#d0d0d0"
GREEN="#1aac08"
YELLOW="#fafe07"
RED="#ff5e5e"
BLUE="#0598fe"

# Launch buttons are 205 pixels wide, 252 apart, with 235 pixels from one
# scene to the next. The strip starts 25 pixels left of the first track and
# level with the top of the labels of scene 1, and covers scenes 1 and 2; one
# label is 30 pixels high.
TRACK_PITCH=252
ROW_PITCH=235
BUTTON_WIDTH=205
LAUNCH_MARGIN=25
LAUNCH_WIDTH=2020
LAUNCH_SIZE=${LAUNCH_WIDTH}x265
SCENE_SIZE=250x30

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# Each label of the launch buttons: whether it is a control, which is lit in
# every launch button's clip editing, or a setting, which is lit only while the
# launch button holds a clip; its track and scene, both from 1; and its text,
# with "|" for a space.
LAUNCH_LABELS="
control 8 1 SET/CLEAR
setting 1 1 WHITE
setting 2 1 RED
setting 3 1 ORANGE
setting 4 1 YELLOW
setting 5 1 LIME
setting 6 1 GREEN
setting 7 1 TURQUOISE
setting 1 2 CYAN
setting 2 2 LIGHT|BLUE
setting 3 2 BLUE
setting 4 2 DARK|BLUE
setting 5 2 PURPLE
setting 6 2 FUCHSIA
setting 7 2 PINK
"

# Draws every label of the launch buttons, the controls in one colour and the
# settings in another. With north gravity the x offset of a text counts from
# the middle of the strip.
draw_launch_labels() {
	control_colour=$1
	setting_colour=$2
	target=$3
	set -- -size "$LAUNCH_SIZE" xc:none -font "$FONT" -pointsize "$POINTSIZE" -gravity north
	while read -r kind track scene text; do
		[ -n "$kind" ] || continue
		if [ "$kind" = control ]; then
			colour=$control_colour
		else
			colour=$setting_colour
		fi
		x=$(( LAUNCH_MARGIN + TRACK_PITCH * (track - 1) + BUTTON_WIDTH / 2 - LAUNCH_WIDTH / 2 ))
		y=$(( ROW_PITCH * (scene - 1) ))
		set -- "$@" -fill "$colour" -annotate "$(printf '%+d%+d' "$x" "$y")" "$(echo "$text" | tr '|' ' ')"
	done <<LABELS
$LAUNCH_LABELS
LABELS
	magick "$@" "PNG32:$target"
}

draw_launch_labels "$UNLIT" "$UNLIT" "$WORK/launch0.png"
draw_launch_labels "$YELLOW" "$UNLIT" "$WORK/launch1.png"
draw_launch_labels "$YELLOW" "$YELLOW" "$WORK/launch2.png"
draw_launch_labels "$RED" "$UNLIT" "$WORK/launch3.png"
draw_launch_labels "$RED" "$RED" "$WORK/launch4.png"
magick "$WORK"/launch[0-4].png -background none -append +repage "PNG32:GUI2D/Labels_Launch_5frames.png"
echo "Wrote GUI2D/Labels_Launch_5frames.png"

scene=1
for text in "16 BARS" "8 BARS" "4 BARS" "2 BARS" "1 BAR" "1/2 BAR" "1 BEAT" "IMMEDIATE"; do
	index=0
	for colour in "$UNLIT" "$WHITE" "$GREEN"; do
		magick -size "$SCENE_SIZE" xc:none -font "$FONT" -pointsize "$POINTSIZE" -fill "$colour" -gravity north -annotate +0+0 "$text" "PNG32:$WORK/scene$index.png"
		index=$((index + 1))
	done
	target=GUI2D/Label_Scene${scene}_3frames.png
	magick "$WORK/scene0.png" "$WORK/scene1.png" "$WORK/scene2.png" -background none -append +repage "PNG32:$target"
	echo "Wrote $target"
	scene=$((scene + 1))
done

# Draws one film strip of a text in several colours, one frame each.
draw_strip() {
	size=$1
	pointsize=$2
	text=$3
	target=$4
	shift 4
	index=0
	for colour in "$@"; do
		magick -size "$size" xc:none -font "$FONT" -pointsize "$pointsize" -fill "$colour" -gravity north -annotate +0+0 "$text" "PNG32:$WORK/strip$index.png"
		index=$((index + 1))
	done
	magick "$WORK"/strip[0-$((index - 1))].png -background none -append +repage "PNG32:$target"
	rm "$WORK"/strip*.png
	echo "Wrote $target"
}

draw_strip "$SCENE_SIZE" "$POINTSIZE" BACK GUI2D/Label_Back_4frames.png "$UNLIT" "$WHITE" "$GREEN" "$YELLOW"
draw_strip "$SCENE_SIZE" "$POINTSIZE" "SET FILL-IN" GUI2D/Label_Set_Fill_In_2frames.png "$UNLIT" "$RED"
draw_strip "$SCENE_SIZE" "$POINTSIZE" FILL-IN GUI2D/Caption_Fill_In_2frames.png "$WHITE" "$RED"
draw_strip "$SCENE_SIZE" "$POINTSIZE" MUTE GUI2D/Caption_Mute_2frames.png "$WHITE" "$YELLOW"
draw_strip "$SCENE_SIZE" "$POINTSIZE" SOLO GUI2D/Caption_Solo_2frames.png "$WHITE" "$BLUE"
draw_strip "$SCENE_SIZE" "$POINTSIZE" STOP GUI2D/Caption_Stop_2frames.png "$WHITE" "$RED"

# The setup button is 70 by 75 pixels; SETUP is 118 by 26 pixels when trimmed.
SETUP_LEFT=720
FUNCTION_TOP=2338
magick -size 300x80 xc:none -font "$FONT" -pointsize "$POINTSIZE" -fill "$WHITE" -annotate +10+50 SETUP -trim +repage "PNG32:$WORK/setup.png"
magick -size 3770x2760 xc:none "$WORK/setup.png" -geometry "+$((SETUP_LEFT - 8 - 118))+$((FUNCTION_TOP + (75 - 26) / 2 - 2))" -composite "PNG32:GUI2D/Labels_Front.png"
echo "Wrote GUI2D/Labels_Front.png"
