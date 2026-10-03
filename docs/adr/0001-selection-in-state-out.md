# A saved selection per track goes in, a computed state per button comes out

A launch or stop button is not one property. Each track has a single **selection** (stop or pattern 1–8) that the user, automation and control surfaces write, and each of the 72 buttons has a separate **state** (off, queued, active) that the realtime code computes and nobody else can write. We chose this because the host only lets realtime code write realtime-owned properties, which are never saved, and only saves, automates and accepts input on document-owned properties, which realtime code cannot write; one property therefore cannot both take a click and report whether that click is still queued.

## Considered options

- **72 momentary buttons plus 72 states.** More literal for a control surface (one pad, one input item), but nothing would be saved with the song, and automation would need 72 lanes.
- **Selection only, no state items.** A control surface could not tell queued from active, which rules out flashing pads on a Launchpad.

## Consequences

- Reopening a song makes every track's selection active at once; a button that was queued when the song was saved comes back active.
- Remote exposes 8 input items ("Track N Selection", 0–8) and 72 output-only items ("Track N Pattern M", "Track N Stop", 0–2). A codec has to translate a pad press into a selection value for the pad's track.
- The state items can be used from a Remote map, but not through "Edit Remote Override Mapping", because they have no clickable widget. Confirmed with the LaunchnikSpike device in Reason Recon on 2026-10-03.
- A state item stays at 1 for as long as its button is queued; the flashing on the front panel is a separate lamp that is not exposed to Remote.
