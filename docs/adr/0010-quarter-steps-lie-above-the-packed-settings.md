# Quarter steps lie above the packed settings

A **unipolar value** and a **bipolar value** can each be raised by one, two or three **quarter steps**. They are saved in the number that already holds the four settings of a **CV output** ([0008](0008-settings-and-switches-are-packed.md)), above what was there: the old number, from 0 to 1,959, plus 1,960 times the quarter steps of the unipolar value, plus 7,840 times those of the bipolar value, 31,360 values in all. We chose this because a number saved before there were quarter steps then reads exactly as it did, so songs, patches, the factory patches and the demo songs need no converting, and because it adds no properties and no Remote items, whose count decides how long the device and a codec take to load.

## Considered options

- **A property of its own per CV output for the quarter steps.** Rejected: 1,920 more properties and as many Remote items, which is what [0008](0008-settings-and-switches-are-packed.md) packed the settings to get away from.
- **Packing afresh, with 25 values for each of the two modes** (25,000 values). Rejected: a little smaller, since the top value has no quarter steps, but every saved number would mean something else, and everything saved so far would need converting.
- **Finer steps on the buttons, or a control of their own.** Not considered further: the grid has seven buttons per output, and everything must be possible from a Launchpad.

## Consequences

- The top value with quarter steps (100 or +100 raised) can be written as a number but means nothing: it reads as the top value.
- A codec that does not know about quarter steps still works, but takes a number above 1,959 for one out of range, and writes a value back without its quarter steps whenever it changes another setting of the output. The Launchpad Pro [MK3] codec changes together with the device.
- Inside the code a value is counted in quarter steps, 0 to 24, and only the packing and the lights know about steps.
- From release on this packing is fixed like the rest of a patch ([0003](0003-patch-holds-clips-not-selections.md)).
