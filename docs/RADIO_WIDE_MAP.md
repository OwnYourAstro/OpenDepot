# Radio Wide map (portable depot)

Reference: `PatPortDepot.exe` (portable depot), dialog resource `[5,1684]`,
title `General`, 41 controls. This is the page reproduced by the Qt GUI's
`Radio Wide` tab set.

## Verified fields (portable, 205CNX2873)

Paired snapshots generated with the portable Depot on 2026-11-22. All offsets
are payload offsets; the record header is not included.

| Field | Record | Payload offset | Encoding | Verified change |
|---|---|---|---|---|
| Out of Range Indicator | `0x000A` | `0x03` | 2-bit field, bits 2-3 (mask `0x0C`) | No Indication `0xA2` -> Alert Only `0xA6` |
| Imbalanced Coverage Indicator | `0x000A` | `0x4D` | field starts at bit 4 (Alert Only sets `0x10`) | No Indication `0x00` -> Alert Only `0x10` |
| System Registration Indicator | `0x000A` | `0x7E` | 2-bit field, bits 0-1 (mask `0x03`) | 0 `00`, Alert Only `01`, Display Only `02` |
| Time Format | `0x000C` | `0x2C` | 2-bit field, bits 6-7 | 24 hr `0x98` -> 12 hr `0x58` |
| Date Format | `0x000C` | `0x2C` | 2-bit field, bits 4-5 | mm/dd/yy domestic `0x98` -> yy-mm-dd international `0x68` |
| Radio ID Alias | `0x000C` | `46` (`0x2E`) | null-terminated ASCII | `COM CARE` |

## Layout engine schema

`CgIshDataLayout::AddField` was hooked in the running Depot. The full layout
table (2,949 fields, 335 layout groups) is in
`docs/codeplug_field_schema.json`; each row has `g` (layout group), `fid`,
`off` (payload byte), `bits`, `msb`, and `d`.

| Field | Layout group | Field ID | Byte | Bits | MSB |
|---|---|---|---|---|---|
| Out of Range Indicator | `0xf1a8450` (0x000A) | `9271` | `0x03` | 2 | 3 |
| Imbalanced Coverage Indicator | `0xf1a8450` (0x000A) | `13335` | `0x4D` | 4 | 7 |
| System Registration Indicator | `0xf1a8450` (0x000A) | `25543` | `0x7E` | 2 | 1 |
| Time Format | `0xf1a7a30` (0x000C) | `9425` | `0x2C` | 2 | 7 |
| Date Format | `0xf1a7a30` (0x000C) | `9426` | `0x2C` | 2 | 5 |

The group-to-record mapping is confirmed by layout size: `0xf1a8450` spans
158 bytes (`0x000A`) and `0xf1a7a30` spans 274 bytes (`0x000C`).
`docs/codeplug_block_map.json` now contains 169 named codeplug blocks; 50
layout groups (912 fields) matched a block data layout directly. The remaining
groups are nested inside composite blocks and still need their final tags.

Field IDs now resolve to names through the Depot RT_STRING table
(`resource_id = fid + 16`): 2,529/2,949 fields are named. See
[CODEPLUG_FIELD_REGISTRY.md](CODEPLUG_FIELD_REGISTRY.md) and
[CODEPLUG_TABLE_BINDINGS.md](CODEPLUG_TABLE_BINDINGS.md).

The reusable decoder is `OpenDepotCore/tools/decode_field_schema.py`; the
base-codeplug Radio Wide dump is
`build/radio-backups/rw-map/radio-wide-fields.json` (321 fields). It validates
OutOfRange=0, Imbalanced=0, SystemRegistration=0, Time=2 (24 hr), Date=1.

The Maximum Channels control is **read-only** for this portable codeplug. The
UI showed `870`, while payload `0x000A + 8` reads `512`; the old `+8` claim
does not describe this field and must not be used.

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

## Paired snapshot evidence

The files are in `build/radio-backups/rw-map/`:

| Comparison | Changed payload byte |
|---|---|
| `portable-base.cpg` -> `step1-outofrange.cpg` | `0x000A[0x03]`: `0xA2` -> `0xA6` |
| `step1` -> `step2-imbalanced.cpg` | `0x000A[0x4D]`: `0x00` -> `0x10` |
| `step2` -> `step3-sysreg.cpg` | `0x000A[0x7E]`: `0x00` -> `0x01` |
| `step3` -> `step4-timeformat.cpg` | `0x000C[0x2C]`: `0x98` -> `0x58` |
| `step4` -> `step5-dateformat.cpg` | `0x000C[0x2C]`: `0x58` -> `0x68` |
| `step5` -> `step6-sysreg-display.cpg` | `0x000A[0x7E]`: `0x01` -> `0x02` |

The `0x000C` record also contains the Radio ID Alias string at payload offset
`46`; `COM CARE` is the current value.

## Remaining blocker

The complete field layout is captured, but only the Radio Wide layouts are
identified. Still open: map all 335 layout groups to record tags/sections,
decode field types/enums, and reconcile the System Registration field ID.

Out of Range/Imbalanced No Indication and Alert Only, System Registration
0/1/2, Time Format, Date Format, and the alias string are verified.
Out of Range/Imbalanced Display Only are inferred but not yet snapshotted.
Maximum Channels remains read-only/unresolved.
