/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <bl31/ehf.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <services/secure_partition.h>

static const mmap_region_t plat_arm_secure_partition_mmap[] = {
	PLAT_SQ_FLASH_MMAP,
	PLAT_SQ_UART1_MMAP,
	PLAT_SQ_PERIPH_MMAP,
	PLAT_SQ_SP_IMAGE_MMAP,
	PLAT_SP_IMAGE_NS_BUF_MMAP,
	PLAT_SQ_SP_IMAGE_RW_MMAP,
	PLAT_SPM_SPM_BUF_EL0_MMAP,
	{0}
};

/*
 * Boot information passed to a secure partition during initialisation. Linear
 * indices in MP information will be filled at runtime.
 */
static secure_partition_mp_info_t sp_mp_info[] = {
	{0x80000000, 0}, {0x80000001, 0}, {0x80000100, 0}, {0x80000101, 0},
	{0x80000200, 0}, {0x80000201, 0}, {0x80000300, 0}, {0x80000301, 0},
	{0x80000400, 0}, {0x80000401, 0}, {0x80000500, 0}, {0x80000501, 0},
	{0x80000600, 0}, {0x80000601, 0}, {0x80000700, 0}, {0x80000701, 0},
	{0x80000800, 0}, {0x80000801, 0}, {0x80000900, 0}, {0x80000901, 0},
	{0x80000a00, 0}, {0x80000a01, 0}, {0x80000b00, 0}, {0x80000b01, 0},
};

const secure_partition_boot_info_t plat_arm_secure_partition_boot_info = {
	.h.type			= PARAM_SP_IMAGE_BOOT_INFO,
	.h.version		= VERSION_1,
	.h.size			= sizeof(secure_partition_boot_info_t),
	.h.attr			= 0,
	.sp_mem_base		= BL32_BASE,
	.sp_mem_limit		= BL32_LIMIT,
	.sp_image_base		= BL32_BASE,
	.sp_stack_base		= PLAT_SP_IMAGE_STACK_BASE,
	.sp_heap_base		= PLAT_SQ_SP_HEAP_BASE,
	.sp_ns_comm_buf_base	= PLAT_SP_IMAGE_NS_BUF_BASE,
	.sp_shared_buf_base	= PLAT_SPM_BUF_BASE,
	.sp_image_size		= PLAT_SQ_SP_IMAGE_SIZE,
	.sp_pcpu_stack_size	= PLAT_SP_IMAGE_STACK_PCPU_SIZE,
	.sp_heap_size		= PLAT_SQ_SP_HEAP_SIZE,
	.sp_ns_comm_buf_size	= PLAT_SP_IMAGE_NS_BUF_SIZE,
	.sp_shared_buf_size	= PLAT_SPM_BUF_SIZE,
	.num_sp_mem_regions	= PLAT_SP_IMAGE_NUM_MEM_REGIONS,
	.num_cpus		= PLATFORM_CORE_COUNT,
	.mp_info		= sp_mp_info,
};

const struct mmap_region *plat_get_secure_partition_mmap(void *cookie)
{
	return plat_arm_secure_partition_mmap;
}

const struct secure_partition_boot_info *plat_get_secure_partition_boot_info(
		void *cookie)
{
	return &plat_arm_secure_partition_boot_info;
}

static ehf_pri_desc_t sq_exceptions[] = {
	EHF_PRI_DESC(PLAT_PRI_BITS, PLAT_SP_PRI),
};
EHF_REGISTER_PRIORITIES(sq_exceptions, ARRAY_SIZE(sq_exceptions), PLAT_PRI_BITS);
