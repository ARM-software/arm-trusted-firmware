/*
 * Copyright (c) 2023, Pengutronix. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <platform_def.h>

#define UCR1    		0x80
#define UCR1_UARTEN		BIT(0)
#define DOMAIN0_RUNNING(d)	(((d) & 0x3) != 0)

static struct imx_uart {
	unsigned int ccm_reg;
	unsigned int uart_base;
} imx8m_uart_info[] = {
	{	/* UART 1 */
		.ccm_reg = 0x4490,
		.uart_base = 0x30860000,
	}, {	/* UART 2 */
		.ccm_reg = 0x44a0,
		.uart_base = 0x30890000,
	}, {	/* UART 3 */
		.ccm_reg = 0x44b0,
		.uart_base = 0x30880000,
	}, {	/* UART 4 */
		.ccm_reg = 0x44c0,
		.uart_base = 0x30a60000,
	}
};

unsigned int imx8m_uart_get_base(void)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(imx8m_uart_info); i++) {
		uint32_t val;

		/*
		 * At least check that the clock-gate is ungated before we
		 * access the UART register.
		 */
		val = mmio_read_32(IMX_CCM_BASE + imx8m_uart_info[i].ccm_reg);
		if (DOMAIN0_RUNNING(val)) {
			val = mmio_read_32(imx8m_uart_info[i].uart_base + UCR1);
			if (val & UCR1_UARTEN) {
				return imx8m_uart_info[i].uart_base;
			}
		}
	}

	/*
	 * We should return an error and inform the user but we can't do it
	 * this early.
	 */
	return 0;
}
