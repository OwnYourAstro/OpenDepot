# CPS object model

`OpenDepotCore` exposes a CPS-style model (`opendepot/core/model.h`) instead of
forcing the GUI to re-parse records. The model wraps a loaded `Codeplug` and
maps records to the same sections the ASTRO 25 CPS shows.

Coverage was derived from 33 codeplugs (29 radio backups plus 4 fixtures)
spanning conventional, trunking, 800/900/UHF/VHF, and several model families:

- **149 distinct record tags** decoded.
- **Every record** is represented as a `RecordNode` and assigned to a section;
  the corpus has **0 unmapped records**.
- The full tag catalog is in [CODEPLUG_TAGS.md](CODEPLUG_TAGS.md).

Confidence levels:

- **verified**: decoded and round-tripped through the hardware-tested radio.
- **inferred**: section/record correlation is strong but field meanings are not
  proven.
- **unknown**: bytes are exposed raw only.

## Record model

Every codeplug record becomes a `RecordNode`:

| Field | Meaning |
|---|---|
| `id`, `tag`, `inst`, `type`, `length` | record identity |
| `section` | owning `CodeplugSection::id` |
| `family` | `identity`, `name_table`, `name`, `list_page`, `references`, `blob` |
| `name`, `names` | decoded name(s), when the record carries text |
| `references` | `(tag, instance, offset)` entries that point at real records |
| `strings` | printable strings in the payload |
| `payload` | raw bytes, always preserved |

Families are detected structurally:

- `name_table`: `[2..3] == 17`, capacity at `[4..5]`, count at `[6..7]`, 17-byte
  names at `8 + 17*i` (`0x000D`, `0x004C`..`0x004F`, `0x0059`, `0x005B`,
  `0x006C`, `0x006E`, `0x0FD6`, `0x0FD9`, `0x0FDC`, `0x0FF8`).
- `name_table` (`0x0FFF` only): count at `[4..5]`, 17-byte names at `6 + 17*i`.
- `name`: `00 00 00 00 00 01` prefix, text at offset 6 (`0x0FAA`, `0x0FAB`,
  `0x0FAC`, `0x0FB0`, `0x0FBC`, `0x0FCE`, `0x0FC7`, `0x0FF4`, `0x0FE8`,
  `0x0FEC`).
- `list_page`: self-header (`tag` at `[0..1]`) with a count at `[4..5]` and
  equal-size entries, plus the known `0x0FFD` (35-byte channel) and `0x0FA1`
  (15-byte scan) pages.
- `references`: payload contains `(tag, instance)` pairs that resolve to real
  records. References may be **odd-aligned** (records such as `0x0FFB` have a
  1-byte prefix), and instance `0xFFFF` means "tag reference, no instance".
- `identity`: `0x1004`, `0x1005`, `0x1006`.
- `blob`: everything else; bytes are preserved and exposed.

## Section map

| CPS section | Records | Confidence |
|---|---|---|
| Digital / Radio Information | `0x1004`, `0x1005`, `0x1006` | verified |
| Radio Configuration / Radio Wide | `0x000A`, `0x000B`, `0x000C`, `0x0014`, `0x0016`, `0x0018`, `0x0019`, `0x0020`, `0x0028`, `0x002A`, `0x002C`, `0x002D`, `0x0048`, `0x0058`, `0x0070`, `0x0071`, `0x0FAB` | inferred |
| Radio Configuration / Test Channel List | `0x0FF0`, `0x0FF1` | unknown |
| Radio Configuration / Narrow Bandwidth 12.5 kHz Only Frequency Range List | `0x0029` | inferred |
| Radio Configuration / Radio Wide VRS-EP Mode Steering | `0x002F`..`0x0033` | unknown |
| Radio Configuration / Controls | `0x001B` | inferred |
| Radio Configuration / Display & Menu | `0x0026`, `0x0027` | inferred |
| Radio Configuration / Phone | `0x0049`, `0x004A`, `0x004B`, `0x005A`, `0x0FCE`, `0x0FCF` | inferred |
| Radio Configuration / Secure | `0x0034`, `0x0057` | unknown |
| Trunking / Trunking Configuration | `0x0015`, `0x0017`, `0x001A`, `0x0021`..`0x0023`, `0x0F9F`, `0x0FA0` | inferred |
| Trunking / Trunking System | `0x003A`..`0x003F` | inferred |
| Trunking / Trunking Personality | `0x0052`, `0x0053`, `0x0FA8`, `0x0FA9`, `0x0FC6`, `0x0FD0`, `0x0FD1` | inferred |
| Trunking / Talkgroup | `0x0FD2`..`0x0FEF` | inferred |
| Conventional / Conventional Configuration | `0x0040`, `0x0050` | inferred |
| Conventional / MPL | `0x0064`, `0x0065`, `0x0FBC`, `0x0FBD` | inferred |
| Conventional / Conventional Personality | `0x0FBE`, `0x0FCD`, `0x0FFA`, `0x0FFB` | inferred |
| Conventional / Message Alias List | `0x005B`, `0x0FAA` | verified |
| Conventional / Status Alias List | `0x0059` | verified |
| Conventional / MDC | `0x0FB4`, `0x0FB5` | inferred |
| Conventional / Quik-Call II | `0x0FCB`, `0x0FCC` | inferred |
| Conventional / DTMF | `0x0FC1`, `0x0FC2`, `0x0FC7`..`0x0FCA` | inferred |
| Conventional / Singletone | `0x0FB2`, `0x0FB3` | inferred |
| Conventional / ASTRO Systems | `0x0FAC`..`0x0FB1`, `0x0FF2`..`0x0FF9` | inferred |
| Scan / Scan Configuration | `0x0054`, `0x005F`, `0x0073`, `0x0074`, `0x0075`, `0x0FA6` | inferred |
| Scan / Scan List | `0x004C`..`0x004F`, `0x006C`, `0x006E`, `0x0FA1` | inferred |
| Zone Channel Assignment / Zone Channel Assignment - 1 | `0x000D`, `0x0FFC`, `0x0FFD`, `0x0FFE`, `0x0FFF` | verified |
| Expert / Codeplug Directory | `0x0047` | verified |
| Expert / Passcode | `0x0051` | verified |
| Expert / Other Records | any tag not otherwise mapped | unknown |

## Zones and channel names

- `0x000D`: `[2..3]` = 17 (entry size), `[4..5]` = capacity, `[6..7]` = zone
  count, zones at `8 + 17*i`.
- `0x0FFE[zone]`: `[8..9]` = name-table tag (`0x0FFF`), `[10..11]` = instance.
- `0x0FFF`: `[4..5]` = channel-name count, names at `6 + 17*i`.
- `0x0FFC[zone]`: `[8..9]` = channel-page tag (`0x0FFD`), `[10..11]` = first
  page. Pages are read for `first..first+1`.
- `0x0FFD` page: `[0..1]` = `0x0FFD` (or zero on a one-channel page),
  `[2..3]` = page ordinal + 1, `[4..5]` = entry count, entries at `6 + 35*i`.

## Channel entry (35 bytes, verified offsets)

| Offset | Meaning |
|---|---|
| `0..1` | `0x0FFB` entry type |
| `2..3` | channel id (instance of `0x0FFB`/`0x0FCD`) |
| `4` | unknown, `0x64` in all tested codeplugs |
| `5` | unknown, `0x2A` |
| `6..11` | unknown, `0x0293 0293 0293` |
| `12..15` | RX word (MHz = word * 5 / 1e6) |
| `16..19` | TX word |
| `20..23` | alternate/TX word |
| `24` | RX squelch code |
| `26` | TX squelch code |
| `27` | flags: bit 7 simplex, bit 6 split |
| `28..33` | unknown, zero |
| `34` | unknown, `0x01` |

## Per-channel records

- `0x0FFB[channel id]` (75 bytes): `[0]` = 0, `[1..2]` = `0x0FCD`,
  `[3..4]` = instance; references at `0x1E` (`0x0FBE`), `0x2B` (`0x0FFA`),
  `0x2F` (`0x0FF3`), `0x37` (`0x0FF7`); trailing `0x26` bytes.
- `0x0FCD[channel id]` (10 bytes): all zero in the tested codeplug. Field
  semantics are unknown; the model exposes the bytes.

## Alias and name tables

- `0x0059` Status Alias List and `0x005B` Message Alias List: `[2..3]` = 17,
  `[4..5]` = capacity (16), `[6..7]` = count, entries at `8 + 17*i`.
- `0x004C`/`0x004D`/`0x004E`/`0x004F`/`0x006C`/`0x006E`: `[6..7]` = count,
  names at `8 + 17*i`.
- `0x0FA1[i]` (246 bytes): scan-list membership table; `[4..5]` = 16, 15-byte
  rows at `6`. Row encoding is not decoded yet.

## Editing

`CodeplugModel` setters mutate the underlying record payloads and refresh the
decoded view:

```cpp
opendepot::Codeplug cp;
opendepot::LoadCodeplug("radio.cpg", cp);
opendepot::CodeplugModel model(cp);
std::string err;
model.Reload(err);

model.SetChannelName(0, 0, "DISPATCH");
model.SetChannelFrequency(0, 0, 851.0125, 806.0125);
model.SetChannelSquelch(0, 0, 3, 5);
model.SetAliasEntry(0, 0, "STATUS OK");

model.Save("radio-edited.cpg", err);  // CRCs + directory recomputed
```

`SetChannelEntryByte`, `SetPersonalityByte`, `SetAstroByte` and
`SetRecordPayload` cover fields that are not yet named. `opendepot-cli --model`
prints every decoded node as JSON, and `--schema` prints the tag/section
catalog for one codeplug.

## Open questions

- Semantics of channel entry bytes `4..11`, `28..33`, `34`.
- `0x0FCD` field layout (likely PL/DPL/ASTRO data; zero in the current sample).
- Radio Wide (`0x000A` etc.), personality internals, MDC/Quik-Call/DTMF/
  Singletone and scan-list row encoding.
- Adding/removing channels and zones: the record set is fixed by the ISH
  directory, so this needs record allocation and directory updates, not just
  field edits.

Decoding these needs varied real codeplugs (non-default PL, bandwidth, power,
signaling) or static analysis of `PatPortDepot.exe` / `fhAdk10c.dll`.
