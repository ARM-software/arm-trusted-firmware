
/*
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/el3_spmc_ffa_memory.h>

#include <plat/common/platform.h>
#include "qemu_private.h"

#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)

#ifdef DEVICE1_BASE
#define MAP_DEVICE1	MAP_REGION_FLAT(DEVICE1_BASE,			\
					DEVICE1_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)
#endif

#ifdef DEVICE2_BASE
#define MAP_DEVICE2	MAP_REGION_FLAT(DEVICE2_BASE,			\
					DEVICE2_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE)
#endif

#define MAP_SHARED_RAM	MAP_REGION_FLAT(SHARED_RAM_BASE,		\
					SHARED_RAM_SIZE,		\
					MT_DEVICE  | MT_RW | MT_SECURE)

#define MAP_BL32_MEM	MAP_REGION_FLAT(BL32_MEM_BASE, BL32_MEM_SIZE,	\
					MT_MEMORY | MT_RW | MT_SECURE)

#define MAP_NS_DRAM0	MAP_REGION_FLAT(NS_DRAM0_BASE, NS_DRAM0_SIZE,	\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_FLASH0	MAP_REGION_FLAT(QEMU_FLASH0_BASE, QEMU_FLASH0_SIZE, \
					MT_MEMORY | MT_RO | MT_SECURE)

#define MAP_FLASH1	MAP_REGION_FLAT(QEMU_FLASH1_BASE, QEMU_FLASH1_SIZE, \
					MT_MEMORY | MT_RO | MT_SECURE)

#ifdef FW_HANDOFF_BASE
#define MAP_FW_HANDOFF MAP_REGION_FLAT(FW_HANDOFF_BASE, FW_HANDOFF_SIZE, \
				       MT_MEMORY | MT_RW | MT_SECURE)
#endif
#ifdef FW_NS_HANDOFF_BASE
#define MAP_FW_NS_HANDOFF MAP_REGION_FLAT(FW_NS_HANDOFF_BASE, FW_HANDOFF_SIZE, \
					  MT_MEMORY | MT_RW | MT_NS)
#endif
/*
 * Table of regions for various BL stages to map using the MMU.
 * This doesn't include TZRAM as the 'mem_layout' argument passed to
 * arm_configure_mmu_elx() will give the available subset of that,
 */
#ifdef IMAGE_BL1
static const mmap_region_t plat_qemu_mmap[] = {
	MAP_FLASH0,
	MAP_FLASH1,
	MAP_SHARED_RAM,
	MAP_DEVICE0,
#ifdef MAP_DEVICE1
	MAP_DEVICE1,
#endif
#ifdef MAP_DEVICE2
	MAP_DEVICE2,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL2
static const mmap_region_t plat_qemu_mmap[] = {
	MAP_FLASH0,
	MAP_FLASH1,
	MAP_SHARED_RAM,
	MAP_DEVICE0,
#ifdef MAP_DEVICE1
	MAP_DEVICE1,
#endif
#ifdef MAP_DEVICE2
	MAP_DEVICE2,
#endif
	MAP_NS_DRAM0,
#if SPM_MM
	QEMU_SP_IMAGE_MMAP,
#else
	MAP_BL32_MEM,
#endif
#ifdef MAP_FW_HANDOFF
	MAP_FW_HANDOFF,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL31
static const mmap_region_t plat_qemu_mmap[] = {
	MAP_SHARED_RAM,
	MAP_DEVICE0,
#ifdef MAP_DEVICE1
	MAP_DEVICE1,
#endif
#ifdef MAP_DEVICE2
	MAP_DEVICE2,
#endif
#ifdef MAP_FW_HANDOFF
	MAP_FW_HANDOFF,
#endif
#ifdef MAP_FW_NS_HANDOFF
	MAP_FW_NS_HANDOFF,
#endif
#if SPM_MM
	MAP_NS_DRAM0,
	QEMU_SPM_BUF_EL3_MMAP,
#elif !SPMC_AT_EL3
	MAP_BL32_MEM,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL32
static const mmap_region_t plat_qemu_mmap[] = {
	MAP_SHARED_RAM,
	MAP_DEVICE0,
#ifdef MAP_DEVICE1
	MAP_DEVICE1,
#endif
#ifdef MAP_DEVICE2
	MAP_DEVICE2,
#endif
	{0}
};
#endif

/*******************************************************************************
 * Returns QEMU platform specific memory map regions.
 ******************************************************************************/
const mmap_region_t *plat_qemu_get_mmap(void)
{
	return plat_qemu_mmap;
}

#if MEASURED_BOOT || TRUSTED_BOARD_BOOT
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	return get_mbedtls_heap_helper(heap_addr, heap_size);
}
#endif

#if SPMC_AT_EL3
/*
 * When using the EL3 SPMC implementation allocate the datastore
 * for tracking shared memory descriptors in normal memory.
 */
#define PLAT_SPMC_SHMEM_DATASTORE_SIZE 64 * 1024

uint8_t plat_spmc_shmem_datastore[PLAT_SPMC_SHMEM_DATASTORE_SIZE];

int plat_spmc_shmem_datastore_get(uint8_t **datastore, size_t *size)
{
	*datastore = plat_spmc_shmem_datastore;
	*size = PLAT_SPMC_SHMEM_DATASTORE_SIZE;
	return 0;
}

int plat_spmc_shmem_begin(struct ffa_mtd *desc)
{
	return 0;
}

int plat_spmc_shmem_reclaim(struct ffa_mtd *desc)
{
	return 0;
}
#endif

#if defined(SPD_spmd) && (SPMC_AT_EL3 == 0)
/*
 * A dummy implementation of the platform handler for Group0 secure interrupt.
 */
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	(void)intid;
	return -1;
}
#endif /*defined(SPD_spmd) && (SPMC_AT_EL3 == 0)*/
