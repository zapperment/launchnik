# Launchnik

A utility rack extension for Reason that launches Player patterns in the manner of a clip launcher, by sending pattern-selecting CV to Combinators that wrap Player devices.

## Language

**Launch button**:
One of the 64 large buttons on the front panel, arranged in eight rows on each of eight **tracks**; it either holds a **clip** or is **empty**.
_Avoid_: Clip button, pad, cell

**Clip**:
What a **launch button** holds: a **colour** and, for each of its track's six **CV outputs**, a **pattern**, a **unipolar value**, a **bipolar value** and a **transition**. Launching the clip makes each CV output emit whichever of the three its **CV mode** picks. A launch button keeps all of them while it is **empty**, so a clip comes back as it was; a new clip has, on every CV output, the pattern of its row, unipolar value 0, bipolar value 0 and transition off. Several clips on a track may use the same pattern. A clip may also hold a **fill-in**, a **landing**, an **intro** and an **outro**, each without the others.
_Avoid_: Slot, pattern (for the clip itself); a **scene** is a row of launch buttons, not a clip

**Clip name**:
Text that the user gives a **launch button** so that its **clip** can be recognised. It is shown above the launch button and changed by double-clicking it. A launch button keeps and shows its clip name while it is **empty**, like its clip's other settings, so the name comes back with the clip. Like a **track name**, it shows about 12 characters. It is never shown empty: clearing it gives the launch button its default name back, Clip 1 to Clip 8 by **scene**, which is also what a new device shows. A **fill-in**, **landing**, **intro** or **outro** has no name of its own. It is part of the **patch**, cannot be automated, and is not available to a control surface. It is shown in **launch mode**, **clear mode**, **duplicate mode** and **clip picking**; in **clip editing**, **fill-in editing**, **landing editing**, **intro editing** and **outro editing** only the clip name of the launch button being set up is shown.
_Avoid_: Clip label, launch button name, title

**Fill-in**:
A second **colour** and, for each **CV output**, a second pattern, unipolar value, bipolar value and **transition** that a **clip** may hold, emitted instead of the clip's own while a fill-in is **playing** and the clip is **active**. It lies over the clip: underneath, the clip goes on as if there were no fill-in, its own transition included, and that is what the CV outputs emit again once the fill-in is cleared or over; at the **switch point** they jump, without a transition, to the clip's value or the queued clip's, or to a **landing**'s. A fill-in's transition is the exception to the rule that the active clip's transition counts: it moves from what is emitted at the press to the fill-in's value, landing on it at the switch point, like a riser; off jumps at the press. A launch button keeps its fill-in while it is **empty**, so that it comes back with the clip; an empty launch button has no fill-in to play. The first time a fill-in is switched on, it starts as a copy of its clip's patterns and values, with every transition off and the colour white; switched off and on again, it keeps its settings, as a clip does. Fill-ins are part of the **patch**.
_Avoid_: Fill, drum fill, fill-in mode (it is not a way of working like **launch mode** or **setup mode**)

**Fill-in button**:
The button to the left of **scene** 6 that triggers a fill-in: from the press until the next **switch point**, every track whose **active** clip holds a fill-in plays it, and its active launch button flashes in the fill-in's colour. Tracks without one, and stopped tracks, go on as before. It works in every mode; a press while a fill-in is already playing, or while the **length** is **immediate**, does nothing. Its **light** is dim white, and vibrant orange while a fill-in is playing, whether or not any track has one to play. Its caption FILL-IN is white, and orange while a fill-in is playing. It can be automated.
_Avoid_: Fill button, trigger

**Playing** (of a fill-in):
Said of a fill-in from the press of the **fill-in button** until the next **switch point**. A change to a fill-in, or to whether the active clip holds one, takes effect at once while a fill-in is playing. With the transport stopped it goes on playing until the song reaches the switch point. At the switch point every track goes on with its **selection**, as it would have anyway.

**Landing**:
A further **colour** and, for each **CV output**, a further pattern, unipolar value, bipolar value and **transition** that a **clip** may hold, emitted instead of the clip's own during the first **switch interval** after the clip becomes **active** at a **switch point**, so that a performance can open with, say, a crash cymbal. It plays whether the track was stopped or playing another clip, and in each track of a **scene** on its own. It also plays again when the clip goes on at a switch point after playing its **fill-in**, with nothing else **queued**, and after a **relaunch**. It does not play when a song is loaded, when the transport starts or the song position jumps, nor ever while the **length** is **immediate**. While the clip is **queued**, the active clip's transition moves towards the landing's value rather than the clip's. A landing's own transition is the exception to the rule that the active clip's transition counts, and it runs the other way round: from the landing's value at the start of the interval towards the clip's; off holds the landing's value and jumps at the switch point. A and B move across the whole interval, A in a straight line and B along a curve that leaves quickly and settles slowly (a clip's B mirrored in time); C and D do the same within the first bar only and emit the clip's value for the rest (so with a length of one bar or less, they behave like A and B). If another clip is queued during the landing, the landing's movement is dropped and the active clip's own transition starts over from the value emitted at that moment. A **fill-in** lies over a landing as over a clip. While it plays, the active launch button shows the landing's colour, vibrant, and a change to the landing takes effect at once. The first time a landing is switched on, it starts as a copy of its clip's patterns and values, with every transition off and the colour white; switched off and on again, it keeps its settings, as a clip does. A launch button keeps its landing while it is **empty**; landings are part of the **patch**.
_Avoid_: Opener, landing mode (it is not a way of working like **launch mode** or **setup mode**), intro (an **intro** plays before the switch point, a landing after it)

**Intro**:
A further **colour** and, for each **CV output**, a further pattern, unipolar value, bipolar value and **transition** that a **clip** may hold, emitted from the moment the clip is **queued** on a stopped **track** until the **switch point** at which it becomes **active**, so that a melody can begin before the bar, as “cause I’m a…” leads into the chorus of “Party Girl”. It plays only from a stopped track, also while an **outro** is playing there, which it cuts short at once: a clip queued while another clip is active has no intro, and the active clip's transition moves towards it as usual. It does not play while the **length** is **immediate**, nor while the transport is stopped, since a clip chosen then becomes active at once, nor when a song is loaded, and a jump of the song position drops it, leaving the outputs as with a clip taken back. Taking the queued clip back drops the intro at once: the pattern outputs emit 0 and the unipolar and bipolar outputs hold the value emitted at that moment, as with **stop**; if the newly queued clip holds an intro, that one starts instead. An intro's transition is an exception to the rule that the active clip's transition counts: it moves from the intro's value towards the clip's, or towards the **landing**'s if the clip holds one, the way a clip's transition does: off holds the intro's value and jumps at the switch point, A and B move from the queue, C and D only within the last bar. While it plays, the queued launch button flashes in the intro's colour, and a change to the intro takes effect at once. The first time a intro is switched on, it starts as a copy of its clip's patterns and values, with every transition off and the colour white; switched off and on again, it keeps its settings, as a clip does. A launch button keeps its intro while it is **empty**; intros are part of the **patch**.
_Avoid_: Pickup, anacrusis, lead-in, landing (a **landing** plays after the switch point)

**Outro**:
A further **colour** and, for each **CV output**, a further pattern, unipolar value, bipolar value and **transition** that a **clip** may hold, emitted for one **switch interval** after **stop** takes over from the clip at a **switch point**, so that, say, a long delay that the clip switched on can die away rather than stop dead or ring on. It plays only when the clip gives way to stop, chosen with the **stop button**, an **empty** launch button or a **scene button**: not when another clip takes over, and not when the active launch button is made empty, which stops the track at once. It does not play while the **length** is **immediate**, nor after a stop chosen while the transport is stopped, which takes effect at once, nor when a song is loaded; a jump of the song position drops it, and the outputs hold what they emitted. With the transport stopped it waits for the song, like a landing. Its transition is an exception to the rule that the active clip's transition counts, and it runs from the value emitted before the switch point towards the outro's: off jumps at the switch point; A and B move across the whole interval, A in a straight line and B along a curve that leaves quickly and settles slowly; C and D do the same within the first bar only and hold the outro's value for the rest. Once the outro is over, the pattern outputs emit 0 and the unipolar and bipolar outputs go on emitting the outro's value, as with stop. While it plays, the launch button that was stopped is **vibrant** in the outro's colour, the stop button is dim, and a change to the outro takes effect at once. A clip launched during the outro is queued as usual and takes over at the next switch point, where the outro ends anyway; if it holds an **intro**, the intro takes over at once and the outro is cut short; choosing stop again does nothing, nor does the **fill-in button**. The first time a outro is switched on, it starts as a copy of its clip's patterns and values, with every transition off and the colour white; switched off and on again, it keeps its settings, as a clip does. A launch button keeps its outro while it is **empty**; outros are part of the **patch**.
_Avoid_: Tail, release, ending

**One-bar pattern**:
A switch that a **fill-in**, a **landing**, an **intro** and an **outro** each hold: while it is on, the CV outputs in pattern mode emit the landing's patterns only in the first bar of the landing and the clip's after that; the fill-in's only in the last bar before the **switch point** (from the press, if that comes later) and the clip's before that; the intro's only in the last bar before the switch point (from the queue, if that comes later) and 0 before that; the outro's only in the first bar of the outro and 0 after that; so that a one-bar pattern such as a crash cymbal or a drum fill sounds once. The rest is unchanged: the unipolar and bipolar values go on with their transitions, and the launch button shows its colour, for as long as the fill-in, landing, intro or outro plays. With a **length** of one bar or less it makes no difference. It is off when a fill-in, landing, intro or outro is switched on for the first time, and kept when it is switched off and on again. In **fill-in editing**, **landing editing**, **intro editing** and **outro editing** it is the launch button in row 2, column 8, captioned 1 BAR PATT.
_Avoid_: Short landing, one-bar landing, pattern length

**Relaunch**:
Launching the clip that is already **active** on its track, from its **launch button** or a **scene button** in **launch mode**: it takes back whatever was queued on the track, and the clip lands at the next **switch point**, if it holds a **landing**; meanwhile its launch button flashes in the landing's colour. Queueing another clip or **stop** takes the relaunch back; launching the clip again does not undo it. A clip that becomes active while the transport is stopped has not been relaunched, and with the **length** **immediate** nothing is. Each launch holds the track's relaunch press down, which can be automated like the mute and solo presses; a **selection** alone, such as one played back by automation, never relaunches.
_Avoid_: Retrigger, restart, re-launch

**Forward button**:
The button to the right of the **back button** that goes forward one step of **setup mode**: from **clip editing** to **fill-in editing**, from fill-in editing to **landing editing**, from landing editing to **intro editing**, and from intro editing to **outro editing**. It works only while the picked launch button holds a **clip**, and is then vibrant in the **mode colour** of the step it leads to: orange in clip editing, blue in fill-in editing, pink in landing editing, turquoise in intro editing; in outro editing it is dark and a click does nothing. Like the **back button**, it acts when it is let go, not when it goes down. It cannot be automated.
_Avoid_: Set fill-in button, set landing button, set intro button, set outro button, right arrow, next button

**Empty**:
Said of a **launch button** that holds no **clip**. A new device has only empty launch buttons. Choosing an empty launch button counts as choosing **stop**. Emptying the **active** launch button, from **clear mode**, set/clear in **clip editing**, a control surface or a **patch**, does not wait for the **switch point**: its track stops at once, whether the song plays or not and whatever the clip was playing, its **fill-in** and **landing** included, with no **outro**, and every **CV output** of the track emits 0, unipolar and bipolar ones too.
_Avoid_: Inactive, disabled, cleared

**Colour**:
One of fourteen hues that mark a **clip** so that it can be recognised: white, red, orange, yellow, lime, green, turquoise, cyan, light blue, blue, dark blue, purple, fuchsia, pink (numbered 1 to 14 in that order). It has no effect on what the clip does. A new clip has the colour of its track: red, orange, yellow, green, cyan, blue, purple, pink for tracks 1 to 8.
_Avoid_: Tint, label

**Patch**:
What all 64 **launch buttons** hold (their **clip names** included), together with the **length** of the switch interval, the **CV mode** and **CV output name** of every **CV output**, the **track names** and the **scene names**, saved as a file that can be loaded into any Launchnik, in any song. It does not hold what the tracks are playing: loading a patch changes no **selection**. A new device has the patch in which every launch button is **empty**.
_Avoid_: Template, preset, layout

**Unlit**, **dim**, **vibrant**:
The three ways a **launch button** can look: showing no **colour** at all, showing a colour at low brightness, or showing it at full brightness. A flashing launch button alternates between dim and vibrant. A **function button** looks the same three ways, in the colour of the **function**.
_Avoid_: Inactive (for unlit), bright, lit

**Stop**:
The **selection** that stands for the Player's pattern selection being "off": the track's pattern outputs emit no pattern (0), while its unipolar and bipolar outputs go on emitting the value they last emitted, or 0 once its active launch button was made **empty**. Choosing an **empty** launch button counts as choosing stop.
_Avoid_: Off (an **off** button is something else)

**Function**:
What the **function buttons** do at the moment: **mute**, **solo** or **stop**, chosen with the **function select buttons**. A song always opens with stop. It is not part of the **patch** and cannot be automated; it changes only what a click on a function button means, not what any track plays.
_Avoid_: Mode (**launch mode** and **setup mode** are something else), function mode

**Function button**:
The small button at the foot of a **track**, below its eight **launch buttons**. What a click on it does, and its **light**, depend on the **function**: it is a **stop button**, a mute button or a solo button. While its track has no **clips** it is **unlit** in every function, and a click on it does nothing. It works the same in every mode.
_Avoid_: Track button, stop button (when the function is not stop)

**Stop button**:
A **function button** while the **function** is stop: clicking it chooses **stop** for its track. Its **light** is red and says whether there is anything to stop: unlit while the track has no **clips**, dim while the track is stopped, vibrant while a clip is **active**, flashing while the stop is **queued**.
_Avoid_: Off button

**Mute button**, **solo button**:
A **function button** while the **function** is mute or solo: clicking it switches its track between **muted** and not muted, or between **soloed** and not soloed. A mute button is vibrant yellow, and dim yellow while its track is muted; a solo button is dim blue, and vibrant blue while its track is soloed. The presses can be automated, like those of the **fill-in button**.

**Function select button**:
One of the buttons in the row below the **function buttons** that choose the **function**: MUTE below track 2, SOLO below track 3, STOP below track 8. They work like radio buttons: the one for the current function is vibrant in its colour (yellow, blue, red), the others dim white. Each has a caption below it, white, and in the button's colour while its function is chosen. They work the same in every mode.
_Avoid_: Mode button, function button (that is the row above)

**Muted**:
Said of a **track** from a click on its mute button until the next **switch point**, a second click, a jump of the song position, or until the track has no **clips** left. While it is muted and not **soloed**, its CV outputs in pattern mode emit 0, and those in unipolar or bipolar mode go on as if it were not muted; its **selection** and its **launch buttons** go on as before. Several tracks can be muted at once. With the **length** **immediate** there is no switch point, so a track stays muted until it is clicked again. A song always opens with no track muted.
_Avoid_: Stopped (a stopped track has chosen **stop**), silenced

**Soloed**:
Said of a **track** from a click on its solo button until the next **switch point**, a second click, a jump of the song position, or until the track has no **clips** left. A soloed track plays as it would without any mute or solo, even if it is **muted**; every track that is not soloed is **solo-muted**. Several tracks can be soloed at once. Switching a solo off leaves every track muted or not muted as it was.
_Avoid_: Isolated

**Solo-muted**:
Said of a **track** that is not **soloed** while another track is. Its CV outputs in pattern mode emit 0, whether or not it is **muted**, and the others go on as before; nothing on the panel shows it apart from its dim blue solo button.

**Track**:
One column of the front panel: eight **launch buttons**, one **function button** and the six **CV outputs** they drive.
_Avoid_: Column, channel, lane

**Track name**:
Text that the user gives a **track** so that it can be recognised, such as the instrument it plays. It is shown above the track on the front panel and above its **CV outputs** on the back panel, the same on both, and changed by double-clicking either. The panel shows about 12 characters, centred; a longer name is kept in full but shown cut off at both ends. It is never shown empty: clearing it gives the track its default name back, Track 1 to Track 8, which is also what a new device shows. It is part of the **patch**, cannot be automated, and is not available to a control surface. On the front panel it is shown in **launch mode**, **clear mode**, **duplicate mode** and **clip picking**; in **clip editing**, **fill-in editing**, **landing editing**, **intro editing** and **outro editing** only the track name of the launch button being set up is shown. On the back panel it is always shown.
_Avoid_: Track label, track title, caption

**Scene**:
One row of the front panel: the eight **launch buttons** at the same height on the eight **tracks**. Scenes are numbered 1 to 8 from the top.
_Avoid_: Row (in prose), line

**Scene name**:
Text that the user gives a **scene** so that it can be recognised. It is shown above the **scene button** and changed by double-clicking it; it renames no **clip** of the scene. Like a **track name**, it shows about 12 characters. It is never shown empty: clearing it gives the scene its default name back, Scene 1 to Scene 8, which is also what a new device shows. It is part of the **patch**, cannot be automated, and is not available to a control surface. It is shown only in **launch mode**, **clear mode** and **duplicate mode**, since in **clip picking** the scene buttons stand for **lengths**.
_Avoid_: Scene label, row name

**Scene button**:
The button to the right of a **scene**. In **launch mode**, clicking it means clicking all eight **launch buttons** of its scene at once, so a track whose launch button in that scene is **empty** stops. In **clip picking** the eight scene buttons stand for the eight **lengths** instead, immediate at the bottom and sixteen bars at the top: clicking one chooses that length and returns to launch mode. In **clear mode** clicking it clears its scene, and in **duplicate mode** it picks its scene as the **source** or **destination**. In **clip editing** a click does nothing. It never shows which scene was last chosen; outside clip picking, clear mode and duplicate mode its **light** belongs to the **interval meter**.
_Avoid_: Scene launch button, row button

**CV output**:
One of the six jacks of a **track** on the back panel, lettered A to F, each driven by one row of settings in every **clip** of the track. Written "output A" in full, never a bare "A", since **transitions** are lettered too.
_Avoid_: Jack, CV out, lane, channel

**CV output name**:
Text that the user gives a **CV output** so that it can be recognised, such as what it controls. It is shown above the CV output and its switch on the back panel and changed by double-clicking it. Like a **clip name**, it shows about 12 characters. It is never shown empty: clearing it gives the CV output its default name back, CV Out nA to CV Out nF for the outputs of track n, which is also what a new device shows. It is part of the **patch**, cannot be automated, and is not available to a control surface. It is always shown.
_Avoid_: Output label, jack name

**CV mode**:
What a **CV output** emits, chosen with a three-way switch beside it on the back panel: **pattern**, **unipolar** or **bipolar**. It is part of the **patch**, cannot be automated, and can only be changed on the back panel, not from a control surface; a change takes effect at once, dropping any transition under way. A new device has output A in pattern mode and outputs B to F in unipolar mode.
_Avoid_: Polarity, output mode, type

**Pattern** (of a CV output):
In pattern mode, which of the eight patterns of a Player a **clip** selects; the CV output emits it in the steps a Combinator reads as pattern selection, and 0 while the track is stopped, **muted** or **solo-muted**.

**Unipolar value**, **bipolar value**:
In unipolar mode, one of seven evenly spaced values from 0 to 1 (labelled 0 to 100); in bipolar mode, one of seven from −1 to +1 (labelled −100 to +100, 0 in the middle). A stopped track goes on emitting the value it last emitted; before any clip has played, and after its active launch button was made **empty**, that is 0.
_Avoid_: Level, amount, CV value (when the mode matters)

**Transition**:
How a CV output in unipolar or bipolar mode moves from the value of the **active** clip to that of a **queued** clip on its track: **off**, A, B, C or D. It is the active clip's transition that counts, not the queued one's (a **fill-in**, a **landing**, an **intro** and an **outro** are the exceptions); towards a queued clip that holds a landing, it moves to the landing's value. Off keeps the active clip's value until the **switch point** and then jumps. A and B move from the moment of queueing to the switch point, A in a straight line and B along an exponential curve that starts slowly; C and D do the same as A and B, but only within the last bar before the switch point (so with a **length** of one bar or less, they behave like A and B). The movement follows the song position and lands on the new value at the switch point. A queued **stop**, a track that is stopped, and the length **immediate** never move: the value jumps. Whenever the queued clip changes, its value is edited or the active clip's transition is changed, the movement starts over from the value emitted at that moment; when nothing is left to move towards (the active clip chosen again, stop queued, transition switched off), the value jumps back to the active clip's.
_Avoid_: Transition mode 1–4, interpolation, glide, ramp, slide

**Selection**:
What the user last chose on a **track**: **stop** or one of the track's **clips**. Choosing an **empty** launch button is a selection of stop. It is what is saved with the song; a selection that the track is not yet emitting is **queued**.
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
The way of working in which the **launch buttons** are used to change **clips** instead of launching them. Tracks go on playing and the **function buttons** and **function select buttons** go on working while it lasts; **scene buttons** launch nothing. The logo on the panel shows the **mode colour** of the step it is in.
_Avoid_: Edit mode, config mode

**Setup button**:
The button to the left of the **function buttons** that switches between **launch mode** and **setup mode**. In **clear mode** and **duplicate mode** a click does nothing. Its **light** is dim green in launch mode, dark in clear mode and **duplicate mode**, and in setup mode vibrant in the **mode colour** of the step: green in **clip picking**, yellow in **clip editing**, orange in **fill-in editing**, blue in **landing editing**, pink in **intro editing**, turquoise in **outro editing**. Its caption SETUP is white in launch mode and otherwise coloured like its light, dark grey where the light is dark.

**Clear mode**:
The way of working in which the user picks what to make **empty**: a **launch button**, or with a **scene button** all eight launch buttons of a **scene** at once. Clearing works as the set/clear button of **clip editing** does: the launch button keeps its **clip**'s settings, and a track that was playing the cleared clip stops at once, with 0 on every **CV output** (see **empty**). Picking an empty launch button, or a scene without clips, leaves it empty. Every pick returns to **launch mode**, and clearing a scene is one step of the undo history, like clearing one launch button. It is entered only from launch mode, with the **clear button**; the clear button and the **back button** each return to launch mode without clearing anything, while the **setup button** is dark and does nothing. The **launch buttons** that hold a clip flash in the clip's **colour**, so that the user can tell which one to clear, and the **scene buttons** of scenes that hold one flash red; the others are **unlit**. Tracks go on playing, and the **fill-in button**, **function buttons** and **function select buttons** go on working. It is not part of **setup mode**.
_Avoid_: Clear picking, delete mode, clip picking (that is the first step of setup mode)

**Clear button**:
The button to the left of **scene** 3, above the **fill-in button**, that switches from **launch mode** to **clear mode** and back. Its **light** is dim white in launch mode, vibrant red in clear mode, and dark in **setup mode** and **duplicate mode**, where a click does nothing. Its caption CLEAR is white, red in clear mode, and dark grey in setup mode and duplicate mode. It cannot be automated.
_Avoid_: Delete button, erase button

**Duplicate mode**:
The way of working in which the user copies one **launch button** onto another, or one **scene** onto another, in two steps: **source picking**, then **destination picking**. The **destination** ends up exactly like the **source**: whether it holds a **clip**, and everything the clip holds, its **fill-in**, **landing**, **intro** and **outro** included (whether or not they are switched on), with a value for every **CV mode** of every **CV output**; whatever the destination held before is overwritten. Names are left as they are: a duplicate copies no **clip name** or **scene name**, since a control surface cannot write names, and it should not matter where the user duplicates. A scene is copied launch button by launch button, so a launch button that is **empty** in the source scene is made empty in the destination scene, as in **clear mode**. A destination that is **active** changes as if the user had set it up by hand. Every copy returns to **launch mode**, and is one step of the undo history. It is entered only from launch mode, with the **duplicate button**; the duplicate button and the **back button** each return to launch mode without copying anything, while the **setup button** is dark and does nothing. Tracks go on playing, and the **fill-in button**, **function buttons** and **function select buttons** go on working. It is not part of **setup mode**. Its **mode colour** is fuchsia.
_Avoid_: Copy mode, clone mode, paste

**Duplicate button**:
The button to the left of **scene** 4, below the **clear button**, that switches from **launch mode** to **duplicate mode** and back. Its **light** is dim white in launch mode, vibrant fuchsia in duplicate mode, and dark in **setup mode** and **clear mode**, where a click does nothing. Its caption DUPLICATE is white, fuchsia in duplicate mode, and dark grey in setup mode and clear mode. It cannot be automated.
_Avoid_: Copy button, clone button

**Source**, **destination**:
In **duplicate mode**, the **launch button** or **scene** that is copied, and the one it is copied onto. A source holds a **clip**, or for a scene at least one; a destination may be anything but the source itself.
_Avoid_: Target (a word avoided for a **selection**), selected clip, original, copy

**Source picking**:
The first step of **duplicate mode**, in which the user chooses the **source**: the **launch buttons** that hold a **clip** flash in its **colour**, and the **scene buttons** of scenes that hold one flash fuchsia; the others are **unlit**, and a click on them does nothing.
_Avoid_: Clip picking (that is the first step of setup mode)

**Destination picking**:
The second step of **duplicate mode**, in which the user chooses the **destination**. With a launch button as the source, it is steadily **vibrant** in its clip's **colour**, the other launch buttons flash, in their clip's colour or in white if they are **empty**, and the **scene buttons** are unlit. With a scene as the source, its scene button is steadily vibrant fuchsia, the others flash fuchsia, and the launch buttons are unlit. A click on the source, or on anything unlit, does nothing.

**Back button**:
The button above the first **track** that goes back one step of **setup mode**: from **outro editing** to **intro editing**, from **intro editing** to **landing editing**, from **landing editing** to **fill-in editing**, from **fill-in editing** to **clip editing**, from **clip editing** to **clip picking**, and from clip picking to **launch mode**, which in clip picking is what the **setup button** does as well. In **clear mode** and **duplicate mode** it returns to launch mode. It is dark in launch mode, where a click does nothing, and otherwise vibrant in the **mode colour** of the step it goes back to: white in clip picking, clear mode and duplicate mode, green in clip editing, yellow in **fill-in editing**, orange in **landing editing**, blue in **intro editing**, pink in **outro editing**. Holding it, that is keeping it down for 200 milliseconds or more, goes straight back to clip picking from fill-in editing, landing editing, intro editing and outro editing, so that the user can pick another launch button; elsewhere a hold does what a click does. So that a hold can be told from a click, the back button and the **forward button** act when they are let go, in whatever step Launchnik is in by then; every other button acts when it goes down. The same holds for the back button of a control surface.
_Avoid_: Arrow button, left arrow, return button, long press, long click

**Clip picking**:
The first step of **setup mode**, in which every **launch button** flashes (in its **clip's** **colour**, or in white if it is **empty**) and the user chooses the one to set up. The **scene buttons** flash as well, the one that stands for the current **length** in green and the others in white, and the user may choose a length instead.
_Avoid_: Clip selection (a **selection** is what a track has chosen to play)

**Clip editing**:
The second step of **setup mode**, in which the **launch buttons** stand for the settings of the one picked launch button: whether it holds a **clip**, the clip's **colour** and its pattern. From here the **forward button** leads on to **fill-in editing**.
_Avoid_: Edit mode

**Fill-in editing**:
The third step of **setup mode**, in which the **launch buttons** stand for the **fill-in** of the launch button picked in **clip picking**: whether its clip holds one, the fill-in's **colour** and its pattern, laid out as in **clip editing**. The **back button** returns to clip editing; the **forward button** leads on to **landing editing**, whether or not the clip holds a fill-in.
_Avoid_: Fill-in mode, fill-in setup

**Landing editing**:
The fourth step of **setup mode**, in which the **launch buttons** stand for the **landing** of the launch button picked in **clip picking**: whether its clip holds one, the landing's **colour** and its pattern, laid out as in **clip editing**. The **back button** returns to fill-in editing; the **forward button** leads on to **intro editing**, whether or not the clip holds a landing.
_Avoid_: Landing mode, landing setup, set landing (that is only the caption of the **forward button**)

**Intro editing**:
The fifth step of **setup mode**, in which the **launch buttons** stand for the **intro** of the launch button picked in **clip picking**: whether its clip holds one, the intro's **colour** and its pattern, laid out as in **clip editing**. The **back button** returns to landing editing; the **forward button** leads on to **outro editing**, whether or not the clip holds an intro.
_Avoid_: Intro mode, intro setup, set intro (that is only the caption of the **forward button** and **back button**)

**Outro editing**:
The sixth and last step of **setup mode**, in which the **launch buttons** stand for the **outro** of the launch button picked in **clip picking**: whether its clip holds one, the outro's **colour** and its pattern, laid out as in **clip editing**. The **back button** returns to intro editing.
_Avoid_: Outro mode, outro setup, set outro (that is only the caption of the **forward button**)

**Light**:
What a **launch button** is showing at the moment, in either mode: **unlit**, or one **colour** that is **dim**, **vibrant** or flashing. A **function button** has a light too, in the colour of the **function**, and so has a **function select button**, and so has a **scene button**, as part of the **interval meter**. The front panel and a control surface show the same light for the same button.
_Avoid_: Lamp, LED, look, state (a state is **off**, **queued** or **active**)

**Label**:
The caption printed below a **launch button** or **scene button**, or beside the **back button** or **forward button**, that tells what the button does in **setup mode** (a colour, a pattern, a **length**, set/clear, the step it leads to). The back button has one line for each step it can go back to, to its left, and the forward button one for each step it can go forward to, to its right; only the line for the step it leads to now is lit, in that step's mode colour. Every label is printed the same in every mode, but like a **light** it is either lit or **unlit**: lit while it applies, in the **mode colour**, in white or in the colour of its button, and barely visible dark grey otherwise. The captions SETUP, CLEAR, DUPLICATE, FILL-IN, MUTE, SOLO and STOP are not labels: SETUP belongs to the **setup button**, CLEAR belongs to the **clear button**, DUPLICATE to the **duplicate button**, FILL-IN to the **fill-in button**, and MUTE, SOLO and STOP to the **function select buttons**. Nor are **track names**, **clip names**, **scene names** and **CV output names**, which the user writes.
_Avoid_: Caption, legend, text, tag

**Logo**:
The Launchnik logo at the top right of the front panel and the folded front panel, which shows the **mode colour**. Double-clicking it gives every **track name**, **clip name**, **scene name** and **CV output name** that still shows its default a single underscore instead, in one step of the undo history; names the user typed are left as they are. The underscore is then a name like any other: it is part of the **patch**, and clearing it gives the default back. Nothing on the panel points to this.

**Mode colour**:
The colour that shows which way of working is on, worn by the logo and by the lit **labels**: white for **launch mode**, green for **clip picking**, yellow for **clip editing**, orange for **fill-in editing**, blue for **landing editing**, pink for **intro editing**, turquoise for **outro editing**, red for **clear mode**, fuchsia for **duplicate mode**.
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
