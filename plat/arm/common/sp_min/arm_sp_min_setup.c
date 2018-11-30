/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <bl32/sp_min/platform_sp_min.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/pl011.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

static entry_point_info_t bl33_image_ep_info;

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak sp_min_platform_setup
#pragma weak sp_min_plat_arch_setup
#pragma weak plat_arm_sp_min_early_platform_setup

#define MAP_BL_SP_MIN_TOTAL	MAP_REGION_FLAT(			\
					BL32_BASE,			\
					BL32_END - BL32_BASE,		\
					MT_MEMORY | MT_RW | MT_SECURE)

/*
 * Check that BL32_BASE is above ARM_TB_FW_CONFIG_LIMIT. The reserved page
 * is required for SOC_FW_CONFIG/TOS_FW_CONFIG passed from BL2.
 */
CASSERT(BL32_BASE >= ARM_TB_FW_CONFIG_LIMIT, assert_bl32_base_overflows);

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *sp_min_plat_get_bl33_ep_info(void)
{
	entry_point_info_t *next_image_info;

	next_image_info = &bl33_image_ep_info;

	/*
	 * None of the images on the ARM development platforms can have 0x0
	 * as the entrypoint
	 */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Utility function to perform early platform setup.
 ******************************************************************************/
void arm_sp_min_early_platform_setup(void *from_bl2, uintptr_t tos_fw_config,
			uintptr_t hw_config, void *plat_params_from_bl2)
{
	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

#if RESET_TO_SP_MIN
	/* There are no parameters from BL2 if SP_MIN is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	/*
	 * Tell SP_MIN where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = arm_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

# if ARM_LINUX_KERNEL_AS_BL33
	/*
	 * According to the file ``Documentation/arm/Booting`` of the Linux
	 * kernel tree, Linux expects:
	 * r0 = 0
	 * r1 = machine type number, optional in DT-only platforms (~0 if so)
	 * r2 = Physical address of the device tree blob
	 */
	bl33_image_ep_info.args.arg0 = 0U;
	bl33_image_ep_info.args.arg1 = ~0U;
	bl33_image_ep_info.args.arg2 = (u_register_t)ARM_PRELOADED_DTB_BASE;
# endif

#else /* RESET_TO_SP_MIN */

	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;
	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL33_IMAGE_ID) {
			bl33_image_ep_info = *bl_params->ep_info;
			break;
		}

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0)
		panic();

#endif /* RESET_TO_SP_MIN */

}

/*******************************************************************************
 * Default implementation for sp_min_platform_setup2() for ARM platforms
 ******************************************************************************/
void plat_arm_sp_min_early_platform_setup(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	arm_sp_min_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_arm_interconnect_init();

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 * Earlier bootloader stages might already do this (e.g. Trusted
	 * Firmware's BL1 does it) but we can't assume so. There is no harm in
	 * executing this code twice anyway.
	 * Platform specific PSCI code will enable coherency for other
	 * clusters.
	 */
	plat_arm_interconnect_enter_coherency();
}

void sp_min_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	plat_arm_sp_min_early_platform_setup(arg0, arg1, arg2, arg3);
}

/*******************************************************************************
 * Perform any SP_MIN platform runtime setup prior to SP_MIN exit.
 * Common to ARM standard platforms.
 ******************************************************************************/
void arm_sp_min_plat_runtime_setup(void)
{
	/* Initialize the runtime console */
	arm_console_runtime_init();
}

/*******************************************************************************
 * Perform platform specific setup for SP_MIN
 ******************************************************************************/
void sp_min_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();

	/*
	 * Do initial security configuration to allow DRAM/device access
	 * (if earlier BL has not already done so).
	 */
#if RESET_TO_SP_MIN
	plat_arm_security_setup();

#if defined(PLAT_ARM_MEM_PROT_ADDR)
	arm_nor_psci_do_dyn_mem_protect();
#endif /* PLAT_ARM_MEM_PROT_ADDR */

#endif

	/* Enable and initialize the System level generic timer */
#ifdef ARM_SYS_CNTCTL_BASE
	mmio_write_32(ARM_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0U) | CNTCR_EN);
#endif
#ifdef ARM_SYS_TIMCTL_BASE
	/* Allow access to the System counter timer module */
	arm_configure_sys_timer();
#endif
	/* Initialize power controller before setting up topology */
	plat_arm_pwrc_setup();
}

void sp_min_plat_runtime_setup(void)
{
	arm_sp_min_plat_runtime_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only initializes the MMU
 ******************************************************************************/
void sp_min_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL_SP_MIN_TOTAL,
		ARM_MAP_BL_RO,
#if USE_COHERENT_MEM
		ARM_MAP_BL_COHERENT_RAM,
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());

	enable_mmu_svc_mon(0);
}
