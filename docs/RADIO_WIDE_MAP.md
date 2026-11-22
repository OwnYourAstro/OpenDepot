# Radio Wide map (portable depot)

Reference: `PatPortDepot.exe` (portable depot), dialog resource `[5,1684]`,
title `General`, 41 controls. This is the page reproduced by the Qt GUI's
`Radio Wide` tab set.

## Verified field

| Field | Record | Offset | Type | Evidence |
|---|---|---|---|---|
| Maximum Channels (Talkgroups) | `0x000A` | `8` | u16 little-endian | 512 reads as `00 02`; GUI change to 511 writes `FF 01` and survives save/reload |

## General page controls (from the dialog resource)

| Control ID | Class | Label / role |
|---|---|---|
| `0x4B0D` | button | Radio Cloning |
| `0x4B05` | button | Block Pending CA/PC |
| `0x4B23` | edit | Maximum Channels value |
| `0x4B17` | combo | Out of Range Indicator |
| `0x4BEE` | combo | Imbalanced Coverage Indicator |
| `0x7059` | combo | System Registration Indicator |
| `0x4C10` | combo | Time Format |
| `0x4BFB` | combo | Date Format |
| `0x5611` | button | Channel Announcement |
| `0x4BFD` | combo | Channel Announcement Priority |
| `0x5F72` | button | Last Selected Channel Per Zone Enable |
| `0x5F57` | button | Power Up On Last Selected Zone and Channel |
| `0x7051` | button | Preset Zone and Channel Enable |
| `0x5F51` | button | ARS User Login Enable |
| `0x5F4E` | edit | Soft ID/Username |
| `0x5F50` | edit | PIN/Password |
| `0x7068` | button | User Login Unit ID Enable |
| `0x7067` | edit | User Login Unit ID |
| `0x706A` | combo | Cached Credential User Login Mode |
| `0x55AE` | button | Radio ID Alias Enable |
| `0x4B25` | edit | Radio ID Alias |
| `0x7064` | combo | Key Type |
| `0x7062` | edit | Owner System ID |
| `0x7060` | button | Disable Write Protect |

The screenshot also shows `Selectable Keypad Mute` and `Power Button`; those
come from the legacy General page variant (`[5,1561]`) and are present in the
GUIs page for parity.

## Field-change evidence

The depot writes a modification log next to each codeplug. Examples:

- `Trunking-patched.cpglog`: `Out of Range Indicator`, `Imbalanced Coverage
  Indicator`, `System Registration Indicator` changed `No Indication -> Display
  Only`; `Preset Zone and Channel Enable` and `Radio ID Alias Enable` toggled.
- `Recipiant_merged.cpglog`: personality, ASTRO system and channel changes.

## Remaining blocker

The logs give names and values but not offsets, and the available codeplugs
already contain the final values, so they cannot isolate the bytes. A paired
snapshot is needed:

1. Save the same portable-depot codeplug twice.
2. Change **one** Radio Wide General field in one copy (for example
   `Out of Range Indicator` from `No Indication` to `Display Only`, or a single
   checkbox).
3. Save both copies and compare the `0x000A` (and neighbouring Radio Wide)
   payloads.

That diff pins each control's offset and bit, after which the disabled controls
can be enabled without guessing.
