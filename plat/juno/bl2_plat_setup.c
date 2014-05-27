/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl2.h>
#include <console.h>
#include <debug.h>
#include <platform.h>
#include <scp_bootloader.h>
#include <string.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted RAM
 ******************************************************************************/
extern unsigned long __RO_START__;
extern unsigned long __RO_END__;

extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;

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

/* Pointer to memory visible to both BL2 and BL3-1 for passing data */
extern unsigned char **bl2_el_change_mem_ptr;

/* Data structure which holds the extents of the trusted RAM for BL2 */
static meminfo_t bl2_tzram_layout
__attribute__ ((aligned(PLATFORM_CACHE_LINE_SIZE),
		section("tzfw_coherent_mem")));

/*******************************************************************************
 * Reference to structures which holds the arguments which need to be passed
 * to BL31
 ******************************************************************************/
static bl31_params_t *bl2_to_bl31_params;
static entry_point_info_t *bl31_ep_info;

meminfo_t *bl2_plat_sec_mem_layout(void)
{
	return &bl2_tzram_layout;
}

/*******************************************************************************
 * This function assigns a pointer to the memory that the platform has kept
 * aside to pass platform specific and trusted firmware related information
 * to BL31. This memory is allocated by allocating memory to
 * bl2_to_bl31_params_mem_t structure which is a superset of all the
 * structure whose information is passed to BL31
 * NOTE: This function should be called only once and should be done
 * before generating params to BL31
 ******************************************************************************/
bl31_params_t *bl2_plat_get_bl31_params(void)
{
	bl2_to_bl31_params_mem_t *bl31_params_mem;

	/*
	 * Allocate the memory for all the arguments that needs to
	 * be passed to BL31
	 */
	bl31_params_mem = (bl2_to_bl31_params_mem_t *)PARAMS_BASE;
	memset((void *)PARAMS_BASE, 0, sizeof(bl2_to_bl31_params_mem_t));

	/* Assign memory for TF related information */
	bl2_to_bl31_params = &bl31_params_mem->bl31_params;
	SET_PARAM_HEAD(bl2_to_bl31_params, PARAM_BL31, VERSION_1, 0);

	/* Fill BL31 related information */
	bl31_ep_info = &bl31_params_mem->bl31_ep_info;
	bl2_to_bl31_params->bl31_image_info = &bl31_params_mem->bl31_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl31_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);

	/* Fill BL32 related information if it exists */
	if (BL32_BASE) {
		bl2_to_bl31_params->bl32_ep_info =
			&bl31_params_mem->bl32_ep_info;
		SET_PARAM_HEAD(bl2_to_bl31_params->bl32_ep_info,
			PARAM_EP, VERSION_1, 0);
		bl2_to_bl31_params->bl32_image_info =
			&bl31_params_mem->bl32_image_info;
		SET_PARAM_HEAD(bl2_to_bl31_params->bl32_image_info,
			PARAM_IMAGE_BINARY,
			VERSION_1, 0);
	}

	/* Fill BL33 related information */
	/* Juno TODO: Pass the primary CPU MPID to UEFI. Must be in x0. */
	bl2_to_bl31_params->bl33_ep_info = &bl31_params_mem->bl33_ep_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_ep_info,
		PARAM_EP, VERSION_1, 0);
	bl2_to_bl31_params->bl33_image_info = &bl31_params_mem->bl33_image_info;
	SET_PARAM_HEAD(bl2_to_bl31_params->bl33_image_info, PARAM_IMAGE_BINARY,
		VERSION_1, 0);

	return bl2_to_bl31_params;
}

/*******************************************************************************
 * This function returns a pointer to the shared memory that the platform
 * has kept to point to entry point information of BL31 to BL2
 ******************************************************************************/
struct entry_point_info *bl2_plat_get_bl31_ep_info(void)
{
	return bl31_ep_info;
}

/*******************************************************************************
 * BL1 has passed the extents of the trusted RAM that should be visible to BL2
 * in x0. This memory layout is sitting at the base of the free trusted RAM.
 * Copy it to a safe loaction before its reclaimed by later BL2 functionality.
 ******************************************************************************/
void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	/* Initialize the console to provide early debug support */
	console_init(PL011_UART0_BASE);

	/* Setup the BL2 memory layout */
	bl2_tzram_layout.total_base = mem_layout->total_base;
	bl2_tzram_layout.total_size = mem_layout->total_size;
	bl2_tzram_layout.free_base = mem_layout->free_base;
	bl2_tzram_layout.free_size = mem_layout->free_size;
	bl2_tzram_layout.attr = mem_layout->attr;
	bl2_tzram_layout.next = 0;
}

/*******************************************************************************
 * Load BL3-0 into Trusted RAM, then transfer it using the SCP Download
 * protocol. The image is loaded into RAM in the same place that BL3-1 will be
 * loaded later so here, we copy the RAM layout structure and use it to load
 * the image into. When this function exits, the RAM layout remains untouched
 * so the BL2 can load BL3-1 as normal.
 ******************************************************************************/
static int load_bl30(void)
{
	meminfo_t *bl2_tzram_layout;
	meminfo_t tmp_tzram_layout;
	uintptr_t bl30_base;
	uint32_t bl30_size;
	unsigned int bl2_load, bl30_load;
	int ret;
	int e;
	image_info_t bl30_image_info;
	entry_point_info_t bl30_ep;

	/* Find out how much free trusted ram remains after BL2 load */
	bl2_tzram_layout = bl2_plat_sec_mem_layout();

	/* copy the TZRAM layout and use it */
	memcpy(&tmp_tzram_layout, bl2_tzram_layout, sizeof(meminfo_t));

	/* Work out where to load BL3-0 before transferring to SCP */
	bl2_load = tmp_tzram_layout.attr & LOAD_MASK;
	assert((bl2_load == TOP_LOAD) || (bl2_load == BOT_LOAD));
	bl30_load = (bl2_load == TOP_LOAD) ? BOT_LOAD : TOP_LOAD;

	SET_PARAM_HEAD(&bl30_image_info, PARAM_IMAGE_BINARY, VERSION_1, 0);

	/* Load the BL3-0 image */
	e = load_image(&tmp_tzram_layout,
		       BL30_IMAGE_NAME,
		       bl30_load,
		       BL30_BASE,
		       &bl30_image_info,
		       &bl30_ep);

	/* Panic if it has not been possible to load BL3-0 */
	if (e) {
		ERROR("Failed to load BL3-0 image.\n");
		panic();
	}

	bl30_base = bl30_image_info.image_base;
	bl30_size = bl30_image_info.image_size;
	assert(bl30_base != 0);
	assert(bl30_size != 0);

	INFO("BL2: BL3-0 loaded at 0x%lx, len=%d (0x%x)\n\r", bl30_base,
		bl30_size, bl30_size);
	flush_dcache_range(bl30_base, bl30_size);
	ret = scp_bootloader_transfer((void *)bl30_base, bl30_size);

	if (ret == 0)
		INFO("BL2: BL3-0 loaded and transferred to SCP\n\r");
	else
		ERROR("BL2: BL3-0 load and transfer failure\n\r");

	return ret;
}

/*******************************************************************************
 * Perform platform specific setup, i.e. initialize the IO layer, load BL3-0
 * image and initialise the memory location to use for passing arguments to
 * BL3-1.
 ******************************************************************************/
void bl2_platform_setup(void)
{
	/* Initialise the IO layer and register platform IO devices */
	io_setup();

	/* Load BL3-0  */
	if (load_bl30() != 0)
		panic();
}

/* Flush the TF params and the TF plat params */
void bl2_plat_flush_bl31_params(void)
{
	flush_dcache_range((unsigned long)PARAMS_BASE,
			sizeof(bl2_to_bl31_params_mem_t));
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl2_plat_arch_setup()
{
	configure_mmu_el1(bl2_tzram_layout.total_base,
			  bl2_tzram_layout.total_size,
			  BL2_RO_BASE,
			  BL2_RO_LIMIT,
			  BL2_COHERENT_RAM_BASE,
			  BL2_COHERENT_RAM_LIMIT);
}


/*******************************************************************************
 * Before calling this function BL31 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL31 and set SPSR and security state.
 * On Juno we are only setting the security state, entrypoint
 ******************************************************************************/
void bl2_plat_set_bl31_ep_info(image_info_t *bl31_image_info,
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
 * On Juno we are only setting the security state, entrypoint
 ******************************************************************************/
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
 * Before calling this function BL33 is loaded in memory and its entrypoint
 * is set by load_image. This is a placeholder for the platform to change
 * the entrypoint of BL33 and set SPSR and security state.
 * On Juno we are only setting the security state, entrypoint
 ******************************************************************************/
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

/*******************************************************************************
 * Populate the extents of memory available for loading BL3-2
 ******************************************************************************/
void bl2_plat_get_bl32_meminfo(meminfo_t *bl32_meminfo)
{
	/*
	 * Populate the extents of memory available for loading BL3-2.
	 */
	bl32_meminfo->total_base = BL32_BASE;
	bl32_meminfo->free_base = BL32_BASE;
	bl32_meminfo->total_size =
		       (TSP_SEC_MEM_BASE + TSP_SEC_MEM_SIZE) - BL32_BASE;
	bl32_meminfo->free_size =
		       (TSP_SEC_MEM_BASE + TSP_SEC_MEM_SIZE) - BL32_BASE;
	bl32_meminfo->attr = BOT_LOAD;
	bl32_meminfo->next = 0;
}


/*******************************************************************************
 * Populate the extents of memory available for loading BL3-3
 ******************************************************************************/
void bl2_plat_get_bl33_meminfo(meminfo_t *bl33_meminfo)
{
	bl33_meminfo->total_base = DRAM_BASE;
	bl33_meminfo->total_size = DRAM_SIZE;
	bl33_meminfo->free_base = DRAM_BASE;
	bl33_meminfo->free_size = DRAM_SIZE;
	bl33_meminfo->attr = 0;
	bl33_meminfo->attr = 0;
}
