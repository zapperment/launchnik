#!/bin/sh
# Copies the decorations of the back panel and the folded back panel from
# Image_Sources into GUI2D: the model label, the sticker, the power label and
# the small logo. Each is centred on a transparent canvas whose sides are
# multiples of 5 pixels, the only sizes RE2DRender accepts.
#
# Needs ImageMagick.
set -e
cd "$(dirname "$0")/.."

decal() {
	magick "Image_Sources/$1.png" -background none -gravity center -extent "$2" +repage "PNG32:GUI2D/$1.png"
	echo "Wrote GUI2D/$1.png"
}

decal Model_Label 550x370
decal Sticker 730x745
decal Power_Label 600x295
decal Launchnik_Logo_small 235x95
