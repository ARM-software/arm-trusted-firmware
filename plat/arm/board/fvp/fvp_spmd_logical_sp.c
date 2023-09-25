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

#define SPMD_LP_MAX_SUPPORTED_SP 10
static void fvp_get_partition_info(void)
{
	struct ffa_value ret = { 0 };
	uint32_t target_uuid[4] = { 0 };
	static struct ffa_partition_info_v1_1
		part_info[SPMD_LP_MAX_SUPPORTED_SP] = { 0 };

	uint16_t num_partitions = 0;

	if (!spmd_el3_invoke_partition_info_get(target_uuid, 0, 0, &ret)) {
		panic();
	}

	if (is_ffa_error(&ret)) {
		panic();
	}

	num_partitions = ffa_partition_info_regs_get_last_idx(&ret) + 1;
	if (num_partitions > SPMD_LP_MAX_SUPPORTED_SP) {
		panic();
	}

	INFO("Number of secure partitions = %d\n", num_partitions);

	for (uint16_t i = 0; i < num_partitions; i++) {
		INFO("***Start Partition***\n");
		if (!ffa_partition_info_regs_get_part_info(&ret, i, &part_info[i]))
			panic();
		INFO("\tPartition ID: 0x%x\n", part_info[i].ep_id);
		INFO("\tvCPU count:0x%x\n", part_info[i].execution_ctx_count);
		INFO("\tProperties: 0x%x\n", part_info[i].properties);
		INFO("\tUUID: 0x%x 0x%x 0x%x 0x%x\n", part_info[i].uuid[0],
				part_info[i].uuid[1], part_info[i].uuid[2],
				part_info[i].uuid[3]);
		INFO("***End Partition***\n");
	}

}

static int32_t fvp_spmd_logical_partition_init(void)
{
	INFO("FVP SPMD LSP: Init function called.\n");

	fvp_get_partition_info();
	return 0;
}

/*
 * Platform specific SMC handler used to translate SIP SMCs or other platform
 * specific SMCs into FF-A direct messages.
 */
uintptr_t plat_spmd_logical_sp_smc_handler(unsigned int smc_fid,
			u_register_t x1,
			u_register_t x2,
			u_register_t x3,
			u_register_t x4,
			void *cookie,
			void *handle,
			u_register_t flags)
{
	struct ffa_value retval = { 0 };
	uint64_t send_recv_id = SPMD_LP_PARTITION_ID << 16 | 0x8001;

	/*
	 * Forward the SMC as direct request.
	 */
	if (!spmd_el3_ffa_msg_direct_req(send_recv_id, x2, x3, x4, handle, &retval)) {
		panic();
	}

	SMC_RET8(handle, retval.func, retval.arg1, retval.arg2, retval.arg3,
			retval.arg4, retval.arg5, retval.arg6, retval.arg7);
}

/* Register SPMD logical partition  */
DECLARE_SPMD_LOGICAL_PARTITION(
	fvp_spmd_logical_partition,
	fvp_spmd_logical_partition_init,/* Init Function */
	SPMD_LP_PARTITION_ID,		/* FF-A Partition ID */
	SPMD_LP_UUID,			/* UUID */
	SPMD_PARTITION_PROPERTIES	/* Partition Properties. */
);
