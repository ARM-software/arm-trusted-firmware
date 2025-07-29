/*
 * Copyright (c) 2015-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/arm/css/sds.h>
#include <lib/smccc.h>
#include <lib/utils_def.h>
#include <services/arm_arch_svc.h>

#include <plat/arm/common/plat_arm.h>
#include <platform_def.h>

/*
 * Table of memory regions for different BL stages to map using the MMU.
 * This doesn't include Trusted SRAM as setup_page_tables() already takes care
 * of mapping it.
 */
#ifdef IMAGE_BL1
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RW,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
#if TRUSTED_BOARD_BOOT
	/* Map DRAM to authenticate NS_BL2U image. */
	ARM_MAP_NS_DRAM1,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL2
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_FLASH0_RW,
#ifdef PLAT_ARM_MEM_PROT_ADDR
	ARM_V2M_MAP_MEM_PROTECT,
#endif
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	ARM_MAP_NS_DRAM1,
#ifdef __aarch64__
	ARM_MAP_DRAM2,
#endif
#ifdef SPD_tspd
	ARM_MAP_TSP_SEC_MEM,
#endif
#ifdef SPD_opteed
	ARM_MAP_OPTEE_CORE_MEM,
	ARM_OPTEE_PAGEABLE_LOAD_MEM,
#endif
#if TRUSTED_BOARD_BOOT && !RESET_TO_BL2
	ARM_MAP_BL1_RW,
#endif
#ifdef JUNO_ETHOSN_TZMP1
	JUNO_ETHOSN_PROT_FW_RW,
#endif
#if SPMC_AT_EL3
	ARM_SP_IMAGE_MMAP,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL2U
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	CSS_MAP_DEVICE,
	CSS_MAP_SCP_BL2U,
	V2M_MAP_IOFPGA,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif
#ifdef IMAGE_BL31
const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
#ifdef PLAT_ARM_MEM_PROT_ADDR
	ARM_V2M_MAP_MEM_PROTECT,
#endif
	SOC_CSS_MAP_DEVICE,
	ARM_DTB_DRAM_NS,
#ifdef JUNO_ETHOSN_TZMP1
	JUNO_ETHOSN_PROT_FW_RO,
#endif
	{0}
};
#endif
#ifdef IMAGE_BL32
const mmap_region_t plat_arm_mmap[] = {
#ifndef __aarch64__
	ARM_MAP_SHARED_RAM,
#ifdef PLAT_ARM_MEM_PROT_ADDR
	ARM_V2M_MAP_MEM_PROTECT,
#endif
#endif
	V2M_MAP_IOFPGA,
	CSS_MAP_DEVICE,
	SOC_CSS_MAP_DEVICE,
	{0}
};
#endif

ARM_CASSERT_MMAP

/*****************************************************************************
 * plat_is_smccc_feature_available() - This function checks whether SMCCC
 *                                     feature is availabile for platform.
 * @fid: SMCCC function id
 *
 * Return SMC_ARCH_CALL_SUCCESS if SMCCC feature is available and
 * SMC_ARCH_CALL_NOT_SUPPORTED otherwise.
 *****************************************************************************/
int32_t plat_is_smccc_feature_available(u_register_t fid)
{
	switch (fid) {
	case SMCCC_ARCH_SOC_ID:
		return SMC_ARCH_CALL_SUCCESS;
	default:
		return SMC_ARCH_CALL_NOT_SUPPORTED;
	}
}

/* Get SOC version */
int32_t plat_get_soc_version(void)
{
	return (int32_t)
		(SOC_ID_SET_JEP_106(ARM_SOC_CONTINUATION_CODE,
				    ARM_SOC_IDENTIFICATION_CODE) |
		 (JUNO_SOC_ID & SOC_ID_IMPL_DEF_MASK));
}

/* Get SOC revision */
int32_t plat_get_soc_revision(void)
{
	unsigned int sys_id;

	sys_id = mmio_read_32(V2M_SYSREGS_BASE + V2M_SYS_ID);
	return (int32_t)(((sys_id >> V2M_SYS_ID_REV_SHIFT) &
			  V2M_SYS_ID_REV_MASK) & SOC_ID_REV_MASK);
}

#if CSS_USE_SCMI_SDS_DRIVER
static sds_region_desc_t juno_sds_regions[] = {
	{ .base = PLAT_ARM_SDS_MEM_BASE },
};

sds_region_desc_t *plat_sds_get_regions(unsigned int *region_count)
{
	*region_count = ARRAY_SIZE(juno_sds_regions);

	return juno_sds_regions;
}
#endif /* CSS_USE_SCMI_SDS_DRIVER */
