/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
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
#include <lib/gpt_rme/gpt_rme.h>
#if TRANSFER_LIST
#include <lib/transfer_list.h>
#endif
#ifdef SPD_opteed
#include <lib/optee_utils.h>
#endif
#include <lib/utils.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

/* Base address of fw_config received from BL1 */
static uintptr_t config_base __unused;

/*
 * Check that BL2_BASE is above ARM_FW_CONFIG_LIMIT. This reserved page is
 * for `meminfo_t` data structure and fw_configs passed from BL1.
 */
#if TRANSFER_LIST
CASSERT(BL2_BASE >= PLAT_ARM_EL3_FW_HANDOFF_BASE + PLAT_ARM_FW_HANDOFF_SIZE,
	assert_bl2_base_overflows);
#else
CASSERT(BL2_BASE >= ARM_FW_CONFIG_LIMIT, assert_bl2_base_overflows);
#endif /* TRANSFER_LIST */

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

static struct transfer_list_header *secure_tl __unused;
static struct transfer_list_header *ns_tl __unused;

/*******************************************************************************
 * BL1 has passed the extents of the trusted SRAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted SRAM.
 * Copy it to a safe location before its reclaimed by later BL2 functionality.
 ******************************************************************************/
void arm_bl2_early_platform_setup(uintptr_t fw_config,
				  struct meminfo *mem_layout)
{
	struct transfer_list_entry *te __unused;
	int __maybe_unused ret;

	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

#if TRANSFER_LIST
	// TODO: modify the prototype of this function fw_config != bl2_tl
	secure_tl = (struct transfer_list_header *)fw_config;

	te = transfer_list_find(secure_tl, TL_TAG_SRAM_LAYOUT64);
	assert(te != NULL);

	bl2_tzram_layout = *(meminfo_t *)transfer_list_entry_data(te);
	transfer_list_rem(secure_tl, te);
#else
	config_base = fw_config;

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;
#endif

	/* Initialise the IO layer and register platform IO devices */
	plat_arm_io_setup();

	/* Load partition table */
#if ARM_GPT_SUPPORT
	ret = gpt_partition_init();
	if (ret != 0) {
		ERROR("GPT partition initialisation failed!\n");
		panic();
	}

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
#if TRANSFER_LIST
/* Assume the secure TL hasn't been initialised if BL2 is running at EL3. */
#if RESET_TO_BL2
	secure_tl = transfer_list_init((void *)PLAT_ARM_EL3_FW_HANDOFF_BASE,
				       PLAT_ARM_FW_HANDOFF_SIZE);

	if (secure_tl == NULL) {
		ERROR("Secure transfer list initialisation failed!\n");
		panic();
	}
#endif

	arm_transfer_list_dyn_cfg_init(secure_tl);
#else
	arm_bl2_dyn_cfg_init();
#endif

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

#if TRANSFER_LIST
	ns_tl = transfer_list_init((void *)FW_NS_HANDOFF_BASE,
				   PLAT_ARM_FW_HANDOFF_SIZE);

	if (ns_tl == NULL) {
		ERROR("Non-secure transfer list initialisation failed!");
		panic();
	}
#endif
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here.
 * When RME is enabled the secure environment is initialised before
 * initialising and enabling Granule Protection.
 * This function initialises the MMU in a quick and dirty way.
 ******************************************************************************/
void arm_bl2_plat_arch_setup(void)
{
#if USE_COHERENT_MEM
	/* Ensure ARM platforms don't use coherent memory in BL2. */
	assert((BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE) == 0U);
#endif

	const mmap_region_t bl_regions[] = {
		MAP_BL2_TOTAL,
		ARM_MAP_BL_RO,
#if USE_ROMLIB
		ARM_MAP_ROMLIB_CODE,
		ARM_MAP_ROMLIB_DATA,
#endif
#if !TRANSFER_LIST
		ARM_MAP_BL_CONFIG_REGION,
#endif /* TRANSFER_LIST */
#if ENABLE_RME
		ARM_MAP_L0_GPT_REGION,
#endif
		{ 0 }
	};

#if ENABLE_RME
	/* Initialise the secure environment */
	plat_arm_security_setup();
#endif
	setup_page_tables(bl_regions, plat_arm_get_mmap());

#ifdef __aarch64__
#if ENABLE_RME
	/* BL2 runs in EL3 when RME enabled. */
	assert(is_feat_rme_present());
	enable_mmu_el3(0);

	/* Initialise and enable granule protection after MMU. */
	arm_gpt_setup();
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
	const struct dyn_cfg_dtb_info_t *tb_fw_config_info __unused;
	struct transfer_list_entry *te __unused;
	arm_bl2_plat_arch_setup();

#if TRANSFER_LIST
	te = transfer_list_find(secure_tl, TL_TAG_TB_FW_CONFIG);
	assert(te != NULL);

	fconf_populate("TB_FW", (uintptr_t)transfer_list_entry_data(te));
	transfer_list_rem(secure_tl, te);
#else
	/* Fill the properties struct with the info from the config dtb */
	fconf_populate("FW_CONFIG", config_base);

	/* TB_FW_CONFIG was also loaded by BL1 */
	tb_fw_config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, TB_FW_CONFIG_ID);
	assert(tb_fw_config_info != NULL);

	fconf_populate("TB_FW", tb_fw_config_info->config_addr);
#endif
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

#if TRANSFER_LIST
	if (image_id == HW_CONFIG_ID) {
		arm_transfer_list_copy_hw_config(secure_tl, ns_tl);
	}
#endif /* TRANSFER_LIST */

	return arm_bl2_handle_post_image_load(image_id);
}

void arm_bl2_setup_next_ep_info(bl_mem_params_node_t *next_param_node)
{
	assert(transfer_list_set_handoff_args(
		       secure_tl, &next_param_node->ep_info) != NULL);

	arm_transfer_list_populate_ep_info(next_param_node, secure_tl, ns_tl);
}
