/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <denver.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <pmc.h>
#include <tegra_def.h>

#define SB_CSR				0x0
#define  SB_CSR_NS_RST_VEC_WR_DIS	(1 << 1)

/* AARCH64 CPU reset vector */
#define SB_AA64_RESET_LOW		0x30	/* width = 31:0 */
#define SB_AA64_RESET_HI		0x34	/* width = 11:0 */

/* AARCH32 CPU reset vector */
#define EVP_CPU_RESET_VECTOR		0x100

extern void tegra_secure_entrypoint(void);

/*
 * For T132, CPUs reset to AARCH32, so the reset vector is first
 * armv8_trampoline which does a warm reset to AARCH64 and starts
 * execution at the address in SB_AA64_RESET_LOW/SB_AA64_RESET_HI.
 */
__aligned(8) const uint32_t armv8_trampoline[] = {
	0xE3A00003,		/* mov	r0, #3 */
	0xEE0C0F50,		/* mcr	p15, 0, r0, c12, c0, 2 */
	0xEAFFFFFE,		/* b	. */
};

/*******************************************************************************
 * Setup secondary CPU vectors
 ******************************************************************************/
void plat_secondary_setup(void)
{
	uint32_t val;
	uint64_t reset_addr = (uint64_t)tegra_secure_entrypoint;

	/*
	 * For T132, CPUs reset to AARCH32, so the reset vector is first
	 * armv8_trampoline, which does a warm reset to AARCH64 and starts
	 * execution at the address in SCRATCH34/SCRATCH35.
	 */
	INFO("Setting up T132 CPU boot\n");

	/* initial AARCH32 reset address */
	tegra_pmc_write_32(PMC_SECURE_SCRATCH22,
		(unsigned long)&armv8_trampoline);

	/* set AARCH32 exception vector (read to flush) */
	mmio_write_32(TEGRA_EVP_BASE + EVP_CPU_RESET_VECTOR,
		(unsigned long)&armv8_trampoline);
	val = mmio_read_32(TEGRA_EVP_BASE + EVP_CPU_RESET_VECTOR);

	/* setup secondary CPU vector */
	mmio_write_32(TEGRA_SB_BASE + SB_AA64_RESET_LOW,
			(reset_addr & 0xFFFFFFFF) | 1);
	val = reset_addr >> 32;
	mmio_write_32(TEGRA_SB_BASE + SB_AA64_RESET_HI, val & 0x7FF);

	/* configure PMC */
	tegra_pmc_cpu_setup(reset_addr);
	tegra_pmc_lock_cpu_vectors();
}
