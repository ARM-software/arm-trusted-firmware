/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <services/el3_spmc_logical_sp.h>
#include <services/ffa_svc.h>
#include <smccc_helpers.h>

#define LP_PARTITION_ID 0xC001
#define LP_UUID {0x47a3bf57, 0xe98e43ad, 0xb7db524f, 0x1588f4e3}

/* Our Logical SP currently only supports receipt of direct messaging. */
#define PARTITION_PROPERTIES FFA_PARTITION_DIRECT_REQ_RECV

static int32_t sp_init(void)
{
	INFO("LSP: Init function called.\n");
	return 0;
}

static uint64_t handle_ffa_direct_request(uint32_t smc_fid,  bool secure_origin,
					  uint64_t x1, uint64_t x2, uint64_t x3,
					  uint64_t x4, void *cookie,
					  void *handle, uint64_t flags)
{
	uint64_t ret;

	/* Determine if we have a 64 or 32 direct request. */
	if (smc_fid == FFA_MSG_SEND_DIRECT_REQ_SMC32) {
		ret = FFA_MSG_SEND_DIRECT_RESP_SMC32;
	} else if (smc_fid == FFA_MSG_SEND_DIRECT_REQ_SMC64) {
		ret = FFA_MSG_SEND_DIRECT_RESP_SMC64;
	} else {
		panic(); /* Unknown SMC. */
	}
	/*
	 * Handle the incoming request. For testing purposes we echo the
	 * incoming message.
	 */
	INFO("Logical Partition: Received Direct Request from %s world!\n",
	     secure_origin ? "Secure" : "Normal");

	/*
	 * Logical SP's must always send a direct response so we can populate
	 * our response directly.
	 */
	SMC_RET8(handle, ret, 0, 0, x4, 0, 0, 0, 0);
}

/* Register logical partition  */
DECLARE_LOGICAL_PARTITION(
	my_logical_partition,
	sp_init,			/* Init Function */
	LP_PARTITION_ID,		/* FF-A Partition ID */
	LP_UUID,			/* UUID */
	PARTITION_PROPERTIES,		/* Partition Properties. */
	handle_ffa_direct_request	/* Callback for direct requests. */
);
