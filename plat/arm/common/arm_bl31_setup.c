/*
 * Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/console.h>
#include <lib/debugfs.h>
#include <lib/extensions/ras.h>
#include <lib/fconf/fconf.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/mmio.h>
#include <services/lfa_svc.h>
#if TRANSFER_LIST
#include <transfer_list.h>
#endif
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/common/plat_arm_lfa_components.h>
#include <plat/common/platform.h>
#include <platform_def.h>

struct transfer_list_header *secure_tl;
struct transfer_list_header *ns_tl __unused;

#if USE_GIC_DRIVER == 3
uintptr_t arm_gicr_base_addrs[2] = {
	PLAT_ARM_GICR_BASE,	/* GICR Base address of the primary CPU */
	0U			/* Zero Termination */
};
#endif

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

#if ENABLE_RME
static entry_point_info_t rmm_image_ep_info;
#if (RME_GPT_BITLOCK_BLOCK == 0)
#define BITLOCK_BASE	UL(0)
#define BITLOCK_SIZE	UL(0)
#else
/*
 * Number of bitlock_t entries in bitlocks array for PLAT_ARM_PPS
 * with RME_GPT_BITLOCK_BLOCK * 512MB per bitlock.
 */
#if (PLAT_ARM_PPS > (RME_GPT_BITLOCK_BLOCK * SZ_512M * UL(8)))
#define BITLOCKS_NUM	(PLAT_ARM_PPS) /	\
			(RME_GPT_BITLOCK_BLOCK * SZ_512M * UL(8))
#else
#define BITLOCKS_NUM	U(1)
#endif
/*
 * Bitlocks array
 */
static bitlock_t gpt_bitlock[BITLOCKS_NUM];
#define BITLOCK_BASE	(uintptr_t)gpt_bitlock
#define BITLOCK_SIZE	sizeof(gpt_bitlock)
#endif /* RME_GPT_BITLOCK_BLOCK */
#endif /* ENABLE_RME */

#if !RESET_TO_BL31
/*
 * Check that BL31_BASE is above ARM_FW_CONFIG_LIMIT. The reserved page
 * is required for SOC_FW_CONFIG/TOS_FW_CONFIG passed from BL2.
 */
#if TRANSFER_LIST
CASSERT(BL31_BASE >= PLAT_ARM_EL3_FW_HANDOFF_LIMIT, assert_bl31_base_overflows);
#else
CASSERT(BL31_BASE >= ARM_FW_CONFIG_LIMIT, assert_bl31_base_overflows);
#endif /* TRANSFER_LIST */
#endif /* RESET_TO_BL31 */

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak bl31_early_platform_setup2
#pragma weak bl31_platform_setup
#pragma weak bl31_plat_arch_setup
#pragma weak bl31_plat_get_next_image_ep_info
#pragma weak bl31_plat_runtime_setup

#define MAP_BL31_TOTAL		MAP_REGION_FLAT(			\
					BL31_START,			\
					BL31_END - BL31_START,		\
					MT_MEMORY | MT_RW | EL3_PAS |	\
					MT_CAP_LD_ST_TRACK)

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
#if TRANSFER_LIST && !RESET_TO_BL31
		next_image_info = transfer_list_set_handoff_args(
			ns_tl, &bl33_image_ep_info);
#else
		next_image_info = &bl33_image_ep_info;
#endif
	}
#if ENABLE_RME
	else if (type == REALM) {
#if LFA_SUPPORT
		if (lfa_is_prime_complete(LFA_RMM_COMPONENT)) {
			rmm_image_ep_info.pc =
					RMM_BASE + RMM_BANK_SIZE;
		}
#endif /* LFA_SUPPORT */
		next_image_info = &rmm_image_ep_info;
	}
#endif
	else {
#if TRANSFER_LIST && !RESET_TO_BL31
		next_image_info = transfer_list_set_handoff_args(
			secure_tl, &bl32_image_ep_info);
#else
		next_image_info = &bl32_image_ep_info;
#endif
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
void __init arm_bl31_early_platform_setup(u_register_t arg0, u_register_t arg1,
					  u_register_t arg2, u_register_t arg3)
{
#if TRANSFER_LIST
#if RESET_TO_BL31
	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	/*
	 * Tell BL31 where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();

	bl33_image_ep_info.spsr = arm_get_spsr(BL33_IMAGE_ID);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	bl33_image_ep_info.args.arg0 = PLAT_ARM_TRANSFER_LIST_DTB_OFFSET;
	bl33_image_ep_info.args.arg1 =
		TRANSFER_LIST_HANDOFF_X1_VALUE(REGISTER_CONVENTION_VERSION);
	bl33_image_ep_info.args.arg3 = FW_NS_HANDOFF_BASE;
#else
	struct transfer_list_entry *te = NULL;
	struct entry_point_info *ep;

	secure_tl = (struct transfer_list_header *)arg3;

	/*
	 * Populate the global entry point structures used to execute subsequent
	 * images.
	 */
	while ((te = transfer_list_next(secure_tl, te)) != NULL) {
		ep = transfer_list_entry_data(te);

		if (te->tag_id == TL_TAG_EXEC_EP_INFO64) {
			switch (GET_SECURITY_STATE(ep->h.attr)) {
			case NON_SECURE:
				bl33_image_ep_info = *ep;
				break;
#if ENABLE_RME
			case REALM:
				rmm_image_ep_info = *ep;
				break;
#endif
			case SECURE:
				bl32_image_ep_info = *ep;
				break;
			default:
				ERROR("Unrecognized Image Security State %lu\n",
				      GET_SECURITY_STATE(ep->h.attr));
				panic();
			}
		}
	}
#endif /* RESET_TO_BL31 */
#else /* (!TRANSFER_LIST) */
#if RESET_TO_BL31
	/* If BL31 is a reset vector, the parameters must be ignored */
	(void)arg0;
	(void)arg1;
	(void)arg2;
	(void)arg3;

# ifdef BL32_BASE
	/* Populate entry point information for BL32 */
	SET_PARAM_HEAD(&bl32_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	SET_SECURITY_STATE(bl32_image_ep_info.h.attr, SECURE);
	bl32_image_ep_info.pc = BL32_BASE;
	bl32_image_ep_info.spsr = arm_get_spsr(BL32_IMAGE_ID);

#if defined(SPD_spmd)
	bl32_image_ep_info.args.arg0 = ARM_SPMC_MANIFEST_BASE;
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
	bl33_image_ep_info.spsr = arm_get_spsr(BL33_IMAGE_ID);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#if ENABLE_RME
	/*
	 * Populate entry point information for RMM.
	 * Only PC needs to be set as other fields are determined by RMMD.
	 */
	rmm_image_ep_info.pc = RMM_BASE;
#endif /* ENABLE_RME */
#else /* RESET_TO_BL31 */
	/*
	 * In debug builds, we pass a special value in 'arg3'
	 * to verify platform parameters from BL2 to BL31.
	 * In release builds, it's not used.
	 */
#if DEBUG
	assert(((uintptr_t)arg3) == ARM_BL31_PLAT_PARAM_VAL);
#endif

	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)(uintptr_t)arg0;
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
#if SPMC_AT_EL3
			/*
			 * Populate the BL32 image base, size and max limit in
			 * the entry point information, since there is no
			 * platform function to retrieve them in generic
			 * code. We choose arg2, arg3 and arg4 since the generic
			 * code uses arg1 for stashing the SP manifest size. The
			 * SPMC setup uses these arguments to update SP manifest
			 * with actual SP's base address and it size.
			 */
			bl32_image_ep_info.args.arg2 =
				bl_params->image_info->image_base;
			bl32_image_ep_info.args.arg3 =
				bl_params->image_info->image_size;
			bl32_image_ep_info.args.arg4 =
				bl_params->image_info->image_base +
				bl_params->image_info->image_max_size;
#endif
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

#if USE_KERNEL_DT_CONVENTION
	/*
	 * Only use the default DT base address if TF-A has not supplied one.
	 * This can occur when the DT is side-loaded and its memory location
	 * is unknown (e.g., RESET_TO_BL31).
	 */

	if (bl33_image_ep_info.args.arg0 == 0U) {
		bl33_image_ep_info.args.arg0 = HW_CONFIG_BASE;
	}

#if ARM_LINUX_KERNEL_AS_BL33
	bl33_image_ep_info.args.arg1 = 0U;
	bl33_image_ep_info.args.arg2 = 0U;
	bl33_image_ep_info.args.arg3 = 0U;
#endif
#endif
#endif /* TRANSFER_LIST */
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3)
{
	/* Initialize the console to provide early debug support */
	arm_console_boot_init();

	arm_bl31_early_platform_setup(arg0, arg1, arg2, arg3);

#if !HW_ASSISTED_COHERENCY
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
#endif
}

/*******************************************************************************
 * Perform any BL31 platform setup common to ARM standard platforms
 ******************************************************************************/
void arm_bl31_platform_setup(void)
{
	struct transfer_list_entry *te __unused;

#if TRANSFER_LIST && !RESET_TO_BL31
	ns_tl = transfer_list_init((void *)FW_NS_HANDOFF_BASE,
				   PLAT_ARM_FW_HANDOFF_SIZE);
	if (ns_tl == NULL) {
		ERROR("Non-secure transfer list initialisation failed!\n");
		panic();
	}
	/* BL31 may modify the HW_CONFIG so defer copying it until later. */
	te = transfer_list_find(secure_tl, TL_TAG_FDT);
	assert(te != NULL);

	/*
	 * A pre-existing assumption is that FCONF is unsupported w/ RESET_TO_BL2 and
	 * RESET_TO_BL31. In the case of RESET_TO_BL31 this makes sense because there
	 * isn't a prior stage to load the device tree, but the reasoning for RESET_TO_BL2 is
	 * less clear. For the moment hardware properties that would normally be
	 * derived from the DT are statically defined.
	 */
#if !RESET_TO_BL2
	fconf_populate("HW_CONFIG", (uintptr_t)transfer_list_entry_data(te));
#endif

	te = transfer_list_add(ns_tl, TL_TAG_FDT, te->data_size,
			       transfer_list_entry_data(te));
	assert(te != NULL);

	te = transfer_list_find(secure_tl, TL_TAG_TPM_EVLOG);
	if (te != NULL) {
		te = transfer_list_add(ns_tl, TL_TAG_TPM_EVLOG, te->data_size,
				  transfer_list_entry_data(te));
		if (te == NULL) {
			ERROR("Failed to load event log in Non-Secure transfer list\n");
			panic();
		}
	}
#endif /* TRANSFER_LIST && !RESET_TO_BL31 */

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

#if ENABLE_FEAT_RAS && FFH_SUPPORT
	ras_init();
#endif

#if USE_DEBUGFS
	debugfs_init();
#endif /* USE_DEBUGFS */
}

/*******************************************************************************
 * Perform any BL31 platform runtime setup prior to BL31 exit common to ARM
 * standard platforms
 ******************************************************************************/
void arm_bl31_plat_runtime_setup(void)
{
	struct transfer_list_entry *te __unused;
	/* Initialize the runtime console */
	arm_console_runtime_init();

#if TRANSFER_LIST && !RESET_TO_BL31
	/*
	 * We assume BL31 has added all TE's required by BL33 at this stage, ensure
	 * that data is visible to all observers by performing a flush operation, so
	 * they can access the updated data even if caching is not enabled.
	 */
	flush_dcache_range((uintptr_t)ns_tl, ns_tl->size);
#endif /* TRANSFER_LIST && !RESET_TO_BL31 */

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

#if USE_GIC_DRIVER == 3
	gic_set_gicr_frames(arm_gicr_base_addrs);
#endif
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
#if RESET_TO_BL31
	/*  initialize GPT only when RME is enabled. */
	assert(is_feat_rme_present());

	/* Initialise and enable granule protection after MMU. */
	arm_gpt_setup();
#endif /* RESET_TO_BL31 */
	/*
	 * Initialise Granule Protection library and enable GPC for the primary
	 * processor. The tables have already been initialized by a previous BL
	 * stage, so there is no need to provide any PAS here. This function
	 * sets up pointers to those tables.
	 */
	if (gpt_runtime_init(BITLOCK_BASE, BITLOCK_SIZE) < 0) {
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
