/*
 * Copyright (c) 2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <common/debug.h>
#include <services/el3_spmd_logical_sp.h>
#include <services/ffa_svc.h>
#include <smccc_helpers.h>

#define SPMD_LP_PARTITION_ID SPMD_LP_ID_START
#define SPMD_LP_UUID {0xe98e43ad, 0xb7db524f, 0x47a3bf57, 0x1588f4e3}

/* SPMD Logical SP currently only supports sending direct message. */
#define SPMD_PARTITION_PROPERTIES FFA_PARTITION_DIRECT_REQ_SEND

static int32_t fvp_spmd_logical_partition_init(void)
{
	INFO("FVP SPMD LSP: Init function called.\n");
	return 0;
}

/* Register SPMD logical partition  */
DECLARE_SPMD_LOGICAL_PARTITION(
	fvp_spmd_logical_partition,
	fvp_spmd_logical_partition_init,/* Init Function */
	SPMD_LP_PARTITION_ID,		/* FF-A Partition ID */
	SPMD_LP_UUID,			/* UUID */
	SPMD_PARTITION_PROPERTIES	/* Partition Properties. */
);
