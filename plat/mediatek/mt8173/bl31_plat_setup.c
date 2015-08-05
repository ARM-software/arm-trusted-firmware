/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
#include <arm_gic.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <mcucfg.h>
#include <mmio.h>
#include <mtcmos.h>
#include <plat_private.h>
#include <platform.h>
#include <spm.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
unsigned long __RO_START__;
unsigned long __RO_END__;

unsigned long __COHERENT_RAM_START__;
unsigned long __COHERENT_RAM_END__;

/*
 * The next 2 constants identify the extents of the code & RO data region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __RO_START__ and __RO_END__ linker symbols refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;

static void platform_setup_cpu(void)
{
	/* turn off all the little core's power except cpu 0 */
	mtcmos_little_cpu_off();

	/* setup big cores */
	mmio_write_32((uintptr_t)&mt8173_mcucfg->mp1_config_res,
		MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp1_miscdbg, MP1_AINACTS);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp1_clkenm_div,
		MP1_SW_CG_GEN);
	mmio_clrbits_32((uintptr_t)&mt8173_mcucfg->mp1_rst_ctl,
		MP1_L2RSTDISABLE);

	/* set big cores arm64 boot mode */
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp1_cpucfg,
		MP1_CPUCFG_64BIT);

	/* set LITTLE cores arm64 boot mode */
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp0_rv_addr[0].rv_addr_hw,
		MP0_CPUCFG_64BIT);
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL3-1 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 * BL2 has flushed this information to memory, so we are guaranteed to pick up
 * good data.
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t *from_bl2,
			       void *plat_params_from_bl2)
{
	console_init(MT8173_UART0_BASE, MT8173_UART_CLOCK, MT8173_BAUDRATE);

	VERBOSE("bl31_setup\n");

	assert(from_bl2 != NULL);
	assert(from_bl2->h.type == PARAM_BL31);
	assert(from_bl2->h.version >= VERSION_1);

	assert(((unsigned long)plat_params_from_bl2) == MT_BL31_PLAT_PARAM_VAL);

	bl32_ep_info = *from_bl2->bl32_ep_info;
	bl33_ep_info = *from_bl2->bl33_ep_info;
}

/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	platform_setup_cpu();

	plat_delay_timer_init();

	/* Initialize the gic cpu and distributor interfaces */
	plat_mt_gic_init();
	arm_gic_setup();

	/* Topologies are best known to the platform. */
	mt_setup_topology();

	/* Initialize spm at boot time */
	spm_boot_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	plat_cci_init();
	plat_cci_enable();

	plat_configure_mmu_el3(BL31_RO_BASE,
			       (BL31_COHERENT_RAM_LIMIT - BL31_RO_BASE),
			       BL31_RO_BASE,
			       BL31_RO_LIMIT,
			       BL31_COHERENT_RAM_BASE,
			       BL31_COHERENT_RAM_LIMIT);
}

