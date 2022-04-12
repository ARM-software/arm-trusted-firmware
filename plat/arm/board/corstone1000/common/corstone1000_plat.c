/*
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>

#include <drivers/generic_delay_timer.h>
#include <drivers/io/io_storage.h>
#include <plat/common/platform.h>
#include <plat/arm/common/arm_fconf_getter.h>
#include <plat/arm/common/arm_fconf_io_storage.h>
#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/*
 * Table of regions to map using the MMU.
 * Replace or extend the below regions as required
 */

const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	ARM_MAP_NS_SHARED_RAM,
	ARM_MAP_NS_DRAM1,
	CORSTONE1000_MAP_DEVICE,
	CORSTONE1000_EXTERNAL_FLASH,
	{0}
};

static void set_fip_image_source(void)
{
	const struct plat_io_policy *policy;
	/*
	 * metadata for firmware update is written at 0x0000 offset of the flash.
	 * PLAT_ARM_BOOT_BANK_FLAG contains the boot bank that TF-M is booted.
	 * As per firmware update spec, at a given point of time, only one bank
	 * is active. This means, TF-A should boot from the same bank as TF-M.
	 */
	volatile uint32_t *boot_bank_flag = (uint32_t *)(PLAT_ARM_BOOT_BANK_FLAG);

	if (*boot_bank_flag > 1) {
		VERBOSE("Boot_bank is set higher than possible values");
	}

	VERBOSE("Boot bank flag = %u.\n\r", *boot_bank_flag);

	policy = FCONF_GET_PROPERTY(arm, io_policies, FIP_IMAGE_ID);

	assert(policy != NULL);
	assert(policy->image_spec != 0UL);

	io_block_spec_t *spec = (io_block_spec_t *)policy->image_spec;

	if ((*boot_bank_flag) == 0) {
		VERBOSE("Booting from bank 0: fip offset = 0x%lx\n\r",
						PLAT_ARM_FIP_BASE_BANK0);
		spec->offset = PLAT_ARM_FIP_BASE_BANK0;
	} else {
		VERBOSE("Booting from bank 1: fip offset = 0x%lx\n\r",
						PLAT_ARM_FIP_BASE_BANK1);
		spec->offset = PLAT_ARM_FIP_BASE_BANK1;
	}
}

void bl2_platform_setup(void)
{
	arm_bl2_platform_setup();
	/*
	 * Identify the start address of the FIP by reading the boot
	 * index flag from the flash.
	 */
	set_fip_image_source();
}

/* corstone1000 only has one always-on power domain and there
 * is no power control present
 */
void __init plat_arm_pwrc_setup(void)
{
}

unsigned int plat_get_syscnt_freq2(void)
{
	/* Returning the Generic Timer Frequency */
	return SYS_COUNTER_FREQ_IN_TICKS;
}


/*
 * Helper function to initialize ARM interconnect driver.
 */
void plat_arm_interconnect_init(void)
{
}

/*
 * Helper function to place current master into coherency
 */
void plat_arm_interconnect_enter_coherency(void)
{
}

/*
 * Helper function to remove current master from coherency
 */
void plat_arm_interconnect_exit_coherency(void)
{
}

/*
 * This function is invoked during Mbed TLS library initialisation to get a heap
 * The function simply returns the default allocated heap.
 */

#if TRUSTED_BOARD_BOOT
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	assert(heap_addr != NULL);
	assert(heap_size != NULL);

	return arm_get_mbedtls_heap(heap_addr, heap_size);
}
#endif
