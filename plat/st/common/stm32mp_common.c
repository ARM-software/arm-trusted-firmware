/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/clk.h>
#include <drivers/delay_timer.h>
#include <drivers/st/stm32_console.h>
#include <drivers/st/stm32mp_clkfunc.h>
#include <drivers/st/stm32mp_reset.h>
#include <lib/mmio.h>
#include <lib/smccc.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <services/arm_arch_svc.h>

#include <platform_def.h>

#define HEADER_VERSION_MAJOR_MASK	GENMASK(23, 16)
#define RESET_TIMEOUT_US_1MS		1000U

/* Internal layout of the 32bit OTP word board_id */
#define BOARD_ID_BOARD_NB_MASK		GENMASK_32(31, 16)
#define BOARD_ID_BOARD_NB_SHIFT		16
#define BOARD_ID_VARCPN_MASK		GENMASK_32(15, 12)
#define BOARD_ID_VARCPN_SHIFT		12
#define BOARD_ID_REVISION_MASK		GENMASK_32(11, 8)
#define BOARD_ID_REVISION_SHIFT		8
#define BOARD_ID_VARFG_MASK		GENMASK_32(7, 4)
#define BOARD_ID_VARFG_SHIFT		4
#define BOARD_ID_BOM_MASK		GENMASK_32(3, 0)

#define BOARD_ID2NB(_id)		(((_id) & BOARD_ID_BOARD_NB_MASK) >> \
					 BOARD_ID_BOARD_NB_SHIFT)
#define BOARD_ID2VARCPN(_id)		(((_id) & BOARD_ID_VARCPN_MASK) >> \
					 BOARD_ID_VARCPN_SHIFT)
#define BOARD_ID2REV(_id)		(((_id) & BOARD_ID_REVISION_MASK) >> \
					 BOARD_ID_REVISION_SHIFT)
#define BOARD_ID2VARFG(_id)		(((_id) & BOARD_ID_VARFG_MASK) >> \
					 BOARD_ID_VARFG_SHIFT)
#define BOARD_ID2BOM(_id)		((_id) & BOARD_ID_BOM_MASK)

#define BOOT_AUTH_MASK			GENMASK_32(23, 20)
#define BOOT_AUTH_SHIFT			20
#define BOOT_PART_MASK			GENMASK_32(19, 16)
#define BOOT_PART_SHIFT			16
#define BOOT_ITF_MASK			GENMASK_32(15, 12)
#define BOOT_ITF_SHIFT			12
#define BOOT_INST_MASK			GENMASK_32(11, 8)
#define BOOT_INST_SHIFT			8

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
#ifdef __aarch64__
	return (read_sctlr_el3() & c_m_bits) == c_m_bits;
#else
	return (read_sctlr() & c_m_bits) == c_m_bits;
#endif
}

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

int stm32_get_otp_index(const char *otp_name, uint32_t *otp_idx,
			uint32_t *otp_len)
{
	assert(otp_name != NULL);
	assert(otp_idx != NULL);

	return dt_find_otp_name(otp_name, otp_idx, otp_len);
}

int stm32_get_otp_value(const char *otp_name, uint32_t *otp_val)
{
	uint32_t otp_idx;

	assert(otp_name != NULL);
	assert(otp_val != NULL);

	if (stm32_get_otp_index(otp_name, &otp_idx, NULL) != 0) {
		return -1;
	}

	if (stm32_get_otp_value_from_idx(otp_idx, otp_val) != 0) {
		ERROR("BSEC: %s Read Error\n", otp_name);
		return -1;
	}

	return 0;
}

int stm32_get_otp_value_from_idx(const uint32_t otp_idx, uint32_t *otp_val)
{
	uint32_t ret = BSEC_NOT_SUPPORTED;

	assert(otp_val != NULL);

#if defined(IMAGE_BL2)
	ret = bsec_shadow_read_otp(otp_val, otp_idx);
#elif defined(IMAGE_BL32)
	ret = bsec_read_otp(otp_val, otp_idx);
#else
#error "Not supported"
#endif
	if (ret != BSEC_OK) {
		ERROR("BSEC: idx=%u Read Error\n", otp_idx);
		return -1;
	}

	return 0;
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

static void set_console(uintptr_t base, uint32_t clk_rate)
{
	unsigned int console_flags;

	if (console_stm32_register(base, clk_rate,
				   (uint32_t)STM32MP_UART_BAUDRATE, &console) == 0) {
		panic();
	}

	console_flags = CONSOLE_FLAG_BOOT | CONSOLE_FLAG_CRASH |
			CONSOLE_FLAG_TRANSLATE_CRLF;
#if !defined(IMAGE_BL2) && defined(DEBUG)
	console_flags |= CONSOLE_FLAG_RUNTIME;
#endif

	console_set_scope(&console, console_flags);
}

int stm32mp_uart_console_setup(void)
{
	struct dt_node_info dt_uart_info;
	uint32_t clk_rate = 0U;
	int result;
	uint32_t boot_itf __unused;
	uint32_t boot_instance __unused;

	result = dt_get_stdout_uart_info(&dt_uart_info);

	if ((result <= 0) ||
	    (dt_uart_info.status == DT_DISABLED)) {
		return -ENODEV;
	}

#if defined(IMAGE_BL2)
	if ((dt_uart_info.clock < 0) ||
	    (dt_uart_info.reset < 0)) {
		return -ENODEV;
	}
#endif

#if STM32MP_UART_PROGRAMMER || !defined(IMAGE_BL2)
	stm32_get_boot_interface(&boot_itf, &boot_instance);

	if ((boot_itf == BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_UART) &&
	    (get_uart_address(boot_instance) == dt_uart_info.base)) {
		return -EACCES;
	}
#endif

#if defined(IMAGE_BL2)
	if (dt_set_stdout_pinctrl() != 0) {
		return -ENODEV;
	}

	clk_enable((unsigned long)dt_uart_info.clock);

	reset_uart((uint32_t)dt_uart_info.reset);

	clk_rate = clk_get_rate((unsigned long)dt_uart_info.clock);
#endif

	set_console(dt_uart_info.base, clk_rate);

	return 0;
}

#if STM32MP_EARLY_CONSOLE
void stm32mp_setup_early_console(void)
{
#if defined(IMAGE_BL2) || STM32MP_RECONFIGURE_CONSOLE
	plat_crash_console_init();
#endif
	set_console(STM32MP_DEBUG_USART_BASE, STM32MP_DEBUG_USART_CLK_FRQ);
	NOTICE("Early console setup\n");
}
#endif /* STM32MP_EARLY_CONSOLE */

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

void stm32_display_board_info(uint32_t board_id)
{
	char rev[2];

	rev[0] = BOARD_ID2REV(board_id) - 1 + 'A';
	rev[1] = '\0';
	NOTICE("Board: MB%04x Var%u.%u Rev.%s-%02u\n",
	       BOARD_ID2NB(board_id),
	       BOARD_ID2VARCPN(board_id),
	       BOARD_ID2VARFG(board_id),
	       rev,
	       BOARD_ID2BOM(board_id));
}

void stm32_save_boot_info(boot_api_context_t *boot_context)
{
	uint32_t auth_status;

	assert(boot_context->boot_interface_instance <= (BOOT_INST_MASK >> BOOT_INST_SHIFT));
	assert(boot_context->boot_interface_selected <= (BOOT_ITF_MASK >> BOOT_ITF_SHIFT));
	assert(boot_context->boot_partition_used_toboot <= (BOOT_PART_MASK >> BOOT_PART_SHIFT));

	switch (boot_context->auth_status) {
	case BOOT_API_CTX_AUTH_NO:
		auth_status = 0x0U;
		break;

	case BOOT_API_CTX_AUTH_SUCCESS:
		auth_status = 0x2U;
		break;

	case BOOT_API_CTX_AUTH_FAILED:
	default:
		auth_status = 0x1U;
		break;
	}

	clk_enable(TAMP_BKP_REG_CLK);

	mmio_clrsetbits_32(stm32_get_bkpr_boot_mode_addr(),
			   BOOT_ITF_MASK | BOOT_INST_MASK | BOOT_PART_MASK | BOOT_AUTH_MASK,
			   (boot_context->boot_interface_instance << BOOT_INST_SHIFT) |
			   (boot_context->boot_interface_selected << BOOT_ITF_SHIFT) |
			   (boot_context->boot_partition_used_toboot << BOOT_PART_SHIFT) |
			   (auth_status << BOOT_AUTH_SHIFT));

	clk_disable(TAMP_BKP_REG_CLK);
}

void stm32_get_boot_interface(uint32_t *interface, uint32_t *instance)
{
	static uint32_t itf;

	if (itf == 0U) {
		clk_enable(TAMP_BKP_REG_CLK);

		itf = mmio_read_32(stm32_get_bkpr_boot_mode_addr()) &
		      (BOOT_ITF_MASK | BOOT_INST_MASK);

		clk_disable(TAMP_BKP_REG_CLK);
	}

	*interface = (itf & BOOT_ITF_MASK) >> BOOT_ITF_SHIFT;
	*instance = (itf & BOOT_INST_MASK) >> BOOT_INST_SHIFT;
}
