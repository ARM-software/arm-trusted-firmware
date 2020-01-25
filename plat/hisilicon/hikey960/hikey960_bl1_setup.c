/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <bl1/tbbr/tbbr_img_desc.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/interrupt_props.h>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/pl011.h>
#include <drivers/delay_timer.h>
#include <drivers/dw_ufs.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/ufs.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <hi3660.h>
#include "hikey960_def.h"
#include "hikey960_private.h"

enum {
	BOOT_MODE_RECOVERY = 0,
	BOOT_MODE_NORMAL,
	BOOT_MODE_MASK = 1,
};

/*
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted RAM
 */

/* Data structure which holds the extents of the trusted RAM for BL1 */
static meminfo_t bl1_tzram_layout;
static console_t console;

/******************************************************************************
 * On a GICv2 system, the Group 1 secure interrupts are treated as Group 0
 * interrupts.
 *****************************************************************************/
static const interrupt_prop_t g0_interrupt_props[] = {
	INTR_PROP_DESC(IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY,
			GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
	INTR_PROP_DESC(IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY,
			GICV2_INTR_GROUP0, GIC_INTR_CFG_LEVEL),
};

const gicv2_driver_data_t hikey960_gic_data = {
	.gicd_base = GICD_REG_BASE,
	.gicc_base = GICC_REG_BASE,
	.interrupt_props = g0_interrupt_props,
	.interrupt_props_num = ARRAY_SIZE(g0_interrupt_props),
};

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/*
 * Perform any BL1 specific platform actions.
 */
void bl1_early_platform_setup(void)
{
	unsigned int id, uart_base;

	generic_delay_timer_init();
	hikey960_read_boardid(&id);
	if (id == 5300)
		uart_base = PL011_UART5_BASE;
	else
		uart_base = PL011_UART6_BASE;
	/* Initialize the console to provide early debug support */
	console_pl011_register(uart_base, PL011_UART_CLK_IN_HZ,
			       PL011_BAUDRATE, &console);

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = BL1_RW_BASE;
	bl1_tzram_layout.total_size = BL1_RW_SIZE;

	INFO("BL1: 0x%lx - 0x%lx [size = %lu]\n", BL1_RAM_BASE, BL1_RAM_LIMIT,
	     BL1_RAM_LIMIT - BL1_RAM_BASE); /* bl1_size */
}

/*
 * Perform the very early platform specific architecture setup here. At the
 * moment this only does basic initialization. Later architectural setup
 * (bl1_arch_setup()) does not do anything platform specific.
 */
void bl1_plat_arch_setup(void)
{
	hikey960_init_mmu_el3(bl1_tzram_layout.total_base,
			      bl1_tzram_layout.total_size,
			      BL1_RO_BASE,
			      BL1_RO_LIMIT,
			      BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END);
}

static void hikey960_ufs_reset(void)
{
	unsigned int data, mask;

	mmio_write_32(CRG_PERDIS7_REG, 1 << 14);
	mmio_clrbits_32(UFS_SYS_PHY_CLK_CTRL_REG, BIT_SYSCTRL_REF_CLOCK_EN);
	do {
		data = mmio_read_32(UFS_SYS_PHY_CLK_CTRL_REG);
	} while (data & BIT_SYSCTRL_REF_CLOCK_EN);
	/* use abb clk */
	mmio_clrbits_32(UFS_SYS_UFS_SYSCTRL_REG, BIT_UFS_REFCLK_SRC_SE1);
	mmio_clrbits_32(UFS_SYS_PHY_ISO_EN_REG, BIT_UFS_REFCLK_ISO_EN);
	mmio_write_32(PCTRL_PERI_CTRL3_REG, (1 << 0) | (1 << 16));
	mdelay(1);
	mmio_write_32(CRG_PEREN7_REG, 1 << 14);
	mmio_setbits_32(UFS_SYS_PHY_CLK_CTRL_REG, BIT_SYSCTRL_REF_CLOCK_EN);

	mmio_write_32(CRG_PERRSTEN3_REG, PERI_UFS_BIT);
	do {
		data = mmio_read_32(CRG_PERRSTSTAT3_REG);
	} while ((data & PERI_UFS_BIT) == 0);
	mmio_setbits_32(UFS_SYS_PSW_POWER_CTRL_REG, BIT_UFS_PSW_MTCMOS_EN);
	mdelay(1);
	mmio_setbits_32(UFS_SYS_HC_LP_CTRL_REG, BIT_SYSCTRL_PWR_READY);
	mmio_write_32(UFS_SYS_UFS_DEVICE_RESET_CTRL_REG,
		      MASK_UFS_DEVICE_RESET);
	/* clear SC_DIV_UFS_PERIBUS */
	mask = SC_DIV_UFS_PERIBUS << 16;
	mmio_write_32(CRG_CLKDIV17_REG, mask);
	/* set SC_DIV_UFSPHY_CFG(3) */
	mask = SC_DIV_UFSPHY_CFG_MASK << 16;
	data = SC_DIV_UFSPHY_CFG(3);
	mmio_write_32(CRG_CLKDIV16_REG, mask | data);
	data = mmio_read_32(UFS_SYS_PHY_CLK_CTRL_REG);
	data &= ~MASK_SYSCTRL_CFG_CLOCK_FREQ;
	data |= 0x39;
	mmio_write_32(UFS_SYS_PHY_CLK_CTRL_REG, data);
	mmio_clrbits_32(UFS_SYS_PHY_CLK_CTRL_REG, MASK_SYSCTRL_REF_CLOCK_SEL);
	mmio_setbits_32(UFS_SYS_CLOCK_GATE_BYPASS_REG,
			MASK_UFS_CLK_GATE_BYPASS);
	mmio_setbits_32(UFS_SYS_UFS_SYSCTRL_REG, MASK_UFS_SYSCTRL_BYPASS);

	mmio_setbits_32(UFS_SYS_PSW_CLK_CTRL_REG, BIT_SYSCTRL_PSW_CLK_EN);
	mmio_clrbits_32(UFS_SYS_PSW_POWER_CTRL_REG, BIT_UFS_PSW_ISO_CTRL);
	mmio_clrbits_32(UFS_SYS_PHY_ISO_EN_REG, BIT_UFS_PHY_ISO_CTRL);
	mmio_clrbits_32(UFS_SYS_HC_LP_CTRL_REG, BIT_SYSCTRL_LP_ISOL_EN);
	mmio_write_32(CRG_PERRSTDIS3_REG, PERI_ARST_UFS_BIT);
	mmio_setbits_32(UFS_SYS_RESET_CTRL_EN_REG, BIT_SYSCTRL_LP_RESET_N);
	mdelay(1);
	mmio_write_32(UFS_SYS_UFS_DEVICE_RESET_CTRL_REG,
		      MASK_UFS_DEVICE_RESET | BIT_UFS_DEVICE_RESET);
	mdelay(20);
	mmio_write_32(UFS_SYS_UFS_DEVICE_RESET_CTRL_REG,
		      0x03300330);

	mmio_write_32(CRG_PERRSTDIS3_REG, PERI_UFS_BIT);
	do {
		data = mmio_read_32(CRG_PERRSTSTAT3_REG);
	} while (data & PERI_UFS_BIT);
}

static void hikey960_ufs_init(void)
{
	dw_ufs_params_t ufs_params;

	memset(&ufs_params, 0, sizeof(ufs_params));
	ufs_params.reg_base = UFS_REG_BASE;
	ufs_params.desc_base = HIKEY960_UFS_DESC_BASE;
	ufs_params.desc_size = HIKEY960_UFS_DESC_SIZE;

	if ((ufs_params.flags & UFS_FLAGS_SKIPINIT) == 0)
		hikey960_ufs_reset();
	dw_ufs_init(&ufs_params);
}

/*
 * Function which will perform any remaining platform-specific setup that can
 * occur after the MMU and data cache have been enabled.
 */
void bl1_platform_setup(void)
{
	hikey960_clk_init();
	hikey960_pmu_init();
	hikey960_regulator_enable();
	hikey960_tzc_init();
	hikey960_peri_init();
	hikey960_ufs_init();
	hikey960_pinmux_init();
	hikey960_gpio_init();
	hikey960_io_setup();
}

/*
 * The following function checks if Firmware update is needed,
 * by checking if TOC in FIP image is valid or not.
 */
unsigned int bl1_plat_get_next_image_id(void)
{
	unsigned int mode, ret;

	mode = mmio_read_32(SCTRL_BAK_DATA0_REG);
	switch (mode & BOOT_MODE_MASK) {
	case BOOT_MODE_RECOVERY:
		ret = NS_BL1U_IMAGE_ID;
		break;
	default:
		WARN("Invalid boot mode is found:%d\n", mode);
		panic();
	}
	return ret;
}

image_desc_t *bl1_plat_get_image_desc(unsigned int image_id)
{
	unsigned int index = 0;

	while (bl1_tbbr_image_descs[index].image_id != INVALID_IMAGE_ID) {
		if (bl1_tbbr_image_descs[index].image_id == image_id)
			return &bl1_tbbr_image_descs[index];
		index++;
	}

	return NULL;
}

void bl1_plat_set_ep_info(unsigned int image_id,
		entry_point_info_t *ep_info)
{
	unsigned int data = 0;
	uintptr_t tmp = HIKEY960_NS_TMP_OFFSET;

	if (image_id != NS_BL1U_IMAGE_ID)
		panic();
	/* Copy NS BL1U from 0x1AC1_8000 to 0x1AC9_8000 */
	memcpy((void *)tmp, (void *)HIKEY960_NS_IMAGE_OFFSET,
		NS_BL1U_SIZE);
	memcpy((void *)NS_BL1U_BASE, (void *)tmp, NS_BL1U_SIZE);
	inv_dcache_range(NS_BL1U_BASE, NS_BL1U_SIZE);
	/* Initialize the GIC driver, cpu and distributor interfaces */
	gicv2_driver_init(&hikey960_gic_data);
	gicv2_distif_init();
	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
	/* CNTFRQ is read-only in EL1 */
	write_cntfrq_el0(plat_get_syscnt_freq2());
	data = read_cpacr_el1();
	do {
		data |= 3 << 20;
		write_cpacr_el1(data);
		data = read_cpacr_el1();
	} while ((data & (3 << 20)) != (3 << 20));
	INFO("cpacr_el1:0x%x\n", data);

	ep_info->args.arg0 = 0xffff & read_mpidr();
	ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS);
}
