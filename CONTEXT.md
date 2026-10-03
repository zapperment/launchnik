# Launchnik

A utility rack extension for Reason that launches Player patterns in the manner of a clip launcher, by sending pattern-selecting CV to Combinators that wrap Player devices.

## Language

**Launch button**:
One of the 64 large buttons on the front panel, arranged in eight rows on each of eight **tracks**; it stands for one pattern of the Player controlled by its track.
_Avoid_: Clip button, pad, cell

**Stop button**:
The small button at the foot of a **track** that stands for the Player's pattern selection being "off".
_Avoid_: Off button, mute button

**Track**:
One column of the front panel: eight **launch buttons**, one **stop button** and the one CV output they drive.
_Avoid_: Column, channel, lane

**Selection**:
The button the user last chose on a **track**: the **stop button** or one of the eight **launch buttons**. It is what is saved with the song; a selection that the track is not yet emitting is **queued**.
_Avoid_: Target, request, choice

**Off**:
The state of a button that is unlit and has nothing pending.

**Queued**:
The state of a button that has been clicked and is flashing while it waits for the next **switch point**; a track has at most one queued button.
_Avoid_: Pending, armed, flashing, cued

**Active**:
The state of a button that is permanently lit because its track's CV output is currently emitting its value; a track always has exactly one active button.
_Avoid_: Static, on, playing

**Switch interval**:
One of the equal, consecutive segments, four bars long, into which the host's song position is divided.
_Avoid_: Quantisation period, launch quantise, phrase

**Switch point**:
The moment in musical time at which one **switch interval** ends and the next begins; the only moment at which a track's selection changes.
_Avoid_: Boundary, downbeat, launch point
