/*
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>

#include "agilex5_iossm_mailbox.h"

/* supported DDR type list */
static const char *ddr_type_list[7] = {
	"DDR4", "DDR5", "DDR5_RDIMM", "LPDDR4", "LPDDR5", "QDRIV", "UNKNOWN"
};

static inline int wait_for_bit(const void *reg,
			       const uint32_t mask,
			       const bool set,
			       const unsigned int timeout_ms)
{
	uint32_t val;
	uint32_t timeout_sec = (timeout_ms / 1000);

	while (timeout_sec > 0) {
		val = mmio_read_32((uintptr_t)reg);

		INFO("IOSSM: timeout_sec %d, val %x\n", timeout_sec, val);

		if (!set) {
			val = ~val;
		}

		if ((val & mask) == mask) {
			INFO("IOSSM: %s, success\n", __func__);
			return 0;
		}

		/* one second delay */
		mdelay(1000);

		timeout_sec--;
	}

	ERROR("IOSSM: %s, failed, time out\n", __func__);
	return -ETIMEDOUT;
}

int io96b_mb_req(phys_addr_t io96b_csr_addr, uint32_t ip_type, uint32_t instance_id,
		 uint32_t usr_cmd_type, uint32_t usr_cmd_opcode, uint32_t cmd_param_0,
		 uint32_t cmd_param_1, uint32_t cmd_param_2, uint32_t cmd_param_3,
		 uint32_t cmd_param_4, uint32_t cmd_param_5, uint32_t cmd_param_6,
		 uint32_t resp_data_len, struct io96b_mb_resp *resp)
{
	int i;
	int ret;
	uint32_t cmd_req, cmd_resp;

	/* Initialized zeros for responses*/
	resp->cmd_resp_status = 0;
	resp->cmd_resp_data_0 = 0;
	resp->cmd_resp_data_1 = 0;
	resp->cmd_resp_data_2 = 0;

	/* Ensure CMD_REQ is cleared before write any command request */
	ret = wait_for_bit((const void *)(io96b_csr_addr + IOSSM_CMD_REQ_OFFSET),
			   GENMASK(31, 0), 0, 10000);

	if (ret != 0) {
		ERROR("%s: CMD_REQ not ready\n", __func__);
		return -1;
	}

	/* Write CMD_PARAM_* */
	for (i = 0; i < 6 ; i++) {
		switch (i) {
		case 0:
			if (cmd_param_0 != 0) {
				mmio_write_32(io96b_csr_addr + IOSSM_CMD_PARAM_0_OFFSET,
					      cmd_param_0);
			}
			break;
		case 1:
			if (cmd_param_1 != 0) {
				mmio_write_32(io96b_csr_addr + IOSSM_CMD_PARAM_1_OFFSET,
					      cmd_param_1);
			}
			break;
		case 2:
			if (cmd_param_2 != 0) {
				mmio_write_32(io96b_csr_addr + IOSSM_CMD_PARAM_2_OFFSET,
					      cmd_param_2);
			}
			break;
		case 3:
			if (cmd_param_3 != 0) {
				mmio_write_32(io96b_csr_addr + IOSSM_CMD_PARAM_3_OFFSET,
					      cmd_param_3);
			}
			break;
		case 4:
			if (cmd_param_4 != 0) {
				mmio_write_32(io96b_csr_addr + IOSSM_CMD_PARAM_4_OFFSET,
					      cmd_param_4);
			}
			break;
		case 5:
			if (cmd_param_5 != 0) {
				mmio_write_32(io96b_csr_addr + IOSSM_CMD_PARAM_5_OFFSET,
					      cmd_param_5);
			}
			break;
		case 6:
			if (cmd_param_6 != 0) {
				mmio_write_32(io96b_csr_addr + IOSSM_CMD_PARAM_6_OFFSET,
					      cmd_param_6);
			}
			break;
		default:
			ERROR("IOSSM: %s: Invalid command parameter\n", __func__);
		}
	}

	/* Write CMD_REQ (IP_TYPE, IP_INSTANCE_ID, CMD_TYPE and CMD_OPCODE) */
	cmd_req = (usr_cmd_opcode << 0) | (usr_cmd_type << 16) | (instance_id << 24) |
		  (ip_type << 29);
	mmio_write_32(io96b_csr_addr + IOSSM_CMD_REQ_OFFSET, cmd_req);
	INFO("IOSSM: %s: Write 0x%x to IOSSM_CMD_REQ_OFFSET 0x%llx\n",
		__func__, cmd_req, io96b_csr_addr + IOSSM_CMD_REQ_OFFSET);

	/* Read CMD_RESPONSE_READY in CMD_RESPONSE_STATUS*/
	ret = wait_for_bit((const void *)(io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET),
			   IOSSM_STATUS_COMMAND_RESPONSE_READY, 1, 10000);

	if (ret != 0) {
		ERROR("%s: CMD_RESPONSE ERROR:\n", __func__);
		cmd_resp = (io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET);
		ERROR("%s: STATUS_GENERAL_ERROR: 0x%x\n", __func__, (cmd_resp >> 1) & 0xF);
		ERROR("%s: STATUS_CMD_RESPONSE_ERROR: 0x%x\n", __func__, (cmd_resp >> 5) & 0x7);
	}

	/* read CMD_RESPONSE_STATUS*/
	resp->cmd_resp_status = mmio_read_32(io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET);
	INFO("IOSSM: %s: CMD_RESPONSE_STATUS 0x%llx: 0x%x\n",
		__func__, io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET, resp->cmd_resp_status);

	/* read CMD_RESPONSE_DATA_* */
	for (i = 0; i < resp_data_len; i++) {
		switch (i) {
		case 0:
			resp->cmd_resp_data_0 =
				mmio_read_32(io96b_csr_addr + IOSSM_CMD_RESPONSE_DATA_0_OFFSET);

			break;
		case 1:
			resp->cmd_resp_data_1 =
				mmio_read_32(io96b_csr_addr + IOSSM_CMD_RESPONSE_DATA_1_OFFSET);

			break;
		case 2:
			resp->cmd_resp_data_2 =
				mmio_read_32(io96b_csr_addr + IOSSM_CMD_RESPONSE_DATA_2_OFFSET);
			break;
		default:
			ERROR("%s: Invalid response data\n", __func__);
		}
	}

	resp->cmd_resp_status = mmio_read_32(io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET);
	INFO("IOSSM: %s: CMD_RESPONSE_STATUS 0x%llx: 0x%x\n",
		__func__, io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET, resp->cmd_resp_status);

	/* write CMD_RESPONSE_READY = 0 */
	mmio_clrbits_32(io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET,
			IOSSM_STATUS_COMMAND_RESPONSE_READY);

	resp->cmd_resp_status = mmio_read_32(io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET);
	INFO("IOSSM: %s: CMD_RESPONSE_READY 0x%llx: 0x%x\n",
		__func__, io96b_csr_addr + IOSSM_CMD_RESPONSE_STATUS_OFFSET, resp->cmd_resp_status);

	return 0;
}

/*
 * Initial function to be called to set memory interface IP type and instance ID
 * IP type and instance ID need to be determined before sending mailbox command
 */
void io96b_mb_init(struct io96b_info *io96b_ctrl)
{
	struct io96b_mb_resp usr_resp;
	uint8_t ip_type_ret, instance_id_ret;
	int i, j, k;

	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		switch (i) {
		case 0:
			/* Get memory interface IP type & instance ID (IP identifier) */
			io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr, 0, 0,
				     CMD_GET_SYS_INFO, GET_MEM_INTF_INFO, 0, 0,
				     0, 0, 0, 0, 0, 2, &usr_resp);
			/* Retrieve number of memory interface(s) */
			io96b_ctrl->io96b_0.mb_ctrl.num_mem_interface =
				IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status) & 0x3;

			/* Retrieve memory interface IP type and instance ID (IP identifier) */
			j = 0;
			for (k = 0; k < MAX_MEM_INTERFACES_SUPPORTED; k++) {
				switch (k) {
				case 0:
					ip_type_ret = (usr_resp.cmd_resp_data_0 >> 29) & 0x7;
					instance_id_ret = (usr_resp.cmd_resp_data_0 >> 24) & 0x1F;
					break;
				case 1:
					ip_type_ret = (usr_resp.cmd_resp_data_1 >> 29) & 0x7;
					instance_id_ret = (usr_resp.cmd_resp_data_1 >> 24) & 0x1F;
					break;
				}

				if (ip_type_ret != 0) {
					io96b_ctrl->io96b_0.mb_ctrl.ip_type[j] = ip_type_ret;
					io96b_ctrl->io96b_0.mb_ctrl.ip_instance_id[j] =
						instance_id_ret;
					j++;
				}
			}
			break;
		case 1:
			/* Get memory interface IP type and instance ID (IP identifier) */
			io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr, 0, 0, CMD_GET_SYS_INFO,
				     GET_MEM_INTF_INFO, 0, 0, 0, 0, 0, 0, 0, 2, &usr_resp);

			/* Retrieve number of memory interface(s) */
			io96b_ctrl->io96b_1.mb_ctrl.num_mem_interface =
				IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status) & 0x3;

			/* Retrieve memory interface IP type and instance ID (IP identifier) */
			j = 0;
			for (k = 0; k < MAX_MEM_INTERFACES_SUPPORTED; k++) {
				switch (k) {
				case 0:
					ip_type_ret = (usr_resp.cmd_resp_data_0 >> 29) & 0x7;
					instance_id_ret = (usr_resp.cmd_resp_data_0 >> 24) & 0x1F;
					break;
				case 1:
					ip_type_ret = (usr_resp.cmd_resp_data_1 >> 29) & 0x7;
					instance_id_ret = (usr_resp.cmd_resp_data_1 >> 24) & 0x1F;
					break;
				}

				if (ip_type_ret != 0) {
					io96b_ctrl->io96b_1.mb_ctrl.ip_type[j] = ip_type_ret;
					io96b_ctrl->io96b_1.mb_ctrl.ip_instance_id[j] =
						instance_id_ret;
					j++;
				}
			}
			break;
		}

	}
}

static inline void hang(void)
{
	ERROR("IOSSM: %s: system is going to die :(\n", __func__);
	while (1)
		;
}

int io96b_cal_status(phys_addr_t addr)
{
	int cal_busy_status, cal_success_status;
	phys_addr_t status_addr = addr + IOSSM_STATUS_OFFSET;

	/* Ensure calibration busy status */
	cal_busy_status = wait_for_bit((const void *)status_addr, IOSSM_STATUS_CAL_BUSY,
					false, 15000);
	if (cal_busy_status != 0) {
		ERROR("IOSSM: One or more EMIF instances are busy with calibration\n");
		return -EBUSY;
	}

	/* Calibration success status check */
	NOTICE("IOSSM: Calibration success status check...\n");
	cal_success_status = wait_for_bit((const void *)status_addr, IOSSM_STATUS_CAL_SUCCESS,
					  true, 15000);
	if (cal_success_status != 0) {
		ERROR("IOSSM: One/more EMIF instances either failed to calibrate/not completed\n");
		return -EBUSY;
	}

	NOTICE("IOSSM: All EMIF instances within the IO96 have calibrated successfully!\n");
	return 0;
}

void init_mem_cal(struct io96b_info *io96b_ctrl)
{
	int count, i, ret;

	/* Initialize overall calibration status */
	io96b_ctrl->overall_cal_status = false;

	/* Check initial calibration status for the assigned IO96B */
	count = 0;
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		switch (i) {
		case 0:
			ret = io96b_cal_status(io96b_ctrl->io96b_0.io96b_csr_addr);
			if (ret != 0) {
				io96b_ctrl->io96b_0.cal_status = false;
				ERROR("%s: Initial DDR calibration IO96B_0 failed %d\n",
					__func__, ret);
				break;
			}
			io96b_ctrl->io96b_0.cal_status = true;
			INFO("IOSSM: %s: Initial DDR calibration IO96B_0 succeed\n", __func__);
			count++;
			break;
		case 1:
			ret = io96b_cal_status(io96b_ctrl->io96b_1.io96b_csr_addr);
			if (ret != 0) {
				io96b_ctrl->io96b_1.cal_status = false;
				ERROR("%s: Initial DDR calibration IO96B_1 failed %d\n",
					__func__, ret);
				break;
			}
			io96b_ctrl->io96b_1.cal_status = true;
			INFO("IOSSM: %s: Initial DDR calibration IO96B_1 succeed\n", __func__);
			count++;
			break;
		}
	}

	if (count == io96b_ctrl->num_instance)
		io96b_ctrl->overall_cal_status = true;
}

/*
 * Trying 3 times re-calibration if initial calibration failed
 */
int trig_mem_cal(struct io96b_info *io96b_ctrl)
{
	struct io96b_mb_resp usr_resp;
	bool recal_success;
	int i;
	uint8_t cal_stat;

	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		switch (i) {
		case 0:
			if (!(io96b_ctrl->io96b_0.cal_status)) {
				/* Get the memory calibration status for first memory interface */
				io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr, 0, 0,
					     CMD_TRIG_MEM_CAL_OP, GET_MEM_CAL_STATUS, 0,
					     0, 0, 0, 0, 0, 0, 2, &usr_resp);

				recal_success = false;

				/* Re-calibration first memory interface with failed calibration */
				for (i = 0; i < 3; i++) {
					cal_stat = usr_resp.cmd_resp_data_0 & GENMASK(2, 0);
					if (cal_stat < 0x2) {
						recal_success = true;
						break;
					}
					io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr,
						     io96b_ctrl->io96b_0.mb_ctrl.ip_type[0],
						     io96b_ctrl->io96b_0.mb_ctrl.ip_instance_id[0],
						     CMD_TRIG_MEM_CAL_OP, TRIG_MEM_CAL, 0, 0, 0, 0,
						     0, 0, 0, 2, &usr_resp);
					mdelay(1000);
					io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr, 0, 0,
						     CMD_TRIG_MEM_CAL_OP, GET_MEM_CAL_STATUS,
						     0, 0, 0, 0, 0, 0, 0, 2, &usr_resp);
				}

				if (!recal_success) {
					ERROR("%s: Error as SDRAM calibration failed\n", __func__);
					hang();
				}

				/* Get the memory calibration status for second memory interface */
				io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr, 0, 0,
					     CMD_TRIG_MEM_CAL_OP, GET_MEM_CAL_STATUS, 0, 0, 0,
					     0, 0, 0, 0, 2, &usr_resp);

				recal_success = false;

				/* Re-calibration second memory interface with failed calibration*/
				for (i = 0; i < 3; i++) {
					cal_stat = usr_resp.cmd_resp_data_1 & GENMASK(2, 0);
					if (cal_stat < 0x2) {
						recal_success = true;
						break;
					}
					io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr,
						     io96b_ctrl->io96b_0.mb_ctrl.ip_type[1],
						     io96b_ctrl->io96b_0.mb_ctrl.ip_instance_id[1],
						     CMD_TRIG_MEM_CAL_OP, TRIG_MEM_CAL, 0, 0, 0, 0,
						     0, 0, 0, 2, &usr_resp);
					mdelay(1000);
					io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr, 0, 0,
						     CMD_TRIG_MEM_CAL_OP, GET_MEM_CAL_STATUS,
						     0, 0, 0, 0, 0, 0, 0, 2, &usr_resp);
				}

				if (!recal_success) {
					ERROR("IOSSMM: Error as SDRAM calibration failed\n");
					hang();
				}

				io96b_ctrl->io96b_0.cal_status = true;
			}
			break;
		case 1:
			if (!(io96b_ctrl->io96b_1.cal_status)) {
				/* Get the memory calibration status for first memory interface */
				io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr, 0, 0,
					     CMD_TRIG_MEM_CAL_OP, GET_MEM_CAL_STATUS, 0,
					     0, 0, 0, 0, 0, 0, 2, &usr_resp);

				recal_success = false;

				/* Re-calibration first memory interface with failed calibration */
				for (i = 0; i < 3; i++) {
					cal_stat = usr_resp.cmd_resp_data_0 & GENMASK(2, 0);
					if (cal_stat < 0x2) {
						recal_success = true;
						break;
					}
					io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr,
						     io96b_ctrl->io96b_1.mb_ctrl.ip_type[0],
						     io96b_ctrl->io96b_1.mb_ctrl.ip_instance_id[0],
						     CMD_TRIG_MEM_CAL_OP, TRIG_MEM_CAL, 0, 0, 0, 0,
						     0, 0, 0, 2, &usr_resp);
					mdelay(1000);
					io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr, 0, 0,
						     CMD_TRIG_MEM_CAL_OP, GET_MEM_CAL_STATUS,
						     0, 0, 0, 0, 0, 0, 0, 2, &usr_resp);
				}

				if (!recal_success) {
					ERROR("IOSSM: Error as SDRAM calibration failed\n");
					hang();
				}

				/* Get the memory calibration status for second memory interface */
				io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr, 0, 0,
					     CMD_TRIG_MEM_CAL_OP, GET_MEM_CAL_STATUS, 0, 0, 0,
					     0, 0, 0, 0, 2, &usr_resp);

				recal_success = false;

				/* Re-calibration second memory interface with failed calibration*/
				for (i = 0; i < 3; i++) {
					cal_stat = usr_resp.cmd_resp_data_0 & GENMASK(2, 0);
					if (cal_stat < 0x2) {
						recal_success = true;
						break;
					}
					io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr,
						     io96b_ctrl->io96b_1.mb_ctrl.ip_type[1],
						     io96b_ctrl->io96b_1.mb_ctrl.ip_instance_id[1],
						     CMD_TRIG_MEM_CAL_OP, TRIG_MEM_CAL, 0, 0, 0, 0,
						     0, 0, 0, 2, &usr_resp);
					mdelay(1000);
					io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr, 0, 0,
						     CMD_TRIG_MEM_CAL_OP, GET_MEM_CAL_STATUS,
						     0, 0, 0, 0, 0, 0, 0, 2, &usr_resp);
				}

				if (!recal_success) {
					ERROR("IOSSM: Error as SDRAM calibration failed\n");
					hang();
				}

				io96b_ctrl->io96b_1.cal_status = true;
			}
			break;
		}
	}

	if (io96b_ctrl->io96b_0.cal_status && io96b_ctrl->io96b_1.cal_status) {
		INFO("IOSSM: %s: Overall SDRAM calibration success\n", __func__);
		io96b_ctrl->overall_cal_status = true;
	}

	return 0;
}

int get_mem_technology(struct io96b_info *io96b_ctrl)
{
	struct io96b_mb_resp usr_resp;
	int i, j;
	uint8_t ddr_type_ret;

	/* Initialize ddr type */
	io96b_ctrl->ddr_type = ddr_type_list[6];

	/* Get and ensure all memory interface(s) same DDR type */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		switch (i) {
		case 0:
			for (j = 0; j < io96b_ctrl->io96b_0.mb_ctrl.num_mem_interface; j++) {
				io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr,
					     io96b_ctrl->io96b_0.mb_ctrl.ip_type[j],
					     io96b_ctrl->io96b_0.mb_ctrl.ip_instance_id[j],
					     CMD_GET_MEM_INFO, GET_MEM_TECHNOLOGY, 0, 0, 0, 0,
					     0, 0, 0, 0, &usr_resp);

				ddr_type_ret =
					IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
					& GENMASK(2, 0);

				if (strcmp(io96b_ctrl->ddr_type, "UNKNOWN") == 0)
					io96b_ctrl->ddr_type = ddr_type_list[ddr_type_ret];

				if (ddr_type_list[ddr_type_ret] != io96b_ctrl->ddr_type) {
					ERROR("IOSSM: Mismatch DDR type on IO96B_0\n");
					return -ENOEXEC;
				}
			}
			break;
		case 1:
			for (j = 0; j < io96b_ctrl->io96b_1.mb_ctrl.num_mem_interface; j++) {
				io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr,
					     io96b_ctrl->io96b_1.mb_ctrl.ip_type[j],
					     io96b_ctrl->io96b_1.mb_ctrl.ip_instance_id[j],
					     CMD_GET_MEM_INFO, GET_MEM_TECHNOLOGY, 0, 0, 0,
					     0, 0, 0, 0, 0, &usr_resp);

				ddr_type_ret =
					IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
					& GENMASK(2, 0);

				if (strcmp(io96b_ctrl->ddr_type, "UNKNOWN") == 0)
					io96b_ctrl->ddr_type = ddr_type_list[ddr_type_ret];

				if (ddr_type_list[ddr_type_ret] != io96b_ctrl->ddr_type) {
					ERROR("IOSSM: Mismatch DDR type on IO96B_1\n");
					return -ENOEXEC;
				}
			}
			break;
		}
	}

	return 0;
}

int get_mem_width_info(struct io96b_info *io96b_ctrl)
{
	struct io96b_mb_resp usr_resp;
	int i, j;
	uint16_t memory_size = 0U;
	uint16_t total_memory_size = 0U;

	/* Get all memory interface(s) total memory size on all instance(s) */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		switch (i) {
		case 0:
			memory_size = 0;
			for (j = 0; j < io96b_ctrl->io96b_0.mb_ctrl.num_mem_interface; j++) {
				io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr,
					     io96b_ctrl->io96b_0.mb_ctrl.ip_type[j],
					     io96b_ctrl->io96b_0.mb_ctrl.ip_instance_id[j],
					     CMD_GET_MEM_INFO, GET_MEM_WIDTH_INFO, 0, 0, 0,
					     0, 0, 0, 0, 2, &usr_resp);

				memory_size = memory_size +
						(usr_resp.cmd_resp_data_1 & GENMASK(7, 0));
			}

			if (memory_size == 0U) {
				ERROR("IOSSM: %s: Failed to get valid memory size\n", __func__);
				return -ENOEXEC;
			}

			io96b_ctrl->io96b_0.size = memory_size;

			break;
		case 1:
			memory_size = 0;
			for (j = 0; j < io96b_ctrl->io96b_1.mb_ctrl.num_mem_interface; j++) {
				io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr,
					     io96b_ctrl->io96b_1.mb_ctrl.ip_type[j],
					     io96b_ctrl->io96b_1.mb_ctrl.ip_instance_id[j],
					     CMD_GET_MEM_INFO, GET_MEM_WIDTH_INFO, 0, 0, 0,
					     0, 0, 0, 0, 2, &usr_resp);

				memory_size = memory_size +
						(usr_resp.cmd_resp_data_1 & GENMASK(7, 0));
			}

			if (memory_size == 0U) {
				ERROR("IOSSM: %s: Failed to get valid memory size\n", __func__);
				return -ENOEXEC;
			}

			io96b_ctrl->io96b_1.size = memory_size;

			break;
		}

		total_memory_size = total_memory_size + memory_size;
	}

	if (total_memory_size == 0U) {
		ERROR("IOSSM: %s: Failed to get valid memory size\n", __func__);
		return -ENOEXEC;
	}

	io96b_ctrl->overall_size = total_memory_size;

	return 0;
}

int ecc_enable_status(struct io96b_info *io96b_ctrl)
{
	struct io96b_mb_resp usr_resp;
	int i, j;
	bool ecc_stat_set = false;
	bool ecc_stat;

	/* Initialize ECC status */
	io96b_ctrl->ecc_status = false;

	/* Get and ensure all memory interface(s) same ECC status */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		switch (i) {
		case 0:
			for (j = 0; j < io96b_ctrl->io96b_0.mb_ctrl.num_mem_interface; j++) {
				io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr,
					     io96b_ctrl->io96b_0.mb_ctrl.ip_type[j],
					     io96b_ctrl->io96b_0.mb_ctrl.ip_instance_id[j],
					     CMD_TRIG_CONTROLLER_OP, ECC_ENABLE_STATUS, 0, 0,
					     0, 0, 0, 0, 0, 0, &usr_resp);

				ecc_stat = ((IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
						& GENMASK(1, 0)) == 0 ? false : true);

				if (!ecc_stat_set) {
					io96b_ctrl->ecc_status = ecc_stat;
					ecc_stat_set = true;
				}

				if (ecc_stat != io96b_ctrl->ecc_status) {
					ERROR("IOSSM: %s: Mismatch DDR ECC status on IO96B_0\n",
						__func__);
					return -ENOEXEC;
				}
			}
			break;
		case 1:
			for (j = 0; j < io96b_ctrl->io96b_1.mb_ctrl.num_mem_interface; j++) {
				io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr,
					     io96b_ctrl->io96b_1.mb_ctrl.ip_type[j],
					     io96b_ctrl->io96b_1.mb_ctrl.ip_instance_id[j],
					     CMD_TRIG_CONTROLLER_OP, ECC_ENABLE_STATUS, 0, 0,
					     0, 0, 0, 0, 0, 0, &usr_resp);

				ecc_stat = ((IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
						& GENMASK(1, 0)) == 0 ? false : true);

				if (!ecc_stat_set) {
					io96b_ctrl->ecc_status = ecc_stat;
					ecc_stat_set = true;
				}

				if (ecc_stat != io96b_ctrl->ecc_status) {
					ERROR("%s: Mismatch DDR ECC status on IO96B_1\n"
						, __func__);
					return -ENOEXEC;
				}
			}
			break;
		}
	}
	return 0;
}

int bist_mem_init_start(struct io96b_info *io96b_ctrl)
{
	struct io96b_mb_resp usr_resp;
	int i, j;
	bool bist_start, bist_success;
	uint32_t read_count;
	uint32_t read_interval_ms;

	/* Full memory initialization BIST performed on all memory interface(s) */
	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		switch (i) {
		case 0:
			for (j = 0; j < io96b_ctrl->io96b_0.mb_ctrl.num_mem_interface; j++) {
				bist_start = false;
				bist_success = false;
				read_interval_ms = 500U;

				/* Start memory initialization BIST on full memory address */
				io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr,
					     io96b_ctrl->io96b_0.mb_ctrl.ip_type[j],
					     io96b_ctrl->io96b_0.mb_ctrl.ip_instance_id[j],
					     CMD_TRIG_CONTROLLER_OP, BIST_MEM_INIT_START, 0x40,
					     0, 0, 0, 0, 0, 0, 0, &usr_resp);

				bist_start =
					(IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
					& 1);

				if (!bist_start) {
					ERROR("IOSSM: %s: Failed to initialized memory on IO96B_0\n"
					, __func__);
					ERROR("IOSSM: %s: BIST_MEM_INIT_START Error code 0x%x\n",
					__func__,
					(IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
					& GENMASK(2, 1)) > 0x1);
					return -ENOEXEC;
				}

				/* Polling for the initiated memory initialization BIST status */
				read_count = read_interval_ms / TIMEOUT;
				while (!bist_success) {
					io96b_mb_req(io96b_ctrl->io96b_0.io96b_csr_addr,
						     io96b_ctrl->io96b_0.mb_ctrl.ip_type[j],
						     io96b_ctrl->io96b_0.mb_ctrl.ip_instance_id[j],
						     CMD_TRIG_CONTROLLER_OP, BIST_MEM_INIT_STATUS,
						     0, 0, 0, 0, 0, 0, 0, 0, &usr_resp);

					bist_success = (IOSSM_CMD_RESPONSE_DATA_SHORT
							(usr_resp.cmd_resp_status) & 1);

					if ((!bist_success) && (read_count == 0U)) {
						ERROR("IOSSM: %s: Timeout init memory on IO96B_0\n"
							, __func__);
						ERROR("IOSSM: %s: BIST_MEM_INIT_STATUS Err code%x\n"
							, __func__, (IOSSM_CMD_RESPONSE_DATA_SHORT
							(usr_resp.cmd_resp_status)
							& GENMASK(2, 1)) > 0x1);
						return -ETIMEDOUT;
					}
					read_count--;
					mdelay(read_interval_ms);
				}
			}

			NOTICE("IOSSM: %s: Memory initialized successfully on IO96B_0\n", __func__);
			break;

		case 1:
			for (j = 0; j < io96b_ctrl->io96b_1.mb_ctrl.num_mem_interface; j++) {
				bist_start = false;
				bist_success = false;
				read_interval_ms = 500U;

				/* Start memory initialization BIST on full memory address */
				io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr,
					     io96b_ctrl->io96b_1.mb_ctrl.ip_type[j],
					     io96b_ctrl->io96b_1.mb_ctrl.ip_instance_id[j],
					     CMD_TRIG_CONTROLLER_OP, BIST_MEM_INIT_START, 0x40,
					     0, 0, 0, 0, 0, 0, 0, &usr_resp);

				bist_start =
					(IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
					& 1);

				if (!bist_start) {
					ERROR("IOSSM: %s: Failed to initialized memory on IO96B_1\n"
						, __func__);
					ERROR("IOSSM: %s: BIST_MEM_INIT_START Error code 0x%x\n",
					__func__,
					(IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
					& GENMASK(2, 1)) > 0x1);
					return -ENOEXEC;
				}

				/* Polling for the initiated memory initialization BIST status */
				read_count = read_interval_ms / TIMEOUT;
				while (!bist_success) {
					io96b_mb_req(io96b_ctrl->io96b_1.io96b_csr_addr,
						     io96b_ctrl->io96b_1.mb_ctrl.ip_type[j],
						     io96b_ctrl->io96b_1.mb_ctrl.ip_instance_id[j],
						     CMD_TRIG_CONTROLLER_OP, BIST_MEM_INIT_STATUS,
						     0, 0, 0, 0, 0, 0, 0, 0, &usr_resp);

					bist_success = (IOSSM_CMD_RESPONSE_DATA_SHORT
							(usr_resp.cmd_resp_status) & 1);

					if ((!bist_success) && (read_count == 0U)) {
						ERROR("IOSSM: %s: Timeout init memory on IO96B_1\n"
							, __func__);
						ERROR("IOSSM: %s: BIST_MEM_INIT_STATUS ErrCode %x\n"
							, __func__, (IOSSM_CMD_RESPONSE_DATA_SHORT
							(usr_resp.cmd_resp_status)
							& GENMASK(2, 1)) > 0x1);
						return -ETIMEDOUT;
					}
					read_count--;
					mdelay(read_interval_ms);
				}
			}

			NOTICE("IOSSM: %s: Memory initialized successfully on IO96B_1\n", __func__);
			break;
		}
	}
	return 0;
}
