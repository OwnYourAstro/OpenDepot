# WARNING — THIS SOFTWARE CAN BRICK YOUR RADIO

> **OpenDepot can permanently brick your radio. Use it at your own risk.**
>
> This is **experimental, reverse-engineered software**. A write can leave a radio
> permanently corrupted (for example a blank or mismatched identity block causing
> `FAIL 01/92`), and an interrupted or incorrect write can leave it unbootable.
>
> - It has **only been tested on a single XTS 2500**. Every other model/band is
>   untested on real hardware.
> - **Firmware flashing is NOT supported.** A radio whose firmware failed to flash
>   cannot be repaired by OpenDepot (codeplugs only).
> - **Always keep a verified backup** and run `--check-write` before writing.
>   **Never force** a write that OpenDepot refuses.
> - Do not use it on a radio you cannot afford to lose.
>
> Provided **as is**, with **no warranty** of any kind. You assume all risk.
> OpenDepot is not affiliated with, endorsed by, or supported by Motorola
> Solutions.

# OpenDepot

Modern desktop codeplug editor and radio programmer for Motorola ASTRO 25
radios. It reads, edits, creates, verifies and writes `.cpg` codeplugs, and
reads and writes a connected radio over its serial/PPP programming link without
needing the Depot.

The project is split into two repositories:

- **OpenDepot** (this repository) — the Qt 6 Widgets desktop app, plus the
  reverse-engineering notes and tooling.
- **OpenDepotCore** — the portable engine: `.cpg` container, crypto, model and
  FLASHcode data, reporting/doctor/diff, and the native radio transport. The GUI
  links it.

The GUI is a modern take on the ASTRO 25 CPS/Depot layout: a tree view, a
card-based Radio Information screen, FLASHcode feature chips, and first-class
radio read/write.

## Current state

Experimental / alpha. The container layer, native radio transport, and a
CPS-style object model now work. The model represents **every record** and maps
it to the CPS section tree (149 tags across 29 radio backups plus 4 fixtures,
with 0 unmapped records), and zone/channel fields are editable. Building
zones/personalities/talkgroups from scratch and field-level decoding of some
radio-wide/personality/trunking payloads are still in progress; creation clones
a same-model built-in schema and then applies identity and FLASHcode.

Working today:

- Decode and verify `.cpg` (portable MS-CFB reader, records, directory, CRC-16/CCITT).
- Re-encode a compatible container with portable RC4 whose keystream matches the
  Windows CryptoAPI output byte for byte.
- Read/edit identity (serial/model), FLASHcode, zone and channel names, and
  frequencies; read/set/clear the codeplug passcode record.
- Read the FLASHcode option map and rebuild a valid FLASHcode from selected
  packages/add-ons (option tables for 13 ASTRO 25 platforms).
- **Native radio read** (no vendor DLL): complete tag-based read of the tested
  model (137 records, all CRC-ok, ~20 s), with a completeness guard.
- **Native radio write** (no vendor DLL, no Python): full record set, reorg, end
  (~17 s), identity verified, radio returned to AT mode.
- **Create a codeplug** from a built-in schema, with or without a template.
- Convert a codeplug to conventional operation.
- Qt 6 GUI: Radio Information dashboard, Zones & Channels page, Service Report,
  Codeplug Doctor, Diff, Records view, and native radio read/write.
- CPS object model in the core: every record decoded into a section tree, with
  typed zone/channel setters, alias/list/scan views, reference decoding, and raw
  record editing.
- CLI: inspect, edit, create, report, doctor, diff, copy zones, dump the model
  (`--model`, `--schema`) and raw records (`--hex`, `--hexall`), and radio I/O.

Not implemented:

- Building zones/channels/personalities/talkgroups from scratch (the record set
  is fixed by the ISH directory).
- Editable Radio Configuration forms (Radio Wide, Controls, Keypad, Display &
  Menu) in the GUI — the core exposes the records, but those tree nodes are
  still placeholders.
- Field-level names for every radio-wide/personality/trunking byte; see
  `docs/CPS_MODEL.md` and `docs/CODEPLUG_TAGS.md`.
- Create wizard, FLASHcode editor, and passcode/backup screens in the GUI.
- Any firmware/boot flash. A radio that failed a firmware flash cannot be
  recovered here.

## Requirements

- Windows 10/11 x64 and Visual Studio 2022 (C++ workload).
- Qt 6.8+ (MSVC 2022 x64).
- The `OpenDepotCore` checkout as a sibling directory (or set
  `-DOPENDEPOT_CORE_DIR=<path>`).

## Build

```powershell
$cm = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cm -S . -B build\qt -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
& $cm --build build\qt --config Release
```

Outputs:

- `build\qt\Release\OpenDepot.exe` — Qt 6 GUI.
- `build\qt\opendepotcore\Release\opendepot-cli.exe` — console (built by the core subproject).

For a standalone run, deploy the Qt runtime next to the GUI:

```powershell
& "C:\Qt\6.8.3\msvc2022_64\bin\windeployqt.exe" --release build\qt\Release\OpenDepot.exe
```

## GUI

Qt 6 Widgets with a custom light theme and a tree generated from the core's
CPS model.

- **Radio Information** - serial, model, FLASHcode, header version, record count,
  passcode status, codeplug file, FLASHcode feature chips.
- **Zone Channel Assignment** - grouped channel table plus an editor for zone and
  channel names, RX/TX MHz, and RX/TX squelch.
- **Every other section** - Radio Wide, Controls, Display & Menu, Phone, Secure,
  Trunking, Conventional Personality, aliases, MPL, MDC, DTMF, ASTRO Systems,
  Scan, Expert - opens a record explorer. It lists the section's records with
  tag/instance/size/family, resolves references and strings, edits name tables,
  and edits any record payload as hex.
- **Service Report**, **Codeplug Doctor**, **Codeplug Diff**, **Records**.
- **Radio** - COM port entry, native Read/Write with a busy dialog and a log.

Edits update the in-memory codeplug and are written with **File > Save** (or
Save As). The Radio page uses `opendepot::RadioSession` (in-tree PPP/RCMP), not
the vendor helper or Python. Writes are validated by the completeness guard and
require explicit confirmation. The app accepts a codeplug path on the command
line.

## CLI

The core builds `opendepot-cli`. The clean-room radio commands are:

```text
opendepot-cli --read-radio-native <port> <out.cpg>
opendepot-cli --read-radio-native <port> <template.cpg> <out.cpg>
opendepot-cli --write-radio-native <port> <in.cpg>
```

The older commands remain for recovery/compatibility:

```text
opendepot-cli --read-radio <port> <out.cpg>
opendepot-cli --read-radio <port> <template.cpg> <out.cpg>
opendepot-cli --write-radio <port> <in.cpg>
opendepot-cli --check-write <in.cpg>
opendepot-cli --verify <in.cpg>
opendepot-cli --dump <in.cpg>
opendepot-cli --records <in.cpg>
opendepot-cli --clone <in.cpg> <out.cpg>
opendepot-cli --models
opendepot-cli --zones <in.cpg>
opendepot-cli --set-zone-name <in.cpg> <out.cpg> <zoneIndex> <name>
opendepot-cli --set-channel-name <in.cpg> <out.cpg> <zoneIndex> <chanIndex> <name>
opendepot-cli --set-channel-freq <in.cpg> <out.cpg> <zoneIndex> <chanIndex> <rxMHz> <txMHz>
opendepot-cli --set-identity <in.cpg> <out.cpg> <serial> <model>
opendepot-cli --password <in.cpg>
opendepot-cli --set-password <in.cpg> <out.cpg> <password>
opendepot-cli --clear-password <in.cpg> <out.cpg>
opendepot-cli --flashcode <in.cpg> [radio|platform]
opendepot-cli --set-flashcode <in.cpg> <out.cpg> <flashcode>
opendepot-cli --flashcode-options <radio|platform>
opendepot-cli --build-flashcode <radio|platform> <base> <code,code,...>
opendepot-cli --create <template.cpg> <out.cpg> <serial> <model> <flashcode>
opendepot-cli --create-new <out.cpg> <model> <serial> [flashcode]
opendepot-cli --conventional <in.cpg> <out.cpg>
opendepot-cli --create-conventional <template.cpg> <out.cpg> <serial> <model>
opendepot-cli --rec <in.cpg> <out.rec>
opendepot-cli --from-rec <template.cpg> <in.rec> <out.cpg>
opendepot-cli --taglist <in.cpg> <out.bin>
opendepot-cli --ishlist <in.cpg> <out.bin>
opendepot-cli --report <in.cpg> [--json]
opendepot-cli --radio-report <port> [--json]
opendepot-cli --doctor <in.cpg> [<out.cpg>]
opendepot-cli --diff <a.cpg> <b.cpg> [--json]
opendepot-cli --copy-zone <src.cpg> <srcZone> <dst.cpg> <dstZone> <out.cpg>
opendepot-cli --copy-zones <src.cpg> <srcZones> <dst.cpg> <dstZones> <out.cpg> [--trunking]
opendepot-cli --backup-radio <port> <outdir>
opendepot-cli --erase-radio <port> [--yes-erase]
opendepot-cli --replace-radio <port> <old.cpg> <new.cpg>
```

- `--verify` returns 0 when every record verifies, 3 when any record is bad.
- `--check-write` returns 0 when safe to write, 3 when refused.
- `--read-radio-native` / `--write-radio-native` use the in-tree transport. The
  native read has a baked complete request set for `H46UCF9PW6BN`; pass a
  codeplug as a template to override it for another model.
- `--read-radio` / `--write-radio` use the legacy helper/Python paths. They
  remain for recovery until the native path is validated on more models.
- `--erase-radio` and `--replace-radio` are destructive and gated; they use the
  legacy vendor helper.

## Radio transport

The core implements the programming link natively:

1. AT modem bringup: `AT&V`, `AT#DATLCL=1`, `AT+IPR=115200`, `CLIENT`.
2. PPP (LCP/IPCP; host `192.168.128.2`, radio `192.168.128.1`).
3. TCP to `192.168.128.1:8002`, with sequence-aware receive.
4. RCMP framing, ISH reads/writes, and the recovered auth challenge/response.
5. **Reads** use a read-only preamble (`01 06 00`, version/serial, challenge) and
   end with `01 06 01`; they do **not** send program-mode config or the reorg
   command. An early version did, and that could rebuild an inconsistent ISH
   directory.
6. **Writes** authenticate, enter program mode, write the full record set
   (identity first, `0x0047` last), reorg, and end.
7. Every session returns the radio to AT mode (`ATZ` / `AT+IPR=9600`) and
   force-resets on failure.

The legacy `tools/radio_tcp.py` and `tools/radio_helper.exe` are kept for
recovery and for `--erase-radio` / `--replace-radio`. `docs/RCMP_INTERNAL.md`
has the frame details.

## Write safety

- `--check-write` validates identity, record lengths/CRCs, the ISH directory, and
  the required tag set for a hardware-tested model. An incomplete set is refused
  with the missing tags.
- A native write also validates before sending anything, and the GUI requires an
  explicit confirmation.
- Always keep a **verified** backup for the exact model/serial, and never force a
  refused write.
- Read-back after a write verifies the identity; compare a full native read for
  every record when you need certainty.
- See `docs/SAFETY.md` for the failure analysis and preflight rules.

## Supported models (built-in model database)

Only the **800 MHz XTS 2500** has been tested on hardware; the rest are untested
— treat them as experimental.

| Model (example) | Radio | Band | Tested |
|---|---|---|---|
| `H46UCF9PW6BN` | XTS 2500 | 800 MHz | **yes** |
| `H46WCF9PW6BN` | XTS 2500 | 900 MHz | no |
| `H46QDD9PW5BN` | XTS 2500 | UHF | no |
| `H46KDD9PW5BN` | XTS 2500 | VHF | no |
| `H18UCC9PW5AN` | XTS 5000 | 800 MHz | no |
| `H66UCC9PW5AN` | XTS 1500 | 800 MHz | no |
| `H67UCC9PW5AN` | MT 1500 | 800 MHz | no |
| `H10SDD9PW6AN` | SSE 5000 | 800 MHz | no |
| `H79KDC9PW5AN` | PR 1500 | 800 MHz | no |
| `H18KCN9PW9AN` | XTS 4000 | 800 MHz | no |

Models are matched by model number; unrelated variants may share some records
but have not been validated.

## Tested hardware

- **One XTS 2500 (Model II), model `H46UCF9PW6BN`, 800 MHz trunking, Host/DSP
  R17.00.02.**
- Native read: 137 records, all CRC-ok, ~20 s. Native write: 137 records, ~17 s,
  identical read-back, radio back to AT mode.
- No other model has been read from or written to a real radio.

## Limitations

- **Codeplug layer only.** No firmware, no boot flash, no repair of a failed
  firmware update.
- **No from-scratch channel/personality/talkgroup authoring.** Creation clones a
  built-in or template codeplug, then changes identity/FLASHcode.
- **Writes are wholesale.** A write sends the entire record set and commits; a
  partial/failed write can corrupt the radio.
- **Reverse-engineered protocol.** The auth and write/commit sequence were
  recovered from observed traffic; a different firmware revision may behave
  differently.
- **Windows-only GUI and serial backend.** The core library is portable; the Qt
  GUI and the Win32 serial backend are not yet ported.
- **Untested breadth.** Only one radio model/band has been exercised end to end.
- **Native read coverage.** The baked request set is complete for
  `H46UCF9PW6BN`; other models fall back to the built-in template and the guard.
- **GUI editing is partial.** Radio-wide configuration forms and the create
  wizard are not implemented; those sections expose records and raw payloads.

## Codeplug passcode

The passcode is codeplug record `0x00510000` (partition `0x80`): `payload[0]` =
enabled, `payload[1..]` = ASCII password, zero padded. The CLI reads and edits it
directly from the codeplug:

- `--password <in.cpg>`
- `--set-password <in.cpg> <out.cpg> <password>`
- `--clear-password <in.cpg> <out.cpg>`

Set/clear only changes the codeplug; save or write applies it. Whether a radio
with a passcode set still allows a native read is untested.

## Service report, doctor, diff

CLI:

- `--report <in.cpg> [--json]` — identity, FLASHcode and features, passcode,
  zones/channels, record counts.
- `--radio-report <port> [--json]` — native read plus the `AT&V` banner.
- `--doctor <in.cpg> [<out.cpg>]` — header/directory/record checks; optional repair.
- `--diff <a.cpg> <b.cpg> [--json]` — identity, FLASHcode, records, zones.

The Qt GUI exposes the same report, doctor, and diff views.

## Backup and recovery

- `--backup-radio <port> <outdir>` writes `codeplug.cpg`, `softpots.bin`,
  `info.txt`, `security.bin` (when available), and a `manifest.json` with SHA-256
  hashes. It is read-only and never writes to the radio.
- To recover a radio with a damaged identity/directory, write a **verified
  complete** codeplug for the exact model/serial:
  `--write-radio-native <port> <known-good.cpg>` (or the legacy `--write-radio`).
- The completeness guard refuses a set that is missing required tags.

## Layout

- `ui/` — Qt 6 Widgets GUI (`MainWindow`, `theme.qss`, resources)
- `tools/radio_tcp.py` — legacy native reader/writer (recovery/reference)
- `tools/radio_helper.cs` — legacy 32-bit vendor-driver helper (erase/replace)
- `tools/gen_model_db.py`, `tools/gen_flashcode_db.py` — data generators
- `docs/` — container, protocol, safety, and feature notes
- The container, crypto, model/FLASHcode data, and native radio transport live in
  [`OpenDepotCore`](https://github.com/OwnYourAstro/OpenDepotCore).

## Disclaimer and license

- **No affiliation.** OpenDepot is an independent project. It is not affiliated
  with, endorsed by, or supported by Motorola Solutions.
- **No warranty.** The software is provided "as is", without warranty of any
  kind, express or implied. The authors are not liable for any damage to radios,
  data, or equipment. You use it entirely at your own risk.
- **Clean room.** The container, crypto, generators, GUI, and native transport
  are original. The optional erase/replace helper uses the vendor driver already
  installed on the machine.
- See `LICENSE` for the license terms (MIT).
