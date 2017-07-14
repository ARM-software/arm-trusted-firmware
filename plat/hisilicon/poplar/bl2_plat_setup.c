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
#include <mmio.h>
#include <partition/partition.h>
#include <platform.h>
#include <string.h>
#include "hi3798cv200.h"
#include "plat_private.h"

/* Memory ranges for code and read only data sections */
#define BL2_RO_BASE	(unsigned long)(&__RO_START__)
#define BL2_RO_LIMIT	(unsigned long)(&__RO_END__)

/* Memory ranges for coherent memory section */
#define BL2_COHERENT_RAM_BASE	(unsigned long)(&__COHERENT_RAM_START__)
#define BL2_COHERENT_RAM_LIMIT	(unsigned long)(&__COHERENT_RAM_END__)

typedef struct bl2_to_bl31_params_mem {
	bl31_params_t		bl31_params;
	image_info_t		bl31_image_info;
	image_info_t		bl33_image_info;
	entry_point_info_t	bl33_ep_info;
	entry_point_info_t	bl31_ep_info;
} bl2_to_bl31_params_mem_t;

static meminfo_t bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);
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
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info,
		       PARAM_IMAGE_BINARY, VERSION_1, 0);

	/* Fill BL3-3 related information */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem.bl33_ep_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_ep_info,
		       PARAM_EP, VERSION_1, 0);

	/* BL3-3 expects to receive the primary CPU MPID (through x0) */
	bl2_to_bl31_params->bl33_ep_info->args.arg0 = 0xffff & read_mpidr();

	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem.bl33_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info,
		       PARAM_IMAGE_BINARY, VERSION_1, 0);

	return bl2_to_bl31_params;
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

static uint32_t hisi_get_spsr_for_bl33_entry(void)
{
	unsigned long el_status;
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	el_status = read_id_aa64pfr0_el1() >> ID_AA64PFR0_EL2_SHIFT;
	el_status &= ID_AA64PFR0_ELX_MASK;

	mode = (el_status) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}

void bl2_plat_set_bl33_ep_info(image_info_t *image,
			       entry_point_info_t *bl33_ep_info)
{
	SET_SECURITY_STATE(bl33_ep_info->h.attr, NON_SECURE);
	bl33_ep_info->spsr = hisi_get_spsr_for_bl33_entry();
	bl33_ep_info->args.arg2 = image->image_size;
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
	bl33_meminfo->free_base  = DDR_BASE;
	bl33_meminfo->free_size  = DDR_SIZE;
}

void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	console_init(PL011_UART0_BASE, PL011_UART0_CLK_IN_HZ, PL011_BAUDRATE);

	/* Enable arch timer */
	generic_delay_timer_init();

	bl2_tzram_layout = *mem_layout;
}

void bl2_plat_arch_setup(void)
{
	plat_configure_mmu_el1(bl2_tzram_layout.total_base,
			       bl2_tzram_layout.total_size,
			       BL2_RO_BASE,
			       BL2_RO_LIMIT,
			       BL2_COHERENT_RAM_BASE,
			       BL2_COHERENT_RAM_LIMIT);
}

void bl2_platform_setup(void)
{
	plat_io_setup();
}

unsigned long plat_get_ns_image_entrypoint(void)
{
	return PLAT_ARM_NS_IMAGE_OFFSET;
}
