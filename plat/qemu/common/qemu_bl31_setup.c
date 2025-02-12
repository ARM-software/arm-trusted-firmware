/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>
#include <drivers/arm/pl061_gpio.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/transfer_list.h>
#include <plat/common/platform.h>
#if ENABLE_RME
#ifdef PLAT_qemu
#include <qemu_pas_def.h>
#elif PLAT_qemu_sbsa
#include <qemu_sbsa_pas_def.h>
#endif /* PLAT_qemu */
#endif /* ENABLE_RME */
#ifdef PLAT_qemu_sbsa
#include <sbsa_platform.h>
#endif

#include "qemu_private.h"

#define MAP_BL31_TOTAL		MAP_REGION_FLAT(			\
					BL31_BASE,			\
					BL31_END - BL31_BASE,		\
					MT_MEMORY | MT_RW | EL3_PAS)
#define MAP_BL31_RO		MAP_REGION_FLAT(			\
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

#if ENABLE_RME
#if (RME_GPT_BITLOCK_BLOCK == 0)
#define BITLOCK_BASE	UL(0)
#define BITLOCK_SIZE	UL(0)
#else

/*
 * Number of bitlock_t entries in the gpt_bitlock array for this platform's
 * Protected Physical Size. One 8-bit bitlock_t entry covers
 * 8 * RME_GPT_BITLOCK_BLOCK * 512MB.
 */
#if (PLAT_QEMU_PPS > (RME_GPT_BITLOCK_BLOCK * SZ_512M * UL(8)))
#define BITLOCKS_NUM	(PLAT_QEMU_PPS /	\
			(RME_GPT_BITLOCK_BLOCK * SZ_512M * UL(8)))
#else
#define BITLOCKS_NUM	1
#endif

static bitlock_t gpt_bitlock[BITLOCKS_NUM];
#define BITLOCK_BASE	(uintptr_t)gpt_bitlock
#define BITLOCK_SIZE	sizeof(gpt_bitlock)
#endif /* RME_GPT_BITLOCK_BLOCK */
#endif /* ENABLE_RME */

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL3-1 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;
#if ENABLE_RME
static entry_point_info_t rmm_image_ep_info;
#endif
static struct transfer_list_header *bl31_tl;

/*******************************************************************************
 * Perform any BL3-1 early platform setup.  Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & EL3 in BL1) before
 * they are lost (potentially). This needs to be done before the MMU is
 * initialized so that the memory layout can be used while creating page
 * tables. BL2 has flushed this information to memory, so we are guaranteed
 * to pick up good data.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	bool is64 = false;
	uint64_t hval;

	/* Initialize the console to provide early debug support */
	qemu_console_init();

/* Platform names have to be lowercase. */
#ifdef PLAT_qemu_sbsa
	sbsa_platform_init();
#endif

	/*
	 * Check params passed from BL2
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)arg0;

	assert(params_from_bl2);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33, BL32 and RMM (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
#ifdef __aarch64__
		if (bl_params->image_id == BL31_IMAGE_ID &&
		    GET_RW(bl_params->ep_info->spsr) == MODE_RW_64)
			is64 = true;
#endif
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

#if ENABLE_RME
		if (bl_params->image_id == RMM_IMAGE_ID)
			rmm_image_ep_info = *bl_params->ep_info;
#endif

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (!bl33_image_ep_info.pc)
		panic();
#if ENABLE_RME
	if (!rmm_image_ep_info.pc)
		panic();
#endif

	if (!TRANSFER_LIST ||
	    !transfer_list_check_header((void *)arg3))
		return;

	if (is64)
		hval = TRANSFER_LIST_HANDOFF_X1_VALUE(REGISTER_CONVENTION_VERSION);
	else
		hval = TRANSFER_LIST_HANDOFF_R1_VALUE(REGISTER_CONVENTION_VERSION);

	if (arg1 != hval)
		return;

	bl31_tl = (void *)arg3; /* saved TL address from BL2 */
}

#if ENABLE_RME
#if PLAT_qemu
/*
 * The GPT library might modify the gpt regions structure to optimize
 * the layout, so the array cannot be constant.
 */
static pas_region_t pas_regions[] = {
	QEMU_PAS_ROOT,
	QEMU_PAS_SECURE,
	QEMU_PAS_GPTS,
	QEMU_PAS_NS0,
	QEMU_PAS_REALM,
	QEMU_PAS_NS1,
};

static inline void bl31_adjust_pas_regions(void) {}
#elif PLAT_qemu_sbsa
/*
 * The GPT library might modify the gpt regions structure to optimize
 * the layout, so the array cannot be constant.
 */
static pas_region_t pas_regions[] = {
	QEMU_PAS_ROOT,
	QEMU_PAS_SECURE,
	QEMU_PAS_GPTS,
	QEMU_PAS_REALM,
	QEMU_PAS_NS0,
};

static void bl31_adjust_pas_regions(void)
{
	uint64_t base_addr = 0, total_size = 0;
	struct platform_memory_data data;
	uint32_t node;

	/*
	 * The amount of memory supported by the SBSA platform is dynamic
	 * and dependent on user input.  Since the configuration of the GPT
	 * needs to reflect the system memory, QEMU_PAS_NS0 needs to be set
	 * based on the information found in the device tree.
	 */

	for (node = 0; node < sbsa_platform_num_memnodes(); node++) {
		data = sbsa_platform_memory_node(node);

		if (data.nodeid == 0) {
			base_addr = data.addr_base;
		}

		total_size += data.addr_size;
	}

	 /* Index '4' correspond to QEMU_PAS_NS0, see pas_regions[] above */
	pas_regions[4].base_pa = base_addr;
	pas_regions[4].size = total_size;
}
#endif /* PLAT_qemu */

static void bl31_plat_gpt_setup(void)
{
	/*
	 * Initialize entire protected space to GPT_GPI_ANY. With each L0 entry
	 * covering 1GB (currently the only supported option), then covering
	 * 256TB of RAM (48-bit PA) would require a 2MB L0 region. At the
	 * moment we use a 8KB table, which covers 1TB of RAM (40-bit PA).
	 */
	if (gpt_init_l0_tables(PLAT_QEMU_GPCCR_PPS, PLAT_QEMU_L0_GPT_BASE,
			       PLAT_QEMU_L0_GPT_SIZE) < 0) {
		ERROR("gpt_init_l0_tables() failed!\n");
		panic();
	}

	bl31_adjust_pas_regions();

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

void bl31_plat_arch_setup(void)
{
	const mmap_region_t bl_regions[] = {
		MAP_BL31_TOTAL,
		MAP_BL31_RO,
#if USE_COHERENT_MEM
		MAP_BL_COHERENT_RAM,
#endif
#if ENABLE_RME
		MAP_GPT_L0_REGION,
		MAP_GPT_L1_REGION,
		MAP_RMM_SHARED_MEM,
#endif
		{0}
	};

	setup_page_tables(bl_regions, plat_qemu_get_mmap());

	enable_mmu_el3(0);

#if ENABLE_RME
	/* Initialise and enable granule protection after MMU. */
	bl31_plat_gpt_setup();

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

}

static void qemu_gpio_init(void)
{
#ifdef SECURE_GPIO_BASE
	pl061_gpio_init();
	pl061_gpio_register(SECURE_GPIO_BASE, 0);
#endif
}

void bl31_platform_setup(void)
{
	plat_qemu_gic_init();
	qemu_gpio_init();
}

unsigned int plat_get_syscnt_freq2(void)
{
	return read_cntfrq_el0();
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image
 * for the security state specified. BL3-3 corresponds to the non-secure
 * image type while BL3-2 corresponds to the secure image type. A NULL
 * pointer is returned if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
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
		next_image_info =  &bl32_image_ep_info;
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

void bl31_plat_runtime_setup(void)
{
#if TRANSFER_LIST
	if (bl31_tl) {
		/*
		 * Relocate the TL from S to NS memory before EL3 exit
		 * to reflect all changes in TL done by BL32
		 */
		if (!transfer_list_relocate(bl31_tl, (void *)FW_NS_HANDOFF_BASE,
					    bl31_tl->max_size))
			ERROR("Relocate TL to NS memory failed\n");
	}
#endif

	console_flush();
	console_switch_state(CONSOLE_FLAG_RUNTIME);
}
