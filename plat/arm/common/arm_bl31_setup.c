/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/debugfs.h>
#include <lib/extensions/ras.h>
#if ENABLE_RME
#include <lib/gpt_rme/gpt_rme.h>
#endif
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;
#if ENABLE_RME
static entry_point_info_t rmm_image_ep_info;
#endif

#if !RESET_TO_BL31
/*
 * Check that BL31_BASE is above ARM_FW_CONFIG_LIMIT. The reserved page
 * is required for SOC_FW_CONFIG/TOS_FW_CONFIG passed from BL2.
 */
CASSERT(BL31_BASE >= ARM_FW_CONFIG_LIMIT, assert_bl31_base_overflows);
#endif

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak bl31_early_platform_setup2
#pragma weak bl31_platform_setup
#pragma weak bl31_plat_arch_setup
#pragma weak bl31_plat_get_next_image_ep_info

#define MAP_BL31_TOTAL		MAP_REGION_FLAT(			\
					BL31_START,			\
					BL31_END - BL31_START,		\
					MT_MEMORY | MT_RW | EL3_PAS)
#if RECLAIM_INIT_CODE
IMPORT_SYM(unsigned long, __INIT_CODE_START__, BL_INIT_CODE_BASE);
IMPORT_SYM(unsigned long, __INIT_CODE_END__, BL_CODE_END_UNALIGNED);
IMPORT_SYM(unsigned long, __STACKS_END__, BL_STACKS_END_UNALIGNED);

#define	BL_INIT_CODE_END	((BL_CODE_END_UNALIGNED + PAGE_SIZE - 1) & \
					~(PAGE_SIZE - 1))
#define	BL_STACKS_END		((BL_STACKS_END_UNALIGNED + PAGE_SIZE - 1) & \
					~(PAGE_SIZE - 1))

#define MAP_BL_INIT_CODE	MAP_REGION_FLAT(			\
					BL_INIT_CODE_BASE,		\
					BL_INIT_CODE_END		\
						- BL_INIT_CODE_BASE,	\
					MT_CODE | EL3_PAS)
#endif

#if SEPARATE_NOBITS_REGION
#define MAP_BL31_NOBITS		MAP_REGION_FLAT(			\
					BL31_NOBITS_BASE,		\
					BL31_NOBITS_LIMIT 		\
						- BL31_NOBITS_BASE,	\
					MT_MEMORY | MT_RW | EL3_PAS)

#endif
/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
struct entry_point_info *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));
	if (type == NON_SECURE) {
		next_image_info = &bl33_image_ep_info;
	}
#if ENABLE_RME
	else if (type == REALM) {
		next_image_info = &rmm_image_ep_info;
	}
#endif
	else {
		next_image_info = &bl32_image_ep_info;
	}

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
 * Perform any BL31 early platform setup common to ARM standard platforms.
 * Here is an opportunity to copy parameters passed by the calling EL (S-EL1
 * in BL2 & EL3 in BL1) before they are lost (potentially). This needs to be
 * done before the MMU is initialized so that the memory layout can be used
 * while creating page tables. BL2 has flushed this information to memory, so
 * we are guaranteed to pick up good data.
 ******************************************************************************/
void __init arm_bl31_early_platform_setup(void *from_bl2, uintptr_t soc_fw_config,
				uintptr_t hw_config, void *plat_params_from_bl2)
{
	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

#if RESET_TO_BL31
	/* There are no parameters from BL2 if BL31 is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);

# ifdef BL32_BASE
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = arm_get_spsr_for_bl32_entry();

#if defined(SPD_spmd)
	/* SPM (hafnium in secure world) expects SPM Core manifest base address
	 * in x0, which in !RESET_TO_BL31 case loaded after base of non shared
	 * SRAM(after 4KB offset of SRAM). But in RESET_TO_BL31 case all non
	 * shared SRAM is allocated to BL31, so to avoid overwriting of manifest
	 * keep it in the last page.
	 */
	bl32_image_ep_info.args.arg0 = ARM_TRUSTED_SRAM_BASE +
				PLAT_ARM_TRUSTED_SRAM_SIZE - PAGE_SIZE;
#endif

# endif /* BL32_BASE */

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();

	bl33_image_ep_info.spsr = arm_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#else /* RESET_TO_BL31 */

	/*
	 * In debug builds, we pass a special value in 'plat_params_from_bl2'
	 * to verify platform parameters from BL2 to BL31.
	 * In release builds, it's not used.
	 */
	assert(((unsigned long long)plat_params_from_bl2) ==
		ARM_BL31_PLAT_PARAM_VAL);

	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;
	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33, BL32 and RMM (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params != NULL) {
		if (bl_params->image_id == BL32_IMAGE_ID) {
			bl32_image_ep_info = *bl_params->ep_info;
		}
#if ENABLE_RME
		else if (bl_params->image_id == RMM_IMAGE_ID) {
			rmm_image_ep_info = *bl_params->ep_info;
		}
#endif
		else if (bl_params->image_id == BL33_IMAGE_ID) {
			bl33_image_ep_info = *bl_params->ep_info;
		}

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0U)
		panic();
#if ENABLE_RME
	if (rmm_image_ep_info.pc == 0U)
		panic();
#endif
#endif /* RESET_TO_BL31 */

# if ARM_LINUX_KERNEL_AS_BL33
	/*
	 * According to the file ``Documentation/arm64/booting.txt`` of the
	 * Linux kernel tree, Linux expects the physical address of the device
	 * tree blob (DTB) in x0, while x1-x3 are reserved for future use and
	 * must be 0.
	 * Repurpose the option to load Hafnium hypervisor in the normal world.
	 * It expects its manifest address in x0. This is essentially the linux
	 * dts (passed to the primary VM) by adding 'hypervisor' and chosen
	 * nodes specifying the Hypervisor configuration.
	 */
#if RESET_TO_BL31
	bl33_image_ep_info.args.arg0 = (u_register_t)ARM_PRELOADED_DTB_BASE;
#else
	bl33_image_ep_info.args.arg0 = (u_register_t)hw_config;
#endif
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
# endif
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	arm_bl31_early_platform_setup((void *)arg0, arg1, arg2, (void *)arg3);

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

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
void arm_bl31_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();

#if RESET_TO_BL31
	/*
	 * Do initial security configuration to allow DRAM/device access
	 * (if earlier BL has not already done so).
	 */
	plat_arm_security_setup();

#if defined(PLAT_ARM_MEM_PROT_ADDR)
	arm_nor_psci_do_dyn_mem_protect();
#endif /* PLAT_ARM_MEM_PROT_ADDR */

#endif /* RESET_TO_BL31 */

	/* Enable and initialize the System level generic timer */
	mmio_write_32(ARM_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0U) | CNTCR_EN);

	/* Allow access to the System counter timer module */
	arm_configure_sys_timer();

	/* Initialize power controller before setting up topology */
	plat_arm_pwrc_setup();

#if RAS_EXTENSION
	ras_init();
#endif

#if USE_DEBUGFS
	debugfs_init();
#endif /* USE_DEBUGFS */
}

/*******************************************************************************
 * Perform any BL31 platform runtime setup prior to BL31 exit common to ARM
 * standard platforms
 * Perform BL31 platform setup
 ******************************************************************************/
void arm_bl31_plat_runtime_setup(void)
{
	console_switch_state(CONSOLE_FLAG_RUNTIME);

	/* Initialize the runtime console */
	arm_console_runtime_init();

#if RECLAIM_INIT_CODE
	arm_free_init_memory();
#endif

#if PLAT_RO_XLAT_TABLES
	arm_xlat_make_tables_readonly();
#endif
}

#if RECLAIM_INIT_CODE
/*
 * Make memory for image boot time code RW to reclaim it as stack for the
 * secondary cores, or RO where it cannot be reclaimed:
 *
 *            |-------- INIT SECTION --------|
 *  -----------------------------------------
 * |  CORE 0  |  CORE 1  |  CORE 2  | EXTRA  |
 * |  STACK   |  STACK   |  STACK   | SPACE  |
 *  -----------------------------------------
 *             <-------------------> <------>
 *                MAKE RW AND XN       MAKE
 *                  FOR STACKS       RO AND XN
 */
void arm_free_init_memory(void)
{
	int ret = 0;

	if (BL_STACKS_END < BL_INIT_CODE_END) {
		/* Reclaim some of the init section as stack if possible. */
		if (BL_INIT_CODE_BASE < BL_STACKS_END) {
			ret |= xlat_change_mem_attributes(BL_INIT_CODE_BASE,
					BL_STACKS_END - BL_INIT_CODE_BASE,
					MT_RW_DATA);
		}
		/* Make the rest of the init section read-only. */
		ret |= xlat_change_mem_attributes(BL_STACKS_END,
				BL_INIT_CODE_END - BL_STACKS_END,
				MT_RO_DATA);
	} else {
		/* The stacks cover the init section, so reclaim it all. */
		ret |= xlat_change_mem_attributes(BL_INIT_CODE_BASE,
				BL_INIT_CODE_END - BL_INIT_CODE_BASE,
				MT_RW_DATA);
	}

	if (ret != 0) {
		ERROR("Could not reclaim initialization code");
		panic();
	}
}
#endif

void __init bl31_platform_setup(void)
{
	arm_bl31_platform_setup();
}

void bl31_plat_runtime_setup(void)
{
	arm_bl31_plat_runtime_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup shared between
 * ARM standard platforms. This only does basic initialization. Later
 * architectural setup (bl31_arch_setup()) does not do anything platform
 * specific.
 ******************************************************************************/
void __init arm_bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL31_TOTAL,
#if ENABLE_RME
		ARM_MAP_L0_GPT_REGION,
#endif
#if RECLAIM_INIT_CODE
		MAP_BL_INIT_CODE,
#endif
#if SEPARATE_NOBITS_REGION
		MAP_BL31_NOBITS,
#endif
		ARM_MAP_BL_RO,
#if USE_ROMLIB
		ARM_MAP_ROMLIB_CODE,
		ARM_MAP_ROMLIB_DATA,
#endif
#if USE_COHERENT_MEM
		ARM_MAP_BL_COHERENT_RAM,
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_arm_get_mmap());

	enable_mmu_el3(0);

#if ENABLE_RME
	/*
	 * Initialise Granule Protection library and enable GPC for the primary
	 * processor. The tables have already been initialized by a previous BL
	 * stage, so there is no need to provide any PAS here. This function
	 * sets up pointers to those tables.
	 */
	if (gpt_runtime_init() < 0) {
		ERROR("gpt_runtime_init() failed!\n");
		panic();
	}
#endif /* ENABLE_RME */

	arm_setup_romlib();
}

void __init bl31_plat_arch_setup(void)
{
	arm_bl31_plat_arch_setup();
}
