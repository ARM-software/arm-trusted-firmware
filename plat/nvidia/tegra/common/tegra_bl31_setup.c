/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <cortex_a53.h>
#include <cortex_a57.h>
#include <denver.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <memctrl.h>
#include <profiler.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/* length of Trusty's input parameters (in bytes) */
#define TRUSTY_PARAMS_LEN_BYTES	(4096*2)

extern void memcpy16(void *dest, const void *src, unsigned int length);

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/

IMPORT_SYM(uint64_t, __RW_START__,	BL31_RW_START);
IMPORT_SYM(uint64_t, __RW_END__,	BL31_RW_END);
IMPORT_SYM(uint64_t, __RODATA_START__,	BL31_RODATA_BASE);
IMPORT_SYM(uint64_t, __RODATA_END__,	BL31_RODATA_END);
IMPORT_SYM(uint64_t, __TEXT_START__,	TEXT_START);
IMPORT_SYM(uint64_t, __TEXT_END__,	TEXT_END);

extern uint64_t tegra_bl31_phys_base;

static entry_point_info_t bl33_image_ep_info, bl32_image_ep_info;
static plat_params_from_bl2_t plat_bl31_params_from_bl2 = {
	.tzdram_size = TZDRAM_SIZE
};
static unsigned long bl32_mem_size;
static unsigned long bl32_boot_params;

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
#pragma weak plat_late_platform_setup

void plat_early_platform_setup(void)
{
	; /* do nothing */
}

struct tegra_bl31_params *plat_get_bl31_params(void)
{
	return NULL;
}

plat_params_from_bl2_t *plat_get_bl31_plat_params(void)
{
	return NULL;
}

void plat_late_platform_setup(void)
{
	; /* do nothing */
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *ep =  NULL;

	/* return BL32 entry point info if it is valid */
	if (type == NON_SECURE) {
		ep = &bl33_image_ep_info;
	} else if ((type == SECURE) && (bl32_image_ep_info.pc != 0U)) {
		ep = &bl32_image_ep_info;
	}

	return ep;
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
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				u_register_t arg2, u_register_t arg3)
{
	struct tegra_bl31_params *arg_from_bl2 = (struct tegra_bl31_params *) arg0;
	plat_params_from_bl2_t *plat_params = (plat_params_from_bl2_t *)arg1;
	image_info_t bl32_img_info = { {0} };
	uint64_t tzdram_start, tzdram_end, bl32_start, bl32_end, console_base;
	uint32_t console_clock;
	int32_t ret;
	static console_16550_t console;

	/*
	 * For RESET_TO_BL31 systems, BL31 is the first bootloader to run so
	 * there's no argument to relay from a previous bootloader. Platforms
	 * might use custom ways to get arguments, so provide handlers which
	 * they can override.
	 */
	if (arg_from_bl2 == NULL) {
		arg_from_bl2 = plat_get_bl31_params();
	}
	if (plat_params == NULL) {
		plat_params = plat_get_bl31_plat_params();
	}

	/*
	 * Copy BL3-3, BL3-2 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	assert(arg_from_bl2 != NULL);
	assert(arg_from_bl2->bl33_ep_info != NULL);
	bl33_image_ep_info = *arg_from_bl2->bl33_ep_info;

	if (arg_from_bl2->bl32_ep_info != NULL) {
		bl32_image_ep_info = *arg_from_bl2->bl32_ep_info;
		bl32_mem_size = arg_from_bl2->bl32_ep_info->args.arg0;
		bl32_boot_params = arg_from_bl2->bl32_ep_info->args.arg2;
	}

	/*
	 * Parse platform specific parameters
	 */
	assert(plat_params != NULL);
	plat_bl31_params_from_bl2.tzdram_base = plat_params->tzdram_base;
	plat_bl31_params_from_bl2.tzdram_size = plat_params->tzdram_size;
	plat_bl31_params_from_bl2.uart_id = plat_params->uart_id;
	plat_bl31_params_from_bl2.l2_ecc_parity_prot_dis = plat_params->l2_ecc_parity_prot_dis;
	plat_bl31_params_from_bl2.sc7entry_fw_size = plat_params->sc7entry_fw_size;
	plat_bl31_params_from_bl2.sc7entry_fw_base = plat_params->sc7entry_fw_base;

	/*
	 * It is very important that we run either from TZDRAM or TZSRAM base.
	 * Add an explicit check here.
	 */
	if ((plat_bl31_params_from_bl2.tzdram_base != (uint64_t)BL31_BASE) &&
	    (TEGRA_TZRAM_BASE != BL31_BASE)) {
		panic();
	}

	/*
	 * Reference clock used by the FPGAs is a lot slower.
	 */
	if (tegra_platform_is_fpga()) {
		console_clock = TEGRA_BOOT_UART_CLK_13_MHZ;
	} else {
		console_clock = TEGRA_BOOT_UART_CLK_408_MHZ;
	}

	/*
	 * Get the base address of the UART controller to be used for the
	 * console
	 */
	console_base = plat_get_console_from_id(plat_params->uart_id);

	if (console_base != 0U) {
		/*
		 * Configure the UART port to be used as the console
		 */
		(void)console_16550_register(console_base,
					     console_clock,
					     TEGRA_CONSOLE_BAUDRATE,
					     &console);
		console_set_scope(&console.console, CONSOLE_FLAG_BOOT |
			CONSOLE_FLAG_RUNTIME | CONSOLE_FLAG_CRASH);
	}

	/*
	 * The previous bootloader passes the base address of the shared memory
	 * location to store the boot profiler logs. Sanity check the
	 * address and initialise the profiler library, if it looks ok.
	 */
	if (plat_params->boot_profiler_shmem_base != 0ULL) {

		ret = bl31_check_ns_address(plat_params->boot_profiler_shmem_base,
				PROFILER_SIZE_BYTES);
		if (ret == (int32_t)0) {

			/* store the membase for the profiler lib */
			plat_bl31_params_from_bl2.boot_profiler_shmem_base =
				plat_params->boot_profiler_shmem_base;

			/* initialise the profiler library */
			boot_profiler_init(plat_params->boot_profiler_shmem_base,
					   TEGRA_TMRUS_BASE);
		}
	}

	/*
	 * Add timestamp for platform early setup entry.
	 */
	boot_profiler_add_record("[TF] early setup entry");

	/*
	 * Initialize delay timer
	 */
	tegra_delay_timer_init();

	/* Early platform setup for Tegra SoCs */
	plat_early_platform_setup();

	/*
	 * Do initial security configuration to allow DRAM/device access.
	 */
	tegra_memctrl_tzdram_setup(plat_bl31_params_from_bl2.tzdram_base,
			(uint32_t)plat_bl31_params_from_bl2.tzdram_size);

	/*
	 * The previous bootloader might not have placed the BL32 image
	 * inside the TZDRAM. We check the BL32 image info to find out
	 * the base/PC values and relocate the image if necessary.
	 */
	if (arg_from_bl2->bl32_image_info != NULL) {

		bl32_img_info = *arg_from_bl2->bl32_image_info;

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
		if ((bl32_start >= tzdram_end) || (bl32_end <= tzdram_start)) {

			INFO("Relocate BL32 to TZDRAM\n");

			(void)memcpy16((void *)(uintptr_t)bl32_image_ep_info.pc,
				 (void *)(uintptr_t)bl32_start,
				 bl32_img_info.image_size);

			/* clean up non-secure intermediate buffer */
			zeromem((void *)(uintptr_t)bl32_start,
				bl32_img_info.image_size);
		}
	}

	/*
	 * Add timestamp for platform early setup exit.
	 */
	boot_profiler_add_record("[TF] early setup exit");

	INFO("BL3-1: Boot CPU: %s Processor [%lx]\n",
	     (((read_midr() >> MIDR_IMPL_SHIFT) & MIDR_IMPL_MASK)
	      == DENVER_IMPL) ? "Denver" : "ARM", read_mpidr());
}

#ifdef SPD_trusty
void plat_trusty_set_boot_args(aapcs64_params_t *args)
{
	args->arg0 = bl32_mem_size;
	args->arg1 = bl32_boot_params;
	args->arg2 = TRUSTY_PARAMS_LEN_BYTES;

	/* update EKS size */
	if (args->arg4 != 0U) {
		args->arg2 = args->arg4;
	}

	/* Profiler Carveout Base */
	args->arg3 = args->arg5;
}
#endif

/*******************************************************************************
 * Initialize the gic, configure the SCR.
 ******************************************************************************/
void bl31_platform_setup(void)
{
	/*
	 * Add timestamp for platform setup entry.
	 */
	boot_profiler_add_record("[TF] plat setup entry");

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

	/*
	 * Late setup handler to allow platforms to performs additional
	 * functionality.
	 * This handler gets called with MMU enabled.
	 */
	plat_late_platform_setup();

	/*
	 * Add timestamp for platform setup exit.
	 */
	boot_profiler_add_record("[TF] plat setup exit");

	INFO("BL3-1: Tegra platform setup complete\n");
}

/*******************************************************************************
 * Perform any BL3-1 platform runtime setup prior to BL3-1 cold boot exit
 ******************************************************************************/
void bl31_plat_runtime_setup(void)
{
	/*
	 * During cold boot, it is observed that the arbitration
	 * bit is set in the Memory controller leading to false
	 * error interrupts in the non-secure world. To avoid
	 * this, clean the interrupt status register before
	 * booting into the non-secure world
	 */
	tegra_memctrl_clear_pending_interrupts();

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

	/*
	 * Add final timestamp before exiting BL31.
	 */
	boot_profiler_add_record("[TF] bl31 exit");
	boot_profiler_deinit();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	uint64_t rw_start = BL31_RW_START;
	uint64_t rw_size = BL31_RW_END - BL31_RW_START;
	uint64_t rodata_start = BL31_RODATA_BASE;
	uint64_t rodata_size = BL31_RODATA_END - BL31_RODATA_BASE;
	uint64_t code_base = TEXT_START;
	uint64_t code_size = TEXT_END - TEXT_START;
	const mmap_region_t *plat_mmio_map = NULL;
#if USE_COHERENT_MEM
	uint32_t coh_start, coh_size;
#endif
	const plat_params_from_bl2_t *params_from_bl2 = bl31_get_plat_params();

	/*
	 * Add timestamp for arch setup entry.
	 */
	boot_profiler_add_record("[TF] arch setup entry");

	/* add MMIO space */
	plat_mmio_map = plat_get_mmio_map();
	if (plat_mmio_map != NULL) {
		mmap_add(plat_mmio_map);
	} else {
		WARN("MMIO map not available\n");
	}

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

#if USE_COHERENT_MEM
	coh_start = total_base + (BL_COHERENT_RAM_BASE - BL31_RO_BASE);
	coh_size = BL_COHERENT_RAM_END - BL_COHERENT_RAM_BASE;

	mmap_add_region(coh_start, coh_start,
			coh_size,
			(uint8_t)MT_DEVICE | (uint8_t)MT_RW | (uint8_t)MT_SECURE);
#endif

	/* map TZDRAM used by BL31 as coherent memory */
	if (TEGRA_TZRAM_BASE == tegra_bl31_phys_base) {
		mmap_add_region(params_from_bl2->tzdram_base,
				params_from_bl2->tzdram_base,
				BL31_SIZE,
				MT_DEVICE | MT_RW | MT_SECURE);
	}

	/* set up translation tables */
	init_xlat_tables();

	/* enable the MMU */
	enable_mmu_el3(0);

	/*
	 * Add timestamp for arch setup exit.
	 */
	boot_profiler_add_record("[TF] arch setup exit");

	INFO("BL3-1: Tegra: MMU enabled\n");
}

/*******************************************************************************
 * Check if the given NS DRAM range is valid
 ******************************************************************************/
int32_t bl31_check_ns_address(uint64_t base, uint64_t size_in_bytes)
{
	uint64_t end = base + size_in_bytes - U(1);
	int32_t ret = 0;

	/*
	 * Check if the NS DRAM address is valid
	 */
	if ((base < TEGRA_DRAM_BASE) || (base >= TEGRA_DRAM_END) ||
	    (end > TEGRA_DRAM_END)) {

		ERROR("NS address 0x%llx is out-of-bounds!\n", base);
		ret = -EFAULT;
	}

	/*
	 * TZDRAM aperture contains the BL31 and BL32 images, so we need
	 * to check if the NS DRAM range overlaps the TZDRAM aperture.
	 */
	if ((base < (uint64_t)TZDRAM_END) && (end > tegra_bl31_phys_base)) {
		ERROR("NS address 0x%llx overlaps TZDRAM!\n", base);
		ret = -ENOTSUP;
	}

	/* valid NS address */
	return ret;
}
