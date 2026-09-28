/*
 * Copyright (c) 2020-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * This file is a thin wrapper layer.  Every public mailbox API function
 * dispatches to the active backend selected by mailbox_core_init():
 *
 *   Agilex72  -> MAILBOX_BACKEND_MCSR  (mcsr_backend)
 *   Agilex3   -> MAILBOX_BACKEND_SCSR  (scsr_backend)
 *
 * No direct hardware (MBOX_OFFSET) accesses are made here.
 */

#include <common/debug.h>

#include "mailbox/socfpga_mailbox_core.h"
#include <platform_def.h>
#include "socfpga_plat_def.h"
#include "socfpga_sip_svc.h"

/* -------------------------------------------------------------------------
 * Public wrappers -- all dispatch through the mailbox vtable pointer
 * set by mailbox_core_init().
 * -------------------------------------------------------------------------
 */

int mailbox_read_response(unsigned int *job_id, uint32_t *response,
			  unsigned int *resp_len)
{
	return mailbox->read_response(job_id, response, resp_len);
}

int mailbox_read_response_async(unsigned int *job_id, uint32_t *header,
				uint32_t *response, unsigned int *resp_len,
				uint8_t ignore_client_id)
{
	return mailbox->read_response_async(job_id, header, response, resp_len,
						ignore_client_id);
}

int iterate_resp(uint32_t mbox_resp_len, uint32_t *resp_buf,
		 unsigned int *resp_len)
{
	return mailbox->iterate_resp(mbox_resp_len, resp_buf, resp_len);
}

int mailbox_send_cmd_async_ext(uint32_t header_cmd, uint32_t *args,
			       unsigned int len)
{
	return mailbox->send_async_ext(header_cmd, args, len);
}

int mailbox_send_cmd_async(uint32_t *job_id, uint32_t cmd, uint32_t *args,
			   unsigned int len, unsigned int indirect)
{
	return mailbox->send_async(job_id, cmd, args, len, indirect);
}

int mailbox_send_cmd(uint32_t job_id, uint32_t cmd, uint32_t *args,
		     unsigned int len, uint32_t urgent, uint32_t *response,
		     unsigned int *resp_len)
{
	return mailbox->send(job_id, cmd, args, len, urgent, response, resp_len);
}

void mailbox_clear_response(void)
{
	mailbox->clear_response();
}

void mailbox_set_int(uint32_t interrupt)
{
	mailbox->set_int(interrupt);
}

#if SIP_SVC_V3
int mailbox_send_cmd_async_v3(uint8_t client_id, uint8_t job_id, uint32_t cmd,
			      uint32_t *args, uint32_t args_len, uint8_t cmd_flag,
			      sdm_command_callback cb, uint32_t *cb_args,
			      uint32_t cb_args_len)
{
	return mailbox->send_async_v3(client_id, job_id, cmd, args, args_len,
				  cmd_flag, cb, cb_args, cb_args_len);
}

int mailbox_response_poll_on_intr_v3(uint8_t *client_id, uint8_t *job_id,
				     uint64_t *bitmap)
{
	return mailbox->response_poll_on_intr_v3(client_id, job_id, bitmap);
}

int mailbox_response_poll_v3(uint8_t client_id, uint8_t job_id,
			     uint64_t *ret_args, uint32_t *ret_args_len)
{
	return mailbox->response_poll_v3(client_id, job_id, ret_args, ret_args_len);
}
#endif /* SIP_SVC_V3 */
