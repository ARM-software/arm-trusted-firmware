/*
 * Copyright (c) 2020-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#include "mailbox/socfpga_mailbox_core.h"
#include <platform_def.h>
#include "socfpga_plat_def.h"
#include "socfpga_private.h"
#include "socfpga_sip_svc.h"
#include "socfpga_system_manager.h"

/*
 * Local helpers - grouped and ATF-style (single exit, explicit variables).
 */

/* Request SDM to open QSPI (enable mailbox interrupts + send OPEN command). */
static void mailbox_set_qspi_open(void)
{
	mailbox->set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);

	/* Fire command - synchronous (blocking) call handled by underlying mailbox layer. */
	mailbox->send(MBOX_JOB_ID, MBOX_CMD_QSPI_OPEN, NULL, 0U,
					CMD_CASUAL, NULL, NULL);
}

/* Query QSPI reference clock from SDM and store kHz value into SYSMGR scratch register. */
static void mailbox_set_qspi_direct(void)
{
	uint32_t response[1];
	unsigned int resp_len = ARRAY_SIZE(response);
	uint32_t qspi_clk = 0U;
	uint32_t reg = 0U;
	int status = 0;

	status = mailbox->send(MBOX_JOB_ID, MBOX_CMD_QSPI_DIRECT, NULL, 0U,
						CMD_CASUAL, response, &resp_len);
	if (status != MBOX_RET_OK) {
		INFO("MBOX: QSPI_DIRECT failed, status: %d\n", status);
		return;
	}

	qspi_clk = response[0];

	INFO("QSPI ref clock: %u\n", qspi_clk);

	/* Convert Hz -> kHz and store into BOOT_SCRATCH_COLD_0 bits[27:0] */
	if (qspi_clk != 0U) {
		uint32_t clk_khz = qspi_clk / 1000U;

		reg = mmio_read_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_0));
		reg &= ~SYSMGR_QSPI_REFCLK_MASK;
		reg |= (clk_khz & SYSMGR_QSPI_REFCLK_MASK);
		mmio_write_32(SOCFPGA_SYSMGR(BOOT_SCRATCH_COLD_0), reg);
	}
}

/* Close QSPI device via SDM command. */
void mailbox_set_qspi_close(void)
{
	mailbox->set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox->send(MBOX_JOB_ID, MBOX_CMD_QSPI_CLOSE, NULL, 0U,
					CMD_CASUAL, NULL, NULL);
}

/* Select QSPI chip-select lines (upper nibble of header argument). */
void mailbox_qspi_set_cs(uint32_t device_select)
{
	uint32_t cs_setting = 0U;

	cs_setting = (device_select << 28);
	mailbox->set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox->send(MBOX_JOB_ID, MBOX_CMD_QSPI_SET_CS, &cs_setting,
					1U, CMD_CASUAL, NULL, NULL);
}

/* Convenience: enable HPS QSPI (open + query direct). */
void mailbox_hps_qspi_enable(void)
{
	mailbox_set_qspi_open();
	mailbox_set_qspi_direct();
}

/* Reboot HPS (cold reset). */
void mailbox_reset_cold(void)
{
	mailbox->set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);
	mailbox->send(MBOX_JOB_ID, MBOX_CMD_REBOOT_HPS, NULL, 0U,
					CMD_CASUAL, NULL, NULL);
}

/* Reboot HPS (warm reset). reset_type is forced to 1 per SDM requirement. */
void mailbox_reset_warm(uint32_t reset_type)
{
	uint32_t hdr = 0U;

	mailbox->set_int(MBOX_INT_FLAG_COE | MBOX_INT_FLAG_RIE);

	hdr = 0x01U; /* Warm reset header data must be 1 */
	(void)reset_type;
	mailbox->send(MBOX_JOB_ID, MBOX_CMD_REBOOT_HPS, &hdr, 1U,
				CMD_CASUAL, NULL, NULL);
}

/* RSU: get subpartition table offset. */
int mailbox_rsu_get_spt_offset(uint32_t *resp_buf, unsigned int resp_buf_len)
{
	int status = MBOX_RET_OK;

	status = mailbox->send(MBOX_JOB_ID, MBOX_GET_SUBPARTITION_TABLE,
			       NULL, 0U, CMD_CASUAL, resp_buf, &resp_buf_len);
	return status;
}

/* RSU status structure used by mailbox_rsu_status helper. */
struct rsu_status_info {
	uint64_t current_image;
	uint64_t fail_image;
	uint32_t state;
	uint32_t version;
	uint32_t error_location;
	uint32_t error_details;
	uint32_t retry_counter;
};

/* RSU: request RSU status and perform minor compatibility patching. */
int mailbox_rsu_status(uint32_t *resp_buf, unsigned int resp_buf_len)
{
	int status = MBOX_RET_OK;
	struct rsu_status_info *info = (struct rsu_status_info *)resp_buf;

	if (info == NULL) {
		return -EINVAL;
	}

	info->retry_counter = ~0U;

	status = mailbox->send(MBOX_JOB_ID, MBOX_RSU_STATUS, NULL, 0U,
							CMD_CASUAL, resp_buf, &resp_buf_len);
	if (status != MBOX_RET_OK) {
		return status;
	}

	if (info->retry_counter != ~0U) {
		if ((info->version & RSU_VERSION_ACMF_MASK) == 0U) {
			info->version |= RSU_VERSION_ACMF;
		}
	}

	return status;
}

/* RSU: get device information. */
int mailbox_rsu_get_device_info(uint32_t *resp_buf, unsigned int resp_buf_len)
{
	int status = MBOX_RET_OK;

	status = mailbox->send(MBOX_JOB_ID, MBOX_RSU_GET_DEVICE_INFO, NULL, 0U,
							CMD_CASUAL, resp_buf, &resp_buf_len);
	return status;
}

/* RSU: request RSU update at given flash offset parameters. */
int mailbox_rsu_update(uint32_t *flash_offset)
{
	int status = MBOX_RET_OK;

	status = mailbox->send(MBOX_JOB_ID, MBOX_RSU_UPDATE,
					flash_offset, 2U, CMD_CASUAL, NULL, NULL);
	return status;
}

/* Notify HPS stage to SDM. */
int mailbox_hps_stage_notify(uint32_t execution_stage)
{
	int status = MBOX_RET_OK;

	status = mailbox->send(MBOX_JOB_ID, MBOX_HPS_STAGE_NOTIFY,
			       &execution_stage, 1U, CMD_CASUAL, NULL, NULL);
	return status;
}

/* Let SDM know FPGA config completed. */
int mailbox_send_fpga_config_comp(void)
{
	int status = MBOX_RET_OK;
	int ret_code = 0;

	status = mailbox->send(MBOX_JOB_ID, MBOX_CMD_FPGA_CONFIG_COMP, NULL, 0U,
							CMD_CASUAL, NULL, NULL);
	if (status != MBOX_RET_OK) {
		ret_code = MBOX_RESP_ERR(-status);
		INFO("SOCFPGA: FPGA configuration complete response, Return Code: 0x%x\n",
		     ret_code);
		return MBOX_RET_ERROR;
	}

	return MBOX_RET_OK;
}

/*
 * Read mailbox configuration/state from SDM and interpret into local config
 * states. Returns MBOX_RET_OK or an error/state code as defined by higher layer.
 */
int intel_mailbox_get_config_status(uint32_t cmd, bool init_done, uint32_t *err_states)
{
	int status = MBOX_RET_OK;
	uint32_t res = 0U;
	uint32_t response[6] = {0};
	unsigned int resp_len = ARRAY_SIZE(response);

	status = mailbox->send(MBOX_JOB_ID, cmd, NULL, 0U, CMD_CASUAL,
							response, &resp_len);
	if (status != MBOX_RET_OK) {
		return status;
	}

	res = response[RECONFIG_STATUS_STATE];

	if (err_states != NULL) {
		*err_states = res;
	}

	if (res == MBOX_CFGSTAT_VAB_BS_PREAUTH) {
		return MBOX_CFGSTAT_STATE_CONFIG;
	}

	if ((res != 0U) && (res != MBOX_CFGSTAT_STATE_CONFIG)) {
		return (int)res;
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

	if (init_done && ((res & SOFTFUNC_STATUS_INIT_DONE) == 0U)) {
		return MBOX_CFGSTAT_STATE_CONFIG;
	}

	return MBOX_RET_OK;
}

/* Returns whether FPGA isn't ready according to SDM reports. */
int intel_mailbox_is_fpga_not_ready(void)
{
	int status = MBOX_RET_OK;

	status = intel_mailbox_get_config_status(MBOX_RECONFIG_STATUS, true, NULL);
	if ((status != MBOX_RET_OK) && (status != MBOX_CFGSTAT_STATE_CONFIG)) {
		status = intel_mailbox_get_config_status(MBOX_CONFIG_STATUS, false, NULL);
	}

	return status;
}

/* Read temperature channel via mailbox. */
int mailbox_hwmon_readtemp(uint32_t chan, uint32_t *resp_buf)
{
	int status = MBOX_RET_OK;
	unsigned int resp_len = sizeof(resp_buf);

	status = mailbox->send(MBOX_JOB_ID, MBOX_HWMON_READTEMP, &chan, 1U,
						CMD_CASUAL, resp_buf, &resp_len);
	return status;
}

/* Read voltage channel via mailbox. */
int mailbox_hwmon_readvolt(uint32_t chan, uint32_t *resp_buf)
{
	int status = MBOX_RET_OK;
	unsigned int resp_len = sizeof(resp_buf);

	status = mailbox->send(MBOX_JOB_ID, MBOX_HWMON_READVOLT, &chan, 1U,
							CMD_CASUAL, resp_buf, &resp_len);
	return status;
}

/* Read SEU error status. */
int mailbox_seu_err_status(uint32_t *resp_buf, unsigned int resp_buf_len)
{
	int status = MBOX_RET_OK;

	status = mailbox->send(MBOX_JOB_ID, MBOX_CMD_SEU_ERR_READ, NULL, 0U,
						CMD_CASUAL, resp_buf, &resp_buf_len);
	return status;
}

/* Inject a safe SEU error via mailbox. */
int mailbox_safe_inject_seu_err(uint32_t *arg, unsigned int len)
{
	int status = MBOX_RET_OK;

	status = mailbox->send(MBOX_JOB_ID, MBOX_CMD_SAFE_INJECT_SEU_ERR, arg, len,
							CMD_CASUAL, NULL, NULL);
	return status;
}
