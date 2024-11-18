/*
 * Copyright (c) 2022-2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMM_CORE_MANIFEST_H
#define RMM_CORE_MANIFEST_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <lib/cassert.h>

#define RMMD_MANIFEST_VERSION_MAJOR		U(0)
#define RMMD_MANIFEST_VERSION_MINOR		U(3)

#define RMM_CONSOLE_MAX_NAME_LEN		U(8)

/*
 * Manifest version encoding:
 *	- Bit[31] RES0
 *	- Bits [30:16] Major version
 *	- Bits [15:0] Minor version
 */
#define SET_RMMD_MANIFEST_VERSION(_major, _minor)		\
	((((_major) & 0x7FFF) << 16) | ((_minor) & 0xFFFF))

#define RMMD_MANIFEST_VERSION	SET_RMMD_MANIFEST_VERSION(	\
				RMMD_MANIFEST_VERSION_MAJOR,	\
				RMMD_MANIFEST_VERSION_MINOR)

#define RMMD_GET_MANIFEST_VERSION_MAJOR(_version)		\
	((_version >> 16) & 0x7FFF)

#define RMMD_GET_MANIFEST_VERSION_MINOR(_version)		\
	(_version & 0xFFFF)

/* NS DRAM bank structure */
struct ns_dram_bank {
	uintptr_t base;			/* Base address */
	uint64_t size;			/* Size of bank */
};

CASSERT(offsetof(struct ns_dram_bank, base) == 0UL,
			rmm_manifest_base_unaligned);
CASSERT(offsetof(struct ns_dram_bank, size) == 8UL,
			rmm_manifest_size_unaligned);

/* NS DRAM layout info structure */
struct ns_dram_info {
	uint64_t num_banks;		/* Number of NS DRAM banks */
	struct ns_dram_bank *banks;	/* Pointer to ns_dram_bank[] */
	uint64_t checksum;		/* Checksum of ns_dram_info data */
};

CASSERT(offsetof(struct ns_dram_info, num_banks) == 0UL,
			rmm_manifest_num_banks_unaligned);
CASSERT(offsetof(struct ns_dram_info, banks) == 8UL,
			rmm_manifest_dram_data_unaligned);
CASSERT(offsetof(struct ns_dram_info, checksum) == 16UL,
			rmm_manifest_checksum_unaligned);

/* Console info structure */
struct console_info {
	uintptr_t base;			/* Console base address */
	uint64_t map_pages;		/* Num of pages to be mapped in RMM for the console MMIO */
	char name[RMM_CONSOLE_MAX_NAME_LEN];	/* Name of console */
	uint64_t clk_in_hz;		/* UART clock (in Hz) for the console */
	uint64_t baud_rate;		/* Baud rate */
	uint64_t flags;			/* Additional flags RES0 */
};

CASSERT(offsetof(struct console_info, base) == 0UL,
			rmm_manifest_console_base_unaligned);
CASSERT(offsetof(struct console_info, map_pages) == 8UL,
			rmm_manifest_console_map_pages_unaligned);
CASSERT(offsetof(struct console_info, name) == 16UL,
			rmm_manifest_console_name_unaligned);
CASSERT(offsetof(struct console_info, clk_in_hz) == 24UL,
			rmm_manifest_console_clk_in_hz_unaligned);
CASSERT(offsetof(struct console_info, baud_rate) == 32UL,
			rmm_manifest_console_baud_rate_unaligned);
CASSERT(offsetof(struct console_info, flags) == 40UL,
			rmm_manifest_console_flags_unaligned);

struct console_list {
	uint64_t num_consoles;		/* Number of consoles */
	struct console_info *consoles;	/* Pointer to console_info[] */
	uint64_t checksum;		/* Checksum of console_list data */
};

CASSERT(offsetof(struct console_list, num_consoles) == 0UL,
			rmm_manifest_num_consoles);
CASSERT(offsetof(struct console_list, consoles) == 8UL,
			rmm_manifest_consoles);
CASSERT(offsetof(struct console_list, checksum) == 16UL,
			rmm_manifest_console_list_checksum);

/* Boot manifest core structure as per v0.3 */
struct rmm_manifest {
	uint32_t version;			/* Manifest version */
	uint32_t padding;			/* RES0 */
	uintptr_t plat_data;			/* Manifest platform data */
	struct ns_dram_info plat_dram;		/* Platform NS DRAM data (v0.2) */
	struct console_list plat_console;	/* Platform console list (v0.3) */
};

CASSERT(offsetof(struct rmm_manifest, version) == 0UL,
			rmm_manifest_version_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_data) == 8UL,
			rmm_manifest_plat_data_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_dram) == 16UL,
			rmm_manifest_plat_dram_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_console) == 40UL,
			rmm_manifest_plat_console_unaligned);

#endif /* RMM_CORE_MANIFEST_H */
