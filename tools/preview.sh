#!/bin/sh
# Rebuilds Launchnik for a look in Reason Recon after a change to the GUI:
# renders the graphics, builds for Recon and clears Recon's graphics cache, in
# that order. Restart Recon afterwards and insert the device again.
set -e

cd "$(dirname "$0")/.."
"$HOME/JukeboxSDK/RE2DRender/RE2DRender" "$PWD/GUI2D" "$PWD/GUI"
python3 build45.py local45 Testing
rm -rf "$HOME/Library/Caches/Reason Recon/GraphicsCache"
echo "Done. Restart Recon and insert Launchnik again."
