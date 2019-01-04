/*
 * Copyright (c) 2015-2018, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include "cpg_registers.h"
#include "rpc_registers.h"
#include "rcar_private.h"

#define MSTPSR9_RPC_BIT		(0x00020000U)
#define RPC_CMNCR_MD_BIT	(0x80000000U)

static void rpc_enable(void)
{
	/* Enable clock supply to RPC. */
	mstpcr_write(CPG_SMSTPCR9, CPG_MSTPSR9, MSTPSR9_RPC_BIT);
}

static void rpc_setup(void)
{
	if (mmio_read_32(RPC_CMNCR) & RPC_CMNCR_MD_BIT)
		mmio_clrbits_32(RPC_CMNCR, RPC_CMNCR_MD_BIT);
}

void rcar_rpc_init(void)
{
	rpc_enable();
	rpc_setup();
}
