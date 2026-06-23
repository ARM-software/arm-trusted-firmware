/*
 * Copyright (c) 2025-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/arm/sbsa.h>
#include <lib/fconf/fconf.h>
#include <lib/fconf/fconf_dyn_cfg_getter.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/arm/common/smccc_def.h>
#include <plat/common/platform.h>
#include <services/arm_arch_svc.h>

#include <platform_def.h>

const mmap_region_t plat_arm_mmap[] = {
	ARM_MAP_SHARED_RAM,
	RDASPEN_MAP_DEVICE,
	RDASPEN_MAP_EXTERNAL_FLASH,
	RDASPEN_MAP_SMD_REGION,
#if PLAT_ARM_MEM_PROT_ADDR
	RDASPEN_MAP_MEM_PROTECT,
#endif
#ifdef IMAGE_BL2
	RDASPEN_MAP_NS_DRAM1,
	RDASPEN_MAP_NS_DRAM2,
	RDASPEN_MAP_TRUSTED_NVCTR,
#endif
#ifdef IMAGE_BL31
	RDASPEN_MAP_CPER_BUF,
#endif
	RDASPEN_MAP_S_DRAM,
	{0}
};

void plat_arm_security_setup(void)
{
}

unsigned int plat_get_syscnt_freq2(void)
{
	/* Returning the Generic Timer Frequency */
	return SYS_COUNTER_FREQ_IN_TICKS;
}

int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size)
{
	assert(heap_addr != NULL);
	assert(heap_size != NULL);

	return arm_get_mbedtls_heap(heap_addr, heap_size);
}

/*****************************************************************************
 * plat_is_smccc_feature_available() - This function checks whether SMCCC
 *                                     feature is available for platform.
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

/* Get Zena CSS version */
int32_t plat_get_soc_version(void)
{
	uint32_t soc_ver;

	soc_ver = mmio_read_32((RDASPEN_SYSTEM_ID_BASE + RDASPEN_SOC_ID_OFFSET));
	return (int32_t)
		(SOC_ID_SET_JEP_106(ARM_SOC_CONTINUATION_CODE, ARM_SOC_IDENTIFICATION_CODE) |
		 ((soc_ver >> RDASPEN_SOC_VER_SHIFT) & SOC_ID_IMPL_DEF_MASK));
}

/* Get Zena CSS revision */
int32_t plat_get_soc_revision(void)
{
	uint32_t soc_id;

	soc_id = mmio_read_32((RDASPEN_SYSTEM_ID_BASE + RDASPEN_SOC_ID_OFFSET));
	return (int32_t)((soc_id >> RDASPEN_SOC_REV_SHIFT) & GENMASK_32(3, 0));
}

/* Get Zena CSS name */
int32_t plat_get_soc_name(char *soc_name)
{
	snprintf(soc_name, SMCCC_SOC_NAME_LEN, "%s", ARM_RDASPEN_SOC_NAME);
	return SMC_ARCH_CALL_SUCCESS;
}
