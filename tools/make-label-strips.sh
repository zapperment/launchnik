#!/bin/sh
# Builds the film strips of the labels below the launch buttons and the scene
# buttons (see "Label" in CONTEXT.md):
#
# - GUI2D/Labels_Launch_3frames.png holds every label of the launch buttons,
#   placed as on the panel: all unlit, then SET/CLEAR lit in yellow,
#   then all lit in yellow. The labels of colours and patterns stay unlit
#   while the launch button being edited is empty.
# - GUI2D/Label_Scene<n>_3frames.png holds the label of scene button n:
#   unlit, then lit in white, then lit in green.
# - GUI2D/Label_Back_3frames.png holds the label of the back button, in the
#   same three frames as a scene button's.
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
magick "$WORK/launch0.png" "$WORK/launch1.png" "$WORK/launch2.png" -background none -append +repage "PNG32:GUI2D/Labels_Launch_3frames.png"
echo "Wrote GUI2D/Labels_Launch_3frames.png"

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

index=0
for colour in "$UNLIT" "$WHITE" "$GREEN"; do
	magick -size "$SCENE_SIZE" xc:none -font "$FONT" -pointsize "$POINTSIZE" -fill "$colour" -gravity north -annotate +0+0 BACK "PNG32:$WORK/back$index.png"
	index=$((index + 1))
done
magick "$WORK/back0.png" "$WORK/back1.png" "$WORK/back2.png" -background none -append +repage "PNG32:GUI2D/Label_Back_3frames.png"
echo "Wrote GUI2D/Label_Back_3frames.png"
