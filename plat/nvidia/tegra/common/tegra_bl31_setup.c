/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl31.h>
#include <bl_common.h>
#include <console.h>
#include <cortex_a57.h>
#include <cortex_a53.h>
#include <debug.h>
#include <errno.h>
#include <memctrl.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <tegra_private.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
extern unsigned long __RO_START__;
extern unsigned long __RO_END__;
extern unsigned long __BL31_END__;

#if USE_COHERENT_MEM
extern unsigned long __COHERENT_RAM_START__;
extern unsigned long __COHERENT_RAM_END__;
#endif

extern uint64_t tegra_bl31_phys_base;

/*
 * The next 3 constants identify the extents of the code, RO data region and the
 * limit of the BL3-1 image.  These addresses are used by the MMU setup code and
 * therefore they must be page-aligned.  It is the responsibility of the linker
 * script to ensure that __RO_START__, __RO_END__ & __BL31_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)
#define BL31_END (unsigned long)(&__BL31_END__)

#if USE_COHERENT_MEM
/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)
#endif

static entry_point_info_t bl33_image_ep_info, bl32_image_ep_info;
static plat_params_from_bl2_t plat_bl31_params_from_bl2 = {
	.tzdram_size = (uint64_t)TZDRAM_SIZE
};

/*******************************************************************************
 * This variable holds the non-secure image entry address
 ******************************************************************************/
extern uint64_t ns_image_entrypoint;

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	if (type == NON_SECURE)
		return &bl33_image_ep_info;

	if (type == SECURE)
		return &bl32_image_ep_info;

	return NULL;
}

/*******************************************************************************
 * Return a pointer to the 'plat_params_from_bl2_t' structure. The BL2 image
 * passes this platform specific information.
 ******************************************************************************/
plat_params_from_bl2_t *bl31_get_plat_params(void)
{
	return &plat_bl31_params_from_bl2;
}

/*******************************************************************************
 * Perform any BL31 specific platform actions. Populate the BL33 and BL32 image
 * info.
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t *from_bl2,
				void *plat_params_from_bl2)
{
	plat_params_from_bl2_t *plat_params =
		(plat_params_from_bl2_t *)plat_params_from_bl2;

	/*
	 * Configure the UART port to be used as the console
	 */
	console_init(TEGRA_BOOT_UART_BASE, TEGRA_BOOT_UART_CLK_IN_HZ,
			TEGRA_CONSOLE_BAUDRATE);

	/* Initialise crash console */
	plat_crash_console_init();

	/*
	 * Copy BL3-3, BL3-2 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	bl33_image_ep_info = *from_bl2->bl33_ep_info;
	bl32_image_ep_info = *from_bl2->bl32_ep_info;

	/*
	 * Parse platform specific parameters - TZDRAM aperture size
	 */
	if (plat_params)
		plat_bl31_params_from_bl2.tzdram_size = plat_params->tzdram_size;
}

/*******************************************************************************
 * Initialize the gic, configure the SCR.
 ******************************************************************************/
void bl31_platform_setup(void)
{
	uint32_t tmp_reg;

	/*
	 * Initialize delay timer
	 */
	tegra_delay_timer_init();

	/*
	 * Setup secondary CPU POR infrastructure.
	 */
	plat_secondary_setup();

	/*
	 * Initial Memory Controller configuration.
	 */
	tegra_memctrl_setup();

	/*
	 * Do initial security configuration to allow DRAM/device access.
	 */
	tegra_memctrl_tzdram_setup(tegra_bl31_phys_base,
			plat_bl31_params_from_bl2.tzdram_size);

	/* Set the next EL to be AArch64 */
	tmp_reg = SCR_RES1_BITS | SCR_RW_BIT;
	write_scr(tmp_reg);

	/* Initialize the gic cpu and distributor interfaces */
	tegra_gic_setup();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	unsigned long bl31_base_pa = tegra_bl31_phys_base;
	unsigned long total_base = bl31_base_pa;
	unsigned long total_size = BL32_BASE - BL31_RO_BASE;
	unsigned long ro_start = bl31_base_pa;
	unsigned long ro_size = BL31_RO_LIMIT - BL31_RO_BASE;
	const mmap_region_t *plat_mmio_map = NULL;
#if USE_COHERENT_MEM
	unsigned long coh_start, coh_size;
#endif

	/* add memory regions */
	mmap_add_region(total_base, total_base,
			total_size,
			MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(ro_start, ro_start,
			ro_size,
			MT_MEMORY | MT_RO | MT_SECURE);

#if USE_COHERENT_MEM
	coh_start = total_base + (BL31_COHERENT_RAM_BASE - BL31_RO_BASE);
	coh_size = BL31_COHERENT_RAM_LIMIT - BL31_COHERENT_RAM_BASE;

	mmap_add_region(coh_start, coh_start,
			coh_size,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	/* add MMIO space */
	plat_mmio_map = plat_get_mmio_map();
	if (plat_mmio_map)
		mmap_add(plat_mmio_map);
	else
		WARN("MMIO map not available\n");

	/* set up translation tables */
	init_xlat_tables();

	/* enable the MMU */
	enable_mmu_el3(0);
}

/*******************************************************************************
 * Check if the given NS DRAM range is valid
 ******************************************************************************/
int bl31_check_ns_address(uint64_t base, uint64_t size_in_bytes)
{
	uint64_t end = base + size_in_bytes - 1;

	/*
	 * Check if the NS DRAM address is valid
	 */
	if ((base < TEGRA_DRAM_BASE) || (end > TEGRA_DRAM_END) ||
	    (base >= end)) {
		ERROR("NS address is out-of-bounds!\n");
		return -EFAULT;
	}

	/*
	 * TZDRAM aperture contains the BL31 and BL32 images, so we need
	 * to check if the NS DRAM range overlaps the TZDRAM aperture.
	 */
	if ((base < TZDRAM_END) && (end > tegra_bl31_phys_base)) {
		ERROR("NS address overlaps TZDRAM!\n");
		return -ENOTSUP;
	}

	/* valid NS address */
	return 0;
}
