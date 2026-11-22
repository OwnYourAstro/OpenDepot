# Codeplug table bindings

Table columns joined to runtime field IDs by normalized CPS field name.
Each binding carries the layout group and payload offset used by the
Depot's `CcgIshDataLayout::AddField` objects.

- Field names: 2529/2949
- Fields assigned to a table group: 119/2949
- Tables: 18
- Columns: 140

| Table | Columns | Bound | Ambiguous | Missing | Primary group | Primary block |
|---|---:|---:|---:|---:|---|---|
| ASTRO 25 Channel ID Table in Trunking System | 6 | 6 | 0 | 0 | `0xede52f0` | 195 Trunking  Personality  Floating Data Block |
| ASTRO 25 Site Alias Table in Trunking System | 6 | 6 | 0 | 0 | `0xf19c420` | -  |
| Channel List Table in Zone Channel Assignment | 47 | 38 | 0 | 9 | `0xf1a7ac0` | -  |
| List in ASTRO Call List | 3 | 3 | 0 | 0 | `0x40abe18` | 15 Mdc Call Id Text blk |
| List in Trunking Call List | 4 | 4 | 0 | 0 | `0x107feb08` | -  |
| Narrow Bandwidth 12.5 kHz Only Frequency Range List | 2 | 2 | 0 | 0 | `0x107ff918` | -  |
| NAT List | 3 | 3 | 0 | 0 | `0xf1a69e0` | -  |
| OBT ASTRO 25 Control Channels Table in Trunking System | 6 | 6 | 0 | 0 | `0xebe85b0` | -  |
| OBT Control Channels Table in Trunking System | 4 | 4 | 0 | 0 | `0xebe87f0` | -  |
| Secure Multikey | 7 | 4 | 0 | 3 | `0xf62f030` | -  |
| SmartZone Preferred  Sites Table in Trunking Personality | 6 | 6 | 0 | 0 | `0xf19cc90` | -  |
| TalkGroup in ASTRO Talkgroup List | 4 | 4 | 0 | 0 | `0x40aba28` | 233 MDC Call ID Text List Distributed Header |
| Talkgroup Table in Trunking Personality | 32 | 31 | 0 | 1 | `0xebe75f0` | -  |
| Test Channel List | 2 | 0 | 0 | 2 | `-` | -  |
| Trunking Message Alias Table in Trunking System | 2 | 2 | 0 | 0 | `0xf1a6a70` | -  |
| Trunking Site Alias Table in Trunking System | 2 | 2 | 0 | 0 | `0xf19ce40` | -  |
| Trunking Status Alias Table in Trunking System | 2 | 2 | 0 | 0 | `0xf1a6cb0` | -  |
| View Radio Configuration Features | 2 | 1 | 0 | 1 | `-` | -  |

## ASTRO 25 Channel ID Table in Trunking System

Bound 6/6 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Identifier Enable | bound | 13406 | Identifier Enable | `0xede52f0` | 2 | 1 | 6 | 195 |
| 1 | Channel Bandwidth (kHz) | bound | 11255 | Channel Bandwidth (kHz) | `0xede52f0` | 0 | 16 | 7 | 195 |
| 2 | Transmit Offset Sign | bound | 11306 | Transmit Offset Sign | `0xede52f0` | 2 | 1 | 7 | 195 |
| 3 | Transmit Offset (MHz) | bound | 11256 | Transmit Offset (MHz) | `0xede52f0` | 2 | 13 | 4 | 195 |
| 4 | Channel Spacing (kHz) | bound | 11257 | Channel Spacing (kHz) | `0xede52f0` | 4 | 16 | 7 | 195 |
| 5 | Base Frequency (MHz) | bound | 11258 | Base Frequency (MHz) | `0xede52f0` | 6 | 32 | 7 | 195 |

## ASTRO 25 Site Alias Table in Trunking System

Bound 6/6 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Rfss Alias Num | bound | 11162 | Rfss Alias Num | `0xf19c420` | 0 | 8 | 7 | - |
| 1 | Site Alias Num | bound | 11163 | Site Alias Num | `0xf19c420` | 1 | 8 | 7 | - |
| 2 | Site Alias Txt | bound | 11164 | Site Alias Txt | `0xf19ca50` | 0 | 136 | 7 | - |
| 3 | Site Alias Type | bound | 13867 | Site Alias Type | `0xf19c420` | 8 | 2 | 1 | - |
| 4 | WACN ID | bound | 13868 | WACN ID | `0xf19c420` | 4 | 32 | 7 | - |
| 5 | System ID | bound | 13833 | System ID  | `0xf19c420` | 2 | 16 | 7 | - |

## Channel List Table in Zone Channel Assignment

Bound 38/47 columns; ambiguous 0; missing 9.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Active Channel | bound | 11557 | Active Channel | `0xf1a7ac0` | 23 | 1 | 5 | - |
| 1 | Channel Name | bound | 11518 | Channel Name | `0xf1a67a0` | 0 | 136 | 7 | - |
| 2 | Channel Announcement | bound | 11570 | Channel Announcement | `0xf1a7ac0` | 27 | 16 | 7 | - |
| 3 | Personality Type | bound | 11501 | Personality Type | `0xf1a77f0` | 0 | 16 | 7 | - |
| 4 | Personality # | bound | 11515 | Personality # | `0xf1a77f0` | 2 | 16 | 7 | - |
| 5 | Talkgroup # | bound | 10672 | Talkgroup | `0xebe8be0` | 0 | 16 | 7 | - |
| 6 | TalkGroup | bound | 10672 | Talkgroup | `0xebe8be0` | 0 | 16 | 7 | - |
| 7 | Strapping | missing | - | - | - | - | - | - | - |
| 8 | User Selectable PL (MPL) | bound | 11586 | User Selectable PL (MPL) | `0xf1a7ac0` | 23 | 1 | 3 | - |
| 9 | Receive Frequency | bound | 11531 | Receive Frequency | `0xf1a7ac0` | 12 | 32 | 7 | - |
| 10 | Receive Frequency | bound | 11531 | Receive Frequency | `0xf1a7ac0` | 12 | 32 | 7 | - |
| 11 | Receive Squelch Type | bound | 11538 | Receive Squelch Type | `0xf1a7ac0` | 21 | 1 | 7 | - |
| 12 | Receive PL Freq | missing | - | - | - | - | - | - | - |
| 13 | Receive PL Code | missing | - | - | - | - | - | - | - |
| 14 | Receive DPL Code | bound | 11535 | Receive DPL Code | `0xf1a7ac0` | 21 | 7 | 6 | - |
| 15 | Receive DPL Invert | bound | 11533 | Receive DPL Invert | `0xf1a7ac0` | 23 | 1 | 0 | - |
| 16 | Transmit Frequency | bound | 11532 | Transmit Frequency | `0xf1a7ac0` | 8 | 32 | 7 | - |
| 17 | Transmit Frequency | bound | 11532 | Transmit Frequency | `0xf1a7ac0` | 8 | 32 | 7 | - |
| 18 | Transmit Squelch Type | bound | 11542 | Transmit Squelch Type | `0xf1a7ac0` | 20 | 1 | 7 | - |
| 19 | Transmit PL Freq | missing | - | - | - | - | - | - | - |
| 20 | Transmit PL Code | missing | - | - | - | - | - | - | - |
| 21 | Transmit DPL Code | bound | 11539 | Transmit DPL Code | `0xf1a7ac0` | 20 | 7 | 6 | - |
| 22 | Transmit DPL Invert | bound | 11534 | Transmit DPL Invert | `0xf1a7ac0` | 23 | 1 | 1 | - |
| 23 | PL Deviation | bound | 11569 | PL Deviation | `0xf1a7ac0` | 1 | 4 | 7 | - |
| 24 | Tx Opt Transmit Dev | bound | 11543 | Tx Opt Transmit Dev | `0xf1a7ac0` | 1 | 2 | 1 | - |
| 25 | Tx Opt Transmit Dev T/A | bound | 11544 | Tx Opt Transmit Dev T/A | `0xf1a7ac0` | 1 | 2 | 3 | - |
| 26 | Tx Opt Channel Spacing | bound | 11545 | Tx Opt Channel Spacing | `0xf1a7ac0` | 0 | 8 | 7 | - |
| 27 | TA Direct/Talkaround | bound | 11548 | TA Direct/Talkaround | `0xf1a7ac0` | 23 | 1 | 6 | - |
| 28 | TA Frequency | bound | 11546 | TA Frequency | `0xf1a7ac0` | 16 | 32 | 7 | - |
| 29 | TA Frequency | bound | 11546 | TA Frequency | `0xf1a7ac0` | 16 | 32 | 7 | - |
| 30 | TA Squelch Type | bound | 11552 | TA Squelch Type | `0xf1a7ac0` | 22 | 1 | 7 | - |
| 31 | TA PL Frequency | missing | - | - | - | - | - | - | - |
| 32 | TA PL Code | missing | - | - | - | - | - | - | - |
| 33 | TA DPL Code | bound | 11549 | TA DPL Code | `0xf1a7ac0` | 22 | 7 | 6 | - |
| 34 | TA DPL Invert | bound | 11547 | TA DPL Invert | `0xf1a7ac0` | 23 | 1 | 2 | - |
| 35 | Direct Network ID | bound | 11528 | Direct Network ID | `0xf1a7ac0` | 6 | 16 | 7 | - |
| 36 | Tx Network ID | bound | 11529 | Tx Network ID | `0xf1a7ac0` | 2 | 16 | 7 | - |
| 37 | Rx Network ID | bound | 11530 | Rx Network ID | `0xf1a7ac0` | 4 | 16 | 7 | - |
| 38 | Sector ID | bound | 11568 | Sector ID | `0xf1a7ac0` | 24 | 8 | 7 | - |
| 39 | Rx Only | missing | - | - | - | - | - | - | - |
| 40 | Rx Voice/Signal Type | bound | 4539 | Rx Voice/Signal Type | `0x40aa9d8` | 26 | 2 | 3 | 187 |
| 41 | Surveillance Mode | bound | 4540 | Surveillance Mode | `0x40aa9d8` | 5 | 1 | 1 | 187 |
| 42 | Adaptive Power | bound | 4501 | Adaptive Power | `0x40aa9d8` | 23 | 1 | 3 | 187 |
| 43 | Secure/Clear Strapping | bound | 4686 | Secure/Clear Strapping | `0x40aa9d8` | 10 | 2 | 1 | 187 |
| 44 | Fireground Enable | bound | 4713 | Fireground Enable | `0x40aa9d8` | 26 | 1 | 0 | 187 |
| 45 | Fireground Registration | bound | 4715 | Fireground Registration | `0x40aa9d8` | 27 | 1 | 7 | 187 |
| 46 | System Type | missing | - | - | - | - | - | - | - |

## List in ASTRO Call List

Bound 3/3 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Type | bound | 3516 | Type | `0x40abe18` | 0 | 1 | 7 | 15 |
| 1 | Call ID | bound | 3518 | Call ID | `0x40abe18` | 0 | 31 | 6 | 15 |
| 2 | ASTRO Call ID Text | bound | 3698 | ASTRO Call ID Text  | `0x40ab878` | 0 | 136 | 7 | - |

## List in Trunking Call List

Bound 4/4 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Call List ID | bound | 13057 | Type II Trk Call List Id CP | `0xede59b0` | 0 | 24 | 7 | 153 |
| 1 | Call List Text | bound | 10109 | Call List Text | `0xede5bf0` | 0 | 136 | 7 | - |
| 2 | WACN ID | bound | 25400 | WACN ID | `0x107feb08` | 0 | 32 | 7 | - |
| 3 | System ID | bound | 25401 | System ID | `0x107feb08` | 4 | 16 | 7 | - |

## Narrow Bandwidth 12.5 kHz Only Frequency Range List

Bound 2/2 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | FCC Narrow Banding Start Frequency(MHz) | bound | 12036 | FCC Narrow Banding Start Frequency | `0x107ff918` | 0 | 32 | 7 | - |
| 1 | FCC Narrow Banding End Frequency(MHz) | bound | 12037 | FCC Narrow Banding End Frequency | `0x107ff918` | 4 | 32 | 7 | - |

## NAT List

Bound 3/3 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | WAN Port | bound | 13552 | WAN Port | `0xf1a69e0` | 0 | 16 | 7 | - |
| 1 | LAN Port | bound | 13553 | LAN Port | `0xf1a69e0` | 2 | 16 | 7 | - |
| 2 | Static NAT IP Address | composite | 13557 | Static NAT IP Address 1 | `0xf1a69e0` | 4 | 8 | 7 | - |
| 2 | Static NAT IP Address | composite | 13558 | Static NAT IP Address 2 | `0xf1a69e0` | 5 | 8 | 7 | - |
| 2 | Static NAT IP Address | composite | 13559 | Static NAT IP Address 3 | `0xf1a69e0` | 6 | 8 | 7 | - |
| 2 | Static NAT IP Address | composite | 13560 | Static NAT IP Address 4 | `0xf1a69e0` | 7 | 8 | 7 | - |

## OBT ASTRO 25 Control Channels Table in Trunking System

Bound 6/6 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Chan Bandwidth (kHz) | bound | 11395 | Chan Bandwidth (kHz) | `0xebe85b0` | 1 | 8 | 7 | - |
| 1 | Tx Deviation (kHz) | bound | 11397 | Tx Deviation (kHz) | `0xebe85b0` | 0 | 2 | 1 | - |
| 2 | Rx Freq (MHz) | bound | 11393 | Rx Freq (MHz) | `0xebe85b0` | 2 | 32 | 7 | - |
| 3 | Rx Freq (MHz) | bound | 11393 | Rx Freq (MHz) | `0xebe85b0` | 2 | 32 | 7 | - |
| 4 | Tx Freq (MHz) | bound | 11394 | Tx Freq (MHz) | `0xebe85b0` | 6 | 32 | 7 | - |
| 5 | Tx Freq (MHz) | bound | 11394 | Tx Freq (MHz) | `0xebe85b0` | 6 | 32 | 7 | - |

## OBT Control Channels Table in Trunking System

Bound 4/4 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Tx Frequency | bound | 10968 | Tx Frequency | `0xebe87f0` | 1 | 16 | 7 | - |
| 1 | Tx Frequency | bound | 10968 | Tx Frequency | `0xebe87f0` | 1 | 16 | 7 | - |
| 2 | Rx Frequency | bound | 10967 | Rx Frequency | `0xebe87f0` | 3 | 16 | 7 | - |
| 3 | Rx Frequency | bound | 10967 | Rx Frequency | `0xebe87f0` | 3 | 16 | 7 | - |

## Secure Multikey

Bound 4/7 columns; ambiguous 0; missing 3.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | CKR # | bound | 9814 | CKR # | `0xf62ff60` | 0 | 16 | 7 | - |
| 1 | Key Name | bound | 9709 | Key Name | `0xf62fed0` | 0 | 136 | 7 | - |
| 2 | Indexed | missing | - | - | - | - | - | - | - |
| 3 | Slot A | missing | - | - | - | - | - | - | - |
| 4 | Slot B | missing | - | - | - | - | - | - | - |
| 5 | Selectable ADP Key ID | bound | 14041 | Selectable ADP Key ID | `0xf62f030` | 0 | 32 | 7 | - |
| 6 | Selectable ADP Key Data | bound | 14042 | Selectable ADP Key Data | `0xf62f030` | 4 | 80 | 7 | - |

## SmartZone Preferred  Sites Table in Trunking Personality

Bound 6/6 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Site ID (Decimal) | bound | 11670 | Site ID (Hex) | `0xf19cc90` | 2 | 8 | 7 | - |
| 1 | Site ID (Hex) | bound | 11670 | Site ID (Hex) | `0xf19cc90` | 2 | 8 | 7 | - |
| 2 | Preferred Status | bound | 10616 | Preferred Status | `0xf19c8a0` | 0 | 2 | 7 | - |
| 3 | Type II Sys ID - RFSS ID | bound | 10761 | Type II Sys ID - RFSS ID | `0xf19cc90` | 1 | 8 | 7 | - |
| 4 | WACN ID | bound | 14611 | WACN ID | `0xf19cc90` | 5 | 32 | 7 | - |
| 5 | P25 System ID | bound | 10875 | P25 System ID | `0xf19cc90` | 3 | 16 | 7 | - |

## TalkGroup in ASTRO Talkgroup List

Bound 4/4 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Talkgroup ID | bound | 3510 | Talkgroup ID | `0x40aba28` | 0 | 16 | 7 | 233 |
| 1 | Talkgroup Alias Text | bound | 3511 | Talkgroup Alias Text | `0x40abc68` | 0 | 136 | 7 | - |
| 2 | Voice Secure / Clear Strapping | bound | 3727 | Voice Secure / Clear Strapping | `0x40aba28` | 2 | 2 | 1 | 233 |
| 3 | Key Select Reference | bound | 3502 | Key Select | `0x40aba28` | 3 | 16 | 7 | 233 |

## Talkgroup Table in Trunking Personality

Bound 31/32 columns; ambiguous 0; missing 1.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | ASTRO 25 Talkgroup | bound | 10857 | ASTRO 25 Talkgroup | `0xebe8d90` | 0 | 16 | 7 | - |
| 1 | Talkgroup | bound | 10672 | Talkgroup | `0xebe8be0` | 0 | 16 | 7 | - |
| 2 | Failsoft | bound | 10526 | Failsoft | `0xebe7cb0` | 0 | 1 | 7 | - |
| 3 | F/S Type II Rx Frequency | bound | 11730 | F/S Type II Rx Frequency | `0xebe7cb0` | 1 | 16 | 7 | - |
| 4 | F/S Type II Rx Frequency | bound | 11730 | F/S Type II Rx Frequency | `0xebe7cb0` | 1 | 16 | 7 | - |
| 5 | Rebanded F/S Type II Rx Frequency | bound | 11766 | Rebanded F/S Type II Rx Frequency | `0xebe7cb0` | 3 | 16 | 7 | - |
| 6 | Rebanded F/S Type II Rx Frequency | bound | 11766 | Rebanded F/S Type II Rx Frequency | `0xebe7cb0` | 3 | 16 | 7 | - |
| 7 | Channel ID Number | bound | 10865 | Channel ID Number | `0xebe7830` | 0 | 4 | 3 | - |
| 8 | F/S ASTRO 25 Rx Frequency | bound | 11727 | F/S ASTRO 25 Rx Frequency | `0xebe7830` | 1 | 16 | 7 | - |
| 9 | F/S ASTRO 25 Rx Frequency | bound | 11727 | F/S ASTRO 25 Rx Frequency | `0xebe7830` | 1 | 16 | 7 | - |
| 10 | F/S ASTRO 25 Tx Frequency | bound | 11717 | F/S ASTRO 25 Tx Freq (MHz) | `0xebe75f0` | 6 | 32 | 7 | - |
| 11 | Rebanded Channel ID Number | bound | 11771 | Rebanded Channel ID Number | `0xebe78c0` | 3 | 4 | 3 | - |
| 12 | Rebanded F/S ASTRO 25 Rx Frequency | bound | 11768 | Rebanded F/S ASTRO 25 Rx Frequency | `0xebe78c0` | 4 | 16 | 7 | - |
| 13 | Rebanded F/S ASTRO 25 Rx Frequency | bound | 11768 | Rebanded F/S ASTRO 25 Rx Frequency | `0xebe78c0` | 4 | 16 | 7 | - |
| 14 | Rebanded F/S ASTRO 25 Tx Frequency | missing | - | - | - | - | - | - | - |
| 15 | OBT Failsoft Enable | bound | 10556 | OBT Failsoft Enable | `0xebe7e60` | 0 | 1 | 6 | - |
| 16 | F/S Type II Rx Freq | bound | 10571 | F/S Type II Rx Freq | `0xebe7e60` | 3 | 16 | 7 | - |
| 17 | F/S Type II Rx Freq | bound | 10571 | F/S Type II Rx Freq | `0xebe7e60` | 3 | 16 | 7 | - |
| 18 | F/S Type II Tx Freq | bound | 10572 | F/S Type II Tx Freq | `0xebe7e60` | 1 | 16 | 7 | - |
| 19 | F/S Type II Tx Freq | bound | 10572 | F/S Type II Tx Freq | `0xebe7e60` | 1 | 16 | 7 | - |
| 20 | Secure/Clear Strapping | bound | 10609 | Secure/Clear Strapping | `0xebe8be0` | 2 | 2 | 1 | - |
| 21 | Tx Voice/Signal Type | bound | 10866 | Tx Voice/Signal Type | `0xebe8be0` | 2 | 1 | 3 | - |
| 22 | Key Select | bound | 10858 | Key Select | `0xebe8be0` | 3 | 16 | 7 | - |
| 23 | F/S Enable | bound | 11719 | F/S Enable | `0xebe75f0` | 0 | 1 | 7 | - |
| 24 | F/S Chan Bandwidth (kHz) | bound | 11718 | F/S Chan Bandwidth (kHz) | `0xebe75f0` | 1 | 8 | 7 | - |
| 25 | F/S Tx Deviation (kHz) | bound | 11720 | F/S Tx Deviation (kHz) | `0xebe75f0` | 0 | 2 | 1 | - |
| 26 | F/S ASTRO 25 Rx Freq (MHz) | bound | 11716 | F/S ASTRO 25 Rx Freq (MHz) | `0xebe75f0` | 2 | 32 | 7 | - |
| 27 | F/S ASTRO 25 Rx Freq (MHz) | bound | 11716 | F/S ASTRO 25 Rx Freq (MHz) | `0xebe75f0` | 2 | 32 | 7 | - |
| 28 | F/S ASTRO 25 Tx Freq (MHz) | bound | 11717 | F/S ASTRO 25 Tx Freq (MHz) | `0xebe75f0` | 6 | 32 | 7 | - |
| 29 | F/S ASTRO 25 Tx Freq (MHz) | bound | 11717 | F/S ASTRO 25 Tx Freq (MHz) | `0xebe75f0` | 6 | 32 | 7 | - |
| 30 | WACN ID | bound | 14610 | WACN ID | `0xebe8d90` | 7 | 32 | 7 | - |
| 31 | P25 System ID | bound | 14601 | P25 System ID | `0xebe8d90` | 5 | 16 | 7 | - |

## Test Channel List

Bound 0/2 columns; ambiguous 0; missing 2.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Test Tx Freq(MHz) | missing | - | - | - | - | - | - | - |
| 1 | Test Rx Freq(MHz) | missing | - | - | - | - | - | - | - |

## Trunking Message Alias Table in Trunking System

Bound 2/2 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Message Alias Number | bound | 11101 | Message Alias Number | `0xebe8760` | 0 | 8 | 7 | - |
| 1 | Message Alias Text | bound | 11116 | Message Alias Text | `0xebe8910` | 0 | 136 | 7 | - |

## Trunking Site Alias Table in Trunking System

Bound 2/2 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Site Alias Number | bound | 11102 | Site Alias Number | `0xf19ce40` | 1 | 8 | 7 | - |
| 1 | Site Alias Text | bound | 11117 | Site Alias Text | `0xf19c810` | 0 | 136 | 7 | - |

## Trunking Status Alias Table in Trunking System

Bound 2/2 columns; ambiguous 0; missing 0.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | Status Alias Number | bound | 11103 | Status Alias Number | `0xebe81c0` | 0 | 8 | 7 | - |
| 1 | Status Alias Text | bound | 11118 | Status Alias Text | `0xebe7b00` | 0 | 136 | 7 | - |

## View Radio Configuration Features

Bound 1/2 columns; ambiguous 0; missing 1.

| # | Column | Match | Field | Name | Group | Off | Bits | MSB | Block |
|---:|---|---|---:|---|---|---:|---:|---:|---|
| 0 | FLASHcode Information | bound | 14152 | FLASHcode | `0xf62f6f0` | 8 | 128 | 7 | - |
| 1 | Current | missing | - | - | - | - | - | - | - |
