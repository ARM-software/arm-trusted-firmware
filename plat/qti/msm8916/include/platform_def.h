/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <plat/common/common_def.h>

/*
 * There is at least 1 MiB available for BL31. However, at the moment the
 * "msm8916_entry_point" variable in the data section is read through the
 * 64 KiB region of the "boot remapper" after reset. For simplicity, limit
 * the end of the data section (BL31_PROGBITS_LIMIT) to 64 KiB for now and
 * the overall limit to 128 KiB. This could be increased if needed by placing
 * the "msm8916_entry_point" variable explicitly in the first 64 KiB of BL31.
 */
#define BL31_LIMIT			(BL31_BASE + 0x20000)	/* 128 KiB */
#define BL31_PROGBITS_LIMIT		(BL31_BASE + 0x10000)	/* 64 KiB */

#define CACHE_WRITEBACK_GRANULE		U(64)
#define PLATFORM_STACK_SIZE		U(0x1000)

/* CPU topology: single cluster with 4 cores */
#define PLATFORM_CLUSTER_COUNT		U(1)
#define PLATFORM_MAX_CPUS_PER_CLUSTER	U(4)
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER_COUNT * \
					 PLATFORM_MAX_CPUS_PER_CLUSTER)

/* Power management */
#define PLATFORM_SYSTEM_COUNT		U(1)
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_SYSTEM_COUNT + \
					 PLATFORM_CLUSTER_COUNT + \
					 PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2
#define PLAT_MAX_RET_STATE		U(2)
#define PLAT_MAX_OFF_STATE		U(3)

/* Translation tables */
#define MAX_MMAP_REGIONS		8
#define MAX_XLAT_TABLES			4

#define PLAT_PHY_ADDR_SPACE_SIZE	(ULL(1) << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(ULL(1) << 32)

/* Timer frequency */
#define PLAT_SYSCNT_FREQ		19200000

/*
 * The Qualcomm QGIC2 implementation seems to have PIDR0-4 and PIDR4-7
 * erroneously swapped for some reason. PIDR2 is actually at 0xFD8.
 * Override the address in <drivers/arm/gicv2.h> to avoid a failing assert().
 */
#define GICD_PIDR2_GICV2		U(0xFD8)

#endif /* PLATFORM_DEF_H */
