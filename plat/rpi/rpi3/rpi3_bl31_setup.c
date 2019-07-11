/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <libfdt.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <lib/xlat_tables/xlat_tables_defs.h>
#include <plat/common/platform.h>

#include <rpi_shared.h>

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type) != 0);

	next_image_info = (type == NON_SECURE)
			? &bl33_image_ep_info : &bl32_image_ep_info;

	/* None of the images can have 0x0 as the entrypoint. */
	if (next_image_info->pc) {
		return next_image_info;
	} else {
		return NULL;
	}
}

/*******************************************************************************
 * Return entrypoint of BL33.
 ******************************************************************************/
uintptr_t plat_get_ns_image_entrypoint(void)
{
#ifdef PRELOADED_BL33_BASE
	return PRELOADED_BL33_BASE;
#else
	return PLAT_RPI3_NS_IMAGE_OFFSET;
#endif
}

/*******************************************************************************
 * Perform any BL31 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & EL3 in BL1) before
 * they are lost (potentially). This needs to be done before the MMU is
 * initialized so that the memory layout can be used while creating page
 * tables. BL2 has flushed this information to memory, so we are guaranteed
 * to pick up good data.
 ******************************************************************************/
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)

{
	/* Initialize the console to provide early debug support */
	rpi3_console_init(PLAT_RPI3_UART_CLK_IN_HZ);

	/*
	 * In debug builds, a special value is passed in 'arg1' to verify
	 * platform parameters from BL2 to BL31. Not used in release builds.
	 */
	assert(arg1 == RPI3_BL31_PLAT_PARAM_VAL);

	/* Check that params passed from BL2 are not NULL. */
	bl_params_t *params_from_bl2 = (bl_params_t *) arg0;

	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 and BL32 (if present), entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL32_IMAGE_ID) {
			bl32_image_ep_info = *bl_params->ep_info;
		}

		if (bl_params->image_id == BL33_IMAGE_ID) {
			bl33_image_ep_info = *bl_params->ep_info;
		}

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0) {
		panic();
	}

#if RPI3_DIRECT_LINUX_BOOT
# if RPI3_BL33_IN_AARCH32
	/*
	 * According to the file ``Documentation/arm/Booting`` of the Linux
	 * kernel tree, Linux expects:
	 * r0 = 0
	 * r1 = machine type number, optional in DT-only platforms (~0 if so)
	 * r2 = Physical address of the device tree blob
	 */
	VERBOSE("rpi3: Preparing to boot 32-bit Linux kernel\n");
	bl33_image_ep_info.args.arg0 = 0U;
	bl33_image_ep_info.args.arg1 = ~0U;
	bl33_image_ep_info.args.arg2 = (u_register_t) RPI3_PRELOADED_DTB_BASE;
# else
	/*
	 * According to the file ``Documentation/arm64/booting.txt`` of the
	 * Linux kernel tree, Linux expects the physical address of the device
	 * tree blob (DTB) in x0, while x1-x3 are reserved for future use and
	 * must be 0.
	 */
	VERBOSE("rpi3: Preparing to boot 64-bit Linux kernel\n");
	bl33_image_ep_info.args.arg0 = (u_register_t) RPI3_PRELOADED_DTB_BASE;
	bl33_image_ep_info.args.arg1 = 0ULL;
	bl33_image_ep_info.args.arg2 = 0ULL;
	bl33_image_ep_info.args.arg3 = 0ULL;
# endif /* RPI3_BL33_IN_AARCH32 */
#endif /* RPI3_DIRECT_LINUX_BOOT */
}

void bl31_plat_arch_setup(void)
{
	rpi3_setup_page_tables(BL31_BASE, BL31_END - BL31_BASE,
			       BL_CODE_BASE, BL_CODE_END,
			       BL_RO_DATA_BASE, BL_RO_DATA_END
#if USE_COHERENT_MEM
			       , BL_COHERENT_RAM_BASE, BL_COHERENT_RAM_END
#endif
			      );

	enable_mmu_el3(0);
}

#ifdef RPI3_PRELOADED_DTB_BASE
/*
 * Add information to the device tree (if any) about the reserved DRAM used by
 * the Trusted Firmware.
 */
static void rpi3_dtb_add_mem_rsv(void)
{
	int i, regions, rc;
	uint64_t addr, size;
	void *dtb = (void *)RPI3_PRELOADED_DTB_BASE;

	INFO("rpi3: Checking DTB...\n");

	/* Return if no device tree is detected */
	if (fdt_check_header(dtb) != 0)
		return;

	regions = fdt_num_mem_rsv(dtb);

	VERBOSE("rpi3: Found %d mem reserve region(s)\n", regions);

	/* We expect to find one reserved region that we can modify */
	if (regions < 1)
		return;

	/*
	 * Look for the region that corresponds to the default boot firmware. It
	 * starts at address 0, and it is not needed when the default firmware
	 * is replaced by this port of the Trusted Firmware.
	 */
	for (i = 0; i < regions; i++) {
		if (fdt_get_mem_rsv(dtb, i, &addr, &size) != 0)
			continue;

		if (addr != 0x0)
			continue;

		VERBOSE("rpi3: Firmware mem reserve region found\n");

		rc = fdt_del_mem_rsv(dtb, i);
		if (rc != 0) {
			INFO("rpi3: Can't remove mem reserve region (%d)\n", rc);
		}

		break;
	}

	if (i == regions) {
		VERBOSE("rpi3: Firmware mem reserve region not found\n");
	}

	/*
	 * Reserve all SRAM. As said in the documentation, this isn't actually
	 * secure memory, so it is needed to tell BL33 that this is a reserved
	 * memory region. It doesn't guarantee it won't use it, though.
	 */
	rc = fdt_add_mem_rsv(dtb, SEC_SRAM_BASE, SEC_SRAM_SIZE);
	if (rc != 0) {
		WARN("rpi3: Can't add mem reserve region (%d)\n", rc);
	}

	INFO("rpi3: Reserved 0x%llx - 0x%llx in DTB\n", SEC_SRAM_BASE,
	     SEC_SRAM_BASE + SEC_SRAM_SIZE);
}
#endif

void bl31_platform_setup(void)
{
#ifdef RPI3_PRELOADED_DTB_BASE
	/* Only modify a DTB if we know where to look for it */
	rpi3_dtb_add_mem_rsv();
#endif
}
