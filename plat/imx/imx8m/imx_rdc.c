/*
 * Copyright (c) 2019, NXP. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>

#include <imx_rdc.h>

void imx_rdc_init(const struct imx_rdc_cfg *rdc_cfg)
{
	const struct imx_rdc_cfg *rdc = rdc_cfg;

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
