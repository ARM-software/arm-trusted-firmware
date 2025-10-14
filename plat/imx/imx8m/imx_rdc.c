/*
 * Copyright (c) 2019, NXP. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <lib/mmio.h>

#include <imx_rdc.h>

struct imx_uart {
	int index;
	unsigned int uart_base;
};

static const struct imx_uart imx8m_uart_info[] = {
	{	/* UART 1 */
		.index = RDC_PDAP_UART1,
		.uart_base = IMX_UART1_BASE,
	}, {	/* UART 2 */
		.index = RDC_PDAP_UART2,
		.uart_base = IMX_UART2_BASE,
	}, {	/* UART 3 */
		.index = RDC_PDAP_UART3,
		.uart_base = IMX_UART3_BASE,
	}, {	/* UART 4 */
		.index = RDC_PDAP_UART4,
		.uart_base = IMX_UART4_BASE,
	}
};

static int imx_rdc_uart_get_pdap_index(unsigned int uart_base)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(imx8m_uart_info); i++) {
		if (imx8m_uart_info[i].uart_base == uart_base) {
			return imx8m_uart_info[i].index;
		}
	}

	return -ENODEV;
}

static void imx_rdc_console_access_enable(struct imx_rdc_cfg *rdc_cfg,
				   unsigned int console_base)
{
	struct imx_rdc_cfg *rdc;
	int console_pdap_index;

	console_pdap_index = imx_rdc_uart_get_pdap_index(console_base);
	if (console_pdap_index < 0) {
		return;
	}

	for (rdc = rdc_cfg; rdc->type != RDC_INVALID; rdc++) {
		if (rdc->type != RDC_PDAP || rdc->index != console_pdap_index) {
			continue;
		}

		if (rdc->index == console_pdap_index) {
			rdc->setting.rdc_pdap = D0R | D0W;
			return;
		}
	}
}

void imx_rdc_init(struct imx_rdc_cfg *rdc_cfg, unsigned int console_base)
{
	struct imx_rdc_cfg *rdc = rdc_cfg;

	imx_rdc_console_access_enable(rdc, console_base);

	while (rdc->type != RDC_INVALID) {
		switch (rdc->type) {
		case RDC_MDA:
			/* MDA config */
			mmio_write_32(MDAn(rdc->index), rdc->setting.rdc_mda);
			break;
		case RDC_PDAP:
			/* peripheral access permission config */
			mmio_write_32(PDAPn(rdc->index), rdc->setting.rdc_pdap);
			break;
		case RDC_MEM_REGION:
			/* memory region access permission config */
			mmio_write_32(MRSAn(rdc->index), rdc->setting.rdc_mem_region[0]);
			mmio_write_32(MREAn(rdc->index), rdc->setting.rdc_mem_region[1]);
			mmio_write_32(MRCn(rdc->index), rdc->setting.rdc_mem_region[2]);
			break;
		default:
			break;
		}

		rdc++;
	}
}
