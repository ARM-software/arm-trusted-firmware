/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <console.h>
#include <mmio.h>
#include <platform.h>
#include <interrupt_mgmt.h>
#include <stddef.h>
#include <xlat_tables.h>
#include <thunder_private.h>
#include <thunder_common.h>
#include <thunder_dt.h>
#include <libfdt.h>
#include <plat_params.h>
#include <errno.h>

#include <debug.h>

#ifndef DEBUG
#define DEBUG 0
#endif

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)
#define BL31_END (unsigned long)(&__BL31_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)


static entry_point_info_t bl33_image_ep_info, bl32_image_ep_info;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type));

	if (type == NON_SECURE)
		return &bl33_image_ep_info;
	if (type == SECURE)
		return &bl32_image_ep_info;

	return NULL;
}

/*******************************************************************************
 * Perform any BL31 specific platform actions. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables. On the FVP
 * we know that BL2 has populated the parameters in secure DRAM. So we just use
 * the reference passed in 'from_bl2' instead of copying. The 'data' parameter
 * is not used since all the information is contained in 'from_bl2'. Also, BL2
 * has flushed this information to memory, so we are guaranteed to pick up good
 * data
 ******************************************************************************/
#if LOAD_IMAGE_V2
void bl31_early_platform_setup(void *from_bl2,
				void *plat_params_from_bl2)
#else
void bl31_early_platform_setup(bl31_params_t *from_bl2,
			        void *plat_params_from_bl2)
#endif
{
	cavium_console_init();

#if LOAD_IMAGE_V2
	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;
	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	fdt_ptr = plat_params_from_bl2;
	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 and BL32 (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL32_IMAGE_ID)
			bl32_image_ep_info = *bl_params->ep_info;

		if (bl_params->image_id == BL33_IMAGE_ID)
			bl33_image_ep_info = *bl_params->ep_info;

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0)
		panic();

#else /* LOAD_IMAGE_V2 */

	/* Check params passed from BL2 should not be NULL,
	 * We are not checking plat_params_from_bl2 as NULL as we are not
	 * using it on FVP
	 */
	assert(from_bl2 != NULL);
	assert(from_bl2->h.type == PARAM_BL31);
	assert(from_bl2->h.version >= VERSION_1);

	params_early_setup(plat_params_from_bl2);

	/*
	 * Copy BL3-3, BL3-2 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	if (from_bl2->bl32_ep_info)
		bl32_image_ep_info = *from_bl2->bl32_ep_info;
	bl33_image_ep_info = *from_bl2->bl33_ep_info;
#endif /* LOAD_IMAGE_V2 */

}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	bl31_early_platform_setup((void *)arg0, (void *)arg1);
}

/*
 * While booting from MMC device, it was necessary to configure SMMU as to grant
 * access for eMMC controller to secure memory, where images were loaded. On
 * the other hand in order to keep MMC functional in non-secure world, we must
 * come back to initial settings. It is safe to do it here, because every
 * firmware image is loaded at this time.
 */
static void thunder_configure_mmc_security(void)
{
	int boot_medium;
	uint64_t val;
	uint64_t ssd_idx = CAVM_PCC_DEV_CON_E_MIO_EMM >> 5;
	uint64_t emm_ssd_mask = (1ULL << (CAVM_PCC_DEV_CON_E_MIO_EMM & 0x1F));
	cavm_gpio_strap_t gpio_strap;

	gpio_strap.u = CSR_READ_PA(0, CAVM_GPIO_STRAP);
	boot_medium = (gpio_strap.u) & 0x7;

	/* If it isn't MMC boot, then nothing to do here */
	if (boot_medium != 0x02 && boot_medium != 0x03)
		return;

	/*
	 * Configure SMMU and mark MMC controller in NODE0
	 * as acting for non-secure domain.
	 */
	val = CSR_READ_PA(0, CAVM_SMMUX_SSDRX(0, ssd_idx));
	val |= emm_ssd_mask;
	CSR_WRITE_PA(0, CAVM_SMMUX_SSDRX(0, ssd_idx), val);
}


typedef struct intr_desc {
	uint32_t id;
	interrupt_handler_t handler;
} intr_desc_t;

static intr_desc_t intr_descs[MAX_INTRS];

static uint64_t
thunder_handle_irq_el3(uint32_t id, uint32_t flags, void *handle, void *cookie)
{
	uint32_t idx;
	uint64_t rc = 0;

	if (id == INTR_ID_UNAVAILABLE)
		id = plat_ic_acknowledge_interrupt();

	for (idx = 0; idx < MAX_INTRS; idx++) {
		if (intr_descs[idx].id == id) {
			rc = intr_descs[idx].handler(id, flags, cookie);
			break;
		}
	}

	if (idx == MAX_INTRS)
		printf("Handler not found!\n");

	plat_ic_end_of_interrupt(id);

	return rc;
}

static void thunder_el3_irq_init(void)
{
	uint32_t flags;
	int32_t rc;

	flags = 0;
	set_interrupt_rm_flag(flags, SECURE);
	set_interrupt_rm_flag(flags, NON_SECURE);

	rc = register_interrupt_type_handler(INTR_TYPE_EL3,
					     thunder_handle_irq_el3,
					     flags);
	if (rc)
		ERROR("error %d on registering secure handler\n", rc);
}

int32_t thunder_register_el3_interrupt_handler(uint32_t id,
					       interrupt_handler_t handler)
{
	uint32_t idx;

	/* Validate the 'handler' parameter */
	if (!handler)
		return -EINVAL;

	/* Setup our IRQ handler */
	thunder_el3_irq_init();

	/* Check if a handler for this id is already been registered */
	for (idx = 0; idx < MAX_INTRS; idx++) {
		if (intr_descs[idx].id == id)
			return -EALREADY;

		if (!intr_descs[idx].handler) {
			/* Save the handler */
			intr_descs[idx].handler = handler;
			intr_descs[idx].id  = id;

			return 0;
		}
	}

	return -E2BIG;
}

/*******************************************************************************
 * Initialize the gic, configure the CLCD and zero out variables needed by the
 * secondaries to boot up correctly.
 ******************************************************************************/
void bl31_platform_setup()
{
	thunder_cpu_setup();
	thunder_gti_init();
	thunder_timers_init();

	thunder_configure_mmc_security();
	thunder_fill_board_details(1);
	thunder_gic_driver_init();
	thunder_gic_init();

	/* Intialize the power controller */
	thunder_pwrc_setup();

	/* Active secure/insecure split */
	thunder_security_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup()
{
	uintptr_t ns_dma_memory_base = thunder_dram_size_node(0) - NS_DMA_MEMORY_SIZE;
	mmap_add_region(BL31_RO_BASE, BL31_RO_BASE,
			BL31_END - BL31_RO_BASE,
			MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(BL31_RO_BASE, BL31_RO_BASE,
			BL31_RO_LIMIT - BL31_RO_BASE,
			MT_MEMORY | MT_RO | MT_SECURE);

#if USE_COHERENT_MEM
	mmap_add_region(BL31_COHERENT_RAM_BASE, BL31_COHERENT_RAM_BASE,
			BL31_COHERENT_RAM_LIMIT - BL31_COHERENT_RAM_BASE,
			MT_MEMORY | MT_RW | MT_SECURE);
#endif

	mmap_add_region(ns_dma_memory_base, ns_dma_memory_base,
			NS_DMA_MEMORY_SIZE,
			MT_DEVICE | MT_RW | MT_SECURE);

	plat_add_mmio_map();

	init_xlat_tables();

	enable_mmu_el3(0);
}

void bl31_plat_runtime_setup(void)
{
	cavium_console_unregister();
}
