# Launchnik

A utility rack extension for Reason that launches Player patterns in the manner of a clip launcher, by sending pattern-selecting CV to Combinators that wrap Player devices.

## Language

**Launch button**:
One of the 64 large buttons on the front panel, arranged in eight rows on each of eight **tracks**; it either holds a **clip** or is **empty**.
_Avoid_: Clip button, pad, cell

**Clip**:
One pattern (1 to 8) of the track's Player together with a **colour**, held by a **launch button**; launching the clip makes the track's Player play that pattern. Several clips on a track may use the same pattern.
_Avoid_: Slot, pattern (for the clip itself); a **scene** is a row of launch buttons, not a clip

**Empty**:
Said of a **launch button** that holds no **clip**. A new device has only empty launch buttons. Choosing an empty launch button counts as choosing the track's **stop button**.
_Avoid_: Inactive, disabled, cleared

**Colour**:
One of fourteen hues that mark a **clip** so that it can be recognised: white, red, orange, yellow, lime, green, turquoise, cyan, light blue, blue, dark blue, purple, fuchsia, pink (numbered 1 to 14 in that order). It has no effect on what the clip does. A new clip has the colour of its track: red, orange, yellow, green, cyan, blue, purple, pink for tracks 1 to 8.
_Avoid_: Tint, label

**Patch**:
What all 64 **launch buttons** hold, together with the **length** of the switch interval, saved as a file that can be loaded into any Launchnik, in any song. It does not hold what the tracks are playing: loading a patch changes no **selection**. A new device has the patch in which every launch button is **empty**.
_Avoid_: Template, preset, layout

**Unlit**, **dim**, **vibrant**:
The three ways a **launch button** can look: showing no **colour** at all, showing a colour at low brightness, or showing it at full brightness. A flashing launch button alternates between dim and vibrant. A **stop button** looks the same three ways, always in red.
_Avoid_: Inactive (for unlit), bright, lit

**Stop button**:
The small button at the foot of a **track** that stands for the Player's pattern selection being "off". Its **light** says whether there is anything to stop: unlit while the track has no **clips**, dim while the track is stopped, vibrant while a clip is **active**, flashing while the stop is **queued**.
_Avoid_: Off button, mute button

**Track**:
One column of the front panel: eight **launch buttons**, one **stop button** and the one CV output they drive.
_Avoid_: Column, channel, lane

**Scene**:
One row of the front panel: the eight **launch buttons** at the same height on the eight **tracks**. Scenes are numbered 1 to 8 from the top.
_Avoid_: Row (in prose), line

**Scene button**:
The button to the right of a **scene**. In **launch mode**, clicking it means clicking all eight **launch buttons** of its scene at once, so a track whose launch button in that scene is **empty** stops. In **clip picking** the eight scene buttons stand for the eight **lengths** instead, immediate at the bottom and sixteen bars at the top: clicking one chooses that length and returns to launch mode. In **clip editing** a click does nothing. It never shows which scene was last chosen; outside clip picking its **light** belongs to the **interval meter**.
_Avoid_: Scene launch button, row button

**Selection**:
What the user last chose on a **track**: the **stop button** or one of the track's **clips**. Choosing an **empty** launch button is a selection of the stop button. It is what is saved with the song; a selection that the track is not yet emitting is **queued**.
_Avoid_: Target, request, choice

**Off**:
The state of a button that is neither **queued** nor **active**. An off launch button is **dim** if it holds a **clip** and **unlit** if it is **empty**. An off stop button is **vibrant**: a clip is active and can be stopped.

**Queued**:
The state of a button that has been clicked and is flashing while it waits for the next **switch point**; a track has at most one queued button. An **active** launch button also flashes while a change of its clip's pattern waits for the switch point.
_Avoid_: Pending, armed, flashing, cued

**Active**:
The state of a button whose value its track's CV output is currently emitting; a track always has exactly one active button. An active launch button is steadily **vibrant**; an active stop button is **dim**, or **unlit** if its track has no clips.
_Avoid_: Static, on, playing; also "active" for a launch button that holds a clip (say it holds a clip, or is not **empty**)

**Launch mode**:
The normal way of working, in which clicking a **launch button** selects its **clip**.
_Avoid_: Performance mode, play mode, normal mode

**Setup mode**:
The way of working in which the **launch buttons** are used to change **clips** instead of launching them. Tracks go on playing and **stop buttons** go on working while it lasts; **scene buttons** launch nothing. The logo on the panel shows the **mode colour** of the step it is in.
_Avoid_: Edit mode, config mode

**Setup button**:
The button to the left of the **stop buttons** that switches between **launch mode** and **setup mode**.

**Clip picking**:
The first step of **setup mode**, in which every **launch button** flashes (in its **clip's** **colour**, or in white if it is **empty**) and the user chooses the one to set up. The **scene buttons** flash as well, the one that stands for the current **length** in green and the others in white, and the user may choose a length instead.
_Avoid_: Clip selection (a **selection** is what a track has chosen to play)

**Clip editing**:
The second step of **setup mode**, in which the **launch buttons** stand for the settings of the one picked launch button: whether it holds a **clip**, the clip's **colour** and its pattern.
_Avoid_: Edit mode

**Light**:
What a **launch button** is showing at the moment, in either mode: **unlit**, or one **colour** that is **dim**, **vibrant** or flashing. A **stop button** has a light too, always red, and so has a **scene button**, as part of the **interval meter**. The front panel and a control surface show the same light for the same button.
_Avoid_: Lamp, LED, look, state (a state is **off**, **queued** or **active**)

**Label**:
The caption printed below a **launch button** or **scene button** that tells what the button does in **setup mode** (a colour, a pattern, a **length**, set/clear, back). Every label is printed the same in every mode, but like a **light** it is either lit or **unlit**: lit while it applies, in the **mode colour** or in white, and barely visible dark grey otherwise. The captions SETUP and STOP and the numbers of the tracks and scenes are not labels: they are always white.
_Avoid_: Caption, legend, text, tag

**Mode colour**:
The colour that shows which way of working is on, worn by the logo and by the lit **labels**: white for **launch mode**, green for **clip picking**, yellow for **clip editing**.
_Avoid_: Theme, accent

**Switch interval**:
One of the equal, consecutive stretches into which the host's song position is divided, counted from the start of the song. How long they are is the **length**.
_Avoid_: Quantisation period, launch quantise, phrase

**Length**:
How long every **switch interval** is; one of eight settings: **immediate**, one beat, half a bar, one bar, two, four, eight or sixteen bars. A beat is the beat of the time signature, so all of them line up with the bars; where a bar has an odd number of beats, half a bar falls between two beats. A new device has four bars. The length is part of a **patch**.
_Avoid_: Quantisation, launch quantise, interval size, resolution

**Immediate**:
The **length** with which there are no **switch intervals** and no **switch points**: a selection takes effect at once, and nothing is ever **queued**.
_Avoid_: Off (an **off** button is something else), none, unquantised

**Switch point**:
The moment in musical time at which one **switch interval** ends and the next begins; while the song plays, the only moment at which a track's selection takes effect, unless the **length** is **immediate**.
_Avoid_: Boundary, downbeat, launch point

**Segment**:
One of the eight equal parts of a **switch interval**; with a **length** of four bars in 4/4 time, two beats. A segment need not begin on a beat.
_Avoid_: Step, slice, division

**Interval meter**:
The **lights** of the eight **scene buttons** read together, showing how far the song position is into the current **switch interval**, whether the song is playing or not, in **launch mode** and in **clip editing**. One scene button stands for each **segment**, counted from the bottom; a scene button is lit once the song position has reached its segment, so the meter fills from the bottom and starts again with one lit button at each **switch point**. The lower five are green, the next two yellow and the top one red. Before the start of the song, and while the **length** is **immediate**, all are **unlit**.
_Avoid_: Progress bar, position meter, countdown
