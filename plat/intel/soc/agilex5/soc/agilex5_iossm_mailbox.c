/*
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
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
			   GENMASK(31, 0), 0, IOSSM_TIMEOUT_MS);

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
			   IOSSM_STATUS_COMMAND_RESPONSE_READY, 1, IOSSM_TIMEOUT_MS);

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
	uint8_t ip_type_ret, instance_id_ret;
	int i, j, k;
	uint32_t mem_interface_0, mem_interface_1;

	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		struct io96b_instance *inst = (i == 0) ?
		&io96b_ctrl->io96b_0 : &io96b_ctrl->io96b_1;
		phys_addr_t base = inst->io96b_csr_addr;

		j = 0;
		inst->mb_ctrl.num_mem_interface = 0;

		mem_interface_0 = mmio_read_32(base + IOSSM_MEM_INTF_INFO_0_OFFSET);
		mem_interface_1 = mmio_read_32(base + IOSSM_MEM_INTF_INFO_1_OFFSET);

		for (k = 0; k < MAX_MEM_INTERFACES_SUPPORTED; k++) {
			switch (k) {
			case 0:
				ip_type_ret = FIELD_GET(INTF_IP_TYPE_MASK, mem_interface_0);
				instance_id_ret = FIELD_GET(INTF_INSTANCE_ID_MASK, mem_interface_0);
				break;
			case 1:
				ip_type_ret = FIELD_GET(INTF_IP_TYPE_MASK, mem_interface_1);
				instance_id_ret = FIELD_GET(INTF_INSTANCE_ID_MASK, mem_interface_1);
				break;
			}

			if (ip_type_ret != 0) {
				inst->mb_ctrl.ip_type[j] = ip_type_ret;
				inst->mb_ctrl.ip_instance_id[j] = instance_id_ret;
				inst->mb_ctrl.num_mem_interface++;
				j++;
			}
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

	/* Ensure calibration fail status */
	cal_busy_status = wait_for_bit((const void *)status_addr, IOSSM_STATUS_CAL_FAIL,
					false, 15000);
	if (cal_busy_status != 0) {
		ERROR("IOSSM: One or more EMIF instances are failed with calibration\n");
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
	int count = 0;
	int ret;

	io96b_ctrl->overall_cal_status = false;

	for (int i = 0; i < io96b_ctrl->num_instance; i++) {
		struct io96b_instance *inst = (i == 0) ?
				&io96b_ctrl->io96b_0 : &io96b_ctrl->io96b_1;

		ret = io96b_cal_status(inst->io96b_csr_addr);
		if (ret != 0) {
			inst->cal_status = false;
			ERROR("IOSSM: %s: Initial DDR calibration IO96B failed %d\n",
					__func__, i);
			continue;
		}

		inst->cal_status = true;
		INFO("IOSSM: %s: Initial DDR calibration IO96B_%d succeeded\n", __func__, i);
		count++;
	}

	if (count == io96b_ctrl->num_instance)
		io96b_ctrl->overall_cal_status = true;
}

/*
 * Trying 3 times re-calibration if initial calibration failed
 */
int trig_mem_cal(struct io96b_info *io96b_ctrl)
{
	bool recal_success;
	uint8_t cal_stat;
	uint32_t cmd_req;
	phys_addr_t base;
	int iface;

	for (int inst = 0; inst < io96b_ctrl->num_instance; inst++) {
		struct io96b_instance *inst_ctrl = (inst == 0) ?
						&io96b_ctrl->io96b_0 : &io96b_ctrl->io96b_1;
		if (inst_ctrl->cal_status)
			continue;

		base = inst_ctrl->io96b_csr_addr;

		for (iface = 0; iface < 2; iface++) {
			// 1. Wait for CMD_REQ to be 0
			if (wait_for_bit((void *)(base + IOSSM_CMD_REQ_OFFSET),
					 GENMASK(31, 0), 0, IOSSM_TIMEOUT_MS) != 0) {
				ERROR("%s: CMD_REQ not ready\n", __func__);
				hang();
			}

			// 2. Write CMD_REQ: GET_MEM_CAL_STATUS
			cmd_req = (GET_MEM_CAL_STATUS << 0) |
				  (CMD_TRIG_MEM_CAL_OP << 16) |
				  (0 << 24) | (0 << 29);
			mmio_write_32(base + IOSSM_CMD_REQ_OFFSET, cmd_req);

			// 3. Wait for response ready
			if (wait_for_bit((void *)(base + IOSSM_CMD_RESPONSE_STATUS_OFFSET),
			IOSSM_STATUS_COMMAND_RESPONSE_READY, 1, IOSSM_TIMEOUT_MS) != 0) {
				ERROR("%s: Response timeout\n", __func__);
				hang();
			}

			// 4. Read response
			uint32_t resp = mmio_read_32(base + (iface == 0 ?

			IOSSM_CMD_RESPONSE_DATA_0_OFFSET :
			IOSSM_CMD_RESPONSE_DATA_1_OFFSET));

			cal_stat = resp & GENMASK(2, 0);
			recal_success = false;

			// 5. Recalibrate if necessary
			for (int retry = 0; retry < 3 && cal_stat >= 0x2; retry++) {
				// Wait for CMD_REQ clear
				if (wait_for_bit((void *)(base + IOSSM_CMD_REQ_OFFSET),
						 GENMASK(31, 0), 0, IOSSM_TIMEOUT_MS) != 0) {
					ERROR("%s: CMD_REQ not ready (retry)\n", __func__);
					hang();
				}

				// Send TRIG_MEM_CAL
				cmd_req = (TRIG_MEM_CAL << 0) |
					  (CMD_TRIG_MEM_CAL_OP << 16) |
					  (inst_ctrl->mb_ctrl.ip_instance_id[iface] << 24) |
					  (inst_ctrl->mb_ctrl.ip_type[iface] << 29);
				mmio_write_32(base + IOSSM_CMD_REQ_OFFSET, cmd_req);

				mdelay(1000);

				// Send GET_MEM_CAL_STATUS again
				if (wait_for_bit((void *)(base + IOSSM_CMD_REQ_OFFSET),
						 GENMASK(31, 0), 0, IOSSM_TIMEOUT_MS) != 0) {
					ERROR("%s: CMD_REQ not ready (post-cal)\n", __func__);
					hang();
				}
				cmd_req = (GET_MEM_CAL_STATUS << 0) |
					  (CMD_TRIG_MEM_CAL_OP << 16) |
					  (0 << 24) | (0 << 29);
				mmio_write_32(base + IOSSM_CMD_REQ_OFFSET, cmd_req);

				if (wait_for_bit((void *)(base + IOSSM_CMD_RESPONSE_STATUS_OFFSET),
					IOSSM_STATUS_COMMAND_RESPONSE_READY, 1,
					IOSSM_TIMEOUT_MS) != 0) {
					ERROR("%s: Response timeout (post-cal)\n", __func__);
					hang();
				}

				resp = mmio_read_32(base + (iface == 0 ?
				IOSSM_CMD_RESPONSE_DATA_0_OFFSET :
				IOSSM_CMD_RESPONSE_DATA_1_OFFSET));
				cal_stat = resp & GENMASK(2, 0);

				if (cal_stat < 0x2) {
					recal_success = true;
					break;
				}
			}

			if (!recal_success) {
				ERROR("IOSSM: SDRAM calibration failed (inst %d iface %d)\n",
				inst, iface);
				hang();
			}

			// Clear CMD_RESPONSE_READY
			mmio_clrbits_32(base + IOSSM_CMD_RESPONSE_STATUS_OFFSET,
					IOSSM_STATUS_COMMAND_RESPONSE_READY);
		}

		inst_ctrl->cal_status = true;
	}

	if (io96b_ctrl->io96b_0.cal_status && io96b_ctrl->io96b_1.cal_status) {
		INFO("IOSSM: %s: Overall SDRAM calibration success\n", __func__);
		io96b_ctrl->overall_cal_status = true;
	}

	return 0;
}

int get_mem_technology(struct io96b_info *io96b_ctrl)
{
	int inst_idx, iface_idx;
	phys_addr_t base;
	uint32_t cmd_req, resp;
	uint8_t ddr_type_ret;
	int iface_count;
	uint32_t *ip_type;
	uint32_t *ip_inst;

	/* Default to UNKNOWN */
	io96b_ctrl->ddr_type = ddr_type_list[6]; // "UNKNOWN"

	for (inst_idx = 0; inst_idx < io96b_ctrl->num_instance; inst_idx++) {
		struct io96b_instance *inst_ctrl = (inst_idx == 0) ?
			&io96b_ctrl->io96b_0 : &io96b_ctrl->io96b_1;

		base = inst_ctrl->io96b_csr_addr;
		iface_count = inst_ctrl->mb_ctrl.num_mem_interface;
		ip_type = inst_ctrl->mb_ctrl.ip_type;
		ip_inst = inst_ctrl->mb_ctrl.ip_instance_id;

		for (iface_idx = 0; iface_idx < iface_count; iface_idx++) {
			/* Ensure CMD_REQ is available */
			if (wait_for_bit((void *)(base + IOSSM_CMD_REQ_OFFSET),
					 GENMASK(31, 0), 0, IOSSM_TIMEOUT_MS) != 0) {
				ERROR("%s: CMD_REQ not ready (IO96B_%d iface %d)\n",
				__func__, inst_idx, iface_idx);
				return -ETIMEDOUT;
			}

			/* Compose and send CMD_REQ */
			cmd_req = (GET_MEM_TECHNOLOGY << 0) |
				  (CMD_GET_MEM_INFO << 16) |
				  (ip_inst[iface_idx] << 24) |
				  (ip_type[iface_idx] << 29);
			mmio_write_32(base + IOSSM_CMD_REQ_OFFSET, cmd_req);

			/* Wait for response */
			if (wait_for_bit((void *)(base + IOSSM_CMD_RESPONSE_STATUS_OFFSET),
				IOSSM_STATUS_COMMAND_RESPONSE_READY, 1, IOSSM_TIMEOUT_MS) != 0) {
				ERROR("%s: CMD_RESPONSE timeout (IO96B_%d iface %d)\n",
				__func__, inst_idx, iface_idx);
				return -ETIMEDOUT;
			}

			/* Extract and interpret DDR type */
			resp = mmio_read_32(base + IOSSM_CMD_RESPONSE_STATUS_OFFSET);
			ddr_type_ret = IOSSM_CMD_RESPONSE_DATA_SHORT(resp) & GENMASK(2, 0);

			if (ddr_type_ret > 6) {
				ERROR("%s: Invalid DDR type ID: %u\n", __func__, ddr_type_ret);
				return -EINVAL;
			}

			if (strcmp(io96b_ctrl->ddr_type, "UNKNOWN") == 0) {
				io96b_ctrl->ddr_type = ddr_type_list[ddr_type_ret];
			} else if (strcmp(ddr_type_list[ddr_type_ret], io96b_ctrl->ddr_type) != 0) {
				ERROR("IOSSM: DDR type mismatch on IO96B_%d iface %d: %s != %s\n",
				      inst_idx, iface_idx,
				      io96b_ctrl->ddr_type, ddr_type_list[ddr_type_ret]);
				return -ENOEXEC;
			}

			INFO("IOSSM: DDR type on IO96B_%d iface %d is %s\n",
			     inst_idx, iface_idx, io96b_ctrl->ddr_type);

			/* Clear CMD_RESPONSE_READY */
			mmio_clrbits_32(base + IOSSM_CMD_RESPONSE_STATUS_OFFSET,
					IOSSM_STATUS_COMMAND_RESPONSE_READY);
		}
	}

	return 0;
}

int get_mem_width_info(struct io96b_info *io96b_ctrl)
{
	int i, j;
	phys_size_t memory_size = 0U;
	phys_size_t total_memory_size = 0U;
	uint32_t mem_width_info;
	uint32_t mem_total_capacity_intf_offset[MAX_MEM_INTERFACE_SUPPORTED] = {
		IOSSM_MEM_TOTAL_CAPACITY_INTF0_OFFSET,
		IOSSM_MEM_TOTAL_CAPACITY_INTF1_OFFSET
	};

	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		struct io96b_instance *instance = (i == 0) ?
		&io96b_ctrl->io96b_0 : &io96b_ctrl->io96b_1;

		memory_size = 0;

		for (j = 0; j < instance->mb_ctrl.num_mem_interface; j++) {
			mem_width_info = mmio_read_32(instance->io96b_csr_addr +
						      mem_total_capacity_intf_offset[j]);

			instance->mb_ctrl.memory_size[j] =
				FIELD_GET(INTF_CAPACITY_GBITS_MASK, mem_width_info) * SZ_1G / SZ_8;

			if (instance->mb_ctrl.memory_size[j] != 0)
				memory_size += instance->mb_ctrl.memory_size[j];
		}

		if (memory_size == 0U) {
			ERROR("IOSSM: %s: Failed to get valid memory size on IO96B_%d\n",
			__func__, i);
			return -ENOEXEC;
		}

		total_memory_size += memory_size;
	}

	if (total_memory_size == 0U) {
		ERROR("IOSSM: %s: Total memory size is zero\n", __func__);
		return -ENOEXEC;
	}

	io96b_ctrl->overall_size = total_memory_size;
	return 0;
}

static inline void print_ecc_enable_status(uint32_t ecc_status)
{
	uint8_t ecc_en_type = ecc_status & GENMASK(1, 0);
	bool ecc_type = ecc_status & BIT(8) ? true : false;

	INFO("DDR: ECC enable type: %s\n",
		(ecc_en_type == 0) ? "Disabled" :
		((ecc_en_type == 1) ? "Enabled without error correction or detection" :
		((ecc_en_type == 2) ? "Enabled with error correction, without detection" :
		((ecc_en_type == 3) ? "Enabled with error correction and detection" : "Unknown"))));

	if (ecc_en_type != 0)
		INFO("DDR: ECC type: %s\n", ecc_type ? "In-line" : "Out-of-Band");
}

int ecc_enable_status(struct io96b_info *io96b_ctrl)
{
	int i, j;
	bool ecc_stat_set = false;
	bool ecc_stat, is_inline_ecc;
	uint32_t ecc_enable_interface;
	uint32_t ecc_enable_intf_offset[MAX_MEM_INTERFACE_SUPPORTED] = {
		IOSSM_ECC_ENABLE_INTF0_OFFSET,
		IOSSM_ECC_ENABLE_INTF1_OFFSET
	};

	/* Initialize ECC status */
	io96b_ctrl->ecc_status = false;
	io96b_ctrl->is_inline_ecc = false;

	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		struct io96b_instance *inst_ctrl = (i == 0) ?
		&io96b_ctrl->io96b_0 : &io96b_ctrl->io96b_1;

		for (j = 0; j < inst_ctrl->mb_ctrl.num_mem_interface; j++) {
			ecc_enable_interface = mmio_read_32(inst_ctrl->io96b_csr_addr +
							    ecc_enable_intf_offset[j]);

			print_ecc_enable_status(ecc_enable_interface);

			ecc_stat = FIELD_GET(INTF_ECC_ENABLE_TYPE_MASK, ecc_enable_interface) != 0;
			is_inline_ecc = FIELD_GET(INTF_ECC_TYPE_MASK, ecc_enable_interface);

			if (!ecc_stat_set) {
				io96b_ctrl->ecc_status = ecc_stat;
				if (ecc_stat) {
					io96b_ctrl->is_inline_ecc = is_inline_ecc;
				}
				ecc_stat_set = true;
			}

			if (ecc_stat != io96b_ctrl->ecc_status) {
				ERROR("IOSSM: %s: Mismatch ECC status on IO96B_%d INT%d\n",
				      __func__, i, j);
				return -ENOEXEC;
			}
		}
	}

	NOTICE("DDR: ECC is %s%s\n",
			io96b_ctrl->ecc_status ? "enabled" : "disabled",
			io96b_ctrl->ecc_status && io96b_ctrl->is_inline_ecc ? " (inline ECC)" : "");

	return 0;
}

static bool is_double_bit_error(enum ecc_error_type err_type)
{
	switch (err_type) {
	case DOUBLE_BIT_ERROR:
	case MULTIPLE_DOUBLE_BIT_ERRORS:
	case WRITE_LINK_DOUBLE_BIT_ERROR:
	case READ_LINK_DOUBLE_BIT_ERROR:
	case READ_MODIFY_WRITE_DOUBLE_BIT_ERROR:
		return true;

	default:
		return false;
	}
}

bool get_ecc_dbe_status(struct io96b_info *io96b_ctrl)
{
	uint32_t ecc_err_status;
	uint16_t ecc_err_counter;
	bool ecc_dbe_err_flag = false;

	struct io96b_instance *instances[] = {
		&io96b_ctrl->io96b_0,
		&io96b_ctrl->io96b_1
	};

	for (uint32_t i = 0; i < io96b_ctrl->num_instance; i++) {
		struct io96b_instance *curr_inst = instances[i];
		uint32_t base = curr_inst->io96b_csr_addr;

		uint32_t instance_id = *curr_inst->mb_ctrl.ip_instance_id;
		uint32_t ip_type = *curr_inst->mb_ctrl.ip_type;

		/* Read ECC error status register */
		ecc_err_status = mmio_read_32(base + IOSSM_ECC_ERR_STATUS_OFFSET);
		ecc_err_counter = FIELD_GET(ECC_ERR_COUNTER_MASK, ecc_err_status);

		INFO("DDR: ECC error status register on IO96B_%d: 0x%x\n", i, ecc_err_status);

		if (ecc_err_counter == 0)
			continue;

		uint32_t address = base + IOSSM_ECC_ERR_DATA_START_OFFSET;

		for (uint32_t j = 0; j < ecc_err_counter && j < MAX_ECC_ERR_COUNT; j++) {
			uint32_t ecc_err_data = mmio_read_32(address);
			uint32_t ecc_err_data_lo = mmio_read_32(address + sizeof(uint32_t));

			INFO("DDR: ECC error details, buffer entry[%d]:", j);
			INFO("- err info addr: 0x%x", address);
			INFO("- err ip type: %lu", FIELD_GET(ECC_ERR_IP_TYPE_MASK, ecc_err_data));
			INFO("- err instance id: %lu",
			FIELD_GET(ECC_ERR_INSTANCE_ID_MASK, ecc_err_data));
			INFO("- err source id: %lu",
			FIELD_GET(ECC_ERR_SOURCE_ID_MASK, ecc_err_data));
			INFO("- err type: %lu", FIELD_GET(ECC_ERR_TYPE_MASK, ecc_err_data));
			INFO("- err addr upper: 0x%lx",
			FIELD_GET(ECC_ERR_ADDR_UPPER_MASK, ecc_err_data));
			INFO("- err addr lower: 0x%x", ecc_err_data_lo);

			if (is_double_bit_error(FIELD_GET(ECC_ERR_TYPE_MASK, ecc_err_data))) {
				ecc_dbe_err_flag = true;
			}

			address += sizeof(uint32_t) * 2;
		}

		NOTICE("DDR: ECC error count value %d", ecc_err_counter);
		NOTICE("DDR: ECC error overflow field 0x%lx",
		FIELD_GET(ECC_ERR_OVERFLOW_MASK, ecc_err_status));

		/* Clear the ECC error buffer using MMIO write (replacing io96b_mb_req) */
		uint32_t cmd_req = (IOSSM_ECC_CLEAR_ERR_BUFFER << 0) |
							(CMD_TRIG_CONTROLLER_OP << 16) |
							(instance_id << 24) |
							(ip_type << 29);

		mmio_write_32(base + IOSSM_CMD_REQ_OFFSET, cmd_req);
		mmio_write_32(base + IOSSM_CONTROLLER_TRIGGER_OFFSET, IOSSM_ECC_CLEAR_ERR_BUFFER);

		/* Read back to confirm reset */
		uint32_t curr_status = mmio_read_32(base + IOSSM_ECC_ERR_STATUS_OFFSET);

		INFO("DDR: %s: Post reset, ECC error count on IO96B_%d: %lu",
			__func__, i, FIELD_GET(ECC_ERR_COUNTER_MASK, curr_status));
	}

	return ecc_dbe_err_flag;
}

static int out_of_band_ecc_bist_mem_init(struct io96b_info *io96b_ctrl)
{
	struct io96b_mb_resp usr_resp;
	int i, j;
	bool bist_start, bist_success;
	uint32_t read_count = 0;
	uint32_t read_interval_ms = 500U;

	const uint32_t mem_init_status_offset[MAX_MEM_INTERFACE_SUPPORTED] = {
		IOSSM_MEM_INIT_STATUS_INTF0_OFFSET,
		IOSSM_MEM_INIT_STATUS_INTF1_OFFSET
	};

	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		struct io96b_instance *inst = (i == 0) ?
		&io96b_ctrl->io96b_0 : &io96b_ctrl->io96b_1;

		for (j = 0; j < inst->mb_ctrl.num_mem_interface; j++) {
			bist_start = false;
			bist_success = false;

			/* Start memory initialization BIST on full memory address */
			io96b_mb_req(inst->io96b_csr_addr,
				     inst->mb_ctrl.ip_type[j],
				     inst->mb_ctrl.ip_instance_id[j],
				     CMD_TRIG_CONTROLLER_OP, BIST_MEM_INIT_START, 0x40,
				     0, 0, 0, 0, 0, 0, 0, &usr_resp);

			bist_start = IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status) & 1;

			if (!bist_start)
				return -ENOEXEC;

			read_count = TIMEOUT / read_interval_ms;

			/* Polling for memory initialization BIST status */
			while (!bist_success) {
				uint32_t status = mmio_read_32(inst->io96b_csr_addr +
				mem_init_status_offset[j]);

				bist_success = FIELD_GET(INTF_BIST_STATUS_MASK, status);

				if (!bist_success && read_count == 0U) {
					ERROR("IOSSM: %s: Timeout init memory on IO96B_%d\n",
					__func__, i);
					ERROR("IOSSM: %s: BIST_MEM_INIT_STATUS Err code 0x%x\n",
					__func__, (IOSSM_CMD_RESPONSE_DATA_SHORT
					(usr_resp.cmd_resp_status) & GENMASK(2, 1)) > 0x1);
					return -ETIMEDOUT;
				}
				read_count--;
				mdelay(read_interval_ms);
			}
		}
		NOTICE("IOSSM: Memory initialized successfully on IO96B\n");
	}
	return 0;
}

static int inline_ecc_bist_mem_init(struct io96b_info *io96b_ctrl)
{
	int i, j;
	struct io96b_mb_resp usr_resp;
	bool bist_start, bist_success;
	phys_size_t mem_size, chunk_size;
	uint32_t mem_exp, cmd_param_0, cmd_param_1, cmd_param_2;
	uint32_t read_count, read_interval_ms = 500U;

	const uint32_t mem_init_status_offset[] = { 0x260, 0x2E0 };

	for (i = 0; i < io96b_ctrl->num_instance; i++) {
		struct io96b_instance *inst = (i == 0) ?
						&io96b_ctrl->io96b_0 : &io96b_ctrl->io96b_1;

		for (j = 0; j < inst->mb_ctrl.num_mem_interface; j++) {
			mem_size = inst->mb_ctrl.memory_size[j];
			chunk_size = mem_size;
			mem_exp = 0;
			bist_start = false;
			bist_success = false;

			if (mem_size == 0 || (mem_size & (mem_size - 1)) != 0) {
				ERROR("IOSSM: %s: Wrong memory size - not power of 2!\n", __func__);
				return -ENOEXEC;
			}

			while (chunk_size >>= 1)
				mem_exp++;

			cmd_param_0 = FIELD_PREP(BIST_START_ADDR_SPACE_MASK, mem_exp);
			cmd_param_1 = FIELD_GET(BIST_START_ADDR_LOW_MASK, 0);
			cmd_param_2 = FIELD_GET(BIST_START_ADDR_HIGH_MASK, 0);

			io96b_mb_req(inst->io96b_csr_addr,
				inst->mb_ctrl.ip_type[j],
				inst->mb_ctrl.ip_instance_id[j],
				CMD_TRIG_CONTROLLER_OP, BIST_MEM_INIT_START,
				cmd_param_0, cmd_param_1, cmd_param_2,
				0, 0, 0, 0, 0, &usr_resp);

			bist_start = IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status) & 1;

			if (!bist_start) {
				ERROR("IOSSM: %s: Failed to initialize memory on IO96B_%d\n",
						__func__, i);
				ERROR("IOSSM: %s: BIST_MEM_INIT_START Error code 0x%x\n",
					__func__,
					(IOSSM_CMD_RESPONSE_DATA_SHORT(usr_resp.cmd_resp_status)
					 & GENMASK(2, 1)) > 0x1);
				return -ENOEXEC;
			}

			read_count = read_interval_ms / TIMEOUT;

			while (!bist_success) {
				uint32_t status = mmio_read_32(inst->io96b_csr_addr +
								mem_init_status_offset[j]);

				bist_success = status & BIT(0);

				if ((!bist_success) && (read_count == 0U)) {
					ERROR("IOSSM: %s: Timeout init memory on IO96B_%d\n",
							__func__, i);
					ERROR("IOSSM: %s: BIST_MEM_INIT_STATUS raw status = 0x%x\n",
							__func__, status);
					return -ETIMEDOUT;
				}
				read_count--;
				mdelay(read_interval_ms);
			}
		}

		NOTICE("IOSSM: %s: Memory initialized successfully on IO96B_%d\n", __func__, i);
	}
	return 0;
}

/*
 * Memory initialization BIST (Built-In Self-Test) start function.
 * This function will call either inline ECC or out-of-band BIST memory init
 * based on the ECC type status.
 */
int bist_mem_init_start(struct io96b_info *io96b_ctrl)
{
	if (io96b_ctrl->is_inline_ecc)
		return inline_ecc_bist_mem_init(io96b_ctrl);
	else
		return out_of_band_ecc_bist_mem_init(io96b_ctrl);
}
