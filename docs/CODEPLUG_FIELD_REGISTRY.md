# Codeplug field registry

The portable Depot's field names are loaded through `CcgIshField`:

- `CcgIshField::CcgIshField(unsigned int id)` calls `CString::LoadStringA` on
  the `CString` at object offset `+4`.
- `CcgIshField::GetFieldName` returns that `CString`.
- The string resource ID is `field_id + 16` in `PatPortDepot.exe`.

Verified examples:

| Field ID | Resource | Name |
|---:|---:|---|
| `9271` | `9287` | Out of Range Indicator |
| `13335` | `13351` | Imbalanced Coverage Indicator |
| `25543` | `25559` | System Registration Indicator |
| `9425` | `9441` | Time format |
| `9426` | `9442` | Date Format |
| `11518` | `11534` | Channel Name |
| `11531` | `11547` | Receive Frequency |
| `11532` | `11548` | Transmit Frequency |

Coverage:

- **2,529 / 2,949** runtime fields named.
- **2,261** unique names.
- **420** remain unnamed; these are structural/container fields or lack a Depot
  string resource.
- All 2,949 rows retain layout group, payload offset, bit count, MSB, and the
  existing block mapping where available.

Table bindings:

- **124 / 140** table columns bind to a field ID by normalized CPS name.
- Zone Channel Assignment: 38/47 bound; primary group `0xf1a7ac0` (24 columns).
- Talkgroup Personality: 31/32 bound.
- ASTRO Talkgroup: 3/4 bound (Key Select Reference is manually resolved to
  `Key Select`).
- Remaining gaps are derived/display-only or missing names, including PL
  frequency/code, Test Tx/Rx frequency, Secure index/slots, and the `Current`
  feature column.

Generated artifacts:

- `docs/codeplug_field_schema.json` - names, groups, offsets, table keys.
- `docs/codeplug_table_schema.json` - table columns with field bindings.
- `docs/CODEPLUG_TABLE_BINDINGS.md` - column-by-column report.

Tools:

- `OpenDepotCore/tools/extract_pe_strings.py` - dump PE RT_STRING resources.
- `OpenDepotCore/tools/bind_table_fields.py` - join names and table columns.
- `OpenDepotCore/tools/gen_field_catalog.py` - generate the C++ registry.
- `OpenDepotCore/tools/decode_field_schema.py` - decode values from a model.

Reproduce from the installed Depot:

```powershell
python OpenDepotCore\tools\extract_pe_strings.py `
  "C:\Program Files (x86)\Motorola\ASTRO 25 Products\ASTRO 25 Portable Depot\PatPortDepot.exe" `
  "$env:TEMP\PatPortDepot.strings.json"

python OpenDepotCore\tools\bind_table_fields.py `
  --names "$env:TEMP\PatPortDepot.strings.json"
```

Example: `Zone Channel Assignment` group `0xf1a7ac0` maps:

| Table column | Field ID | Byte | Bits | MSB |
|---|---:|---:|---:|---:|
| Receive Frequency | `11531` | `12` | 32 | 7 |
| Transmit Frequency | `11532` | `8` | 32 | 7 |
| Channel Announcement | `11570` | `27` | 16 | 7 |
| Active Channel | `11557` | `23` | 1 | 5 |

`Channel Name` is a separate 136-bit name field (`fid 11518`, group
`0xf1a67a0`, byte 0).
