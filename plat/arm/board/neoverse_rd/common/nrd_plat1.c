/*
 * Copyright (c) 2018-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/ccn.h>
#include <drivers/arm/css/sds.h>
#include <lib/utils_def.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <drivers/arm/sbsa.h>

#if SPM_MM
#include <services/spm_mm_partition.h>
#endif

/*
 * Table of regions for different BL stages to map using the MMU.
 * This doesn't include Trusted RAM as the 'mem_layout' argument passed to
 * arm_configure_mmu_elx() will give the available subset of that.
 *
 * Replace or extend the below regions as required
 */
#if IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	NRD_MAP_FLASH0_RO,
	NRD_CSS_PERIPH_MMAP(0),
	NRD_ROS_PERIPH_MMAP(0),
	{0}
};
#endif
#if IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	NRD_MAP_FLASH0_RO,
#ifdef PLAT_ARM_MEM_PROT_ADDR
	ARM_V2M_MAP_MEM_PROTECT,
#endif
	NRD_CSS_PERIPH_MMAP(0),
	NRD_ROS_PERIPH_MMAP(0),
	ARM_MAP_NS_DRAM1,
#if NRD_CHIP_COUNT > 1
	NRD_CSS_PERIPH_MMAP(1),
#endif
#if NRD_CHIP_COUNT > 2
	NRD_CSS_PERIPH_MMAP(2),
#endif
#if NRD_CHIP_COUNT > 3
	NRD_CSS_PERIPH_MMAP(3),
#endif
#if ARM_BL31_IN_DRAM
	ARM_MAP_BL31_SEC_DRAM,
#endif
#if SPM_MM
	ARM_SP_IMAGE_MMAP,
#endif
#if TRUSTED_BOARD_BOOT && !RESET_TO_BL2
	ARM_MAP_BL1_RW,
#endif
	{0}
};
#endif
#if IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_IOFPGA,
	NRD_CSS_PERIPH_MMAP(0),
#ifdef PLAT_ARM_MEM_PROT_ADDR
	ARM_V2M_MAP_MEM_PROTECT,
#endif
	NRD_ROS_PERIPH_MMAP(0),
#if SPM_MM
	ARM_SPM_BUF_EL3_MMAP,
#endif
	{0}
};

#if SPM_MM && defined(IMAGE_BL31)
const mmap_region_t plat_arm_secure_partition_mmap[] = {
	NRD_ROS_SECURE_SYSTEMREG_USER_MMAP,
	NRD_ROS_SECURE_NOR2_USER_MMAP,
	NRD_CSS_SECURE_UART_MMAP,
	ARM_SP_IMAGE_MMAP,
	ARM_SP_IMAGE_NS_BUF_MMAP,
	ARM_SP_IMAGE_RW_MMAP,
	ARM_SPM_BUF_EL0_MMAP,
	{0}
};
#endif /* SPM_MM && defined(IMAGE_BL31) */
#endif

ARM_CASSERT_MMAP

#if SPM_MM && defined(IMAGE_BL31)
/*
 * Boot information passed to a secure partition during initialisation. Linear
 * indices in MP information will be filled at runtime.
 */
static spm_mm_mp_info_t sp_mp_info[] = {
	[0] = {0x81000000, 0},
	[1] = {0x81000100, 0},
	[2] = {0x81000200, 0},
	[3] = {0x81000300, 0},
	[4] = {0x81010000, 0},
	[5] = {0x81010100, 0},
	[6] = {0x81010200, 0},
	[7] = {0x81010300, 0},
};

const spm_mm_boot_info_t plat_arm_secure_partition_boot_info = {
	.h.type              = PARAM_SP_IMAGE_BOOT_INFO,
	.h.version           = VERSION_1,
	.h.size              = sizeof(spm_mm_boot_info_t),
	.h.attr              = 0,
	.sp_mem_base         = ARM_SP_IMAGE_BASE,
	.sp_mem_limit        = ARM_SP_IMAGE_LIMIT,
	.sp_image_base       = ARM_SP_IMAGE_BASE,
	.sp_stack_base       = PLAT_SP_IMAGE_STACK_BASE,
	.sp_heap_base        = ARM_SP_IMAGE_HEAP_BASE,
	.sp_ns_comm_buf_base = PLAT_SP_IMAGE_NS_BUF_BASE,
	.sp_shared_buf_base  = PLAT_SPM_BUF_BASE,
	.sp_image_size       = ARM_SP_IMAGE_SIZE,
	.sp_pcpu_stack_size  = PLAT_SP_IMAGE_STACK_PCPU_SIZE,
	.sp_heap_size        = ARM_SP_IMAGE_HEAP_SIZE,
	.sp_ns_comm_buf_size = PLAT_SP_IMAGE_NS_BUF_SIZE,
	.sp_shared_buf_size  = PLAT_SPM_BUF_SIZE,
	.num_sp_mem_regions  = ARM_SP_IMAGE_NUM_MEM_REGIONS,
	.num_cpus            = PLATFORM_CORE_COUNT,
	.mp_info             = &sp_mp_info[0],
};

const struct mmap_region *plat_get_secure_partition_mmap(void *cookie)
{
	return plat_arm_secure_partition_mmap;
}

const struct spm_mm_boot_info *plat_get_secure_partition_boot_info(
		void *cookie)
{
	return &plat_arm_secure_partition_boot_info;
}
#endif /* SPM_MM && defined(IMAGE_BL31) */

#if TRUSTED_BOARD_BOOT
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	assert(heap_addr != NULL);
	assert(heap_size != NULL);

	return arm_get_mbedtls_heap(heap_addr, heap_size);
}
#endif

void plat_arm_secure_wdt_start(void)
{
	sbsa_wdog_start(NRD_CSS_WDOG_BASE, NRD_CSS_WDOG_TIMEOUT);
}

void plat_arm_secure_wdt_stop(void)
{
	sbsa_wdog_stop(NRD_CSS_WDOG_BASE);
}

static sds_region_desc_t nrd_sds_regions[] = {
	{ .base = PLAT_ARM_SDS_MEM_BASE },
};

sds_region_desc_t *plat_sds_get_regions(unsigned int *region_count)
{
	*region_count = ARRAY_SIZE(nrd_sds_regions);

	return nrd_sds_regions;
}
