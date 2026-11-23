# Verified codeplug block types (live Depot)

Status: **verified** block `type -> record tag` map, recovered live from
`PatPortDepot.exe` (ASTRO 25 Portable Depot) with an in-process helper DLL.
Model exercised: `H46UCF9PW6BN`, serial `205CNX2873`.

This is the block/serialization layer: every CPS block type a model uses and the
`Strm_N` record tag it is written to in the `.cpg`. It is the authoritative
replacement for the earlier heuristic `type -> item_id` guesses.

## How it was recovered

1. Attach x32dbg to a running Depot (the Depot exits if started under the
   debugger, so attach after launch). Open a codeplug so the model layouts load.
2. The layout objects are `CcgIshBlkLayout`; exported getters drive them:

   | Offset | Meaning |
   |---|---|
   | `+0x00` | vtable `??_7CcgIshBlkLayout@@6B@` |
   | `+0x04` | u16 block **type** (`GetBlkID`) |
   | `+0x06` | u16 **record/ISH tag** (`GetIshType`) |
   | `+0x24` | block name (`GetBlkName`, MFC `CString`) |
   | `+0x2C` | `CcgIshDataLayout*` (`GetDataLayout`) |

3. A helper DLL (`cgish_dump*.dll`, MSVC x86 `/LD /MT`) enumerated every
   `CcgIshBlkLayout` by scanning committed memory for the vtable and calling the
   exported getters. It was injected by `LoadLibraryA` executed on a hijacked
   thread under x32dbg.

Key exports used (all exported by `cgish10c.dll`):

```
?GetBlkID@CcgIshBlkLayout@@QBEGXZ
?GetIshType@CcgIshBlkLayout@@QBEGXZ
?GetDataLayout@CcgIshBlkLayout@@QBEPBVCcgIshDataLayout@@XZ
?GetFieldID@CcgIshFieldLayout@@QBEIXZ
?GetByteOffset@CcgIshFieldLayout@@QBEIXZ
?GetLengthInBits@CcgIshFieldLayout@@UBEIXZ
?GetMsbStart@CcgIshFieldLayout@@QBEEXZ
?GetParentDataLayout@CcgIshFieldLayout@@QBEPBVCcgIshDataLayout@@XZ
```

## Table (77 verified blocks)

| Record tag | Block type | CPS block name |
|---|---|---|
| `0x000A` | `0x0031` | Radio Wide |
| `0x000B` | `0x0002` | Display & Menu |
| `0x000C` | `0x0012` | Dynamic Radio Block |
| `0x0014` | `0x00B0` | ISH Conventional Switch Block |
| `0x0016` | `0x0011` | Cnv Menu Item Blk |
| `0x0018` | `0x00BA` | Global Button Config List |
| `0x0019` | `0x00BB` | Conventional Button Config List |
| `0x001B` | `0x0038` | Hardware Button Configuration Block |
| `0x0020` | `0x00AE` | Ish Portable Conventional Button Block |
| `0x0021` | `0x00B4` | HHCH Conventional Block |
| `0x0026` | `0x0040` | Conventional Configuration |
| `0x0027` | `0x0029` | Test Mode Personality Block |
| `0x0028` | `0x00C7` | Test Mode Personalitty Float Data Block |
| `0x0029` | `0x002A` | Test Channel Table Block |
| `0x002A` | `0x00A6` | Astro Peripheral Data Block |
| `0x002B` | `0x00A1` | ASTRO OTAR Radio Block |
| `0x002C` | `0x007B` | Astro Radio Block |
| `0x002D` | `0x0084` | Astro dynamic Block |
| `0x002E` | `0x0082` | OTACR block vector |
| `0x002F` | `0x0095` | Factory Overrides DSP Clock Shifts Block Pointer |
| `0x0030` | `0x0094` | Factory Overrides Host Clock Shifts Block Pointer |
| `0x0031` | `0x00AB` | FGU RxSynthesizer Block Pointer |
| `0x0032` | `0x0096` | Factory Overrides Secure Clock Shifts Block Pointer |
| `0x0033` | `0x00A3` | FGU Tx Synthesizer Block Pointer |
| `0x0037` | `0x00A0` | CKR Key Number Block |
| `0x0038` | `0x0085` | Hardware Key Name Block |
| `0x0040` | `0x00E5` | Test Personality Cnv Channel Data |
| `0x0047` | `0x0030` | External Radio Blk |
| `0x0048` | `0x0007` | ID Disp List Vector |
| `0x0049` | `0x00CF` | Phone Number List Distributed Header |
| `0x004A` | `0x00CE` | Phone Number Text List Distributed Header |
| `0x004B` | `0x00D1` | Second LO Injection Default Freq Blk |
| `0x004C` | `0x00D2` | Astro System Call List Alias List |
| `0x0052` | `0x00DA` | Rx Frequency Split Block |
| `0x0053` | `0x00DB` | Tx Frequency Split Block |
| `0x0054` | `0x00DC` | NAT Entry |
| `0x0055` | `0x0066` | OTAR Radio Block |
| `0x0057` | `0x0034` | DTMF Codes List Block |
| `0x0058` | `0x00DE` | Mdc Status Alias Distributed List Header |
| `0x0059` | `0x0006` | Sts Alias Text Block |
| `0x005A` | `0x00DF` | Mdc Message Alias Distributed List Header |
| `0x005B` | `0x0009` | Msg Alias Text blk |
| `0x005E` | `0x00F1` | (unlabeled) |
| `0x0070` | `0x0119` | Data User List Distributed Header |
| `0x0071` | `0x011A` | Quick Text Message Distributed Header |
| `0x0075` | `0x012A` | Trunking APCO Inter WACN Dyn Chan Id Distributed List Header |
| `0x0F9F` | `0x012B` | APCO Trunking Call WACN System ID List Block |
| `0x0FA0` | `0x012C` | APCO Trunking Call WACN System ID List Distributed Header |
| `0x0FA3` | `0x0128` | Conventional Personality MVS Channel Alias Text Header Block |
| `0x0FA5` | `0x0126` | Conventional Personality MVS Channel Options Header Block |
| `0x0FAA` | `0x011C` | Quick Text Message Entry |
| `0x0FAB` | `0x011B` | Data User List Entry |
| `0x0FBF` | `0x00F0` | (unlabeled) |
| `0x0FC1` | `0x0008` | Message Alias Table |
| `0x0FC2` | `0x0005` | Status Alias Table |
| `0x0FC4` | `0x00EB` | Secure KMF Profile List LED |
| `0x0FC5` | `0x00EA` | Secure KMF Profile List Header |
| `0x0FCC` | `0x000C` | MDC System |
| `0x0FCD` | `0x00D5` | Conventional Personality Floating Block |
| `0x0FCE` | `0x0039` | Phone List Text Block |
| `0x0FCF` | `0x0035` | Phone Number Table |
| `0x0FF2` | `0x0091` | Astro Talkgroup ID Block |
| `0x0FF3` | `0x00C6` | Astro Talkgroup ID List Distributed Header |
| `0x0FF4` | `0x0080` | Astro Call ID Text Block |
| `0x0FF5` | `0x00C2` | Astro System Call ID Text List Distributed Header |
| `0x0FF6` | `0x007F` | List |
| `0x0FF7` | `0x00C1` | Astro System Call ID List Distributed Header |
| `0x0FF8` | `0x0092` | Astro Talkgroup Alias Text Block |
| `0x0FF9` | `0x0081` | Astro Emergency Block |
| `0x0FFA` | `0x007D` | Astro Systems Block |
| `0x0FFB` | `0x0004` | Conventional Personality |
| `0x0FFC` | `0x00B1` | Zone Channel Assignment Dist Header Block |
| `0x0FFD` | `0x002D` | Zone Channel Assignment |
| `0x0FFE` | `0x00B2` | Channel Text Header Block |
| `0x0FFF` | `0x00AD` | Channel Text Block |
| `0x12BE` | `0x00EF` | Voice File Data Distributed List Block |
| `0x12BF` | `0x00EE` | Voice File Data Distributed List Header Block |

Names come from `docs/codeplug_block_map.json` (CPS block id -> name), joined by
block type. The tags are the verified live values.

## Important limitation: field registry

The 2,949-field `CcgIshDataLayout::AddField` registry documented in
[RADIO_WIDE_MAP.md](RADIO_WIDE_MAP.md) is **not instantiated by the Depot's depot
flows**. In a freshly opened codeplug, and after a Create-codeplug model/package
selection, live memory contains:

- many `CcgIshBlkLayout` objects (this table), but
- **zero** populated `CcgIshDataLayout` field maps, and
- `CcgIshDataLayout::AddField` is not called.

Practical consequence: block/tag mapping and the `.cpg` container are confirmed
from the Depot, but per-field bit offsets must continue to come from
`docs/codeplug_field_schema.json` (captured elsewhere) plus hardware-verified
samples. Do not assume the Depot will rebuild that registry on demand.

## Wiring in OpenDepot

- Core: `include/opendepot/core/block_map.h`, `src/model/block_map.cpp`
  (`BlockTagCatalog`, `FindBlockByTag`, `FindBlockByType`, `BlockNameForTag`).
- GUI: **CPS Blocks** page lists every verified block with its record tag and
  whether the loaded codeplug contains it (`Present` column).

## CPS panel record bindings

Reproduced in the GUI **CPS Panels** page from the live records:

- **Test Channel List** — record tag `0x0029`, one row per instance; Tx frequency at
  payload `+8`, Rx at `+12`, each a big-endian u32 in Hz (displayed MHz).
- **Narrow Bandwidth 12.5 kHz Only Frequency Range List** — schema group
  `0x107ff918`, Start `+0` / End `+4` (big-endian u32 Hz). The list is detected from
  the codeplug's start/end record; absent in a codeplug with no narrow-band entries.
- **Buttons** — conventional button config record `0x0020` (reached from the
  `0x0019` pointer): Top `+9`, Side 1 `+13`, Side 2 `+15`, Side 3 `+17`, Data `+21`,
  8-bit function codes. Observed codes: `0x23` Unprogrammed, `0x19` Monitor,
  `0x1A` TalkAround/Direct. The trunking button config (schema group `0xf19bbb0`)
  is a separate record and is shown disabled when the codeplug has none.

## Reproduce

1. Launch `PatPortDepot.exe` normally and open a codeplug.
2. Attach x32dbg; find `cgish10c.dll`; `CcgIshBlkLayout` vtable is
   `base + 0x1D290` in this build.
3. Inject a DLL that scans committed memory for that vtable and calls
   `GetBlkID`/`GetIshType`/`GetDataLayout`.
4. Deduplicate by object address; join block type to names via
   `codeplug_block_map.json`.

Safety: this only reads the running app. It never writes a radio or codeplug.
