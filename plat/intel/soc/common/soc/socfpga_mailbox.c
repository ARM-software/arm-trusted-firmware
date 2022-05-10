/*
 * Copyright (c) 2020-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>

#include "socfpga_mailbox.h"
#include "socfpga_sip_svc.h"

static mailbox_payload_t mailbox_resp_payload;
static mailbox_container_t mailbox_resp_ctr = {0, 0, &mailbox_resp_payload};

static bool is_mailbox_cmdbuf_full(uint32_t cin)
{
	uint32_t cout = mmio_read_32(MBOX_OFFSET + MBOX_COUT);

	return (((cin + 1U) % MBOX_CMD_BUFFER_SIZE) == cout);
}

static bool is_mailbox_cmdbuf_empty(uint32_t cin)
{
	uint32_t cout = mmio_read_32(MBOX_OFFSET + MBOX_COUT);

	return (((cout + 1U) % MBOX_CMD_BUFFER_SIZE) == cin);
}

static int wait_for_mailbox_cmdbuf_empty(uint32_t cin)
{
	unsigned int timeout = 200U;

	do {
		if (is_mailbox_cmdbuf_empty(cin)) {
			break;
		}
		mdelay(10U);
	} while (--timeout != 0U);

	if (timeout == 0U) {
		return MBOX_TIMEOUT;
	}

	return MBOX_RET_OK;
}

static int write_mailbox_cmd_buffer(uint32_t *cin, uint32_t cout,
				    uint32_t data,
				    bool *is_doorbell_triggered)
{
	unsigned int timeout = 100U;

	do {
		if (is_mailbox_cmdbuf_full(*cin)) {
			if (!(*is_doorbell_triggered)) {
				mmio_write_32(MBOX_OFFSET +
					      MBOX_DOORBELL_TO_SDM, 1U);
				*is_doorbell_triggered = true;
			}
			mdelay(10U);
		} else {
			mmio_write_32(MBOX_ENTRY_TO_ADDR(CMD, (*cin)++), data);
			*cin %= MBOX_CMD_BUFFER_SIZE;
			mmio_write_32(MBOX_OFFSET + MBOX_CIN, *cin);
			break;
		}
	} while (--timeout != 0U);

	if (timeout == 0U) {
		return MBOX_TIMEOUT;
	}

	if (*is_doorbell_triggered) {
		int ret = wait_for_mailbox_cmdbuf_empty(*cin);
		return ret;
	}

	return MBOX_RET_OK;
}

static int fill_mailbox_circular_buffer(uint32_t header_cmd, uint32_t *args,
					unsigned int len)
{
	uint32_t sdm_read_offset, cmd_free_offset;
	unsigned int i;
	int ret;
	bool is_doorbell_triggered = false;

	cmd_free_offset = mmio_read_32(MBOX_OFFSET + MBOX_CIN);
	sdm_read_offset = mmio_read_32(MBOX_OFFSET + MBOX_COUT);

	ret = write_mailbox_cmd_buffer(&cmd_free_offset, sdm_read_offset,
				       header_cmd, &is_doorbell_triggered);
	if (ret != 0) {
		goto restart_mailbox;
	}

	for (i = 0U; i < len; i++) {
		is_doorbell_triggered = false;
		ret = write_mailbox_cmd_buffer(&cmd_free_offset,
					       sdm_read_offset, args[i],
					       &is_doorbell_triggered);
		if (ret != 0) {
			goto restart_mailbox;
		}
	}

	mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_TO_SDM, 1U);

	return MBOX_RET_OK;

restart_mailbox:
	/*
	 * Attempt to restart mailbox if the driver not able to write
	 * into mailbox command buffer
	 */
	if (MBOX_CMD_MASK(header_cmd) != MBOX_CMD_RESTART) {
		INFO("Mailbox timed out: Attempting mailbox reset\n");
		ret = mailbox_init();

		if (ret == MBOX_TIMEOUT) {
			INFO("Error: Mailbox fail to restart\n");
		}
	}

	return MBOX_TIMEOUT;
}

int mailbox_read_response(unsigned int *job_id, uint32_t *response,
				unsigned int *resp_len)
{
	uint32_t rin;
	uint32_t rout;
	uint32_t resp_data;
	unsigned int ret_resp_len;

	if (mmio_read_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM) == 1U) {
		mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM, 0U);
	}

	rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
	rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);

	if (rout != rin) {
		resp_data = mmio_read_32(MBOX_ENTRY_TO_ADDR(RESP, (rout)++));

		rout %= MBOX_RESP_BUFFER_SIZE;
		mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);


		if (MBOX_RESP_CLIENT_ID(resp_data) != MBOX_ATF_CLIENT_ID) {
			return MBOX_WRONG_ID;
		}

		*job_id = MBOX_RESP_JOB_ID(resp_data);

		ret_resp_len = MBOX_RESP_LEN(resp_data);

		if (iterate_resp(ret_resp_len, response, resp_len)
			!= MBOX_RET_OK) {
			return MBOX_TIMEOUT;
		}

		if (MBOX_RESP_ERR(resp_data) > 0U) {
			INFO("Error in response: %x\n", resp_data);
			return -MBOX_RESP_ERR(resp_data);
		}

		return MBOX_RET_OK;
	}
	return MBOX_NO_RESPONSE;
}

int mailbox_read_response_async(unsigned int *job_id, uint32_t *header,
				uint32_t *response, unsigned int *resp_len,
				uint8_t ignore_client_id)
{
	uint32_t rin;
	uint32_t rout;
	uint32_t resp_data;
	uint32_t ret_resp_len = 0;
	uint8_t is_done = 0;

	if ((mailbox_resp_ctr.flag & MBOX_PAYLOAD_FLAG_BUSY) != 0) {
		ret_resp_len = MBOX_RESP_LEN(
				mailbox_resp_ctr.payload->header) -
				mailbox_resp_ctr.index;
	}

	if (mmio_read_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM) == 1U) {
		mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM, 0U);
	}

	rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
	rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);

	while (rout != rin && !is_done) {

		resp_data = mmio_read_32(MBOX_ENTRY_TO_ADDR(RESP, (rout)++));

		rout %= MBOX_RESP_BUFFER_SIZE;
		mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);
		rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);

		if ((mailbox_resp_ctr.flag & MBOX_PAYLOAD_FLAG_BUSY) != 0) {
			mailbox_resp_ctr.payload->data[mailbox_resp_ctr.index] = resp_data;
			mailbox_resp_ctr.index++;
			ret_resp_len--;
		} else {
			if (!ignore_client_id) {
				if (MBOX_RESP_CLIENT_ID(resp_data) != MBOX_ATF_CLIENT_ID) {
					*resp_len = 0;
					return MBOX_WRONG_ID;
				}
			}

			*job_id = MBOX_RESP_JOB_ID(resp_data);
			ret_resp_len = MBOX_RESP_LEN(resp_data);
			mailbox_resp_ctr.payload->header = resp_data;
			mailbox_resp_ctr.flag |= MBOX_PAYLOAD_FLAG_BUSY;
		}

		if (ret_resp_len == 0) {
			is_done = 1;
		}
	}

	if (is_done != 0) {

		/* copy header data to input address if applicable */
		if (header != 0) {
			*header = mailbox_resp_ctr.payload->header;
		}

		/* copy response data to input buffer if applicable */
		ret_resp_len = MBOX_RESP_LEN(mailbox_resp_ctr.payload->header);
		if ((ret_resp_len > 0) && (response == NULL) && resp_len) {
			if (*resp_len > ret_resp_len) {
				*resp_len = ret_resp_len;
			}

			memcpy((uint8_t *) response,
				(uint8_t *) mailbox_resp_ctr.payload->data,
				*resp_len * MBOX_WORD_BYTE);
		}

		/* reset async response param */
		mailbox_resp_ctr.index = 0;
		mailbox_resp_ctr.flag = 0;

		if (MBOX_RESP_ERR(mailbox_resp_ctr.payload->header) > 0U) {
			INFO("Error in async response: %x\n",
				mailbox_resp_ctr.payload->header);
			return -MBOX_RESP_ERR(mailbox_resp_ctr.payload->header);
		}

		return MBOX_RET_OK;
	}

	*resp_len = 0;
	return (mailbox_resp_ctr.flag & MBOX_PAYLOAD_FLAG_BUSY) ? MBOX_BUSY : MBOX_NO_RESPONSE;
}

int mailbox_poll_response(uint32_t job_id, uint32_t urgent, uint32_t *response,
				unsigned int *resp_len)
{
	unsigned int timeout = 40U;
	unsigned int sdm_loop = 255U;
	unsigned int ret_resp_len;
	uint32_t rin;
	uint32_t rout;
	uint32_t resp_data;

	while (sdm_loop != 0U) {

		do {
			if (mmio_read_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM)
				== 1U) {
				break;
			}
			mdelay(10U);
		} while (--timeout != 0U);

		if (timeout == 0U) {
			break;
		}

		mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM, 0U);

		if ((urgent & 1U) != 0U) {
			mdelay(5U);
			if ((mmio_read_32(MBOX_OFFSET + MBOX_STATUS) &
				MBOX_STATUS_UA_MASK) ^
				(urgent & MBOX_STATUS_UA_MASK)) {
				mmio_write_32(MBOX_OFFSET + MBOX_URG, 0U);
				return MBOX_RET_OK;
			}

			mmio_write_32(MBOX_OFFSET + MBOX_URG, 0U);
			INFO("Error: Mailbox did not get UA");
			return MBOX_RET_ERROR;
		}

		rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
		rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);

		while (rout != rin) {
			resp_data = mmio_read_32(MBOX_ENTRY_TO_ADDR(RESP,
								(rout)++));

			rout %= MBOX_RESP_BUFFER_SIZE;
			mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);

			if (MBOX_RESP_CLIENT_ID(resp_data) != MBOX_ATF_CLIENT_ID
				|| MBOX_RESP_JOB_ID(resp_data) != job_id) {
				continue;
			}

			ret_resp_len = MBOX_RESP_LEN(resp_data);

			if (iterate_resp(ret_resp_len, response, resp_len)
				!= MBOX_RET_OK) {
				return MBOX_TIMEOUT;
			}

			if (MBOX_RESP_ERR(resp_data) > 0U) {
				INFO("Error in response: %x\n", resp_data);
				return -MBOX_RESP_ERR(resp_data);
			}

			return MBOX_RET_OK;
		}

	sdm_loop--;
	}

	INFO("Timed out waiting for SDM\n");
	return MBOX_TIMEOUT;
}

int iterate_resp(uint32_t mbox_resp_len, uint32_t *resp_buf,
			unsigned int *resp_len)
{
	unsigned int timeout, total_resp_len = 0U;
	uint32_t resp_data;
	uint32_t rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
	uint32_t rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);

	while (mbox_resp_len > 0U) {
		timeout = 100U;
		mbox_resp_len--;
		resp_data = mmio_read_32(MBOX_ENTRY_TO_ADDR(RESP, (rout)++));

		if ((resp_buf != NULL) && (resp_len != NULL)
			&& (*resp_len != 0U)) {
			*(resp_buf + total_resp_len)
					= resp_data;
			*resp_len = *resp_len - 1;
			total_resp_len++;
		}
		rout %= MBOX_RESP_BUFFER_SIZE;
		mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);

		do {
			rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
			if (rout == rin) {
				mdelay(10U);
			} else {
				break;
			}
			timeout--;
		} while ((mbox_resp_len > 0U) && (timeout != 0U));

		if (timeout == 0U) {
			INFO("Timed out waiting for SDM\n");
			return MBOX_TIMEOUT;
		}
	}

	if (resp_len)
		*resp_len = total_resp_len;

	return MBOX_RET_OK;
}

int mailbox_send_cmd_async_ext(uint32_t header_cmd, uint32_t *args,
			unsigned int len)
{
	return fill_mailbox_circular_buffer(header_cmd, args, len);
}

int mailbox_send_cmd_async(uint32_t *job_id, uint32_t cmd, uint32_t *args,
			  unsigned int len, unsigned int indirect)
{
	int status;

	status = fill_mailbox_circular_buffer(
				MBOX_CLIENT_ID_CMD(MBOX_ATF_CLIENT_ID) |
				MBOX_JOB_ID_CMD(*job_id) |
				MBOX_CMD_LEN_CMD(len) |
				MBOX_INDIRECT(indirect) |
				cmd, args, len);
	if (status < 0) {
		return status;
	}

	*job_id = (*job_id + 1U) % MBOX_MAX_IND_JOB_ID;

	return MBOX_RET_OK;
}

int mailbox_send_cmd(uint32_t job_id, uint32_t cmd, uint32_t *args,
			unsigned int len, uint32_t urgent, uint32_t *response,
			unsigned int *resp_len)
{
	int status = 0;

	if (urgent != 0U) {
		urgent |= mmio_read_32(MBOX_OFFSET + MBOX_STATUS) &
					MBOX_STATUS_UA_MASK;
		mmio_write_32(MBOX_OFFSET + MBOX_URG, cmd);
		mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_TO_SDM, 1U);
	}

	else {
		status = fill_mailbox_circular_buffer(
			MBOX_CLIENT_ID_CMD(MBOX_ATF_CLIENT_ID) |
			MBOX_JOB_ID_CMD(job_id) |
			MBOX_CMD_LEN_CMD(len) |
			cmd, args, len);
	}

	if (status != 0) {
		return status;
	}

	status = mailbox_poll_response(job_id, urgent, response, resp_len);

	return status;
}

void mailbox_clear_response(void)
{
	mmio_write_32(MBOX_OFFSET + MBOX_ROUT,
		mmio_read_32(MBOX_OFFSET + MBOX_RIN));
}

void mailbox_set_int(uint32_t interrupt)
{

	mmio_write_32(MBOX_OFFSET+MBOX_INT, MBOX_COE_BIT(interrupt) |
			MBOX_UAE_BIT(interrupt));
}


void mailbox_set_qspi_open(void)
{
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_OPEN, NULL, 0U,
				CMD_CASUAL, NULL, NULL);
}

void mailbox_set_qspi_direct(void)
{
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_DIRECT, NULL, 0U,
				CMD_CASUAL, NULL, NULL);
}

void mailbox_set_qspi_close(void)
{
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_CLOSE, NULL, 0U,
				CMD_CASUAL, NULL, NULL);
}

void mailbox_qspi_set_cs(uint32_t device_select)
{
	uint32_t cs_setting;

	/* QSPI device select settings at 31:28 */
	cs_setting = (device_select << 28);
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_SET_CS, &cs_setting,
				1U, CMD_CASUAL, NULL, NULL);
}

void mailbox_hps_qspi_enable(void)
{
	mailbox_set_qspi_open();
	mailbox_set_qspi_direct();
}

void mailbox_reset_cold(void)
{
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_REBOOT_HPS, NULL, 0U,
				CMD_CASUAL, NULL, NULL);
}

int mailbox_rsu_get_spt_offset(uint32_t *resp_buf, unsigned int resp_buf_len)
{
	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_GET_SUBPARTITION_TABLE,
				NULL, 0U, CMD_CASUAL, resp_buf,
				&resp_buf_len);
}

struct rsu_status_info {
	uint64_t current_image;
	uint64_t fail_image;
	uint32_t state;
	uint32_t version;
	uint32_t error_location;
	uint32_t error_details;
	uint32_t retry_counter;
};

int mailbox_rsu_status(uint32_t *resp_buf, unsigned int resp_buf_len)
{
	int ret;
	struct rsu_status_info *info = (struct rsu_status_info *)resp_buf;

	info->retry_counter = ~0U;

	ret = mailbox_send_cmd(MBOX_JOB_ID, MBOX_RSU_STATUS, NULL, 0U,
				CMD_CASUAL, resp_buf,
				&resp_buf_len);

	if (ret < 0) {
		return ret;
	}

	if (info->retry_counter != ~0U) {
		if ((info->version & RSU_VERSION_ACMF_MASK) == 0U) {
			info->version |= RSU_VERSION_ACMF;
		}
	}

	return ret;
}

int mailbox_rsu_update(uint32_t *flash_offset)
{
	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_RSU_UPDATE,
				flash_offset, 2U,
				CMD_CASUAL, NULL, NULL);
}

int mailbox_hps_stage_notify(uint32_t execution_stage)
{
	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_HPS_STAGE_NOTIFY,
				&execution_stage, 1U, CMD_CASUAL,
				NULL, NULL);
}

int mailbox_init(void)
{
	int status;

	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE |
			MBOX_INT_FLAG_UAE);
	mmio_write_32(MBOX_OFFSET + MBOX_URG, 0U);
	mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM, 0U);

	status = mailbox_send_cmd(0U, MBOX_CMD_RESTART, NULL, 0U,
					CMD_URGENT, NULL, NULL);

	if (status != 0) {
		return status;
	}

	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE |
			MBOX_INT_FLAG_UAE);

	return MBOX_RET_OK;
}

int intel_mailbox_get_config_status(uint32_t cmd, bool init_done)
{
	int status;
	uint32_t res, response[6];
	unsigned int resp_len = ARRAY_SIZE(response);

	status = mailbox_send_cmd(MBOX_JOB_ID, cmd, NULL, 0U, CMD_CASUAL,
				response, &resp_len);

	if (status < 0) {
		return status;
	}

	res = response[RECONFIG_STATUS_STATE];
	if ((res != 0U) && (res != MBOX_CFGSTAT_STATE_CONFIG)) {
		return res;
	}

	res = response[RECONFIG_STATUS_PIN_STATUS];
	if ((res & PIN_STATUS_NSTATUS) == 0U) {
		return MBOX_CFGSTAT_STATE_ERROR_HARDWARE;
	}

	res = response[RECONFIG_STATUS_SOFTFUNC_STATUS];
	if ((res & SOFTFUNC_STATUS_SEU_ERROR) != 0U) {
		return MBOX_CFGSTAT_STATE_ERROR_HARDWARE;
	}

	if ((res & SOFTFUNC_STATUS_CONF_DONE) == 0U) {
		return MBOX_CFGSTAT_STATE_CONFIG;
	}

	if (init_done && (res & SOFTFUNC_STATUS_INIT_DONE) == 0U) {
		return MBOX_CFGSTAT_STATE_CONFIG;
	}

	return MBOX_RET_OK;
}

int intel_mailbox_is_fpga_not_ready(void)
{
	int ret = intel_mailbox_get_config_status(MBOX_RECONFIG_STATUS, true);

	if ((ret != MBOX_RET_OK) && (ret != MBOX_CFGSTAT_STATE_CONFIG)) {
		ret = intel_mailbox_get_config_status(MBOX_CONFIG_STATUS,
							false);
	}

	return ret;
}

int mailbox_hwmon_readtemp(uint32_t chan, uint32_t *resp_buf)
{
	unsigned int resp_len = sizeof(resp_buf);

	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_HWMON_READTEMP, &chan, 1U,
				CMD_CASUAL, resp_buf,
				&resp_len);

}

int mailbox_hwmon_readvolt(uint32_t chan, uint32_t *resp_buf)
{
	unsigned int resp_len = sizeof(resp_buf);

	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_HWMON_READVOLT, &chan, 1U,
				CMD_CASUAL, resp_buf,
				&resp_len);
}
