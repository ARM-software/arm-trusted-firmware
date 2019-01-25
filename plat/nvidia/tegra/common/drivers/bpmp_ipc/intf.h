/*
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTF_H
#define INTF_H

/**
 * Flags used in IPC req
 */
#define FLAG_DO_ACK			(U(1) << 0)
#define FLAG_RING_DOORBELL		(U(1) << 1)

/* Bit 1 is designated for CCPlex in secure world */
#define HSP_MASTER_CCPLEX_BIT		(U(1) << 1)
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
 * MRQ code to issue a module reset command to BPMP
 */
#define MRQ_RESET			U(20)

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

#endif /* INTF_H */
