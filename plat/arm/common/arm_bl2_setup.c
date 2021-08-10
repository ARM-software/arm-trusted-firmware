/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch_features.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/partition/partition.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#if ENABLE_RME
#include <lib/gpt_rme/gpt_rme.h>
#endif /* ENABLE_RME */
#ifdef SPD_opteed
#include <lib/optee_utils.h>
#endif
#include <lib/utils.h>
#if ENABLE_RME
#include <plat/arm/common/arm_pas_def.h>
#endif /* ENABLE_RME */
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

/* Base address of fw_config received from BL1 */
static uintptr_t config_base;

/*
 * Check that BL2_BASE is above ARM_FW_CONFIG_LIMIT. This reserved page is
 * for `meminfo_t` data structure and fw_configs passed from BL1.
 */
CASSERT(BL2_BASE >= ARM_FW_CONFIG_LIMIT, assert_bl2_base_overflows);

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak bl2_early_platform_setup2
#pragma weak bl2_platform_setup
#pragma weak bl2_plat_arch_setup
#pragma weak bl2_plat_sec_mem_layout

#if ENABLE_RME
#define MAP_BL2_TOTAL		MAP_REGION_FLAT(			\
					bl2_tzram_layout.total_base,	\
					bl2_tzram_layout.total_size,	\
					MT_MEMORY | MT_RW | MT_ROOT)
#else
#define MAP_BL2_TOTAL		MAP_REGION_FLAT(			\
					bl2_tzram_layout.total_base,	\
					bl2_tzram_layout.total_size,	\
					MT_MEMORY | MT_RW | MT_SECURE)
#endif /* ENABLE_RME */

#pragma weak arm_bl2_plat_handle_post_image_load

/*******************************************************************************
 * BL1 has passed the extents of the trusted SRAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted SRAM.
 * Copy it to a safe location before its reclaimed by later BL2 functionality.
 ******************************************************************************/
void arm_bl2_early_platform_setup(uintptr_t fw_config,
				  struct meminfo *mem_layout)
{
	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;

	config_base = fw_config;

	/* Initialise the IO layer and register platform IO devices */
	plat_arm_io_setup();

	/* Load partition table */
#if ARM_GPT_SUPPORT
	partition_init(GPT_IMAGE_ID);
#endif /* ARM_GPT_SUPPORT */

}

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1, u_register_t arg2, u_register_t arg3)
{
	arm_bl2_early_platform_setup((uintptr_t)arg0, (meminfo_t *)arg1);

	generic_delay_timer_init();
}

/*
 * Perform  BL2 preload setup. Currently we initialise the dynamic
 * configuration here.
 */
void bl2_plat_preload_setup(void)
{
	arm_bl2_dyn_cfg_init();

#if ARM_GPT_SUPPORT && !PSA_FWU_SUPPORT
	/* Always use the FIP from bank 0 */
	arm_set_fip_addr(0U);
#endif /* ARM_GPT_SUPPORT && !PSA_FWU_SUPPORT */
}

/*
 * Perform ARM standard platform setup.
 */
void arm_bl2_platform_setup(void)
{
#if !ENABLE_RME
	/* Initialize the secure environment */
	plat_arm_security_setup();
#endif

#if defined(PLAT_ARM_MEM_PROT_ADDR)
	arm_nor_psci_do_static_mem_protect();
#endif
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();
}

#if ENABLE_RME

static void arm_bl2_plat_gpt_setup(void)
{
	/*
	 * The GPT library might modify the gpt regions structure to optimize
	 * the layout, so the array cannot be constant.
	 */
	pas_region_t pas_regions[] = {
		ARM_PAS_KERNEL,
		ARM_PAS_SECURE,
		ARM_PAS_REALM,
		ARM_PAS_EL3_DRAM,
		ARM_PAS_GPTS
	};

	/* Initialize entire protected space to GPT_GPI_ANY. */
	if (gpt_init_l0_tables(GPCCR_PPS_4GB, ARM_L0_GPT_ADDR_BASE,
		ARM_L0_GPT_SIZE) < 0) {
		ERROR("gpt_init_l0_tables() failed!\n");
		panic();
	}

	/* Carve out defined PAS ranges. */
	if (gpt_init_pas_l1_tables(GPCCR_PGS_4K,
				   ARM_L1_GPT_ADDR_BASE,
				   ARM_L1_GPT_SIZE,
				   pas_regions,
				   (unsigned int)(sizeof(pas_regions) /
				   sizeof(pas_region_t))) < 0) {
		ERROR("gpt_init_pas_l1_tables() failed!\n");
		panic();
	}

	INFO("Enabling Granule Protection Checks\n");
	if (gpt_enable() < 0) {
		ERROR("gpt_enable() failed!\n");
		panic();
	}
}

#endif /* ENABLE_RME */

/*******************************************************************************
 * Perform the very early platform specific architectural setup here.
 * When RME is enabled the secure environment is initialised before
 * initialising and enabling Granule Protection.
 * This function initialises the MMU in a quick and dirty way.
 ******************************************************************************/
void arm_bl2_plat_arch_setup(void)
{
#if USE_COHERENT_MEM && !ARM_CRYPTOCELL_INTEG
	/*
	 * Ensure ARM platforms don't use coherent memory in BL2 unless
	 * cryptocell integration is enabled.
	 */
	assert((BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE) == 0U);
#endif

	const mmap_region_t bl_regions[] = {
		MAP_BL2_TOTAL,
		ARM_MAP_BL_RO,
#if USE_ROMLIB
		ARM_MAP_ROMLIB_CODE,
		ARM_MAP_ROMLIB_DATA,
#endif
#if ARM_CRYPTOCELL_INTEG
		ARM_MAP_BL_COHERENT_RAM,
#endif
		ARM_MAP_BL_CONFIG_REGION,
#if ENABLE_RME
		ARM_MAP_L0_GPT_REGION,
#endif
		{0}
	};

#if ENABLE_RME
	/* Initialise the secure environment */
	plat_arm_security_setup();
#endif
	setup_page_tables(bl_regions, plat_arm_get_mmap());

#ifdef __aarch64__
#if ENABLE_RME
	/* BL2 runs in EL3 when RME enabled. */
	assert(get_armv9_2_feat_rme_support() != 0U);
	enable_mmu_el3(0);

	/* Initialise and enable granule protection after MMU. */
	arm_bl2_plat_gpt_setup();
#else
	enable_mmu_el1(0);
#endif
#else
	enable_mmu_svc_mon(0);
#endif

	arm_setup_romlib();
}

void bl2_plat_arch_setup(void)
{
	const struct dyn_cfg_dtb_info_t *tb_fw_config_info;

	arm_bl2_plat_arch_setup();

	/* Fill the properties struct with the info from the config dtb */
	fconf_populate("FW_CONFIG", config_base);

	/* TB_FW_CONFIG was also loaded by BL1 */
	tb_fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, TB_FW_CONFIG_ID);
	assert(tb_fw_config_info != NULL);

	fconf_populate("TB_FW", tb_fw_config_info->config_addr);
}

int arm_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
#ifdef SPD_opteed
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif
	assert(bl_mem_params != NULL);

	switch (image_id) {
#ifdef __aarch64__
	case BL32_IMAGE_ID:
#ifdef SPD_opteed
		pager_mem_params = get_bl_mem_params_node(BL32_EXTRA1_IMAGE_ID);
		assert(pager_mem_params);

		paged_mem_params = get_bl_mem_params_node(BL32_EXTRA2_IMAGE_ID);
		assert(paged_mem_params);

		err = parse_optee_header(&bl_mem_params->ep_info,
				&pager_mem_params->image_info,
				&paged_mem_params->image_info);
		if (err != 0) {
			WARN("OPTEE header parse error.\n");
		}
#endif
		bl_mem_params->ep_info.spsr = arm_get_spsr_for_bl32_entry();
		break;
#endif

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = arm_get_spsr_for_bl33_entry();
		break;

#ifdef SCP_BL2_BASE
	case SCP_BL2_IMAGE_ID:
		/* The subsequent handling of SCP_BL2 is platform specific */
		err = plat_arm_bl2_handle_scp_bl2(&bl_mem_params->image_info);
		if (err) {
			WARN("Failure in platform-specific handling of SCP_BL2 image.\n");
		}
		break;
#endif
	default:
		/* Do nothing in default case */
		break;
	}

	return err;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int arm_bl2_plat_handle_post_image_load(unsigned int image_id)
{
#if defined(SPD_spmd) && BL2_ENABLE_SP_LOAD
	/* For Secure Partitions we don't need post processing */
	if ((image_id >= (MAX_NUMBER_IDS - MAX_SP_IDS)) &&
		(image_id < MAX_NUMBER_IDS)) {
		return 0;
	}
#endif
	return arm_bl2_handle_post_image_load(image_id);
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return arm_bl2_plat_handle_post_image_load(image_id);
}
