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
#include <errno.h>
#include <generic_delay_timer.h>
#include <hi3660.h>
#include <mmio.h>
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

typedef struct bl2_to_bl31_params_mem {
	bl31_params_t		bl31_params;
	image_info_t		bl31_image_info;
	image_info_t		bl32_image_info;
	image_info_t		bl33_image_info;
	entry_point_info_t	bl33_ep_info;
	entry_point_info_t	bl32_ep_info;
	entry_point_info_t	bl31_ep_info;
} bl2_to_bl31_params_mem_t;

static bl2_to_bl31_params_mem_t bl31_params_mem;

meminfo_t *bl2_plat_sec_mem_layout(void)
{
	return &bl2_tzram_layout;
}

bl31_params_t *bl2_plat_get_bl31_params(void)
{
	bl31_params_t *bl2_to_bl31_params = NULL;

	/*
	 * Initialise the memory for all the arguments that needs to
	 * be passed to BL3-1
	 */
	memset(&bl31_params_mem, 0, sizeof(bl2_to_bl31_params_mem_t));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem.bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, PARAM_BL31, VERSION_1, 0);

	/* Fill BL3-1 related information */
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem.bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);

	/* Fill BL3-2 related information if it exists */
#if BL32_BASE
	bl2_to_bl31_params->bl32_ep_info = &bl31_params_mem.bl32_ep_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_ep_info, PARAM_EP,
		VERSION_1, 0);
	bl2_to_bl31_params->bl32_image_info = &bl31_params_mem.bl32_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl32_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);
#endif

	/* Fill BL3-3 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_ep_info,
		PARAM_EP, VERSION_1, 0);

	/* BL3-3 expects to receive the primary CPU MPID (through x0) */
	bl2_to_bl31_params->bl33_ep_info->args.arg0 = 0xffff & read_mpidr();

	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);

	return bl2_to_bl31_params;
}

/*******************************************************************************
 * Populate the extents of memory available for loading SCP_BL2 (if used),
 * i.e. anywhere in trusted RAM as long as it doesn't overwrite BL2.
 ******************************************************************************/
void bl2_plat_get_scp_bl2_meminfo(meminfo_t *scp_bl2_meminfo)
{
	ufs_params_t ufs_params;

	memset(&ufs_params, 0, sizeof(ufs_params_t));
	ufs_params.reg_base = UFS_REG_BASE;
	ufs_params.desc_base = HIKEY960_UFS_DESC_BASE;
	ufs_params.desc_size = HIKEY960_UFS_DESC_SIZE;
	ufs_params.flags = UFS_FLAGS_SKIPINIT;
	ufs_init(NULL, &ufs_params);

	hikey960_io_setup();

	*scp_bl2_meminfo = bl2_tzram_layout;
}

extern int load_lpm3(void);

int bl2_plat_handle_scp_bl2(image_info_t *scp_bl2_image_info)
{
	int i;
	int *buf;

	assert(scp_bl2_image_info->image_size < SCP_MEM_SIZE);

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

	memcpy((void *)SCP_MEM_BASE,
	       (void *)scp_bl2_image_info->image_base,
	       scp_bl2_image_info->image_size);

	INFO("BL2: SCP_BL2 transferred to SCP\n");

	load_lpm3();
	(void)buf;

	return 0;
}

struct entry_point_info *bl2_plat_get_bl31_ep_info(void)
{
	return &bl31_params_mem.bl31_ep_info;
}

void bl2_plat_set_bl31_ep_info(image_info_t *image,
			       entry_point_info_t *bl31_ep_info)
{
	SET_SECURITY_STATE(bl31_ep_info->h.attr, SECURE);
	bl31_ep_info->spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
				       DISABLE_ALL_EXCEPTIONS);
}

/*******************************************************************************
 * Before calling this function BL32 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL32 and set SPSR and security state.
 * On Hikey we only set the security state of the entrypoint
 ******************************************************************************/
#ifdef BL32_BASE
void bl2_plat_set_bl32_ep_info(image_info_t *bl32_image_info,
					entry_point_info_t *bl32_ep_info)
{
	SET_SECURITY_STATE(bl32_ep_info->h.attr, SECURE);
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	bl32_ep_info->spsr = 0;
}

/*******************************************************************************
 * Populate the extents of memory available for loading BL32
 ******************************************************************************/
void bl2_plat_get_bl32_meminfo(meminfo_t *bl32_meminfo)
{
	/*
	 * Populate the extents of memory available for loading BL32.
	 */
	bl32_meminfo->total_base = BL32_BASE;
	bl32_meminfo->free_base = BL32_BASE;
	bl32_meminfo->total_size =
			(TSP_SEC_MEM_BASE + TSP_SEC_MEM_SIZE) - BL32_BASE;
	bl32_meminfo->free_size =
			(TSP_SEC_MEM_BASE + TSP_SEC_MEM_SIZE) - BL32_BASE;
}
#endif /* BL32_BASE */

void bl2_plat_set_bl33_ep_info(image_info_t *image,
			       entry_point_info_t *bl33_ep_info)
{
	unsigned long el_status;
	unsigned int mode;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	if (el_status)
		mode = MODE_EL2;
	else
		mode = MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	bl33_ep_info->spsr = SPSR_64(mode, MODE_SP_ELX,
				       DISABLE_ALL_EXCEPTIONS);
	SET_SECURITY_STATE(bl33_ep_info->h.attr, NON_SECURE);
}

void bl2_plat_flush_bl31_params(void)
{
	flush_dcache_range((unsigned long)&bl31_params_mem,
			   sizeof(bl2_to_bl31_params_mem_t));
}

void bl2_plat_get_bl33_meminfo(meminfo_t *bl33_meminfo)
{
	bl33_meminfo->total_base = DDR_BASE;
	bl33_meminfo->total_size = DDR_SIZE;
	bl33_meminfo->free_base = DDR_BASE;
	bl33_meminfo->free_size = DDR_SIZE;
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
