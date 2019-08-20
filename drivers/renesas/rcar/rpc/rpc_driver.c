/*
 * Copyright (c) 2015-2019, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "cpg_registers.h"
#include "rcar_def.h"
#include "rcar_private.h"
#include "rpc_registers.h"

#define MSTPSR9_RPC_BIT		(0x00020000U)
#define RPC_CMNCR_MD_BIT	(0x80000000U)
#define RPC_PHYCNT_CAL		BIT(31)
#define RPC_PHYCNT_STRTIM_M3V1	(0x6 << 15UL)
#define RPC_PHYCNT_STRTIM	(0x7 << 15UL)

static void rpc_enable(void)
{
	/* Enable clock supply to RPC. */
	mstpcr_write(CPG_SMSTPCR9, CPG_MSTPSR9, MSTPSR9_RPC_BIT);
}

static void rpc_setup(void)
{
	uint32_t product, cut, reg, phy_strtim;

	if (mmio_read_32(RPC_CMNCR) & RPC_CMNCR_MD_BIT)
		mmio_clrbits_32(RPC_CMNCR, RPC_CMNCR_MD_BIT);

	product = mmio_read_32(RCAR_PRR) & PRR_PRODUCT_MASK;
	cut = mmio_read_32(RCAR_PRR) & PRR_CUT_MASK;

	if ((product ==  PRR_PRODUCT_M3) && (cut < PRR_PRODUCT_30))
		phy_strtim = RPC_PHYCNT_STRTIM_M3V1;
	else
		phy_strtim = RPC_PHYCNT_STRTIM;

	reg = mmio_read_32(RPC_PHYCNT);
	reg &= ~RPC_PHYCNT_STRTIM;
	reg |= phy_strtim;
	mmio_write_32(RPC_PHYCNT, reg);
	reg |= RPC_PHYCNT_CAL;
	mmio_write_32(RPC_PHYCNT, reg);
}

void rcar_rpc_init(void)
{
	rpc_enable();
	rpc_setup();
}
