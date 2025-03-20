
/*
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/el3_spmc_ffa_memory.h>
#if ENABLE_RME
#include <services/rmm_core_manifest.h>
#endif
#ifdef PLAT_qemu_sbsa
#include <sbsa_platform.h>
#endif

#include <plat/common/platform.h>
#include "qemu_private.h"

#define MAP_DEVICE0	MAP_REGION_FLAT(DEVICE0_BASE,			\
					DEVICE0_SIZE,			\
					MT_DEVICE | MT_RW | EL3_PAS)

#ifdef DEVICE1_BASE
#define MAP_DEVICE1	MAP_REGION_FLAT(DEVICE1_BASE,			\
					DEVICE1_SIZE,			\
					MT_DEVICE | MT_RW | EL3_PAS)
#endif

#ifdef DEVICE2_BASE
#define MAP_DEVICE2	MAP_REGION_FLAT(DEVICE2_BASE,			\
					DEVICE2_SIZE,			\
					MT_DEVICE | MT_RW | EL3_PAS)
#endif

#define MAP_SHARED_RAM	MAP_REGION_FLAT(SHARED_RAM_BASE,		\
					SHARED_RAM_SIZE,		\
					MT_DEVICE  | MT_RW | EL3_PAS)

#define MAP_BL32_MEM	MAP_REGION_FLAT(BL32_MEM_BASE, BL32_MEM_SIZE,	\
					MT_MEMORY | MT_RW | EL3_PAS)

#define MAP_NS_DRAM0	MAP_REGION_FLAT(NS_DRAM0_BASE, NS_DRAM0_SIZE,	\
					MT_MEMORY | MT_RW | MT_NS)

#define MAP_FLASH0	MAP_REGION_FLAT(QEMU_FLASH0_BASE, QEMU_FLASH0_SIZE, \
					MT_MEMORY | MT_RO | EL3_PAS)

#define MAP_FLASH1	MAP_REGION_FLAT(QEMU_FLASH1_BASE, QEMU_FLASH1_SIZE, \
					MT_MEMORY | MT_RO | EL3_PAS)

#ifdef FW_HANDOFF_BASE
#define MAP_FW_HANDOFF MAP_REGION_FLAT(FW_HANDOFF_BASE, FW_HANDOFF_SIZE, \
				       MT_MEMORY | MT_RW | EL3_PAS)
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

#ifdef IMAGE_RMM
const mmap_region_t plat_qemu_mmap[] = {
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

uint8_t plat_spmc_shmem_datastore[PLAT_SPMC_SHMEM_DATASTORE_SIZE] __aligned(2 * sizeof(long));

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

#if defined(SPD_spmd)
int plat_spmd_handle_group0_interrupt(uint32_t intid)
{
	/*
	 * Currently, there are no sources of Group0 secure interrupt
	 * enabled for QEMU.
	 */
	(void)intid;
	return -1;
}
#endif /*defined(SPD_spmd)*/

#if ENABLE_RME
/*
 * Get a pointer to the RMM-EL3 Shared buffer and return it
 * through the pointer passed as parameter.
 *
 * This function returns the size of the shared buffer.
 */
size_t plat_rmmd_get_el3_rmm_shared_mem(uintptr_t *shared)
{
	*shared = (uintptr_t)RMM_SHARED_BASE;

	return (size_t)RMM_SHARED_SIZE;
}

#ifdef PLAT_qemu
static uint32_t plat_get_num_memnodes(void)
{
	return 1;
}

static void plat_get_memory_node(int index, struct memory_bank *bank_ptr)
{
	(void) index;
	bank_ptr->base = NS_DRAM0_BASE;
	bank_ptr->size = NS_DRAM0_SIZE;
}
#elif PLAT_qemu_sbsa
static uint32_t plat_get_num_memnodes(void)
{
	return sbsa_platform_num_memnodes();
}

static void plat_get_memory_node(int index, struct memory_bank *bank_ptr)
{
	struct platform_memory_data data = {0, 0, 0};

	if (index < sbsa_platform_num_memnodes()) {
		data = sbsa_platform_memory_node(index);
	}

	bank_ptr->base = data.addr_base;
	bank_ptr->size = data.addr_size;
}
#endif /* PLAT_qemu */

/*
 * Calculate checksum of 64-bit words @buffer, of @size bytes
 */
static uint64_t checksum_calc(uint64_t *buffer, size_t size)
{
	uint64_t sum = 0UL;

	assert(((uintptr_t)buffer & (sizeof(uint64_t) - 1UL)) == 0UL);
	assert((size & (sizeof(uint64_t) - 1UL)) == 0UL);

	for (unsigned long i = 0UL; i < (size / sizeof(uint64_t)); i++) {
		sum += buffer[i];
	}

	return sum;
}

int plat_rmmd_load_manifest(struct rmm_manifest *manifest)
{
	int i, last;
	uint64_t checksum;
	size_t num_banks = plat_get_num_memnodes();
	size_t num_consoles = 1;
	struct memory_bank *bank_ptr;
	struct console_info *console_ptr;

	assert(manifest != NULL);

	manifest->version = RMMD_MANIFEST_VERSION;
	manifest->padding = 0U; /* RES0 */
	manifest->plat_data = (uintptr_t)NULL;
	manifest->plat_dram.num_banks = num_banks;
	manifest->plat_console.num_consoles = num_consoles;

	/*
	 * Boot manifest structure illustration:
	 *
	 * +----------------------------------------+
	 * |  offset  |   field      |  comment     |
	 * +----------+--------------+--------------+
	 * |    0     |  version     | 0x00000003   |
	 * +----------+--------------+--------------+
	 * |    4     |  padding     | 0x00000000   |
	 * +----------+--------------+--------------+
	 * |    8     | plat_data    |    NULL      |
	 * +----------+--------------+--------------+
	 * |    16    | num_banks    |              |
	 * +----------+--------------+              |
	 * |    24    |   banks      | plat_dram    |
	 * +----------+--------------+              |
	 * |    32    | checksum     |              |
	 * +----------+--------------+--------------+
	 * |    40    | num_consoles |              |
	 * +----------+--------------+              |
	 * |    48    | consoles     | plat_console |
	 * +----------+--------------+              |
	 * |    56    | checksum     |              |
	 * +----------+--------------+--------------+
	 * |    64    |  base 0      |              |
	 * +----------+--------------+   bank[0]    |
	 * |    72    |  size 0      |              |
	 * +----------+--------------+--------------+
	 * |    80    |  base        |              |
	 * +----------+--------------+              |
	 * |    88    |  map_pages   |              |
	 * +----------+--------------+              |
	 * |    96    |  name        |              |
	 * +----------+--------------+  consoles[0] |
	 * |   104    |  clk_in_hz   |              |
	 * +----------+--------------+              |
	 * |   112    |  baud_rate   |              |
	 * +----------+--------------+              |
	 * |   120    |  flags       |              |
	 * +----------+--------------+--------------+
	 */
	bank_ptr = (struct memory_bank *)
		(((uintptr_t)manifest) + sizeof(*manifest));

	console_ptr = (struct console_info *)
		((uintptr_t)bank_ptr + (num_banks * sizeof(*bank_ptr)));

	manifest->plat_dram.banks = bank_ptr;
	manifest->plat_console.consoles = console_ptr;

	/* Ensure the manifest is not larger than the shared buffer */
	assert((sizeof(struct rmm_manifest) +
		(sizeof(struct console_info) * num_consoles) +
		(sizeof(struct memory_bank) * num_banks)) <= RMM_SHARED_SIZE);

	/* Calculate checksum of plat_dram structure */
	checksum = num_banks + (uint64_t)bank_ptr;

	/*
	 * In the TF-A, NUMA nodes (if present) are stored in descending
	 * order, i.e:
	 *
	 * INFO:    RAM 0: node-id: 1, address: 0x10080000000 - 0x101ffffffff
	 * INFO:    RAM 1: node-id: 0, address: 0x10043000000 - 0x1007fffffff
	 *
	 * The RMM expects the memory banks to be presented in ascending order:
	 *
	 * INFO:    RAM 1: node-id: 0, address: 0x10043000000 - 0x1007fffffff
	 * INFO:    RAM 0: node-id: 1, address: 0x10080000000 - 0x101ffffffff
	 *
	 * As such, go through the NUMA nodes one by one and fill out
	 * @bank_ptr[] starting from the end.  When NUMA nodes are not present
	 * there is only one memory bank and none of the above matters.
	 */
	last = num_banks - 1;
	for (i = 0; i < num_banks; i++) {
		plat_get_memory_node(i, &bank_ptr[last]);
		last--;
	}

	checksum += checksum_calc((uint64_t *)bank_ptr,
				  num_banks * sizeof(*bank_ptr));

	/* Checksum must be 0 */
	manifest->plat_dram.checksum = ~checksum + 1UL;

	/* Calculate the checksum of the plat_consoles structure */
	checksum = num_consoles + (uint64_t)console_ptr;

	/* Zero out the console info struct */
	memset((void *)console_ptr, 0, sizeof(struct console_info) * num_consoles);

	console_ptr[0].map_pages = 1;
	console_ptr[0].base = PLAT_QEMU_BOOT_UART_BASE;
	console_ptr[0].clk_in_hz = PLAT_QEMU_BOOT_UART_CLK_IN_HZ;
	console_ptr[0].baud_rate = PLAT_QEMU_CONSOLE_BAUDRATE;

	strlcpy(console_ptr[0].name, "pl011", sizeof(console_ptr[0].name));

	/* Update checksum */
	checksum += checksum_calc((uint64_t *)console_ptr,
				  num_consoles * sizeof(*console_ptr));

	/* Checksum must be 0 */
	manifest->plat_console.checksum = ~checksum + 1UL;

	return 0;
}

/*
 * Update encryption key associated with @mecid.
 */
int plat_rmmd_mecid_key_update(uint16_t mecid)
{
	/*
	 * QEMU does not provide an interface to change the encryption key
	 * associated with MECID. Hence always return success.
	 */
	return 0;
}
#endif  /* ENABLE_RME */

/**
 * plat_qemu_dt_runtime_address() - Get the final DT location in RAM
 *
 * When support is enabled on SBSA, the device tree is relocated from its
 * original place at the beginning of the NS RAM to after the RMM.  This
 * function returns the address of the final location in RAM of the device
 * tree.  See function update_dt() in qemu_bl2_setup.c
 *
 * Return: The address of the final location in RAM of the device tree
 */
#if (ENABLE_RME && PLAT_qemu_sbsa)
void *plat_qemu_dt_runtime_address(void)
{
	return (void *)(uintptr_t)PLAT_QEMU_DT_BASE;
}
#else
void *plat_qemu_dt_runtime_address(void)
{
	return (void *)(uintptr_t)ARM_PRELOADED_DTB_BASE;
}
#endif /* (ENABLE_RME && PLAT_qemu_sbsa) */
