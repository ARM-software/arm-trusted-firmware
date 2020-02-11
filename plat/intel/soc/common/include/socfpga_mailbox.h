/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_MBOX_H
#define SOCFPGA_MBOX_H

#include <lib/utils_def.h>

#define MBOX_OFFSET			0xffa30000

#define MBOX_MAX_JOB_ID			0xf
#define MBOX_ATF_CLIENT_ID		0x1
#define MBOX_JOB_ID			0x1

/* Mailbox interrupt flags and masks */
#define MBOX_INT_FLAG_COE		0x1
#define MBOX_INT_FLAG_RIE		0x2
#define MBOX_INT_FLAG_UAE		0x100
#define MBOX_COE_BIT(INTERRUPT)		((INTERRUPT) & 0x3)
#define MBOX_UAE_BIT(INTERRUPT)		(((INTERRUPT) & (1<<8)))

/* Mailbox response and status */
#define MBOX_RESP_BUFFER_SIZE		16
#define MBOX_RESP_ERR(BUFFER)		((BUFFER) & 0x00000fff)
#define MBOX_RESP_LEN(BUFFER)		(((BUFFER) & 0x007ff000) >> 12)
#define MBOX_RESP_CLIENT_ID(BUFFER)	(((BUFFER) & 0xf0000000) >> 28)
#define MBOX_RESP_JOB_ID(BUFFER)	(((BUFFER) & 0x0f000000) >> 24)
#define MBOX_STATUS_UA_MASK		(1<<8)

/* Mailbox command and response */
#define MBOX_CMD_FREE_OFFSET		0x14
#define MBOX_CMD_BUFFER_SIZE		32
#define MBOX_CLIENT_ID_CMD(CLIENT_ID)	((CLIENT_ID) << 28)
#define MBOX_JOB_ID_CMD(JOB_ID)		(JOB_ID<<24)
#define MBOX_CMD_LEN_CMD(CMD_LEN)	((CMD_LEN) << 12)
#define MBOX_INDIRECT			(1 << 11)
#define MBOX_INSUFFICIENT_BUFFER	-2
#define MBOX_CIN			0x00
#define MBOX_ROUT			0x04
#define MBOX_URG			0x08
#define MBOX_INT			0x0C
#define MBOX_COUT			0x20
#define MBOX_RIN			0x24
#define MBOX_STATUS			0x2C
#define MBOX_CMD_BUFFER			0x40
#define MBOX_RESP_BUFFER		0xC0

#define MBOX_RESP_BUFFER_SIZE		16
#define MBOX_RESP_OK			0
#define MBOX_RESP_INVALID_CMD		1
#define MBOX_RESP_UNKNOWN_BR		2
#define MBOX_RESP_UNKNOWN		3
#define MBOX_RESP_NOT_CONFIGURED	256

/* Mailbox SDM doorbell */
#define MBOX_DOORBELL_TO_SDM		0x400
#define MBOX_DOORBELL_FROM_SDM		0x480

/* Mailbox QSPI commands */
#define MBOX_CMD_RESTART		2
#define MBOX_CMD_QSPI_OPEN		50
#define MBOX_CMD_QSPI_CLOSE		51
#define MBOX_CMD_QSPI_DIRECT		59
#define MBOX_CMD_GET_IDCODE		16
#define MBOX_CMD_QSPI_SET_CS		52

/* Mailbox CANCEL command */
#define MBOX_CMD_CANCEL			0x3

/* Mailbox REBOOT commands */
#define MBOX_CMD_REBOOT_HPS		71

/* Mailbox RSU commands */
#define MBOX_GET_SUBPARTITION_TABLE	90
#define MBOX_RSU_STATUS			91
#define MBOX_RSU_UPDATE			92

/* Mailbox RSU macros */
#define RSU_VERSION_ACMF		BIT(8)
#define RSU_VERSION_ACMF_MASK		0xff00

/* HPS stage notify command */
#define MBOX_HPS_STAGE_NOTIFY		93

/* Execution states for HPS_STAGE_NOTIFY */
#define HPS_EXECUTION_STATE_FSBL	0
#define HPS_EXECUTION_STATE_SSBL	1
#define HPS_EXECUTION_STATE_OS		2

/* Mailbox reconfiguration commands */
#define MBOX_CONFIG_STATUS		4
#define MBOX_RECONFIG			6
#define MBOX_RECONFIG_DATA		8
#define MBOX_RECONFIG_STATUS		9

/* Generic error handling */
#define MBOX_TIMEOUT			-2047
#define MBOX_NO_RESPONSE		-2
#define MBOX_WRONG_ID			-3

/* Mailbox status */
#define RECONFIG_STATUS_STATE				0
#define RECONFIG_STATUS_PIN_STATUS			2
#define RECONFIG_STATUS_SOFTFUNC_STATUS			3
#define PIN_STATUS_NSTATUS				(U(1) << 31)
#define SOFTFUNC_STATUS_SEU_ERROR			(1 << 3)
#define SOFTFUNC_STATUS_INIT_DONE			(1 << 1)
#define SOFTFUNC_STATUS_CONF_DONE			(1 << 0)
#define MBOX_CFGSTAT_STATE_IDLE				0x00000000
#define MBOX_CFGSTAT_STATE_CONFIG			0x10000000
#define MBOX_CFGSTAT_STATE_FAILACK			0x08000000
#define MBOX_CFGSTAT_STATE_ERROR_INVALID		0xf0000001
#define MBOX_CFGSTAT_STATE_ERROR_CORRUPT		0xf0000002
#define MBOX_CFGSTAT_STATE_ERROR_AUTH			0xf0000003
#define MBOX_CFGSTAT_STATE_ERROR_CORE_IO		0xf0000004
#define MBOX_CFGSTAT_STATE_ERROR_HARDWARE		0xf0000005
#define MBOX_CFGSTAT_STATE_ERROR_FAKE			0xf0000006
#define MBOX_CFGSTAT_STATE_ERROR_BOOT_INFO		0xf0000007
#define MBOX_CFGSTAT_STATE_ERROR_QSPI_ERROR		0xf0000008

void mailbox_set_int(int interrupt_input);
int mailbox_init(void);
void mailbox_set_qspi_close(void);
void mailbox_set_qspi_open(void);
void mailbox_set_qspi_direct(void);
int mailbox_send_cmd(int job_id, unsigned int cmd, uint64_t *args,
			int len, int urgent, uint32_t *response, int resp_len);
int mailbox_send_cmd_async(int job_id, unsigned int cmd, uint64_t *args,
				int len, int urgent);
int mailbox_read_response(int job_id, uint32_t *response, int resp_len);
int mailbox_get_qspi_clock(void);
void mailbox_reset_cold(void);
void mailbox_clear_response(void);

int intel_mailbox_get_config_status(uint32_t cmd);
int intel_mailbox_is_fpga_not_ready(void);

int mailbox_rsu_get_spt_offset(uint32_t *resp_buf, uint32_t resp_buf_len);
int mailbox_rsu_status(uint32_t *resp_buf, uint32_t resp_buf_len);
int mailbox_rsu_update(uint64_t *flash_offset);
int mailbox_hps_stage_notify(uint64_t execution_stage);

#endif /* SOCFPGA_MBOX_H */
