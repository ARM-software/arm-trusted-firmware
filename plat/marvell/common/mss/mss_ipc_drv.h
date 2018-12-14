/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MSS_IPC_DRV_H
#define MSS_IPC_DRV_H

#include <lib/psci/psci.h>

#define MV_PM_FW_IPC_VERSION_MAGIC	(0xCA530000) /* Do NOT change */
/* Increament for each version */
#define MV_PM_FW_IPC_VERSION_SEQ	(0x00000001)
#define MV_PM_FW_IPC_VERSION		(MV_PM_FW_IPC_VERSION_MAGIC | \
					 MV_PM_FW_IPC_VERSION_SEQ)

#define IPC_MSG_STATE_LOC		(0x0)
#define IPC_MSG_SYNC_ID_LOC		(0x4)
#define IPC_MSG_ID_LOC			(0x8)
#define IPC_MSG_RET_CH_ID_LOC	(0xC)
#define IPC_MSG_CPU_ID_LOC		(0x10)
#define IPC_MSG_CLUSTER_ID_LOC	(0x14)
#define IPC_MSG_SYSTEM_ID_LOC	(0x18)
#define IPC_MSG_POWER_STATE_LOC	(0x1C)
#define IPC_MSG_REPLY_LOC		(0x20)
#define IPC_MSG_RESERVED_LOC	(0x24)

/* IPC initialization state */
enum mss_pm_ipc_init_state {
	IPC_UN_INITIALIZED	= 1,
	IPC_INITIALIZED		= 2
};

/* IPC queue direction */
enum mss_pm_ipc_init_msg_dir {
	IPC_MSG_TX	= 0,
	IPC_MSG_RX	= 1
};

/* IPC message state */
enum mss_pm_ipc_msg_state {
	IPC_MSG_FREE	= 1,
	IPC_MSG_OCCUPY	= 2

};

/* IPC control block */
struct mss_pm_ipc_ctrl {
	unsigned int ctrl_base_address;
	unsigned int msg_base_address;
	unsigned int num_of_channels;
	unsigned int channel_size;
	unsigned int queue_size;
};

/* IPC message types */
enum mss_pm_msg_id {
	PM_IPC_MSG_CPU_SUSPEND		= 1,
	PM_IPC_MSG_CPU_OFF		= 2,
	PM_IPC_MSG_CPU_ON		= 3,
	PM_IPC_MSG_SYSTEM_RESET		= 4,
	PM_IPC_MSG_SYSTEM_SUSPEND	= 5,
	PM_IPC_MAX_MSG
};

struct mss_pm_ipc_msg {
	unsigned int	msg_sync_id;	/*
					 * Sync number, validate message
					 * reply corresponding to message
					 * received
					 */
	unsigned int	msg_id;		/* Message Id */
	unsigned int	ret_channel_id;	/* IPC channel reply */
	unsigned int	cpu_id;		/* CPU Id */
	unsigned int	cluster_id;	/* Cluster Id */
	unsigned int	system_id;	/* System Id */
	unsigned int	power_state;
	unsigned int	msg_reply;	/* Message reply */
};

/* IPC queue */
struct mss_pm_ipc_queue {
	unsigned int	state;
	struct mss_pm_ipc_msg		msg;
};

/* IPC channel */
struct mss_pm_ipc_ch {
	struct mss_pm_ipc_queue *tx_queue;
	struct mss_pm_ipc_queue *rx_queue;
};

/*****************************************************************************
 * mv_pm_ipc_init
 *
 * DESCRIPTION: Initialize PM IPC infrastructure
 *****************************************************************************
 */
int mv_pm_ipc_init(unsigned long ipc_control_addr);

/*****************************************************************************
 * mv_pm_ipc_msg_rx
 *
 * DESCRIPTION: Retrieve message from IPC channel
 *****************************************************************************
 */
int mv_pm_ipc_msg_rx(unsigned int channel_id, struct mss_pm_ipc_msg *msg);

/*****************************************************************************
 * mv_pm_ipc_msg_tx
 *
 * DESCRIPTION: Send message via IPC channel
 *****************************************************************************
 */
int mv_pm_ipc_msg_tx(unsigned int channel_id, unsigned int msg_id,
			unsigned int cluster_power_state);

#endif /* MSS_IPC_DRV_H */
