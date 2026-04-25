/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Diagnostic wrappers around the FlexSPI NOR driver primitives.
 * See xspi_nor_diag.h for the API contract.
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <common/debug.h>
#include <fspi_api.h>
#include <xspi_error_codes.h>
#include <xspi_nor_diag.h>

/*
 * SR1 layout (common to most SPI NOR chips, e.g. GD55 Table 4):
 *   bit 7 SRP0  Status Register Protection
 *   bit 6 BP4   Block Protect
 *   bit 5 BP3   Block Protect
 *   bit 4 BP2   Block Protect
 *   bit 3 BP1   Block Protect
 *   bit 2 BP0   Block Protect
 *   bit 1 WEL   Write Enable Latch (volatile, RO)
 *   bit 0 WIP   Write/Erase In Progress (volatile, RO)
 *
 * BP[4:0] != 0 silently blocks SE/PP.
 */
void xspi_print_sr1(const char *label, uint8_t sr1)
{
	uint8_t bp = (sr1 >> 2) & 0x1FU;

	NOTICE("xspi: SR1 %s = 0x%02x [SRP0=%u BP4=%u BP3=%u BP2=%u BP1=%u BP0=%u WEL=%u WIP=%u]\n",
	       label, sr1,
	       (sr1 >> 7) & 1U, (sr1 >> 6) & 1U, (sr1 >> 5) & 1U,
	       (sr1 >> 4) & 1U, (sr1 >> 3) & 1U, (sr1 >> 2) & 1U,
	       (sr1 >> 1) & 1U, (sr1 >> 0) & 1U);
	if (bp != 0U) {
		NOTICE("xspi:   WARNING: BP bits nonzero (0x%02x); "
		       "sectors may be write/erase protected\n", bp);
	}
}

/* JEDEC manufacturer / part name tables. */

const char *xspi_jedec_mfg_name(uint8_t mfg)
{
	switch (mfg) {
	case 0x01U:
		return "Spansion / Cypress / Infineon";
	case 0x1CU:
		return "EON";
	case 0x1FU:
		return "Atmel / Microchip";
	case 0x20U:
		return "Micron / Numonyx / STMicro";
	case 0x34U:
		return "Cypress (ex-Spansion S28)";
	case 0x62U:
		return "SANYO";
	case 0x89U:
		return "Intel";
	case 0x9DU:
		return "ISSI";
	case 0xBFU:
		return "SST";
	case 0xC2U:
		return "Macronix";
	case 0xC8U:
		return "GigaDevice";
	case 0xEFU:
		return "Winbond";
	case 0x00U:
	case 0xFFU:
		return "<flash not responding>";
	default:
		return "unknown vendor";
	}
}

const char *xspi_jedec_part_name(uint8_t mfg, uint8_t type, uint8_t cap)
{
	/* GigaDevice GD55 family: memtype 0x60 = 1.8 V xSPI NOR. */
	if ((mfg == 0xC8U) && (type == 0x60U)) {
		switch (cap) {
		case 0x18U:
			return "GD55LB512ME (512 Mbit / 64 MiB)";
		case 0x19U:
			return "GD55LB256E  (256 Mbit / 32 MiB)";
		case 0x1BU:
			return "GD55LB01GF  (1 Gbit / 128 MiB)";
		case 0x1CU:
			return "GD55LB02GF  (2 Gbit / 256 MiB)";
		default:
			break;
		}
	}

	/* Micron MT25Q / MT35X NOR. */
	if (mfg == 0x20U) {
		if (type == 0x5BU) {
			switch (cap) {
			case 0x1AU:
				return "MT35XU512A  (512 Mbit / 64 MiB)";
			case 0x1CU:
				return "MT35XU02G   (2 Gbit / 256 MiB)";
			default:
				break;
			}
		}
		if ((type == 0xBAU) || (type == 0xBBU)) {
			switch (cap) {
			case 0x19U:
				return "MT25Q256A   (256 Mbit / 32 MiB)";
			case 0x20U:
				return "MT25QU512A  (512 Mbit / 64 MiB)";
			case 0x21U:
				return "MT25QU01GBBB (1 Gbit / 128 MiB)";
			default:
				break;
			}
		}
	}

	/* Macronix MX25U / MX66U. */
	if (mfg == 0xC2U) {
		if (type == 0x25U) {
			switch (cap) {
			case 0x39U:
				return "MX25U25645G (256 Mbit / 32 MiB)";
			case 0x3AU:
				return "MX25U51245G (512 Mbit / 64 MiB)";
			default:
				break;
			}
		}
		if (type == 0x75U) {
			switch (cap) {
			case 0x3BU:
				return "MX66U1G45G  (1 Gbit / 128 MiB)";
			default:
				break;
			}
		}
	}

	/* Winbond W25Q family. */
	if ((mfg == 0xEFU) && ((type == 0x60U) || (type == 0x40U))) {
		switch (cap) {
		case 0x19U:
			return "W25Q256        (256 Mbit / 32 MiB)";
		case 0x21U:
			return "W25Q01NW       (1 Gbit / 128 MiB)";
		default:
			break;
		}
	}

	return "unknown part";
}

/* SR1 before/after WREN, WEL-latch check. */
int xspi_probe_status(void)
{
	uint8_t sr_before = 0U;
	uint8_t sr_after = 0U;
	int ret;

	ret = fspi_read_sr1(&sr_before);
	if (ret != XSPI_SUCCESS) {
		return ret;
	}
	xspi_print_sr1("before WREN", sr_before);

	ret = xspi_wren(0U);
	if (ret != XSPI_SUCCESS) {
		ERROR("xspi: WREN failed (%d)\n", ret);
		return ret;
	}

	ret = fspi_read_sr1(&sr_after);
	if (ret != XSPI_SUCCESS) {
		return ret;
	}
	xspi_print_sr1("after  WREN", sr_after);

	if (((sr_after >> 1) & 1U) == 0U) {
		NOTICE("xspi:   WARNING: WEL did not latch after WREN; "
		       "erase/write will be silently ignored by the chip\n");
	}
	return 0;
}

/* JEDEC RDID 0x9F + name decode. */
int xspi_probe_rdid(void)
{
	uint8_t id[5] = { 0U };
	int ret;

	ret = xspi_read_id(id, sizeof(id));
	if (ret != XSPI_SUCCESS) {
		ERROR("xspi: RDID failed (%d)\n", ret);
		return ret;
	}

	NOTICE("xspi: flash JEDEC ID = %02x %02x %02x %02x %02x\n",
	       id[0], id[1], id[2], id[3], id[4]);
	NOTICE("xspi:   mfg=0x%02x, memtype=0x%02x, capacity=0x%02x\n",
	       id[0], id[1], id[2]);
	NOTICE("xspi:   vendor = %s\n", xspi_jedec_mfg_name(id[0]));
	NOTICE("xspi:   part   = %s\n",
	       xspi_jedec_part_name(id[0], id[1], id[2]));

	if ((id[0] == 0x00U) || (id[0] == 0xFFU)) {
		ERROR("xspi: RDID returned 0x%02x; flash not responding\n",
		      id[0]);
		return -EIO;
	}
	return 0;
}

/*
 * SFDP header + BFPT density + erase types. JEDEC JESD216 encoding;
 * we decode only the fields that cross-check driver assumptions
 * (density, address-byte cap, the four erase types).
 */

static uint32_t xspi_rd_le32(const uint8_t *p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
	       ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/*
 * Density in bytes from BFPT DWORD 2:
 *   bit31=0: dw2 = density_in_bits - 1           (<= 2 Gbit)
 *   bit31=1: dw2[30:0] = log2(density_in_bits)   (> 2 Gbit)
 */
static uint64_t xspi_sfdp_density_bytes(uint32_t dw2)
{
	uint64_t bits;

	if ((dw2 & 0x80000000U) != 0U) {
		uint32_t n = dw2 & 0x7FFFFFFFU;

		if (n >= 64U) {
			return 0ULL;
		}
		bits = 1ULL << n;
	} else {
		bits = (uint64_t)dw2 + 1ULL;
	}
	return bits / 8ULL;
}

static void xspi_print_erase_type(uint32_t idx, uint8_t size_log2,
				  uint8_t opcode)
{
	if (size_log2 == 0U) {
		NOTICE("xspi:   SFDP erase type %u: (unsupported)\n", idx);
		return;
	}
	NOTICE("xspi:   SFDP erase type %u: 2^%u = %u bytes, opcode 0x%02x\n",
	       idx, size_log2, 1U << size_log2, opcode);
}

void xspi_probe_sfdp(void)
{
	uint8_t hdr[16];
	uint8_t bfpt[36];
	uint32_t bfpt_ptr;
	uint32_t bfpt_dw;
	uint32_t read_len;
	uint64_t density_bytes;
	int ret;

	/* 8-byte SFDP header + first 8-byte param header (BFPT). */
	ret = xspi_read_sfdp(0U, hdr, sizeof(hdr));
	if (ret != XSPI_SUCCESS) {
		WARN("xspi: SFDP header read failed (%d)\n", ret);
		return;
	}

	if ((hdr[0] != 'S') || (hdr[1] != 'F') ||
	    (hdr[2] != 'D') || (hdr[3] != 'P')) {
		WARN("xspi: SFDP signature mismatch (%02x %02x %02x %02x); "
		     "skipping SFDP probe\n",
		     hdr[0], hdr[1], hdr[2], hdr[3]);
		return;
	}

	NOTICE("xspi: SFDP signature OK, rev %u.%u, %u header(s)\n",
	       hdr[5], hdr[4], (uint32_t)hdr[6] + 1U);

	/* First parameter header must be BFPT (ID MSB=0xFF, LSB=0x00). */
	if ((hdr[8] != 0x00U) || (hdr[15] != 0xFFU)) {
		WARN("xspi: first SFDP param is not BFPT (ID %02x%02x); "
		     "skipping density/erase decode\n", hdr[15], hdr[8]);
		return;
	}

	bfpt_dw  = hdr[11];
	bfpt_ptr = (uint32_t)hdr[12] |
		   ((uint32_t)hdr[13] << 8) |
		   ((uint32_t)hdr[14] << 16);

	/* DW1..DW9 (36 bytes) cover density + all 4 erase types. */
	read_len = bfpt_dw * 4U;
	if (read_len > sizeof(bfpt)) {
		read_len = sizeof(bfpt);
	}

	ret = xspi_read_sfdp(bfpt_ptr, bfpt, read_len);
	if (ret != XSPI_SUCCESS) {
		WARN("xspi: BFPT read @0x%x failed (%d)\n", bfpt_ptr, ret);
		return;
	}

	density_bytes = xspi_sfdp_density_bytes(xspi_rd_le32(&bfpt[4]));
	if (density_bytes == 0ULL) {
		WARN("xspi: SFDP BFPT DW2 density encoding invalid\n");
	} else {
		NOTICE("xspi: SFDP density = %llu bytes (%llu MB)\n",
		       (unsigned long long)density_bytes,
		       (unsigned long long)(density_bytes >> 20));
	}

	/* BFPT DW1[17:16] = address-byte capability. */
	{
		uint32_t dw1 = xspi_rd_le32(&bfpt[0]);
		uint32_t ab  = (dw1 >> 17) & 0x3U;

		NOTICE("xspi: SFDP address-byte capability = %s\n",
		       (ab == 0U) ? "3B-only" :
		       (ab == 1U) ? "3B default, 4B switchable" :
		       (ab == 2U) ? "4B-only" : "reserved");
	}

	if (read_len >= 36U) {
		uint32_t dw8 = xspi_rd_le32(&bfpt[28]);
		uint32_t dw9 = xspi_rd_le32(&bfpt[32]);

		xspi_print_erase_type(1U, (uint8_t)(dw8 & 0xFFU),
				      (uint8_t)((dw8 >> 8) & 0xFFU));
		xspi_print_erase_type(2U, (uint8_t)((dw8 >> 16) & 0xFFU),
				      (uint8_t)((dw8 >> 24) & 0xFFU));
		xspi_print_erase_type(3U, (uint8_t)(dw9 & 0xFFU),
				      (uint8_t)((dw9 >> 8) & 0xFFU));
		xspi_print_erase_type(4U, (uint8_t)((dw9 >> 16) & 0xFFU),
				      (uint8_t)((dw9 >> 24) & 0xFFU));
	}
}
