/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <assert.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/desc_image_load.h>
#include <common/tbbr/tbbr_img_def.h>
#include <drivers/console.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <mmu_def.h>
#include <sfp.h>
#include <plat_common.h>


#pragma weak soc_early_init
#pragma weak plat_early_init
#pragma weak plat_setup_page_tables
#pragma weak plat_bl2_platform_setup

void plat_setup_page_tables(uintptr_t total_base,
			   size_t total_size,
			   uintptr_t code_start,
			   uintptr_t code_limit,
			   uintptr_t rodata_start,
			   uintptr_t rodata_limit
#if USE_COHERENT_MEM
			   ,
			   uintptr_t coh_start,
			   uintptr_t coh_limit
#endif
			   )
{
}

void plat_early_init(void)
{
}

void soc_early_init(void)
{
}

static struct dram_rgn_info dram_regions_info  = {0};


/*******************************************************************************
 * Return the pointer to the 'dram_regions_info structure of the DRAM.
 * This structure is populated after _init_ddr().
 ******************************************************************************/
struct dram_rgn_info *get_dram_regions_info(void)
{
	return &dram_regions_info;
}

#ifdef DDR_INIT
static void populate_dram_regions_info(void)
{
	long long dram_remain_size = dram_regions_info.total_dram_size;
	uint8_t reg_id = 0;

	dram_regions_info.region[reg_id].addr = NXP_DRAM0_ADDR;
	dram_regions_info.region[reg_id].size =
			dram_remain_size > NXP_DRAM0_MAX_SIZE ?
				NXP_DRAM0_MAX_SIZE : dram_remain_size;

	if (dram_regions_info.region[reg_id].size != NXP_DRAM0_SIZE)
		NOTICE("Incorrect DRAM0 size is defined in platform_def.h\n");

	dram_remain_size -= dram_regions_info.region[reg_id].size;
	dram_regions_info.region[reg_id].size -= (NXP_SECURE_DRAM_SIZE
						+ NXP_SP_SHRD_DRAM_SIZE);

	assert(dram_regions_info.region[reg_id].size > 0);

	/* Reducing total dram size by 66MB */
	dram_regions_info.total_dram_size -= (NXP_SECURE_DRAM_SIZE
						+ NXP_SP_SHRD_DRAM_SIZE);

#if defined(NXP_DRAM1_ADDR) && defined(NXP_DRAM1_MAX_SIZE)
	if (dram_remain_size > 0) {
		reg_id++;
		dram_regions_info.region[reg_id].addr = NXP_DRAM1_ADDR;
		dram_regions_info.region[reg_id].size =
				dram_remain_size > NXP_DRAM1_MAX_SIZE ?
					NXP_DRAM1_MAX_SIZE : dram_remain_size;
		dram_remain_size -= dram_regions_info.region[reg_id].size;
	}
#endif
#if defined(NXP_DRAM1_ADDR) && defined(NXP_DRAM1_MAX_SIZE)
	if (dram_remain_size > 0) {
		reg_id++;
		dram_regions_info.region[reg_id].addr = NXP_DRAM1_ADDR;
		dram_regions_info.region[reg_id].size =
				dram_remain_size > NXP_DRAM1_MAX_SIZE ?
					NXP_DRAM1_MAX_SIZE : dram_remain_size;
		dram_remain_size -= dram_regions_info.region[reg_id].size;
	}
#endif
	reg_id++;
	dram_regions_info.num_dram_regions = reg_id;
}
#endif

void bl2_el3_early_platform_setup(u_register_t arg0 __unused,
				  u_register_t arg1 __unused,
				  u_register_t arg2 __unused,
				  u_register_t arg3 __unused)
{
#if LOG_LEVEL > 0
	/* Initialize the console to provide early debug support */
	plat_console_init();
#endif

	plat_early_init();

	/*
	 * SoC specific early init
	 * Any errata handling or SoC specific early initialization can
	 * be done here
	 * Set Counter Base Frequency in CNTFID0 and in cntfrq_el0.
	 * Initialize the interconnect.
	 * Enable coherency for primary CPU cluster
	 */
	soc_early_init();

	/*
	 * Initialize system level generic timer for Layerscape Socs.
	 */
	delay_timer_init();

	/* Initialise the IO layer and register platform IO devices */
	plat_io_setup();

#ifdef DDR_INIT
#ifndef DDR_LATE_INIT
	/* Initialize DDR */
#ifdef I2C_INIT
	i2c_init();
#endif
	dram_regions_info.total_dram_size = _init_ddr();
	if (dram_regions_info.total_dram_size < NXP_DRAM0_SIZE) {
		NOTICE("ERROR: DRAM0 Size is not correctly configured.");
		assert(0);
	}
	if (dram_regions_info.total_dram_size > 0)
		populate_dram_regions_info();
#endif
#endif

}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only initializes the mmu in a quick and dirty way.
 ******************************************************************************/
void plat_bl2_el3_plat_arch_setup(void)
{
	/* Initialise the IO layer and register platform IO devices */
	plat_setup_page_tables(
#if SEPARATE_RW_AND_NOLOAD
			      BL2_START,
			      BL2_LIMIT - BL2_START,
#else
			      BL2_BASE,
			      BL2_END - BL2_BASE,
#endif
			      BL_CODE_BASE,
			      BL_CODE_END,
			      BL_RO_DATA_BASE,
			      BL_RO_DATA_END
#if USE_COHERENT_MEM
			      , BL_COHERENT_RAM_BASE,
			      BL_COHERENT_RAM_END
#endif
			      );

#ifdef AARCH32
	enable_mmu_secure(0);
#else
	enable_mmu_el3(0);
#endif
}

void bl2_el3_plat_arch_setup(void)
{
	plat_bl2_el3_plat_arch_setup();
}

/*
 * Perform ARM standard platform setup.
 */
void plat_bl2_platform_setup(void)
{
}

void bl2_platform_setup(void)
{
	plat_bl2_platform_setup();
}

/*******************************************************************************
 * Gets SPSR for BL32 entry
 ******************************************************************************/
uint32_t plat_get_spsr_for_bl32_entry(void)
{
	/*
	 * The Secure Payload Dispatcher service is responsible for
	 * setting the SPSR prior to entry into the BL32 image.
	 */
	return 0;
}

/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
#ifndef AARCH32
uint32_t plat_get_spsr_for_bl33_entry(void)
{
	unsigned int mode;
	uint32_t spsr;

	/* Figure out what mode we enter the non-secure world in */
	mode = (el_implemented(2) != EL_IMPL_NONE) ? MODE_EL2 : MODE_EL1;

	/*
	 * TODO: Consider the possibility of specifying the SPSR in
	 * the FIP ToC and allowing the platform to have a say as
	 * well.
	 */
	spsr = SPSR_64(mode, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#else
/*******************************************************************************
 * Gets SPSR for BL33 entry
 ******************************************************************************/
uint32_t plat_get_spsr_for_bl33_entry(void)
{
	unsigned int hyp_status, mode, spsr;

	hyp_status = GET_VIRT_EXT(read_id_pfr1());

	mode = (hyp_status) ? MODE32_hyp : MODE32_svc;

	spsr = SPSR_MODE32(mode, plat_get_ns_image_entrypoint() & 0x1,
			SPSR_E_LITTLE, DISABLE_ALL_EXCEPTIONS);
	return spsr;
}
#endif /* AARCH32 */


int plat_bl2_handle_post_image_load(unsigned int image_id)
{
	int err = 0;
	bl_mem_params_node_t *bl_mem_params = get_bl_mem_params_node(image_id);

	assert(bl_mem_params);

	switch (image_id) {
	case BL31_IMAGE_ID:
		bl_mem_params->ep_info.args.arg3 =
					(u_register_t) &dram_regions_info;

		/* Pass the value of PORSR1 register in Argument 4 */
		bl_mem_params->ep_info.args.arg4 =
					(u_register_t)read_saved_porsr1();
		flush_dcache_range((uintptr_t)&dram_regions_info,
				sizeof(dram_regions_info));
		break;
#ifdef AARCH64
	case BL32_IMAGE_ID:
		bl_mem_params->ep_info.spsr = plat_get_spsr_for_bl32_entry();
		break;
#endif
	case BL33_IMAGE_ID:
		/* BL33 expects to receive the primary CPU MPID (through r0) */
		bl_mem_params->ep_info.args.arg0 = 0xffff & read_mpidr();
		bl_mem_params->ep_info.spsr = plat_get_spsr_for_bl33_entry();
		break;
	}

	return err;
}

/*******************************************************************************
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 ******************************************************************************/
int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return plat_bl2_handle_post_image_load(image_id);
}

void bl2_el3_plat_prepare_exit(void)
{
}

void bl2_plat_preload_setup(void)
{

#ifdef DDR_INIT
/* DDR is initialized late after MMU has been enabled */
#ifdef DDR_LATE_INIT
	/* Initialize DDR */
#ifdef I2C_INIT
	i2c_init();
#endif
	dram_regions_info.total_dram_size = _init_ddr();
	if (dram_regions_info.total_dram_size < NXP_DRAM0_SIZE) {
		NOTICE("ERROR: DRAM0 Size is not correctly configured.");
		assert(0);
	}
	if (dram_regions_info.total_dram_size > 0)
		populate_dram_regions_info();

	mmap_add_ddr_region_dynamically();
#endif
#endif

    /* setup the memory region access permissions */
	soc_mem_access();

}

