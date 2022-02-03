/*
 * Copyright (c) 2021, Stephan Gerhold <stephan@gerhold.net>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <drivers/generic_delay_timer.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include "msm8916_gicv2.h"
#include <msm8916_mmap.h>
#include <platform_def.h>
#include <uartdm_console.h>

static const mmap_region_t msm8916_mmap[] = {
	MAP_REGION_FLAT(PCNOC_BASE, PCNOC_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE | MT_EXECUTE_NEVER),
	MAP_REGION_FLAT(APCS_BASE, APCS_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE | MT_EXECUTE_NEVER),
	{},
};

static struct {
	entry_point_info_t bl32;
	entry_point_info_t bl33;
} image_ep_info = {
	/* BL32 entry point */
	SET_STATIC_PARAM_HEAD(bl32, PARAM_EP, VERSION_1,
			      entry_point_info_t, SECURE),
	.bl32.pc = BL32_BASE,

	/* BL33 entry point */
	SET_STATIC_PARAM_HEAD(bl33, PARAM_EP, VERSION_1,
			      entry_point_info_t, NON_SECURE),
	.bl33.pc = PRELOADED_BL33_BASE,
	.bl33.spsr = SPSR_64(MODE_EL2, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS),
};

static console_t console;

unsigned int plat_get_syscnt_freq2(void)
{
	return PLAT_SYSCNT_FREQ;
}

#define CLK_ENABLE			BIT_32(0)
#define CLK_OFF				BIT_32(31)

#define GPIO_BLSP_UART2_TX		4
#define GPIO_BLSP_UART2_RX		5
#define GPIO_CFG_FUNC_BLSP_UART2	(U(0x2) << 2)
#define GPIO_CFG_DRV_STRENGTH_16MA	(U(0x7) << 6)

#define GCC_BLSP1_AHB_CBCR		(GCC_BASE + 0x01008)
#define GCC_BLSP1_UART2_APPS_CBCR	(GCC_BASE + 0x0302c)
#define GCC_APCS_CLOCK_BRANCH_ENA_VOTE	(GCC_BASE + 0x45004)
#define BLSP1_AHB_CLK_ENA		BIT_32(10)

/*
 * The previous boot stage seems to disable most of the UART setup before exit
 * so it must be enabled here again before the UART console can be used.
 */
static void msm8916_enable_blsp_uart2(void)
{
	/* Route GPIOs to BLSP UART2 */
	mmio_write_32(TLMM_GPIO_CFG(GPIO_BLSP_UART2_TX),
		      GPIO_CFG_FUNC_BLSP_UART2 | GPIO_CFG_DRV_STRENGTH_16MA);
	mmio_write_32(TLMM_GPIO_CFG(GPIO_BLSP_UART2_RX),
		      GPIO_CFG_FUNC_BLSP_UART2 | GPIO_CFG_DRV_STRENGTH_16MA);

	/* Enable AHB clock */
	mmio_setbits_32(GCC_APCS_CLOCK_BRANCH_ENA_VOTE, BLSP1_AHB_CLK_ENA);
	while (mmio_read_32(GCC_BLSP1_AHB_CBCR) & CLK_OFF)
		;

	/* Enable BLSP UART2 clock */
	mmio_setbits_32(GCC_BLSP1_UART2_APPS_CBCR, CLK_ENABLE);
	while (mmio_read_32(GCC_BLSP1_UART2_APPS_CBCR) & CLK_OFF)
		;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	/* Initialize the debug console as early as possible */
	msm8916_enable_blsp_uart2();
	console_uartdm_register(&console, BLSP_UART2_BASE);
}

void bl31_plat_arch_setup(void)
{
	mmap_add_region(BL31_BASE, BL31_BASE, BL31_END - BL31_BASE,
			MT_RW_DATA | MT_SECURE);
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);
	mmap_add_region(BL_RO_DATA_BASE, BL_RO_DATA_BASE,
			BL_RO_DATA_END - BL_RO_DATA_BASE,
			MT_RO_DATA | MT_SECURE);
	mmap_add_region(BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_BASE,
			BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE,
			MT_DEVICE | MT_RW | MT_SECURE | MT_EXECUTE_NEVER);

	mmap_add(msm8916_mmap);
	init_xlat_tables();
	enable_mmu_el3(0);
}

static void msm8916_configure_timer(void)
{
	/* Set timer frequency */
	mmio_write_32(APCS_QTMR + CNTCTLBASE_CNTFRQ, plat_get_syscnt_freq2());

	/* Make frame 0 available to non-secure world */
	mmio_write_32(APCS_QTMR + CNTNSAR, BIT_32(CNTNSAR_NS_SHIFT(0)));
	mmio_write_32(APCS_QTMR + CNTACR_BASE(0),
		      BIT_32(CNTACR_RPCT_SHIFT) | BIT_32(CNTACR_RVCT_SHIFT) |
		      BIT_32(CNTACR_RFRQ_SHIFT) | BIT_32(CNTACR_RVOFF_SHIFT) |
		      BIT_32(CNTACR_RWVT_SHIFT) | BIT_32(CNTACR_RWPT_SHIFT));
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

	/* Make sure all further warm boots end up in BL31 and aarch64 state */
	CASSERT((BL31_BASE & 0xffff) == 0, assert_bl31_base_64k_aligned);
	mmio_write_32(APCS_BOOT_START_ADDR_SEC, BL31_BASE | REMAP_EN);
	mmio_write_32(APCS_AA64NAA32_REG, 1);
}

/*
 * MSM8916 has a special "interrupt aggregation logic" in the APPS SMMU,
 * which allows routing context bank interrupts to one of 3 interrupt numbers
 * ("TZ/HYP/NS"). Route all interrupts to the non-secure interrupt number
 * by default to avoid special setup on the non-secure side.
 */
#define GCC_SMMU_CFG_CBCR			(GCC_BASE + 0x12038)
#define GCC_APCS_SMMU_CLOCK_BRANCH_ENA_VOTE	(GCC_BASE + 0x4500c)
#define SMMU_CFG_CLK_ENA			BIT_32(12)
#define APPS_SMMU_INTR_SEL_NS			(APPS_SMMU_QCOM + 0x2000)
#define APPS_SMMU_INTR_SEL_NS_EN_ALL		U(0xffffffff)

static void msm8916_configure_smmu(void)
{
	/* Enable SMMU configuration clock to enable register access */
	mmio_setbits_32(GCC_APCS_SMMU_CLOCK_BRANCH_ENA_VOTE, SMMU_CFG_CLK_ENA);
	while (mmio_read_32(GCC_SMMU_CFG_CBCR) & CLK_OFF)
		;

	/* Route all context bank interrupts to non-secure interrupt */
	mmio_write_32(APPS_SMMU_INTR_SEL_NS, APPS_SMMU_INTR_SEL_NS_EN_ALL);

	/* Disable configuration clock again */
	mmio_clrbits_32(GCC_APCS_SMMU_CLOCK_BRANCH_ENA_VOTE, SMMU_CFG_CLK_ENA);
}

void bl31_platform_setup(void)
{
	INFO("BL31: Platform setup start\n");
	generic_delay_timer_init();
	msm8916_configure_timer();
	msm8916_gicv2_init();
	msm8916_configure_cpu_pm();
	msm8916_configure_smmu();
	INFO("BL31: Platform setup done\n");
}

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	switch (type) {
	case SECURE:
		return &image_ep_info.bl32;
	case NON_SECURE:
		return &image_ep_info.bl33;
	default:
		assert(sec_state_is_valid(type));
		return NULL;
	}
}
