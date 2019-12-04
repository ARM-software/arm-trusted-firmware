/*
 * Copyright (C) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef IMX_SNVS_H
#define IMX_SNVS_H

#include <cdefs.h>
#include <stdint.h>

#include <arch.h>

struct snvs {
	uint32_t hplr;
	uint32_t hpcomr;
	uint32_t hpcr;
	uint32_t hpsicr;
	uint32_t hpsvcr;
	uint32_t hpsr;
	uint32_t hpsvsr;
	uint32_t hphacivr;
	uint32_t hphacr;
	uint32_t hprtcmr;
	uint32_t hprtclr;
	uint32_t hptamr;
	uint32_t hptalr;
	uint32_t lplr;
	uint32_t lpcr;
	uint32_t lpmkcr;
	uint32_t lpsvcr;
	uint32_t lptgfcr;
	uint32_t lptdcr;
	uint32_t lpsr;
	uint32_t lpsrtcmr;
	uint32_t lpsrtclr;
	uint32_t lptar;
	uint32_t lpsmcmr;
	uint32_t lpsmclr;
	uint32_t lppgdr;
	uint32_t lpgpr0_alias;
	uint8_t  lpzmkr[32];
	uint16_t res0;
	uint32_t lpgpr0[4];
	uint32_t lptdc2r;
	uint32_t lptdsr;
	uint32_t lptgf1cr;
	uint32_t lptgf2cr;
	uint32_t res1[4];
	uint32_t lpat1cr;
	uint32_t lpat2cr;
	uint32_t lpat3cr;
	uint32_t lpat4cr;
	uint32_t lpat5cr;
	uint32_t res2[3];
	uint32_t lpatctlr;
	uint32_t lpatclkr;
	uint32_t lpatrc1r;
	uint32_t lpatrc2r;
	uint32_t res3[706];
	uint32_t hpvidr1;
	uint32_t hpvidr2;
} __packed;

/* Define the HPCOMR bits */
#define HPCOMR_NPSWA_EN		BIT(31)
#define HPCOMR_HAC_STOP		BIT(19)
#define HPCOMR_HAC_CLEAR	BIT(18)
#define HPCOMR_HAC_LOAD		BIT(17)
#define HPCOMR_HAC_EN		BIT(16)
#define HPCOMR_MKS_EN		BIT(13)
#define HPCOMR_PROG_ZMK		BIT(12)
#define HPCOMR_SW_LPSV		BIT(10)
#define HPCOMR_SW_FSV		BIT(9)
#define HPCOMR_SW_SV		BIT(8)
#define HPCOMR_LP_SWR_DIS	BIT(5)
#define HPCOMR_LP_SWR		BIT(4)
#define HPCOMR_SSM_SFNS_DIS	BIT(2)
#define HPCOMR_SSM_ST_DIS	BIT(1)
#define HPCOMR_SSM_ST		BIT(0)

void imx_snvs_init(void);

#endif /* IMX_SNVS_H */
