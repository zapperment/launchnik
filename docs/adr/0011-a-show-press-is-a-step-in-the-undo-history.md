# A show press is a step in the undo history

A show press makes a **clip** the **shown clip** without launching it. It changes nothing that is saved, yet each one is a step in the undo history, "Show clip", whose undo does nothing. We accept that, because the realtime code decides the shown clip and computes what the **big display** says about it, and the only properties a click can write that the realtime code can read are document-owned, and the host lists every click that writes one in its undo history.

## Considered options

- **A GUI-owned property, passed on by the realtime controller.** The host keeps GUI-owned changes out of the undo menu, and the documentation allows any custom property as the source of a binding. Rejected: with such a binding Reason Recon crashes when the device is added to the rack.
- **A performance property**, like a modulation wheel, which the host may leave out of the undo history. Not tried: the host can record it on the device's sequencer track, and a show press is not to be automatable.
- **Letting the big display decide the shown clip itself**, from a GUI-owned property. Rejected for now: the display would have to work out the value tables on its own and be given the **output graphs** of all eight tracks instead of one, with many more properties bound to it.

## Consequences

- A shift-click on a launch button that holds a clip adds one step to the undo history, and so does a show press from a control surface. Undoing it changes nothing.
- The show press is one property, held down like a relaunch press: the number of the launch button while it is held, 0 otherwise, so that showing the same clip twice is a change each time. It is not saved and has no automation lane.
- The Remote item "Show Clip" is fixed from release on, like every Remote item.
