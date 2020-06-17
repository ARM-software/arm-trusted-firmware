/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MSS_MEM_H
#define MSS_MEM_H

/* MSS SRAM Memory base */
#define MSS_SRAM_PM_CONTROL_BASE		(MVEBU_REGS_BASE + 0x520000)

enum mss_pm_ctrl_handshake {
	MSS_UN_INITIALIZED	= 0,
	MSS_COMPATIBILITY_ERROR	= 1,
	MSS_ACKNOWLEDGMENT	= 2,
	HOST_ACKNOWLEDGMENT	= 3
};

enum mss_pm_ctrl_rtos_env {
	MSS_MULTI_PROCESS_ENV	= 0,
	MSS_SINGLE_PROCESS_ENV	= 1,
	MSS_MAX_PROCESS_ENV
};

struct mss_pm_ctrl_block {
	/* This field is used to synchronize the Host
	 * and MSS initialization sequence
	 * Valid Values
	 * 0 - Un-Initialized
	 * 1 - Compatibility Error
	 * 2 - MSS Acknowledgment
	 * 3 - Host Acknowledgment
	 */
	unsigned int handshake;

	/*
	 * This field include Host IPC version. Once received by the MSS
	 * It will be compared to MSS IPC version and set MSS Acknowledge to
	 * "compatibility error" in case there is no match
	 */
	unsigned int ipc_version;
	unsigned int ipc_base_address;
	unsigned int ipc_state;

	/* Following fields defines firmware core architecture */
	unsigned int num_of_cores;
	unsigned int num_of_clusters;
	unsigned int num_of_cores_per_cluster;

	/* Following fields define pm trace debug base address */
	unsigned int pm_trace_ctrl_base_address;
	unsigned int pm_trace_info_base_address;
	unsigned int pm_trace_info_core_size;

	unsigned int ctrl_blk_size;
};

#endif /* MSS_MEM_H */
