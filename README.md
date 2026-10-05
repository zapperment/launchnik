# Launchnik

Launchnik is a utility Rack Extension for Reason that launches Player patterns the way clips are launched in the session view of Ableton Live. It sends pattern-selecting CV to Combinators that wrap Player devices, and changes patterns only on four-bar boundaries so that everything stays in time.

Version 1.0.0d3, a development build. Requires Reason 14 or later.

## The panel

The front panel has eight **tracks**, one per column. Each track has eight **launch buttons**, one per pattern, and a **stop button** underneath. The back panel has eight CV outputs, one per track.

A button is in one of three states:

| State | Light | Meaning |
|---|---|---|
| Off | Dark | Nothing pending. |
| Queued | Flashing on the beat | Clicked, waiting for the next switch point. |
| Active | Lit | The track is emitting this button's CV value. |

A track always has exactly one active button and at most one queued button. A new device starts with all stop buttons active.

## Setting up a track

1. Put a Player device (for example PolyStep Sequencer) and its instrument in a Combinator.
2. Connect one of Launchnik's CV outputs to a CV input on the back of the Combinator.
3. In the Combinator's editor, add a mapping with that CV input as the source and the Player's pattern selection as the target.
4. Set the mapping to Min = Off, Max = 8, and leave the source range at 0 %–100 %.

The launch button in row N of that track now selects pattern N, and the stop button selects Off.

## How launching works

- The song is divided into **switch intervals** of four bars, counted from the start of the song. The moment one ends and the next begins is a **switch point** (bars 1, 5, 9, 13 and so on in 4/4).
- **While the transport is running**, a click queues the button. At the next switch point it becomes active, the previously active button on the track goes off, and the CV changes.
- **While the transport is stopped**, a click takes effect at once.
- Clicking another button on the same track before the switch point replaces the queued one. Clicking the queued button again does nothing. Clicking the active button cancels whatever is queued.
- When the transport stops, queued buttons become active immediately and the CV outputs keep their values.
- Each track's choice is saved with the song. On reopening, it is active straight away.

## Automation and Remote

Each track has one automatable parameter, "Track N Selection", with nine values: Stop and Pattern 1–8. Automated changes behave like clicks. A change that falls exactly on a switch point takes effect there, not four bars later.

For control surfaces, Launchnik provides these Remote items:

| Remote item | Direction | Values |
|---|---|---|
| `Track N Selection` (8 items) | Input and output | 0 = stop, 1–8 = pattern |
| `Track N Pattern M` (64 items) | Output only | 0 = off, 1 = queued, 2 = active |
| `Track N Stop` (8 items) | Output only | 0 = off, 1 = queued, 2 = active |

The output-only items are meant for lighting pads. They can be used from a Remote map, but they do not appear in "Edit Remote Override Mapping". A codec has to translate a pad press into a selection value for the pad's track; see [the decision record](docs/adr/0001-selection-in-state-out.md) for why.

Launchnik does not respond to MIDI CC messages.

## Known limitations

- The switch interval is fixed at four bars.
- Switch points are worked out from the current time signature. In a song whose time signature changes part-way through, they may not line up with the bar numbers in the sequencer.
- Only playing through a switch point fires a queued button. Dragging the song position past one does not, and a loop that neither contains nor ends on a switch point keeps a button queued indefinitely.
- There is no scene launch and no stop-all button.

## Building

From this directory:

```bash
python3 build45.py local45 Testing
```

This compiles the device and installs it for Reason Recon. Restart Recon to pick up a new build.

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

The switching rules can be tested from the command line, without Reason:

```bash
./tests/run.sh
```

The panel, the CV output into a Combinator, automation and Remote can only be tested in Reason Recon.

## Layout of the code

| Path | Contents |
|---|---|
| `Launcher.h`, `Launcher.cpp` | The switching rules. Plain C++ with no dependency on the SDK. |
| `Launchnik.h`, `Launchnik.cpp` | Connects the rules to the host: reads selections and the transport, writes states, lamps and CV. |
| `JukeBoxExports.cpp` | Entry points the host calls. |
| `motherboard_def.lua` | Properties, Remote items, automation and CV outputs. |
| `realtime_controller.lua` | Which property changes are passed to the realtime code. |
| `GUI2D/` | Panel layout and source graphics. |
| `Resources/English/` | Texts shown by the host. |
| `tests/` | Command-line tests for the switching rules. |
| `CONTEXT.md` | Glossary of the terms used in the code and in this document. |
| `docs/adr/` | Records of design decisions. |
