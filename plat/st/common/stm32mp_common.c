/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <services/arm_arch_svc.h>

#define HEADER_VERSION_MAJOR_MASK	GENMASK(23, 16)

uintptr_t plat_get_ns_image_entrypoint(void)
{
	return BL33_BASE;
}

unsigned int plat_get_syscnt_freq2(void)
{
	return read_cntfrq_el0();
}

static uintptr_t boot_ctx_address;
static uint16_t boot_itf_selected;

void stm32mp_save_boot_ctx_address(uintptr_t address)
{
	boot_api_context_t *boot_context = (boot_api_context_t *)address;

	boot_ctx_address = address;
	boot_itf_selected = boot_context->boot_interface_selected;
}

uintptr_t stm32mp_get_boot_ctx_address(void)
{
	return boot_ctx_address;
}

uint16_t stm32mp_get_boot_itf_selected(void)
{
	return boot_itf_selected;
}

uintptr_t stm32mp_ddrctrl_base(void)
{
	return DDRCTRL_BASE;
}

uintptr_t stm32mp_ddrphyc_base(void)
{
	return DDRPHYC_BASE;
}

uintptr_t stm32mp_pwr_base(void)
{
	return PWR_BASE;
}

uintptr_t stm32mp_rcc_base(void)
{
	return RCC_BASE;
}

bool stm32mp_lock_available(void)
{
	const uint32_t c_m_bits = SCTLR_M_BIT | SCTLR_C_BIT;

	/* The spinlocks are used only when MMU and data cache are enabled */
	return (read_sctlr() & c_m_bits) == c_m_bits;
}

#if STM32MP_USE_STM32IMAGE
int stm32mp_check_header(boot_api_image_header_t *header, uintptr_t buffer)
{
	uint32_t i;
	uint32_t img_checksum = 0U;

	/*
	 * Check header/payload validity:
	 *	- Header magic
	 *	- Header version
	 *	- Payload checksum
	 */
	if (header->magic != BOOT_API_IMAGE_HEADER_MAGIC_NB) {
		ERROR("Header magic\n");
		return -EINVAL;
	}

	if ((header->header_version & HEADER_VERSION_MAJOR_MASK) !=
	    (BOOT_API_HEADER_VERSION & HEADER_VERSION_MAJOR_MASK)) {
		ERROR("Header version\n");
		return -EINVAL;
	}

	for (i = 0U; i < header->image_length; i++) {
		img_checksum += *(uint8_t *)(buffer + i);
	}

	if (header->payload_checksum != img_checksum) {
		ERROR("Checksum: 0x%x (awaited: 0x%x)\n", img_checksum,
		      header->payload_checksum);
		return -EINVAL;
	}

	return 0;
}
#endif /* STM32MP_USE_STM32IMAGE */

int stm32mp_map_ddr_non_cacheable(void)
{
	return  mmap_add_dynamic_region(STM32MP_DDR_BASE, STM32MP_DDR_BASE,
					STM32MP_DDR_MAX_SIZE,
					MT_NON_CACHEABLE | MT_RW | MT_SECURE);
}

int stm32mp_unmap_ddr(void)
{
	return  mmap_remove_dynamic_region(STM32MP_DDR_BASE,
					   STM32MP_DDR_MAX_SIZE);
}

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
	uint32_t chip_id = stm32mp_get_chip_dev_id();
	uint32_t manfid = SOC_ID_SET_JEP_106(JEDEC_ST_BKID, JEDEC_ST_MFID);

	return (int32_t)(manfid | (chip_id & SOC_ID_IMPL_DEF_MASK));
}

/* Get SOC revision */
int32_t plat_get_soc_revision(void)
{
	return (int32_t)(stm32mp_get_chip_version() & SOC_ID_REV_MASK);
}
