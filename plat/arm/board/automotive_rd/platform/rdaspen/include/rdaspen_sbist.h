/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RDASPEN_SBIST_H
#define RDASPEN_SBIST_H

#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/common/common_def.h>

/* RD-Aspen AP Cluster Utility Region Base */
#define RDASPEN_CLUSTER_BASE	UL(0x140000000)
#define SBIST_OFFSET		UL(0x100000)

/* SBIST region of each cluster */
#define SBIST_BASE(cl)		(RDASPEN_CLUSTER_BASE + ((cl) * SZ_64M) + SBIST_OFFSET)
#define BASE_SBIST_SIZE		UL(0x0100000)

#define MAP_SBIST_MEM(cl)	MAP_REGION_FLAT(SBIST_BASE(cl),  \
						BASE_SBIST_SIZE, \
						MT_DEVICE | MT_RW | MT_SECURE)

#endif /* RDASPEN_SBIST_H */
