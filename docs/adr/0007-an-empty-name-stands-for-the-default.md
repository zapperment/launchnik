# An empty name stands for the default

**Track names**, **clip names** and **scene names** are stored as text the user typed, and as empty text otherwise. Nothing ever stores a default name such as "Track 1", "Clip 3" or "Scene 8": while a name is empty, the panel shows its default in its place. Clearing a name and confirming with Return therefore gives it back its default.

We chose this because the host leaves no way to repair a name after the user has typed it. Names are typed into the host's own text field, which stores whatever it is given, empty text included, and has no setting for a minimum or maximum length. Realtime code cannot write saved properties, and control surfaces have no access to names, so nothing inside the device could put "Track 1" back once an empty name had been stored.

## Considered options

- **Refuse empty text.** What the user first asked for, so that a name could never vanish and leave nothing to double-click. The host's text field cannot be told to refuse it.
- **Store the defaults.** A new device would store "Track 1" to "Track 8" and so on. An empty name would then be shown as nothing at all, and there would be no way back to the default except typing it.

## Consequences

- Each name is shown by two text widgets bound to the same property, on top of each other: one shows the name and is visible while it is not empty, the other shows the fixed default and is visible while it is empty. Both open the host's text field on a double click.
- Patches saved before version 1.0.0d14 carry no names, so they load with every name empty, which is to say with the default names.
- A name typed to read exactly like its default ("Track 3" on track 3) is stored as text, and looks the same as the default. Should the defaults ever change, such a name would stay as typed while empty names followed the new defaults. Once patches with names have been shared, the defaults should be treated as fixed.
- The default names live in the text resources, not in the saved properties.
