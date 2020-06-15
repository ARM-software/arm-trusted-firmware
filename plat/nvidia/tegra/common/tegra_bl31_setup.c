/*
 * Copyright (c) 2015-2020, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
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
#include <cortex_a57.h>
#include <denver.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/utils.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <memctrl.h>
#include <profiler.h>
#include <smmu.h>
#include <tegra_def.h>
#include <tegra_platform.h>
#include <tegra_private.h>

/* length of Trusty's input parameters (in bytes) */
#define TRUSTY_PARAMS_LEN_BYTES	(4096*2)

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
IMPORT_SYM(uint64_t, __RW_START__,	BL31_RW_START);

extern uint64_t tegra_bl31_phys_base;

static entry_point_info_t bl33_image_ep_info, bl32_image_ep_info;
static plat_params_from_bl2_t plat_bl31_params_from_bl2 = {
	.tzdram_size = TZDRAM_SIZE
};
#ifdef SPD_trusty
static aapcs64_params_t bl32_args;
#endif

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
	int32_t ret;

	/*
	 * For RESET_TO_BL31 systems, BL31 is the first bootloader to run so
	 * there's no argument to relay from a previous bootloader. Platforms
	 * might use custom ways to get arguments.
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
#ifdef SPD_trusty
		/* save BL32 boot parameters */
		memcpy(&bl32_args, &arg_from_bl2->bl32_ep_info->args, sizeof(bl32_args));
#endif
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
	 * Enable console for the platform
	 */
	plat_enable_console(plat_params->uart_id);

	/*
	 * The previous bootloader passes the base address of the shared memory
	 * location to store the boot profiler logs. Sanity check the
	 * address and initialise the profiler library, if it looks ok.
	 */
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
	/*
	* arg0 = TZDRAM aperture available for BL32
	* arg1 = BL32 boot params
	* arg2 = EKS Blob Length
	* arg3 = Boot Profiler Carveout Base
	*/
	args->arg0 = bl32_args.arg0;
	args->arg1 = bl32_args.arg2;

	/* update EKS size */
	args->arg2 = bl32_args.arg4;

	/* Profiler Carveout Base */
	args->arg3 = bl32_args.arg5;
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
	 * Platform specific runtime setup
	 */
	plat_runtime_setup();

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
	uint64_t rw_size = BL_END - BL31_RW_START;
	uint64_t rodata_start = BL_RO_DATA_BASE;
	uint64_t rodata_size = BL_RO_DATA_END - BL_RO_DATA_BASE;
	uint64_t code_base = BL_CODE_BASE;
	uint64_t code_size = BL_CODE_END - BL_CODE_BASE;
	const mmap_region_t *plat_mmio_map = NULL;
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

	/*
	 * Sanity check the input values
	 */
	if ((base == 0U) || (size_in_bytes == 0U)) {
		ERROR("NS address 0x%llx (%lld bytes) is invalid\n",
			base, size_in_bytes);
		return -EINVAL;
	}

	/*
	 * Check if the NS DRAM address is valid
	 */
	if ((base < TEGRA_DRAM_BASE) || (base >= TEGRA_DRAM_END) ||
	    (end > TEGRA_DRAM_END)) {

		ERROR("NS address 0x%llx is out-of-bounds!\n", base);
		return -EFAULT;
	}

	/*
	 * TZDRAM aperture contains the BL31 and BL32 images, so we need
	 * to check if the NS DRAM range overlaps the TZDRAM aperture.
	 */
	if ((base < (uint64_t)TZDRAM_END) && (end > tegra_bl31_phys_base)) {
		ERROR("NS address 0x%llx overlaps TZDRAM!\n", base);
		return -ENOTSUP;
	}

	/* valid NS address */
	return 0;
}
