/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>

#include "socfpga_mailbox.h"
#include "socfpga_sip_svc.h"

static int fill_mailbox_circular_buffer(uint32_t header_cmd, uint64_t *args,
					int len)
{
	uint32_t cmd_free_offset;
	int i;

	cmd_free_offset = mmio_read_32(MBOX_OFFSET + MBOX_CIN);

	mmio_write_32(MBOX_OFFSET + MBOX_CMD_BUFFER + (cmd_free_offset++ * 4),
			header_cmd);


	for (i = 0; i < len; i++) {
		cmd_free_offset %= MBOX_CMD_BUFFER_SIZE;
		mmio_write_32(MBOX_OFFSET + MBOX_CMD_BUFFER +
				(cmd_free_offset++ * 4), args[i]);
	}

	cmd_free_offset %= MBOX_CMD_BUFFER_SIZE;
	mmio_write_32(MBOX_OFFSET + MBOX_CIN, cmd_free_offset);

	return 0;
}

int mailbox_read_response(int job_id, uint32_t *response, int resp_len)
{
	int rin = 0;
	int rout = 0;
	int response_length = 0;
	int resp = 0;
	int total_resp_len = 0;

	if (mmio_read_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM))
		mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM, 0);

	rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
	rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);

	if (rout != rin) {
		resp = mmio_read_32(MBOX_OFFSET +
				    MBOX_RESP_BUFFER + ((rout++)*4));

		rout %= MBOX_RESP_BUFFER_SIZE;
		mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);

		if (MBOX_RESP_CLIENT_ID(resp) != MBOX_ATF_CLIENT_ID ||
		   MBOX_RESP_JOB_ID(resp) != job_id) {
			return MBOX_WRONG_ID;
		}

		if (MBOX_RESP_ERR(resp) > 0) {
			INFO("Error in response: %x\n", resp);
			return -resp;
		}
		response_length = MBOX_RESP_LEN(resp);

		while (response_length) {

			response_length--;
			resp = mmio_read_32(MBOX_OFFSET +
						MBOX_RESP_BUFFER +
						(rout)*4);
			if (response && resp_len) {
				*(response + total_resp_len) = resp;
				resp_len--;
				total_resp_len++;
			}
			rout++;
			rout %= MBOX_RESP_BUFFER_SIZE;
			mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);
		}
		return total_resp_len;
	}

	return MBOX_NO_RESPONSE;
}


int mailbox_poll_response(int job_id, int urgent, uint32_t *response,
				int resp_len)
{
	int timeout = 0xFFFFFF;
	int rin = 0;
	int rout = 0;
	int response_length = 0;
	int resp = 0;
	int total_resp_len = 0;

	while (1) {

		while (timeout > 0 &&
			!(mmio_read_32(MBOX_OFFSET +
				MBOX_DOORBELL_FROM_SDM) & 1)) {
			timeout--;
		}

		if (!timeout) {
			INFO("Timed out waiting for SDM");
			return MBOX_TIMEOUT;
		}

		mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM, 0);

		if (urgent & 1) {
			mdelay(5);
			if ((mmio_read_32(MBOX_OFFSET + MBOX_STATUS) &
				MBOX_STATUS_UA_MASK) ^
				(urgent & MBOX_STATUS_UA_MASK)) {
				mmio_write_32(MBOX_OFFSET + MBOX_URG, 0);
				return 0;
			}

			mmio_write_32(MBOX_OFFSET + MBOX_URG, 0);
			INFO("Error: Mailbox did not get UA");
			return -1;
		}

		rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
		rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);

		while (rout != rin) {
			resp = mmio_read_32(MBOX_OFFSET +
					    MBOX_RESP_BUFFER + ((rout++)*4));

			rout %= MBOX_RESP_BUFFER_SIZE;
			mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);

			if (MBOX_RESP_CLIENT_ID(resp) != MBOX_ATF_CLIENT_ID ||
			   MBOX_RESP_JOB_ID(resp) != job_id)
				continue;

			if (MBOX_RESP_ERR(resp) > 0) {
				INFO("Error in response: %x\n", resp);
				return -MBOX_RESP_ERR(resp);
			}
			response_length = MBOX_RESP_LEN(resp);

			while (response_length) {
				response_length--;
				resp = mmio_read_32(MBOX_OFFSET +
							MBOX_RESP_BUFFER +
							(rout)*4);
				if (response && resp_len) {
					*(response + total_resp_len) = resp;
					resp_len--;
					total_resp_len++;
				}
				rout++;
				rout %= MBOX_RESP_BUFFER_SIZE;
				mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);
			}
			return total_resp_len;
		}
	}
}

int mailbox_send_cmd_async(int job_id, unsigned int cmd, uint64_t *args,
			  int len, int urgent)
{
	if (urgent)
		mmio_write_32(MBOX_OFFSET + MBOX_URG, 1);

	fill_mailbox_circular_buffer(MBOX_CLIENT_ID_CMD(MBOX_ATF_CLIENT_ID) |
					MBOX_JOB_ID_CMD(job_id) |
					MBOX_CMD_LEN_CMD(len) |
					MBOX_INDIRECT |
					cmd, args, len);

	mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_TO_SDM, 1);

	return 0;
}

int mailbox_send_cmd(int job_id, unsigned int cmd, uint64_t *args,
			int len, int urgent, uint32_t *response, int resp_len)
{
	int status = 0;

	if (urgent) {
		urgent |= mmio_read_32(MBOX_OFFSET + MBOX_STATUS) &
					MBOX_STATUS_UA_MASK;
		mmio_write_32(MBOX_OFFSET + MBOX_URG, cmd);
	}

	else {
		status = fill_mailbox_circular_buffer(
			MBOX_CLIENT_ID_CMD(MBOX_ATF_CLIENT_ID) |
			MBOX_JOB_ID_CMD(job_id) |
			MBOX_CMD_LEN_CMD(len) |
			cmd, args, len);
	}

	if (status)
		return status;

	mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_TO_SDM, 1);
	status = mailbox_poll_response(job_id, urgent, response, resp_len);

	return status;
}

void mailbox_clear_response(void)
{
	mmio_write_32(MBOX_OFFSET + MBOX_ROUT,
		mmio_read_32(MBOX_OFFSET + MBOX_RIN));
}

void mailbox_set_int(int interrupt)
{

	mmio_write_32(MBOX_OFFSET+MBOX_INT, MBOX_COE_BIT(interrupt) |
			MBOX_UAE_BIT(interrupt));
}


void mailbox_set_qspi_open(void)
{
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_OPEN, 0, 0, 0, NULL, 0);
}

void mailbox_set_qspi_direct(void)
{
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_DIRECT, 0, 0, 0, NULL, 0);
}

void mailbox_set_qspi_close(void)
{
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_CLOSE, 0, 0, 0, NULL, 0);
}

int mailbox_get_qspi_clock(void)
{
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_DIRECT, 0, 0, 0,
				NULL, 0);
}

void mailbox_qspi_set_cs(int device_select)
{
	uint64_t cs_setting = device_select;

	/* QSPI device select settings at 31:28 */
	cs_setting = (cs_setting << 28);
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_SET_CS, &cs_setting,
				1, 0, NULL, 0);
}

void mailbox_reset_cold(void)
{
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_REBOOT_HPS, 0, 0, 0, NULL, 0);
}

int mailbox_rsu_get_spt_offset(uint32_t *resp_buf, uint32_t resp_buf_len)
{
	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_GET_SUBPARTITION_TABLE,
				NULL, 0, 0, (uint32_t *)resp_buf,
				resp_buf_len);
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

int mailbox_rsu_status(uint32_t *resp_buf, uint32_t resp_buf_len)
{
	int ret;
	struct rsu_status_info *info = (struct rsu_status_info *)resp_buf;

	info->retry_counter = ~0;

	ret = mailbox_send_cmd(MBOX_JOB_ID, MBOX_RSU_STATUS, NULL, 0, 0,
			       (uint32_t *)resp_buf, resp_buf_len);

	if (ret < 0)
		return ret;

	if (info->retry_counter != ~0)
		if (!(info->version & RSU_VERSION_ACMF_MASK))
			info->version |= RSU_VERSION_ACMF;

	return ret;
}

int mailbox_rsu_update(uint64_t *flash_offset)
{
	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_RSU_UPDATE,
				flash_offset, 2, 0, NULL, 0);
}

int mailbox_hps_stage_notify(uint64_t execution_stage)
{
	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_HPS_STAGE_NOTIFY,
				&execution_stage, 1, 0, NULL, 0);
}

int mailbox_init(void)
{
	int status = 0;

	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE |
			MBOX_INT_FLAG_UAE);
	mmio_write_32(MBOX_OFFSET + MBOX_URG, 0);
	mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM, 0);

	status = mailbox_send_cmd(0, MBOX_CMD_RESTART, 0, 0, 1, NULL, 0);

	if (status)
		return status;

	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE |
			MBOX_INT_FLAG_UAE);

	return 0;
}

int intel_mailbox_get_config_status(uint32_t cmd)
{
	int status;
	uint32_t res, response[6];

	status = mailbox_send_cmd(1, cmd, NULL, 0, 0, response,
		sizeof(response) / sizeof(response[0]));

	if (status < 0)
		return status;

	res = response[RECONFIG_STATUS_STATE];
	if (res && res != MBOX_CFGSTAT_STATE_CONFIG)
		return res;

	res = response[RECONFIG_STATUS_PIN_STATUS];
	if (!(res & PIN_STATUS_NSTATUS))
		return MBOX_CFGSTAT_STATE_ERROR_HARDWARE;

	res = response[RECONFIG_STATUS_SOFTFUNC_STATUS];
	if (res & SOFTFUNC_STATUS_SEU_ERROR)
		return MBOX_CFGSTAT_STATE_ERROR_HARDWARE;

	if ((res & SOFTFUNC_STATUS_CONF_DONE) &&
		(res & SOFTFUNC_STATUS_INIT_DONE))
		return 0;

	return MBOX_CFGSTAT_STATE_CONFIG;
}

int intel_mailbox_is_fpga_not_ready(void)
{
	int ret = intel_mailbox_get_config_status(MBOX_RECONFIG_STATUS);

	if (ret && ret != MBOX_CFGSTAT_STATE_CONFIG)
		ret = intel_mailbox_get_config_status(MBOX_CONFIG_STATUS);

	return ret;
}
