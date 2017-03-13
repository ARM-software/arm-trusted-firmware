/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PMC_H__
#define __PMC_H__

#include <mmio.h>
#include <tegra_def.h>
#include <utils_def.h>

#define PMC_CONFIG				U(0x0)
#define PMC_PWRGATE_STATUS			U(0x38)
#define PMC_PWRGATE_TOGGLE			U(0x30)
#define  PMC_TOGGLE_START			U(0x100)
#define PMC_SCRATCH39				U(0x138)
#define PMC_SECURE_DISABLE2			U(0x2c4)
#define  PMC_SECURE_DISABLE2_WRITE22_ON		(U(1) << 28)
#define PMC_SECURE_SCRATCH22			U(0x338)
#define PMC_SECURE_DISABLE3			U(0x2d8)
#define  PMC_SECURE_DISABLE3_WRITE34_ON		(U(1) << 20)
#define  PMC_SECURE_DISABLE3_WRITE35_ON		(U(1) << 22)
#define PMC_SECURE_SCRATCH34			U(0x368)
#define PMC_SECURE_SCRATCH35			U(0x36c)

static inline uint32_t tegra_pmc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_PMC_BASE + off);
}

static inline void tegra_pmc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_PMC_BASE + off, val);
}

void tegra_pmc_cpu_setup(uint64_t reset_addr);
void tegra_pmc_lock_cpu_vectors(void);
void tegra_pmc_cpu_on(int32_t cpu);
__dead2 void tegra_pmc_system_reset(void);

#endif /* __PMC_H__ */
