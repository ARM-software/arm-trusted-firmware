/*
 * Copyright (c) 2019, NXP. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_RDC_H
#define IMX_RDC_H

#include <lib/utils_def.h>

#include <platform_def.h>

#define MDAn(x)		(IMX_RDC_BASE + 0x200 + (x) * 4)
#define PDAPn(x)	(IMX_RDC_BASE + 0x400 + (x) * 4)
#define MRSAn(x)	(IMX_RDC_BASE + 0x800 + (x) * 0x10)
#define MREAn(x)	(IMX_RDC_BASE + 0x804 + (x) * 0x10)
#define MRCn(x)		(IMX_RDC_BASE + 0x808 + (x) * 0x10)

#define LCK		BIT(31)
#define SREQ		BIT(30)
#define ENA		BIT(30)

#define DID0		U(0x0)
#define DID1		U(0x1)
#define DID2		U(0x2)
#define DID3		U(0x3)

#define D3R		BIT(7)
#define D3W		BIT(6)
#define D2R		BIT(5)
#define D2W		BIT(4)
#define D1R		BIT(3)
#define D1W		BIT(2)
#define D0R		BIT(1)
#define D0W		BIT(0)

union rdc_setting {
	uint32_t rdc_mda; /* Master Domain Assignment */
	uint32_t rdc_pdap; /* Peripheral Domain Access Permissions */
	uint32_t rdc_mem_region[3]; /* Memory Region Access Control */
};

enum rdc_type {
	RDC_INVALID,
	RDC_MDA,
	RDC_PDAP,
	RDC_MEM_REGION,
};

struct imx_rdc_cfg {
	enum rdc_type type; /* config type Master, Peripheral or Memory region */
	int index;
	union rdc_setting setting;
};

#define RDC_MDAn(i, mda)	\
	{RDC_MDA, (i), .setting.rdc_mda = (mda), }
#define RDC_PDAPn(i, pdap)	\
	{RDC_PDAP, (i), .setting.rdc_pdap = (pdap), }

#define RDC_MEM_REGIONn(i, msa, mea, mrc)	\
	{ RDC_MEM_REGION, (i), 			\
	  .setting.rdc_mem_region[0] = (msa),	\
	  .setting.rdc_mem_region[1] = (mea),	\
	  .setting.rdc_mem_region[2] = (mrc),	\
	}

void imx_rdc_init(const struct imx_rdc_cfg *cfg);

#endif /* IMX_RDC_H */

