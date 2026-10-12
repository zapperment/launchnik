# A scene press writes eight selections; there is no scene property

A **scene button** stands for the eight launch buttons of its row, so a press of it is carried out by its writer (the panel's click handler, or a codec) as eight writes, one **selection** per track, following the same rule as eight presses of launch buttons: a track whose launch button in the scene is **empty** gets the stop button. The device has no property that says "scene N". We chose this because a track's selection is the single saved, automatable statement of what the track plays ([0001](0001-selection-in-state-out.md)); a scene value beside it would be a second statement about the same thing, and the realtime code, which cannot write selections, would have to decide which of the two wins.

## Considered options

- **One saved "scene" property that the realtime code applies to all tracks.** One Remote input item, one write per press and one automation lane. Rejected: after a scene launch, a click on a single launch button would leave the scene property claiming something the tracks no longer play, and reopening a song or playing back automation would need a rule for which value is newer, which the host gives realtime code no means to know.

## Consequences

- Nothing records which scene was launched last, so a scene button cannot show it. Its light is free, and is used for the **interval meter**.
- A scene launch recorded as automation lands in the selection lanes of up to eight tracks.
- On the panel a scene press is one step in the undo history. A codec hands the host eight inputs for one press.
- The rule "a press changes one property" of [0002](0002-realtime-computes-lights-writers-change-clips.md) becomes "a press yields a list of changes", in the panel's code and in every codec.
- Scene buttons launch in **launch mode** only. In **clip picking** a press chooses the **length** of the switch interval and returns to launch mode, which is two writes: the length and the setup state. In **clip editing** a press does nothing.
- Remote gains eight output-only items, "Scene N Light", encoded like the light of a launch button, and no input items for scenes. The length has an item of its own, "Switch Interval".
