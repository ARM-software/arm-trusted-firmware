/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <desc_image_load.h>
#include <errno.h>
#include <generic_delay_timer.h>
#include <hi3660.h>
#include <mmio.h>
#ifdef SPD_opteed
#include <optee_utils.h>
#endif
#include <platform_def.h>
#include <string.h>
#include <ufs.h>

#include "hikey960_def.h"
#include "hikey960_private.h"

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL2_RO_BASE (unsigned long)(&__RO_START__)
#define BL2_RO_LIMIT (unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL2_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL2_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

extern int load_lpm3(void);

/*******************************************************************************
 * Transfer SCP_BL2 from Trusted RAM using the SCP Download protocol.
 * Return 0 on success, -1 otherwise.
 ******************************************************************************/
int plat_hikey960_bl2_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	int i;
	int *buf;

	assert(scp_bl2_image_info->image_size < SCP_BL2_SIZE);

	INFO("BL2: Initiating SCP_BL2 transfer to SCP\n");

	INFO("BL2: SCP_BL2: 0x%lx@0x%x\n",
	     scp_bl2_image_info->image_base,
	     scp_bl2_image_info->image_size);

	buf = (int *)scp_bl2_image_info->image_base;

	INFO("BL2: SCP_BL2 HEAD:\n");
	for (i = 0; i < 64; i += 4)
		INFO("BL2: SCP_BL2 0x%x 0x%x 0x%x 0x%x\n",
			buf[i], buf[i+1], buf[i+2], buf[i+3]);

	buf = (int *)(scp_bl2_image_info->image_base +
		      scp_bl2_image_info->image_size - 256);

	INFO("BL2: SCP_BL2 TAIL:\n");
	for (i = 0; i < 64; i += 4)
		INFO("BL2: SCP_BL2 0x%x 0x%x 0x%x 0x%x\n",
			buf[i], buf[i+1], buf[i+2], buf[i+3]);

	INFO("BL2: SCP_BL2 transferred to SCP\n");

	load_lpm3();
	(void)buf;

	return 0;
}

void hikey960_init_ufs(void)
{
	ufs_params_t ufs_params;

	memset(&ufs_params, 0, sizeof(ufs_params_t));
	ufs_params.reg_base = UFS_REG_BASE;
	ufs_params.desc_base = HIKEY960_UFS_DESC_BASE;
	ufs_params.desc_size = HIKEY960_UFS_DESC_SIZE;
	ufs_params.flags = UFS_FLAGS_SKIPINIT;
	ufs_init(NULL, &ufs_params);
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t hikey960_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL3-2 image.
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
#ifndef AARCH32
uint32_t hikey960_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = EL_IMPLEMENTED(2) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#else
uint32_t hikey960_get_spsr_for_bl33_entry(void)
{
	unsigned int hyp_status, mode, spsr;

	hyp_status = GET_VIRT_EXT(read_id_pfr1());

	mode = (hyp_status) ? MODE32_hyp : MODE32_svc;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_MODE32(mode, plat_get_ns_image_entrypoint() & 0x1,
			SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#endif /* AARCH32 */

int hikey960_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);
#ifdef SPD_opteed
	bl_mem_params_node_t *pager_mem_params = NULL;
	bl_mem_params_node_t *paged_mem_params = NULL;
#endif
	assert(bl_mem_params);

	switch (image_id) {
#ifdef AARCH64
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
		bl_mem_params->ep_info.spsr = hikey960_get_spsr_for_bl32_entry();
		break;
#endif

	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = hikey960_get_spsr_for_bl33_entry();
		break;

#ifdef SCP_BL2_BASE
	case SCP_BL2_IMAGE_ID:
		/* The subsequent handling of SCP_BL2 is platform specific */
		err = plat_hikey960_bl2_handle_scp_bl2(&bl_mem_params->image_info);
		if (err) {
			WARN("Failure in platform-specific handling of SCP_BL2 image.\n");
		}
		break;
#endif
	}

	return err;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return hikey960_bl2_handle_post_image_load(image_id);
}

void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	unsigned int id, uart_base;

	generic_delay_timer_init();
	hikey960_read_boardid(&id);
	if (id == 5300)
		uart_base = PL011_UART5_BASE;
	else
		uart_base = PL011_UART6_BASE;

	/* Initialize the console to provide early debug support */
	console_init(uart_base, PL011_UART_CLK_IN_HZ, PL011_BAUDRATE);

	/* Setup the BL2 memory layout */
	bl2_tzram_layout = *mem_layout;
}

void bl2_plat_arch_setup(void)
{
	hikey960_init_mmu_el1(bl2_tzram_layout.total_base,
			      bl2_tzram_layout.total_size,
			      BL2_RO_BASE,
			      BL2_RO_LIMIT,
			      BL2_COHERENT_RAM_BASE,
			      BL2_COHERENT_RAM_LIMIT);
}

void bl2_platform_setup(void)
{
	/* disable WDT0 */
	if (mmio_read_32(WDT0_REG_BASE + WDT_LOCK_OFFSET) == WDT_LOCKED) {
		mmio_write_32(WDT0_REG_BASE + WDT_LOCK_OFFSET, WDT_UNLOCK);
		mmio_write_32(WDT0_REG_BASE + WDT_CONTROL_OFFSET, 0);
		mmio_write_32(WDT0_REG_BASE + WDT_LOCK_OFFSET, 0);
	}
}
