/*
 * Copyright (c) 2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include "ddr.h"
#include <lib/mmio.h>
#include "socfpga_handoff.h"

int ddr_calibration_check(void)
{
	// DDR calibration check
	int status = 0;
	uint32_t u32data_read = 0;

	NOTICE("DDR: Access address 0x%x:...\n", IO96B_0_REG_BASE);
	u32data_read = mmio_read_32(IO96B_0_REG_BASE);
	NOTICE("DDR: Access address 0x%x: read 0x%04x\n", IO96B_0_REG_BASE, u32data_read);

	if (u32data_read == -EPERM) {
		status = -EPERM;
		assert(u32data_read);
	}

	u32data_read = 0x0;
	NOTICE("DDR: Access address 0x%x: ...\n", IO96B_1_REG_BASE);
	u32data_read = mmio_read_32(IO96B_1_REG_BASE);
	NOTICE("DDR: Access address 0x%x: read 0x%04x\n", IO96B_1_REG_BASE, u32data_read);

	if (u32data_read == -EPERM) {
		status = -EPERM;
		assert(u32data_read);
	}

	return status;
}

int iossm_mb_init(void)
{
	// int status;

	// Update according to IOSSM mailbox spec

	// if (status) {
		// return status;
	// }

	return 0;
}

int wait_respond(uint16_t timeout)
{
	uint32_t status = 0;
	uint32_t count = 0;
	uint32_t data = 0;

	/* Wait status command response ready */
	do {
		data = mmio_read_32(IO96B_CSR_REG(CMD_RESPONSE_STATUS));
		count++;
		if (count >= timeout) {
			return -ETIMEDOUT;
		}

	} while (STATUS_COMMAND_RESPONSE(data) != STATUS_COMMAND_RESPONSE_READY);

	status = (data & STATUS_GENERAL_ERROR_MASK) >> STATUS_GENERAL_ERROR_OFFSET;
	if (status != 0) {
		return status;
	}

	status = (data & STATUS_CMD_RESPONSE_ERROR_MASK) >> STATUS_CMD_RESPONSE_ERROR_OFFSET;
	if (status != 0) {
		return status;
	}

	return status;
}

int iossm_mb_read_response(void)
{
	uint32_t status = 0;
	unsigned int i;
	uint32_t resp_data[IOSSM_RESP_MAX_WORD_SIZE];
	uint32_t resp_param_reg;

	// Check STATUS_CMD_RESPONSE_DATA_PTR_VALID in
	// STATUS_COMMAND_RESPONSE to ensure data pointer response

	/* Read CMD_RESPONSE_STATUS and CMD_RESPONSE_DATA_* */
	resp_data[0] = mmio_read_32(IO96B_CSR_REG(CMD_RESPONSE_STATUS));
	resp_data[0] = (resp_data[0] & CMD_RESPONSE_DATA_SHORT_MASK) >>
			CMD_RESPONSE_DATA_SHORT_OFFSET;
	resp_param_reg = CMD_RESPONSE_STATUS;
	for (i = 1; i < IOSSM_RESP_MAX_WORD_SIZE; i++) {
		resp_param_reg = resp_param_reg - CMD_RESPONSE_OFFSET;
		resp_data[i] = mmio_read_32(IO96B_CSR_REG(resp_param_reg));
	}

	/* Wait for STATUS_COMMAND_RESPONSE_READY*/
	status = wait_respond(1000);

	/* Read CMD_RESPONSE_STATUS and CMD_RESPONSE_DATA_* */
	mmio_setbits_32(STATUS_COMMAND_RESPONSE(IO96B_CSR_REG(
			CMD_RESPONSE_STATUS)),
			STATUS_COMMAND_RESPONSE_READY_CLEAR);

	return status;
}

int iossm_mb_send(uint32_t cmd_target_ip_type, uint32_t cmd_target_ip_instance_id,
			uint32_t cmd_type, uint32_t cmd_opcode, uint32_t *args,
			unsigned int len)
{
	unsigned int i;
	uint32_t status = 0;
	uint32_t cmd_req;
	uint32_t cmd_param_reg;

	cmd_target_ip_type = (cmd_target_ip_type & CMD_TARGET_IP_TYPE_MASK) <<
				CMD_TARGET_IP_TYPE_OFFSET;
	cmd_target_ip_instance_id = (cmd_target_ip_instance_id &
				CMD_TARGET_IP_INSTANCE_ID_MASK) <<
				CMD_TARGET_IP_INSTANCE_ID_OFFSET;
	cmd_type = (cmd_type & CMD_TYPE_MASK) << CMD_TYPE_OFFSET;
	cmd_opcode = (cmd_opcode & CMD_OPCODE_MASK) << CMD_OPCODE_OFFSET;
	cmd_req = cmd_target_ip_type | cmd_target_ip_instance_id | cmd_type |
			cmd_opcode;

	/* send mailbox request */
	IOSSM_MB_WRITE(IO96B_CSR_REG(CMD_REQ), cmd_req);
	if (len != 0) {
		cmd_param_reg = CMD_REQ;
		for (i = 0; i < len; i++) {
			cmd_param_reg = cmd_param_reg - CMD_PARAM_OFFSET;
			IOSSM_MB_WRITE(IO96B_CSR_REG(cmd_param_reg), args[i]);
		}
	}

	status = iossm_mb_read_response();
	if (status != 0) {
		return status;
	}

	return status;
}

int ddr_iossm_mailbox_cmd(uint32_t cmd_opcode)
{
	// IOSSM
	uint32_t status = 0;
	unsigned int i = 0;
	uint32_t payload[IOSSM_CMD_MAX_WORD_SIZE] = {0U};

	switch (cmd_opcode) {
	case CMD_INIT:
		status = iossm_mb_init();
		break;

	case OPCODE_GET_MEM_INTF_INFO:
		status = iossm_mb_send(0, 0, MBOX_CMD_GET_SYS_INFO,
		OPCODE_GET_MEM_INTF_INFO, payload, i);
		break;

	case OPCODE_GET_MEM_TECHNOLOGY:
		status = iossm_mb_send(0, 0, MBOX_CMD_GET_MEM_INFO,
		OPCODE_GET_MEM_TECHNOLOGY, payload, i);
		break;

	case OPCODE_GET_MEM_WIDTH_INFO:
		status = iossm_mb_send(0, 0, MBOX_CMD_GET_MEM_INFO,
		OPCODE_GET_MEM_WIDTH_INFO, payload, i);
		break;

	case OPCODE_ECC_ENABLE_STATUS:
		status = iossm_mb_send(0, 0,
		MBOX_CMD_TRIG_CONTROLLER_OP, OPCODE_ECC_ENABLE_STATUS,
		payload, i);
		break;

	case OPCODE_ECC_INTERRUPT_MASK:
		// payload[i] = CMD_PARAM_0 [16:0]: ECC_INTERRUPT_MASK
		status = iossm_mb_send(0, 0,
		MBOX_CMD_TRIG_CONTROLLER_OP, OPCODE_ECC_INTERRUPT_MASK,
		payload, i);
		break;

	case OPCODE_ECC_SCRUB_MODE_0_START:
		// payload[i] = CMD_PARAM_0 [15:0]: ECC_SCRUB_INTERVAL
		//i++;
		// payload[i] = CMD_PARAM_1 [11:0]: ECC_SCRUB_LEN
		//i++;
		// payload[i] = CMD_PARAM_2 [0:0]: ECC_SCRUB_FULL_MEM
		//i++;
		// payload[i]= CMD_PARAM_3 [31:0]: ECC_SCRUB_START_ADDR [31:0]
		//i++;
		// payload[i] = CMD_PARAM_4 [5:0]: ECC_SCRUB_START_ADDR [36:32]
		//i++;
		// payload[i] = CMD_PARAM_5 [31:0]: ECC_SCRUB_END_ADDR [31:0]
		//i++;
		// payload[i] = CMD_PARAM_6 [5:0]: ECC_SCRUB_END_ADDR [36:32]
		//i++;
		status = iossm_mb_send(0, 0,
		MBOX_CMD_TRIG_CONTROLLER_OP, OPCODE_ECC_SCRUB_MODE_0_START,
		payload, i);
		break;

	case OPCODE_ECC_SCRUB_MODE_1_START:
		// payload[i] = CMD_PARAM_0 [15:0]: ECC_SCRUB_IDLE_CNT
		//i++;
		// payload[i] = CMD_PARAM_1 [11:0]: ECC_SCRUB_LEN
		//i++;
		// payload[i] = CMD_PARAM_2 [0:0]: ECC_SCRUB_FULL_MEM
		//i++;
		// payload[i] = CMD_PARAM_3 [31:0]: ECC_SCRUB_START_ADDR [31:0]
		//i++;
		// payload[i] = CMD_PARAM_4 [5:0]: ECC_SCRUB_START_ADDR [36:32]
		//i++;
		// payload[i] = CMD_PARAM_5 [31:0]: ECC_SCRUB_END_ADDR [31:0]
		//i++;
		// payload[i] = CMD_PARAM_6 [5:0]: ECC_SCRUB_END_ADDR [36:32]
		//i++;
		status = iossm_mb_send(0, 0,
		MBOX_CMD_TRIG_CONTROLLER_OP, OPCODE_ECC_SCRUB_MODE_1_START,
		payload, i);
		break;

	case OPCODE_BIST_RESULTS_STATUS:
		status = iossm_mb_send(0, 0,
		MBOX_CMD_TRIG_CONTROLLER_OP, OPCODE_BIST_RESULTS_STATUS,
		payload, i);
		break;

	case OPCODE_BIST_MEM_INIT_START:
		status = iossm_mb_send(0, 0,
		MBOX_CMD_TRIG_CONTROLLER_OP, OPCODE_BIST_MEM_INIT_START,
		payload, i);
		break;

	case OPCODE_TRIG_MEM_CAL:
		status = iossm_mb_send(0, 0, MBOX_CMD_TRIG_MEM_CAL_OP,
		OPCODE_TRIG_MEM_CAL, payload, i);
		break;

	default:
		break;
	}

	if (status == -EPERM) {
		assert(status);
	}

	return status;
}

int ddr_config_handoff(handoff *hoff_ptr)
{
	/* Populate DDR handoff data */
	/* TODO: To add in DDR handoff configuration once available */
	return 0;
}

// DDR firewall and non secure access
void ddr_enable_ns_access(void)
{
	/* Please set the ddr non secure registers accordingly */

	mmio_setbits_32(CCU_REG(DMI0_DMIUSMCTCR),
			CCU_DMI_ALLOCEN | CCU_DMI_LOOKUPEN);
	mmio_setbits_32(CCU_REG(DMI1_DMIUSMCTCR),
			CCU_DMI_ALLOCEN | CCU_DMI_LOOKUPEN);

	/* TODO: To add in CCU NCORE OCRAM bypass mask for non secure registers */
	NOTICE("DDR non secure configured\n");
}

void ddr_enable_firewall(void)
{
	/* Please set the ddr firewall registers accordingly */
	/* TODO: To add in CCU NCORE OCRAM bypass mask for firewall registers */
	NOTICE("DDR firewall enabled\n");
}

bool is_ddr_init_in_progress(void)
{
	uint32_t reg = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_POR_0));

	if (reg & SOCFPGA_SYSMGR_BOOT_SCRATCH_POR_0_MASK) {
		return true;
	}
	return false;
}

int ddr_init(void)
{
	// DDR driver initialization
	int status = -EPERM;
	uint32_t cmd_opcode = 0;

	// Check and set Boot Scratch Register
	if (is_ddr_init_in_progress()) {
		return status;
	}
	mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_POR_0), 0x01);

	// Populate DDR handoff data
	handoff reverse_handoff_ptr;

	if (!socfpga_get_handoff(&reverse_handoff_ptr)) {
		assert(status);
	}
	status = ddr_config_handoff(&reverse_handoff_ptr);
	if (status == -EPERM) {
		assert(status);
	}

	// CCU and firewall setup
	ddr_enable_ns_access();
	ddr_enable_firewall();

	// DDR calibration check
	status = ddr_calibration_check();
	if (status == -EPERM) {
		assert(status);
	}

	// DDR mailbox command
	status = ddr_iossm_mailbox_cmd(cmd_opcode);
	if (status != 0) {
		assert(status);
	}

	// Check and set Boot Scratch Register
	mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_POR_0), 0x00);

	NOTICE("DDR init successfully\n");
	return status;
}

int ddr_config_scrubber(phys_addr_t umctl2_base, enum ddr_type umctl2_type)
{
	uint32_t temp[9] = {0};
	int ret = 0;

	/* Write default value to prevent scrubber stop due to lower power */
	mmio_write_32(0, umctl2_base + DDR4_PWRCTL_OFFSET);

	/* To backup user configurations in temp array */
	temp[0] = mmio_read_32(umctl2_base + DDR4_SBRCTL_OFFSET);
	temp[1] = mmio_read_32(umctl2_base + DDR4_SBRWDATA0_OFFSET);
	temp[2] = mmio_read_32(umctl2_base + DDR4_SBRSTART0_OFFSET);
	if (umctl2_type == DDR_TYPE_DDR4) {
		temp[3] = mmio_read_32(umctl2_base + DDR4_SBRWDATA1_OFFSET);
		temp[4] = mmio_read_32(umctl2_base + DDR4_SBRSTART1_OFFSET);
	}
	temp[5] = mmio_read_32(umctl2_base + DDR4_SBRRANGE0_OFFSET);
	temp[6] = mmio_read_32(umctl2_base + DDR4_SBRRANGE1_OFFSET);
	temp[7] = mmio_read_32(umctl2_base + DDR4_ECCCFG0_OFFSET);
	temp[8] = mmio_read_32(umctl2_base + DDR4_ECCCFG1_OFFSET);

	if (umctl2_type != DDR_TYPE_DDR4) {
		/* Lock ECC region, ensure this regions is not being accessed */
		mmio_setbits_32(umctl2_base + DDR4_ECCCFG1_OFFSET,
			     LPDDR4_ECCCFG1_ECC_REGIONS_PARITY_LOCK);
	}
	/* Disable input traffic per port */
	mmio_clrbits_32(umctl2_base + DDR4_PCTRL0_OFFSET, DDR4_PCTRL0_PORT_EN);
	/* Disables scrubber */
	mmio_clrbits_32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);
	/* Polling all scrub writes data have been sent */
	ret = poll_idle_status((umctl2_base + DDR4_SBRSTAT_OFFSET),
			       DDR4_SBRSTAT_SCRUB_BUSY, true, 5000);

	if (ret) {
		INFO("%s: Timeout while waiting for", __func__);
		INFO(" sending all scrub data\n");
		return ret;
	}

	/* LPDDR4 supports inline ECC only */
	if (umctl2_type != DDR_TYPE_DDR4) {
		/*
		 * Setting all regions for protected, this is required for
		 * srubber to init whole LPDDR4 expect ECC region
		 */
		mmio_write_32(((ONE_EIGHT <<
		       LPDDR4_ECCCFG0_ECC_REGION_MAP_GRANU_SHIFT) |
		       (ALL_PROTECTED << LPDDR4_ECCCFG0_ECC_REGION_MAP_SHIFT)),
		       umctl2_base + DDR4_ECCCFG0_OFFSET);
	}

	/* Scrub_burst = 1, scrub_mode = 1(performs writes) */
	mmio_write_32(DDR4_SBRCTL_SCRUB_BURST_1 | DDR4_SBRCTL_SCRUB_WRITE,
	       umctl2_base + DDR4_SBRCTL_OFFSET);

	/* Wipe DDR content after calibration */
	ret = ddr_zerofill_scrubber(umctl2_base, umctl2_type);
	if (ret) {
		ERROR("Failed to clear DDR content\n");
	}

	/* Polling all scrub writes data have been sent */
	ret = poll_idle_status((umctl2_base + DDR4_SBRSTAT_OFFSET),
			       DDR4_SBRSTAT_SCRUB_BUSY, true, 5000);
	if (ret) {
		INFO("%s: Timeout while waiting for", __func__);
		INFO(" sending all scrub data\n");
		return ret;
	}

	/* Disables scrubber */
	mmio_clrbits_32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);

	/* Restore user configurations */
	mmio_write_32(temp[0], umctl2_base + DDR4_SBRCTL_OFFSET);
	mmio_write_32(temp[1], umctl2_base + DDR4_SBRWDATA0_OFFSET);
	mmio_write_32(temp[2], umctl2_base + DDR4_SBRSTART0_OFFSET);
	if (umctl2_type == DDR_TYPE_DDR4) {
		mmio_write_32(temp[3], umctl2_base + DDR4_SBRWDATA1_OFFSET);
		mmio_write_32(temp[4], umctl2_base + DDR4_SBRSTART1_OFFSET);
	}
	mmio_write_32(temp[5], umctl2_base + DDR4_SBRRANGE0_OFFSET);
	mmio_write_32(temp[6], umctl2_base + DDR4_SBRRANGE1_OFFSET);
	mmio_write_32(temp[7], umctl2_base + DDR4_ECCCFG0_OFFSET);
	mmio_write_32(temp[8], umctl2_base + DDR4_ECCCFG1_OFFSET);

	/* Enables ECC scrub on scrubber */
	if (!(mmio_read_32(umctl2_base + DDR4_SBRCTL_OFFSET) & DDR4_SBRCTL_SCRUB_WRITE)) {
		/* Enables scrubber */
		mmio_setbits_32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);
	}

	return 0;
}

int ddr_zerofill_scrubber(phys_addr_t umctl2_base, enum ddr_type umctl2_type)
{
	int ret = 0;

	/* Zeroing whole DDR */
	mmio_write_32(0, umctl2_base + DDR4_SBRWDATA0_OFFSET);
	mmio_write_32(0, umctl2_base + DDR4_SBRSTART0_OFFSET);
	if (umctl2_type == DDR_TYPE_DDR4) {
		mmio_write_32(0, umctl2_base + DDR4_SBRWDATA1_OFFSET);
		mmio_write_32(0, umctl2_base + DDR4_SBRSTART1_OFFSET);
	}
	mmio_write_32(0, umctl2_base + DDR4_SBRRANGE0_OFFSET);
	mmio_write_32(0, umctl2_base + DDR4_SBRRANGE1_OFFSET);

	NOTICE("Enabling scrubber (zeroing whole DDR) ...\n");

	/* Enables scrubber */
	mmio_setbits_32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);
	/* Polling all scrub writes commands have been sent */
	ret = poll_idle_status((umctl2_base + DDR4_SBRSTAT_OFFSET),
			       DDR4_SBRSTAT_SCRUB_DONE, true, 5000);
	if (ret) {
		INFO("%s: Timeout while waiting for", __func__);
		INFO(" sending all scrub commands\n");
		return ret;
	}

	return 0;
}

int poll_idle_status(uint32_t addr, uint32_t mask, uint32_t match, uint32_t delay_ms)
{
	int time_out = delay_ms;

	while (time_out-- > 0) {

		if ((mmio_read_32(addr) & mask) == match) {
			return 0;
		}
		udelay(1000);
	}
	return -ETIMEDOUT;
}
