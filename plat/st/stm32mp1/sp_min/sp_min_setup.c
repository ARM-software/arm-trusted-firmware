/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <context.h>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/tzc400.h>
#include <drivers/generic_delay_timer.h>
#include <drivers/st/bsec.h>
#include <drivers/st/stm32_console.h>
#include <drivers/st/stm32_gpio.h>
#include <drivers/st/stm32mp1_clk.h>
#include <drivers/st/stm32mp1_reset.h>
#include <dt-bindings/clock/stm32mp1-clks.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>

#include <platform_sp_min.h>
#include <stm32mp1_dt.h>
#include <stm32mp1_private.h>

/******************************************************************************
 * Placeholder variables for copying the arguments that have been passed to
 * BL32 from BL2.
 ******************************************************************************/
static entry_point_info_t bl33_image_ep_info;

static struct console_stm32 console;

/*******************************************************************************
 * Interrupt handler for FIQ (secure IRQ)
 ******************************************************************************/
void sp_min_plat_fiq_handler(uint32_t id)
{
	switch (id & INT_ID_MASK) {
	case STM32MP1_IRQ_TZC400:
		ERROR("STM32MP1_IRQ_TZC400 generated\n");
		panic();
		break;
	case STM32MP1_IRQ_AXIERRIRQ:
		ERROR("STM32MP1_IRQ_AXIERRIRQ generated\n");
		panic();
		break;
	default:
		ERROR("SECURE IT handler not define for it : %u", id);
		break;
	}
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *sp_min_plat_get_bl33_ep_info(void)
{
	entry_point_info_t *next_image_info;

	next_image_info = &bl33_image_ep_info;

	if (next_image_info->pc == 0U) {
		return NULL;
	}

	return next_image_info;
}

/*******************************************************************************
 * Perform any BL32 specific platform actions.
 ******************************************************************************/
void sp_min_early_platform_setup2(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3)
{
	struct dt_node_info dt_uart_info;
	int result;
	bl_params_t *params_from_bl2 = (bl_params_t *)arg0;

	/* Imprecise aborts can be masked in NonSecure */
	write_scr(read_scr() | SCR_AW_BIT);

	assert(params_from_bl2 != NULL);
	assert(params_from_bl2->h.type == PARAM_BL_PARAMS);
	assert(params_from_bl2->h.version >= VERSION_2);

	bl_params_node_t *bl_params = params_from_bl2->head;

	/*
	 * Copy BL33 entry point information.
	 * They are stored in Secure RAM, in BL2's address space.
	 */
	while (bl_params != NULL) {
		if (bl_params->image_id == BL33_IMAGE_ID) {
			bl33_image_ep_info = *bl_params->ep_info;
			break;
		}

		bl_params = bl_params->next_params_info;
	}

	if (dt_open_and_check() < 0) {
		panic();
	}

	if (bsec_probe() != 0) {
		panic();
	}

	if (stm32mp1_clk_probe() < 0) {
		panic();
	}

	result = dt_get_stdout_uart_info(&dt_uart_info);

	if ((result > 0) && (dt_uart_info.status != 0U)) {
		if (console_stm32_register(dt_uart_info.base, 0,
					   STM32MP1_UART_BAUDRATE, &console) ==
		    0) {
			panic();
		}
	}
}

/*******************************************************************************
 * Initialize the MMU, security and the GIC.
 ******************************************************************************/
void sp_min_platform_setup(void)
{
	mmap_add_region(BL_CODE_BASE, BL_CODE_BASE,
			BL_CODE_END - BL_CODE_BASE,
			MT_CODE | MT_SECURE);

	configure_mmu();

	stm32mp1_reset_init();

	/* Initialize tzc400 after DDR initialization */
	stm32mp1_security_setup();

	generic_delay_timer_init();

	stm32mp1_gic_init();

	/* Unlock ETZPC securable peripherals */
#define STM32MP1_ETZPC_BASE	0x5C007000U
#define ETZPC_DECPROT0		0x010U
	mmio_write_32(STM32MP1_ETZPC_BASE + ETZPC_DECPROT0, 0xFFFFFFFF);

	/* Set GPIO bank Z as non secure */
	for (uint32_t pin = 0U; pin < STM32MP_GPIOZ_PIN_MAX_COUNT; pin++) {
		set_gpio_secure_cfg(GPIO_BANK_Z, pin, false);
	}
}

void sp_min_plat_arch_setup(void)
{
}
