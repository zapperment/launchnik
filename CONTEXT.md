# Launchnik

A utility rack extension for Reason that launches Player patterns in the manner of a clip launcher, by sending pattern-selecting CV to Combinators that wrap Player devices.

## Language

**Launch button**:
One of the 64 large buttons on the front panel, arranged in eight rows on each of eight **tracks**; it either holds a **clip** or is **empty**.
_Avoid_: Clip button, pad, cell

**Clip**:
One pattern (1 to 8) of the track's Player together with a **colour**, held by a **launch button**; launching the clip makes the track's Player play that pattern. Several clips on a track may use the same pattern.
_Avoid_: Slot, scene, pattern (for the clip itself)

**Empty**:
Said of a **launch button** that holds no **clip**. A new device has only empty launch buttons. Choosing an empty launch button counts as choosing the track's **stop button**.
_Avoid_: Inactive, disabled, cleared

**Colour**:
One of fourteen hues that mark a **clip** so that it can be recognised: white, red, orange, yellow, lime, green, turquoise, cyan, light blue, blue, dark blue, purple, fuchsia, pink (numbered 1 to 14 in that order). It has no effect on what the clip does.
_Avoid_: Tint, label

**Unlit**, **dim**, **vibrant**:
The three ways a **launch button** can look: showing no **colour** at all, showing a colour at low brightness, or showing it at full brightness. A flashing launch button alternates between dim and vibrant.
_Avoid_: Inactive (for unlit), bright, lit

**Stop button**:
The small button at the foot of a **track** that stands for the Player's pattern selection being "off".
_Avoid_: Off button, mute button

**Track**:
One column of the front panel: eight **launch buttons**, one **stop button** and the one CV output they drive.
_Avoid_: Column, channel, lane

**Selection**:
What the user last chose on a **track**: the **stop button** or one of the track's **clips**. Choosing an **empty** launch button is a selection of the stop button. It is what is saved with the song; a selection that the track is not yet emitting is **queued**.
_Avoid_: Target, request, choice

**Off**:
The state of a button that is neither **queued** nor **active**. An off launch button is **dim** if it holds a **clip** and **unlit** if it is **empty**; an off stop button is unlit.

**Queued**:
The state of a button that has been clicked and is flashing while it waits for the next **switch point**; a track has at most one queued button.
_Avoid_: Pending, armed, flashing, cued

**Active**:
The state of a button whose value its track's CV output is currently emitting; a track always has exactly one active button. An active launch button is steadily **vibrant**; an active stop button is steadily lit.
_Avoid_: Static, on, playing; also "active" for a launch button that holds a clip (say it holds a clip, or is not **empty**)

**Launch mode**:
The normal way of working, in which clicking a **launch button** selects its **clip**.
_Avoid_: Performance mode, play mode, normal mode

**Setup mode**:
The way of working in which the **launch buttons** are used to change **clips** instead of launching them. Tracks go on playing, and **stop buttons** go on working, while it lasts.
_Avoid_: Edit mode, config mode

**Setup button**:
The button to the left of the **stop buttons** that switches between **launch mode** and **setup mode**.

**Clip picking**:
The first step of **setup mode**, in which every **launch button** flashes (in its **clip's** **colour**, or in white if it is **empty**) and the user chooses the one to set up.
_Avoid_: Clip selection (a **selection** is what a track has chosen to play)

**Clip editing**:
The second step of **setup mode**, in which the **launch buttons** stand for the settings of the one picked launch button: whether it holds a **clip**, the clip's **colour** and its pattern.
_Avoid_: Edit mode

**Light**:
What a **launch button** is showing at the moment, in either mode: **unlit**, or one **colour** that is **dim**, **vibrant** or flashing. The front panel and a control surface show the same light for the same launch button.
_Avoid_: Lamp, LED, look, state (a state is **off**, **queued** or **active**)

**Switch interval**:
One of the equal, consecutive segments, four bars long, into which the host's song position is divided.
_Avoid_: Quantisation period, launch quantise, phrase

**Switch point**:
The moment in musical time at which one **switch interval** ends and the next begins; the only moment at which a track's selection changes.
_Avoid_: Boundary, downbeat, launch point
