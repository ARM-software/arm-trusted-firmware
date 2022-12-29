/*
 * Copyright (c) 2022-2023, Arm Limited. All rights reserved.
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

/* Boot manifest core structure as per v0.2 */
struct rmm_manifest {
	uint32_t version;		/* Manifest version */
	uint32_t padding;		/* RES0 */
	uintptr_t plat_data;		/* Manifest platform data */
	struct ns_dram_info plat_dram;	/* Platform NS DRAM data */
};

CASSERT(offsetof(struct rmm_manifest, version) == 0UL,
			rmm_manifest_version_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_data) == 8UL,
			rmm_manifest_plat_data_unaligned);
CASSERT(offsetof(struct rmm_manifest, plat_dram) == 16UL,
			rmm_manifest_plat_dram_unaligned);

#endif /* RMM_CORE_MANIFEST_H */
