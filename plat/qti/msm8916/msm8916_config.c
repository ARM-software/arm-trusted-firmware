/*
 * Copyright (c) 2021-2023, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <drivers/arm/cci.h>
#include <lib/mmio.h>

#include "msm8916_config.h"
#include "msm8916_gicv2.h"
#include <msm8916_mmap.h>
#include <platform_def.h>

static const int cci_map[] = { 3, 4 };

void msm8916_configure_early(void)
{
	if (PLATFORM_CLUSTER_COUNT > 1) {
		cci_init(APCS_CCI_BASE, cci_map, ARRAY_SIZE(cci_map));
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
	}
}

static void msm8916_configure_timer(uintptr_t base)
{
	/* Set timer frequency */
	mmio_write_32(base + CNTCTLBASE_CNTFRQ, PLAT_SYSCNT_FREQ);

	/* Make all timer frames available to non-secure world */
	mmio_write_32(base + CNTNSAR, GENMASK_32(7, 0));
}

/*
 * The APCS register regions always start with a SECURE register that should
 * be cleared to 0 to only allow secure access. Since BL31 handles most of
 * the CPU power management, most of them can be cleared to secure access only.
 */
#define APCS_GLB_SECURE_STS_NS		BIT_32(0)
#define APCS_GLB_SECURE_PWR_NS		BIT_32(1)
#if PLATFORM_CORE_COUNT > 1
#define APCS_BOOT_START_ADDR_SEC	0x04
#define APCS_AA64NAA32_REG		0x0c
#else
#define APCS_BOOT_START_ADDR_SEC	0x18
#endif
#define REMAP_EN			BIT_32(0)

static void msm8916_configure_apcs_cluster(unsigned int cluster)
{
	uintptr_t cfg = APCS_CFG(cluster);
	unsigned int cpu;

	/* Disallow non-secure access to boot remapper / TCM registers */
	mmio_write_32(cfg, 0);

	/*
	 * Disallow non-secure access to power management registers.
	 * However, allow STS and PWR since those also seem to control access
	 * to CPU frequency related registers (e.g. APCS_CMD_RCGR). If these
	 * bits are not set, CPU frequency control fails in the non-secure world.
	 */
	mmio_write_32(APCS_GLB(cluster),
		      APCS_GLB_SECURE_STS_NS | APCS_GLB_SECURE_PWR_NS);

	if (PLATFORM_CORE_COUNT > 1) {
		/* Disallow non-secure access to L2 SAW2 */
		mmio_write_32(APCS_L2_SAW2(cluster), 0);

		/* Disallow non-secure access to CPU ACS and SAW2 */
		for (cpu = 0; cpu < PLATFORM_CPUS_PER_CLUSTER; cpu++) {
			mmio_write_32(APCS_ALIAS_ACS(cluster, cpu), 0);
			mmio_write_32(APCS_ALIAS_SAW2(cluster, cpu), 0);
		}
	} else {
		/* There is just one core so no aliases exist */
		mmio_write_32(APCS_BANKED_ACS, 0);
		mmio_write_32(APCS_BANKED_SAW2, 0);
	}

#ifdef __aarch64__
	/* Make sure all further warm boots end up in BL31 and aarch64 state */
	CASSERT((BL31_BASE & 0xffff) == 0, assert_bl31_base_64k_aligned);
	mmio_write_32(cfg + APCS_BOOT_START_ADDR_SEC, BL31_BASE | REMAP_EN);
	mmio_write_32(cfg + APCS_AA64NAA32_REG, 1);
#else
	/* Make sure all further warm boots end up in BL32 */
	CASSERT((BL32_BASE & 0xffff) == 0, assert_bl32_base_64k_aligned);
	mmio_write_32(cfg + APCS_BOOT_START_ADDR_SEC, BL32_BASE | REMAP_EN);
#endif

	msm8916_configure_timer(APCS_QTMR(cluster));
}

static void msm8916_configure_apcs(void)
{
	unsigned int cluster;

	for (cluster = 0; cluster < PLATFORM_CLUSTER_COUNT; cluster++) {
		msm8916_configure_apcs_cluster(cluster);
	}

	if (PLATFORM_CLUSTER_COUNT > 1) {
		/* Disallow non-secure access to CCI ACS and SAW2 */
		mmio_write_32(APCS_CCI_ACS, 0);
		mmio_write_32(APCS_CCI_SAW2, 0);
	}
}

/*
 * MSM8916 has a special "interrupt aggregation logic" in the APPS SMMU,
 * which allows routing context bank interrupts to one of 3 interrupt numbers
 * ("TZ/HYP/NS"). Route all interrupts to the non-secure interrupt number
 * by default to avoid special setup on the non-secure side.
 */
#define CLK_OFF					BIT_32(31)
#define GCC_APSS_TCU_CBCR			(GCC_BASE + 0x12018)
#define GCC_GFX_TCU_CBCR			(GCC_BASE + 0x12020)
#define GCC_SMMU_CFG_CBCR			(GCC_BASE + 0x12038)
#define GCC_RPM_SMMU_CLOCK_BRANCH_ENA_VOTE	(GCC_BASE + 0x3600c)
#define GCC_APCS_SMMU_CLOCK_BRANCH_ENA_VOTE	(GCC_BASE + 0x4500c)
#define APSS_TCU_CLK_ENA			BIT_32(1)
#define GFX_TCU_CLK_ENA				BIT_32(2)
#define GFX_TBU_CLK_ENA				BIT_32(3)
#define SMMU_CFG_CLK_ENA			BIT_32(12)
#define APPS_SMMU_INTR_SEL_NS			(APPS_SMMU_QCOM + 0x2000)
#define APPS_SMMU_INTR_SEL_NS_EN_ALL		U(0xffffffff)

#define SMMU_SACR				0x010
#define SMMU_SACR_CACHE_LOCK			BIT_32(26)
#define SMMU_IDR7				0x03c
#define SMMU_IDR7_MINOR(val)			(((val) >> 0) & 0xf)
#define SMMU_IDR7_MAJOR(val)			(((val) >> 4) & 0xf)

static void msm8916_smmu_cache_unlock(uintptr_t smmu_base, uintptr_t clk_cbcr)
{
	uint32_t version;

	/* Wait for clock */
	while (mmio_read_32(clk_cbcr) & CLK_OFF) {
	}

	version = mmio_read_32(smmu_base + SMMU_IDR7);
	VERBOSE("SMMU(0x%lx) r%dp%d\n", smmu_base,
		SMMU_IDR7_MAJOR(version), SMMU_IDR7_MINOR(version));

	/* For SMMU r2p0+ clear CACHE_LOCK to allow writes to CBn_ACTLR */
	if (SMMU_IDR7_MAJOR(version) >= 2) {
		mmio_clrbits_32(smmu_base + SMMU_SACR, SMMU_SACR_CACHE_LOCK);
	}
}

static void msm8916_configure_smmu(void)
{
	uint32_t ena_bits = APSS_TCU_CLK_ENA | SMMU_CFG_CLK_ENA;

	/* Single core (MDM) platforms do not have a GPU */
	if (PLATFORM_CORE_COUNT > 1) {
		ena_bits |= GFX_TCU_CLK_ENA | GFX_TBU_CLK_ENA;
	}

	/* Enable SMMU clocks to enable register access */
	mmio_write_32(GCC_APCS_SMMU_CLOCK_BRANCH_ENA_VOTE, ena_bits);

	/* Wait for configuration clock */
	while (mmio_read_32(GCC_SMMU_CFG_CBCR) & CLK_OFF) {
	}

	/* Route all context bank interrupts to non-secure interrupt */
	mmio_write_32(APPS_SMMU_INTR_SEL_NS, APPS_SMMU_INTR_SEL_NS_EN_ALL);

	/* Clear sACR.CACHE_LOCK bit if needed for MMU-500 r2p0+ */
	msm8916_smmu_cache_unlock(APPS_SMMU_BASE, GCC_APSS_TCU_CBCR);
	if (PLATFORM_CORE_COUNT > 1) {
		msm8916_smmu_cache_unlock(GPU_SMMU_BASE, GCC_GFX_TCU_CBCR);
	}

	/*
	 * Keep APCS vote for SMMU clocks for rest of booting process, but make
	 * sure other vote registers (such as RPM) do not keep permanent votes.
	 */
	VERBOSE("Clearing GCC_RPM_SMMU_CLOCK_BRANCH_ENA_VOTE (was: 0x%x)\n",
		mmio_read_32(GCC_RPM_SMMU_CLOCK_BRANCH_ENA_VOTE));
	mmio_write_32(GCC_RPM_SMMU_CLOCK_BRANCH_ENA_VOTE, 0);
}

void msm8916_configure(void)
{
	msm8916_gicv2_configure();
	msm8916_configure_apcs();
	msm8916_configure_smmu();
}
