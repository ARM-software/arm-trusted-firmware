/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <plat_arm.h>
#include <platform.h>
#include <platform_def.h>
#include <platform_sp_min.h>

#define BL32_END (uintptr_t)(&__BL32_END__)

#if USE_COHERENT_MEM
/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols refer to
 * page-aligned addresses.
 */
#define BL32_COHERENT_RAM_BASE (uintptr_t)(&__COHERENT_RAM_START__)
#define BL32_COHERENT_RAM_LIMIT (uintptr_t)(&__COHERENT_RAM_END__)
#endif


static entry_point_info_t bl33_image_ep_info;

/* Weak definitions may be overridden in specific ARM standard platform */
#pragma weak sp_min_early_platform_setup
#pragma weak sp_min_platform_setup
#pragma weak sp_min_plat_arch_setup


/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for the
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *sp_min_plat_get_bl33_ep_info(void)
{
	entry_point_info_t *next_image_info;

	next_image_info = &bl33_image_ep_info;

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
 * Perform early platform setup.
 ******************************************************************************/
void arm_sp_min_early_platform_setup(void *from_bl2,
		void *plat_params_from_bl2)
{
	/* Initialize the console to provide early debug support */
	console_init(PLAT_ARM_BOOT_UART_BASE, PLAT_ARM_BOOT_UART_CLK_IN_HZ,
				ARM_CONSOLE_BAUDRATE);

#if RESET_TO_SP_MIN
	/* There are no parameters from BL2 if SP_MIN is a reset vector */
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info,
				PARAM_EP,
				VERSION_1,
				0);
	/*
	 * Tell SP_MIN where the non-trusted software image
	 * is located and the entry state information
	 */
	bl33_image_ep_info.pc = plat_get_ns_image_entrypoint();
	bl33_image_ep_info.spsr = arm_get_spsr_for_bl33_entry();
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

#else /* RESET_TO_SP_MIN */

	/*
	 * Check params passed from BL2 should not be NULL,
	 */
	bl_params_t *params_from_bl2 = (bl_params_t *)from_bl2;
	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params) {
		if (bl_params->image_id == BL33_IMAGE_ID) {
			bl33_image_ep_info = *bl_params->ep_info;
			break;
		}

		bl_params = bl_params->next_params_info;
	}

	if (bl33_image_ep_info.pc == 0)
		panic();

#endif /* RESET_TO_SP_MIN */

}

void sp_min_early_platform_setup(void *from_bl2,
		void *plat_params_from_bl2)
{
	arm_sp_min_early_platform_setup(from_bl2, plat_params_from_bl2);

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
 * Perform platform specific setup for SP_MIN
 ******************************************************************************/
void sp_min_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();

	/*
	 * Do initial security configuration to allow DRAM/device access
	 * (if earlier BL has not already done so).
	 */
#if RESET_TO_SP_MIN
	plat_arm_security_setup();
#endif

	/* Enable and initialize the System level generic timer */
	mmio_write_32(ARM_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0) | CNTCR_EN);

	/* Allow access to the System counter timer module */
	arm_configure_sys_timer();

	/* Initialize power controller before setting up topology */
	plat_arm_pwrc_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only initializes the MMU
 ******************************************************************************/
void sp_min_plat_arch_setup(void)
{

	arm_setup_page_tables(BL32_BASE,
			      (BL32_END - BL32_BASE),
			      BL_CODE_BASE,
			      BL_CODE_LIMIT,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_LIMIT
#if USE_COHERENT_MEM
			      , BL32_COHERENT_RAM_BASE,
			      BL32_COHERENT_RAM_LIMIT
#endif
			      );

	enable_mmu_secure(0);
}
