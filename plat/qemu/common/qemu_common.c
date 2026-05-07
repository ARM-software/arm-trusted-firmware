
/*
 * Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/el3_spmc_ffa_memory.h>
#if ENABLE_RMM
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

#if ENABLE_RMM
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

const struct memory_bank ncoh_region_data[] = {};
const struct memory_bank coh_region_data[] = {};
const struct root_complex_info rc_data[] = {};

/* Number of device non-coherent address ranges */
#define QEMU_RMM_NCOH_REGIONS	ARRAY_SIZE(ncoh_region_data)
/* Number of device coherent address ranges */
#define QEMU_RMM_COH_REGIONS	ARRAY_SIZE(coh_region_data)
/* Number of SMMUs */
#define QEMU_RMM_SMMU_COUNT	0
/* Number of PCIe Root Complexes */
#define QEMU_RMM_RC_COUNT	ARRAY_SIZE(rc_data)

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

struct memory_bank ncoh_region_data[] = {
	/* SBSA SBSA_PCIE_MMIO region */
	{SBSA_PCIE_MMIO_BASE,
	 SBSA_PCIE_MMIO_SIZE
	},
	/*
	 * SBSA SBSA_PCIE_MMIO_HIGH region.  The architecture supports up
	 * to 1020GB but 10GB should be plenty.
	 */
	{SBSA_PCIE_MMIO_HIGH_BASE,
	 SBSA_PCIE_MMIO_HIGH_SIZE
	}
};

const struct memory_bank coh_region_data[] = {};

/* BDF mappings for RP0 RC0 */
const struct bdf_mapping_info rc0rp0_bdf_data[] = {
	/* BDF0 */
	{0U,		/* mapping_base */
	 0x8000U,	/* mapping_top */
	 0U,		/* mapping_off */
	 0U		/* smmu_idx */
	}
};

/* Root ports for RC0 */
const struct root_port_info rc0rp_data[] = {
	/* RP0 */
	{0U,						/* root_port_id */
	 0U,						/* padding */
	 ARRAY_SIZE(rc0rp0_bdf_data),			/* num_bdf_mappings */
	 (struct bdf_mapping_info *)rc0rp0_bdf_data	/* bdf_mappings */
	}
};

/* See @sbsa_ref_memmap in QEMU's hw/arm/sbsa-ref.c */
#define PCIE_EXP_BASE	0xf0000000
/* Root complexes */
const struct root_complex_info rc_data[] = {
	/* RC0 */
	{PCIE_EXP_BASE,				/* ecam_base */
	 0U,					/* segment */
	 {0U, 0U, 0U},				/* padding */
	 ARRAY_SIZE(rc0rp_data),		/* num_root_ports */
	 (struct root_port_info *)rc0rp_data	/* root_ports */
	}
};

/* Number of device non-coherent address ranges */
#define QEMU_RMM_NCOH_REGIONS	ARRAY_SIZE(ncoh_region_data)
/* Number of device non-coherent address ranges */
#define QEMU_RMM_COH_REGIONS	ARRAY_SIZE(coh_region_data)
/* Number of SMMUs */
#define QEMU_RMM_SMMU_COUNT	0
/* Number of PCIe Root Complexes */
#define QEMU_RMM_RC_COUNT	ARRAY_SIZE(rc_data)

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

static void set_memory_region(struct memory_info *region_ptr,
			      struct memory_bank *bank_ptr,
			      const struct memory_bank *bank_data,
			      uint64_t num_bank_data)
{
	struct memory_bank *dst;
	uint64_t checksum, i;

	if (!region_ptr || !bank_ptr || !bank_data)
		return;

	if (!num_bank_data) {
		region_ptr->num_banks = 0UL;
		region_ptr->banks = NULL;
		region_ptr->checksum = 0UL;
		return;
	}


	/* Calculate the checksum of address range info structure */
	checksum = num_bank_data + (uint64_t)bank_ptr;

	/* Zero out the PCIe region info struct */
	memset((void *)bank_ptr, 0,
	       sizeof(struct memory_bank) * num_bank_data);

	dst = bank_ptr;

	for (i = 0; i < num_bank_data; i++) {
		memcpy((void *)dst, (void *)(bank_data + i),
		       sizeof(struct memory_bank));
		dst++;
	}

	/* Update checksum */
	checksum += checksum_calc((uint64_t *)bank_ptr,
				  sizeof(struct memory_bank) * num_bank_data);

	/* Checksum must be 0 */
	region_ptr->checksum = ~checksum + 1UL;
	region_ptr->num_banks = num_bank_data;
	region_ptr->banks = bank_ptr;
}

static void set_root_complex(struct root_complex_list *plat_rc_ptr,
			     struct root_complex_info *root_complex_ptr,
			     struct root_port_info *root_port_ptr,
			     struct bdf_mapping_info *bdf_mapping_ptr,
			     uint64_t num_root_complex)
{
	struct root_complex_info *rc_ptr;
	struct bdf_mapping_info *bdf_ptr;
	struct root_port_info *rp_ptr;
	uint64_t num_root_ports, num_bdf_mappings;
	uint64_t checksum, i, j;

	if (!plat_rc_ptr || !root_complex_ptr ||
	    !root_port_ptr || !bdf_mapping_ptr)
		return;

	if (!num_root_complex) {
		plat_rc_ptr->num_root_complex = 0UL;
		plat_rc_ptr->rc_info_version = 0UL;
		plat_rc_ptr->root_complex = NULL;
		plat_rc_ptr->padding = 0U;
		plat_rc_ptr->checksum = 0UL;
	}

	/* Set pointers for data in manifest */
	rc_ptr = root_complex_ptr;
	rp_ptr = root_port_ptr;
	bdf_ptr = bdf_mapping_ptr;

	/* Calculate the checksum of the plat_root_complex structure */
	checksum = num_root_complex + (uint64_t)root_complex_ptr;

	num_root_ports = 0U;
	num_bdf_mappings = 0U;

	for (i = 0; i < num_root_complex; i++) {
		const struct root_complex_info *rc_info = &rc_data[i];
		const struct root_port_info *rp_info = rc_info->root_ports;

		num_root_ports += rc_data[i].num_root_ports;

		/* Copy root complex data, except root_ports pointer */
		memcpy((void *)rc_ptr, (void *)rc_info,
			sizeof(struct root_complex_info) -
			sizeof(struct root_port_info *));

		/* Set root_ports for root complex */
		rc_ptr->root_ports = rp_ptr;

		/* Scan root ports */
		for (j = 0; j < rc_ptr->num_root_ports; j++) {
			const struct bdf_mapping_info *bdf_info;

			num_bdf_mappings += rc_data[i].root_ports[j].num_bdf_mappings;

			bdf_info = rp_info->bdf_mappings;

			/* Copy root port data, except bdf_mappings pointer */
			memcpy((void *)rp_ptr, (void *)rp_info,
			       sizeof(struct root_port_info) -
			       sizeof(struct bdf_mapping_info *));

			/* Set bdf_mappings for root port */
			rp_ptr->bdf_mappings = bdf_ptr;

			/* Copy all BDF mappings for root port */
			memcpy((void *)bdf_ptr, (void *)bdf_info,
			       sizeof(struct bdf_mapping_info) *
			       rp_ptr->num_bdf_mappings);

			bdf_ptr += rp_ptr->num_bdf_mappings;
			rp_ptr++;
			rp_info++;
		}
		rc_ptr++;
	}

	/* Check that all data are written in manifest */
	assert(rc_ptr == (root_complex_ptr + num_root_complex));
	assert(rp_ptr == (root_port_ptr + num_root_ports));
	assert(bdf_ptr == (bdf_mapping_ptr + num_bdf_mappings));

	/* Update checksum for all PCIe data */
	checksum += checksum_calc((uint64_t *)root_complex_ptr,
				  (uintptr_t)bdf_ptr - (uintptr_t)root_complex_ptr);

	plat_rc_ptr->num_root_complex = num_root_complex;
	plat_rc_ptr->rc_info_version = PCIE_RC_INFO_VERSION;
	plat_rc_ptr->root_complex = root_complex_ptr;
	plat_rc_ptr->padding = 0U; /* RES0 */
	/* Checksum must be 0 */
	plat_rc_ptr->checksum = ~checksum + 1UL;
}

/*
 * Boot manifest structure illustration:
 *
 * +--------------------------------------------------+
 * | offset |        field       |      comment       |
 * +--------+--------------------+--------------------+
 * |   0    |       version      |     0x00000005     |
 * +--------+--------------------+--------------------+
 * |   4    |       padding      |     0x00000000     |
 * +--------+--------------------+--------------------+
 * |   8    |      plat_data     |       NULL         |
 * +--------+--------------------+--------------------+
 * |   16   |      num_banks     |                    |
 * +--------+--------------------+                    |
 * |   24   |       banks        |     plat_dram      +--+
 * +--------+--------------------+                    |  |
 * |   32   |      checksum      |                    |  |
 * +--------+--------------------+--------------------+  |
 * |   40   |    num_consoles    |                    |  |
 * +--------+--------------------+                    |  |
 * |   48   |      consoles      |    plat_console    +--|--+
 * +--------+--------------------+                    |  |  |
 * |   56   |      checksum      |                    |  |  |
 * +--------+--------------------+--------------------+  |  |
 * |   64   |      num_banks     |                    |  |  |
 * +--------+--------------------+                    |  |  |
 * |   72   |        banks       |  plat_ncoh_region  +--|--|--+
 * +--------+--------------------+                    |  |  |  |
 * |   80   |      checksum      |                    |  |  |  |
 * +--------+--------------------+--------------------+  |  |  |
 * |   88   |      num_banks     |                    |  |  |  |
 * +--------+--------------------+                    |  |  |  |
 * |   96   |       banks        |   plat_coh_region  |  |  |  |
 * +--------+--------------------+                    |  |  |  |
 * |   104  |      checksum      |                    |  |  |  |
 * +--------+--------------------+--------------------+  |  |  |
 * |   112  |     num_smmus      |                    |  |  |  |
 * +--------+--------------------+                    |  |  |  |
 * |   120  |       smmus        |     plat_smmu      +--|--|--|--+
 * +--------+--------------------+                    |  |  |  |  |
 * |   128  |      checksum      |                    |  |  |  |  |
 * +--------+--------------------+--------------------+  |  |  |  |
 * |   136  |  num_root_complex  |                    |  |  |  |  |
 * +--------+--------------------+                    |  |  |  |  |
 * |   144  |   rc_info_version  |                    |  |  |  |  |
 * +--------+--------------------+                    |  |  |  |  |
 * |   148  |      padding       | plat_root_complex  +--|--|--|--|--+
 * +--------+--------------------+                    |  |  |  |  |  |
 * |   152  |    root_complex    |                    |  |  |  |  |  |
 * +--------+--------------------+                    |  |  |  |  |  |
 * |   160  |      checksum      |                    |  |  |  |  |  |
 * +--------+--------------------+--------------------+<-+  |  |  |  |
 * |   168  |       base 0       |                    |     |  |  |  |
 * +--------+--------------------+     mem_bank[0]    |     |  |  |  |
 * |   176  |       size 0       |                    |     |  |  |  |
 * +--------+--------------------+--------------------+<----+  |  |  |
 * |   184  |       base         |                    |        |  |  |
 * +--------+--------------------+                    |        |  |  |
 * |   192  |      map_pages     |                    |        |  |  |
 * +--------+--------------------+                    |        |  |  |
 * |   200  |       name         |                    |        |  |  |
 * +--------+--------------------+     consoles[0]    |        |  |  |
 * |   208  |     clk_in_hz      |                    |        |  |  |
 * +--------+--------------------+                    |        |  |  |
 * |   216  |     baud_rate      |                    |        |  |  |
 * +--------+--------------------+                    |        |  |  |
 * |   224  |       flags        |                    |        |  |  |
 * +--------+--------------------+--------------------+<-------+  |  |
 * |   232  |       base 0       |                    |           |  |
 * +--------+--------------------+    ncoh_region[0]  |           |  |
 * |   240  |       size 0       |                    |           |  |
 * +--------+--------------------+--------------------+           |  |
 * |   248  |       base 1       |                    |           |  |
 * +--------+--------------------+    ncoh_region[1]  |           |  |
 * |   256  |       size 1       |                    |           |  |
 * +--------+--------------------+--------------------+<----------+  |
 * |   264  |     smmu_base      |                    |              |
 * +--------+--------------------+      smmus[0]      |              |
 * |   272  |     smmu_r_base    |                    |              |
 * +--------+--------------------+--------------------+<-------------+
 * |   280  |     ecam_base      |                    |
 * +--------+--------------------+                    |
 * |   288  |      segment       |                    |
 * +--------+--------------------+                    |
 * |   289  |      padding       |   root_complex[0]  +--+
 * +--------+--------------------+                    |  |
 * |   292  |   num_root_ports   |                    |  |
 * +--------+--------------------+                    |  |
 * |   296  |     root_ports     |                    |  |
 * +--------+--------------------+--------------------+<-+
 * |   304  |    root_port_id    |                    |
 * +--------+--------------------+                    |
 * |   306  |      padding       |                    |
 * +--------+--------------------+   root_ports[0]    +--+
 * |   308  |  num_bdf_mappings  |                    |  |
 * +--------+--------------------+                    |  |
 * |   312  |    bdf_mappings    |                    |  |
 * +--------+--------------------+--------------------+<-+
 * |   320  |    mapping_base    |                    |
 * +--------+--------------------+                    |
 * |   322  |    mapping_top     |                    |
 * +--------+--------------------+   bdf_mappings[0]  |
 * |   324  |    mapping_off     |                    |
 * +--------+--------------------+                    |
 * |   326  |     smmu_idx       |                    |
 * +--------+--------------------+--------------------+
 */

int plat_rmmd_load_manifest(struct rmm_manifest *manifest)
{
	int i, j, last;
	uint64_t checksum;
	uint64_t num_ncoh_regions, num_coh_regions;
	uint64_t num_smmus, num_root_complex;
	uint64_t num_root_ports, num_bdf_mappings;
	size_t num_banks = plat_get_num_memnodes();
	size_t num_consoles = 1;
	struct memory_bank *bank_ptr;
	struct memory_bank *ncoh_region_ptr, *coh_region_ptr;
	struct console_info *console_ptr;
	struct smmu_info *smmu_ptr;
	struct root_complex_info *root_complex_ptr;
	struct root_port_info *root_port_ptr;
	struct bdf_mapping_info *bdf_mapping_ptr;

	assert(manifest != NULL);

	num_ncoh_regions = QEMU_RMM_NCOH_REGIONS;
	num_coh_regions = QEMU_RMM_COH_REGIONS;
	num_smmus = QEMU_RMM_SMMU_COUNT;
	num_root_complex = QEMU_RMM_RC_COUNT;

	/* Calculate and set number of all PCIe root ports and BDF mappings */
	num_root_ports = 0U;
	num_bdf_mappings = 0U;

	/* Scan all root complex entries */
	for (i = 0UL; i < num_root_complex; i++) {
		num_root_ports += rc_data[i].num_root_ports;
		/* Scan all root ports entries in root complex */
		for (j = 0U; j < rc_data[i].num_root_ports; j++) {
			num_bdf_mappings += rc_data[i].root_ports[j].num_bdf_mappings;
		}
	}

	manifest->version = RMMD_MANIFEST_VERSION;
	manifest->padding = 0U; /* RES0 */
	manifest->plat_data = (uintptr_t)NULL;
	manifest->plat_dram.num_banks = num_banks;
	manifest->plat_console.num_consoles = num_consoles;

	bank_ptr = (struct memory_bank *)
		(((uintptr_t)manifest) + sizeof(*manifest));

	console_ptr = (struct console_info *)
		((uintptr_t)bank_ptr + (num_banks * sizeof(*bank_ptr)));

	ncoh_region_ptr = (struct memory_bank *)
			((uintptr_t)console_ptr + (num_consoles *
						sizeof(struct console_info)));
	coh_region_ptr = (struct memory_bank *)
			((uintptr_t)ncoh_region_ptr + (num_ncoh_regions *
						sizeof(struct memory_bank)));
	smmu_ptr = (struct smmu_info *)
			((uintptr_t)coh_region_ptr + (num_coh_regions *
						sizeof(struct memory_bank)));
	root_complex_ptr = (struct root_complex_info *)
			((uintptr_t)smmu_ptr + (num_smmus *
						sizeof(struct smmu_info)));
	root_port_ptr = (struct	root_port_info *)
			((uintptr_t)root_complex_ptr + (num_root_complex *
						sizeof(struct root_complex_info)));
	bdf_mapping_ptr = (struct bdf_mapping_info *)
			((uintptr_t)root_port_ptr + (num_root_ports *
						sizeof(struct root_port_info)));

	/* Currently supported */
	manifest->plat_dram.banks = bank_ptr;
	manifest->plat_console.consoles = console_ptr;

	/* Currently not supported */
	manifest->plat_coh_region.num_banks = num_coh_regions;
	manifest->plat_coh_region.banks = NULL;
	manifest->plat_coh_region.checksum = 0UL;

	manifest->plat_smmu.num_smmus = num_smmus;
	manifest->plat_smmu.smmus = NULL;
	manifest->plat_smmu.checksum = 0UL;

	/* Ensure the manifest is not larger than the shared buffer */
	assert((sizeof(struct rmm_manifest) +
		(sizeof(struct console_info) * num_consoles) +
		(sizeof(struct memory_bank) * num_banks) +
		(sizeof(struct memory_bank) * num_ncoh_regions) +
		(sizeof(struct memory_bank) * num_coh_regions) +
		(sizeof(struct memory_bank) * num_coh_regions) +
		(sizeof(struct smmu_info) * num_smmus) +
		(sizeof(struct root_complex_info) * num_root_complex) +
		(sizeof(struct root_port_info) * num_root_ports) +
		(sizeof(struct bdf_mapping_info) * num_bdf_mappings))
		<= RMM_SHARED_SIZE);

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

	/*
	 * Calculate the checksum of device non-coherent address ranges
	 */
	set_memory_region(&manifest->plat_ncoh_region, ncoh_region_ptr,
			  ncoh_region_data, num_ncoh_regions);
	/*
	 * Calculate the checksum of device coherent address ranges
	 */
	set_memory_region(&manifest->plat_coh_region, coh_region_ptr,
			  coh_region_data, num_coh_regions);

	/* Calculate the checksum of the plat_root_complex structure */
	set_root_complex(&manifest->plat_root_complex, root_complex_ptr,
			 root_port_ptr, bdf_mapping_ptr, num_root_complex);

	return 0;
}

/*
 * Update encryption key associated with @mecid.
 */
int plat_rmmd_mecid_key_update(uint16_t mecid, unsigned int reason)
{
	/*
	 * QEMU does not provide an interface to change the encryption key
	 * associated with MECID. Hence always return success.
	 */
	return 0;
}
#endif  /* ENABLE_RMM */

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
#if (ENABLE_RMM && PLAT_qemu_sbsa)
void *plat_qemu_dt_runtime_address(void)
{
	return (void *)(uintptr_t)PLAT_QEMU_DT_BASE;
}
#else
void *plat_qemu_dt_runtime_address(void)
{
	return (void *)(uintptr_t)ARM_PRELOADED_DTB_BASE;
}
#endif /* (ENABLE_RMM && PLAT_qemu_sbsa) */
