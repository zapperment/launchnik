#!/bin/sh
# Builds the film strips of the labels below the launch buttons and the scene
# buttons, and beside the back button and the forward button (see "Label" in
# CONTEXT.md):
#
# - GUI2D/Labels_Launch_11frames.png holds the labels of the launch buttons in
#   scenes 1 and 2, placed as on the panel: all unlit, then SET/CLEAR lit and
#   all lit, in yellow, in orange, in blue, in pink and in turquoise. Yellow
#   is for clip editing, orange for fill-in editing, blue for landing
#   editing, pink for intro editing, turquoise for outro editing; 1 BAR PATT,
#   the one-bar pattern switch of a fill-in, a landing, an intro or an outro,
#   is lit only in the frames with all lit in orange, blue, pink and
#   turquoise. The labels of colours stay unlit while there is nothing to set
#   up. The labels of the CV outputs in scenes 3 to 8 are made by
#   tools/make-cv-labels.py.
# - GUI2D/Label_Scene<n>_3frames.png holds the label of scene button n:
#   unlit, then lit in white, then lit in green.
# - GUI2D/Label_Back_7frames.png holds the labels to the left of the back
#   button, one line for each step it goes back to, aligned on the right: all
#   unlit, then LAUNCH lit in white, PICK CLIP in green, EDIT CLIP in yellow,
#   SET FILL-IN in orange, SET LANDING in blue and SET INTRO in pink.
# - GUI2D/Label_Forward_5frames.png holds the labels to the right of the
#   forward button, one line for each step it goes forward to, aligned on the
#   left: all unlit, then SET FILL-IN lit in orange, SET LANDING in blue, SET
#   INTRO in pink and SET OUTRO in turquoise.
# - GUI2D/Caption_Fill_In_2frames.png holds the caption of the fill-in button:
#   white, orange.
# - GUI2D/Caption_Clear_3frames.png holds the caption of the clear button:
#   white, then dark grey like an unlit label, then red.
# - GUI2D/Caption_Duplicate_3frames.png holds the caption of the duplicate
#   button: white, then dark grey like an unlit label, then fuchsia.
# - GUI2D/Caption_Mute_2frames.png, Caption_Solo_2frames.png and
#   Caption_Stop_2frames.png hold the captions of the function select buttons:
#   white, then yellow, blue and red.
# - GUI2D/Caption_Setup_8frames.png holds the caption of the setup button,
#   which stands to its left, aligned on the right: white, then dark grey
#   like an unlit label, then green, yellow, orange, blue, pink and turquoise.
#
# The labels are set in Arial Bold, capitals 26 pixels high, in white or
# coloured. Each one is centred below its button, its top 8 pixels above the
# bottom of the button's image, where the button itself ends. The labels of
# the back button and the forward button are smaller, so that six lines fit
# beside a button, and their strips stand 20 pixels above and below it. The
# layout numbers must match GUI2D/device_2D.lua.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

FONT="/System/Library/Fonts/Supplemental/Arial Bold.ttf"
POINTSIZE=36

UNLIT="#4d4d4d"
WHITE="#d0d0d0"
GREEN="#1aac08"
# The green of clip picking, brighter than that of the scene labels
STEP_GREEN="#4bfd58"
YELLOW="#fafe07"
RED="#ff5e5e"
# The orange of the logo in fill-in editing
ORANGE="#fe881c"
BLUE="#0598fe"
# The fuchsia of the logo in duplicate mode
FUCHSIA="#f225fe"
# The pink of the logo in intro editing and the turquoise in outro editing
PINK="#fd3aa9"
TURQUOISE="#21ffc2"

# Launch buttons are 205 pixels wide, 283 apart, with 250 pixels from one
# scene to the next. The strip starts 25 pixels left of the first track and
# level with the top of the labels of scene 1, and covers scenes 1 and 2; one
# label is 30 pixels high.
TRACK_PITCH=283
ROW_PITCH=250
BUTTON_WIDTH=205
LAUNCH_MARGIN=25
LAUNCH_WIDTH=2240
LAUNCH_SIZE=${LAUNCH_WIDTH}x280
SCENE_SIZE=250x30
# The labels beside the back button and the forward button: one line every 29
# pixels, centred on the button, which is 135 pixels high, in a strip 20
# pixels higher than it at the top and the bottom. A line's capitals are 20
# pixels high.
STEP_HEIGHT=175
STEP_SIZE=300x$STEP_HEIGHT
STEP_POINTSIZE=27
STEP_PITCH=29
STEP_CAP_HEIGHT=20

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

# Each label of the launch buttons: whether it is a control, which is lit in
# every launch button's clip editing, a setting, which is lit only while the
# launch button holds a clip, or an overlay setting, which is lit only in
# fill-in editing of a fill-in, and so on; its track and
# scene, both from 1; and its text, with "|" for a space.
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
overlay 8 2 1|BAR|PATT
"

# Draws every label of the launch buttons, the controls in one colour, the
# settings in another and the overlay settings in a third. With north gravity
# the x offset of a text counts from the middle of the strip.
draw_launch_labels() {
	control_colour=$1
	setting_colour=$2
	overlay_colour=$3
	target=$4
	set -- -size "$LAUNCH_SIZE" xc:none -font "$FONT" -pointsize "$POINTSIZE" -gravity north
	while read -r kind track scene text; do
		[ -n "$kind" ] || continue
		if [ "$kind" = control ]; then
			colour=$control_colour
		elif [ "$kind" = overlay ]; then
			colour=$overlay_colour
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

draw_launch_labels "$UNLIT" "$UNLIT" "$UNLIT" "$WORK/launch0.png"
draw_launch_labels "$YELLOW" "$UNLIT" "$UNLIT" "$WORK/launch1.png"
draw_launch_labels "$YELLOW" "$YELLOW" "$UNLIT" "$WORK/launch2.png"
draw_launch_labels "$ORANGE" "$UNLIT" "$UNLIT" "$WORK/launch3.png"
draw_launch_labels "$ORANGE" "$ORANGE" "$ORANGE" "$WORK/launch4.png"
draw_launch_labels "$BLUE" "$UNLIT" "$UNLIT" "$WORK/launch5.png"
draw_launch_labels "$BLUE" "$BLUE" "$BLUE" "$WORK/launch6.png"
draw_launch_labels "$PINK" "$UNLIT" "$UNLIT" "$WORK/launch7.png"
draw_launch_labels "$PINK" "$PINK" "$PINK" "$WORK/launch8.png"
draw_launch_labels "$TURQUOISE" "$UNLIT" "$UNLIT" "$WORK/launch9.png"
draw_launch_labels "$TURQUOISE" "$TURQUOISE" "$TURQUOISE" "$WORK/launch10.png"
magick "$WORK"/launch[0-9].png "$WORK"/launch10.png -background none -append +repage "PNG32:GUI2D/Labels_Launch_11frames.png"
echo "Wrote GUI2D/Labels_Launch_11frames.png"

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

# The lines beside the back button and the forward button, from the top: the
# colour each is lit in, and its text, with "|" for a space.
BACK_LINES="
$WHITE LAUNCH
$STEP_GREEN PICK|CLIP
$YELLOW EDIT|CLIP
$ORANGE SET|FILL-IN
$BLUE SET|LANDING
$PINK SET|INTRO
"
FORWARD_LINES="
$ORANGE SET|FILL-IN
$BLUE SET|LANDING
$PINK SET|INTRO
$TURQUOISE SET|OUTRO
"

# Draws one frame of the lines beside a button, line number lit (from 1) lit
# and the others unlit; with lit 0 all are unlit. The gravity aligns them on
# one side.
# The shell has no local variables, so these have names of their own.
draw_step_frame() {
	frame_lines=$1
	frame_gravity=$2
	frame_lit=$3
	frame_target=$4
	frame_count=$(echo "$frame_lines" | grep -c .)
	frame_top=$(( (STEP_HEIGHT - (frame_count - 1) * STEP_PITCH - STEP_CAP_HEIGHT) / 2 ))
	set -- -size "$STEP_SIZE" xc:none -font "$FONT" -pointsize "$STEP_POINTSIZE" -gravity "$frame_gravity"
	line=1
	while read -r colour text; do
		[ -n "$colour" ] || continue
		[ "$line" -eq "$frame_lit" ] || colour=$UNLIT
		set -- "$@" -fill "$colour" -annotate "+0+$((frame_top + (line - 1) * STEP_PITCH))" "$(echo "$text" | tr '|' ' ')"
		line=$((line + 1))
	done <<LINES
$frame_lines
LINES
	magick "$@" "PNG32:$frame_target"
}

# Draws the film strip of the lines beside a button: all unlit, then each
# line lit in turn.
draw_step_strip() {
	lines=$1
	gravity=$2
	target=$3
	count=$(echo "$lines" | grep -c .)
	lit=0
	while [ "$lit" -le "$count" ]; do
		draw_step_frame "$lines" "$gravity" "$lit" "$WORK/step$lit.png"
		lit=$((lit + 1))
	done
	magick "$WORK"/step[0-$count].png -background none -append +repage "PNG32:$target"
	rm "$WORK"/step*.png
	echo "Wrote $target"
}

draw_step_strip "$BACK_LINES" northeast GUI2D/Label_Back_7frames.png
draw_step_strip "$FORWARD_LINES" northwest GUI2D/Label_Forward_5frames.png
draw_strip "$SCENE_SIZE" "$POINTSIZE" FILL-IN GUI2D/Caption_Fill_In_2frames.png "$WHITE" "$ORANGE"
draw_strip "$SCENE_SIZE" "$POINTSIZE" CLEAR GUI2D/Caption_Clear_3frames.png "$WHITE" "$UNLIT" "$RED"
draw_strip "$SCENE_SIZE" "$POINTSIZE" DUPLICATE GUI2D/Caption_Duplicate_3frames.png "$WHITE" "$UNLIT" "$FUCHSIA"
draw_strip "$SCENE_SIZE" "$POINTSIZE" MUTE GUI2D/Caption_Mute_2frames.png "$WHITE" "$YELLOW"
draw_strip "$SCENE_SIZE" "$POINTSIZE" SOLO GUI2D/Caption_Solo_2frames.png "$WHITE" "$BLUE"
draw_strip "$SCENE_SIZE" "$POINTSIZE" STOP GUI2D/Caption_Stop_2frames.png "$WHITE" "$RED"

# SETUP is 118 by 26 pixels when trimmed, aligned on the right and centred
# vertically in a frame of 125 by 30 pixels.
index=0
for colour in "$WHITE" "$UNLIT" "$STEP_GREEN" "$YELLOW" "$ORANGE" "$BLUE" "$PINK" "$TURQUOISE"; do
	magick -size 300x80 xc:none -font "$FONT" -pointsize "$POINTSIZE" -fill "$colour" -annotate +10+50 SETUP -trim +repage -background none -gravity east -extent 125x30 "PNG32:$WORK/setup$index.png"
	index=$((index + 1))
done
magick "$WORK"/setup[0-7].png -background none -append +repage "PNG32:GUI2D/Caption_Setup_8frames.png"
echo "Wrote GUI2D/Caption_Setup_8frames.png"
