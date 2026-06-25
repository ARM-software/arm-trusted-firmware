<!--
SPDX-License-Identifier: BSD-3-Clause
Copyright 2026 Free Mobile - Vincent Jardin
-->

# nbxv3 flash manifest schema

The nbxv3 BL2 bootstrap reads a single JSON file at start-up that
lists from the jtag semihost every region to flash on the on-board NOR.

## Lookup

The bootstrap opens `flash_manifest.json` over ARM semihosting,
the file name is fixed.

## Top-level object

```json
{
  "spi_mhz": 150,
  "regions": [ ... ]
}
```

| Field      | Type             | Required | Meaning                                            |
|------------|------------------|----------|----------------------------------------------------|
| `spi_mhz`  | unsigned integer | no       | FlexSPI SCK ceiling in MHz. Omit (or 0) if default |
| `regions`  | array of region  | yes      | List of regions to flash. At least one entry.      |

Regarding `spi_mhz`, the driver picks the largest divider that keeps SCK at
or below this value.

Unknown top-level fields are ignored (forward compatibility).

## Examples, region object

```json
{
  "file":          "rcw_bootstrap.bin",
  "offset":        "0x000000",
  "size_max":      "0x100000",
  "erase":         true,
  "verify_erase":  false,
  "sha256":        "8c5b...e3a1",
  "label":         "PBL"
}
```

| Field            | Type         | Required | Meaning                                                                  |
|------------------|--------------|----------|--------------------------------------------------------------------------|
| `file`           | string       | yes      | File name relative to the semihost basedir.                              |
| `offset`         | hex string   | yes      | Flash-relative byte offset of the destination, e.g. `"0x100000"`.        |
| `size_max`       | hex string   | yes      | Upper bound on the source file size. The bootstrap fails fast if the file is larger. |
| `erase`          | bool         | no       | Default `false`. Erase the slot before writing.                          |
| `verify_erase`   | bool         | no       | Default `false`. Post-erase 0xFF readback scan.                          |
| `sha256`         | hex string   | no       | Hashes the NOR contents and panics on mismatch. Check data landed in flash.|
| `label`          | string       | no       | Short label printed on the progress-bar header. Default is the file name. |

Field names are case-sensitive. Unknown region fields are
ignored.

## Worked example: full firmware + Linux provisioning

```json
{
    "spi_mhz": 150,
    "regions": [
        {
            "file":         "rcw_bootstrap.bin",
            "offset":       "0x000000",
            "size_max":     "0x100000",
            "erase":        true,
            "label":        "PBL"
        },
        {
            "file":         "fip.bin",
            "offset":       "0x100000",
            "size_max":     "0x400000",
            "erase":        true,
            "label":        "FIP"
        },
        {
            "file":         "poison_env_256.bin",
            "offset":       "0x500000",
            "size_max":     "0x10000",
            "erase":        true,
            "label":        "env-wipe"
        },
        {
            "file":         "ddr_fip.bin",
            "offset":       "0x800000",
            "size_max":     "0x100000",
            "erase":        true,
            "label":        "DDR FIP"
        },
        {
            "file":         "fit.itb",
            "offset":       "0x1000000",
            "size_max":     "0x1000000",
            "erase":        true,
            "label":        "FIT"
        }
    ]
}
```
