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
#define RMMD_MANIFEST_VERSION_MINOR		U(1)

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

/* Boot manifest core structure as per v0.1 */
typedef struct rmm_manifest {
	uint32_t version;	/* Manifest version */
	uint32_t padding;	/* RES0 */
	uintptr_t plat_data;	/* Manifest platform data */
} rmm_manifest_t;

CASSERT(offsetof(rmm_manifest_t, version) == 0,
				rmm_manifest_t_version_unaligned);
CASSERT(offsetof(rmm_manifest_t, plat_data) == 8,
				rmm_manifest_t_plat_data_unaligned);

#endif /* RMM_CORE_MANIFEST_H */
