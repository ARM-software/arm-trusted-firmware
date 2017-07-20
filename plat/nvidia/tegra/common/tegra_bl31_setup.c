/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl31.h>
#include <bl_common.h>
#include <console.h>
#include <cortex_a53.h>
#include <cortex_a57.h>
#include <debug.h>
#include <denver.h>
#include <errno.h>
#include <memctrl.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <stddef.h>
#include <string.h>
#include <tegra_def.h>
#include <tegra_private.h>

extern void zeromem16(void *mem, unsigned int length);

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
extern unsigned long __TEXT_START__;
extern unsigned long __TEXT_END__;
extern unsigned long __RW_START__;
extern unsigned long __RW_END__;
extern unsigned long __RODATA_START__;
extern unsigned long __RODATA_END__;
extern unsigned long __BL31_END__;

extern uint64_t tegra_bl31_phys_base;
extern uint64_t tegra_console_base;

/*
 * The next 3 constants identify the extents of the code, RO data region and the
 * limit of the BL3-1 image.  These addresses are used by the MMU setup code and
 * therefore they must be page-aligned.  It is the responsibility of the linker
 * script to ensure that __RO_START__, __RO_END__ & __BL31_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_RW_START (unsigned long)(&__RW_START__)
#define BL31_RW_END (unsigned long)(&__RW_END__)
#define BL31_RODATA_BASE (unsigned long)(&__RODATA_START__)
#define BL31_RODATA_END (unsigned long)(&__RODATA_END__)
#define BL31_END (unsigned long)(&__BL31_END__)

static entry_point_info_t bl33_image_ep_info, bl32_image_ep_info;
static plat_params_from_bl2_t plat_bl31_params_from_bl2 = {
	.tzdram_size = (uint64_t)TZDRAM_SIZE
};

/*******************************************************************************
 * This variable holds the non-secure image entry address
 ******************************************************************************/
extern uint64_t ns_image_entrypoint;

/*******************************************************************************
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that will be overridden by a SoC.
 ******************************************************************************/
#pragma weak plat_early_platform_setup
#pragma weak plat_get_bl31_params
#pragma weak plat_get_bl31_plat_params

void plat_early_platform_setup(void)
{
	; /* do nothing */
}

bl31_params_t *plat_get_bl31_params(void)
{
	return NULL;
}

plat_params_from_bl2_t *plat_get_bl31_plat_params(void)
{
	return NULL;
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	if (type == NON_SECURE)
		return &bl33_image_ep_info;

	/* return BL32 entry point info if it is valid */
	if (type == SECURE && bl32_image_ep_info.pc)
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
#if LOG_LEVEL >= LOG_LEVEL_INFO
	int impl = (read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK;
#endif
	image_info_t bl32_img_info = { {0} };
	uint64_t tzdram_start, tzdram_end, bl32_start, bl32_end;

	/*
	 * For RESET_TO_BL31 systems, BL31 is the first bootloader to run so
	 * there's no argument to relay from a previous bootloader. Platforms
	 * might use custom ways to get arguments, so provide handlers which
	 * they can override.
	 */
	if (from_bl2 == NULL)
		from_bl2 = plat_get_bl31_params();
	if (plat_params == NULL)
		plat_params = plat_get_bl31_plat_params();

	/*
	 * Copy BL3-3, BL3-2 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	assert(from_bl2);
	assert(from_bl2->bl33_ep_info);
	bl33_image_ep_info = *from_bl2->bl33_ep_info;

	if (from_bl2->bl32_ep_info)
		bl32_image_ep_info = *from_bl2->bl32_ep_info;

	/*
	 * Parse platform specific parameters - TZDRAM aperture base and size
	 */
	assert(plat_params);
	plat_bl31_params_from_bl2.tzdram_base = plat_params->tzdram_base;
	plat_bl31_params_from_bl2.tzdram_size = plat_params->tzdram_size;
	plat_bl31_params_from_bl2.uart_id = plat_params->uart_id;

	/*
	 * It is very important that we run either from TZDRAM or TZSRAM base.
	 * Add an explicit check here.
	 */
	if ((plat_bl31_params_from_bl2.tzdram_base != BL31_BASE) &&
	    (TEGRA_TZRAM_BASE != BL31_BASE))
		panic();

	/*
	 * Get the base address of the UART controller to be used for the
	 * console
	 */
	tegra_console_base = plat_get_console_from_id(plat_params->uart_id);

	if (tegra_console_base != (uint64_t)0) {
		/*
		 * Configure the UART port to be used as the console
		 */
		console_init(tegra_console_base, TEGRA_BOOT_UART_CLK_IN_HZ,
			TEGRA_CONSOLE_BAUDRATE);
	}

	/*
	 * Initialize delay timer
	 */
	tegra_delay_timer_init();

	/*
	 * Do initial security configuration to allow DRAM/device access.
	 */
	tegra_memctrl_tzdram_setup(plat_bl31_params_from_bl2.tzdram_base,
			plat_bl31_params_from_bl2.tzdram_size);

	/*
	 * The previous bootloader might not have placed the BL32 image
	 * inside the TZDRAM. We check the BL32 image info to find out
	 * the base/PC values and relocate the image if necessary.
	 */
	if (from_bl2->bl32_image_info) {

		bl32_img_info = *from_bl2->bl32_image_info;

		/* Relocate BL32 if it resides outside of the TZDRAM */
		tzdram_start = plat_bl31_params_from_bl2.tzdram_base;
		tzdram_end = plat_bl31_params_from_bl2.tzdram_base +
				plat_bl31_params_from_bl2.tzdram_size;
		bl32_start = bl32_img_info.image_base;
		bl32_end = bl32_img_info.image_base + bl32_img_info.image_size;

		assert(tzdram_end > tzdram_start);
		assert(bl32_end > bl32_start);
		assert(bl32_image_ep_info.pc > tzdram_start);
		assert(bl32_image_ep_info.pc < tzdram_end);

		/* relocate BL32 */
		if (bl32_start >= tzdram_end || bl32_end <= tzdram_start) {

			INFO("Relocate BL32 to TZDRAM\n");

			memcpy16((void *)(uintptr_t)bl32_image_ep_info.pc,
				 (void *)(uintptr_t)bl32_start,
				 bl32_img_info.image_size);

			/* clean up non-secure intermediate buffer */
			zeromem16((void *)(uintptr_t)bl32_start,
				bl32_img_info.image_size);
		}
	}

	/* Early platform setup for Tegra SoCs */
	plat_early_platform_setup();

	INFO("BL3-1: Boot CPU: %s Processor [%lx]\n", (impl == DENVER_IMPL) ?
		"Denver" : "ARM", read_mpidr());
}

/*******************************************************************************
 * Initialize the gic, configure the SCR.
 ******************************************************************************/
void bl31_platform_setup(void)
{
	uint32_t tmp_reg;

	/* Initialize the gic cpu and distributor interfaces */
	plat_gic_setup();

	/*
	 * Setup secondary CPU POR infrastructure.
	 */
	plat_secondary_setup();

	/*
	 * Initial Memory Controller configuration.
	 */
	tegra_memctrl_setup();

	/*
	 * Set up the TZRAM memory aperture to allow only secure world
	 * access
	 */
	tegra_memctrl_tzram_setup(TEGRA_TZRAM_BASE, TEGRA_TZRAM_SIZE);

	/* Set the next EL to be AArch64 */
	tmp_reg = SCR_RES1_BITS | SCR_RW_BIT;
	write_scr(tmp_reg);

	INFO("BL3-1: Tegra platform setup complete\n");
}

/*******************************************************************************
 * Perform any BL3-1 platform runtime setup prior to BL3-1 cold boot exit
 ******************************************************************************/
void bl31_plat_runtime_setup(void)
{
	/*
	 * During boot, USB3 and flash media (SDMMC/SATA) devices need
	 * access to IRAM. Because these clients connect to the MC and
	 * do not have a direct path to the IRAM, the MC implements AHB
	 * redirection during boot to allow path to IRAM. In this mode
	 * accesses to a programmed memory address aperture are directed
	 * to the AHB bus, allowing access to the IRAM. This mode must be
	 * disabled before we jump to the non-secure world.
	 */
	tegra_memctrl_disable_ahb_redirection();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	unsigned long rw_start = BL31_RW_START;
	unsigned long rw_size = BL31_RW_END - BL31_RW_START;
	unsigned long rodata_start = BL31_RODATA_BASE;
	unsigned long rodata_size = BL31_RODATA_END - BL31_RODATA_BASE;
	unsigned long code_base = (unsigned long)(&__TEXT_START__);
	unsigned long code_size = (unsigned long)(&__TEXT_END__) - code_base;
	const mmap_region_t *plat_mmio_map = NULL;
#if USE_COHERENT_MEM
	unsigned long coh_start, coh_size;
#endif
	plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();

	/* add memory regions */
	mmap_add_region(rw_start, rw_start,
			rw_size,
			MT_MEMORY | MT_RW | MT_SECURE);
	mmap_add_region(rodata_start, rodata_start,
			rodata_size,
			MT_RO_DATA | MT_SECURE);
	mmap_add_region(code_base, code_base,
			code_size,
			MT_CODE | MT_SECURE);

	/* map TZDRAM used by BL31 as coherent memory */
	if (TEGRA_TZRAM_BASE == tegra_bl31_phys_base) {
		mmap_add_region(params_from_bl2->tzdram_base,
				params_from_bl2->tzdram_base,
				BL31_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE);
	}

#if USE_COHERENT_MEM
	coh_start = total_base + (BL_COHERENT_RAM_BASE - BL31_RO_BASE);
	coh_size = BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE;

	mmap_add_region(coh_start, coh_start,
			coh_size,
			MT_DEVICE | MT_RW | MT_SECURE);
#endif

	/* map on-chip free running uS timer */
	mmap_add_region(page_align((uint64_t)TEGRA_TMRUS_BASE, 0),
			page_align((uint64_t)TEGRA_TMRUS_BASE, 0),
			(uint64_t)TEGRA_TMRUS_SIZE,
			MT_DEVICE | MT_RO | MT_SECURE);

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

	INFO("BL3-1: Tegra: MMU enabled\n");
}

/*******************************************************************************
 * Check if the given NS DRAM range is valid
 ******************************************************************************/
int bl31_check_ns_address(uint64_t base, uint64_t size_in_bytes)
{
	uint64_t end = base + size_in_bytes;

	/*
	 * Check if the NS DRAM address is valid
	 */
	if ((base < TEGRA_DRAM_BASE) || (end > TEGRA_DRAM_END)) {
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
