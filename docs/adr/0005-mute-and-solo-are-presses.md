# Mute and solo are presses; the realtime code keeps the state

A track is **muted** or **soloed** from a click on its **function button** until the next **switch point**, or until a second click. The device therefore has no saved "muted" or "soloed" property. Each track has two momentary, automatable properties, "Track N Mute" and "Track N Solo", held down for as long as the button is, like the **fill-in button**. The realtime code switches the track's mute or solo on each press, keeps that state in realtime-owned properties, and clears it at every switch point. A press on a track with no clips does nothing, and a track that loses its last clip stops being muted or soloed. We chose this because the realtime code cannot write saved properties ([0001](0001-selection-in-state-out.md)): a saved on/off value could not clear itself at the switch point, and would go on claiming a mute that had already ended.

The **function** (mute, solo or stop) only decides which of these properties a click on a function button writes: the panel's click handler and every codec look it up before they write. It is not saved, not part of the **patch** and not automatable, so a recorded session plays back the same whatever function is chosen during playback.

## Considered options

- **A saved boolean per track for mute and for solo.** Automation would record states rather than toggles, which is more robust. Rejected: nothing could clear it at the switch point; the panel and the codecs would have to do it, and they only run when the user presses something.
- **One automatable "function press" per track, with the function automated too.** Fewer properties, but a click on MUTE during playback would change what every recorded press does.

## Consequences

- A song always opens with nothing muted or soloed, and with the function stop.
- Each mute or solo press is one step in the host's undo history, like a fill-in press.
- Because a press is a toggle, the mute and solo state depends on every press since the last switch point; with the **length** **immediate**, since the song started. To keep playback from part-way through a song predictable, the realtime code clears all mutes and solos whenever the song position jumps: a locate, a loop going back, or a start of playback away from where the transport stopped.
- Remote gains 16 input items for the presses and one input and output item, "Function", for the function select buttons. The items "Track N Stop" become "Track N Function"; their values (unlit, dim, vibrant, flashing) keep their meaning, and the colour comes from the function.
