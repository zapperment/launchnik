# Launchnik

Launchnik is a utility Rack Extension for Reason that launches Player patterns the way clips are launched in the session view of Ableton Live. It sends pattern-selecting CV to Combinators that wrap Player devices, and changes patterns only at regular points in the song, every four bars unless set otherwise, so that everything stays in time. Each track has six CV outputs: one or more of them select patterns, the others send unipolar or bipolar values that can glide from clip to clip, for filter sweeps or risers. What each launch button sends, and in which colour it shows, is set up on the panel itself.

Version 1.0.0d21, a development build. Requires Reason 14 or later.

## The panel

The front panel has eight **tracks**, one per column. Each track has eight **launch buttons** and a **function button** underneath, which stops, mutes or solos the track depending on the **function** chosen with the **function select buttons** below: MUTE under track 2, SOLO under track 3 and STOP under track 8. A row of launch buttons is a **scene**, with its **scene button** to the right. To the left of the function buttons is the **setup button**, to the left of scene 3 the **clear button**, to the left of scene 4 the **duplicate button**, to the left of scene 6 the **fill-in button**, and above the first two tracks the **back button** and the **forward button**, with the steps of setup mode they lead to beside them. To the right of the scene buttons are the **transition diagrams**, one set each for clips, fill-ins, landings, intros and outros. Above every track, launch button and scene button is its name, which you can change. The back panel has six **CV outputs**, A to F from the top, for each track, each with a switch for its **CV mode** and a name, which you can change, and the track names above them.

A launch button either holds a **clip** or is **empty**. A clip has one of fourteen colours and, for each of its track's six CV outputs, a pattern 1–8, a unipolar value, a bipolar value and a transition. A new device has only empty launch buttons.

In **launch mode**, the normal way of working, a button is in one of three states:

| State | Launch button with a clip | Meaning |
|---|---|---|
| Off | Dim, in the clip's colour | Nothing pending. |
| Queued | Flashing on the beat | Clicked, waiting for the next switch point. |
| Active | Vibrant | The track is emitting this button's CV value. |

An empty launch button is unlit, and clicking it has the same effect as clicking the track's stop button. A track always has exactly one active button and at most one queued button. A new device starts with every track stopped.

A song always opens with the function STOP, in which the function buttons are **stop buttons**. A stop button shows whether there is anything to stop, as in Ableton Live:

| Stop button | Meaning |
|---|---|
| Unlit | The track has no clips. |
| Dim red | The track is stopped. |
| Vibrant red | A clip is active; clicking stops it. |
| Flashing | Clicked, the track stops at the next switch point. |

## Mute and solo

The function select buttons work like radio buttons: the one for the current function is lit in its colour, red for STOP, yellow for MUTE, blue for SOLO, with its caption; the others are dim white with a white caption. Choosing a function changes only what the function buttons do, never what any track plays.

In MUTE the function buttons are yellow. Click one to **mute** its track: from the click until the next switch point its CV outputs in pattern mode emit 0 (those in unipolar or bipolar mode carry on), and its function button is dim yellow; the launch buttons go on as before. Click it again to unmute the track sooner. Several tracks can be muted at once.

In SOLO the function buttons are blue. Click one to **solo** its track: from the click until the next switch point every track that is not soloed emits 0, and the soloed track's function button is vibrant blue. Several tracks can be soloed at once. A soloed track plays even if it is muted; switching the solo off again leaves every track muted or not, as it was.

| Function button | STOP | MUTE | SOLO |
|---|---|---|---|
| Unlit | The track has no clips | The track has no clips | The track has no clips |
| Dim | The track is stopped | The track is muted | The track is not soloed |
| Vibrant | A clip is active | The track is not muted | The track is soloed |
| Flashing | The track stops at the next switch point | – | – |

Mutes and solos end by themselves at the next switch point, so a break needs no second click on the downbeat. With the switch interval "immediate" there is no switch point, so they last until they are clicked off. They also end when the song position jumps, by a locate or a loop, and when their track has no clips left. A click on the function button of a track without clips does nothing, in every function. With the transport stopped, mutes and solos last until the song plays through the next switch point.

The function buttons and the function select buttons work in launch mode and setup mode alike. A song always opens with nothing muted or soloed.

## Scenes and the interval meter

Clicking a scene button has the same effect as clicking all eight launch buttons of its scene: every track with a clip in that row is queued to play it, and every track whose launch button in that row is empty is queued to stop. Scene buttons launch in launch mode only; in setup mode they are used to choose the length of the switch interval, see below.

A scene button does not show which scene was launched; the launch buttons do that. Together, the eight scene buttons are the **interval meter**, which shows how far the song position is into the current switch interval. Each stands for one eighth of the interval, a **segment**: two beats in 4/4 with a switch interval of four bars. They light up from the bottom, the lower five in green, the next two in yellow and the top one in red, so a red button means that the switch point is less than a segment away. At the switch point the meter starts again with the bottom button.

The meter follows the song position whether the song plays or not, and goes on running in clip editing. Before the start of the song, and while the length of the switch interval is "immediate", all scene buttons are unlit.

## Naming tracks, clips and scenes

Every track, launch button, scene and CV output has a name, so that you can tell at a glance what each is for: the instrument a track plays, the pattern a clip sends, the part of the song a scene belongs to, what a CV output controls. A **track name** stands above the track on the front panel and above the track's CV outputs on the back panel; a **clip name** stands above its launch button, a **scene name** above its scene button, and a **CV output name** above its CV output and switch on the back panel. A new device names them Track 1 to Track 8, Clip 1 to Clip 8 (by scene, on every track), Scene 1 to Scene 8, and CV Out 1A to CV Out 8F.

Double-click a name to change it: a text field opens with the name selected. Type the new name and press Return, or press Escape to leave it as it was. Clear the field and press Return to give the name back its default, such as "Track 3". Changing a track name on one panel changes it on the other.

About twelve characters fit; a longer name is kept in full, but shown cut off at both ends. Names are not in capitals, unlike the labels.

Track names and clip names are shown in launch mode, clear mode, duplicate mode and clip picking. In clip editing, fill-in editing, landing editing, intro editing and outro editing, where the launch buttons stand for a clip's settings, only the name of the clip being set up and of its track stay visible, so you can see which one it is. Scene names are shown in launch mode, clear mode and duplicate mode only, since in clip picking the scene buttons stand for the lengths of the switch interval. On the back panel the track names and CV output names are always shown.

A launch button keeps its clip name while it is empty, like the rest of its clip, so the name comes back with the clip. Fill-ins, landings, intros and outros have no names of their own.

Names are saved with the song and in patches, and each change is a step in the undo history. They cannot be automated, and control surfaces can neither show nor change them.

## CV outputs

Each track has six CV outputs on the back panel, A to F from the top, each with its name above it and a three-way switch to its left that chooses its **CV mode**:

| CV mode | What the output emits |
|---|---|
| Pattern | The pattern of the playing clip, in the steps a Combinator reads as pattern selection; 0 (Off) while the track is stopped, muted or solo-muted. |
| Unipolar | One of seven values from 0 to 1 (labelled 0 to 100). |
| Bipolar | One of seven values from −1 to +1 (labelled −100 to +100, 0 in the middle). |

A new device has output A in pattern mode and B to F in unipolar mode. The CV modes are saved with the song and in patches, but cannot be automated or changed from a control surface. Flipping a switch takes effect at once.

Unipolar and bipolar outputs ignore mute and solo, and when a track stops they go on emitting the value they last had; before any clip has played, that is 0.

### Connecting a track to a Player

1. Put a Player device (for example PolyStep Sequencer) and its instrument in a Combinator.
2. Connect a CV output in pattern mode, such as output A, to a CV input on the back of the Combinator.
3. In the Combinator's editor, add a mapping with that CV input as the source and the Player's pattern selection as the target.
4. Set the mapping to Min = Off, Max = 8, and leave the source range at 0 %–100 %.

A clip with pattern N on that output now selects pattern N, and the stop button selects Off. A unipolar or bipolar output can be connected to any CV input, for example a filter's cutoff.

### Transitions

A unipolar or bipolar output of a clip can have a **transition**: off, A, B, C or D. While the clip is playing and another clip on its track is queued, the transition moves the output from the playing clip's value to the queued clip's, landing on it at the switch point. It is the playing clip's transition that counts. If the queued clip holds a landing, the output moves to the landing's value instead (see [Landings](#landings)). With transition off the value jumps at the switch point.

| Transition | Moves | When |
|---|---|---|
| A | In a straight line | From the moment the other clip is queued |
| B | Along an exponential curve, slowly at first | From the moment the other clip is queued |
| C | In a straight line | Within the last bar before the switch point |
| D | Along an exponential curve | Within the last bar before the switch point |

With a switch interval of one bar or less, C and D behave like A and B. A queued stop, a stopped track and the length "immediate" never move: the value jumps. Queueing a different clip, editing the queued clip's value or changing the transition starts the movement over from where the output is; choosing the playing clip again, or queueing stop, makes the value jump back. Stopping the transport makes whatever is queued take effect at once, without a transition.

## Setting up clips

Click the setup button to enter **setup mode**; the logo in the header shows the colour of the step you are in: green, then yellow, then orange, then blue. Tracks go on playing and the function buttons and function select buttons go on working; only launching, from the grid or the scene buttons, is unavailable.

1. **Clip picking.** Every launch button flashes: in its clip's colour, or in white if it is empty. Click the one you want to set up.
2. **Clip editing.** The grid now shows the settings of that one launch button:

| Where | What it does |
|---|---|
| Row 1, column 8 | Switches the clip on and off: vibrant green with a clip, dim red without. |
| Rows 1 and 2, columns 1–7 | The colour: white, red, orange, yellow, lime, green, turquoise in row 1; cyan, light blue, blue, dark blue, purple, fuchsia, pink in row 2. The chosen one is vibrant. |
| Rows 3 to 8 | One row for each CV output, A to F, laid out by the output's CV mode (see below). |

Each CV output's row shows what its CV mode needs:

| CV mode | Columns 1–7 | Column 8 |
|---|---|---|
| Pattern | Patterns 1–7, dim orange, the chosen one vibrant orange | Pattern 8 |
| Unipolar | Values 0 to 100: the chosen one vibrant cyan, those to its left dim cyan | The transition |
| Bipolar | Values −100 to +100: the chosen one vibrant fuchsia, those between it and 0 dim fuchsia, 0 included | The transition |

Clicking the transition button goes round from off through A, B, C and D. It is dim red while off and otherwise vibrant in the colour of its transition in the diagram: lime for A, light blue for B, purple for C, pink for D. The labels below each launch button have two lines: the unipolar value on the left and the bipolar value on the right (in column 8 the transitions OFF A B C D), and the pattern below. Only the labels that go with the output's CV mode are lit, in the mode colour, and of the transitions only the chosen one. Whenever the CV labels are lit, so is the set of transition diagrams for the step: TRANSITION in clip editing, FILL-IN in fill-in editing, LANDING in landing editing, INTRO in intro editing, OUTRO in outro editing.

The colour and CV rows only show while the launch button holds a clip. A new clip has the colour of its track (red, orange, yellow, green, cyan, blue, purple, pink for tracks 1 to 8) and, on every CV output, the pattern of its row, unipolar and bipolar value 0 and transition off. A clip that is switched off and on again comes back with all its settings. Changing a unipolar or bipolar value of the playing clip takes effect at once.

3. **Fill-in editing.** While the launch button holds a clip, the **forward button** beside the back button lights up in orange, with SET FILL-IN to its right. Click it to set up the clip's **fill-in**; the logo turns orange. The grid is laid out and lit as in clip editing: SET/CLEAR switches the fill-in on and off, and the colour and CV rows only show while there is one. A new fill-in is a copy of its clip's patterns and values, with every transition off, in white. The labels are orange instead of yellow. Row 2, column 8, labelled 1 BAR PATT, switches the fill-in's **one-bar pattern** on and off, as in landing editing (below).
4. **Landing editing.** In fill-in editing the forward button lights up in blue, with SET LANDING, whether or not the clip holds a fill-in. Click it to set up the clip's **landing**; the logo turns blue. The grid is laid out and lit as before, with SET/CLEAR for the landing, and a new landing is likewise a copy of its clip's patterns and values, with every transition off, in white. The labels are blue. Row 2, column 8, labelled 1 BAR PATT, switches **one-bar pattern** on and off: vibrant green when on, dim red when off, like SET/CLEAR.
5. **Intro editing.** In landing editing the forward button lights up in pink, with SET INTRO, whether or not the clip holds a landing. Click it to set up the clip's **intro**; the logo turns pink. The grid is laid out and lit as before, with SET/CLEAR and 1 BAR PATT for the intro, and a new intro is a copy of its clip's patterns and values, with every transition off, in white. The labels are pink.
6. **Outro editing.** In intro editing the forward button lights up in turquoise, with SET OUTRO. Click it to set up the clip's **outro**; the logo turns turquoise, and the labels are turquoise. This is the last step: the forward button is dark.

Click the setup button again to return to launch mode, from any step. The back button above the first track goes back one step instead, and shows the colour of the step it returns to, with that step lit to its left: vibrant pink in outro editing, it returns to intro editing (SET INTRO); vibrant blue in intro editing, it returns to landing editing (SET LANDING); vibrant orange in landing editing, it returns to fill-in editing (SET FILL-IN); vibrant yellow in fill-in editing, it returns to clip editing (EDIT CLIP); vibrant green in clip editing, it returns to clip picking (PICK CLIP); vibrant white in clip picking, it returns to launch mode (LAUNCH). In launch mode it is dark and does nothing.

## Clearing clips

Click the **clear button** to the left of scene 3 to enter **clear mode**, in which you pick what to clear. The clear button and its caption turn red, like the logo, every launch button that holds a clip flashes in its own colour, so that you can tell which is which, and the scene button of every scene that holds one flashes red; empty launch buttons and scenes are unlit. Tracks go on playing, and the fill-in button, function buttons and function select buttons go on working.

- Click a launch button to clear it: it becomes empty, as if you had switched its clip off with SET/CLEAR in clip editing. It keeps its clip's settings, fill-in, landing, intro, outro and name, so switching it on again in setup mode brings the clip back as it was. If its track is playing that clip, the track stops at once, without waiting for the switch point, even if the clip was filling in or landing, without an outro, and every CV output of the track goes to 0, unipolar and bipolar ones too.
- Click a scene button to clear every launch button of its scene at once.

Either way Launchnik returns to launch mode, and clearing is one step in the undo history. Clicking an empty launch button or the scene button of an empty scene clears nothing and returns to launch mode as well; so do the clear button, the setup button and the back button, which is vibrant white in clear mode, with LAUNCH lit beside it.

The clear button is dim white in launch mode. In setup mode and duplicate mode it does nothing, so it is dark and its caption dark grey.

## Duplicating clips and scenes

Click the **duplicate button** to the left of scene 4 to enter **duplicate mode**, in which you copy one launch button onto another, or one scene onto another. The duplicate button and its caption turn fuchsia, like the logo. Tracks go on playing, and the fill-in button, function buttons and function select buttons go on working.

1. **Source picking.** Every launch button that holds a clip flashes in its own colour, and the scene button of every scene that holds one flashes fuchsia; empty launch buttons and scenes are unlit. Click the launch button or scene button you want to copy, the **source**.
2. **Destination picking.** If you picked a launch button, it stops flashing and stays lit in its colour, while every other launch button flashes, in its clip's colour or, if empty, in white, and the scene buttons go dark. If you picked a scene, its scene button stays lit in fuchsia, the other scene buttons flash fuchsia, and the launch buttons go dark. Click where the copy goes, the **destination**; clicking the source again does nothing.

The destination becomes exactly like the source: whether it holds a clip, its colour, and every CV output's pattern, unipolar value, bipolar value and transition, its fill-in, landing, intro and outro included, with their one-bar pattern, whether or not they are switched on. Whatever the destination held before is overwritten. A scene is copied launch button by launch button, so a launch button that is empty in the source scene is made empty in the destination scene, as clear mode would; if its track was playing it, the track stops at once. A destination that is playing changes as if you had set it up by hand. Names are not copied: clip names and scene names stay as they were, since a control surface cannot write names and a duplicate should do the same wherever you make it.

Launchnik then returns to launch mode, and the copy is one step in the undo history. The duplicate button, the setup button and the back button, which is vibrant white with LAUNCH lit beside it, return to launch mode without copying anything.

The duplicate button is dim white in launch mode. In setup mode and clear mode it does nothing, so it is dark and its caption dark grey. It cannot be automated.

## Fill-ins

A fill-in is a pattern that a clip plays for a short while instead of its own, such as a drum fill leading into the next pattern. Click the **fill-in button** to the left of scene 6: every track whose playing clip has a fill-in switches to it at once, without waiting for a switch point, and its launch button flashes in the fill-in's colour. At the next switch point each track goes back to its clip, or on to whatever was launched in the meantime. Tracks without a fill-in carry on as before.

The fill-in button is dim white, and vibrant orange, with its caption, from the click until the switch point, whether or not any track has a fill-in to play. It works in launch mode, setup mode, clear mode and duplicate mode alike. A click while a fill-in is playing does nothing, and so does a click while the switch interval is "immediate", which has no switch point to end the fill-in at. With the transport stopped, a fill-in goes on until the song plays through the next switch point.

With **one-bar pattern** on, the outputs in pattern mode play the fill-in's patterns only in the last bar before the switch point, or from the click if that comes later, and the clip's until then; a drum fill one bar long thus leads straight into the switch point. The unipolar and bipolar outputs and the flashing launch button start at the click as usual. A new fill-in has it off.

Changing a playing clip's fill-in takes effect at once. A launch button keeps its fill-in while it is empty, so that it comes back with the clip; fill-ins are part of a patch.

On unipolar and bipolar outputs a fill-in's transition works differently from a clip's: it moves from whatever the output emits at the click to the fill-in's own value, landing on it at the switch point, like a riser, and then the output jumps to the clip's value, or to the queued clip's. With transition off the value jumps to the fill-in's at the click. While a fill-in plays, any transition of its clip is hidden.

## Landings

A landing is what a clip plays for its first switch interval, instead of its own patterns and values, such as a crash cymbal at the start of a drum pattern. It plays when the clip becomes active at a switch point, whether the track was stopped or playing another clip, and each track of a launched scene lands on its own. A clip that has played its fill-in lands again at the switch point where the fill-in ends, unless another clip or stop was queued meanwhile. The launch button shows the landing's colour meanwhile. At the next switch point the clip goes on with its own settings. Launching a clip that is already playing **relaunches** it: whatever was queued on the track is taken back, the launch button flashes in the landing's colour, and the clip lands at the next switch point. Queueing another clip or stop takes the relaunch back. A song that is opened, a clip chosen while the transport is stopped and the switch interval "immediate" never land; moving the song position ends a landing.

On unipolar and bipolar outputs a landing's transition works the other way round from a clip's: it starts at the landing's value at the switch point and moves towards the clip's value, so that the clip's value is reached by the end.

| Transition | Moves | When |
|---|---|---|
| Off | Not at all: the landing's value, then a jump to the clip's | At the next switch point |
| A | In a straight line | Across the whole switch interval |
| B | Along an exponential curve, quickly at first | Across the whole switch interval |
| C | In a straight line | Within the first bar; the clip's value after that |
| D | Along an exponential curve, quickly at first | Within the first bar; the clip's value after that |

With a switch interval of one bar or less, C and D behave like A and B.

With **one-bar pattern** on, the outputs in pattern mode play the landing's patterns for the first bar only, then the clip's, so that a one-bar pattern such as a crash cymbal sounds once even with a longer switch interval. The unipolar and bipolar outputs and the launch button's colour go on with the landing until the switch point. A new landing has it off.

While a clip that holds a landing is queued, the playing clip's transition moves towards the landing's value. If another clip is queued while a landing plays, the landing's movement stops and the playing clip's own transition moves on from where the output is. A fill-in lies over a landing as it does over a clip. Changing a playing landing takes effect at once. A launch button keeps its landing while it is empty; landings are part of a patch.

## Intros

An intro is what a clip plays from the moment it is queued on a stopped track until the switch point at which it starts, so that a melody can begin before the bar, as a chorus may start with a few notes before its first downbeat. It plays only when the track is stopped, also while an outro plays there, which it cuts short; a clip queued while another clip is playing has none. The queued launch button flashes in the intro's colour. At the switch point the clip takes over, or its landing if it holds one. Taking the queued clip back, by choosing stop or another clip, drops the intro at once: the outputs in pattern mode emit nothing and the unipolar and bipolar outputs hold the value they had, unless the newly queued clip has an intro of its own. As with landings, the transport stopped, a song being opened and the switch interval "immediate" play no intro, since nothing is queued; moving the song position drops it.

On unipolar and bipolar outputs an intro's transition moves from the intro's value towards the clip's, or towards its landing's, like a clip's transition: A and B from the moment of queueing to the switch point, C and D within the last bar only. With transition off the intro's value holds until the switch point.

With **one-bar pattern** on, the outputs in pattern mode play the intro's patterns only in the last bar before the switch point (from the queueing, if that comes later) and nothing before that. The unipolar and bipolar outputs start at the queueing all the same. A new intro has it off.

## Outros

An outro is what a clip plays for one switch interval after it stops at a switch point, so that, say, a long delay that the clip switched on can die away rather than stop dead or ring on. It plays when stop takes over from the clip, by the stop button, an empty launch button or a scene, but not when another clip takes over, and not when the playing launch button is emptied, which stops the track at once. The launch button that was stopped stays vibrant in the outro's colour meanwhile. When the outro is over, the outputs in pattern mode emit nothing and the unipolar and bipolar outputs keep the outro's value. A clip launched during an outro takes over at the next switch point, where the outro ends anyway; one that holds an intro cuts the outro short. Stopping while the transport is stopped, a song being opened and the switch interval "immediate" play no outro; moving the song position drops it, and the outputs hold the value they had.

On unipolar and bipolar outputs an outro's transition moves from the value emitted before the switch point towards the outro's value, like a landing's: A and B across the whole switch interval, B and D quickly at first, C and D within the first bar only. With transition off the value jumps to the outro's at the switch point.

With **one-bar pattern** on, the outputs in pattern mode play the outro's patterns for the first bar only and nothing after that. A new outro has it off.

Changing a playing intro or outro takes effect at once. A launch button keeps its intro and outro while it is empty; both are part of a patch.

## Setting the length of the switch interval

In clip picking the scene buttons flash as well. They stand for the eight **lengths** the switch interval can have; the current one flashes in green, the others in white. Click one to choose it, which also returns to launch mode. Clicking the green one returns to launch mode without changing anything.

| Scene button | Length |
|---|---|
| 1 (top) | 16 bars |
| 2 | 8 bars |
| 3 | 4 bars, as on a new device |
| 4 | 2 bars |
| 5 | 1 bar |
| 6 | Half a bar |
| 7 | One beat |
| 8 (bottom) | Immediate: no switch points, every click takes effect at once |

A beat is the beat of the time signature: a crotchet in 4/4, a quaver in 6/8. Where a bar has an odd number of beats, half a bar falls between two beats.

Switch points are always counted from the start of the song, so changing the length while the song plays only moves the next switch point: whatever is queued switches there, or at once if the new length is "immediate".

Changing the pattern of the clip that is playing takes effect at the next switch point, and its launch button flashes until then; a pattern change while the transport is stopped is heard at once. Switching that clip off stops its track at once, whether the transport runs or not, with 0 on every CV output.

Clips are saved with the song, and each change is a step in the undo history. Setup mode itself is not saved: a song always opens in launch mode.

## Patches

A **patch** is what all 64 launch buttons hold, their clip names included, together with the length of the switch interval, the CV modes and names of the CV outputs and the track and scene names, saved as a file, so that the same set of clips can be used in any song. The header of the panel, folded or not, shows the name of the patch and Reason's buttons for stepping through patches, browsing and saving.

A patch does not hold what the tracks are playing. Loading one does not change what any track has selected, so it can be loaded while the song plays: a clip whose pattern changes switches at the next switch point, and a track whose playing launch button is now empty stops at once, with 0 on every CV output. If the patch has another length, the switch points move as they do when the length is changed by hand.

Launchnik comes with two patches: "Init Patch", in which every launch button is empty and which a new device starts with, and "All Patterns", in which every launch button holds a clip, with patterns 1–8 in rows 1–8 and each track in its own colour. Both have a switch interval of four bars and the default names. Patches saved with earlier versions load with the default names too, for the CV outputs those from before 1.0.0d15 (see [0007](docs/adr/0007-an-empty-name-stands-for-the-default.md)).

## How launching works

- The song is divided into **switch intervals**, counted from the start of the song; they are four bars long unless another length is chosen. The moment one ends and the next begins is a **switch point** (bars 1, 5, 9, 13 and so on with four bars).
- With the length "immediate" there are no switch points: a click takes effect at once, whether the transport is running or not.
- **While the transport is running**, a click queues the button. At the next switch point it becomes active, the previously active button on the track goes off, and the CV changes.
- **While the transport is stopped**, a click takes effect at once.
- Clicking another button on the same track before the switch point replaces the queued one. Clicking the queued button again does nothing. Clicking the active button cancels whatever is queued.
- When the transport stops, queued buttons become active immediately, without a transition, and the CV outputs otherwise keep their values.
- Each track's choice is saved with the song. On reopening, it is active straight away.

## Automation and Remote

Each track has one automatable parameter, "Track N Selection", with nine values: Stop and Clip 1–8, the clip being the one in that row. Automated changes behave like clicks. A change that falls exactly on a switch point takes effect there, not one switch interval later. A selection that names an empty launch button counts as Stop.

The fill-in button can be automated as well, as "Fill-In": it records being pressed and released, and a fill-in plays back where it was pressed. So can each track's mute and solo button, as "Track N Mute" and "Track N Solo": a recorded session plays back with the same mutes and solos, whichever function is chosen during playback (see [0005](docs/adr/0005-mute-and-solo-are-presses.md)). Every launch also records a press of "Track N Relaunch", so that relaunches play back too; a selection alone never relaunches.

Right-clicking a launch button or a function button offers automation and Remote Override for its track's selection, in any mode and function; alt-clicking opens the automation lane. The fill-in button does the same for "Fill-In". A scene launch is recorded in the selection lanes of the eight tracks; there is no lane of its own for scenes (see [0004](docs/adr/0004-a-scene-press-writes-eight-selections.md)).

Clips, fill-ins, setup mode, clear mode, duplicate mode, the function, the length of the switch interval and the names cannot be automated.

For control surfaces, Launchnik provides these Remote items (N is the track, M the row, both 1–8):

| Remote item | Direction | Values |
|---|---|---|
| `Track N Selection` (8 items) | Input and output | 0 = stop, 1–8 = clip in that row |
| `Track N Function Light` (8 items) | Output only | 0 = unlit, 1 = dim, 2 = vibrant, 3 = flashing, in the colour of the function |
| `Track N Mute` (8 items) | Input and output | 1 while the track's mute button is held down, 0 otherwise |
| `Track N Solo` (8 items) | Input and output | 1 while the track's solo button is held down, 0 otherwise |
| `Track N Relaunch` (8 items) | Input and output | 1 while a launch on the track is held down, 0 otherwise; a codec holds it down with every launch in launch mode, from a pad or a scene button |
| `Function` | Input and output | 0 = stop, 1 = mute, 2 = solo |
| `Track N Clip M Light` (64 items) | Output only | 0 = unlit, then three values per colour: dim, vibrant, flashing (1–3 white, 4–6 red, … 40–42 pink) |
| `Scene M Light` (8 items) | Output only | As for a launch button; in practice 0 = unlit, 4 = dim red, 10 = dim yellow, 16 = dim green, in clip picking 3 = flashing white, 18 = flashing green, in clear mode 6 = flashing red, and in duplicate mode 39 = flashing fuchsia, 38 = vibrant fuchsia |
| `Track N Clip M Switches` (64 items) | Input and output | 1 if the launch button holds a clip, + 2 × its colour (0–13 for white to pink), 0–27; see [0008](docs/adr/0008-settings-and-switches-are-packed.md) |
| `Track N Clip M Output X` (384 items, X = A–F) | Input and output | The pattern (0–7 for patterns 1–8) + 8 × the unipolar value (0–6 for 0 to 100) + 56 × the bipolar value (0–6 for −100 to +100) + 392 × the transition (0 = off, 1–4 = A–D), 0–1959; see [0008](docs/adr/0008-settings-and-switches-are-packed.md) |
| `Track N Clip M Fill-In Switches` (64 items) | Input and output | 1 if the clip holds a fill-in, + 2 × its colour, + 28 if one-bar pattern is on (the fill-in's patterns play in the last bar only), 0–55 |
| `Track N Clip M Fill-In Output X` (384 items) | Input and output | As for the clip |
| `Track N Clip M Landing Switches` (64 items) | Input and output | As for the fill-in; with one-bar pattern the landing's patterns play for the first bar only |
| `Track N Clip M Landing Output X` (384 items) | Input and output | As for the clip |
| `Track N Clip M Intro Switches`, `… Outro Switches` (64 items each) | Input and output | As for the fill-in; with one-bar pattern the intro's patterns play in the last bar only, the outro's in the first bar only |
| `Track N Clip M Intro Output X`, `… Outro Output X` (384 items each) | Input and output | As for the clip |
| `Track N Output X CV Mode` (48 items) | Output only | 0 = bipolar, 1 = unipolar, 2 = pattern; a copy of the switch on the back panel, which only the mouse can change |
| `Fill-In` | Input and output | 1 while the fill-in button is held down, 0 otherwise |
| `Fill-In Light` | Output only | As for a launch button: 1 = dim white, 8 = vibrant orange |
| `Forward Light` | Output only | As for a launch button: 0 = unlit, 8 = vibrant orange, 29 = vibrant blue, 41 = vibrant pink, 20 = vibrant turquoise |
| `Switch Interval` | Input and output | 0 = immediate, 1 = one beat, 2 = half a bar, 3–7 = 1, 2, 4, 8 and 16 bars |
| `Setup` | Input and output | 0 = launch mode, 1 = clip picking, 2–65 = clip editing, 66–129 = fill-in editing, 130–193 = landing editing, 194–257 = intro editing and 258–321 = outro editing for launch button 1–64, counted down track 1 first, 322 = clear mode, 323 = source picking, 324–387 = destination picking for launch button 1–64 as the source, 388–395 = destination picking for scene 1–8 as the source |

A light is what the launch button shows on the panel, in launch mode and in setup mode alike, so a surface that shows the lights mirrors the panel. "Flashing" is one steady value; the surface flashes by itself.

The output-only items can be used from a Remote map, but they do not appear in "Edit Remote Override Mapping". A codec has to translate a pad press into the property change the same press makes on the panel; see the decision records [0001](docs/adr/0001-selection-in-state-out.md) and [0002](docs/adr/0002-realtime-computes-lights-writers-change-clips.md) for why, and `Press.h` for the rules. A scene button has no Remote item for presses: a codec writes the eight selections itself, in clip picking the length and the setup state, in clear mode the scene's `Switches` items, keeping their colours, and the setup state, and in duplicate mode every setting of the destination scene's launch buttons and the setup state, see [0004](docs/adr/0004-a-scene-press-writes-eight-selections.md).

Launchnik does not respond to MIDI CC messages.

## Known limitations

- Nothing shows the length of the switch interval outside clip picking.
- At the lengths of one beat and half a bar the interval meter runs very fast.
- Switch points are worked out from the current time signature. In a song whose time signature changes part-way through, they may not line up with the bar numbers in the sequencer.
- Only playing through a switch point fires a queued button. Dragging the song position past one does not, and a loop that neither contains nor ends on a switch point keeps a button queued indefinitely.
- A clip that was switched off while its track's selection still named it is queued again as soon as it is switched back on, or its clear undone.
- There is no stop-all button.
- A scene launch is recorded as automation of each track's selection, not as one event.
- Patches cannot be loaded or saved from a control surface.
- Names cannot be shown or changed from a control surface.
- Nothing stops a name from being longer than its space; it is then shown cut off at both ends.
- Only one patch besides the empty one is included; the Shop expects more.

## Building

From this directory:

```bash
python3 build45.py local45 Testing
```

This compiles the device and installs it for Reason Recon. Restart Recon to pick up a new build.

Every button has a film strip of 29 frames: unlit, then dim and vibrant for each of the fourteen colours. There are five of them, one for each kind of button: side buttons (the fill-in button, the clear button, the duplicate button and the function select buttons), back button, forward buttons (the scene buttons and the forward button), launch buttons and function buttons (with the setup button). Each is built from a source image in `Image_Sources` that holds an unlit, a vibrant red and a dim red frame; the other colours are made from the red ones with ImageMagick:

```bash
./tools/make-button-strips.py
```

The logo's fourteen frames, one for each colour, are built from the white lettering:

```bash
./tools/make-logo-strip.sh
```

The labels below the launch buttons in scenes 1 and 2, of the scene buttons and the other buttons, and the SETUP caption are drawn with ImageMagick; those of the CV outputs, the transition diagrams with their labels and the captions of the back panel with Pillow, through uv, which also copies the CV mode switch into `GUI2D`:

```bash
./tools/make-label-strips.sh
```

```bash
./tools/make-cv-labels.py
```

After changing anything in `GUI2D`, regenerate the panel graphics first, from the `RE2DRender` directory of the SDK download:

```bash
./RE2DRender ~/JukeboxSDK/SDK/Examples/Launchnik/GUI2D ~/JukeboxSDK/SDK/Examples/Launchnik/GUI
```

Two things to know when changing panel graphics:

- Image sizes in `GUI2D` must be multiples of 5 pixels. For any other size, RE2DRender writes a corrected `-reframed.png` copy next to the original and keeps using that copy on later runs, even after the original has changed.
- Reason Recon caches panel graphics and does not refresh them on restart. If a changed image does not show up in the rack, quit Recon and clear the cache:

```bash
rm -rf ~/Library/Caches/"Reason Recon"/GraphicsCache
```

## Testing

The rules for switching, lights and presses can be tested from the command line, without Reason:

```bash
./tests/run.sh
```

The panel, the CV output into a Combinator, automation and Remote can only be tested in Reason Recon.

## Layout of the code

| Path | Contents |
|---|---|
| `Launcher.h`, `Launcher.cpp` | The rules: when a track switches, what it emits, what every button shows. Plain C++ with no dependency on the SDK. |
| `Press.h`, `Press.cpp` | What a press of a button changes, in launch mode and in setup mode. Also free of the SDK. |
| `Launchnik.h`, `Launchnik.cpp` | Connects the rules to the host: reads selections, clips, CV modes, the setup state and the transport, writes states, lights, lamps, labels and CV. |
| `Panel.h`, `Panel.cpp` | The click handler of the launch buttons, the scene buttons, the function buttons, the function select buttons and the buttons of setup mode and fill-ins. |
| `Tags.h` | The property tags shared by the two. |
| `JukeBoxExports.cpp` | Entry points the host calls. |
| `motherboard_def.lua` | Properties, Remote items, automation and CV outputs. |
| `realtime_controller.lua` | Which property changes are passed to the realtime code. |
| `GUI2D/` | Panel layout and the graphics it uses; `gui_functions.lua`, written by `tools/make-gui-functions.py`, decides when the names are shown. |
| `Image_Sources/` | Source graphics that are not stock images. |
| `tools/` | Scripts that build the film strips of the buttons, of the labels and of the logo, and `GUI2D/gui_functions.lua`. |
| `Resources/English/` | Texts shown by the host. |
| `Resources/Public/` | The patches that come with the device. |
| `tests/` | Command-line tests for the rules. |
| `CONTEXT.md` | Glossary of the terms used in the code and in this document. |
| `docs/adr/` | Records of design decisions. |
