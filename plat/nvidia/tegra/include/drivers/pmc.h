/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMC_H
#define PMC_H

#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <stdbool.h>

#include <tegra_def.h>

#define PMC_CONFIG				U(0x0)
#define PMC_IO_DPD_SAMPLE			U(0x20)
#define PMC_DPD_ENABLE_0			U(0x24)
#define PMC_PWRGATE_STATUS			U(0x38)
#define PMC_PWRGATE_TOGGLE			U(0x30)
#define PMC_SCRATCH1				U(0x54)
#define PMC_CRYPTO_OP_0				U(0xf4)
#define  PMC_TOGGLE_START			U(0x100)
#define PMC_SCRATCH31				U(0x118)
#define PMC_SCRATCH32				U(0x11C)
#define PMC_SCRATCH33				U(0x120)
#define PMC_SCRATCH39				U(0x138)
#define PMC_SCRATCH40				U(0x13C)
#define PMC_SCRATCH41 				U(0x140)
#define PMC_SCRATCH42				U(0x144)
#define PMC_SCRATCH43				U(0x22C)
#define PMC_SCRATCH44				U(0x230)
#define PMC_SCRATCH45				U(0x234)
#define PMC_SCRATCH46				U(0x238)
#define PMC_SCRATCH47				U(0x23C)
#define PMC_SCRATCH48				U(0x240)
#define PMC_SCRATCH50				U(0x248)
#define PMC_SCRATCH51				U(0x24C)
#define PMC_TSC_MULT_0				U(0x2B4)
#define PMC_STICKY_BIT				U(0x2C0)
#define PMC_SECURE_DISABLE2			U(0x2C4)
#define  PMC_SECURE_DISABLE2_WRITE22_ON		(U(1) << 28)
#define PMC_FUSE_CONTROL_0			U(0x450)
#define PMC_SECURE_DISABLE3			U(0x2D8)
#define  PMC_SECURE_DISABLE3_WRITE34_ON		(U(1) << 20)
#define  PMC_SECURE_DISABLE3_WRITE35_ON		(U(1) << 22)
#define PMC_SECURE_SCRATCH22			U(0x338)
#define PMC_SECURE_SCRATCH34			U(0x368)
#define PMC_SECURE_SCRATCH35			U(0x36c)
#define PMC_SCRATCH56				U(0x600)
#define PMC_SCRATCH57				U(0x604)
#define PMC_SCRATCH201				U(0x844)

static inline uint32_t tegra_pmc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_PMC_BASE + off);
}

static inline void tegra_pmc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_PMC_BASE + off, val);
}

void tegra_pmc_cpu_on(int32_t cpu);
void tegra_pmc_cpu_setup(uint64_t reset_addr);
bool tegra_pmc_is_last_on_cpu(void);
void tegra_pmc_lock_cpu_vectors(void);
void tegra_pmc_resume(void);
__dead2 void tegra_pmc_system_reset(void);

#endif /* PMC_H */
