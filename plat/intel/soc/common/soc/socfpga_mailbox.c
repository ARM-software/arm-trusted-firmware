/*
 * Copyright (c) 2020-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <platform_def.h>

#include "socfpga_mailbox.h"
#include "socfpga_plat_def.h"
#include "socfpga_private.h"
#include "socfpga_sip_svc.h"
#include "socfpga_system_manager.h"

#if SIP_SVC_V3
/* Function prototypes */
void mailbox_init_v3(void);
static int mailbox_response_handler_fsm(void);
static inline void mailbox_free_cmd_desc(sdm_command_t *cmd_desc);
static sdm_response_t *mailbox_get_resp_desc(uint8_t client_id, uint8_t job_id,
					     uint8_t *index);
static sdm_command_t *mailbox_get_cmd_desc(uint8_t client_id, uint8_t job_id);
static inline void mailbox_free_resp_desc(uint8_t index);
static sdm_command_t *mailbox_get_free_cmd_desc(void);
static sdm_response_t *mailbox_get_resp_desc_cid(uint8_t client_id,
						 uint8_t *index);
static int mailbox_read_response_v3(uint8_t client_id, uint8_t *job_id,
				    uint32_t *header, uint32_t *resp,
				    uint32_t *resp_len,
				    uint8_t ignore_client_id);
static int mailbox_poll_response_v3(uint8_t client_id, uint8_t job_id,
				    uint32_t *resp, unsigned int *resp_len,
				    uint32_t urgent);

static spinlock_t mbox_db_lock;		/* Mailbox service data base lock */
static spinlock_t mbox_write_lock;	/* Hardware mailbox FIFO write lock */
static spinlock_t mbox_read_lock;	/* Hardware mailbox FIFO read lock */

static mailbox_service_t mbox_svc;	/* Mailbox service data base */
static uint8_t async_v1_job_id;
#endif /* #if SIP_SVC_V3 */

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
	unsigned int timeout = 20000U;

	do {
		if (is_mailbox_cmdbuf_empty(cin)) {
			break;
		}
		udelay(50U);
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
	unsigned int timeout = 20000U;

	VERBOSE("MBOX: 0x%x\n", data);

	do {
		if (is_mailbox_cmdbuf_full(*cin)) {
			if (!(*is_doorbell_triggered)) {
				mmio_write_32(MBOX_OFFSET +
					      MBOX_DOORBELL_TO_SDM, 1U);
				*is_doorbell_triggered = true;
			}
			udelay(50U);
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

/*
 * Function description: Write the command header, and its payload one by one
 * into the mailbox command buffer. Along with this, check for mailbox buffer
 * full condition and trigger doorbell to SDM if the command buffer is full.
 */
static int fill_mailbox_circular_buffer(uint32_t header_cmd, uint32_t *args,
					unsigned int len)
{
	uint32_t sdm_read_offset, cmd_free_offset;
	unsigned int i;
	int ret;
	bool is_doorbell_triggered = false;

#if SIP_SVC_V3
	spin_lock(&mbox_write_lock);
#endif

	cmd_free_offset = mmio_read_32(MBOX_OFFSET + MBOX_CIN);
	sdm_read_offset = mmio_read_32(MBOX_OFFSET + MBOX_COUT);

	/* Write the command header here */
	ret = write_mailbox_cmd_buffer(&cmd_free_offset, sdm_read_offset,
				       header_cmd, &is_doorbell_triggered);
	if (ret != 0) {
		goto restart_mailbox;
	}

	/* Write the payload here w.r.to args and its len - one by one. */
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

#if SIP_SVC_V3
	spin_unlock(&mbox_write_lock);
#endif
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

#if SIP_SVC_V3
	spin_unlock(&mbox_write_lock);
#endif
	return MBOX_TIMEOUT;
}

int mailbox_read_response(unsigned int *job_id, uint32_t *response,
				unsigned int *resp_len)
{
#if SIP_SVC_V3
	return mailbox_read_response_v3(MBOX_ATF_CLIENT_ID,
					(uint8_t *)job_id, NULL,
					response, resp_len,
					0);
#else
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
			INFO("SDM response: Return Code: 0x%x\n", MBOX_RESP_ERR(resp_data));
			return -MBOX_RESP_ERR(resp_data);
		}

		return MBOX_RET_OK;
	}

	return MBOX_NO_RESPONSE;
#endif
}

int mailbox_read_response_async(unsigned int *job_id, uint32_t *header,
				uint32_t *response, unsigned int *resp_len,
				uint8_t ignore_client_id)
{
#if SIP_SVC_V3
	/* Just to avoid the build warning */
	(void)mailbox_resp_ctr;
	return mailbox_read_response_v3(MBOX_ATF_CLIENT_ID,
					(uint8_t *)job_id, header,
					response, resp_len,
					ignore_client_id);
#else
	uint32_t rin;
	uint32_t rout;
	uint32_t resp_data;
	uint32_t ret_resp_len = 0;
	uint8_t is_done = 0;
	uint32_t resp_len_check = 0;

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
		if ((ret_resp_len > 0) && (response != NULL) && (resp_len != NULL)) {
			if (*resp_len > ret_resp_len) {
				*resp_len = ret_resp_len;
			}

			resp_len_check = (uint32_t) *resp_len;

			if (resp_len_check > MBOX_DATA_MAX_LEN) {
				return MBOX_RET_ERROR;
			}

			memcpy_s((uint8_t *) response, *resp_len * MBOX_WORD_BYTE,
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
#endif
}

int mailbox_poll_response(uint32_t job_id, uint32_t urgent, uint32_t *response,
			  unsigned int *resp_len)
{
#if SIP_SVC_V3
	return mailbox_poll_response_v3(MBOX_ATF_CLIENT_ID, (uint8_t)job_id,
					response, resp_len, urgent);
#else
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
			ERROR("MBOX: Mailbox did not get UA");
			return MBOX_RET_ERROR;
		}

		rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
		rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);

		while (rout != rin) {
			resp_data = mmio_read_32(MBOX_ENTRY_TO_ADDR(RESP,
								(rout)++));

			rout %= MBOX_RESP_BUFFER_SIZE;
			mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);

			if ((MBOX_RESP_CLIENT_ID(resp_data) != MBOX_ATF_CLIENT_ID) ||
			    (MBOX_RESP_JOB_ID(resp_data) != job_id)) {
				continue;
			}

			/* Get the return response len from the response header. */
			ret_resp_len = MBOX_RESP_LEN(resp_data);

			/* Print the response header. */
			VERBOSE("MBOX: RespHdr: cid %d, jid %d, len %d, err_code 0x%x\n",
				MBOX_RESP_CLIENT_ID(resp_data),
				MBOX_RESP_JOB_ID(resp_data),
				MBOX_RESP_LEN(resp_data),
				MBOX_RESP_ERR(resp_data));

			if (iterate_resp(ret_resp_len, response, resp_len)
				!= MBOX_RET_OK) {
				return MBOX_TIMEOUT;
			}

			if (MBOX_RESP_ERR(resp_data) > 0U) {
				INFO("SDM response: Return Code: 0x%x\n", MBOX_RESP_ERR(resp_data));
				return -MBOX_RESP_ERR(resp_data);
			}

			return MBOX_RET_OK;
		}

	sdm_loop--;
	}

	INFO("Timed out waiting for SDM\n");
	return MBOX_TIMEOUT;
#endif
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
			*(resp_buf + total_resp_len) = resp_data;
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

#if SIP_SVC_V3
	async_v1_job_id = (uint8_t)*job_id;
#endif
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
	} else {
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
	mmio_write_32(MBOX_OFFSET+MBOX_INT,
			MBOX_COE_BIT(interrupt) |
			MBOX_RIE_BIT(interrupt) |
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
	uint32_t response[1], qspi_clk, reg;
	unsigned int resp_len = ARRAY_SIZE(response);

	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_QSPI_DIRECT, NULL, 0U,
			 CMD_CASUAL, response, &resp_len);

	qspi_clk = response[0];
	INFO("QSPI ref clock: %u\n", qspi_clk);

	/*
	 * Store QSPI ref clock frequency in BOOT_SCRATCH_COLD_0 register for
	 * later boot loader (i.e. u-boot) use.
	 * The frequency is stored in kHz and occupies BOOT_SCRATCH_COLD_0
	 * register bits[27:0].
	 */
	qspi_clk /= 1000;
	reg = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_0));
	reg &= ~SYSMGR_QSPI_REFCLK_MASK;
	reg |= qspi_clk & SYSMGR_QSPI_REFCLK_MASK;
	mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_0), reg);
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

	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_REBOOT_HPS, 0U, 0U,
				 CMD_CASUAL, NULL, NULL);
}

void mailbox_reset_warm(uint32_t reset_type)
{
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);

	reset_type = 0x01; // Warm reset header data must be 1
	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_REBOOT_HPS, &reset_type, 1U,
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

int mailbox_rsu_get_device_info(uint32_t *resp_buf, unsigned int resp_buf_len)
{
	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_RSU_GET_DEVICE_INFO, NULL, 0U,
				CMD_CASUAL, resp_buf,
				&resp_buf_len);
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

#if SIP_SVC_V3
	/* Initialize the mailbox version3 implementation, and in V3 we
	 * are interested in only RIE interrupt
	 */
	mailbox_init_v3();
	mailbox_set_int(MBOX_INT_FLAG_RIE);
#else
	mailbox_set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE |
			MBOX_INT_FLAG_UAE);
#endif

	return MBOX_RET_OK;
}

int mailbox_send_fpga_config_comp(void)
{
	int ret;

	ret = mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_FPGA_CONFIG_COMP, NULL, 0U,
				 CMD_CASUAL, NULL, NULL);

	if (ret != 0) {
		INFO("SOCFPGA: FPGA configuration complete response, Return Code: 0x%x\n",
			MBOX_RESP_ERR(-ret));
		return MBOX_RET_ERROR;
	}

	return MBOX_RET_OK;
}

int intel_mailbox_get_config_status(uint32_t cmd, bool init_done,
				    uint32_t *err_states)
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

	if (err_states != NULL)
		*err_states = res;

	if (res == MBOX_CFGSTAT_VAB_BS_PREAUTH) {
		return MBOX_CFGSTAT_STATE_CONFIG;
	}

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
	int ret = intel_mailbox_get_config_status(MBOX_RECONFIG_STATUS, true, NULL);

	if ((ret != MBOX_RET_OK) && (ret != MBOX_CFGSTAT_STATE_CONFIG)) {
		ret = intel_mailbox_get_config_status(MBOX_CONFIG_STATUS,
							false, NULL);
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

int mailbox_seu_err_status(uint32_t *resp_buf, unsigned int resp_buf_len)
{

	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_SEU_ERR_READ, NULL, 0U,
				CMD_CASUAL, resp_buf,
				&resp_buf_len);
}

int mailbox_safe_inject_seu_err(uint32_t *arg, unsigned int len)
{
	return mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_SAFE_INJECT_SEU_ERR, arg, len,
			CMD_CASUAL, NULL, NULL);
}

#if SIP_SVC_V3
static int mailbox_fill_cmd_desc(uint8_t client_id, uint8_t job_id,
				 uint32_t *resp_buff)
{
	sdm_command_t *cmd_desc = NULL;

	/* Get a free command descriptor */
	cmd_desc = mailbox_get_free_cmd_desc();
	if (cmd_desc == NULL) {
		return MBOX_BUFFER_FULL;
	}

	/* Record all the given values for the command. */
	cmd_desc->client_id = client_id;
	cmd_desc->job_id = job_id;
	cmd_desc->cb = NULL;
	cmd_desc->cb_args = resp_buff;
	cmd_desc->cb_args_len = 0U;

	return MBOX_RET_OK;
}

/* Returns the command descriptor based on the client and job ID. */
static sdm_command_t *mailbox_get_cmd_desc(uint8_t client_id, uint8_t job_id)
{
	spin_lock(&mbox_db_lock);
	for (uint32_t count = 0; count < MBOX_SVC_CMD_QUEUE_SIZE; count++) {
		if ((mbox_svc.cmd_queue[count].client_id == client_id) &&
		    (mbox_svc.cmd_queue[count].job_id == job_id) &&
		    (mbox_svc.cmd_queue[count].flags & MBOX_SVC_CMD_IS_USED)) {
			spin_unlock(&mbox_db_lock);
			return &(mbox_svc.cmd_queue[count]);
		}
	}

	spin_unlock(&mbox_db_lock);
	VERBOSE("MBOX: Command descriptor not found for cid %d, jid %d\n",
		client_id, job_id);

	return NULL;
}

/* Returns the response descriptor based on only client ID. */
static sdm_response_t *mailbox_get_resp_desc_cid(uint8_t client_id, uint8_t *index)
{
	spin_lock(&mbox_db_lock);

	for (uint32_t count = 0; count < MBOX_SVC_RESP_QUEUE_SIZE; count++) {
		if ((mbox_svc.resp_queue[count].client_id == client_id) &&
		    (mbox_svc.resp_queue[count].flags & FLAG_SDM_RESPONSE_IS_VALID)) {
			*index = count;
			/*
			 * Once we get the valid response descriptor, get the
			 * job ID and mark up the bitmaps.
			 */
			uint8_t job_id = mbox_svc.resp_queue[count].job_id;
			uint8_t transaction_id = MBOX_GET_TRANS_ID(client_id, job_id);

			mbox_svc.received_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &=
				~(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE));
			mbox_svc.interrupt_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &=
				~(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE));
			spin_unlock(&mbox_db_lock);
			return &(mbox_svc.resp_queue[count]);
		}
	}

	spin_unlock(&mbox_db_lock);
	VERBOSE("MBOX: Response descriptor not found for cid %d\n", client_id);

	return NULL;
}

/* Returns the response descriptor based on the client and job ID. */
static sdm_response_t *mailbox_get_resp_desc(uint8_t client_id, uint8_t job_id, uint8_t *index)
{
	spin_lock(&mbox_db_lock);
	/*
	 * Let's first check whether we have a response bitmap set for the given
	 * client ID and job ID.
	 */
	uint8_t transaction_id = MBOX_GET_TRANS_ID(client_id, job_id);

	if ((mbox_svc.received_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &
		(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE))) == 0) {
		spin_unlock(&mbox_db_lock);
		VERBOSE("MBOX: Response bitmap not set for cid %d, jid %d, bitmap 0x%16lx\n",
			client_id, job_id, mbox_svc.received_bitmap[transaction_id / 64]);
		return NULL;
	}

	for (uint32_t count = 0; count < MBOX_SVC_RESP_QUEUE_SIZE; count++) {
		if (mbox_svc.resp_queue[count].flags & FLAG_SDM_RESPONSE_IS_VALID) {
			if ((mbox_svc.resp_queue[count].client_id == client_id) &&
			    (mbox_svc.resp_queue[count].job_id == job_id)) {
				*index = count;
				mbox_svc.received_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &=
					~(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE));
				mbox_svc.interrupt_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &=
					~(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE));
				spin_unlock(&mbox_db_lock);
				return &(mbox_svc.resp_queue[count]);
			}
		}
	}

	spin_unlock(&mbox_db_lock);
	VERBOSE("MBOX: Response descriptor not found for cid %d, jid %d\n",
		client_id, job_id);

	return NULL;
}

static int32_t mailbox_get_free_resp_desc(void)
{
	spin_lock(&mbox_db_lock);
	static uint32_t free_index = MBOX_SVC_RESP_QUEUE_SIZE - 1;
	uint32_t count = 0U, try = 0U;

	for (try = 0; try < MBOX_SVC_RESP_QUEUE_SIZE; try++) {
		free_index = (free_index + 1) % MBOX_SVC_RESP_QUEUE_SIZE;
		if (!(mbox_svc.resp_queue[free_index].flags &
			FLAG_SDM_RESPONSE_IS_USED)) {
			count = free_index;
			spin_unlock(&mbox_db_lock);
			return count;
		}
	}

	/* No free descriptors are available */
	spin_unlock(&mbox_db_lock);
	VERBOSE("MBOX: No free response descriptors are available\n");

	return MBOX_BUFFER_FULL;
}

static sdm_command_t *mailbox_get_free_cmd_desc(void)
{
	spin_lock(&mbox_db_lock);
	static uint32_t free_index;

	/* Rollover the command queue free index */
	if (free_index == (MBOX_SVC_CMD_QUEUE_SIZE - 1)) {
		free_index = 0U;
	}

	for (; free_index < MBOX_SVC_CMD_QUEUE_SIZE; free_index++) {
		if (!(mbox_svc.cmd_queue[free_index].flags &
			MBOX_SVC_CMD_IS_USED)) {
			mbox_svc.cmd_queue[free_index].flags |= MBOX_SVC_CMD_IS_USED;
			spin_unlock(&mbox_db_lock);
			return &(mbox_svc.cmd_queue[free_index]);
		}
	}

	/* No free command descriptors are available */
	spin_unlock(&mbox_db_lock);
	VERBOSE("MBOX: No free command descriptors available\n");

	return NULL;
}

static inline void mailbox_free_cmd_desc(sdm_command_t *cmd_desc)
{
	if (cmd_desc == NULL) {
		return;
	}

	spin_lock(&mbox_db_lock);
	memset((void *)cmd_desc, 0, sizeof(sdm_command_t));
	spin_unlock(&mbox_db_lock);
}

static inline void mailbox_free_resp_desc(uint8_t index)
{
	if (index >= MBOX_SVC_RESP_QUEUE_SIZE) {
		return;
	}

	spin_lock(&mbox_db_lock);
	memset((void *)&mbox_svc.resp_queue[index], 0, sizeof(sdm_response_t));
	spin_unlock(&mbox_db_lock);
}

/*
 * This function serves the V1 _sync_read and _async_read functionality, and it
 * is introduced as part of V3 framework to keep backward compatible with V1
 * clients.
 */
static int mailbox_read_response_v3(uint8_t client_id, uint8_t *job_id,
				    uint32_t *header, uint32_t *resp,
				    uint32_t *resp_len,
				    uint8_t ignore_client_id)
{
	uint8_t di = 0U;
	int status = MBOX_RET_OK;
	sdm_response_t *resp_desc = NULL;
	sdm_command_t *cmd_desc = NULL;

	/*
	 * In the V1, the client ID is always MBOX_ATF_CLIENT_ID and in this
	 * routine we will collect the response which only belongs to this
	 * client ID. So safe to ignore this input.
	 */
	(void)ignore_client_id;

	/* Clear the SDM doorbell interrupt */
	if (mmio_read_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM) == 1U)
		mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM, 0U);

	/* Fill the command descriptor index and get the same */
	status = mailbox_fill_cmd_desc(client_id, async_v1_job_id, resp);
	if (status != MBOX_RET_OK) {
		return status;
	}

	cmd_desc = mailbox_get_cmd_desc(client_id, async_v1_job_id);

	/* Get the response from SDM, just go through one cycle */
	status = mailbox_response_handler_fsm();
	if (status != MBOX_RET_OK) {
		mailbox_free_cmd_desc(cmd_desc);
		*resp_len = 0U;
		return status;
	}

	/* Check the local response queue with the given client ID */
	resp_desc = mailbox_get_resp_desc_cid(client_id, &di);
	if (resp_desc == NULL) {
		mailbox_free_cmd_desc(cmd_desc);
		*resp_len = 0U;
		return MBOX_NO_RESPONSE;
	}

	/* Update the received mailbox response length, job ID and header */
	*job_id = resp_desc->job_id;
	*resp_len = resp_desc->rcvd_resp_len;
	if (header != NULL) {
		*header = resp_desc->header;
	}

	/* Check the mailbox response error code */
	if (MBOX_RESP_ERR(resp_desc->header) > 0U) {
		INFO("MBOX: Error in async response: %x\n", resp_desc->header);
		status = -MBOX_RESP_ERR(resp_desc->header);
	}

	/* Free up the response and command descriptors */
	mailbox_free_resp_desc(di);
	mailbox_free_cmd_desc(cmd_desc);

	return status;
}

int mailbox_send_cmd_async_v3(uint8_t client_id, uint8_t job_id, uint32_t cmd,
			      uint32_t *args, uint32_t args_len, uint8_t cmd_flag,
			      sdm_command_callback cb, uint32_t *cb_args,
			      uint32_t cb_args_len)
{
	int status = 0;
	sdm_command_t *cmd_desc = NULL;

	VERBOSE("MBOX: cid: %d, jid: %d, cmd: %d, cmd_flag: %d\n",
		client_id, job_id, cmd, cmd_flag);

	if (IS_CMD_SET(cmd_flag, URGENT)) {
		mmio_write_32(MBOX_OFFSET + MBOX_URG, cmd);
		mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_TO_SDM, 1U);
	} else {
		/* Get a free command descriptor */
		cmd_desc = mailbox_get_free_cmd_desc();
		if (cmd_desc == NULL) {
			return MBOX_BUFFER_FULL;
		}

		/* Record all the given values for the command. */
		cmd_desc->client_id = client_id;
		cmd_desc->job_id = job_id;
		cmd_desc->cb = cb;
		cmd_desc->cb_args = cb_args;
		cmd_desc->cb_args_len = cb_args_len;

		/* Push the command to mailbox FIFO */
		status = fill_mailbox_circular_buffer(
					MBOX_FRAME_CMD_HEADER(client_id, job_id,
					args_len, IS_CMD_SET(cmd_flag, INDIRECT), cmd),
					args,
					args_len);

		if (status != MBOX_RET_OK) {
			INFO("MBOX: Failed to push the command to mailbox FIFO\n");
			/* Free the command descriptor. */
			mailbox_free_cmd_desc(cmd_desc);
		}
	}

	INFO("MBOX: %s: status: %d\n", __func__, status);

	return status;
}

static int mailbox_poll_response_v3(uint8_t client_id, uint8_t job_id,
				    uint32_t *resp, unsigned int *resp_len,
				    uint32_t urgent)
{
	unsigned int timeout = 40U;
	unsigned int sdm_loop = 255U;
	bool is_cmd_desc_fill = false;
	uint8_t di = 0U;
	sdm_response_t *resp_desc = NULL;
	sdm_command_t *cmd_desc = NULL;

	while (sdm_loop != 0U) {
		do {
			if (mmio_read_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM)
				== 1U) {
				break;
			}
			mdelay(10U);
		} while (--timeout != 0U);

		if (timeout == 0U) {
			INFO("%s: Timed out waiting for SDM intr\n", __func__);
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
			ERROR("MBOX: Mailbox did not get UA");
			return MBOX_RET_ERROR;
		}

		/* Fill the command descriptor index and get the same. */
		if (!is_cmd_desc_fill) {
			if (mailbox_fill_cmd_desc(client_id, job_id, resp) !=
				MBOX_RET_OK) {
				return MBOX_BUFFER_FULL;
			}

			cmd_desc = mailbox_get_cmd_desc(client_id, job_id);
			is_cmd_desc_fill = true;
		}

		/* Since it is sync call, will try to read till it time out */
		(void)mailbox_response_handler_fsm();

		/* Check the response queue with the given client ID and job ID */
		resp_desc = mailbox_get_resp_desc(client_id, job_id, &di);
		if (resp_desc != NULL) {
			VERBOSE("%s: Resp received for cid %d, jid %d\n",
				__func__, resp_desc->client_id, resp_desc->job_id);

			uint16_t header = resp_desc->header;

			/* Update the return response length */
			if (resp_len != NULL) {
				*resp_len = resp_desc->rcvd_resp_len;
			}

			/* Free the response and command descriptor */
			mailbox_free_resp_desc(di);
			mailbox_free_cmd_desc(cmd_desc);

			if (MBOX_RESP_ERR(header) > 0U) {
				INFO("%s: SDM err code: 0x%x\n", __func__,
					MBOX_RESP_ERR(header));
				return -MBOX_RESP_ERR(header);
			}

			VERBOSE("%s: MBOX_RET_OK\n", __func__);
			return MBOX_RET_OK;
		}
		sdm_loop--;
	}

	INFO("%s: Timed out waiting for SDM\n", __func__);
	return MBOX_TIMEOUT;
}

/* SDM response parser handler state machine. */
static void mailbox_response_parser(void)
{
	int di = -1;		/* Descriptor index */
	uint32_t rin;
	uint32_t rout;

	switch (mbox_svc.next_resp_state) {
	case SRS_WAIT_FOR_RESP:
	{
		mbox_svc.resp_state = SRS_WAIT_FOR_RESP;

		rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
		rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);
		if (rin != rout) {
			mbox_svc.next_resp_state = SRS_WAIT_FOR_HEADER;
		}

		break;
	}

	case SRS_WAIT_FOR_HEADER:
	{
		mbox_svc.resp_state = SRS_WAIT_FOR_HEADER;
		uint32_t resp_hdr;
		uint8_t trans_id = 0U;

		rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
		rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);
		if (rin != rout) {
			/* Read the header and dequeue from the queue. */
			resp_hdr = mmio_read_32(MBOX_ENTRY_TO_ADDR(RESP, (rout)++));
			rout %= MBOX_RESP_BUFFER_SIZE;
			mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);

			/* Allocate a new response descriptor */
			di = mailbox_get_free_resp_desc();
			if (di != -1) {
				mbox_svc.curr_di = di;
				mbox_svc.resp_queue[di].header = resp_hdr;
				mbox_svc.resp_queue[di].client_id = MBOX_RESP_CLIENT_ID(resp_hdr);
				mbox_svc.resp_queue[di].job_id = MBOX_RESP_JOB_ID(resp_hdr);
				mbox_svc.resp_queue[di].resp_len = MBOX_RESP_LEN(resp_hdr);
				mbox_svc.resp_queue[di].flags |= (FLAG_SDM_RESPONSE_IS_USED |
								  FLAG_SDM_RESPONSE_IS_IN_PROGRESS);
				mbox_svc.resp_queue[di].err_code = MBOX_RESP_ERR(resp_hdr);
				trans_id = MBOX_RESP_TRANSACTION_ID(resp_hdr);

				VERBOSE("MBOX: Resp Hdr: cid %d, jid %d, len %d, err_code 0x%x\n",
					mbox_svc.resp_queue[di].client_id,
					mbox_svc.resp_queue[di].job_id,
					mbox_svc.resp_queue[di].resp_len,
					mbox_svc.resp_queue[di].err_code);

				/* Check if the response is an argument response */
				if (mbox_svc.resp_queue[di].resp_len > 0) {
					mbox_svc.next_resp_state = SRS_WAIT_FOR_ARGUMENTS;
				} else {
					VERBOSE("MBOX: Received complete response with no args\n");
					/* Non argument response, done */
					mbox_svc.resp_queue[mbox_svc.curr_di].flags |=
									FLAG_SDM_RESPONSE_IS_VALID;

					/* Go back to waiting for new response */
					mbox_svc.next_resp_state = SRS_WAIT_FOR_RESP;
					mbox_svc.curr_di = -1;

					/* Mark the transaction ID as received */
					spin_lock(&mbox_db_lock);
					mbox_svc.received_bitmap[trans_id / MBOX_TID_BITMAP_SIZE] |=
							(1ULL << (trans_id % MBOX_TID_BITMAP_SIZE));
					spin_unlock(&mbox_db_lock);
				}
			} else {
				mbox_svc.next_resp_state = SRS_SYNC_ERROR;
			}
		}
		break;
	}

	case SRS_WAIT_FOR_ARGUMENTS:
	{
		mbox_svc.resp_state = SRS_WAIT_FOR_ARGUMENTS;
		uint16_t mbox_resp_len = mbox_svc.resp_queue[mbox_svc.curr_di].resp_len;
		uint32_t *read_buff = NULL;
		uint16_t read_len = 0U;
		uint16_t read_max_len = 0U;
		uint32_t timeout = 0U;

		/* Determine where to copy the buffer. */
		sdm_command_t *cmd_desc = mailbox_get_cmd_desc(
						mbox_svc.resp_queue[mbox_svc.curr_di].client_id,
						mbox_svc.resp_queue[mbox_svc.curr_di].job_id);
		if (cmd_desc != NULL && cmd_desc->cb_args != NULL) {
			read_buff = cmd_desc->cb_args;
			read_max_len = mbox_resp_len;
		} else {
			read_buff = (uint32_t *)mbox_svc.resp_queue[mbox_svc.curr_di].resp_data;
			read_max_len = MBOX_SVC_MAX_RESP_DATA_SIZE;
		}

		rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
		rout = mmio_read_32(MBOX_OFFSET + MBOX_ROUT);

		while ((read_len < mbox_resp_len) && (rin != rout) && (read_len < read_max_len)) {
			timeout = 10000U;

			/* Copy the response data to the buffer */
			read_buff[read_len++] = mmio_read_32(MBOX_ENTRY_TO_ADDR(RESP, (rout)++));

			VERBOSE("MBOX: 0x%x\n", read_buff[read_len - 1]);

			/* Update the read out buffer index */
			rout %= MBOX_RESP_BUFFER_SIZE;
			mmio_write_32(MBOX_OFFSET + MBOX_ROUT, rout);

			/*
			 * The response buffer is of 16 words size, this loop checks
			 * if the response buffer is empty and if empty trigger an
			 * interrupt to SDM and wait for the response buffer to fill
			 */
			do {
				if (read_len == mbox_resp_len)
					break;

				rin = mmio_read_32(MBOX_OFFSET + MBOX_RIN);
				if (rout == rin) {
					mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_TO_SDM, 1U);
					udelay(100);
				} else {
					break;
				}
				timeout--;
			} while ((read_len < mbox_resp_len) && (timeout != 0U));

			if (timeout == 0U) {
				INFO("MBOX: Timeout waiting for response data\n");
				mbox_svc.next_resp_state = SRS_SYNC_ERROR;
				break;
			}
		}

		/* Check if we have received all the arguments */
		mbox_svc.resp_queue[mbox_svc.curr_di].rcvd_resp_len = read_len;
		if (mbox_resp_len == read_len) {
			uint8_t transaction_id =
					((mbox_svc.resp_queue[mbox_svc.curr_di].client_id << 4) |
					 (mbox_svc.resp_queue[mbox_svc.curr_di].job_id));
			VERBOSE("MBOX: Received all the response data len %d, transaction_id %d\n",
				read_len, transaction_id);

			spin_lock(&mbox_db_lock);
			mbox_svc.received_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] |=
						(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE));
			spin_unlock(&mbox_db_lock);

			mbox_svc.resp_queue[mbox_svc.curr_di].flags |= FLAG_SDM_RESPONSE_IS_VALID;
			mbox_svc.next_resp_state = SRS_WAIT_FOR_RESP;
			mbox_svc.curr_di = -1;
		} else {
			mbox_svc.next_resp_state = SRS_SYNC_ERROR;
			VERBOSE("MBOX: Received partial response data len %d, max len %d\n",
				read_len, read_max_len);
		}
		break;
	}

	case SRS_SYNC_ERROR:
	{
		mbox_svc.resp_state = SRS_SYNC_ERROR;
		INFO("MBOX: Error in response handling\n");
		break;
	}

	default:
		break;
	} /* switch */
}

static int mailbox_response_handler_fsm(void)
{
	int status = MBOX_RET_OK;

	spin_lock(&mbox_read_lock);
	/* Mailbox peripheral response parser */
	do {
		/* Iterate till the state machine transition ends */
		mailbox_response_parser();

		/* Note down if there is any error in the response parsing */
		if (mbox_svc.next_resp_state == SRS_SYNC_ERROR) {
			status = MBOX_RET_ERROR;
		}

	} while (mbox_svc.resp_state != mbox_svc.next_resp_state);
	spin_unlock(&mbox_read_lock);

	return status;
}

int mailbox_response_poll_on_intr_v3(uint8_t *client_id, uint8_t *job_id,
				     uint64_t *bitmap)
{
	uint32_t i = 0U;
	int status = MBOX_RET_OK;

	/* Clear the SDM doorbell interrupt immediately */
	if (mmio_read_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM) == 1U) {
		mmio_write_32(MBOX_OFFSET + MBOX_DOORBELL_FROM_SDM, 0U);
	}

	/* Check mailbox FIFO for any pending responses available to read. */
	status = mailbox_response_handler_fsm();
	if (status != MBOX_RET_OK) {
		return status;
	}

	/*
	 * Once we read the complete mailbox FIFO, let's mark up the bitmap for
	 * available responses with respect to each transcation IDs.
	 */
	status = MBOX_NO_RESPONSE;
	spin_lock(&mbox_db_lock);
	for (i = 0; i < MBOX_MAX_TIDS_BITMAP; i++) {
		bitmap[i] = mbox_svc.interrupt_bitmap[i] ^ mbox_svc.received_bitmap[i];
		if (bitmap[i] != 0 && status == MBOX_NO_RESPONSE) {
			status = MBOX_RET_OK;
		}

		mbox_svc.interrupt_bitmap[i] = mbox_svc.received_bitmap[i];
	}
	spin_unlock(&mbox_db_lock);

	return status;
}

int mailbox_response_poll_v3(uint8_t client_id, uint8_t job_id,
			     uint64_t *ret_args, uint32_t *ret_args_len)
{
	sdm_command_t *cmd_desc = NULL;
	sdm_response_t *resp_desc = NULL;
	uint8_t di = 0U;
	int status = MBOX_RET_OK;

	/*
	 * Let's first check the local response queue with the given
	 * client ID and job ID
	 */
	resp_desc = mailbox_get_resp_desc(client_id, job_id, &di);
	if (resp_desc == NULL) {
		/* Not available in the local queue, let's read mailbox FIFO */
		status = mailbox_response_handler_fsm();
		if (status != MBOX_RET_OK) {
			return status;
		}

		resp_desc = mailbox_get_resp_desc(client_id, job_id, &di);
	}
	cmd_desc = mailbox_get_cmd_desc(client_id, job_id);

	if (cmd_desc != NULL && resp_desc != NULL) {
		VERBOSE("MBOX: Resp found for cid %d, jid %d\n", client_id, job_id);

		/* Command callback function */
		*ret_args_len = cmd_desc->cb(resp_desc, cmd_desc, ret_args);

		/* Free the command and response descriptors. */
		mailbox_free_cmd_desc(cmd_desc);
		mailbox_free_resp_desc(di);

		return MBOX_RET_OK;
	}

	INFO("MBOX: No resp found for cid: %d, jid: %d\n", client_id, job_id);

	return MBOX_NO_RESPONSE;
}

void mailbox_init_v3(void)
{
	uint32_t count;

	memset((void *)&mbox_svc, 0, sizeof(mbox_svc));

	mbox_svc.next_resp_state = SRS_WAIT_FOR_RESP;
	mbox_svc.resp_state = SRS_WAIT_FOR_RESP;

	/* Free all entries from the response queue. */
	for (count = 0; count < MBOX_SVC_RESP_QUEUE_SIZE; count++) {
		mbox_svc.resp_queue[count].flags = 0;
	}

	/* Free all entries from the command queue. */
	for (count = 0; count < MBOX_SVC_CMD_QUEUE_SIZE; count++) {
		mbox_svc.cmd_queue[count].flags = 0;
	}

	mbox_svc.curr_di = -1;
}
#endif /* #if SIP_SVC_V3 */
