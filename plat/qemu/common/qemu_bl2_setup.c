/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <libfdt.h>

#include <platform_def.h>

#include <arch_features.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <lib/optee_utils.h>
#include <lib/transfer_list.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#if ENABLE_RME
#include <qemu_pas_def.h>
#endif

#include "qemu_private.h"

#define MAP_BL2_TOTAL		MAP_REGION_FLAT(			\
					bl2_tzram_layout.total_base,	\
					bl2_tzram_layout.total_size,	\
					MT_MEMORY | MT_RW | EL3_PAS)

#define MAP_BL2_RO		MAP_REGION_FLAT(			\
					BL_CODE_BASE,			\
					BL_CODE_END - BL_CODE_BASE,	\
					MT_CODE | EL3_PAS),		\
				MAP_REGION_FLAT(			\
					BL_RO_DATA_BASE,		\
					BL_RO_DATA_END			\
						- BL_RO_DATA_BASE,	\
					MT_RO_DATA | EL3_PAS)

#if USE_COHERENT_MEM
#define MAP_BL_COHERENT_RAM	MAP_REGION_FLAT(			\
					BL_COHERENT_RAM_BASE,		\
					BL_COHERENT_RAM_END		\
						- BL_COHERENT_RAM_BASE,	\
					MT_DEVICE | MT_RW | EL3_PAS)
#endif

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);
static struct transfer_list_header *bl2_tl;

void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			       u_register_t arg2, u_register_t arg3)
{
	meminfo_t *mem_layout = (void *)arg1;

	/* Initialize the console to provide early debug support */
	qemu_console_init();

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;

	plat_qemu_io_setup();
}

static void security_setup(void)
{
	/*
	 * This is where a TrustZone address space controller and other
	 * security related peripherals, would be configured.
	 */
}

static void update_dt(void)
{
#if TRANSFER_LIST
	struct transfer_list_entry *te;
#endif
	int ret;
	void *fdt = (void *)(uintptr_t)ARM_PRELOADED_DTB_BASE;

	ret = fdt_open_into(fdt, fdt, PLAT_QEMU_DT_MAX_SIZE);
	if (ret < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", fdt, ret);
		return;
	}

	if (dt_add_psci_node(fdt)) {
		ERROR("Failed to add PSCI Device Tree node\n");
		return;
	}

	if (dt_add_psci_cpu_enable_methods(fdt)) {
		ERROR("Failed to add PSCI cpu enable methods in Device Tree\n");
		return;
	}

#if ENABLE_RME
	if (fdt_add_reserved_memory(fdt, "rmm", REALM_DRAM_BASE,
				    REALM_DRAM_SIZE)) {
		ERROR("Failed to reserve RMM memory in Device Tree\n");
		return;
	}

	INFO("Reserved RMM memory [0x%lx, 0x%lx] in Device tree\n",
	     (uintptr_t)REALM_DRAM_BASE,
	     (uintptr_t)REALM_DRAM_BASE + REALM_DRAM_SIZE - 1);
#endif

	ret = fdt_pack(fdt);
	if (ret < 0)
		ERROR("Failed to pack Device Tree at %p: error %d\n", fdt, ret);

#if TRANSFER_LIST
	/* create a TE */
	te = transfer_list_add(bl2_tl, TL_TAG_FDT, fdt_totalsize(fdt), fdt);
	if (!te) {
		ERROR("Failed to add FDT entry to Transfer List\n");
		return;
	}
#endif
}

void bl2_platform_setup(void)
{
#if TRANSFER_LIST
	bl2_tl = transfer_list_init((void *)(uintptr_t)FW_HANDOFF_BASE,
				    FW_HANDOFF_SIZE);
	if (!bl2_tl) {
		ERROR("Failed to initialize Transfer List at 0x%lx\n",
		      (unsigned long)FW_HANDOFF_BASE);
	}
#endif
	security_setup();
	update_dt();

	/* TODO Initialize timer */
}

void qemu_bl2_sync_transfer_list(void)
{
#if TRANSFER_LIST
	transfer_list_update_checksum(bl2_tl);
#endif
}

#if ENABLE_RME
static void bl2_plat_gpt_setup(void)
{
	/*
	 * The GPT library might modify the gpt regions structure to optimize
	 * the layout, so the array cannot be constant.
	 */
	pas_region_t pas_regions[] = {
		QEMU_PAS_ROOT,
		QEMU_PAS_SECURE,
		QEMU_PAS_GPTS,
		QEMU_PAS_NS0,
		QEMU_PAS_REALM,
		QEMU_PAS_NS1,
	};

	/*
	 * Initialize entire protected space to GPT_GPI_ANY. With each L0 entry
	 * covering 1GB (currently the only supported option), then covering
	 * 256TB of RAM (48-bit PA) would require a 2MB L0 region. At the
	 * moment we use a 8KB table, which covers 1TB of RAM (40-bit PA).
	 */
	if (gpt_init_l0_tables(GPCCR_PPS_1TB, PLAT_QEMU_L0_GPT_BASE,
			       PLAT_QEMU_L0_GPT_SIZE +
			       PLAT_QEMU_GPT_BITLOCK_SIZE) < 0) {
		ERROR("gpt_init_l0_tables() failed!\n");
		panic();
	}

	/* Carve out defined PAS ranges. */
	if (gpt_init_pas_l1_tables(GPCCR_PGS_4K,
				   PLAT_QEMU_L1_GPT_BASE,
				   PLAT_QEMU_L1_GPT_SIZE,
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
#endif

void bl2_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL2_TOTAL,
		MAP_BL2_RO,
#if USE_COHERENT_MEM
		MAP_BL_COHERENT_RAM,
#endif
#if ENABLE_RME
		MAP_RMM_DRAM,
		MAP_GPT_L0_REGION,
		MAP_GPT_L1_REGION,
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_qemu_get_mmap());

#if ENABLE_RME
	/* BL2 runs in EL3 when RME enabled. */
	assert(is_feat_rme_present());
	enable_mmu_el3(0);

	/* Initialise and enable granule protection after MMU. */
	bl2_plat_gpt_setup();
#else /* ENABLE_RME */

#ifdef __aarch64__
	enable_mmu_el1(0);
#else
	enable_mmu_svc_mon(0);
#endif
#endif /* ENABLE_RME */
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
static uint32_t qemu_get_spsr_for_bl32_entry(void)
{
#ifdef __aarch64__
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL3-2 image.
	 */
	return 0;
#else
	return SPSR_MODE32(MODE32_svc, SPSR_T_ARM, SPSR_E_LITTLE,
			   DISABLE_ALL_EXCEPTIONS);
#endif
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
static uint32_t qemu_get_spsr_for_bl33_entry(void)
{
	uint32_t spsr;
#ifdef __aarch64__
	unsigned int mode;

	/* Figure out what mode we enter the non-secure world in */
	mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
#else
	spsr = SPSR_MODE32(MODE32_svc,
		    plat_get_ns_image_entrypoint() & 0x1,
		    SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
#endif
	return spsr;
}

#if defined(SPD_spmd) && SPMD_SPM_AT_SEL2
static int load_sps_from_tb_fw_config(struct image_info *image_info)
{
	void *dtb = (void *)image_info->image_base;
	const char *compat_str = "arm,sp";
	const struct fdt_property *uuid;
	uint32_t load_addr;
	const char *name;
	int sp_node;
	int node;

	node = fdt_node_offset_by_compatible(dtb, -1, compat_str);
	if (node < 0) {
		ERROR("Can't find %s in TB_FW_CONFIG", compat_str);
		return -1;
	}

	fdt_for_each_subnode(sp_node, dtb, node) {
		name = fdt_get_name(dtb, sp_node, NULL);
		if (name == NULL) {
			ERROR("Can't get name of node in dtb\n");
			return -1;
		}
		uuid = fdt_get_property(dtb, sp_node, "uuid", NULL);
		if (uuid == NULL) {
			ERROR("Can't find property uuid in node %s", name);
			return -1;
		}
		if (fdt_read_uint32(dtb, sp_node, "load-address",
				    &load_addr) < 0) {
			ERROR("Can't read load-address in node %s", name);
			return -1;
		}
		if (qemu_io_register_sp_pkg(name, uuid->data, load_addr) < 0) {
			return -1;
		}
	}

	return 0;
}
#endif /*defined(SPD_spmd) && SPMD_SPM_AT_SEL2*/

#if defined(SPD_opteed) || defined(AARCH32_SP_OPTEE) || defined(SPMC_OPTEE)
static int handoff_pageable_part(uint64_t pagable_part)
{
#if TRANSFER_LIST
	struct transfer_list_entry *te;

	te = transfer_list_add(bl2_tl, TL_TAG_OPTEE_PAGABLE_PART,
			       sizeof(pagable_part), &pagable_part);
	if (!te) {
		INFO("Cannot add TE for pageable part\n");
		return -1;
	}
#endif
	return 0;
}
#endif

static int qemu_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
#if defined(SPD_opteed) || defined(AARCH32_SP_OPTEE) || defined(SPMC_OPTEE)
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif
#if defined(SPD_spmd)
	bl_mem_params_node_t *bl32_mem_params = NULL;
#endif
#if TRANSFER_LIST
	struct transfer_list_header *ns_tl = NULL;
#endif

	assert(bl_mem_params);

	switch (image_id) {
#if TRANSFER_LIST
	case BL31_IMAGE_ID:
		/*
		 * arg0 is a bl_params_t reserved for bl31_early_platform_setup2
		 * we just need arg1 and arg3 for BL31 to update the TL from S
		 * to NS memory before it exits
		 */
#ifdef __aarch64__
		if (GET_RW(bl_mem_params->ep_info.spsr) == MODE_RW_64) {
			bl_mem_params->ep_info.args.arg1 =
				TRANSFER_LIST_HANDOFF_X1_VALUE(REGISTER_CONVENTION_VERSION);
		} else
#endif
		{
			bl_mem_params->ep_info.args.arg1 =
				TRANSFER_LIST_HANDOFF_R1_VALUE(REGISTER_CONVENTION_VERSION);
		}

		bl_mem_params->ep_info.args.arg3 = (uintptr_t)bl2_tl;
		break;
#endif
	case BL32_IMAGE_ID:
#if defined(SPD_opteed) || defined(AARCH32_SP_OPTEE) || defined(SPMC_OPTEE)
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

		/* add TL_TAG_OPTEE_PAGABLE_PART entry to the TL */
		if (handoff_pageable_part(bl_mem_params->ep_info.args.arg1)) {
			return -1;
		}
#endif

		INFO("Handoff to BL32\n");
		bl_mem_params->ep_info.spsr = qemu_get_spsr_for_bl32_entry();
		if (TRANSFER_LIST &&
			transfer_list_set_handoff_args(bl2_tl,
				&bl_mem_params->ep_info))
			break;

		INFO("Using default arguments\n");
#if defined(SPMC_OPTEE)
		/*
		 * Explicit zeroes to unused registers since they may have
		 * been populated by parse_optee_header() above.
		 *
		 * OP-TEE expects system DTB in x2 and TOS_FW_CONFIG in x0,
		 * the latter is filled in below for TOS_FW_CONFIG_ID and
		 * applies to any other SPMC too.
		 */
		bl_mem_params->ep_info.args.arg2 = ARM_PRELOADED_DTB_BASE;
#elif defined(SPD_opteed)
		/*
		 * OP-TEE expect to receive DTB address in x2.
		 * This will be copied into x2 by dispatcher.
		 */
		bl_mem_params->ep_info.args.arg3 = ARM_PRELOADED_DTB_BASE;
#elif defined(AARCH32_SP_OPTEE)
		bl_mem_params->ep_info.args.arg0 =
					bl_mem_params->ep_info.args.arg1;
		bl_mem_params->ep_info.args.arg1 = 0;
		bl_mem_params->ep_info.args.arg2 = ARM_PRELOADED_DTB_BASE;
		bl_mem_params->ep_info.args.arg3 = 0;
#endif
		break;

	case BL33_IMAGE_ID:
#ifdef AARCH32_SP_OPTEE
		/* AArch32 only core: OP-TEE expects NSec EP in register LR */
		pager_mem_params = get_bl_mem_params_node(BL32_IMAGE_ID);
		assert(pager_mem_params);
		pager_mem_params->ep_info.lr_svc = bl_mem_params->ep_info.pc;
#endif

		bl_mem_params->ep_info.spsr = qemu_get_spsr_for_bl33_entry();

#if ARM_LINUX_KERNEL_AS_BL33
		/*
		 * According to the file ``Documentation/arm64/booting.txt`` of
		 * the Linux kernel tree, Linux expects the physical address of
		 * the device tree blob (DTB) in x0, while x1-x3 are reserved
		 * for future use and must be 0.
		 */
		bl_mem_params->ep_info.args.arg0 =
			(u_register_t)ARM_PRELOADED_DTB_BASE;
		bl_mem_params->ep_info.args.arg1 = 0U;
		bl_mem_params->ep_info.args.arg2 = 0U;
		bl_mem_params->ep_info.args.arg3 = 0U;
#elif TRANSFER_LIST
		if (bl2_tl) {
			/* relocate the tl to pre-allocate NS memory */
			ns_tl = transfer_list_relocate(bl2_tl,
					(void *)(uintptr_t)FW_NS_HANDOFF_BASE,
					bl2_tl->max_size);
			if (!ns_tl) {
				ERROR("Relocate TL to 0x%lx failed\n",
					(unsigned long)FW_NS_HANDOFF_BASE);
				return -1;
			}
		}

		INFO("Handoff to BL33\n");
		if (!transfer_list_set_handoff_args(ns_tl,
						    &bl_mem_params->ep_info)) {
			INFO("Invalid TL, fallback to default arguments\n");
			bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		}
#else
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
#endif /* ARM_LINUX_KERNEL_AS_BL33 */

		break;
#ifdef SPD_spmd
#if SPMD_SPM_AT_SEL2
	case TB_FW_CONFIG_ID:
		err = load_sps_from_tb_fw_config(&bl_mem_params->image_info);
		break;
#endif
	case TOS_FW_CONFIG_ID:
		/* An SPMC expects TOS_FW_CONFIG in x0/r0 */
		bl32_mem_params = get_bl_mem_params_node(BL32_IMAGE_ID);
		bl32_mem_params->ep_info.args.arg0 =
					bl_mem_params->image_info.image_base;
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
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return qemu_bl2_handle_post_image_load(image_id);
}

uintptr_t plat_get_ns_image_entrypoint(void)
{
	return NS_IMAGE_OFFSET;
}
