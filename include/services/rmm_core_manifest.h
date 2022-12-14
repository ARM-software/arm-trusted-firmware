/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
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
#define RMMD_MANIFEST_VERSION_MINOR		U(2)

/*
 * Manifest version encoding:
 *	- Bit[31] RES0
 *	- Bits [30:16] Major version
 *	- Bits [15:0] Minor version
 */
#define _RMMD_MANIFEST_VERSION(_major, _minor)				\
	((((_major) & 0x7FFF) << 16) | ((_minor) & 0xFFFF))

#define RMMD_MANIFEST_VERSION _RMMD_MANIFEST_VERSION(			\
				RMMD_MANIFEST_VERSION_MAJOR,		\
				RMMD_MANIFEST_VERSION_MINOR)

#define RMMD_GET_MANIFEST_VERSION_MAJOR(_version)			\
	((_version >> 16) & 0x7FFF)

#define RMMD_GET_MANIFEST_VERSION_MINOR(_version)			\
	(_version & 0xFFFF)

/* DRAM bank structure */
struct dram_bank {
	uintptr_t base;			/* Base address */
	uint64_t size;			/* Size of bank */
};

CASSERT(offsetof(struct dram_bank, base) == 0,
			rmm_manifest_base_unaligned);
CASSERT(offsetof(struct dram_bank, size) == 8,
			rmm_manifest_size_unaligned);

/* DRAM layout info structure */
struct dram_info {
	uint64_t banks_num;		/* Number of DRAM banks */
	struct dram_bank *dram_data;	/* Pointer to dram_bank[] */
	uint64_t check_sum;		/* Checksum of dram_info data */
};

CASSERT(offsetof(struct dram_info, banks_num) == 0,
			rmm_manifest_banks_num_unaligned);
CASSERT(offsetof(struct dram_info, dram_data) == 8,
			rmm_manifest_dram_data_unaligned);
CASSERT(offsetof(struct dram_info, check_sum) == 16,
			rmm_manifest_check_sum_unaligned);

/* Boot manifest core structure as per v0.2 */
struct rmm_manifest {
	uint32_t version;		/* Manifest version */
	uint32_t padding;		/* RES0 */
	uintptr_t plat_data;		/* Manifest platform data */
	struct dram_info plat_dram;	/* Platform DRAM data */
};

CASSERT(offsetof(struct rmm_manifest, version) == 0,
			rmm_manifest_version_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_data) == 8,
			rmm_manifest_plat_data_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_dram) == 16,
			rmm_manifest_plat_dram_unaligned);

#endif /* RMM_CORE_MANIFEST_H */
