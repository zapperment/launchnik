# The four settings of a CV output are one property

Each **clip**, **fill-in**, **landing**, **intro** and **outro** still keeps a pattern, a unipolar value, a bipolar value and a transition for each of its track's six **CV outputs** ([0006](0006-a-clip-keeps-a-value-for-every-cv-mode.md)), but the four are saved as one number property, and are one Remote item: the pattern, plus 8 times the unipolar value, plus 56 times the bipolar value, plus 392 times the transition, 1,960 values in all. We chose this because one property per setting had grown to 7,680 of the device's 9,033 properties, and as many Remote items, with intros and outros, and loading had become unacceptable: in Reason Recon, Launchnik 1.0.0d19, with 5,577 properties, took about 17 seconds to insert and the Launchpad Pro [MK3] codec, with about 5,300 items, another 11 to 15 seconds to start, even without a Launchnik in the rack; 1.0.0d20 took more than a minute. The codec's own code takes milliseconds, so the time goes on the host's handling of the properties and items themselves, and the SDK advises to define as few custom properties as possible. Packed, the device has 3,273 properties and 2,982 Remote items.

## Considered options

- **One value per output, read according to the CV mode.** Rejected for the reasons in [0006](0006-a-clip-keeps-a-value-for-every-cv-mode.md): flipping a switch would reinterpret every clip.
- **Fewer Remote items, the codec keeping its own copies.** Rejected: a duplicate and a new fill-in, landing, intro or outro need the source's settings, which the codec can only learn from the host, and the panel can change them at any time.
- **Packing all six outputs, or a whole clip, into one property.** Rejected: 1,960 to the sixth power does not fit a property's number, and a string property cannot be a Remote item.

## Consequences

- A press that changes one setting writes the whole output: the panel and every codec need all four settings of the output being edited, not just its transition, and keep the three they do not change (`Press.h`, `CLauncher::PackOutputSettings`).
- The undo history names a change after the setting it changes, which the panel finds by comparing the old and the new number.
- Songs and patches saved with one property per setting lose their clips' CV output settings, which come back as a new clip's; the factory patches were converted. Launchnik has not been released, so nothing else needs migrating, but from release on the packing is fixed like the rest of a patch ([0003](0003-patch-holds-clips-not-selections.md)).
- A Remote map can no longer give a surface control of, say, a clip's unipolar value alone; Launchnik's settings were never meant for direct mapping ([0002](0002-realtime-computes-lights-writers-change-clips.md)).
