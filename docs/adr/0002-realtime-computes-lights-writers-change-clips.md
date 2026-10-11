# Realtime code works out every light; the panel and control surfaces change clips themselves

Launchnik has to be fully usable from a control surface, **setup mode** included. What a launch button holds (clip or not, pattern, colour), whether setup mode is on and which launch button has been picked are all document-owned properties. The realtime code reads them and computes one **light** per launch button for both modes; the front panel and Remote both show that light and compute nothing themselves. In the other direction, the rule "pressing this launch button in the current mode changes that property" is implemented by each writer: the panel's click handler, and every codec.

We chose this because the host leaves no alternative for input. Only two parties may write a saved property: a click handler on the panel, and a control surface through Remote. Realtime code cannot, and a display's notify callback cannot either, so nothing inside the device can turn a pad press that arrives over Remote into a change of a clip.

## Considered options

- **64 "launch button pressed" Remote items, interpreted by the device.** One rule in one place, and the simplest possible codec, but impossible for the reason above.
- **Setup state as GUI-owned properties.** Keeps "Setup on" and "pick clip" out of the undo history. Realtime code cannot read GUI-owned properties, so the lights of setup mode would have to be worked out a second time in the panel's draw code and a third time in each codec.
- **Setup from the panel only.** Rejected: the Launchpad is meant to replace the panel, not accompany it.

## Consequences

- Switching setup mode on or off and picking a clip are steps in the host's undo history. The setup state is not saved: a song always opens in launch mode.
- What each launch button shows is decided in one place, in code that has no dependency on the SDK and is covered by the command-line tests.
- The press rule exists once per writer, in different languages and repositories. They have to be kept in step by hand; the same table of cases should drive the tests of each.
- Remote gains an input item for every clip setting, for the setup state and for the picked launch button. A Remote map could use them to change a clip without setup mode; nothing requires it to.
- The 72 state items of the launch and stop buttons (off, queued, active) are replaced by light items. "Flashing" is one steady value of a light, so a control surface flashes on its own clock and may be out of step with the panel.
- A codec can only write while it handles MIDI from its surface, which is enough here: every write answers a press.
