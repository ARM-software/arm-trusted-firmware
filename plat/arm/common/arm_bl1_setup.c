/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <arch.h>
#include <bl1/bl1.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#if TRANSFER_LIST
#include <lib/transfer_list.h>
#endif
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak bl1_early_platform_setup
#pragma weak bl1_plat_arch_setup
#pragma weak bl1_plat_sec_mem_layout
#pragma weak arm_bl1_early_platform_setup
#pragma weak bl1_plat_prepare_exit
#pragma weak bl1_plat_get_next_image_id
#pragma weak plat_arm_bl1_fwu_needed
#pragma weak arm_bl1_plat_arch_setup
#pragma weak arm_bl1_platform_setup

#define MAP_BL1_TOTAL		MAP_REGION_FLAT(			\
					bl1_tzram_layout.total_base,	\
					bl1_tzram_layout.total_size,	\
					MT_MEMORY | MT_RW | EL3_PAS)
/*
 * If SEPARATE_CODE_AND_RODATA=1 we define a region for each section
 * otherwise one region is defined containing both
 */
#if SEPARATE_CODE_AND_RODATA
#define MAP_BL1_RO		MAP_REGION_FLAT(			\
					BL_CODE_BASE,			\
					BL1_CODE_END - BL_CODE_BASE,	\
					MT_CODE | EL3_PAS),		\
				MAP_REGION_FLAT(			\
					BL1_RO_DATA_BASE,		\
					BL1_RO_DATA_END			\
						- BL_RO_DATA_BASE,	\
					MT_RO_DATA | EL3_PAS)
#else
#define MAP_BL1_RO		MAP_REGION_FLAT(			\
					BL_CODE_BASE,			\
					BL1_CODE_END - BL_CODE_BASE,	\
					MT_CODE | EL3_PAS)
#endif

/* Data structure which holds the extents of the trusted SRAM for BL1*/
static meminfo_t bl1_tzram_layout;

/* Boolean variable to hold condition whether firmware update needed or not */
static bool is_fwu_needed;

#if TRANSFER_LIST
static struct transfer_list_header *secure_tl;
#endif

struct meminfo *bl1_plat_sec_mem_layout(void)
{
	return &bl1_tzram_layout;
}

/*******************************************************************************
 * BL1 specific platform actions shared between ARM standard platforms.
 ******************************************************************************/
void arm_bl1_early_platform_setup(void)
{

#if !ARM_DISABLE_TRUSTED_WDOG
	/* Enable watchdog */
	plat_arm_secure_wdt_start();
#endif

	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

	/* Allow BL1 to see the whole Trusted RAM */
	bl1_tzram_layout.total_base = ARM_BL_RAM_BASE;
	bl1_tzram_layout.total_size = ARM_BL_RAM_SIZE;
}

void bl1_early_platform_setup(void)
{
	arm_bl1_early_platform_setup();

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_arm_interconnect_init();
	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 */
	plat_arm_interconnect_enter_coherency();
}

/******************************************************************************
 * Perform the very early platform specific architecture setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl1_arch_setup()) does not do anything platform
 * specific.
 *****************************************************************************/
void arm_bl1_plat_arch_setup(void)
{
#if USE_COHERENT_MEM
	/* Ensure ARM platforms don't use coherent memory in BL1. */
	assert((BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE) == 0U);
#endif

	const mmap_region_t bl_regions[] = {
		MAP_BL1_TOTAL,
		MAP_BL1_RO,
#if USE_ROMLIB
		ARM_MAP_ROMLIB_CODE,
		ARM_MAP_ROMLIB_DATA,
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());
#ifdef __aarch64__
	enable_mmu_el3(0);
#else
	enable_mmu_svc_mon(0);
#endif /* __aarch64__ */

	arm_setup_romlib();
}

void bl1_plat_arch_setup(void)
{
	arm_bl1_plat_arch_setup();
}

/*
 * Perform the platform specific architecture setup shared between
 * ARM standard platforms.
 */
void arm_bl1_platform_setup(void)
{
	const struct dyn_cfg_dtb_info_t *config_info __unused;
	uint32_t fw_config_max_size __unused;
	image_info_t config_image_info __unused;
	struct transfer_list_entry *te __unused;

	image_desc_t *desc;

	int err = -1;

	/* Initialise the IO layer and register platform IO devices */
	plat_arm_io_setup();

	/* Check if we need FWU before further processing */
	is_fwu_needed = plat_arm_bl1_fwu_needed();
	if (is_fwu_needed) {
		ERROR("Skip platform setup as FWU detected\n");
		return;
	}

#if TRANSFER_LIST
	secure_tl = transfer_list_init((void *)PLAT_ARM_EL3_FW_HANDOFF_BASE,
				       PLAT_ARM_FW_HANDOFF_SIZE);

	if (secure_tl == NULL) {
		ERROR("Secure transfer list initialisation failed!\n");
		panic();
	}

	te = transfer_list_add(secure_tl, TL_TAG_TB_FW_CONFIG,
			       ARM_TB_FW_CONFIG_MAX_SIZE, NULL);
	assert(te != NULL);

	/*
	 * Set the load address of TB_FW_CONFIG in the data section of the TE just
	 * allocated in the secure transfer list.
	 */
	SET_PARAM_HEAD(&config_image_info, PARAM_IMAGE_BINARY, VERSION_2, 0);
	config_image_info.image_base = (uintptr_t)transfer_list_entry_data(te);
	config_image_info.image_max_size = te->data_size;

	VERBOSE("FCONF: Loading config with image ID: %u\n", TB_FW_CONFIG_ID);
	err = load_auth_image(TB_FW_CONFIG_ID, &config_image_info);
	if (err != 0) {
		VERBOSE("Failed to load config %u\n", TB_FW_CONFIG_ID);
		plat_error_handler(err);
	}

	transfer_list_update_checksum(secure_tl);
	fconf_populate("TB_FW", (uintptr_t)transfer_list_entry_data(te));
#else
	/* Set global DTB info for fixed fw_config information */
	fw_config_max_size = ARM_FW_CONFIG_LIMIT - ARM_FW_CONFIG_BASE;
	set_config_info(ARM_FW_CONFIG_BASE, ~0UL, fw_config_max_size, FW_CONFIG_ID);

	/* Fill the device tree information struct with the info from the config dtb */
	err = fconf_load_config(FW_CONFIG_ID);
	if (err < 0) {
		ERROR("Loading of FW_CONFIG failed %d\n", err);
		plat_error_handler(err);
	}

	/*
	 * FW_CONFIG loaded successfully. If FW_CONFIG device tree parsing
	 * is successful then load TB_FW_CONFIG device tree.
	 */
	config_info = FCONF_GET_PROPERTY(dyn_cfg, dtb, FW_CONFIG_ID);
	if (config_info != NULL) {
		err = fconf_populate_dtb_registry(config_info->config_addr);
		if (err < 0) {
			ERROR("Parsing of FW_CONFIG failed %d\n", err);
			plat_error_handler(err);
		}

		/* load TB_FW_CONFIG */
		err = fconf_load_config(TB_FW_CONFIG_ID);
		if (err < 0) {
			ERROR("Loading of TB_FW_CONFIG failed %d\n", err);
			plat_error_handler(err);
		}
	} else {
		ERROR("Invalid FW_CONFIG address\n");
		plat_error_handler(err);
	}
#endif /* TRANSFER_LIST */

	desc = bl1_plat_get_image_desc(BL2_IMAGE_ID);

#if TRANSFER_LIST
	transfer_list_set_handoff_args(secure_tl, &desc->ep_info);
#else
	/* The BL2 ep_info arg0 is modified to point to FW_CONFIG */
	assert(desc != NULL);
	desc->ep_info.args.arg0 = config_info->config_addr;
#endif /* TRANSFER_LIST */

#if CRYPTO_SUPPORT
	/* Share the Mbed TLS heap info with other images */
	arm_bl1_set_mbedtls_heap();
#endif /* CRYPTO_SUPPORT */

	/*
	 * Allow access to the System counter timer module and program
	 * counter frequency for non secure images during FWU
	 */
#ifdef ARM_SYS_TIMCTL_BASE
	arm_configure_sys_timer();
#endif
#if (ARM_ARCH_MAJOR > 7) || defined(ARMV7_SUPPORTS_GENERIC_TIMER)
	write_cntfrq_el0(plat_get_syscnt_freq2());
#endif
}

void bl1_plat_prepare_exit(entry_point_info_t *ep_info)
{
#if !ARM_DISABLE_TRUSTED_WDOG
	/* Disable watchdog before leaving BL1 */
	plat_arm_secure_wdt_stop();
#endif

#ifdef EL3_PAYLOAD_BASE
	/*
	 * Program the EL3 payload's entry point address into the CPUs mailbox
	 * in order to release secondary CPUs from their holding pen and make
	 * them jump there.
	 */
	plat_arm_program_trusted_mailbox(ep_info->pc);
	dsbsy();
	sev();
#endif
}

/*
 * On Arm platforms, the FWU process is triggered when the FIP image has
 * been tampered with.
 */
bool plat_arm_bl1_fwu_needed(void)
{
	return !arm_io_is_toc_valid();
}

/*******************************************************************************
 * The following function checks if Firmware update is needed,
 * by checking if TOC in FIP image is valid or not.
 ******************************************************************************/
unsigned int bl1_plat_get_next_image_id(void)
{
	return  is_fwu_needed ? NS_BL1U_IMAGE_ID : BL2_IMAGE_ID;
}

// Use the default implementation of this function when Firmware Handoff is
// disabled to avoid duplicating its logic.
#if TRANSFER_LIST
int bl1_plat_handle_post_image_load(unsigned int image_id)
{
	image_desc_t *image_desc __unused;

	assert(image_id == BL2_IMAGE_ID);
	struct transfer_list_entry *te;

	/* Convey this information to BL2 via its TL. */
	te = transfer_list_add(secure_tl, TL_TAG_SRAM_LAYOUT64,
			       sizeof(meminfo_t), NULL);
	assert(te != NULL);

	bl1_plat_calc_bl2_layout(&bl1_tzram_layout,
				 (meminfo_t *)transfer_list_entry_data(te));

	transfer_list_update_checksum(secure_tl);

	/**
	 * Before exiting make sure the contents of the TL are flushed in case there's no
	 * support for hardware cache coherency.
	 */
	flush_dcache_range((uintptr_t)secure_tl, secure_tl->size);
	return 0;
}
#endif /* TRANSFER_LIST*/
