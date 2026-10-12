# Launchnik

Launchnik is a utility Rack Extension for Reason that launches Player patterns the way clips are launched in the session view of Ableton Live. It sends pattern-selecting CV to Combinators that wrap Player devices, and changes patterns only on four-bar boundaries so that everything stays in time. Which pattern each launch button plays, and in which colour it shows, is set up on the panel itself.

Version 1.0.0d5, a development build. Requires Reason 14 or later.

## The panel

The front panel has eight **tracks**, one per column. Each track has eight **launch buttons** and a **stop button** underneath. To the left of the stop buttons is the **setup button**. The back panel has eight CV outputs, one per track.

A launch button either holds a **clip** or is **empty**. A clip is one of the Player's patterns 1–8 together with one of fourteen colours. A new device has only empty launch buttons.

In **launch mode**, the normal way of working, a button is in one of three states:

| State | Launch button with a clip | Meaning |
|---|---|---|
| Off | Dim, in the clip's colour | Nothing pending. |
| Queued | Flashing on the beat | Clicked, waiting for the next switch point. |
| Active | Vibrant | The track is emitting this button's CV value. |

An empty launch button is unlit, and clicking it has the same effect as clicking the track's stop button. A track always has exactly one active button and at most one queued button. A new device starts with all stop buttons active.

A stop button shows whether there is anything to stop, as in Ableton Live:

| Stop button | Meaning |
|---|---|
| Unlit | The track has no clips. |
| Dim red | The track is stopped. |
| Vibrant red | A clip is active; clicking stops it. |
| Flashing | Clicked, the track stops at the next switch point. |

## Connecting a track

1. Put a Player device (for example PolyStep Sequencer) and its instrument in a Combinator.
2. Connect one of Launchnik's CV outputs to a CV input on the back of the Combinator.
3. In the Combinator's editor, add a mapping with that CV input as the source and the Player's pattern selection as the target.
4. Set the mapping to Min = Off, Max = 8, and leave the source range at 0 %–100 %.

A clip with pattern N on that track now selects pattern N, and the stop button selects Off.

## Setting up clips

Click the setup button to enter **setup mode**. Tracks go on playing and the stop buttons go on working; only launching from the grid is unavailable.

1. **Clip picking.** Every launch button flashes: in its clip's colour, or in white if it is empty. Click the one you want to set up.
2. **Clip editing.** The grid now shows the settings of that one launch button:

| Where | What it does |
|---|---|
| Row 1, column 8 | Switches the clip on and off: vibrant green with a clip, dim red without. |
| Rows 2 and 3, columns 1–7 | The colour: white, red, orange, yellow, lime, green, turquoise in row 2; cyan, light blue, blue, dark blue, purple, fuchsia, pink in row 3. The chosen one is vibrant. |
| Row 5 | The pattern, 1–8, in pink. The chosen one is vibrant. |
| Row 8, column 8 | Dim blue: back to clip picking. |

The colour and pattern rows only show while the launch button holds a clip. A new clip plays the pattern of its row and has the colour of its track: red, orange, yellow, green, cyan, blue, purple, pink for tracks 1 to 8. A clip that is switched off and on again comes back with its pattern and colour.

Click the setup button again to return to launch mode, from either step.

Changing the pattern of the clip that is playing takes effect at the next switch point, and its launch button flashes until then; switching that clip off stops its track at the next switch point. While the transport is stopped, both happen at once.

Clips are saved with the song, and each change is a step in the undo history. Setup mode itself is not saved: a song always opens in launch mode.

## Patches

A **patch** is what all 64 launch buttons hold, saved as a file, so that the same set of clips can be used in any song. The header of the panel, folded or not, shows the name of the patch and Reason's buttons for stepping through patches, browsing and saving.

A patch holds the clips only. Loading one does not change what any track has selected, so it can be loaded while the song plays: a clip whose pattern changes switches at the next switch point, and a track whose selected launch button is now empty stops there.

Launchnik comes with two patches: "Init Patch", in which every launch button is empty and which a new device starts with, and "All Patterns", in which every launch button holds a clip, with patterns 1–8 in rows 1–8 and each track in its own colour.

## How launching works

- The song is divided into **switch intervals** of four bars, counted from the start of the song. The moment one ends and the next begins is a **switch point** (bars 1, 5, 9, 13 and so on in 4/4).
- **While the transport is running**, a click queues the button. At the next switch point it becomes active, the previously active button on the track goes off, and the CV changes.
- **While the transport is stopped**, a click takes effect at once.
- Clicking another button on the same track before the switch point replaces the queued one. Clicking the queued button again does nothing. Clicking the active button cancels whatever is queued.
- When the transport stops, queued buttons become active immediately and the CV outputs keep their values.
- Each track's choice is saved with the song. On reopening, it is active straight away.

## Automation and Remote

Each track has one automatable parameter, "Track N Selection", with nine values: Stop and Clip 1–8, the clip being the one in that row. Automated changes behave like clicks. A change that falls exactly on a switch point takes effect there, not four bars later. A selection that names an empty launch button counts as Stop.

Clips and setup mode cannot be automated.

For control surfaces, Launchnik provides these Remote items (N is the track, M the row, both 1–8):

| Remote item | Direction | Values |
|---|---|---|
| `Track N Selection` (8 items) | Input and output | 0 = stop, 1–8 = clip in that row |
| `Track N Stop Light` (8 items) | Output only | 0 = unlit, 1 = dim, 2 = vibrant, 3 = flashing |
| `Track N Clip M Light` (64 items) | Output only | 0 = unlit, then three values per colour: dim, vibrant, flashing (1–3 white, 4–6 red, … 40–42 pink) |
| `Track N Clip M Present` (64 items) | Input and output | 0 = empty, 1 = holds a clip |
| `Track N Clip M Pattern` (64 items) | Input and output | 0–7 for patterns 1–8 |
| `Track N Clip M Colour` (64 items) | Input and output | 0–13 for white to pink |
| `Setup` | Input and output | 0 = launch mode, 1 = clip picking, 2–65 = clip editing for launch button 1–64, counted down track 1 first |

A light is what the launch button shows on the panel, in launch mode and in setup mode alike, so a surface that shows the lights mirrors the panel. "Flashing" is one steady value; the surface flashes by itself.

The output-only items can be used from a Remote map, but they do not appear in "Edit Remote Override Mapping". A codec has to translate a pad press into the property change the same press makes on the panel; see the decision records [0001](docs/adr/0001-selection-in-state-out.md) and [0002](docs/adr/0002-realtime-computes-lights-writers-change-clips.md) for why, and `Press.h` for the rules.

Launchnik does not respond to MIDI CC messages.

## Known limitations

- The switch interval is fixed at four bars.
- Switch points are worked out from the current time signature. In a song whose time signature changes part-way through, they may not line up with the bar numbers in the sequencer.
- Only playing through a switch point fires a queued button. Dragging the song position past one does not, and a loop that neither contains nor ends on a switch point keeps a button queued indefinitely.
- Clip editing does not show which launch button is being edited.
- A clip that was switched off while its track's selection still named it is queued again as soon as it is switched back on.
- Launch buttons have no context menu for automation or Remote; use a stop button's menu to reach the track's selection.
- There is no scene launch and no stop-all button.
- Patches cannot be loaded or saved from a control surface.
- Only one patch besides the empty one is included; the Shop expects more.

## Building

From this directory:

```bash
python3 build45.py local45 Testing
```

This compiles the device and installs it for Reason Recon. Restart Recon to pick up a new build.

The film strip of the launch buttons is built from the source graphics in `RE2D_Launchnik_Graphics`, that of the stop buttons from the stock button image (both need ImageMagick):

```bash
./tools/make-launch-button-strip.sh
```

```bash
./tools/make-stop-button-strip.sh
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
| `Launchnik.h`, `Launchnik.cpp` | Connects the rules to the host: reads selections, clips, the setup state and the transport, writes states, lights, lamps and CV. |
| `Panel.h`, `Panel.cpp` | The click handler of the launch buttons and the setup button. |
| `Tags.h` | The property tags shared by the two. |
| `JukeBoxExports.cpp` | Entry points the host calls. |
| `motherboard_def.lua` | Properties, Remote items, automation and CV outputs. |
| `realtime_controller.lua` | Which property changes are passed to the realtime code. |
| `GUI2D/` | Panel layout and the graphics it uses. |
| `RE2D_Launchnik_Graphics/` | Source graphics that are not stock images. |
| `tools/` | Scripts that build the film strips of the launch and stop buttons. |
| `Resources/English/` | Texts shown by the host. |
| `Resources/Public/` | The patches that come with the device. |
| `tests/` | Command-line tests for the rules. |
| `CONTEXT.md` | Glossary of the terms used in the code and in this document. |
| `docs/adr/` | Records of design decisions. |
