/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MSS_PM_IPC_H
#define MSS_PM_IPC_H

#include <mss_ipc_drv.h>

/* Currently MSS does not support Cluster level Power Down */
#define DISABLE_CLUSTER_LEVEL


/*****************************************************************************
 * mss_pm_ipc_msg_send
 *
 * DESCRIPTION: create and transmit IPC message
 *****************************************************************************
 */
int mss_pm_ipc_msg_send(unsigned int channel_id, unsigned int msg_id,
			const psci_power_state_t *target_state);

/*****************************************************************************
 * mss_pm_ipc_msg_trigger
 *
 * DESCRIPTION: Trigger IPC message interrupt to MSS
 *****************************************************************************
 */
int mss_pm_ipc_msg_trigger(void);


#endif /* MSS_PM_IPC_H */
