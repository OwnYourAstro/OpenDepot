# Depot-generated corpus analysis

Analysis of the clean Depot-generated codeplugs used to derive model manifests.
No user zones, channels, names, or identities are reproduced here.

## Corpus

- Portable: `C:\Users\Rose\Desktop\depot_codeplugs`
  - 392 `.cpg` files, 106 models, 7 option families.
- Mobile: `C:\Users\Rose\Desktop\depot_codeplugs_mobile`
  - 144 `.cpg` files, not yet included in the report below.

Reproduce:

```powershell
OpenDepotCore\build\local\Release\corpus-analyzer.exe `
  C:\Users\Rose\Desktop\depot_codeplugs `
  OpenDepotCore\build\analysis\depot_portable.json

python OpenDepotCore\tools\analyze_corpus.py `
  OpenDepotCore\build\analysis\depot_portable.json `
  --model H46UCF9PW6BN --option P25_MDC_Conv_Data
```

`corpus-analyzer` now also does a save/reload round-trip per file:

- Portable: **392/392 byte-identical**, 0 failures.
- Mobile: **72/72 byte-identical**, 0 failures.

It emits per model/option:

- tag, instances, min/max payload length, partitions
- current section assignment
- verified block name
- count of field-schema fields with that exact record tag

## Option families

- `P25_MDC_Conv_Data`
- `3600_Analog_Trunk_Data`
- `3600_Digital_Trunk_Data`
- `9600_Digital_Trunk_Data`
- `MT_PR_Analog_Cnv_Data`
- `MT_Analog_SmartZone_Data`
- `Analog_Cnv_Data`

Each model folder contains a clean codeplug per supported option family.

## H46UCF9PW6BN (XTS 2500) option matrix

| Option | Records | Tags |
|---|---:|---:|
| P25_MDC_Conv_Data | 99 | 96 |
| 3600_Analog_Trunk_Data | 140 | 137 |
| 3600_Digital_Trunk_Data | 140 | 137 |
| 9600_Digital_Trunk_Data | 147 | 144 |

- Conventional base tags: **96**
- Trunking adds **41** tags for 3600; 9600 adds **48**
- Conventional tags are a subset of every trunking option; the conventional
  profile is the correct clean base for the first target.

Manifest generated:
`OpenDepotCore/schema/manifests/H46UCF9PW6BN-P25_MDC_Conv_Data.json`

The manifest currently carries the **observed** section assignment from
`CodeplugModel`; that assignment is not authoritative yet (see below).

## Conventional clean profile vs a real 2-zone/16-channel codeplug

The clean conventional profile contains the same 96 tag types but only one
default zone/channel. A real codeplug changes instance counts and table
lengths, not the base tag set:

| Tag | Clean instances | Clean payload | Real example | Relation |
|---|---:|---:|---:|---|
| `0x0FFB` Conventional Personality | 1 | 75 | 16 | one per channel |
| `0x0FCD` Conventional Personality Floating | 1 | 10 | 16 | one per channel |
| `0x0FFD` Zone Channel Assignment page | 1 | 41 | 2 pages, 216/356 | `6 + 35 * channels_in_page` |
| `0x0FFF` Channel Text | 1 | 23 | 278 | `6 + 17 * channel_count` |
| `0x0FB4` | 1 | 13 | 2 | one per page slot (0/1) |
| `0x0FB5` | 1 | 18 | 2 | one per page slot (0/1) |
| `0x0FBE` | 1 | 3 | 2 | one per page slot (0/1) |
| `0x0FCB` | 1 | 15 | 2 | one per page slot (0/1) |
| `0x0FCC` | 1 | 33 | 2 | one per page slot (0/1) |
| `0x0FF9` | 1 | 14 | 2 | one per page slot (0/1) |
| `0x0FFA` | 1 | 60 | 2 | one per page slot (0/1) |

Verified formulas from the example:

- `0x0FFD`: payload `6 + 35 * N`, where `N` is the page channel count. The
  page header carries the tag, page instance, and count.
- `0x0FFF`: payload `6 + 17 * C`, where `C` is the channel-name count.
- `0x0FFB` and `0x0FCD`: exactly one instance per channel; the `0x0FFD`
  channel entry references their instances.
- `0x0FFC` stays 15 bytes and references the zone page.
- `0x0016` was 10 bytes clean and 11 bytes in the real example; not yet
  explained (candidate: capacity/zone count).
- `0x0FFE` was 12 bytes clean and 14 bytes in the real example; not yet
  explained.

These are the conventional object graph edges the builder must maintain.

## Capacity data in the manifest

The generated manifest merges `ModelReadRequests` into each record as
`capacity`:

| Tag | Capacity instances | Capacity max payload |
|---|---:|---:|
| `0x0FFB` | 16 | 77 |
| `0x0FCD` | 16 | 12 |
| `0x0FFD` | 2 | 358 |
| `0x0FFF` | 1 | 280 |

The clean profile only has one channel. The read set is therefore required for
a builder that can create or resize zones/channels to model capacity.

## Conventional channel builder (implemented)

`CodeplugModel::AddChannel` performs the multi-record channel transaction:

- appends a 35-byte entry to the zone's `0x0FFD` page and bumps its count
- creates the zone's second `0x0FFD` page after 10 entries and links it from
  the first page
- appends the channel name to the zone's `0x0FFF` table and bumps the count
- creates `0x0FFB` and `0x0FCD` records and points the new personality at the
  new `0x0FCD` instance
- creates the shared second-page-slot records
  `0x0FB4/0x0FB5/0x0FBE/0x0FCB/0x0FCC/0x0FF9/0x0FFA` once (instances 0 and 1
  are shared by every zone's first/second page, not copied per zone)
- enforces the `0x0FFB` instance capacity from `ModelReadRequests` (16 for
  H46UCF9PW6BN)

Result on H46UCF9PW6BN: a clean one-channel profile grew to a two-page
16-channel codeplug with **137 records**, the same record ids, instances, and
payload lengths as the real 16-channel backup. The only difference was
`0x0016` (Cnv Menu Item Block, 10 vs 11 bytes), which is a menu/switch
configuration record and is unchanged by adding channels.

`CodeplugModel::RemoveChannel` is the inverse transaction:

- refuses to remove the last channel in a zone
- refuses if another record (for example a scan list) still references the
  personality
- removes the 35-byte entry and its `0x0FFB`/`0x0FCD` records
- shifts/resizes the zone `0x0FFF` name table
- repacks `0x0FFD` and deletes the zone's second page when it becomes empty
- deletes the shared slot-1 records only when no other zone still has a second
  page

`opendepot_conventional_test` covers add/remove, page-one deletion, name
shifting, save/reload, validation, capacity rejection, and the last-channel
policy. CLI: `--add-channel <in> <out> <zone> <name> <rx> <tx>` and
`--remove-channel <in> <out> <zone> <channel>`.

Remaining conventional work: reorder channel, add/remove zone, scan-list
membership, and the authoritative section map.
## What this means

1. **The base record set for H46 conventional is known** (96 tags) and the
   clean profile is a valid empty starting point (one default zone/channel).
2. **Record capacities are not encoded in the clean profile.** The clean
   profile is a minimal one-channel codeplug. Full capacities must come from
   the model layout/read set (for H46: 16 personality instances, 280-byte
   channel text, etc.).
3. **Adding/removing a channel is a multi-record transaction**: resize
   `0x0FFD`, update its count, resize/update `0x0FFF`, create/delete
   `0x0FFB` + `0x0FCD`, update the zone page reference, and keep the
   personality/name references consistent.
4. **The current section map is still guesswork.** Examples from the clean
   conventional profile:
   - `0x0018`, `0x0019`, `0x0020` are button blocks but currently sit under
     Radio Wide.
   - `0x0027` Test Mode Personality sits under Display & Menu.
   - `0x004C` Astro System Call List Alias sits under Scan List.
   - `0x0057` DTMF Codes List sits under Secure.
   - `0x0058`, `0x0070`, `0x0071` are list headers under Radio Wide.
   - `0x0FAB` Data User List Entry sits under Radio Wide.
   - `0x0FFA` Astro Systems Block sits under Conventional Personality.
   These labels must be replaced by a Depot tree/block manifest before the UI
   is treated as CPS-parity.
5. **Field coverage is still partial.** Many 96 base tags have zero matching
   schema fields (e.g., `0x001B`, `0x000D`, `0x0FCD`, `0x0FFF`).

## Next work

1. Build the authoritative section manifest from the Depot CPS tree/block
   layout instead of `SectionDefs` guessing.
2. Add capacity rules to the manifest from the model read set/layout.
3. Implement the conventional object model and transactional channel/zone
   CRUD using the formulas above.
4. Add a clean H46 conventional fixture and round-trip/CRUD tests that assert
   only the expected records change.
5. Re-run the same analysis on the mobile corpus and the trunking options.





