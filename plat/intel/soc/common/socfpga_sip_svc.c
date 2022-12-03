/*
 * Copyright (c) 2019-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/mmio.h>
#include <tools_share/uuid.h>

#include "socfpga_fcs.h"
#include "socfpga_mailbox.h"
#include "socfpga_reset_manager.h"
#include "socfpga_sip_svc.h"


/* Total buffer the driver can hold */
#define FPGA_CONFIG_BUFFER_SIZE 4

static config_type request_type = NO_REQUEST;
static int current_block, current_buffer;
static int read_block, max_blocks;
static uint32_t send_id, rcv_id;
static uint32_t bytes_per_block, blocks_submitted;
static bool bridge_disable;

/* RSU static variables */
static uint32_t rsu_dcmf_ver[4] = {0};
static uint16_t rsu_dcmf_stat[4] = {0};
static uint32_t rsu_max_retry;

/*  SiP Service UUID */
DEFINE_SVC_UUID2(intl_svc_uid,
		0xa85273b0, 0xe85a, 0x4862, 0xa6, 0x2a,
		0xfa, 0x88, 0x88, 0x17, 0x68, 0x81);

static uint64_t socfpga_sip_handler(uint32_t smc_fid,
				   uint64_t x1,
				   uint64_t x2,
				   uint64_t x3,
				   uint64_t x4,
				   void *cookie,
				   void *handle,
				   uint64_t flags)
{
	ERROR("%s: unhandled SMC (0x%x)\n", __func__, smc_fid);
	SMC_RET1(handle, SMC_UNK);
}

struct fpga_config_info fpga_config_buffers[FPGA_CONFIG_BUFFER_SIZE];

static int intel_fpga_sdm_write_buffer(struct fpga_config_info *buffer)
{
	uint32_t args[3];

	while (max_blocks > 0 && buffer->size > buffer->size_written) {
		args[0] = (1<<8);
		args[1] = buffer->addr + buffer->size_written;
		if (buffer->size - buffer->size_written <= bytes_per_block) {
			args[2] = buffer->size - buffer->size_written;
			current_buffer++;
			current_buffer %= FPGA_CONFIG_BUFFER_SIZE;
		} else {
			args[2] = bytes_per_block;
		}

		buffer->size_written += args[2];
		mailbox_send_cmd_async(&send_id, MBOX_RECONFIG_DATA, args,
					3U, CMD_INDIRECT);

		buffer->subblocks_sent++;
		max_blocks--;
	}

	return !max_blocks;
}

static int intel_fpga_sdm_write_all(void)
{
	for (int i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		if (intel_fpga_sdm_write_buffer(
			&fpga_config_buffers[current_buffer])) {
			break;
		}
	}
	return 0;
}

static uint32_t intel_mailbox_fpga_config_isdone(void)
{
	uint32_t ret;

	switch (request_type) {
	case RECONFIGURATION:
		ret = intel_mailbox_get_config_status(MBOX_RECONFIG_STATUS,
							true);
		break;
	case BITSTREAM_AUTH:
		ret = intel_mailbox_get_config_status(MBOX_RECONFIG_STATUS,
							false);
		break;
	default:
		ret = intel_mailbox_get_config_status(MBOX_CONFIG_STATUS,
							false);
		break;
	}

	if (ret != 0U) {
		if (ret == MBOX_CFGSTAT_STATE_CONFIG) {
			return INTEL_SIP_SMC_STATUS_BUSY;
		} else {
			request_type = NO_REQUEST;
			return INTEL_SIP_SMC_STATUS_ERROR;
		}
	}

	if (bridge_disable != 0U) {
		socfpga_bridges_enable(~0);	/* Enable bridge */
		bridge_disable = false;
	}
	request_type = NO_REQUEST;

	return INTEL_SIP_SMC_STATUS_OK;
}

static int mark_last_buffer_xfer_completed(uint32_t *buffer_addr_completed)
{
	int i;

	for (i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		if (fpga_config_buffers[i].block_number == current_block) {
			fpga_config_buffers[i].subblocks_sent--;
			if (fpga_config_buffers[i].subblocks_sent == 0
			&& fpga_config_buffers[i].size <=
			fpga_config_buffers[i].size_written) {
				fpga_config_buffers[i].write_requested = 0;
				current_block++;
				*buffer_addr_completed =
					fpga_config_buffers[i].addr;
				return 0;
			}
		}
	}

	return -1;
}

static int intel_fpga_config_completed_write(uint32_t *completed_addr,
					uint32_t *count, uint32_t *job_id)
{
	uint32_t resp[5];
	unsigned int resp_len = ARRAY_SIZE(resp);
	int status = INTEL_SIP_SMC_STATUS_OK;
	int all_completed = 1;
	*count = 0;

	while (*count < 3) {

		status = mailbox_read_response(job_id,
				resp, &resp_len);

		if (status < 0) {
			break;
		}

		max_blocks++;

		if (mark_last_buffer_xfer_completed(
			&completed_addr[*count]) == 0) {
			*count = *count + 1;
		} else {
			break;
		}
	}

	if (*count <= 0) {
		if (status != MBOX_NO_RESPONSE &&
			status != MBOX_TIMEOUT && resp_len != 0) {
			mailbox_clear_response();
			request_type = NO_REQUEST;
			return INTEL_SIP_SMC_STATUS_ERROR;
		}

		*count = 0;
	}

	intel_fpga_sdm_write_all();

	if (*count > 0) {
		status = INTEL_SIP_SMC_STATUS_OK;
	} else if (*count == 0) {
		status = INTEL_SIP_SMC_STATUS_BUSY;
	}

	for (int i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		if (fpga_config_buffers[i].write_requested != 0) {
			all_completed = 0;
			break;
		}
	}

	if (all_completed == 1) {
		return INTEL_SIP_SMC_STATUS_OK;
	}

	return status;
}

static int intel_fpga_config_start(uint32_t flag)
{
	uint32_t argument = 0x1;
	uint32_t response[3];
	int status = 0;
	unsigned int size = 0;
	unsigned int resp_len = ARRAY_SIZE(response);

	request_type = RECONFIGURATION;

	if (!CONFIG_TEST_FLAG(flag, PARTIAL_CONFIG)) {
		bridge_disable = true;
	}

	if (CONFIG_TEST_FLAG(flag, AUTHENTICATION)) {
		size = 1;
		bridge_disable = false;
		request_type = BITSTREAM_AUTH;
	}

	mailbox_clear_response();

	mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_CANCEL, NULL, 0U,
			CMD_CASUAL, NULL, NULL);

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_RECONFIG, &argument, size,
			CMD_CASUAL, response, &resp_len);

	if (status < 0) {
		bridge_disable = false;
		request_type = NO_REQUEST;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	max_blocks = response[0];
	bytes_per_block = response[1];

	for (int i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		fpga_config_buffers[i].size = 0;
		fpga_config_buffers[i].size_written = 0;
		fpga_config_buffers[i].addr = 0;
		fpga_config_buffers[i].write_requested = 0;
		fpga_config_buffers[i].block_number = 0;
		fpga_config_buffers[i].subblocks_sent = 0;
	}

	blocks_submitted = 0;
	current_block = 0;
	read_block = 0;
	current_buffer = 0;

	/* Disable bridge on full reconfiguration */
	if (bridge_disable) {
		socfpga_bridges_disable(~0);
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

static bool is_fpga_config_buffer_full(void)
{
	for (int i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		if (!fpga_config_buffers[i].write_requested) {
			return false;
		}
	}
	return true;
}

bool is_address_in_ddr_range(uint64_t addr, uint64_t size)
{
	if (!addr && !size) {
		return true;
	}
	if (size > (UINT64_MAX - addr)) {
		return false;
	}
	if (addr < BL31_LIMIT) {
		return false;
	}
	if (addr + size > DRAM_BASE + DRAM_SIZE) {
		return false;
	}

	return true;
}

static uint32_t intel_fpga_config_write(uint64_t mem, uint64_t size)
{
	int i;

	intel_fpga_sdm_write_all();

	if (!is_address_in_ddr_range(mem, size) ||
		is_fpga_config_buffer_full()) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	for (i = 0; i < FPGA_CONFIG_BUFFER_SIZE; i++) {
		int j = (i + current_buffer) % FPGA_CONFIG_BUFFER_SIZE;

		if (!fpga_config_buffers[j].write_requested) {
			fpga_config_buffers[j].addr = mem;
			fpga_config_buffers[j].size = size;
			fpga_config_buffers[j].size_written = 0;
			fpga_config_buffers[j].write_requested = 1;
			fpga_config_buffers[j].block_number =
				blocks_submitted++;
			fpga_config_buffers[j].subblocks_sent = 0;
			break;
		}
	}

	if (is_fpga_config_buffer_full()) {
		return INTEL_SIP_SMC_STATUS_BUSY;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

static int is_out_of_sec_range(uint64_t reg_addr)
{
#if DEBUG
	return 0;
#endif

	switch (reg_addr) {
	case(0xF8011100):	/* ECCCTRL1 */
	case(0xF8011104):	/* ECCCTRL2 */
	case(0xF8011110):	/* ERRINTEN */
	case(0xF8011114):	/* ERRINTENS */
	case(0xF8011118):	/* ERRINTENR */
	case(0xF801111C):	/* INTMODE */
	case(0xF8011120):	/* INTSTAT */
	case(0xF8011124):	/* DIAGINTTEST */
	case(0xF801112C):	/* DERRADDRA */
	case(0xFA000000):	/* SMMU SCR0 */
	case(0xFA000004):	/* SMMU SCR1 */
	case(0xFA000400):	/* SMMU NSCR0 */
	case(0xFA004000):	/* SMMU SSD0_REG */
	case(0xFA000820):	/* SMMU SMR8 */
	case(0xFA000c20):	/* SMMU SCR8 */
	case(0xFA028000):	/* SMMU CB8_SCTRL */
	case(0xFA001020):	/* SMMU CBAR8 */
	case(0xFA028030):	/* SMMU TCR_LPAE */
	case(0xFA028020):	/* SMMU CB8_TTBR0_LOW */
	case(0xFA028024):	/* SMMU CB8_PRRR_HIGH */
	case(0xFA028038):	/* SMMU CB8_PRRR_MIR0 */
	case(0xFA02803C):	/* SMMU CB8_PRRR_MIR1 */
	case(0xFA028010):	/* SMMU_CB8)TCR2 */
	case(0xFFD080A4):	/* SDM SMMU STREAM ID REG */
	case(0xFA001820):	/* SMMU_CBA2R8 */
	case(0xFA000074):	/* SMMU_STLBGSTATUS */
	case(0xFA0287F4):	/* SMMU_CB8_TLBSTATUS */
	case(0xFA000060):	/* SMMU_STLBIALL */
	case(0xFA000070):	/* SMMU_STLBGSYNC */
	case(0xFA028618):	/* CB8_TLBALL */
	case(0xFA0287F0):	/* CB8_TLBSYNC */
	case(0xFFD12028):	/* SDMMCGRP_CTRL */
	case(0xFFD12044):	/* EMAC0 */
	case(0xFFD12048):	/* EMAC1 */
	case(0xFFD1204C):	/* EMAC2 */
	case(0xFFD12090):	/* ECC_INT_MASK_VALUE */
	case(0xFFD12094):	/* ECC_INT_MASK_SET */
	case(0xFFD12098):	/* ECC_INT_MASK_CLEAR */
	case(0xFFD1209C):	/* ECC_INTSTATUS_SERR */
	case(0xFFD120A0):	/* ECC_INTSTATUS_DERR */
	case(0xFFD120C0):	/* NOC_TIMEOUT */
	case(0xFFD120C4):	/* NOC_IDLEREQ_SET */
	case(0xFFD120C8):	/* NOC_IDLEREQ_CLR */
	case(0xFFD120D0):	/* NOC_IDLEACK */
	case(0xFFD120D4):	/* NOC_IDLESTATUS */
	case(0xFFD12200):	/* BOOT_SCRATCH_COLD0 */
	case(0xFFD12204):	/* BOOT_SCRATCH_COLD1 */
	case(0xFFD12220):	/* BOOT_SCRATCH_COLD8 */
	case(0xFFD12224):	/* BOOT_SCRATCH_COLD9 */
		return 0;

	default:
		break;
	}

	return -1;
}

/* Secure register access */
uint32_t intel_secure_reg_read(uint64_t reg_addr, uint32_t *retval)
{
	if (is_out_of_sec_range(reg_addr)) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*retval = mmio_read_32(reg_addr);

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_secure_reg_write(uint64_t reg_addr, uint32_t val,
				uint32_t *retval)
{
	if (is_out_of_sec_range(reg_addr)) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	mmio_write_32(reg_addr, val);

	return intel_secure_reg_read(reg_addr, retval);
}

uint32_t intel_secure_reg_update(uint64_t reg_addr, uint32_t mask,
				 uint32_t val, uint32_t *retval)
{
	if (!intel_secure_reg_read(reg_addr, retval)) {
		*retval &= ~mask;
		*retval |= val & mask;
		return intel_secure_reg_write(reg_addr, *retval, retval);
	}

	return INTEL_SIP_SMC_STATUS_ERROR;
}

/* Intel Remote System Update (RSU) services */
uint64_t intel_rsu_update_address;

static uint32_t intel_rsu_status(uint64_t *respbuf, unsigned int respbuf_sz)
{
	if (mailbox_rsu_status((uint32_t *)respbuf, respbuf_sz) < 0) {
		return INTEL_SIP_SMC_RSU_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

static uint32_t intel_rsu_update(uint64_t update_address)
{
	intel_rsu_update_address = update_address;
	return INTEL_SIP_SMC_STATUS_OK;
}

static uint32_t intel_rsu_notify(uint32_t execution_stage)
{
	if (mailbox_hps_stage_notify(execution_stage) < 0) {
		return INTEL_SIP_SMC_RSU_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

static uint32_t intel_rsu_retry_counter(uint32_t *respbuf, uint32_t respbuf_sz,
					uint32_t *ret_stat)
{
	if (mailbox_rsu_status((uint32_t *)respbuf, respbuf_sz) < 0) {
		return INTEL_SIP_SMC_RSU_ERROR;
	}

	*ret_stat = respbuf[8];
	return INTEL_SIP_SMC_STATUS_OK;
}

static uint32_t intel_rsu_copy_dcmf_version(uint64_t dcmf_ver_1_0,
					    uint64_t dcmf_ver_3_2)
{
	rsu_dcmf_ver[0] = dcmf_ver_1_0;
	rsu_dcmf_ver[1] = dcmf_ver_1_0 >> 32;
	rsu_dcmf_ver[2] = dcmf_ver_3_2;
	rsu_dcmf_ver[3] = dcmf_ver_3_2 >> 32;

	return INTEL_SIP_SMC_STATUS_OK;
}

static uint32_t intel_rsu_copy_dcmf_status(uint64_t dcmf_stat)
{
	rsu_dcmf_stat[0] = 0xFFFF & (dcmf_stat >> (0 * 16));
	rsu_dcmf_stat[1] = 0xFFFF & (dcmf_stat >> (1 * 16));
	rsu_dcmf_stat[2] = 0xFFFF & (dcmf_stat >> (2 * 16));
	rsu_dcmf_stat[3] = 0xFFFF & (dcmf_stat >> (3 * 16));

	return INTEL_SIP_SMC_STATUS_OK;
}

/* Intel HWMON services */
static uint32_t intel_hwmon_readtemp(uint32_t chan, uint32_t *retval)
{
	if (mailbox_hwmon_readtemp(chan, retval) < 0) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

static uint32_t intel_hwmon_readvolt(uint32_t chan, uint32_t *retval)
{
	if (mailbox_hwmon_readvolt(chan, retval) < 0) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

/* Mailbox services */
static uint32_t intel_smc_fw_version(uint32_t *fw_version)
{
	int status;
	unsigned int resp_len = CONFIG_STATUS_WORD_SIZE;
	uint32_t resp_data[CONFIG_STATUS_WORD_SIZE] = {0U};

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_CONFIG_STATUS, NULL, 0U,
			CMD_CASUAL, resp_data, &resp_len);

	if (status < 0) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	if (resp_len <= CONFIG_STATUS_FW_VER_OFFSET) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*fw_version = resp_data[CONFIG_STATUS_FW_VER_OFFSET] & CONFIG_STATUS_FW_VER_MASK;

	return INTEL_SIP_SMC_STATUS_OK;
}

static uint32_t intel_mbox_send_cmd(uint32_t cmd, uint32_t *args,
				unsigned int len, uint32_t urgent, uint64_t response,
				unsigned int resp_len, int *mbox_status,
				unsigned int *len_in_resp)
{
	*len_in_resp = 0;
	*mbox_status = GENERIC_RESPONSE_ERROR;

	if (!is_address_in_ddr_range((uint64_t)args, sizeof(uint32_t) * len)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	int status = mailbox_send_cmd(MBOX_JOB_ID, cmd, args, len, urgent,
					(uint32_t *) response, &resp_len);

	if (status < 0) {
		*mbox_status = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	*mbox_status = 0;
	*len_in_resp = resp_len;

	flush_dcache_range(response, resp_len * MBOX_WORD_BYTE);

	return INTEL_SIP_SMC_STATUS_OK;
}

static int intel_smc_get_usercode(uint32_t *user_code)
{
	int status;
	unsigned int resp_len = sizeof(user_code) / MBOX_WORD_BYTE;

	status = mailbox_send_cmd(MBOX_JOB_ID, MBOX_CMD_GET_USERCODE, NULL,
				0U, CMD_CASUAL, user_code, &resp_len);

	if (status < 0) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_smc_service_completed(uint64_t addr, uint32_t size,
				uint32_t mode, uint32_t *job_id,
				uint32_t *ret_size, uint32_t *mbox_error)
{
	int status = 0;
	uint32_t resp_len = size / MBOX_WORD_BYTE;

	if (resp_len > MBOX_DATA_MAX_LEN) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (!is_address_in_ddr_range(addr, size)) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

	if (mode == SERVICE_COMPLETED_MODE_ASYNC) {
		status = mailbox_read_response_async(job_id,
				NULL, (uint32_t *) addr, &resp_len, 0);
	} else {
		status = mailbox_read_response(job_id,
				(uint32_t *) addr, &resp_len);

		if (status == MBOX_NO_RESPONSE) {
			status = MBOX_BUSY;
		}
	}

	if (status == MBOX_NO_RESPONSE) {
		return INTEL_SIP_SMC_STATUS_NO_RESPONSE;
	}

	if (status == MBOX_BUSY) {
		return INTEL_SIP_SMC_STATUS_BUSY;
	}

	*ret_size = resp_len * MBOX_WORD_BYTE;
	flush_dcache_range(addr, *ret_size);

	if (status == MBOX_RET_SDOS_DECRYPTION_ERROR_102 ||
		status == MBOX_RET_SDOS_DECRYPTION_ERROR_103) {
		*mbox_error = -status;
	} else if (status != MBOX_RET_OK) {
		*mbox_error = -status;
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

/* Miscellaneous HPS services */
uint32_t intel_hps_set_bridges(uint64_t enable, uint64_t mask)
{
	int status = 0;

	if ((enable & SOCFPGA_BRIDGE_ENABLE) != 0U) {
		if ((enable & SOCFPGA_BRIDGE_HAS_MASK) != 0U) {
			status = socfpga_bridges_enable((uint32_t)mask);
		} else {
			status = socfpga_bridges_enable(~0);
		}
	} else {
		if ((enable & SOCFPGA_BRIDGE_HAS_MASK) != 0U) {
			status = socfpga_bridges_disable((uint32_t)mask);
		} else {
			status = socfpga_bridges_disable(~0);
		}
	}

	if (status < 0) {
		return INTEL_SIP_SMC_STATUS_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

/*
 * This function is responsible for handling all SiP calls from the NS world
 */

uintptr_t sip_smc_handler_v1(uint32_t smc_fid,
			 u_register_t x1,
			 u_register_t x2,
			 u_register_t x3,
			 u_register_t x4,
			 void *cookie,
			 void *handle,
			 u_register_t flags)
{
	uint32_t retval = 0, completed_addr[3];
	uint32_t retval2 = 0;
	uint32_t mbox_error = 0;
	uint64_t retval64, rsu_respbuf[9];
	int status = INTEL_SIP_SMC_STATUS_OK;
	int mbox_status;
	unsigned int len_in_resp;
	u_register_t x5, x6, x7;

	switch (smc_fid) {
	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, intl_svc_uid);

	case INTEL_SIP_SMC_FPGA_CONFIG_ISDONE:
		status = intel_mailbox_fpga_config_isdone();
		SMC_RET4(handle, status, 0, 0, 0);

	case INTEL_SIP_SMC_FPGA_CONFIG_GET_MEM:
		SMC_RET3(handle, INTEL_SIP_SMC_STATUS_OK,
			INTEL_SIP_SMC_FPGA_CONFIG_ADDR,
			INTEL_SIP_SMC_FPGA_CONFIG_SIZE -
				INTEL_SIP_SMC_FPGA_CONFIG_ADDR);

	case INTEL_SIP_SMC_FPGA_CONFIG_START:
		status = intel_fpga_config_start(x1);
		SMC_RET4(handle, status, 0, 0, 0);

	case INTEL_SIP_SMC_FPGA_CONFIG_WRITE:
		status = intel_fpga_config_write(x1, x2);
		SMC_RET4(handle, status, 0, 0, 0);

	case INTEL_SIP_SMC_FPGA_CONFIG_COMPLETED_WRITE:
		status = intel_fpga_config_completed_write(completed_addr,
							&retval, &rcv_id);
		switch (retval) {
		case 1:
			SMC_RET4(handle, INTEL_SIP_SMC_STATUS_OK,
				completed_addr[0], 0, 0);

		case 2:
			SMC_RET4(handle, INTEL_SIP_SMC_STATUS_OK,
				completed_addr[0],
				completed_addr[1], 0);

		case 3:
			SMC_RET4(handle, INTEL_SIP_SMC_STATUS_OK,
				completed_addr[0],
				completed_addr[1],
				completed_addr[2]);

		case 0:
			SMC_RET4(handle, status, 0, 0, 0);

		default:
			mailbox_clear_response();
			SMC_RET1(handle, INTEL_SIP_SMC_STATUS_ERROR);
		}

	case INTEL_SIP_SMC_REG_READ:
		status = intel_secure_reg_read(x1, &retval);
		SMC_RET3(handle, status, retval, x1);

	case INTEL_SIP_SMC_REG_WRITE:
		status = intel_secure_reg_write(x1, (uint32_t)x2, &retval);
		SMC_RET3(handle, status, retval, x1);

	case INTEL_SIP_SMC_REG_UPDATE:
		status = intel_secure_reg_update(x1, (uint32_t)x2,
						 (uint32_t)x3, &retval);
		SMC_RET3(handle, status, retval, x1);

	case INTEL_SIP_SMC_RSU_STATUS:
		status = intel_rsu_status(rsu_respbuf,
					ARRAY_SIZE(rsu_respbuf));
		if (status) {
			SMC_RET1(handle, status);
		} else {
			SMC_RET4(handle, rsu_respbuf[0], rsu_respbuf[1],
					rsu_respbuf[2], rsu_respbuf[3]);
		}

	case INTEL_SIP_SMC_RSU_UPDATE:
		status = intel_rsu_update(x1);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_RSU_NOTIFY:
		status = intel_rsu_notify(x1);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_RSU_RETRY_COUNTER:
		status = intel_rsu_retry_counter((uint32_t *)rsu_respbuf,
						ARRAY_SIZE(rsu_respbuf), &retval);
		if (status) {
			SMC_RET1(handle, status);
		} else {
			SMC_RET2(handle, status, retval);
		}

	case INTEL_SIP_SMC_RSU_DCMF_VERSION:
		SMC_RET3(handle, INTEL_SIP_SMC_STATUS_OK,
			 ((uint64_t)rsu_dcmf_ver[1] << 32) | rsu_dcmf_ver[0],
			 ((uint64_t)rsu_dcmf_ver[3] << 32) | rsu_dcmf_ver[2]);

	case INTEL_SIP_SMC_RSU_COPY_DCMF_VERSION:
		status = intel_rsu_copy_dcmf_version(x1, x2);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_RSU_DCMF_STATUS:
		SMC_RET2(handle, INTEL_SIP_SMC_STATUS_OK,
			 ((uint64_t)rsu_dcmf_stat[3] << 48) |
			 ((uint64_t)rsu_dcmf_stat[2] << 32) |
			 ((uint64_t)rsu_dcmf_stat[1] << 16) |
			 rsu_dcmf_stat[0]);

	case INTEL_SIP_SMC_RSU_COPY_DCMF_STATUS:
		status = intel_rsu_copy_dcmf_status(x1);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_RSU_MAX_RETRY:
		SMC_RET2(handle, INTEL_SIP_SMC_STATUS_OK, rsu_max_retry);

	case INTEL_SIP_SMC_RSU_COPY_MAX_RETRY:
		rsu_max_retry = x1;
		SMC_RET1(handle, INTEL_SIP_SMC_STATUS_OK);

	case INTEL_SIP_SMC_ECC_DBE:
		status = intel_ecc_dbe_notification(x1);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_SERVICE_COMPLETED:
		status = intel_smc_service_completed(x1, x2, x3, &rcv_id,
						&len_in_resp, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x1, len_in_resp);

	case INTEL_SIP_SMC_FIRMWARE_VERSION:
		status = intel_smc_fw_version(&retval);
		SMC_RET2(handle, status, retval);

	case INTEL_SIP_SMC_MBOX_SEND_CMD:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_mbox_send_cmd(x1, (uint32_t *)x2, x3, x4, x5, x6,
						&mbox_status, &len_in_resp);
		SMC_RET3(handle, status, mbox_status, len_in_resp);

	case INTEL_SIP_SMC_GET_USERCODE:
		status = intel_smc_get_usercode(&retval);
		SMC_RET2(handle, status, retval);

	case INTEL_SIP_SMC_FCS_CRYPTION:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);

		if (x1 == FCS_MODE_DECRYPT) {
			status = intel_fcs_decryption(x2, x3, x4, x5, &send_id);
		} else if (x1 == FCS_MODE_ENCRYPT) {
			status = intel_fcs_encryption(x2, x3, x4, x5, &send_id);
		} else {
			status = INTEL_SIP_SMC_STATUS_REJECTED;
		}

		SMC_RET3(handle, status, x4, x5);

	case INTEL_SIP_SMC_FCS_CRYPTION_EXT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);

		if (x3 == FCS_MODE_DECRYPT) {
			status = intel_fcs_decryption_ext(x1, x2, x4, x5, x6,
					(uint32_t *) &x7, &mbox_error);
		} else if (x3 == FCS_MODE_ENCRYPT) {
			status = intel_fcs_encryption_ext(x1, x2, x4, x5, x6,
					(uint32_t *) &x7, &mbox_error);
		} else {
			status = INTEL_SIP_SMC_STATUS_REJECTED;
		}

		SMC_RET4(handle, status, mbox_error, x6, x7);

	case INTEL_SIP_SMC_FCS_RANDOM_NUMBER:
		status = intel_fcs_random_number_gen(x1, &retval64,
							&mbox_error);
		SMC_RET4(handle, status, mbox_error, x1, retval64);

	case INTEL_SIP_SMC_FCS_RANDOM_NUMBER_EXT:
		status = intel_fcs_random_number_gen_ext(x1, x2, x3,
							&send_id);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_FCS_SEND_CERTIFICATE:
		status = intel_fcs_send_cert(x1, x2, &send_id);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_FCS_GET_PROVISION_DATA:
		status = intel_fcs_get_provision_data(&send_id);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_FCS_CNTR_SET_PREAUTH:
		status = intel_fcs_cntr_set_preauth(x1, x2, x3,
							&mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_HPS_SET_BRIDGES:
		status = intel_hps_set_bridges(x1, x2);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_HWMON_READTEMP:
		status = intel_hwmon_readtemp(x1, &retval);
		SMC_RET2(handle, status, retval);

	case INTEL_SIP_SMC_HWMON_READVOLT:
		status = intel_hwmon_readvolt(x1, &retval);
		SMC_RET2(handle, status, retval);

	case INTEL_SIP_SMC_FCS_PSGSIGMA_TEARDOWN:
		status = intel_fcs_sigma_teardown(x1, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_CHIP_ID:
		status = intel_fcs_chip_id(&retval, &retval2, &mbox_error);
		SMC_RET4(handle, status, mbox_error, retval, retval2);

	case INTEL_SIP_SMC_FCS_ATTESTATION_SUBKEY:
		status = intel_fcs_attestation_subkey(x1, x2, x3,
					(uint32_t *) &x4, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x3, x4);

	case INTEL_SIP_SMC_FCS_ATTESTATION_MEASUREMENTS:
		status = intel_fcs_get_measurement(x1, x2, x3,
					(uint32_t *) &x4, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x3, x4);

	case INTEL_SIP_SMC_FCS_GET_ATTESTATION_CERT:
		status = intel_fcs_get_attestation_cert(x1, x2,
					(uint32_t *) &x3, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x2, x3);

	case INTEL_SIP_SMC_FCS_CREATE_CERT_ON_RELOAD:
		status = intel_fcs_create_cert_on_reload(x1, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_OPEN_CS_SESSION:
		status = intel_fcs_open_crypto_service_session(&retval, &mbox_error);
		SMC_RET3(handle, status, mbox_error, retval);

	case INTEL_SIP_SMC_FCS_CLOSE_CS_SESSION:
		status = intel_fcs_close_crypto_service_session(x1, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_IMPORT_CS_KEY:
		status = intel_fcs_import_crypto_service_key(x1, x2, &send_id);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_FCS_EXPORT_CS_KEY:
		status = intel_fcs_export_crypto_service_key(x1, x2, x3,
					(uint32_t *) &x4, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x3, x4);

	case INTEL_SIP_SMC_FCS_REMOVE_CS_KEY:
		status = intel_fcs_remove_crypto_service_key(x1, x2,
					&mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_GET_CS_KEY_INFO:
		status = intel_fcs_get_crypto_service_key_info(x1, x2, x3,
					(uint32_t *) &x4, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x3, x4);

	case INTEL_SIP_SMC_FCS_GET_DIGEST_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_get_digest_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_GET_DIGEST_UPDATE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_get_digest_update_finalize(x1, x2, x3,
					x4, x5, (uint32_t *) &x6, false,
					&mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_GET_DIGEST_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_get_digest_update_finalize(x1, x2, x3,
					x4, x5, (uint32_t *) &x6, true,
					&mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_GET_DIGEST_SMMU_UPDATE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_get_digest_smmu_update_finalize(x1, x2, x3,
					x4, x5, (uint32_t *) &x6, false,
					&mbox_error, &send_id);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_GET_DIGEST_SMMU_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_get_digest_smmu_update_finalize(x1, x2, x3,
					x4, x5, (uint32_t *) &x6, true,
					&mbox_error, &send_id);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_MAC_VERIFY_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_mac_verify_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_MAC_VERIFY_UPDATE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
		status = intel_fcs_mac_verify_update_finalize(x1, x2, x3,
					x4, x5, (uint32_t *) &x6, x7,
					false, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_MAC_VERIFY_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
		status = intel_fcs_mac_verify_update_finalize(x1, x2, x3,
					x4, x5, (uint32_t *) &x6, x7,
					true, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_MAC_VERIFY_SMMU_UPDATE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
		status = intel_fcs_mac_verify_smmu_update_finalize(x1, x2, x3,
					x4, x5, (uint32_t *) &x6, x7,
					false, &mbox_error, &send_id);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_MAC_VERIFY_SMMU_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
		status = intel_fcs_mac_verify_smmu_update_finalize(x1, x2, x3,
					x4, x5, (uint32_t *) &x6, x7,
					true, &mbox_error, &send_id);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIGN_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_ecdsa_sha2_data_sign_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIGN_UPDATE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdsa_sha2_data_sign_update_finalize(x1, x2,
					x3, x4, x5, (uint32_t *) &x6, false,
					&mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIGN_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdsa_sha2_data_sign_update_finalize(x1, x2,
					x3, x4, x5, (uint32_t *) &x6, true,
					&mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIGN_SMMU_UPDATE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdsa_sha2_data_sign_smmu_update_finalize(x1,
					x2, x3, x4, x5, (uint32_t *) &x6, false,
					&mbox_error, &send_id);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIGN_SMMU_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdsa_sha2_data_sign_smmu_update_finalize(x1,
					x2, x3, x4, x5, (uint32_t *) &x6, true,
					&mbox_error, &send_id);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_HASH_SIGN_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_ecdsa_hash_sign_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_ECDSA_HASH_SIGN_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdsa_hash_sign_finalize(x1, x2, x3,
					 x4, x5, (uint32_t *) &x6, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_HASH_SIG_VERIFY_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_ecdsa_hash_sig_verify_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_ECDSA_HASH_SIG_VERIFY_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdsa_hash_sig_verify_finalize(x1, x2, x3,
					 x4, x5, (uint32_t *) &x6, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIG_VERIFY_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_ecdsa_sha2_data_sig_verify_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIG_VERIFY_UPDATE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
		status = intel_fcs_ecdsa_sha2_data_sig_verify_update_finalize(
					x1, x2, x3, x4, x5, (uint32_t *) &x6,
					x7, false, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIG_VERIFY_SMMU_UPDATE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
		status = intel_fcs_ecdsa_sha2_data_sig_verify_smmu_update_finalize(
					x1, x2, x3, x4, x5, (uint32_t *) &x6,
					x7, false, &mbox_error, &send_id);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIG_VERIFY_SMMU_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
		status = intel_fcs_ecdsa_sha2_data_sig_verify_smmu_update_finalize(
					x1, x2, x3, x4, x5, (uint32_t *) &x6,
					x7, true, &mbox_error, &send_id);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIG_VERIFY_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
		status = intel_fcs_ecdsa_sha2_data_sig_verify_update_finalize(
					x1, x2, x3, x4, x5, (uint32_t *) &x6,
					x7, true, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_GET_PUBKEY_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_ecdsa_get_pubkey_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_ECDSA_GET_PUBKEY_FINALIZE:
		status = intel_fcs_ecdsa_get_pubkey_finalize(x1, x2, x3,
					(uint32_t *) &x4, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x3, x4);

	case INTEL_SIP_SMC_FCS_ECDH_REQUEST_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_ecdh_request_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_ECDH_REQUEST_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdh_request_finalize(x1, x2, x3,
					 x4, x5, (uint32_t *) &x6, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_AES_CRYPT_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_aes_crypt_init(x1, x2, x3, x4, x5,
					&mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_AES_CRYPT_UPDATE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_aes_crypt_update_finalize(x1, x2, x3, x4,
					x5, x6, false, &send_id);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_FCS_AES_CRYPT_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_aes_crypt_update_finalize(x1, x2, x3, x4,
					x5, x6, true, &send_id);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_GET_ROM_PATCH_SHA384:
		status = intel_fcs_get_rom_patch_sha384(x1, &retval64,
							&mbox_error);
		SMC_RET4(handle, status, mbox_error, x1, retval64);

	case INTEL_SIP_SMC_SVC_VERSION:
		SMC_RET3(handle, INTEL_SIP_SMC_STATUS_OK,
					SIP_SVC_VERSION_MAJOR,
					SIP_SVC_VERSION_MINOR);

	default:
		return socfpga_sip_handler(smc_fid, x1, x2, x3, x4,
			cookie, handle, flags);
	}
}

uintptr_t sip_smc_handler(uint32_t smc_fid,
			 u_register_t x1,
			 u_register_t x2,
			 u_register_t x3,
			 u_register_t x4,
			 void *cookie,
			 void *handle,
			 u_register_t flags)
{
	uint32_t cmd = smc_fid & INTEL_SIP_SMC_CMD_MASK;

	if (cmd >= INTEL_SIP_SMC_CMD_V2_RANGE_BEGIN &&
	    cmd <= INTEL_SIP_SMC_CMD_V2_RANGE_END) {
		return sip_smc_handler_v2(smc_fid, x1, x2, x3, x4,
			cookie, handle, flags);
	} else {
		return sip_smc_handler_v1(smc_fid, x1, x2, x3, x4,
			cookie, handle, flags);
	}
}

DECLARE_RT_SVC(
	socfpga_sip_svc,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_FAST,
	NULL,
	sip_smc_handler
);

DECLARE_RT_SVC(
	socfpga_sip_svc_std,
	OEN_SIP_START,
	OEN_SIP_END,
	SMC_TYPE_YIELD,
	NULL,
	sip_smc_handler
);
