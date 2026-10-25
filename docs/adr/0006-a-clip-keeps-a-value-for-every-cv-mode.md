# A clip keeps a value for every CV mode

Each **clip**, and each **fill-in**, holds four saved settings for every one of its track's six **CV outputs**: a **pattern**, a **unipolar value**, a **bipolar value** and a **transition**. The **CV mode** switch on the back panel only decides which of the three values the output emits. We chose this because the three modes do not share a range (eight patterns plus "off", against seven unipolar or bipolar steps, with the transition button taking the eighth column), so a single value per output would mean something different, or nothing at all, every time a switch is flipped; and because each value can then have a fixed default (the pattern of the clip's row, unipolar 0, bipolar 0) that a new clip has without anyone writing it, just as a launch button already keeps its pattern and colour while it is **empty**.

## Considered options

- **One value per output, a column from 0 to 7, read according to the CV mode.** Half the properties and Remote items. Rejected: flipping a switch would reinterpret every clip on that output (pattern 8 is no unipolar value at all), and a new clip's value would depend on the mode at the moment SET/CLEAR is pressed, so the panel and every codec would have to read the mode and write the value themselves.

## Consequences

- Every setting is a Remote item, because only the panel and a codec can write saved properties ([0002](0002-realtime-computes-lights-writers-change-clips.md)): 64 launch buttons × 6 outputs × 4 settings × clip and fill-in = 3,072 items, about twice what one value per output would need. Remote documents no limit; whether Reason copes with a map that size has to be seen. If it does not, the fallback is one value per output, with the problems above. It did not cope well: with landings, intros and outros loading became too slow, and the four settings of an output became one property instead, keeping every value ([0008](0008-a-cv-output-setting-is-one-property.md)).
- The Remote map and the codec's item list are too long to keep by hand and are generated.
- Changing a CV mode loses nothing: switching back brings the old values back. A patch carries the values of all three modes, whichever are in use.
- Once patches have been shared, the four settings per output are fixed, like the rest of a patch ([0003](0003-patch-holds-clips-not-selections.md)).
