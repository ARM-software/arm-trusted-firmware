/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <string.h>

#include <common/debug.h>
#include <lib/psci/psci.h>
#include <lib/mmio.h>

#include <mss_pm_ipc.h>

/*
 * SISR is 32 bit interrupt register representing 32 interrupts
 *
 * +======+=============+=============+
 * + Bits + 31          + 30 - 00     +
 * +======+=============+=============+
 * + Desc + MSS Msg Int + Reserved    +
 * +======+=============+=============+
 */
#define MSS_SISR		(MVEBU_REGS_BASE + 0x5800D0)
#define MSS_SISTR		(MVEBU_REGS_BASE + 0x5800D8)

#define MSS_MSG_INT_MASK	(0x80000000)
#define MSS_TIMER_BASE		(MVEBU_REGS_BASE_MASK + 0x580110)
#define MSS_TRIGGER_TIMEOUT	(2000)

/*****************************************************************************
 * mss_pm_ipc_msg_send
 *
 * DESCRIPTION: create and transmit IPC message
 *****************************************************************************
 */
int mss_pm_ipc_msg_send(unsigned int channel_id, unsigned int msg_id,
			const psci_power_state_t *target_state)
{
	/* Transmit IPC message */
#ifndef DISABLE_CLUSTER_LEVEL
	mv_pm_ipc_msg_tx(channel_id, msg_id,
			 (unsigned int)target_state->pwr_domain_state[
					MPIDR_AFFLVL1]);
#else
	mv_pm_ipc_msg_tx(channel_id, msg_id, 0);
#endif

	return 0;
}

/*****************************************************************************
 * mss_pm_ipc_msg_trigger
 *
 * DESCRIPTION: Trigger IPC message interrupt to MSS
 *****************************************************************************
 */
int mss_pm_ipc_msg_trigger(void)
{
	unsigned int timeout;
	unsigned int t_end;
	unsigned int t_start = mmio_read_32(MSS_TIMER_BASE);

	mmio_write_32(MSS_SISR, MSS_MSG_INT_MASK);

	do {
		/* wait while SCP process incoming interrupt */
		if (mmio_read_32(MSS_SISTR) != MSS_MSG_INT_MASK)
			break;

		/* check timeout */
		t_end = mmio_read_32(MSS_TIMER_BASE);

		timeout = ((t_start > t_end) ?
			   (t_start - t_end) : (t_end - t_start));
		if (timeout > MSS_TRIGGER_TIMEOUT) {
			ERROR("PM MSG Trigger Timeout\n");
			break;
		}

	} while (1);

	return 0;
}
