/*
 * Copyright (c) 2021-2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <lib/mmio.h>

#include "msm8916_config.h"
#include "msm8916_gicv2.h"
#include <msm8916_mmap.h>
#include <platform_def.h>

static void msm8916_configure_timer(void)
{
	/* Set timer frequency */
	mmio_write_32(APCS_QTMR + CNTCTLBASE_CNTFRQ, PLAT_SYSCNT_FREQ);

	/* Make all timer frames available to non-secure world */
	mmio_write_32(APCS_QTMR + CNTNSAR, GENMASK_32(7, 0));
}

/*
 * The APCS register regions always start with a SECURE register that should
 * be cleared to 0 to only allow secure access. Since BL31 handles most of
 * the CPU power management, most of them can be cleared to secure access only.
 */
#define APCS_GLB_SECURE_STS_NS		BIT_32(0)
#define APCS_GLB_SECURE_PWR_NS		BIT_32(1)
#define APCS_BOOT_START_ADDR_SEC	(APCS_CFG + 0x04)
#define REMAP_EN			BIT_32(0)
#define APCS_AA64NAA32_REG		(APCS_CFG + 0x0c)

static void msm8916_configure_cpu_pm(void)
{
	unsigned int cpu;

	/* Disallow non-secure access to boot remapper / TCM registers */
	mmio_write_32(APCS_CFG, 0);

	/*
	 * Disallow non-secure access to power management registers.
	 * However, allow STS and PWR since those also seem to control access
	 * to CPU frequency related registers (e.g. APCS_CMD_RCGR). If these
	 * bits are not set, CPU frequency control fails in the non-secure world.
	 */
	mmio_write_32(APCS_GLB, APCS_GLB_SECURE_STS_NS | APCS_GLB_SECURE_PWR_NS);

	/* Disallow non-secure access to L2 SAW2 */
	mmio_write_32(APCS_L2_SAW2, 0);

	/* Disallow non-secure access to CPU ACS and SAW2 */
	for (cpu = 0; cpu < PLATFORM_CORE_COUNT; cpu++) {
		mmio_write_32(APCS_ALIAS_ACS(cpu), 0);
		mmio_write_32(APCS_ALIAS_SAW2(cpu), 0);
	}

#ifdef __aarch64__
	/* Make sure all further warm boots end up in BL31 and aarch64 state */
	CASSERT((BL31_BASE & 0xffff) == 0, assert_bl31_base_64k_aligned);
	mmio_write_32(APCS_BOOT_START_ADDR_SEC, BL31_BASE | REMAP_EN);
	mmio_write_32(APCS_AA64NAA32_REG, 1);
#else
	/* Make sure all further warm boots end up in BL32 */
	CASSERT((BL32_BASE & 0xffff) == 0, assert_bl32_base_64k_aligned);
	mmio_write_32(APCS_BOOT_START_ADDR_SEC, BL32_BASE | REMAP_EN);
#endif
}

/*
 * MSM8916 has a special "interrupt aggregation logic" in the APPS SMMU,
 * which allows routing context bank interrupts to one of 3 interrupt numbers
 * ("TZ/HYP/NS"). Route all interrupts to the non-secure interrupt number
 * by default to avoid special setup on the non-secure side.
 */
#define CLK_OFF					BIT_32(31)
#define GCC_SMMU_CFG_CBCR			(GCC_BASE + 0x12038)
#define GCC_APCS_SMMU_CLOCK_BRANCH_ENA_VOTE	(GCC_BASE + 0x4500c)
#define SMMU_CFG_CLK_ENA			BIT_32(12)
#define APPS_SMMU_INTR_SEL_NS			(APPS_SMMU_QCOM + 0x2000)
#define APPS_SMMU_INTR_SEL_NS_EN_ALL		U(0xffffffff)

static void msm8916_configure_smmu(void)
{
	/* Enable SMMU configuration clock to enable register access */
	mmio_setbits_32(GCC_APCS_SMMU_CLOCK_BRANCH_ENA_VOTE, SMMU_CFG_CLK_ENA);
	while (mmio_read_32(GCC_SMMU_CFG_CBCR) & CLK_OFF) {
	}

	/* Route all context bank interrupts to non-secure interrupt */
	mmio_write_32(APPS_SMMU_INTR_SEL_NS, APPS_SMMU_INTR_SEL_NS_EN_ALL);

	/* Disable configuration clock again */
	mmio_clrbits_32(GCC_APCS_SMMU_CLOCK_BRANCH_ENA_VOTE, SMMU_CFG_CLK_ENA);
}

void msm8916_configure(void)
{
	msm8916_gicv2_configure();
	msm8916_configure_timer();
	msm8916_configure_cpu_pm();
	msm8916_configure_smmu();
}
