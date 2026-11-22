# Codeplug record catalog

Derived from 33 codeplugs (29 radio backups and 4 fixtures) with
`OpenDepotCore/tools/analyze_schema.py`. Regenerate the raw dumps with
`opendepot-cli --hexall` / `--records` / `--model`, then:

```sh
python tools/analyze_schema.py --dir <dump-dir> --out <dump-dir>/schema
```

- files: 33
- distinct tags: 149
- records with no section: 0

## Files

| file | model | serial | records | tags |
|---|---|---|---|---|
| 205CNX2873-test-radio-backup | H46UCF9PW6BN | 205CNX2873 | 139 | 98 |
| MT1500-conv | H46WCC9PW5BN | 123ABC1234 | 92 | 89 |
| PR1500-conv | H46WCC9PW5BN | 123ABC1234 | 92 | 89 |
| SSE5000-conv | H46WCC9PW5BN | 123ABC1234 | 92 | 89 |
| XTS1500-conv | H46WCC9PW5BN | 123ABC1234 | 92 | 89 |
| XTS2000_ATS3000-conv | H46WCC9PW5BN | 123ABC1234 | 92 | 89 |
| XTS2500-conv | H46UCF9PW6BN | 123ABC1234 | 99 | 96 |
| XTS2500_UHF | H46QDD9PW5BN | 123ABC1234 | 133 | 130 |
| XTS2500_VHF | H46KDD9PW5BN | 123ABC1234 | 133 | 130 |
| XTS4000-conv | H46WCC9PW5BN | 123ABC1234 | 92 | 89 |
| current-radio | H46UCF9PW6BN | 205CNX2873 | 85 | 85 |
| current-radio2 | H46UCF9PW6BN | 205CNX2873 | 86 | 86 |
| mt1500-800 | H67UCC9PW5AN | TEST000003 | 92 | 89 |
| post-full-q387 | H46UCF9PW6BN | 205CNX2873 | 85 | 85 |
| post-fullflash | H46UCF9PW6BN | 205CNX2873 | 146 | 96 |
| post-gen-trunk | H46UCF9PW6BN | 205CNX2873 | 143 | 143 |
| post-q387-reorder | H46UCF9PW6BN | 205CNX2873 | 137 | 96 |
| post-reorg | H46UCF9PW6BN | 205CNX2873 | 3 | 3 |
| post-zone | H46UCF9PW6BN | 205CNX2873 | 137 | 96 |
| post-zone-commit | H46UCF9PW6BN | 205CNX2873 | 137 | 96 |
| postwrite-q387 | H46UCF9PW6BN | 205CNX2873 | 137 | 96 |
| postwrite-q387-map | H46UCF9PW6BN | 205CNX2873 | 137 | 96 |
| prewrite-205CNX2873-20261108-233727 | H46UCF9PW6BN | 205CNX2873 | 85 | 85 |
| prewrite-205CNX2873-20261109-010020 | H46UCF9PW6BN | 205CNX2873 | 143 | 143 |
| prewrite-205CNX2873-20261109-011900 | H46UCF9PW6BN | 205CNX2873 | 145 | 144 |
| prewrite-q387 | H46UCF9PW6BN | 205CNX2873 | 137 | 96 |
| test-q387 | H46UCF9PW6BN | 205CNX2873 | 137 | 96 |
| test-q387-map | H46UCF9PW6BN | 205CNX2873 | 137 | 96 |
| test-zone | H46UCF9PW6BN | 205CNX2873 | 137 | 96 |
| xts2500-800 | H46UCF9PW6BN | TEST000001 | 147 | 144 |
| xts2500-edited | H46UCF9PW6BN | TEST000001 | 147 | 144 |
| xts2500_trunking_template | H46UCF9PW6BN | 123ABC1234 | 147 | 144 |
| xts5000-800 | H18UCC9PW5AN | TEST000002 | 141 | 138 |

## Tags

| tag | files | lengths | section | node family | families |
|---|---|---|---|---|---|
| 0x000A | 32 | 158 | radio_wide | references | refs(74:003C,112:003C) |
| 0x000B | 32 | 16 | radio_wide | references |  |
| 0x000C | 32 | 274 | radio_wide | blob | refs(168:0030); strings("01234567","01234567") |
| 0x000D | 32 | 25,382 | zone_channel_assignment | name_table | name_table_17(count=1); refs(4:0032) |
| 0x0014 | 32 | 32 | radio_wide | blob | refs(20:0026,26:0026); strings("&&&&&","&H&&&","&&&&&","&&&&&") |
| 0x0015 | 10 | 32 | trunking_config | blob | refs(20:0026,26:0026); strings("&&&&&","&H&&&","&&&&&","&&&&&") |
| 0x0016 | 22 | 10,11,12 | radio_wide | blob |  |
| 0x0017 | 7 | 10,12 | trunking_config | blob |  |
| 0x0018 | 32 | 5 | radio_wide | references |  |
| 0x0019 | 32 | 5,9 | radio_wide | references |  |
| 0x001A | 10 | 5,9 | trunking_config | references |  |
| 0x001B | 32 | 106 | controls | blob | refs(4:0031,6:0031,8:002D); strings("30"1"2"3"4"5"6"7"8"9":";"C","PWQXR@SAp") |
| 0x0020 | 32 | 22 | radio_wide | blob |  |
| 0x0021 | 1 | 20 | trunking_config | blob | strings("c#d#e#f#g#h#") |
| 0x0022 | 10 | 22 | trunking_config | blob | strings("`#a#b#") |
| 0x0023 | 1 | 20 | trunking_config | blob | strings("c#d#e#f#g#h#") |
| 0x0026 | 32 | 10 | display_menu | blob | refs(2:0014); strings("&&&&P") |
| 0x0027 | 32 | 63 | display_menu | references | refs(4:0040) |
| 0x0028 | 32 | 9 | radio_wide | blob | refs(4:000C) |
| 0x0029 | 32 | 56,80 | narrow_band_list | blob |  |
| 0x002A | 32 | 5 | radio_wide | blob |  |
| 0x002C | 32 | 45 | radio_wide | references | refs(40:000A) |
| 0x002D | 32 | 33 | radio_wide | blob | strings("        ","                    ") |
| 0x002F | 32 | 8 | vrs_ep | blob |  |
| 0x0030 | 32 | 8 | vrs_ep | blob |  |
| 0x0031 | 32 | 8 | vrs_ep | references | refs(4:0020) |
| 0x0032 | 32 | 8 | vrs_ep | blob |  |
| 0x0033 | 32 | 8 | vrs_ep | references | refs(4:0020) |
| 0x0034 | 32 | 16 | secure | blob | refs(12:0057) |
| 0x003A | 10 | 216 | trunking_system | references | refs(2:000D,30:0FFF,56:0FFF,82:0FFF,108:0FFF,134:0FFF,160:0FFF,186:0FFF) |
| 0x003B | 10 | 23 | trunking_system | references | refs(14:0FFF) |
| 0x003C | 10 | 112 | trunking_system | blob | refs(2:000D) |
| 0x003D | 10 | 23 | trunking_system | blob |  |
| 0x003E | 10 | 29 | trunking_system | blob |  |
| 0x003F | 10 | 19 | trunking_system | blob | strings("4@FPxP") |
| 0x0040 | 32 | 24 | conventional_config | blob |  |
| 0x0047 | 32 | 44 | codeplug_directory | blob | refs(42:0014) |
| 0x0048 | 32 | 23 | radio_wide | blob | strings("ID: ","EM: ","CA: ") |
| 0x0049 | 32 | 31 | phone | references | refs(4:0032,6:0014,8:0FCF) |
| 0x004A | 32 | 12 | phone | references | refs(4:0032,6:0014,8:0FCE) |
| 0x004B | 32 | 12,16 | phone | blob |  |
| 0x004C | 32 | 25 | scan_list | name_table | name_table_17(count=1); refs(4:0032); strings("LIST 1") |
| 0x004D | 10 | 25 | scan_list | name_table | name_table_17(count=1); refs(4:0032); strings("LIST 1") |
| 0x004E | 8 | 25 | scan_list | name_table | name_table_17(count=1); refs(4:0032); strings("LIST 1") |
| 0x004F | 32 | 25 | scan_list | name_table | name_table_17(count=1); refs(4:0032); strings("LIST 1") |
| 0x0050 | 32 | 10 | conventional_config | blob |  |
| 0x0051 | 32 | 33 | passcode | blob |  |
| 0x0052 | 1 | 32 | trunking_personality | blob |  |
| 0x0053 | 1 | 48 | trunking_personality | blob |  |
| 0x0054 | 32 | 16 | scan_config | blob |  |
| 0x0057 | 32 | 12 | secure | blob |  |
| 0x0058 | 22 | 13 | radio_wide | references | refs(8:0FC2) |
| 0x0059 | 22 | 280 | status_alias | name_table | name_table_17(count=16); refs(24:0053,58:0053,92:0053,126:0053,160:0053,194:0053,228:0053,262:0053); strings("STATUS 1","STATUS 2","STATUS 3","STATUS 4") |
| 0x005A | 22 | 12 | phone | references | refs(8:0FC1) |
| 0x005B | 22 | 280 | message_alias | name_table | name_table_17(count=16); refs(24:004D,58:004D,92:004D,126:004D,160:004D,194:004D,228:004D,262:004D); strings("MESSAGE 1","MESSAGE 2","MESSAGE 3","MESSAGE 4") |
| 0x005F | 32 | 10 | scan_config | blob |  |
| 0x0064 | 32 | 15 | mpl | references | refs(4:0040,8:0FBD) |
| 0x0065 | 32 | 12 | mpl | references | refs(4:0040,8:0FBC) |
| 0x006C | 32 | 25 | scan_list | name_table | name_table_17(count=1); refs(4:0032); strings("LIST 1") |
| 0x006E | 32 | 25 | scan_list | name_table | name_table_17(count=1); refs(4:0032); strings("LIST 1") |
| 0x0070 | 32 | 12 | radio_wide | references | refs(2:0029,4:0064,8:0FAB) |
| 0x0071 | 32 | 12 | radio_wide | references | refs(2:0033,8:0FAA) |
| 0x0073 | 32 | 47 | scan_config | references | refs(4:000D,6:000D) |
| 0x0074 | 32 | 32 | scan_config | blob |  |
| 0x0075 | 32 | 12 | scan_config | references | refs(4:0040,8:0FA1) |
| 0x0F9F | 8 | 12 | trunking_config | blob |  |
| 0x0FA0 | 8 | 12 | trunking_config | references | refs(4:0064,6:0032,8:0F9F) |
| 0x0FA1 | 26 | 246 | scan_list | list_page | self_header; entries(entry=15,count=16) |
| 0x0FA6 | 32 | 13 | scan_config | blob | refs(4:0028) |
| 0x0FA8 | 10 | 32 | trunking_personality | blob |  |
| 0x0FA9 | 10 | 24 | trunking_personality | blob |  |
| 0x0FAA | 32 | 57 | message_alias | name | strings("MESSAGE 1") |
| 0x0FAB | 32 | 47 | radio_wide | name | strings("USERNAME 1") |
| 0x0FAC | 32 | 23 | astro_systems | name | strings("CALL 1") |
| 0x0FAD | 32 | 12 | astro_systems | references | refs(4:0032,6:0014,8:0FAC) |
| 0x0FAE | 32 | 11 | astro_systems | blob |  |
| 0x0FAF | 32 | 16 | astro_systems | references | refs(4:0032,6:0032,8:0FAE,12:0FAD) |
| 0x0FB0 | 32 | 23 | astro_systems | name | strings("CALL 1") |
| 0x0FB1 | 32 | 12 | astro_systems | references | refs(4:0032,6:0014,8:0FB0) |
| 0x0FB2 | 32 | 10 | singletone | blob |  |
| 0x0FB3 | 32 | 21 | singletone | references | refs(4:0032,6:0032,8:0FB2,12:0FB1) |
| 0x0FB4 | 28 | 13 | mdc | blob |  |
| 0x0FB5 | 28 | 18 | mdc | blob | refs(12:0064) |
| 0x0FBC | 32 | 21 | mpl | name | strings("MPL 1") |
| 0x0FBD | 32 | 10 | mpl | blob |  |
| 0x0FBE | 28 | 3 | conventional_personality | blob |  |
| 0x0FC1 | 22 | 22 | dtmf | blob |  |
| 0x0FC2 | 22 | 22 | dtmf | blob |  |
| 0x0FC6 | 2 | 56 | trunking_personality | blob |  |
| 0x0FC7 | 32 | 23 | dtmf | name | strings("CALL 1") |
| 0x0FC8 | 32 | 12 | dtmf | references | refs(4:0032,6:0014,8:0FC7) |
| 0x0FC9 | 32 | 8 | dtmf | blob |  |
| 0x0FCA | 32 | 19 | dtmf | references | refs(4:0032,6:0032,8:0FC9,12:0FC8) |
| 0x0FCB | 28 | 15 | quik_call | blob |  |
| 0x0FCC | 29 | 33 | quik_call | references |  |
| 0x0FCD | 28 | 10 | conventional_personality | blob |  |
| 0x0FCE | 32 | 23 | phone | name | strings("PHONE 1") |
| 0x0FCF | 32 | 24 | phone | blob |  |
| 0x0FD0 | 10 | 168 | trunking_personality | blob | refs(2:000A,12:0032,22:0032,32:0032,42:0032,52:0032,62:0032,72:0032) |
| 0x0FD1 | 10 | 17 | trunking_personality | blob |  |
| 0x0FD2 | 9 | 11 | talkgroup | blob |  |
| 0x0FD3 | 9 | 17 | talkgroup | references | refs(4:0030,6:0030,8:0FD2) |
| 0x0FD4 | 9 | 17 | talkgroup | blob |  |
| 0x0FD5 | 9 | 28 | talkgroup | references | refs(2:000B,4:0030,6:0030,8:0FD4,18:0FD3) |
| 0x0FD6 | 9 | 25 | talkgroup | name_table | name_table_17(count=1); strings("MESSAGE 1") |
| 0x0FD7 | 9 | 7 | talkgroup | blob |  |
| 0x0FD8 | 9 | 16 | talkgroup | references | refs(8:0FD7,12:0FD6) |
| 0x0FD9 | 9 | 25 | talkgroup | name_table | name_table_17(count=1); strings("STATUS 1") |
| 0x0FDA | 9 | 7 | talkgroup | blob |  |
| 0x0FDB | 9 | 16 | talkgroup | references | refs(8:0FDA,12:0FD9) |
| 0x0FDC | 9 | 25 | talkgroup | name_table | name_table_17(count=1); refs(4:0032); strings("SITE 1") |
| 0x0FDD | 9 | 15 | talkgroup | blob |  |
| 0x0FDE | 9 | 16 | talkgroup | references | refs(4:0032,6:0032,8:0FDD,12:0FDC) |
| 0x0FDF | 9 | 11 | talkgroup | blob |  |
| 0x0FE0 | 9 | 16 | talkgroup | references | refs(8:0FDF,12:0FC6) |
| 0x0FE1 | 9 | 13 | talkgroup | blob | refs(6:0FFF) |
| 0x0FE2 | 9 | 11 | talkgroup | references |  |
| 0x0FE3 | 9 | 2 | talkgroup | blob |  |
| 0x0FE4 | 9 | 33 | talkgroup | references | refs(12:0022) |
| 0x0FE5 | 9 | 18 | talkgroup | references |  |
| 0x0FE6 | 9 | 43 | talkgroup | blob | refs(8:0028) |
| 0x0FE7 | 9 | 134 | talkgroup | references | refs(32:0FDB,36:0FD8,40:0FDE,114:0FA9,118:0FA8,130:0FA6) |
| 0x0FE8 | 7 | 23 | talkgroup | name | strings("CALL 1") |
| 0x0FE9 | 7 | 12 | talkgroup | references | refs(4:0064,6:0014,8:0FE8) |
| 0x0FEA | 7 | 10 | talkgroup | blob |  |
| 0x0FEB | 7 | 24 | talkgroup | references | refs(4:0064,6:0032,8:0FEA,12:0FE9,20:0FA0) |
| 0x0FEC | 9 | 23 | talkgroup | name | strings("CALL 1") |
| 0x0FED | 9 | 12 | talkgroup | references | refs(4:0064,6:0014,8:0FEC) |
| 0x0FEE | 9 | 9 | talkgroup | blob |  |
| 0x0FEF | 9 | 19 | talkgroup | references | refs(4:0064,6:0064,8:0FEE,12:0FED) |
| 0x0FF0 | 31 | 51 | test_channel_list | blob |  |
| 0x0FF1 | 31 | 30 | test_channel_list | references | refs(8:0FF0,28:000A) |
| 0x0FF2 | 31 | 11 | astro_systems | blob |  |
| 0x0FF3 | 31 | 23 | astro_systems | references | refs(8:0FF2) |
| 0x0FF4 | 31 | 23 | astro_systems | name | strings("CALL 1") |
| 0x0FF5 | 31 | 12 | astro_systems | references | refs(4:0032,6:0014,8:0FF4) |
| 0x0FF6 | 31 | 10 | astro_systems | blob |  |
| 0x0FF7 | 31 | 24 | astro_systems | references | refs(4:0032,6:0032,8:0FF6,12:0FF5) |
| 0x0FF8 | 31 | 25 | astro_systems | name_table | name_table_17(count=1); strings("TG 00002") |
| 0x0FF9 | 27 | 14 | astro_systems | blob |  |
| 0x0FFA | 27 | 60 | conventional_personality | references |  |
| 0x0FFB | 27 | 75 | conventional_personality | references | refs(4:0040,30:0FBE,62:0026); strings("&&&&") |
| 0x0FFC | 31 | 15 | zone_channel_assignment | references | refs(2:0023,6:000A,8:0FFD) |
| 0x0FFD | 27 | 41,216,356 | zone_channel_assignment | list_page | self_header; entries(entry=35,count=10); refs(4:000A,6:0FFB,32:0040,76:0FFB,102:0040,146:0FFB,172:0040,216:0FFB) |
| 0x0FFE | 31 | 12 | zone_channel_assignment | references | refs(6:0014,8:0FFF) |
| 0x0FFF | 32 | 23,125,159,227,244,261,278 | zone_channel_assignment | name_table | strings("CHAN 1","CHAN 2","CHAN 3","CHAN 4") |
| 0x1004 | 32 | 68 | radio_information | identity | refs(12:0048); strings("205CNX2873","H46UCF9PW6BN") |
| 0x1005 | 32 | 43 | radio_information | identity |  |
| 0x1006 | 32 | 24 | radio_information | identity | strings("100001-000100-6") |

## Sections

| section | tags |
|---|---|
| astro_systems | 0x0FAC, 0x0FAD, 0x0FAE, 0x0FAF, 0x0FB0, 0x0FB1, 0x0FF2, 0x0FF3, 0x0FF4, 0x0FF5, 0x0FF6, 0x0FF7, 0x0FF8, 0x0FF9 |
| codeplug_directory | 0x0047 |
| controls | 0x001B |
| conventional_config | 0x0040, 0x0050 |
| conventional_personality | 0x0FBE, 0x0FCD, 0x0FFA, 0x0FFB |
| display_menu | 0x0026, 0x0027 |
| dtmf | 0x0FC1, 0x0FC2, 0x0FC7, 0x0FC8, 0x0FC9, 0x0FCA |
| mdc | 0x0FB4, 0x0FB5 |
| message_alias | 0x005B, 0x0FAA |
| mpl | 0x0064, 0x0065, 0x0FBC, 0x0FBD |
| narrow_band_list | 0x0029 |
| passcode | 0x0051 |
| phone | 0x0049, 0x004A, 0x004B, 0x005A, 0x0FCE, 0x0FCF |
| quik_call | 0x0FCB, 0x0FCC |
| radio_information | 0x1004, 0x1005, 0x1006 |
| radio_wide | 0x000A, 0x000B, 0x000C, 0x0014, 0x0016, 0x0018, 0x0019, 0x0020, 0x0028, 0x002A, 0x002C, 0x002D, 0x0048, 0x0058, 0x0070, 0x0071, 0x0FAB |
| scan_config | 0x0054, 0x005F, 0x0073, 0x0074, 0x0075, 0x0FA6 |
| scan_list | 0x004C, 0x004D, 0x004E, 0x004F, 0x006C, 0x006E, 0x0FA1 |
| secure | 0x0034, 0x0057 |
| singletone | 0x0FB2, 0x0FB3 |
| status_alias | 0x0059 |
| talkgroup | 0x0FD2, 0x0FD3, 0x0FD4, 0x0FD5, 0x0FD6, 0x0FD7, 0x0FD8, 0x0FD9, 0x0FDA, 0x0FDB, 0x0FDC, 0x0FDD, 0x0FDE, 0x0FDF, 0x0FE0, 0x0FE1, 0x0FE2, 0x0FE3, 0x0FE4, 0x0FE5, 0x0FE6, 0x0FE7, 0x0FE8, 0x0FE9, 0x0FEA, 0x0FEB, 0x0FEC, 0x0FED, 0x0FEE, 0x0FEF |
| test_channel_list | 0x0FF0, 0x0FF1 |
| trunking_config | 0x0015, 0x0017, 0x001A, 0x0021, 0x0022, 0x0023, 0x0F9F, 0x0FA0 |
| trunking_personality | 0x0052, 0x0053, 0x0FA8, 0x0FA9, 0x0FC6, 0x0FD0, 0x0FD1 |
| trunking_system | 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F |
| vrs_ep | 0x002F, 0x0030, 0x0031, 0x0032, 0x0033 |
| zone_channel_assignment | 0x000D, 0x0FFC, 0x0FFD, 0x0FFE, 0x0FFF |
