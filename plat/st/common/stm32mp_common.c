/*
 * Copyright (c) 2015-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_console.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <services/arm_arch_svc.h>

#include <platform_def.h>

#define HEADER_VERSION_MAJOR_MASK	GENMASK(23, 16)
#define RESET_TIMEOUT_US_1MS		1000U

static console_t console;

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

#if  defined(IMAGE_BL2)
static void reset_uart(uint32_t reset)
{
	int ret;

	ret = stm32mp_reset_assert(reset, RESET_TIMEOUT_US_1MS);
	if (ret != 0) {
		panic();
	}

	udelay(2);

	ret = stm32mp_reset_deassert(reset, RESET_TIMEOUT_US_1MS);
	if (ret != 0) {
		panic();
	}

	mdelay(1);
}
#endif

int stm32mp_uart_console_setup(void)
{
	struct dt_node_info dt_uart_info;
	unsigned int console_flags;
	uint32_t clk_rate;
	int result;

	result = dt_get_stdout_uart_info(&dt_uart_info);

	if ((result <= 0) ||
	    (dt_uart_info.status == DT_DISABLED) ||
	    (dt_uart_info.clock < 0) ||
	    (dt_uart_info.reset < 0)) {
		return -ENODEV;
	}

#if defined(IMAGE_BL2)
	if (dt_set_stdout_pinctrl() != 0) {
		return -ENODEV;
	}
#endif

	stm32mp_clk_enable((unsigned long)dt_uart_info.clock);

#if defined(IMAGE_BL2)
	reset_uart((uint32_t)dt_uart_info.reset);
#endif

	clk_rate = stm32mp_clk_get_rate((unsigned long)dt_uart_info.clock);

	if (console_stm32_register(dt_uart_info.base, clk_rate,
				   STM32MP_UART_BAUDRATE, &console) == 0) {
		panic();
	}

	console_flags = CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH |
			CONSOLE_FLAG_TRANSLATE_CRLF;
#if !defined(IMAGE_BL2) && defined(DEBUG)
	console_flags |= CONSOLE_FLAG_RUNTIME;
#endif
	console_set_scope(&console, console_flags);

	return 0;
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
