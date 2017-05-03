/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <bl_common.h>
#include <common_def.h>
#include <console.h>
#include <debug.h>
#include <generic_delay_timer.h>
#include <mcucfg.h>
#include <mmio.h>
#include <mtcmos.h>
#include <plat_arm.h>
#include <plat_private.h>
#include <platform.h>
#include <spm.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
unsigned long __RO_START__;
unsigned long __RO_END__;

/*
 * The next 3 constants identify the extents of the code, RO data region and the
 * limit of the BL31 image.  These addresses are used by the MMU setup code and
 * therefore they must be page-aligned.  It is the responsibility of the linker
 * script to ensure that __RO_START__, __RO_END__ & __BL31_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)
#define BL31_END (unsigned long)(&__BL31_END__)

static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;

static void platform_setup_cpu(void)
{
	/* turn off all the little core's power except cpu 0 */
	mtcmos_little_cpu_off();

	/* setup big cores */
	mmio_write_32((uintptr_t)&mt8173_mcucfg->mp1_config_res,
		MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp1_miscdbg, MP1_AINACTS);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp1_clkenm_div,
		MP1_SW_CG_GEN);
	mmio_clrbits_32((uintptr_t)&mt8173_mcucfg->mp1_rst_ctl,
		MP1_L2RSTDISABLE);

	/* set big cores arm64 boot mode */
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp1_cpucfg,
		MP1_CPUCFG_64BIT);

	/* set LITTLE cores arm64 boot mode */
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp0_rv_addr[0].rv_addr_hw,
		MP0_CPUCFG_64BIT);

	/* enable dcm control */
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->bus_fabric_dcm_ctrl,
		ADB400_GRP_DCM_EN | CCI400_GRP_DCM_EN | ADBCLK_GRP_DCM_EN |
		EMICLK_GRP_DCM_EN | ACLK_GRP_DCM_EN | L2C_IDLE_DCM_EN |
		INFRACLK_PSYS_DYNAMIC_CG_EN);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->l2c_sram_ctrl,
		L2C_SRAM_DCM_EN);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->cci_clk_ctrl,
		MCU_BUS_DCM_EN);
}

static void platform_setup_sram(void)
{
	/* protect BL31 memory from non-secure read/write access */
	mmio_write_32(SRAMROM_SEC_ADDR, (uint32_t)(BL31_END + 0x3ff) & 0x3fc00);
	mmio_write_32(SRAMROM_SEC_CTRL, 0x10000ff9);
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL3-1 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 * BL2 has flushed this information to memory, so we are guaranteed to pick up
 * good data.
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t *from_bl2,
			       void *plat_params_from_bl2)
{
	console_init(MT8173_UART0_BASE, MT8173_UART_CLOCK, MT8173_BAUDRATE);

	VERBOSE("bl31_setup\n");

	assert(from_bl2 != NULL);
	assert(from_bl2->h.type == PARAM_BL31);
	assert(from_bl2->h.version >= VERSION_1);

	bl32_ep_info = *from_bl2->bl32_ep_info;
	bl33_ep_info = *from_bl2->bl33_ep_info;
}

/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	platform_setup_cpu();
	platform_setup_sram();

	generic_delay_timer_init();

	/* Initialize the gic cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();

#if ENABLE_PLAT_COMPAT
	/* Topologies are best known to the platform. */
	mt_setup_topology();
#endif

	/* Initialize spm at boot time */
	spm_boot_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	plat_cci_init();
	plat_cci_enable();

	plat_configure_mmu_el3(BL31_RO_BASE,
			       BL_COHERENT_RAM_END - BL31_RO_BASE,
			       BL31_RO_BASE,
			       BL31_RO_LIMIT,
			       BL_COHERENT_RAM_BASE,
			       BL_COHERENT_RAM_END);
}

