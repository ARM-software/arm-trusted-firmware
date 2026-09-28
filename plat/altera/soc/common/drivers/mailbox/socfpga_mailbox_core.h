/*
 * Copyright (c) 2019-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_MBOX_CORE_H
#define SOCFPGA_MBOX_CORE_H

#include "mailbox/socfpga_mailbox.h"

enum mailbox_backend_type {
	MAILBOX_BACKEND_NONE,
	MAILBOX_BACKEND_MCSR,
	MAILBOX_BACKEND_SCSR
};

/* -------------------------------------------------------------------------
 * Backend vtable
 * -------------------------------------------------------------------------
 */
struct mailbox_backend {
	int (*init)(void);

	int32_t (*get_free_resp_desc)(void);
	sdm_command_t* (*get_cmd_desc)(uint8_t client_id, uint8_t job_id, bool ignore_job_id);
	sdm_response_t* (*get_resp_desc)(uint8_t client_id, uint8_t job_id, uint8_t *index);
	void (*free_cmd_desc)(sdm_command_t *cmd_desc);
	void (*free_resp_desc)(uint8_t index);
	sdm_command_t* (*get_free_cmd_desc)(void);
	sdm_response_t* (*get_resp_desc_cid)(uint8_t client_id, uint8_t *index);
	int (*response_handler)(void);
	int (*c2s_mailbox_send)(uint32_t header_cmd, uint32_t *args, unsigned int len);
	int (*wait_for_response_internal)(uint8_t client_id, uint32_t job_id, bool ignore_job_id,
					  uint32_t *response, unsigned int *resp_len);
	int (*mailbox_exec)(uint8_t client_id, uint8_t job_id, uint32_t cmd_header,
			    uint32_t *args, uint32_t args_len,
			    uint32_t *resp_buf, unsigned int *resp_len,
			    uint32_t flags);
	int (*send)(uint32_t job_id, uint32_t cmd, uint32_t *args,
		    unsigned int len, uint32_t urgent, uint32_t *response,
		    unsigned int *resp_len);
	int (*send_async)(uint32_t *job_id, uint32_t cmd, uint32_t *args,
			  unsigned int len, unsigned int indirect);
	int (*send_async_ext)(uint32_t header_cmd, uint32_t *args, unsigned int len);
	int (*send_async_v3)(uint8_t client_id, uint8_t job_id, uint32_t cmd,
			     uint32_t *args, uint32_t args_len, uint8_t cmd_flag,
			     sdm_command_callback cb, uint32_t *cb_args,
			     uint32_t cb_args_len);
	int (*read_response_async)(unsigned int *job_id, uint32_t *header,
				   uint32_t *response, unsigned int *resp_len,
				   uint8_t ignore_client_id);
	int (*read_response)(unsigned int *job_id, uint32_t *response, unsigned int *resp_len);
	void (*set_int)(uint32_t interrupt);
	void (*clear_response)(void);
	int (*response_poll_v3)(uint8_t client_id, uint8_t job_id,
				uint64_t *ret_args, uint32_t *ret_args_len);
	int (*response_poll_on_intr_v3)(uint8_t *client_id, uint8_t *job_id,
					uint64_t *bitmap);

	bool (*if_cmdbuf_full)(uint32_t cin);
	bool (*if_cmdbuf_empty)(uint32_t cin);
	int (*wait_cmdbuf_empty)(uint32_t cin);
	int (*write_cmd_buffer)(uint32_t *cin, uint32_t cout,
				uint32_t data, bool *is_doorbell_triggered);
	int (*fill_circular_buffer)(uint32_t header_cmd, uint32_t *args,
				    unsigned int len);
	int (*poll_response)(uint32_t job_id, uint32_t urgent, uint32_t *response,
			     unsigned int *resp_len);
	int (*iterate_resp)(uint32_t mbox_resp_len, uint32_t *resp_buf,
			    unsigned int *resp_len);
	int (*send_cmd_async)(uint32_t *job_id, uint32_t cmd, uint32_t *args,
			      unsigned int len, unsigned int indirect);
	int (*fill_cmd_desc)(uint8_t client_id, uint8_t job_id,
			     uint32_t *resp_buff);
	int (*read_response_v3)(uint8_t client_id, uint8_t *job_id,
				uint32_t *header, uint32_t *resp,
				uint32_t *resp_len,
				uint8_t ignore_client_id);
	int (*send_cmd_async_v3)(uint8_t client_id, uint8_t job_id, uint32_t cmd,
				 uint32_t *args, uint32_t args_len, uint8_t cmd_flag,
				 sdm_command_callback cb, uint32_t *cb_args,
				 uint32_t cb_args_len);
	int (*poll_response_v3)(uint8_t client_id, uint8_t job_id,
				uint32_t *resp, unsigned int *resp_len,
				uint32_t urgent);
	void (*response_parser)(void);
	int (*response_handler_fsm)(void);
	void (*init_v3)(void);

};

extern struct mailbox_backend *mailbox;

int mailbox_core_init(enum mailbox_backend_type type);

/*
 * Return the vtable of the mailbox backend built for this platform.  Exactly
 * one backend translation unit (mcsr/ or scsr/) is linked per platform and
 * provides this accessor, so the common core dispatcher never has to name a
 * backend that is not compiled in.
 */
struct mailbox_backend *mailbox_backend_get(enum mailbox_backend_type type);

#endif /* SOCFPGA_MBOX_CORE_H */
