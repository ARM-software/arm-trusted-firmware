/*
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/bl_common.h>

#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <platform_def.h>

/*
 * Table of regions to map using the MMU.
 * Replace or extend the below regions as required
 */

const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	ARM_MAP_NS_SHARED_RAM,
	ARM_MAP_NS_DRAM1,
	DIPHDA_MAP_DEVICE,
	DIPHDA_EXTERNAL_FLASH,
	{0}
};

/* diphda only has one always-on power domain and there
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
