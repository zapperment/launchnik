# Launchnik

A utility rack extension for Reason that launches Player patterns in the manner of a clip launcher, by sending pattern-selecting CV to Combinators that wrap Player devices.

## Language

**Launch button**:
One of the 64 large buttons on the front panel, arranged in eight rows on each of eight **tracks**; it either holds a **clip** or is **empty**.
_Avoid_: Clip button, pad, cell

**Clip**:
What a **launch button** holds: a **colour** and, for each of its track's six **CV outputs**, a **pattern**, a **unipolar value**, a **bipolar value** and a **transition**. Launching the clip makes each CV output emit whichever of the three its **CV mode** picks. A launch button keeps all of them while it is **empty**, so a clip comes back as it was; a new clip has, on every CV output, the pattern of its row, unipolar value 0, bipolar value 0 and transition off. Several clips on a track may use the same pattern. A clip may also hold a **fill-in**.
_Avoid_: Slot, pattern (for the clip itself); a **scene** is a row of launch buttons, not a clip

**Clip name**:
Text that the user gives a **launch button** so that its **clip** can be recognised. It is shown above the launch button and changed by double-clicking it. A launch button keeps and shows its clip name while it is **empty**, like its clip's other settings, so the name comes back with the clip. Like a **track name**, it shows about 12 characters. It is never shown empty: clearing it gives the launch button its default name back, Clip 1 to Clip 8 by **scene**, which is also what a new device shows. A **fill-in** has no name of its own. It is part of the **patch**, cannot be automated, and is not available to a control surface. It is shown in **launch mode** and **clip picking**; in **clip editing** and **fill-in editing** only the clip name of the launch button being set up is shown.
_Avoid_: Clip label, launch button name, title

**Fill-in**:
A second **colour** and, for each **CV output**, a second pattern, unipolar value, bipolar value and **transition** that a **clip** may hold, emitted instead of the clip's own while a fill-in is **playing** and the clip is **active**. It lies over the clip: underneath, the clip goes on as if there were no fill-in, its own transition included, and that is what the CV outputs emit again once the fill-in is cleared or over; at the **switch point** they jump, without a transition, to the clip's value or the queued clip's. A fill-in's transition is the exception to the rule that the active clip's transition counts: it moves from what is emitted at the press to the fill-in's value, landing on it at the switch point, like a riser; off jumps at the press. A launch button keeps its fill-in while it is **empty**, so that it comes back with the clip; an empty launch button has no fill-in to play. A new fill-in starts as a copy of its clip's patterns and values, with every transition off and the colour white, each time it is switched on. Fill-ins are part of the **patch**.
_Avoid_: Fill, drum fill, fill-in mode (it is not a way of working like **launch mode** or **setup mode**)

**Fill-in button**:
The button to the left of **scene** 6 that triggers a fill-in: from the press until the next **switch point**, every track whose **active** clip holds a fill-in plays it, and its active launch button flashes in the fill-in's colour. Tracks without one, and stopped tracks, go on as before. It works in every mode; a press while a fill-in is already playing, or while the **length** is **immediate**, does nothing. Its **light** is dim white, and vibrant red while a fill-in is playing, whether or not any track has one to play. Its caption FILL-IN is white, and red while a fill-in is playing. It can be automated.
_Avoid_: Fill button, trigger

**Playing** (of a fill-in):
Said of a fill-in from the press of the **fill-in button** until the next **switch point**. A change to a fill-in, or to whether the active clip holds one, takes effect at once while a fill-in is playing. With the transport stopped it goes on playing until the song reaches the switch point. At the switch point every track goes on with its **selection**, as it would have anyway.

**Set fill-in button**:
The button to the right of the **back button** that goes from **clip editing** to **fill-in editing**. It is lit in vibrant red, with its label, only in clip editing of a launch button that holds a clip; otherwise it is dark and a click does nothing. It cannot be automated.
_Avoid_: Setup fill-in button, fill-in edit button

**Empty**:
Said of a **launch button** that holds no **clip**. A new device has only empty launch buttons. Choosing an empty launch button counts as choosing **stop**.
_Avoid_: Inactive, disabled, cleared

**Colour**:
One of fourteen hues that mark a **clip** so that it can be recognised: white, red, orange, yellow, lime, green, turquoise, cyan, light blue, blue, dark blue, purple, fuchsia, pink (numbered 1 to 14 in that order). It has no effect on what the clip does. A new clip has the colour of its track: red, orange, yellow, green, cyan, blue, purple, pink for tracks 1 to 8.
_Avoid_: Tint, label

**Patch**:
What all 64 **launch buttons** hold (their **clip names** included), together with the **length** of the switch interval, the **CV mode** of every **CV output**, the **track names** and the **scene names**, saved as a file that can be loaded into any Launchnik, in any song. It does not hold what the tracks are playing: loading a patch changes no **selection**. A new device has the patch in which every launch button is **empty**.
_Avoid_: Template, preset, layout

**Unlit**, **dim**, **vibrant**:
The three ways a **launch button** can look: showing no **colour** at all, showing a colour at low brightness, or showing it at full brightness. A flashing launch button alternates between dim and vibrant. A **function button** looks the same three ways, in the colour of the **function**.
_Avoid_: Inactive (for unlit), bright, lit

**Stop**:
The **selection** that stands for the Player's pattern selection being "off": the track's pattern outputs emit no pattern (0), while its unipolar and bipolar outputs go on emitting the value they last emitted. Choosing an **empty** launch button counts as choosing stop.
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
Text that the user gives a **track** so that it can be recognised, such as the instrument it plays. It is shown above the track on the front panel and above its **CV outputs** on the back panel, the same on both, and changed by double-clicking either. The panel shows about 12 characters, centred; a longer name is kept in full but shown cut off at both ends. It is never shown empty: clearing it gives the track its default name back, Track 1 to Track 8, which is also what a new device shows. It is part of the **patch**, cannot be automated, and is not available to a control surface. On the front panel it is shown in **launch mode** and **clip picking**; in **clip editing** and **fill-in editing** only the track name of the launch button being set up is shown. On the back panel it is always shown.
_Avoid_: Track label, track title, caption

**Scene**:
One row of the front panel: the eight **launch buttons** at the same height on the eight **tracks**. Scenes are numbered 1 to 8 from the top.
_Avoid_: Row (in prose), line

**Scene name**:
Text that the user gives a **scene** so that it can be recognised. It is shown above the **scene button** and changed by double-clicking it; it renames no **clip** of the scene. Like a **track name**, it shows about 12 characters. It is never shown empty: clearing it gives the scene its default name back, Scene 1 to Scene 8, which is also what a new device shows. It is part of the **patch**, cannot be automated, and is not available to a control surface. It is shown only in **launch mode**, since in **clip picking** the scene buttons stand for **lengths**.
_Avoid_: Scene label, row name

**Scene button**:
The button to the right of a **scene**. In **launch mode**, clicking it means clicking all eight **launch buttons** of its scene at once, so a track whose launch button in that scene is **empty** stops. In **clip picking** the eight scene buttons stand for the eight **lengths** instead, immediate at the bottom and sixteen bars at the top: clicking one chooses that length and returns to launch mode. In **clip editing** a click does nothing. It never shows which scene was last chosen; outside clip picking its **light** belongs to the **interval meter**.
_Avoid_: Scene launch button, row button

**CV output**:
One of the six jacks of a **track** on the back panel, lettered A to F, each driven by one row of settings in every **clip** of the track. Written "output A" in full, never a bare "A", since **transitions** are lettered too.
_Avoid_: Jack, CV out, lane, channel

**CV mode**:
What a **CV output** emits, chosen with a three-way switch beside it on the back panel: **pattern**, **unipolar** or **bipolar**. It is part of the **patch**, cannot be automated, and can only be changed on the back panel, not from a control surface; a change takes effect at once, dropping any transition under way. A new device has output A in pattern mode and outputs B to F in unipolar mode.
_Avoid_: Polarity, output mode, type

**Pattern** (of a CV output):
In pattern mode, which of the eight patterns of a Player a **clip** selects; the CV output emits it in the steps a Combinator reads as pattern selection, and 0 while the track is stopped, **muted** or **solo-muted**.

**Unipolar value**, **bipolar value**:
In unipolar mode, one of seven evenly spaced values from 0 to 1 (labelled 0 to 100); in bipolar mode, one of seven from −1 to +1 (labelled −100 to +100, 0 in the middle). A stopped track goes on emitting the value it last emitted; before any clip has played, that is 0.
_Avoid_: Level, amount, CV value (when the mode matters)

**Transition**:
How a CV output in unipolar or bipolar mode moves from the value of the **active** clip to that of a **queued** clip on its track: **off**, A, B, C or D. It is the active clip's transition that counts, not the queued one's (a fill-in is the exception). Off keeps the active clip's value until the **switch point** and then jumps. A and B move from the moment of queueing to the switch point, A in a straight line and B along an exponential curve that starts slowly; C and D do the same as A and B, but only within the last bar before the switch point (so with a **length** of one bar or less, they behave like A and B). The movement follows the song position and lands on the new value at the switch point. A queued **stop**, a track that is stopped, and the length **immediate** never move: the value jumps. Whenever the queued clip changes, its value is edited or the active clip's transition is changed, the movement starts over from the value emitted at that moment; when nothing is left to move towards (the active clip chosen again, stop queued, transition switched off), the value jumps back to the active clip's.
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
The button to the left of the **function buttons** that switches between **launch mode** and **setup mode**.

**Back button**:
The button above the first **track** that goes back one step of **setup mode**: from **fill-in editing** to **clip editing**, from **clip editing** to **clip picking**, and from clip picking to **launch mode**, which in clip picking is what the **setup button** does as well. It is dark in launch mode, where a click does nothing, and otherwise vibrant in the **mode colour** of the step it goes back to: white in clip picking, green in clip editing, yellow in **fill-in editing**.
_Avoid_: Arrow button, left arrow, return button

**Clip picking**:
The first step of **setup mode**, in which every **launch button** flashes (in its **clip's** **colour**, or in white if it is **empty**) and the user chooses the one to set up. The **scene buttons** flash as well, the one that stands for the current **length** in green and the others in white, and the user may choose a length instead.
_Avoid_: Clip selection (a **selection** is what a track has chosen to play)

**Clip editing**:
The second step of **setup mode**, in which the **launch buttons** stand for the settings of the one picked launch button: whether it holds a **clip**, the clip's **colour** and its pattern. From here the **set fill-in button** leads on to **fill-in editing**.
_Avoid_: Edit mode

**Fill-in editing**:
The third step of **setup mode**, in which the **launch buttons** stand for the **fill-in** of the launch button picked in **clip picking**: whether its clip holds one, the fill-in's **colour** and its pattern, laid out as in **clip editing**. The **back button** returns to clip editing.
_Avoid_: Fill-in mode, fill-in setup

**Light**:
What a **launch button** is showing at the moment, in either mode: **unlit**, or one **colour** that is **dim**, **vibrant** or flashing. A **function button** has a light too, in the colour of the **function**, and so has a **function select button**, and so has a **scene button**, as part of the **interval meter**. The front panel and a control surface show the same light for the same button.
_Avoid_: Lamp, LED, look, state (a state is **off**, **queued** or **active**)

**Label**:
The caption printed below a **launch button**, **scene button** or the **back button** that tells what the button does in **setup mode** (a colour, a pattern, a **length**, set/clear, back). Every label is printed the same in every mode, but like a **light** it is either lit or **unlit**: lit while it applies, in the **mode colour**, in white or in the colour of its button, and barely visible dark grey otherwise. The captions SETUP, FILL-IN, MUTE, SOLO and STOP are not labels: SETUP is always white, FILL-IN belongs to the **fill-in button**, and MUTE, SOLO and STOP to the **function select buttons**. Nor are **track names**, **clip names** and **scene names**, which the user writes.
_Avoid_: Caption, legend, text, tag

**Mode colour**:
The colour that shows which way of working is on, worn by the logo and by the lit **labels**: white for **launch mode**, green for **clip picking**, yellow for **clip editing**, red for **fill-in editing**.
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
