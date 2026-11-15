# The realtime code records the output graphs, for every track

The **output graphs** of the **big display** show what the six **CV outputs** of one track emitted over the last two **switch intervals**. The realtime code records that for all 48 CV outputs, all the time, at the resolution of the graph for the current **length**, and hands the display the record of the **shown clip**'s track, together with the values emitted at the moment, as one realtime-owned string property. The display itself keeps nothing: it draws what the string holds.

We chose this because a display script is redrawn from the current values of its properties and has no memory, and because the graph should already be there when the shown clip moves to another track, which is what most launches do. A record kept on the display's side could only cover what the display was showing, and only while the panel was visible.

## Considered options

- **The display records what it shows** (a display written in C++, with memory of its own and a periodical notification). Rejected: the graph would start empty with every launch on another track, the notification only runs while the panel is visible, so folding the device or scrolling it out of view would leave gaps, and its period follows the frame rate, not the song position.
- **One number property per pixel column.** Rejected: about 700 more properties, against [0008](0008-settings-and-switches-are-packed.md).
- **Recording at the finest resolution, so that a change of length rescales the graph.** Rejected: 32 bars at the resolution of two is sixteen times the memory and work, for a change that is rare; the graph starts empty instead.

## Consequences

- The rules of what is recorded, and when the record moves on, belong to `CLauncher`, so that they can be tested without the SDK: it moves with the song position, not with time.
- A realtime string may be written at most once every 25 milliseconds, so the display follows the outputs at no more than 40 updates a second, and a graph cannot be finer than the string is long (8,192 bytes at most).
- The record is not saved: after a song is loaded every graph starts empty.
- What the **value table** shows as emitted comes from the same string, so that numbers and graph never disagree.
