#!/bin/sh
# Builds the film strips of the labels below the launch buttons and the scene
# buttons (see "Label" in CONTEXT.md):
#
# - GUI2D/Labels_Launch_5frames.png holds every label of the launch buttons,
#   placed as on the panel: all unlit, then SET/CLEAR lit in yellow, all lit
#   in yellow, SET/CLEAR lit in red, all lit in red. Yellow is for clip
#   editing, red for fill-in editing. The labels of colours and patterns stay
#   unlit while there is no clip, or no fill-in, to set up.
# - GUI2D/Label_Scene<n>_3frames.png holds the label of scene button n:
#   unlit, then lit in white, then lit in green.
# - GUI2D/Label_Back_4frames.png holds the label of the back button: unlit,
#   white, green, yellow.
# - GUI2D/Label_Set_Fill_In_2frames.png holds the label of the set fill-in
#   button: unlit, red.
# - GUI2D/Caption_Fill_In_2frames.png holds the caption of the fill-in button:
#   white, red.
#
# The labels are set in Arial, capitals 26 pixels high, about three quarters
# of the size of SETUP and STOP, whose white they share. Each one is centred
# below its button with its top 20 pixels below the button. The layout numbers
# must match GUI2D/device_2D.lua.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

FONT=/System/Library/Fonts/Supplemental/Arial.ttf
POINTSIZE=36

UNLIT="#4d4d4d"
WHITE="#d0d0d0"
GREEN="#1aac08"
YELLOW="#fafe07"
RED="#ff5e5e"

# Launch buttons are 205 pixels wide, 252 apart, with 235 pixels from one
# scene to the next. The strip starts 25 pixels left of the first track and
# level with the top of the labels of scene 1; one label is 30 pixels high.
TRACK_PITCH=252
ROW_PITCH=235
BUTTON_WIDTH=205
LAUNCH_MARGIN=25
LAUNCH_WIDTH=2020
LAUNCH_SIZE=${LAUNCH_WIDTH}x1675
SCENE_SIZE=250x30

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# Each label of the launch buttons: whether it is a control, which is lit in
# every launch button's clip editing, or a setting, which is lit only while the
# launch button holds a clip; its track and scene, both from 1; and its text,
# with "|" for a space.
LAUNCH_LABELS="
control 8 1 SET/CLEAR
setting 1 2 WHITE
setting 2 2 RED
setting 3 2 ORANGE
setting 4 2 YELLOW
setting 5 2 LIME
setting 6 2 GREEN
setting 7 2 TURQUOISE
setting 1 3 CYAN
setting 2 3 LIGHT|BLUE
setting 3 3 BLUE
setting 4 3 DARK|BLUE
setting 5 3 PURPLE
setting 6 3 FUCHSIA
setting 7 3 PINK
setting 1 5 PATTERN|1
setting 2 5 PATTERN|2
setting 3 5 PATTERN|3
setting 4 5 PATTERN|4
setting 5 5 PATTERN|5
setting 6 5 PATTERN|6
setting 7 5 PATTERN|7
setting 8 5 PATTERN|8
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
