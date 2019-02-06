/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
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
#define PMC_SECURE_SCRATCH0			U(0xb0)
#define PMC_SECURE_SCRATCH5			U(0xc4)
#define PMC_CRYPTO_OP_0				U(0xf4)
#define  PMC_TOGGLE_START			U(0x100)
#define PMC_SCRATCH39				U(0x138)
#define PMC_SCRATCH41 				U(0x140)
#define PMC_SECURE_SCRATCH6			U(0x224)
#define PMC_SECURE_SCRATCH7			U(0x228)
#define PMC_SECURE_DISABLE2			U(0x2c4)
#define  PMC_SECURE_DISABLE2_WRITE22_ON		(U(1) << 28)
#define PMC_SECURE_SCRATCH8			U(0x300)
#define PMC_SECURE_SCRATCH79			U(0x41c)
#define PMC_FUSE_CONTROL_0			U(0x450)
#define PMC_SECURE_SCRATCH22			U(0x338)
#define PMC_SECURE_DISABLE3			U(0x2d8)
#define  PMC_SECURE_DISABLE3_WRITE34_ON		(U(1) << 20)
#define  PMC_SECURE_DISABLE3_WRITE35_ON		(U(1) << 22)
#define PMC_SECURE_SCRATCH34			U(0x368)
#define PMC_SECURE_SCRATCH35			U(0x36c)
#define PMC_SECURE_SCRATCH80			U(0xa98)
#define PMC_SECURE_SCRATCH119			U(0xb34)
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
