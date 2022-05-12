/*
 * Copyright (c) 2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>

#include "socfpga_mailbox.h"
#include "socfpga_sip_svc.h"

static uint32_t intel_v2_mbox_send_cmd(uint32_t req_header,
				uint32_t *data, uint32_t data_size)
{
	uint32_t value;
	uint32_t len;

	if ((data == NULL) || (data_size == 0)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (data_size > (MBOX_INC_HEADER_MAX_WORD_SIZE * MBOX_WORD_BYTE)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(data_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	/* Make sure client id align in SMC SiP V2 header and mailbox header */
	value = (req_header >> INTEL_SIP_SMC_HEADER_CID_OFFSET) &
				INTEL_SIP_SMC_HEADER_CID_MASK;

	if (value != MBOX_RESP_CLIENT_ID(data[0])) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	/* Make sure job id align in SMC SiP V2 header and mailbox header */
	value = (req_header >> INTEL_SIP_SMC_HEADER_JOB_ID_OFFSET) &
				INTEL_SIP_SMC_HEADER_JOB_ID_MASK;

	if (value != MBOX_RESP_JOB_ID(data[0])) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	/*
	 * Make sure data length align in SMC SiP V2 header and
	 * mailbox header
	 */
	len = (data_size / MBOX_WORD_BYTE) - 1;

	if (len != MBOX_RESP_LEN(data[0])) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	return mailbox_send_cmd_async_ext(data[0], &data[1], len);
}

static uint32_t intel_v2_mbox_poll_resp(uint64_t req_header,
				uint32_t *data, uint32_t *data_size,
				uint64_t *resp_header)
{
	int status = 0;
	uint32_t resp_len;
	uint32_t job_id = 0;
	uint32_t client_id = 0;
	uint32_t version;

	if ((data == NULL) || (data_size == NULL) || (resp_header == NULL)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_size_4_bytes_aligned(*data_size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	resp_len = (*data_size / MBOX_WORD_BYTE) - 1;
	status = mailbox_read_response_async(&job_id, &data[0], &data[1],
				&resp_len, 1);

	if (status == MBOX_BUSY) {
		status = INTEL_SIP_SMC_STATUS_BUSY;
	} else if (status == MBOX_NO_RESPONSE) {
		status = INTEL_SIP_SMC_STATUS_NO_RESPONSE;
	} else {
		*data_size = 0;

		if (resp_len > 0) {
			/*
			 * Fill in the final response length,
			 * the length include both mailbox header and payload
			 */
			*data_size = (resp_len + 1) * MBOX_WORD_BYTE;

			/* Extract the client id from mailbox header */
			client_id = MBOX_RESP_CLIENT_ID(data[0]);
		}

		/*
		 * Extract SMC SiP V2 protocol version from
		 * SMC request header
		 */
		version = (req_header >> INTEL_SIP_SMC_HEADER_VERSION_OFFSET) &
				INTEL_SIP_SMC_HEADER_VERSION_MASK;

		/* Fill in SMC SiP V2 protocol response header */
		*resp_header = 0;
		*resp_header |= (((uint64_t)job_id) &
				INTEL_SIP_SMC_HEADER_JOB_ID_MASK) <<
				INTEL_SIP_SMC_HEADER_JOB_ID_OFFSET;
		*resp_header |= (((uint64_t)client_id) &
				INTEL_SIP_SMC_HEADER_CID_MASK) <<
				INTEL_SIP_SMC_HEADER_CID_OFFSET;
		*resp_header |= (((uint64_t)version) &
				INTEL_SIP_SMC_HEADER_VERSION_MASK) <<
				INTEL_SIP_SMC_HEADER_VERSION_OFFSET;
	}

	return status;
}

uintptr_t sip_smc_handler_v2(uint32_t smc_fid,
				u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *cookie,
				void *handle,
				u_register_t flags)
{
	uint32_t retval = 0;
	uint64_t retval64 = 0;
	int status = INTEL_SIP_SMC_STATUS_OK;

	switch (smc_fid) {
	case INTEL_SIP_SMC_V2_GET_SVC_VERSION:
		SMC_RET4(handle, INTEL_SIP_SMC_STATUS_OK, x1,
				SIP_SVC_VERSION_MAJOR,
				SIP_SVC_VERSION_MINOR);

	case INTEL_SIP_SMC_V2_REG_READ:
		status = intel_secure_reg_read(x2, &retval);
		SMC_RET4(handle, status, x1, retval, x2);

	case INTEL_SIP_SMC_V2_REG_WRITE:
		status = intel_secure_reg_write(x2, (uint32_t)x3, &retval);
		SMC_RET4(handle, status, x1, retval, x2);

	case INTEL_SIP_SMC_V2_REG_UPDATE:
		status = intel_secure_reg_update(x2, (uint32_t)x3,
				(uint32_t)x4, &retval);
		SMC_RET4(handle, status, x1, retval, x2);

	case INTEL_SIP_SMC_V2_HPS_SET_BRIDGES:
		status = intel_hps_set_bridges(x2, x3);
		SMC_RET2(handle, status, x1);

	case INTEL_SIP_SMC_V2_MAILBOX_SEND_COMMAND:
		status = intel_v2_mbox_send_cmd(x1, (uint32_t *)x2, x3);
		SMC_RET2(handle, status, x1);

	case INTEL_SIP_SMC_V2_MAILBOX_POLL_RESPONSE:
		status = intel_v2_mbox_poll_resp(x1, (uint32_t *)x2,
				(uint32_t *) &x3, &retval64);
		SMC_RET4(handle, status, retval64, x2, x3);

	default:
		ERROR("%s: unhandled SMC V2 (0x%x)\n", __func__, smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}
