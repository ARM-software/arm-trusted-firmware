/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PMC_H__
#define __PMC_H__

#include <mmio.h>
#include <tegra_def.h>

#define PMC_CONFIG				0x0U
#define PMC_PWRGATE_STATUS			0x38U
#define PMC_PWRGATE_TOGGLE			0x30U
#define  PMC_TOGGLE_START			0x100U
#define PMC_SCRATCH39				0x138U
#define PMC_SECURE_DISABLE2			0x2c4U
#define  PMC_SECURE_DISABLE2_WRITE22_ON		(1U << 28)
#define PMC_SECURE_SCRATCH22			0x338U
#define PMC_SECURE_DISABLE3			0x2d8U
#define  PMC_SECURE_DISABLE3_WRITE34_ON		(1U << 20)
#define  PMC_SECURE_DISABLE3_WRITE35_ON		(1U << 22)
#define PMC_SECURE_SCRATCH34			0x368U
#define PMC_SECURE_SCRATCH35			0x36cU

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
void tegra_pmc_cpu_on(int cpu);
__dead2 void tegra_pmc_system_reset(void);

#endif /* __PMC_H__ */
