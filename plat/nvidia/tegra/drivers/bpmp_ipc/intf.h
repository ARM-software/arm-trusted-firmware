/*
 * Copyright (c) 2017-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BPMP_INTF_H
#define BPMP_INTF_H

/**
 * Flags used in IPC req
 */
#define FLAG_DO_ACK			(U(1) << 0)
#define FLAG_RING_DOORBELL		(U(1) << 1)

/* Bit 1 is designated for CCPlex in secure world */
#define HSP_MASTER_CCPLEX_BIT	(U(1) << 1)
/* Bit 19 is designated for BPMP in non-secure world */
#define HSP_MASTER_BPMP_BIT		(U(1) << 19)
/* Timeout to receive response from BPMP is 1 sec */
#define TIMEOUT_RESPONSE_FROM_BPMP_US	U(1000000) /* in microseconds */

/**
 * IVC protocol defines and command/response frame
 */

/**
 * IVC specific defines
 */
#define IVC_CMD_SZ_BYTES		U(128)
#define IVC_DATA_SZ_BYTES		U(120)

/**
 * Holds frame data for an IPC request
 */
struct frame_data {
	/* Identification as to what kind of data is being transmitted */
	uint32_t mrq;

	/* Flags for slave as to how to respond back */
	uint32_t flags;

	/* Actual data being sent */
	uint8_t data[IVC_DATA_SZ_BYTES];
};

/**
 * Commands send to the BPMP firmware
 */

/**
 * MRQ command codes
 */
#define MRQ_RESET			U(20)
#define MRQ_CLK				U(22)

/**
 * Reset sub-commands
 */
#define CMD_RESET_ASSERT		U(1)
#define CMD_RESET_DEASSERT		U(2)
#define CMD_RESET_MODULE		U(3)

/**
 * Used by the sender of an #MRQ_RESET message to request BPMP to
 * assert or deassert a given reset line.
 */
struct __attribute__((packed)) mrq_reset_request {
	/* reset action to perform (mrq_reset_commands) */
	uint32_t cmd;
	/* id of the reset to affected */
	uint32_t reset_id;
};

/**
 * MRQ_CLK sub-commands
 *
 */
enum {
	CMD_CLK_GET_RATE = U(1),
	CMD_CLK_SET_RATE = U(2),
	CMD_CLK_ROUND_RATE = U(3),
	CMD_CLK_GET_PARENT = U(4),
	CMD_CLK_SET_PARENT = U(5),
	CMD_CLK_IS_ENABLED = U(6),
	CMD_CLK_ENABLE = U(7),
	CMD_CLK_DISABLE = U(8),
	CMD_CLK_GET_ALL_INFO = U(14),
	CMD_CLK_GET_MAX_CLK_ID = U(15),
	CMD_CLK_MAX,
};

/**
 * Used by the sender of an #MRQ_CLK message to control clocks. The
 * clk_request is split into several sub-commands. Some sub-commands
 * require no additional data. Others have a sub-command specific
 * payload
 *
 * |sub-command                 |payload                |
 * |----------------------------|-----------------------|
 * |CMD_CLK_GET_RATE            |-                      |
 * |CMD_CLK_SET_RATE            |clk_set_rate           |
 * |CMD_CLK_ROUND_RATE          |clk_round_rate         |
 * |CMD_CLK_GET_PARENT          |-                      |
 * |CMD_CLK_SET_PARENT          |clk_set_parent         |
 * |CMD_CLK_IS_ENABLED          |-                      |
 * |CMD_CLK_ENABLE              |-                      |
 * |CMD_CLK_DISABLE             |-                      |
 * |CMD_CLK_GET_ALL_INFO        |-                      |
 * |CMD_CLK_GET_MAX_CLK_ID      |-                      |
 *
 */
struct mrq_clk_request {
	/**
	 * sub-command and clock id concatenated to 32-bit word.
	 * - bits[31..24] is the sub-cmd.
	 * - bits[23..0] is the clock id
	 */
	uint32_t cmd_and_id;
};

/**
 * Macro to prepare the MRQ_CLK sub-command
 */
#define make_mrq_clk_cmd(cmd, id)	(((cmd) << 24) | (id & 0xFFFFFF))

#endif /* BPMP_INTF_H */
