# A patch holds the clips, not the selections

A **patch** carries what the 64 launch buttons hold: whether each holds a **clip**, and the clip's pattern and **colour**. It does not carry the **selection** of any track. Selections stay saved with the song and automatable, but are marked as song-only so that the host leaves them out of patch files and leaves them alone when a patch is loaded.

We chose this because a patch is meant as a reusable layout of the grid that is carried from song to song. What the tracks are playing belongs to the song and its automation; a layout that also changed what plays could not be loaded into a running song without disturbing it.

## Considered options

- **Clips and selections.** This is what the host does when nothing is said: every saved property goes into both songs and patches. Loading a patch would then set all eight tracks to whatever was selected when the patch was saved, usually the stop buttons, and would stop the music.

## Consequences

- Loading a patch behaves like editing many clips at once in **setup mode**, and needs no rules of its own. A changed pattern under an **active** launch button waits for the next **switch point**; a selection that now points at an **empty** launch button counts as the stop button, so that track stops at the switch point.
- A patch carries pattern and colour for empty launch buttons too, because the device keeps them so that a clip comes back as it was. Loading a patch replaces those remembered values.
- Loading a patch, or stepping to the previous or next one, replaces the clips without asking. That is the host's behaviour for every device, and one step in its undo history.
- The setup state is not saved at all, so a load during setup mode leaves the device in setup mode, showing the loaded clips.
- Once patches have been shared, adding selections to them would make old patches incomplete and new ones disruptive. Treat the contents of a patch as fixed.
