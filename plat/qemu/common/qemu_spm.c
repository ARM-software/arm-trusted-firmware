/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2020, Linaro Limited and Contributors. All rights reserved.
 */

#include <libfdt.h>

#include <bl31/ehf.h>
#include <common/debug.h>
#include <common/fdt_fixup.h>
#include <common/fdt_wrappers.h>
#include <lib/xlat_tables/xlat_tables_compat.h>
#include <services/spm_mm_partition.h>

#include <platform_def.h>

/* Region equivalent to MAP_DEVICE1 suitable for mapping at EL0 */
#define MAP_DEVICE1_EL0	MAP_REGION_FLAT(DEVICE1_BASE,			\
					DEVICE1_SIZE,			\
					MT_DEVICE | MT_RW | MT_SECURE | MT_USER)

mmap_region_t plat_qemu_secure_partition_mmap[] = {
	QEMU_SP_IMAGE_NS_BUF_MMAP,	/* must be placed at first entry */
	MAP_DEVICE1_EL0,		/* for the UART */
	QEMU_SP_IMAGE_MMAP,
	QEMU_SPM_BUF_EL0_MMAP,
	QEMU_SP_IMAGE_RW_MMAP,
	MAP_SECURE_VARSTORE,
	{0}
};

/*
 * Boot information passed to a secure partition during initialisation.
 * Linear indices in MP information will be filled at runtime.
 */
static spm_mm_mp_info_t sp_mp_info[] = {
	[0] = {0x80000000, 0},
	[1] = {0x80000001, 0},
	[2] = {0x80000002, 0},
	[3] = {0x80000003, 0},
	[4] = {0x80000004, 0},
	[5] = {0x80000005, 0},
	[6] = {0x80000006, 0},
	[7] = {0x80000007, 0}
};

spm_mm_boot_info_t plat_qemu_secure_partition_boot_info = {
	.h.type              = PARAM_SP_IMAGE_BOOT_INFO,
	.h.version           = VERSION_1,
	.h.size              = sizeof(spm_mm_boot_info_t),
	.h.attr              = 0,
	.sp_mem_base         = PLAT_QEMU_SP_IMAGE_BASE,
	.sp_mem_limit        = BL32_LIMIT,
	.sp_image_base       = PLAT_QEMU_SP_IMAGE_BASE,
	.sp_stack_base       = PLAT_SP_IMAGE_STACK_BASE,
	.sp_heap_base        = PLAT_QEMU_SP_IMAGE_HEAP_BASE,
	.sp_ns_comm_buf_base = PLAT_QEMU_SP_IMAGE_NS_BUF_BASE,
	.sp_shared_buf_base  = PLAT_SPM_BUF_BASE,
	.sp_image_size       = PLAT_QEMU_SP_IMAGE_SIZE,
	.sp_pcpu_stack_size  = PLAT_SP_IMAGE_STACK_PCPU_SIZE,
	.sp_heap_size        = PLAT_QEMU_SP_IMAGE_HEAP_SIZE,
	.sp_ns_comm_buf_size = PLAT_QEMU_SP_IMAGE_NS_BUF_SIZE,
	.sp_shared_buf_size  = PLAT_SPM_BUF_SIZE,
	.num_sp_mem_regions  = PLAT_QEMU_SP_IMAGE_NUM_MEM_REGIONS,
	.num_cpus            = PLATFORM_CORE_COUNT,
	.mp_info             = sp_mp_info
};

/* Enumeration of priority levels on QEMU platforms. */
ehf_pri_desc_t qemu_exceptions[] = {
	EHF_PRI_DESC(QEMU_PRI_BITS, PLAT_SP_PRI)
};

int dt_add_ns_buf_node(uintptr_t *base)
{
	uintptr_t addr;
	size_t size;
	uintptr_t ns_buf_addr;
	int node;
	int err;
	void *fdt = (void *)ARM_PRELOADED_DTB_BASE;

	err = fdt_open_into(fdt, fdt, PLAT_QEMU_DT_MAX_SIZE);
	if (err < 0) {
		ERROR("Invalid Device Tree at %p: error %d\n", fdt, err);
		return err;
	}

	/*
	 * reserved-memory for standaloneMM non-secure buffer
	 * is allocated at the top of the first system memory region.
	 */
	node = fdt_path_offset(fdt, "/memory");

	err = fdt_get_reg_props_by_index(fdt, node, 0, &addr, &size);
	if (err < 0) {
		ERROR("Failed to get the memory node information\n");
		return err;
	}
	INFO("System RAM @ 0x%lx - 0x%lx\n", addr, addr + size - 1);

	ns_buf_addr = addr + (size - PLAT_QEMU_SP_IMAGE_NS_BUF_SIZE);
	INFO("reserved-memory for spm-mm @ 0x%lx - 0x%llx\n", ns_buf_addr,
	     ns_buf_addr + PLAT_QEMU_SP_IMAGE_NS_BUF_SIZE - 1);

	err = fdt_add_reserved_memory(fdt, "ns-buf-spm-mm", ns_buf_addr,
				      PLAT_QEMU_SP_IMAGE_NS_BUF_SIZE);
	if (err < 0) {
		ERROR("Failed to add the reserved-memory node\n");
		return err;
	}

	*base = ns_buf_addr;
	return 0;
}

/* Plug in QEMU exceptions to Exception Handling Framework. */
EHF_REGISTER_PRIORITIES(qemu_exceptions, ARRAY_SIZE(qemu_exceptions),
			QEMU_PRI_BITS);

const mmap_region_t *plat_get_secure_partition_mmap(void *cookie)
{
	uintptr_t ns_buf_base;

	dt_add_ns_buf_node(&ns_buf_base);

	plat_qemu_secure_partition_mmap[0].base_pa = ns_buf_base;
	plat_qemu_secure_partition_mmap[0].base_va = ns_buf_base;
	plat_qemu_secure_partition_boot_info.sp_ns_comm_buf_base = ns_buf_base;

	return plat_qemu_secure_partition_mmap;
}

const spm_mm_boot_info_t *
plat_get_secure_partition_boot_info(void *cookie)
{
	return &plat_qemu_secure_partition_boot_info;
}
