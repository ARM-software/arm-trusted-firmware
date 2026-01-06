
/*
 * Copyright (c) 2019-2025, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <tools_share/uuid.h>

#include "lib/utils/alignment_utils.h"
#include "socfpga_fcs.h"
#include "socfpga_mailbox.h"
#include "socfpga_plat_def.h"
#include "socfpga_private.h"
#include "socfpga_reset_manager.h"
#include "socfpga_sip_svc.h"
#include "socfpga_system_manager.h"

/* Total buffer the driver can hold */
#define FPGA_CONFIG_BUFFER_SIZE 4

static config_type request_type = NO_REQUEST;
static int current_block, current_buffer;
static int read_block, max_blocks;
static uint32_t send_id, rcv_id;
static uint32_t bytes_per_block, blocks_submitted;
static bool bridge_disable;
#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
static uint32_t g_remapper_bypass;
#endif

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

static uint32_t intel_mailbox_fpga_config_isdone(uint32_t *err_states)
{
	uint32_t ret;

	if (err_states == NULL)
		return INTEL_SIP_SMC_STATUS_REJECTED;

	switch (request_type) {
	case RECONFIGURATION:
		ret = intel_mailbox_get_config_status(MBOX_RECONFIG_STATUS,
							true, err_states);
		break;
	case BITSTREAM_AUTH:
		ret = intel_mailbox_get_config_status(MBOX_RECONFIG_STATUS,
							false, err_states);
		break;
	default:
		ret = intel_mailbox_get_config_status(MBOX_CONFIG_STATUS,
							false, err_states);
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

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	/*
	 * To trigger isolation
	 * FPGA configuration complete signal should be de-asserted
	 */
	INFO("SOCFPGA: Request SDM to trigger isolation\n");
	status = mailbox_send_fpga_config_comp();

	if (status < 0) {
		INFO("SOCFPGA: Isolation for FPGA configuration complete is not executed\n");
	}
#endif

	request_type = RECONFIGURATION;

	if (!CONFIG_TEST_FLAG(flag, PARTIAL_CONFIG)) {
		bridge_disable = true;
	}

	if (CONFIG_TEST_FLAG(flag, AUTHENTICATION)) {
		size = 1;
		bridge_disable = false;
		request_type = BITSTREAM_AUTH;
	}

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	intel_smmu_hps_remapper_init(0U);
#endif

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
	uint128_t dram_max_sz = (uint128_t)DRAM_BASE + (uint128_t)DRAM_SIZE;
	uint128_t dram_region_end = (uint128_t)addr + (uint128_t)size;

	if (!addr && !size) {
		return true;
	}
	if (size > (UINT64_MAX - addr)) {
		return false;
	}
	if (addr < BL31_LIMIT) {
		return false;
	}
	if (dram_region_end > dram_max_sz) {
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

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	intel_smmu_hps_remapper_init(&mem);
#endif

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

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	if (is_agilex5_A5F4() == true) {
		switch (reg_addr) {
		/* TSN stream control registers — only accessible on Agilex5 B0 */
		case SOCFPGA_SYSMGR(TSN_TBU_STREAM_CTRL_REG_3_TSN0):
		case SOCFPGA_SYSMGR(TSN_TBU_STREAM_CTRL_REG_3_TSN1):
		case SOCFPGA_SYSMGR(TSN_TBU_STREAM_CTRL_REG_3_TSN2):
			return 0;

		default:
			break;
		}
	}
#endif

#if PLATFORM_MODEL != PLAT_SOCFPGA_AGILEX5
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
#else
	switch (reg_addr) {

	case(0xF8011104):	/* ECCCTRL2 */
	case(0xFFD12028):	/* SDMMCGRP_CTRL */
	case(0xFFD120C4):	/* NOC_IDLEREQ_SET */
	case(0xFFD120C8):	/* NOC_IDLEREQ_CLR */
	case(0xFFD120D0):	/* NOC_IDLEACK */


	case(SOCFPGA_MEMCTRL(ECCCTRL1)):	/* ECCCTRL1 */
	case(SOCFPGA_MEMCTRL(ERRINTEN)):	/* ERRINTEN */
	case(SOCFPGA_MEMCTRL(ERRINTENS)):	/* ERRINTENS */
	case(SOCFPGA_MEMCTRL(ERRINTENR)):	/* ERRINTENR */
	case(SOCFPGA_MEMCTRL(INTMODE)):	/* INTMODE */
	case(SOCFPGA_MEMCTRL(INTSTAT)):	/* INTSTAT */
	case(SOCFPGA_MEMCTRL(DIAGINTTEST)):	/* DIAGINTTEST */
	case(SOCFPGA_MEMCTRL(DERRADDRA)):	/* DERRADDRA */

	case(SOCFPGA_ECC_QSPI(INITSTAT)):	/* ECC_QSPI_INITSTAT */
	case(SOCFPGA_SYSMGR(EMAC_0)):	/* EMAC0 */
	case(SOCFPGA_SYSMGR(EMAC_1)):	/* EMAC1 */
	case(SOCFPGA_SYSMGR(EMAC_2)):	/* EMAC2 */
	case(SOCFPGA_SYSMGR(ECC_INTMASK_VALUE)):	/* ECC_INT_MASK_VALUE */
	case(SOCFPGA_SYSMGR(ECC_INTMASK_SET)):	/* ECC_INT_MASK_SET */
	case(SOCFPGA_SYSMGR(ECC_INTMASK_CLR)):	/* ECC_INT_MASK_CLEAR */
	case(SOCFPGA_SYSMGR(ECC_INTMASK_SERR)):	/* ECC_INTSTATUS_SERR */
	case(SOCFPGA_SYSMGR(ECC_INTMASK_DERR)):	/* ECC_INTSTATUS_DERR */
	case(SOCFPGA_SYSMGR(NOC_TIMEOUT)):	/* NOC_TIMEOUT */
	case(SOCFPGA_SYSMGR(NOC_IDLESTATUS)):	/* NOC_IDLESTATUS */
	case(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_0)):	/* BOOT_SCRATCH_COLD0 */
	case(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_1)):	/* BOOT_SCRATCH_COLD1 */
	case(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_8)):	/* BOOT_SCRATCH_COLD8 */
	case(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_9)):	/* BOOT_SCRATCH_COLD9 */
#endif
	case(SOCFPGA_ECC_QSPI(CTRL)):			/* ECC_QSPI_CTRL */
	case(SOCFPGA_ECC_QSPI(ERRINTEN)):		/* ECC_QSPI_ERRINTEN */
	case(SOCFPGA_ECC_QSPI(ERRINTENS)):		/* ECC_QSPI_ERRINTENS */
	case(SOCFPGA_ECC_QSPI(ERRINTENR)):		/* ECC_QSPI_ERRINTENR */
	case(SOCFPGA_ECC_QSPI(INTMODE)):		/* ECC_QSPI_INTMODE */
	case(SOCFPGA_ECC_QSPI(ECC_ACCCTRL)):	/* ECC_QSPI_ECC_ACCCTRL */
	case(SOCFPGA_ECC_QSPI(ECC_STARTACC)):	/* ECC_QSPI_ECC_STARTACC */
	case(SOCFPGA_ECC_QSPI(ECC_WDCTRL)):		/* ECC_QSPI_ECC_WDCTRL */
	case(SOCFPGA_ECC_QSPI(INTSTAT)):		/* ECC_QSPI_INTSTAT */
	case(SOCFPGA_ECC_QSPI(INTTEST)):		/* ECC_QSPI_INTMODE */
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

	switch (reg_addr) {
	case(SOCFPGA_ECC_QSPI(INTSTAT)):		/* ECC_QSPI_INTSTAT */
	case(SOCFPGA_ECC_QSPI(INTTEST)):		/* ECC_QSPI_INTMODE */
		mmio_write_16(reg_addr, val);
		break;
	default:
		mmio_write_32(reg_addr, val);
		break;
	}

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

static uint32_t intel_rsu_get_device_info(uint32_t *respbuf,
					  unsigned int respbuf_sz)
{
	if (mailbox_rsu_get_device_info((uint32_t *)respbuf, respbuf_sz) < 0) {
		return INTEL_SIP_SMC_RSU_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

uint32_t intel_rsu_update(uint64_t update_address)
{
	if (update_address > SIZE_MAX) {
		return INTEL_SIP_SMC_STATUS_REJECTED;
	}

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

/* SDM SEU Error services */
static uint32_t intel_sdm_seu_err_read(uint32_t *respbuf, unsigned int respbuf_sz)
{
	if (mailbox_seu_err_status(respbuf, respbuf_sz) < 0) {
		return INTEL_SIP_SMC_SEU_ERR_READ_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

/* SDM SAFE SEU Error inject services */
static uint32_t intel_sdm_safe_inject_seu_err(uint32_t *command, uint32_t len)
{
	if (mailbox_safe_inject_seu_err(command, len) < 0) {
		return INTEL_SIP_SMC_SEU_ERR_READ_ERROR;
	}

	return INTEL_SIP_SMC_STATUS_OK;
}

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
/* SMMU HPS Remapper */
void intel_smmu_hps_remapper_init(uint64_t *mem)
{
	/* Read out Bit 1 value */
	uint32_t remap = (mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_POR_1)) & 0x02);

	if ((remap == 0x00) && (g_remapper_bypass == 0x00)) {
		/* Update DRAM Base address for SDM SMMU */
		mmio_write_32(SOCFPGA_SYSMGR(SDM_BE_ARADDR_REMAP), DRAM_BASE);
		mmio_write_32(SOCFPGA_SYSMGR(SDM_BE_AWADDR_REMAP), DRAM_BASE);
		*mem = *mem - DRAM_BASE;
	} else {
		*mem = *mem - DRAM_BASE;
	}
}

int intel_smmu_hps_remapper_config(uint32_t remapper_bypass)
{
	/* Read out the JTAG-ID from boot scratch register */
	if (is_agilex5_A5C0() || is_agilex5_A5C4()) {
		if (remapper_bypass == 0x01) {
			g_remapper_bypass = remapper_bypass;
			mmio_write_32(SOCFPGA_SYSMGR(SDM_BE_ARADDR_REMAP), 0);
			mmio_write_32(SOCFPGA_SYSMGR(SDM_BE_AWADDR_REMAP), 0);
		}
	}
	return INTEL_SIP_SMC_STATUS_OK;
}

static void intel_inject_io96b_ecc_err(const uint32_t *syndrome, const uint32_t command)
{
	volatile uint64_t atf_ddr_buffer;
	volatile uint64_t val;

	mmio_write_32(IOSSM_CMD_PARAM, *syndrome);
	mmio_write_32(IOSSM_CMD_TRIG_OP, command);
	udelay(IOSSM_ECC_ERR_INJ_DELAY_USECS);
	atf_ddr_buffer = 0xCAFEBABEFEEDFACE;	/* Write data */
	memcpy_s((void *)&val, sizeof(val),
		 (void *)&atf_ddr_buffer, sizeof(atf_ddr_buffer));

	/* Clear response_ready BIT0 of status_register before sending next command. */
	mmio_clrbits_32(IOSSM_CMD_RESP_STATUS, IOSSM_CMD_STATUS_RESP_READY);
}
#endif

#if SIP_SVC_V3
uint8_t sip_smc_cmd_cb_ret2(void *resp_desc, void *cmd_desc, uint64_t *ret_args)
{
	uint8_t ret_args_len = 0U;
	sdm_response_t *resp = (sdm_response_t *)resp_desc;
	sdm_command_t *cmd = (sdm_command_t *)cmd_desc;

	(void)cmd;
	/* Returns 3 SMC arguments for SMC_RET3 */
	ret_args[ret_args_len++] = INTEL_SIP_SMC_STATUS_OK;
	ret_args[ret_args_len++] = resp->err_code;

	return ret_args_len;
}

uint8_t sip_smc_cmd_cb_ret3(void *resp_desc, void *cmd_desc, uint64_t *ret_args)
{
	uint8_t ret_args_len = 0U;
	sdm_response_t *resp = (sdm_response_t *)resp_desc;
	sdm_command_t *cmd = (sdm_command_t *)cmd_desc;

	(void)cmd;
	/* Returns 3 SMC arguments for SMC_RET3 */
	ret_args[ret_args_len++] = INTEL_SIP_SMC_STATUS_OK;
	ret_args[ret_args_len++] = resp->err_code;
	ret_args[ret_args_len++] = resp->resp_data[0];

	return ret_args_len;
}

uint8_t sip_smc_ret_nbytes_cb(void *resp_desc, void *cmd_desc, uint64_t *ret_args)
{
	uint8_t ret_args_len = 0U;
	sdm_response_t *resp = (sdm_response_t *)resp_desc;
	sdm_command_t *cmd = (sdm_command_t *)cmd_desc;

	INFO("MBOX: %s: mailbox_err 0%x, nbytes_ret %d\n",
		__func__, resp->err_code, resp->rcvd_resp_len * MBOX_WORD_BYTE);

	ret_args[ret_args_len++] = INTEL_SIP_SMC_STATUS_OK;
	ret_args[ret_args_len++] = resp->err_code;
	ret_args[ret_args_len++] = resp->rcvd_resp_len * MBOX_WORD_BYTE;

	/* Flush the response data buffer. */
	flush_dcache_range((uintptr_t)cmd->cb_args, resp->rcvd_resp_len * MBOX_WORD_BYTE);

	return ret_args_len;
}

uint8_t sip_smc_get_chipid_cb(void *resp_desc, void *cmd_desc, uint64_t *ret_args)
{
	uint8_t ret_args_len = 0U;
	sdm_response_t *resp = (sdm_response_t *)resp_desc;
	sdm_command_t *cmd = (sdm_command_t *)cmd_desc;

	(void)cmd;
	INFO("MBOX: %s: mailbox_err 0%x, data[0] 0x%x, data[1] 0x%x\n",
		__func__, resp->err_code, resp->resp_data[0], resp->resp_data[1]);

	ret_args[ret_args_len++] = INTEL_SIP_SMC_STATUS_OK;
	ret_args[ret_args_len++] = resp->err_code;
	ret_args[ret_args_len++] = resp->resp_data[0];
	ret_args[ret_args_len++] = resp->resp_data[1];

	return ret_args_len;
}

uint8_t sip_smc_cmd_cb_rsu_status(void *resp_desc, void *cmd_desc, uint64_t *ret_args)
{
	uint8_t ret_args_len = 0U;
	uint32_t retry_counter = ~0U;
	uint32_t failure_source = 0U;
	sdm_response_t *resp = (sdm_response_t *)resp_desc;
	sdm_command_t *cmd = (sdm_command_t *)cmd_desc;

	(void)cmd;
	/* Get the failure source and current image retry counter value from the response. */
	failure_source = resp->resp_data[5] & RSU_VERSION_ACMF_MASK;
	retry_counter = resp->resp_data[8];

	if ((retry_counter != ~0U) && (failure_source == 0U))
		resp->resp_data[5] |= RSU_VERSION_ACMF;

	ret_args[ret_args_len++] = INTEL_SIP_SMC_STATUS_OK;
	ret_args[ret_args_len++] = resp->err_code;
	/* Current CMF */
	ret_args[ret_args_len++] = GET_ADDR64(resp->resp_data[1], resp->resp_data[0]);
	/* Last Failing CMF Address */
	ret_args[ret_args_len++] = GET_ADDR64(resp->resp_data[3], resp->resp_data[2]);
	/* Config State */
	ret_args[ret_args_len++] = resp->resp_data[4];
	/* Version */
	ret_args[ret_args_len++] = resp->resp_data[5];
	/* Failure Source */
	ret_args[ret_args_len++] = ((GENMASK(32, 17) & resp->resp_data[5]) >> 16);
	/* Error location */
	ret_args[ret_args_len++] = resp->resp_data[6];
	/* Error details */
	ret_args[ret_args_len++] = resp->resp_data[7];
	/* Current image retry counter */
	ret_args[ret_args_len++] = resp->resp_data[8];

	return ret_args_len;
}

uint8_t sip_smc_cmd_cb_rsu_spt(void *resp_desc, void *cmd_desc, uint64_t *ret_args)
{
	uint8_t ret_args_len = 0U;
	sdm_response_t *resp = (sdm_response_t *)resp_desc;
	sdm_command_t *cmd = (sdm_command_t *)cmd_desc;

	(void)cmd;

	ret_args[ret_args_len++] = INTEL_SIP_SMC_STATUS_OK;
	ret_args[ret_args_len++] = resp->err_code;
	/* Sub Partition Table (SPT) 0 address */
	ret_args[ret_args_len++] = GET_ADDR64(resp->resp_data[0], resp->resp_data[1]);
	/* Sub Partition Table (SPT) 1 address */
	ret_args[ret_args_len++] = GET_ADDR64(resp->resp_data[2], resp->resp_data[3]);

	return ret_args_len;
}

static uintptr_t smc_ret(void *handle, uint64_t *ret_args, uint32_t ret_args_len)
{

	switch (ret_args_len) {
	case SMC_RET_ARGS_ONE:
		VERBOSE("SVC V3: %s: x0 0x%lx\n", __func__, ret_args[0]);
		SMC_RET1(handle, ret_args[0]);
		break;

	case SMC_RET_ARGS_TWO:
		VERBOSE("SVC V3: %s: x0 0x%lx, x1 0x%lx\n", __func__, ret_args[0], ret_args[1]);
		SMC_RET2(handle, ret_args[0], ret_args[1]);
		break;

	case SMC_RET_ARGS_THREE:
		VERBOSE("SVC V3: %s: x0 0x%lx, x1 0x%lx, x2 0x%lx\n",
			__func__, ret_args[0],	ret_args[1], ret_args[2]);
		SMC_RET3(handle, ret_args[0], ret_args[1], ret_args[2]);
		break;

	case SMC_RET_ARGS_FOUR:
		VERBOSE("SVC V3: %s: x0 0x%lx, x1 0x%lx, x2 0x%lx, x3 0x%lx\n",
			__func__, ret_args[0], ret_args[1], ret_args[2], ret_args[3]);
		SMC_RET4(handle, ret_args[0], ret_args[1], ret_args[2], ret_args[3]);
		break;

	case SMC_RET_ARGS_FIVE:
		VERBOSE("SVC V3: %s: x0 0x%lx, x1 0x%lx, x2 0x%lx, x3 0x%lx, x4 0x%lx\n",
			__func__, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4]);
		SMC_RET5(handle, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4]);
		break;

	case SMC_RET_ARGS_SIX:
		VERBOSE("SVC V3: %s: x0 0x%lx, x1 0x%lx x2 0x%lx x3 0x%lx, x4 0x%lx x5 0x%lx\n",
			__func__, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4],
			ret_args[5]);
		SMC_RET6(handle, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4],
			 ret_args[5]);
		break;

	case SMC_RET_ARGS_SEVEN:
		VERBOSE("SVC V3: %s: x0 0x%lx, x1 0x%lx x2 0x%lx, x3 0x%lx, x4 0x%lx, x5 0x%lx\t"
			"x6 0x%lx\n",
			__func__, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4],
			ret_args[5], ret_args[6]);
		SMC_RET7(handle, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4],
			 ret_args[5], ret_args[6]);
		break;

	case SMC_RET_ARGS_EIGHT:
		VERBOSE("SVC V3: %s: x0 0x%lx, x1 0x%lx x2 0x%lx, x3 0x%lx, x4 0x%lx x5 0x%lx\t"
			"x6 0x%lx, x7 0x%lx\n",
			__func__, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4],
			ret_args[5], ret_args[6], ret_args[7]);
		SMC_RET8(handle, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4],
			 ret_args[5], ret_args[6], ret_args[7]);
		break;

	case SMC_RET_ARGS_NINE:
		VERBOSE("SVC V3: %s: x0 0x%lx, x1 0x%lx x2 0x%lx, x3 0x%lx, x4 0x%lx, x5 0x%lx\t"
			"x6 0x%lx, x7 0x%lx, x8 0x%lx\n",
			__func__, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4],
			ret_args[5], ret_args[6], ret_args[7], ret_args[8]);
		SMC_RET18(handle, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4],
			 ret_args[5], ret_args[6], ret_args[7], ret_args[8],
			 0, 0, 0, 0, 0, 0, 0, 0, 0);
		break;

	case SMC_RET_ARGS_TEN:
		VERBOSE("SVC V3: %s: x0 0x%lx, x1 0x%lx, x2 0x%lx, x3 0x%lx, x4 0x%lx x5 0x%lx\t"
			"x6 0x%lx, x7 0x%lx x8 0x%lx, x9 0x%lx, x10 0x%lx\n",
			__func__, ret_args[0], ret_args[1], ret_args[2], ret_args[3],
			ret_args[4], ret_args[5], ret_args[6], ret_args[7], ret_args[8],
			ret_args[9], ret_args[10]);
		SMC_RET18(handle, ret_args[0], ret_args[1], ret_args[2], ret_args[3], ret_args[4],
			  ret_args[5], ret_args[6], ret_args[7], ret_args[8], ret_args[9],
			  0, 0, 0, 0, 0, 0, 0, 0);
		break;

	default:
		VERBOSE("SVC V3: %s ret_args_len is wrong, please check %d\n ",
			__func__, ret_args_len);
		SMC_RET1(handle, INTEL_SIP_SMC_STATUS_ERROR);
		break;
	}
}

static inline bool is_gen_mbox_cmd_allowed(uint32_t cmd)
{
	/* Check if the command is allowed to be executed in generic mbox format */
	bool is_cmd_allowed = false;

	switch (cmd) {
	case MBOX_FCS_OPEN_CS_SESSION:
	case MBOX_FCS_CLOSE_CS_SESSION:
	case MBOX_FCS_IMPORT_CS_KEY:
	case MBOX_FCS_EXPORT_CS_KEY:
	case MBOX_FCS_REMOVE_CS_KEY:
	case MBOX_FCS_GET_CS_KEY_INFO:
	case MBOX_FCS_CREATE_CS_KEY:
	case MBOX_FCS_GET_DIGEST_REQ:
	case MBOX_FCS_MAC_VERIFY_REQ:
	case MBOX_FCS_ECDSA_HASH_SIGN_REQ:
	case MBOX_FCS_GET_PROVISION:
	case MBOX_FCS_CNTR_SET_PREAUTH:
	case MBOX_FCS_ENCRYPT_REQ:
	case MBOX_FCS_DECRYPT_REQ:
	case MBOX_FCS_RANDOM_GEN:
	case MBOX_FCS_AES_CRYPT_REQ:
	case MBOX_FCS_ECDSA_SHA2_DATA_SIGN_REQ:
	case MBOX_FCS_ECDSA_HASH_SIG_VERIFY:
	case MBOX_FCS_ECDSA_SHA2_DATA_SIGN_VERIFY:
	case MBOX_FCS_ECDSA_GET_PUBKEY:
	case MBOX_FCS_ECDH_REQUEST:
	case MBOX_FCS_HKDF_REQUEST:
		/* These mailbox commands are not supported in the generic mailbox format. */
		break;

	default:
		is_cmd_allowed = true;
		break;
	} /* switch */

	return is_cmd_allowed;
}

/*
 * This function is responsible for handling all SiP SVC V3 calls from the
 * non-secure world.
 */
static uintptr_t sip_smc_handler_v3(uint32_t smc_fid,
				    u_register_t x1,
				    u_register_t x2,
				    u_register_t x3,
				    u_register_t x4,
				    void *cookie,
				    void *handle,
				    u_register_t flags)
{
	int status = 0;
	uint32_t mbox_error = 0U;
	u_register_t x5, x6, x7, x8, x9, x10, x11;

	/* Get all the SMC call arguments */
	x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
	x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
	x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
	x8 = SMC_GET_GP(handle, CTX_GPREG_X8);
	x9 = SMC_GET_GP(handle, CTX_GPREG_X9);
	x10 = SMC_GET_GP(handle, CTX_GPREG_X10);
	x11 = SMC_GET_GP(handle, CTX_GPREG_X11);

	INFO("MBOX: SVC_V3: x0 0x%x, x1 0x%lx, x2 0x%lx, x3 0x%lx, x4 0x%lx, x5 0x%lx\n",
		smc_fid, x1, x2, x3, x4, x5);
	INFO("MBOX: SVC_V3: x6 0x%lx, x7 0x%lx, x8 0x%lx, x9 0x%lx, x10 0x%lx x11 0x%lx\n",
		x6, x7, x8, x9, x10, x11);

	switch (smc_fid) {
	case ALTERA_SIP_SMC_ASYNC_RESP_POLL:
	{
		uint64_t ret_args[16] = {0};
		uint32_t ret_args_len = 0;

		status = mailbox_response_poll_v3(GET_CLIENT_ID(x1),
						  GET_JOB_ID(x1),
						  ret_args,
						  &ret_args_len);
		/* Always reserve [0] index for command status. */
		ret_args[0] = status;

		/* Return SMC call based on the number of return arguments */
		return smc_ret(handle, ret_args, ret_args_len);
	}

	case ALTERA_SIP_SMC_ASYNC_RESP_POLL_ON_INTR:
	{
		/* TBD: Here now we don't need these CID and JID?? */
		uint8_t client_id = 0U;
		uint8_t job_id = 0U;
		uint64_t trans_id_bitmap[4] = {0U};

		status = mailbox_response_poll_on_intr_v3(&client_id,
							  &job_id,
							  trans_id_bitmap);

		SMC_RET5(handle, status, trans_id_bitmap[0], trans_id_bitmap[1],
			 trans_id_bitmap[2], trans_id_bitmap[3]);
		break;
	}

	case ALTERA_SIP_SMC_ASYNC_GET_DEVICE_IDENTITY:
	{
		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_GET_DEVICEID,
						   NULL,
						   0U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_ret_nbytes_cb,
						   (uint32_t *)x2,
						   2);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_GET_IDCODE:
	{
		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_GET_IDCODE,
						   NULL,
						   0U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret3,
						   NULL,
						   0);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_QSPI_OPEN:
	{
		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_QSPI_OPEN,
						   NULL,
						   0U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret2,
						   NULL,
						   0U);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_QSPI_CLOSE:
	{
		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_QSPI_CLOSE,
						   NULL,
						   0U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret2,
						   NULL,
						   0U);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_QSPI_SET_CS:
	{
		uint32_t cmd_data = 0U;
		uint32_t chip_sel = (uint32_t)x2;
		uint32_t comb_addr_mode = (uint32_t)x3;
		uint32_t ext_dec_mode = (uint32_t)x4;

		cmd_data = (chip_sel << MBOX_QSPI_SET_CS_OFFSET) |
			   (comb_addr_mode << MBOX_QSPI_SET_CS_CA_OFFSET) |
			   (ext_dec_mode << MBOX_QSPI_SET_CS_MODE_OFFSET);

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_QSPI_SET_CS,
						   &cmd_data,
						   1U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret2,
						   NULL,
						   0U);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_QSPI_ERASE:
	{
		uint32_t qspi_addr = (uint32_t)x2;
		uint32_t qspi_nwords = (uint32_t)x3;

		/* QSPI address offset to start erase, must be 4K aligned */
		if (MBOX_IS_4K_ALIGNED(qspi_addr)) {
			ERROR("MBOX: 0x%x: QSPI address not 4K aligned\n",
				smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		/* Number of words to erase, multiples of 0x400 or 4K */
		if (qspi_nwords % MBOX_QSPI_ERASE_SIZE_GRAN) {
			ERROR("MBOX: 0x%x: Given words not in multiples of 4K\n",
				smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		uint32_t cmd_data[2] = {qspi_addr, qspi_nwords};

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_QSPI_ERASE,
						   cmd_data,
						   sizeof(cmd_data) / MBOX_WORD_BYTE,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret2,
						   NULL,
						   0U);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_QSPI_WRITE:
	{
		uint32_t *qspi_payload = (uint32_t *)x2;
		uint32_t qspi_total_nwords = (((uint32_t)x3) / MBOX_WORD_BYTE);
		uint32_t qspi_addr = qspi_payload[0];
		uint32_t qspi_nwords = qspi_payload[1];

		if (!MBOX_IS_WORD_ALIGNED(qspi_addr)) {
			ERROR("MBOX: 0x%x: Given address is not WORD aligned\n",
				smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		if (qspi_nwords > MBOX_QSPI_RW_MAX_WORDS) {
			ERROR("MBOX: 0x%x: Number of words exceeds max limit\n",
				smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_QSPI_WRITE,
						   qspi_payload,
						   qspi_total_nwords,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret2,
						   NULL,
						   0U);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_QSPI_READ:
	{
		uint32_t qspi_addr = (uint32_t)x2;
		uint32_t qspi_nwords = (((uint32_t)x4) / MBOX_WORD_BYTE);

		if (qspi_nwords > MBOX_QSPI_RW_MAX_WORDS) {
			ERROR("MBOX: 0x%x: Number of words exceeds max limit\n",
				smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		uint32_t cmd_data[2] = {qspi_addr, qspi_nwords};

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_QSPI_READ,
						   cmd_data,
						   sizeof(cmd_data) / MBOX_WORD_BYTE,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_ret_nbytes_cb,
						   (uint32_t *)x3,
						   2);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_QSPI_GET_DEV_INFO:
	{
		uint32_t *dst_addr = (uint32_t *)x2;

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_QSPI_GET_DEV_INFO,
						   NULL,
						   0U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_ret_nbytes_cb,
						   (uint32_t *)dst_addr,
						   2);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_HWMON_READVOLT:
	case ALTERA_SIP_SMC_ASYNC_HWMON_READTEMP:
	{
		uint32_t channel = (uint32_t)x2;
		uint32_t mbox_cmd = ((smc_fid == ALTERA_SIP_SMC_ASYNC_HWMON_READVOLT) ?
					MBOX_HWMON_READVOLT : MBOX_HWMON_READTEMP);

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   mbox_cmd,
						   &channel,
						   1U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret3,
						   NULL,
						   0);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_RSU_GET_SPT:
	{
		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_GET_SUBPARTITION_TABLE,
						   NULL,
						   0,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_rsu_spt,
						   NULL,
						   0);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_RSU_GET_STATUS:
	{
		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_RSU_STATUS,
						   NULL,
						   0,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_rsu_status,
						   NULL,
						   0);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_RSU_NOTIFY:
	{
		uint32_t notify_code = (uint32_t)x2;

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_HPS_STAGE_NOTIFY,
						   &notify_code,
						   1U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret2,
						   NULL,
						   0);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_GEN_MBOX_CMD:
	{
		/* Collect all the args passed in, and send the mailbox command. */
		uint32_t mbox_cmd = (uint32_t)x2;
		uint32_t *cmd_payload_addr = NULL;
		uint32_t cmd_payload_len = (uint32_t)x4 / MBOX_WORD_BYTE;
		uint32_t *resp_payload_addr = NULL;
		uint32_t resp_payload_len = (uint32_t)x6 / MBOX_WORD_BYTE;

		/* Filter the required commands here. */
		if (!is_gen_mbox_cmd_allowed(mbox_cmd)) {
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		if ((cmd_payload_len > MBOX_GEN_CMD_MAX_WORDS) ||
		    (resp_payload_len > MBOX_GEN_CMD_MAX_WORDS)) {
			ERROR("MBOX: 0x%x: Command/Response payload length exceeds max limit\n",
				smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		/* Make sure we have valid command payload length and buffer */
		if (cmd_payload_len != 0U) {
			cmd_payload_addr = (uint32_t *)x3;
			if (cmd_payload_addr == NULL) {
				ERROR("MBOX: 0x%x: Command payload address is NULL\n",
					smc_fid);
				status = INTEL_SIP_SMC_STATUS_REJECTED;
				SMC_RET1(handle, status);
			}
		}

		/* Make sure we have valid response payload length and buffer */
		if (resp_payload_len != 0U) {
			resp_payload_addr = (uint32_t *)x5;
			if (resp_payload_addr == NULL) {
				ERROR("MBOX: 0x%x: Response payload address is NULL\n",
					smc_fid);
				status = INTEL_SIP_SMC_STATUS_REJECTED;
				SMC_RET1(handle, status);
			}
		}

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   mbox_cmd,
						   (uint32_t *)cmd_payload_addr,
						   cmd_payload_len,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_ret_nbytes_cb,
						   (uint32_t *)resp_payload_addr,
						   resp_payload_len);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_RANDOM_NUMBER_EXT:
	{
		uint32_t session_id = (uint32_t)x2;
		uint32_t context_id = (uint32_t)x3;
		uint64_t ret_random_addr = (uint64_t)x4;
		uint32_t random_len = (uint32_t)SMC_GET_GP(handle, CTX_GPREG_X5);
		uint32_t crypto_header = 0U;

		if ((random_len > (FCS_RANDOM_EXT_MAX_WORD_SIZE * MBOX_WORD_BYTE)) ||
		    (random_len == 0U) ||
		    (!is_size_4_bytes_aligned(random_len))) {
			ERROR("MBOX: 0x%x is rejected\n", smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		crypto_header = ((FCS_CS_FIELD_FLAG_INIT | FCS_CS_FIELD_FLAG_FINALIZE) <<
				  FCS_CS_FIELD_FLAG_OFFSET);
		fcs_rng_payload payload = {session_id, context_id,
					   crypto_header, random_len};

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_FCS_RANDOM_GEN,
						   (uint32_t *)&payload,
						   sizeof(payload) / MBOX_WORD_BYTE,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_ret_nbytes_cb,
						   (uint32_t *)ret_random_addr,
						   2);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_GET_PROVISION_DATA:
	{
		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_FCS_GET_PROVISION,
						   NULL,
						   0U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_ret_nbytes_cb,
						   (uint32_t *)x2,
						   2);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_CNTR_SET_PREAUTH:
	{
		status = intel_fcs_cntr_set_preauth(smc_fid, x1, x2, x3,
					x4, &mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_CHIP_ID:
	{
		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_GET_CHIPID,
						   NULL,
						   0U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_get_chipid_cb,
						   NULL,
						   0);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_GET_ATTESTATION_CERT:
	{
		status = intel_fcs_get_attestation_cert(smc_fid, x1, x2, x3,
					(uint32_t *) &x4, &mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_CREATE_CERT_ON_RELOAD:
	{
		status = intel_fcs_create_cert_on_reload(smc_fid, x1,
					x2, &mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_CRYPTION_EXT:
	{
		if (x4 == FCS_MODE_ENCRYPT) {
			status = intel_fcs_encryption_ext(smc_fid, x1, x2, x3,
					x5, x6, x7, (uint32_t *) &x8,
					&mbox_error, x10, x11);
		} else if (x4 == FCS_MODE_DECRYPT) {
			status = intel_fcs_decryption_ext(smc_fid, x1, x2, x3,
					x5, x6, x7, (uint32_t *) &x8,
					&mbox_error, x9, x10, x11);
		} else {
			ERROR("MBOX: 0x%x: Wrong crypto mode\n", smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
		}
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_SEND_CERTIFICATE:
	{
		status = intel_fcs_send_cert(smc_fid, x1, x2, x3, &mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_OPEN_CS_SESSION:
	{
		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_FCS_OPEN_CS_SESSION,
						   NULL,
						   0U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret3,
						   NULL,
						   0);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_CLOSE_CS_SESSION:
	{
		uint32_t session_id = (uint32_t)x2;

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_FCS_CLOSE_CS_SESSION,
						   &session_id,
						   1U,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret2,
						   NULL,
						   0);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_IMPORT_CS_KEY:
	{
		uint64_t key_addr = x2;
		uint32_t key_len_words = (uint32_t)x3 / MBOX_WORD_BYTE;

		if ((key_len_words > FCS_CS_KEY_OBJ_MAX_WORD_SIZE) ||
		    (!is_address_in_ddr_range(key_addr, key_len_words * 4))) {
			ERROR("MBOX: 0x%x: Addr not in DDR range or key len exceeds\n",
				smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_FCS_IMPORT_CS_KEY,
						   (uint32_t *)key_addr,
						   key_len_words,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret3,
						   NULL,
						   0);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_CREATE_CS_KEY:
	{
		uint64_t key_addr = x2;
		uint32_t key_len_words = (uint32_t)x3 / MBOX_WORD_BYTE;

		if (!is_address_in_ddr_range(key_addr, key_len_words * 4)) {
			ERROR("MBOX: 0x%x: Addr not in DDR range\n", smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_FCS_CREATE_CS_KEY,
						   (uint32_t *)key_addr,
						   key_len_words,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret3,
						   NULL,
						   0);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_EXPORT_CS_KEY:
	{
		uint32_t session_id = (uint32_t)x2;
		uint32_t key_uid = (uint32_t)x3;
		uint64_t ret_key_addr = (uint64_t)x4;
		uint32_t key_len = (uint32_t)SMC_GET_GP(handle, CTX_GPREG_X5);

		if (!is_address_in_ddr_range(ret_key_addr, key_len)) {
			ERROR("MBOX: 0x%x: Addr not in DDR range\n", smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		fcs_cs_key_payload payload = {session_id, RESERVED_AS_ZERO,
					      RESERVED_AS_ZERO, key_uid};

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_FCS_EXPORT_CS_KEY,
						   (uint32_t *)&payload,
						   sizeof(payload) / MBOX_WORD_BYTE,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_ret_nbytes_cb,
						   (uint32_t *)ret_key_addr,
						   2);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_REMOVE_CS_KEY:
	{
		uint32_t session_id = (uint32_t)x2;
		uint32_t key_uid = (uint32_t)x3;

		fcs_cs_key_payload payload = {session_id, RESERVED_AS_ZERO,
					      RESERVED_AS_ZERO, key_uid};

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_FCS_REMOVE_CS_KEY,
						   (uint32_t *)&payload,
						   sizeof(payload) / MBOX_WORD_BYTE,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_cmd_cb_ret3,
						   NULL,
						   0);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_GET_CS_KEY_INFO:
	{
		uint32_t session_id = (uint32_t)x2;
		uint32_t key_uid = (uint32_t)x3;
		uint64_t ret_key_addr = (uint64_t)x4;
		uint32_t key_len = (uint32_t)SMC_GET_GP(handle, CTX_GPREG_X5);

		if (!is_address_in_ddr_range(ret_key_addr, key_len)) {
			ERROR("MBOX: 0x%x: Addr not in DDR range\n", smc_fid);
			status = INTEL_SIP_SMC_STATUS_REJECTED;
			SMC_RET1(handle, status);
		}

		fcs_cs_key_payload payload = {session_id, RESERVED_AS_ZERO,
					      RESERVED_AS_ZERO, key_uid};

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_FCS_GET_CS_KEY_INFO,
						   (uint32_t *)&payload,
						   sizeof(payload) / MBOX_WORD_BYTE,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_ret_nbytes_cb,
						   (uint32_t *)ret_key_addr,
						   2);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_AES_CRYPT_INIT:
	{
		status = intel_fcs_aes_crypt_init(x2, x3, x4, x5,
					x6, &mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_AES_CRYPT_UPDATE:
	case ALTERA_SIP_SMC_ASYNC_FCS_AES_CRYPT_FINALIZE:
	{
		uint32_t job_id = 0U;
		bool is_final = (smc_fid == ALTERA_SIP_SMC_ASYNC_FCS_AES_CRYPT_FINALIZE) ?
				true : false;

		status = intel_fcs_aes_crypt_update_finalize(smc_fid, x1, x2,
					x3, x4, x5, x6, x7, x8, is_final,
					&job_id, x9, x10);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_GET_DIGEST_INIT:
	{
		status = intel_fcs_get_digest_init(x2, x3, x4, x5, x6,
					&mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_GET_DIGEST_UPDATE:
	case ALTERA_SIP_SMC_ASYNC_FCS_GET_DIGEST_FINALIZE:
	{
		bool is_final = (smc_fid == ALTERA_SIP_SMC_ASYNC_FCS_GET_DIGEST_FINALIZE) ?
				true : false;

		status = intel_fcs_get_digest_update_finalize(smc_fid, x1, x2,
					x3, x4, x5, x6, (uint32_t *) &x7,
					is_final, &mbox_error, x8);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_MAC_VERIFY_INIT:
	{
		status = intel_fcs_mac_verify_init(x2, x3, x4, x5, x6,
					&mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_MAC_VERIFY_UPDATE:
	case ALTERA_SIP_SMC_ASYNC_FCS_MAC_VERIFY_FINALIZE:
	{
		bool is_final = (smc_fid == ALTERA_SIP_SMC_ASYNC_FCS_MAC_VERIFY_FINALIZE) ?
				true : false;

		status = intel_fcs_mac_verify_update_finalize(smc_fid, x1, x2,
					x3, x4, x5, x6, (uint32_t *) &x7, x8,
					is_final, &mbox_error, x9);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_HASH_SIGN_INIT:
	{
		status = intel_fcs_ecdsa_hash_sign_init(x2, x3, x4, x5, x6,
					&mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_HASH_SIGN_FINALIZE:
	{
		status = intel_fcs_ecdsa_hash_sign_finalize(smc_fid, x1, x2, x3,
					x4, x5, x6, (uint32_t *) &x7,
					&mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_SHA2_DATA_SIGN_INIT:
	{
		status = intel_fcs_ecdsa_sha2_data_sign_init(x2, x3, x4, x5, x6,
					&mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_SHA2_DATA_SIGN_UPDATE:
	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_SHA2_DATA_SIGN_FINALIZE:
	{
		bool is_final = (smc_fid == ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_SHA2_DATA_SIGN_FINALIZE)
				? true : false;

		status = intel_fcs_ecdsa_sha2_data_sign_update_finalize(smc_fid,
					x1, x2, x3, x4, x5, x6, (uint32_t *) &x7,
					is_final, &mbox_error, x8);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_HASH_SIG_VERIFY_INIT:
	{
		status = intel_fcs_ecdsa_hash_sig_verify_init(x2, x3, x4, x5,
					x6, &mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_HASH_SIG_VERIFY_FINALIZE:
	{
		status = intel_fcs_ecdsa_hash_sig_verify_finalize(smc_fid, x1,
					x2, x3, x4, x5, x6, (uint32_t *) &x7,
					&mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_SHA2_DATA_SIG_VERIFY_INIT:
	{
		status = intel_fcs_ecdsa_sha2_data_sig_verify_init(x2, x3, x4,
					x5, x6, &mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_SHA2_DATA_SIG_VERIFY_UPDATE:
	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_SHA2_DATA_SIG_VERIFY_FINALIZE:
	{
		bool is_final = (smc_fid ==
				ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_SHA2_DATA_SIG_VERIFY_FINALIZE) ?
				true : false;

		status = intel_fcs_ecdsa_sha2_data_sig_verify_update_finalize(
					smc_fid, x1, x2, x3, x4, x5, x6,
					(uint32_t *) &x7, x8, is_final,
					&mbox_error, x9);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_GET_PUBKEY_INIT:
	{
		status = intel_fcs_ecdsa_get_pubkey_init(x2, x3, x4, x5, x6,
					&mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDSA_GET_PUBKEY_FINALIZE:
	{
		status = intel_fcs_ecdsa_get_pubkey_finalize(smc_fid, x1, x2, x3,
					x4, (uint32_t *) &x5, &mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDH_REQUEST_INIT:
	{
		status = intel_fcs_ecdh_request_init(x2, x3, x4, x5, x6,
					&mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_ECDH_REQUEST_FINALIZE:
	{
		uint32_t dest_size = (uint32_t)x7;

		NOTICE("MBOX: %s, %d: x7 0x%x, dest_size 0x%x\n",
			__func__, __LINE__, (uint32_t)x7, dest_size);

		status = intel_fcs_ecdh_request_finalize(smc_fid, x1, x2, x3,
					x4, x5, x6, (uint32_t *) &dest_size,
					&mbox_error);
		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_MCTP_MSG:
	{
		uint32_t *src_addr = (uint32_t *)x2;
		uint32_t src_size = (uint32_t)x3;
		uint32_t *dst_addr = (uint32_t *)x4;

		status = mailbox_send_cmd_async_v3(GET_CLIENT_ID(x1),
						   GET_JOB_ID(x1),
						   MBOX_CMD_MCTP_MSG,
						   src_addr,
						   src_size / MBOX_WORD_BYTE,
						   MBOX_CMD_FLAG_CASUAL,
						   sip_smc_ret_nbytes_cb,
						   dst_addr,
						   2);

		SMC_RET1(handle, status);
	}

	case ALTERA_SIP_SMC_ASYNC_FCS_HKDF_REQUEST:
	{
		status = intel_fcs_hkdf_request(smc_fid, x1, x2, x3, x4, x5, x6,
					x7);
		SMC_RET1(handle, status);
	}

	default:
		return socfpga_sip_handler(smc_fid, x1, x2, x3, x4,
					   cookie, handle, flags);
	} /* switch (smc_fid) */
}
#endif

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
	uint32_t err_states = 0;
	uint64_t retval64, rsu_respbuf[9];
	uint32_t seu_respbuf[3];
	int status = INTEL_SIP_SMC_STATUS_OK;
	int mbox_status;
	unsigned int len_in_resp = 0;
	u_register_t x5, x6, x7;

	switch (smc_fid) {
	case SIP_SVC_UID:
		/* Return UID to the caller */
		SMC_UUID_RET(handle, intl_svc_uid);

	case INTEL_SIP_SMC_FPGA_CONFIG_ISDONE:
		status = intel_mailbox_fpga_config_isdone(&err_states);
		SMC_RET4(handle, status, err_states, 0, 0);

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

	case INTEL_SIP_SMC_RSU_GET_DEVICE_INFO:
		status = intel_rsu_get_device_info((uint32_t *)rsu_respbuf,
					ARRAY_SIZE(rsu_respbuf));
		if (status) {
			SMC_RET1(handle, status);
		} else {
			SMC_RET5(handle, status, rsu_respbuf[0], rsu_respbuf[1],
				 rsu_respbuf[2], rsu_respbuf[3]);
		}

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
			status = intel_fcs_decryption_ext(smc_fid, 0, x1, x2, x4, x5, x6,
					(uint32_t *) &x7, &mbox_error, 0, 0, 0);
		} else if (x3 == FCS_MODE_ENCRYPT) {
			status = intel_fcs_encryption_ext(smc_fid, 0, x1, x2, x4, x5, x6,
					(uint32_t *) &x7, &mbox_error, 0, 0);
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
		status = intel_fcs_send_cert(smc_fid, 0, x1, x2, &send_id);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_FCS_GET_PROVISION_DATA:
		status = intel_fcs_get_provision_data(&send_id);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_FCS_CNTR_SET_PREAUTH:
		status = intel_fcs_cntr_set_preauth(smc_fid, 0, x1, x2, x3,
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
		status = intel_fcs_get_attestation_cert(smc_fid, 0, x1, x2,
					(uint32_t *) &x3, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x2, x3);

	case INTEL_SIP_SMC_FCS_CREATE_CERT_ON_RELOAD:
		status = intel_fcs_create_cert_on_reload(smc_fid, 0, x1, &mbox_error);
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
		status = intel_fcs_get_digest_update_finalize(smc_fid, 0, x1, x2,
					x3, x4, x5, (uint32_t *) &x6, false,
					&mbox_error, 0);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_GET_DIGEST_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_get_digest_update_finalize(smc_fid, 0, x1, x2,
					x3, x4, x5, (uint32_t *) &x6, true,
					&mbox_error, 0);
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
		status = intel_fcs_mac_verify_update_finalize(smc_fid, 0, x1, x2,
					x3, x4, x5, (uint32_t *) &x6, x7, false,
					&mbox_error, 0);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_MAC_VERIFY_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		x7 = SMC_GET_GP(handle, CTX_GPREG_X7);
		status = intel_fcs_mac_verify_update_finalize(smc_fid, 0, x1, x2,
					x3, x4, x5, (uint32_t *) &x6, x7, true,
					&mbox_error, 0);
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
		status = intel_fcs_ecdsa_sha2_data_sign_update_finalize(smc_fid,
					0, x1, x2, x3, x4, x5, (uint32_t *) &x6,
					false, &mbox_error, 0);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_SHA2_DATA_SIGN_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdsa_sha2_data_sign_update_finalize(smc_fid,
					0, x1, x2, x3, x4, x5, (uint32_t *) &x6,
					true, &mbox_error, 0);
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
		status = intel_fcs_ecdsa_hash_sign_finalize(smc_fid, 0, x1, x2,
					x3, x4, x5, (uint32_t *) &x6,
					&mbox_error);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_HASH_SIG_VERIFY_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_ecdsa_hash_sig_verify_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_ECDSA_HASH_SIG_VERIFY_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdsa_hash_sig_verify_finalize(smc_fid, 0, x1,
					x2, x3, x4, x5, (uint32_t *) &x6,
					&mbox_error);
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
					smc_fid, 0, x1, x2, x3, x4, x5,
					(uint32_t *) &x6, x7, false,
					&mbox_error, 0);
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
					smc_fid, 0, x1, x2, x3, x4, x5,
					(uint32_t *) &x6, x7, true,
					&mbox_error, 0);
		SMC_RET4(handle, status, mbox_error, x5, x6);

	case INTEL_SIP_SMC_FCS_ECDSA_GET_PUBKEY_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_ecdsa_get_pubkey_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_ECDSA_GET_PUBKEY_FINALIZE:
		status = intel_fcs_ecdsa_get_pubkey_finalize(
				INTEL_SIP_SMC_FCS_ECDSA_GET_PUBKEY_FINALIZE, 0,
				x1, x2, x3, (uint32_t *) &x4, &mbox_error);
		SMC_RET4(handle, status, mbox_error, x3, x4);

	case INTEL_SIP_SMC_FCS_ECDH_REQUEST_INIT:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		status = intel_fcs_ecdh_request_init(x1, x2, x3,
					x4, x5, &mbox_error);
		SMC_RET2(handle, status, mbox_error);

	case INTEL_SIP_SMC_FCS_ECDH_REQUEST_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_ecdh_request_finalize(smc_fid, 0, x1, x2, x3,
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
		status = intel_fcs_aes_crypt_update_finalize(smc_fid, 0, x1, x2,
					x3, x4, x5, x6, 0, false, &send_id, 0, 0);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_FCS_AES_CRYPT_FINALIZE:
		x5 = SMC_GET_GP(handle, CTX_GPREG_X5);
		x6 = SMC_GET_GP(handle, CTX_GPREG_X6);
		status = intel_fcs_aes_crypt_update_finalize(smc_fid, 0, x1, x2,
					x3, x4, x5, x6, 0, true, &send_id, 0, 0);
		SMC_RET1(handle, status);

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	case INTEL_SIP_SMC_FCS_SDM_REMAPPER_CONFIG:
		status = intel_smmu_hps_remapper_config(x1);
		SMC_RET1(handle, status);
#endif

	case INTEL_SIP_SMC_GET_ROM_PATCH_SHA384:
		status = intel_fcs_get_rom_patch_sha384(x1, &retval64,
							&mbox_error);
		SMC_RET4(handle, status, mbox_error, x1, retval64);

	case INTEL_SIP_SMC_SVC_VERSION:
		SMC_RET3(handle, INTEL_SIP_SMC_STATUS_OK,
					SIP_SVC_VERSION_MAJOR,
					SIP_SVC_VERSION_MINOR);

	case INTEL_SIP_SMC_SEU_ERR_STATUS:
		status = intel_sdm_seu_err_read(seu_respbuf,
					ARRAY_SIZE(seu_respbuf));
		if (status) {
			SMC_RET1(handle, status);
		} else {
			SMC_RET3(handle, seu_respbuf[0], seu_respbuf[1], seu_respbuf[2]);
		}

	case INTEL_SIP_SMC_SAFE_INJECT_SEU_ERR:
		status = intel_sdm_safe_inject_seu_err((uint32_t *)&x1, (uint32_t)x2);
		SMC_RET1(handle, status);

	case INTEL_SIP_SMC_ATF_BUILD_VER:
		SMC_RET4(handle, INTEL_SIP_SMC_STATUS_OK, VERSION_MAJOR,
			 VERSION_MINOR, VERSION_PATCH);

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	case INTEL_SIP_SMC_INJECT_IO96B_ECC_ERR:
		intel_inject_io96b_ecc_err((uint32_t *)&x1, (uint32_t)x2);
		SMC_RET1(handle, INTEL_SIP_SMC_STATUS_OK);
#endif

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
	}
#if SIP_SVC_V3
	else if ((cmd >= INTEL_SIP_SMC_CMD_V3_RANGE_BEGIN) &&
		(cmd <= INTEL_SIP_SMC_CMD_V3_RANGE_END)) {
		uintptr_t ret = sip_smc_handler_v3(smc_fid, x1, x2, x3, x4,
						   cookie, handle, flags);
		return ret;
	}
#endif
	else {
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
