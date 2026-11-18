# No pattern lies above the quarter steps

A **CV output** of a **clip**, **fill-in**, **landing**, **intro** or **outro** may have **no pattern**, so that a Player can be silenced without giving up one of its eight patterns. Whether it has none is saved in the number that already holds the output's settings ([0008](0008-settings-and-switches-are-packed.md), [0010](0010-quarter-steps-lie-above-the-packed-settings.md)), above what was there: the old number, from 0 to 31,359, plus 31,360 if there is no pattern, 62,720 values in all. Underneath, the pattern is then written as pattern 1 and read as nothing. We chose this because a number saved before then reads exactly as it did, so songs, patches, the factory patches and the demo songs need no converting, because it adds no properties and no Remote items, and because a press that chooses or clears a pattern still writes one property.

## Considered options

- **A ninth pattern, packed afresh as 0 to 8.** Rejected: every saved number would mean something else, and everything saved so far would need converting.
- **One switch per CV output in the switches of the clip.** Rejected: choosing a pattern on an output with none would have to write two properties, and the switches would grow from 112 to 7,168 values for no gain.
- **Remembering the pattern to bring back.** Not needed: the pattern is chosen again by clicking its button.

## Consequences

- A codec that does not know about no pattern takes a number above 31,359 for one out of range. The Launchpad codecs change together with the device.
- From release on this packing is fixed like the rest of a patch ([0003](0003-patch-holds-clips-not-selections.md)).
