/*
 * Copyright (c) 2017-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <drivers/delay_timer.h>

#include <platform_def.h>
#include <scp.h>
#include <scp_cmd.h>

#include "m0_ipc.h"

/*
 * Reads a response from CRMU MAILBOX
 * Assumes that access has been granted and locked.
 * Note that this is just a temporary implementation until
 * channels are introduced
 */
static void scp_read_response(crmu_response_t *resp)
{
	uint32_t code;

	code = mmio_read_32(CRMU_MAIL_BOX0);
	resp->completed = code & MCU_IPC_CMD_DONE_MASK;
	resp->cmd = code & SCP_CMD_MASK;
	resp->ret = (code & MCU_IPC_CMD_REPLY_MASK) >> MCU_IPC_CMD_REPLY_SHIFT;
}

/*
 * Send a command to SCP and wait for timeout us.
 * Return:  0 on success
 *         -1 if there was no proper reply from SCP
 *         >0 if there was a response from MCU, but
 *            command completed with an error.
 */
int scp_send_cmd(uint32_t cmd, uint32_t param, uint32_t timeout)
{
	int ret = -1;

	mmio_write_32(CRMU_MAIL_BOX0, cmd);
	mmio_write_32(CRMU_MAIL_BOX1, param);
	do {
		crmu_response_t scp_resp;

		udelay(1);
		scp_read_response(&scp_resp);
		if (scp_resp.completed &&
			(scp_resp.cmd == cmd)) {
			/* This command has completed */
			ret = scp_resp.ret;
			break;
		}
	} while (--timeout);

	return ret;
}
