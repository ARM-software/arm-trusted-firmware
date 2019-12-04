/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_MBOX_H
#define SOCFPGA_MBOX_H

#include <lib/utils_def.h>

#define MBOX_OFFSET			0xffa30000

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

/* Mailbox REBOOT commands */
#define MBOX_CMD_REBOOT_HPS		71

/* Generic error handling */
#define MBOX_TIMEOUT			-2047
#define MBOX_NO_RESPONSE		-2
#define MBOX_WRONG_ID			-3

/* Mailbox status */
#define RECONFIG_STATUS_STATE		0
#define RECONFIG_STATUS_PIN_STATUS	2
#define RECONFIG_STATUS_SOFTFUNC_STATUS 3
#define PIN_STATUS_NSTATUS		(U(1) << 31)
#define SOFTFUNC_STATUS_SEU_ERROR	(1 << 3)
#define SOFTFUNC_STATUS_INIT_DONE	(1 << 1)
#define SOFTFUNC_STATUS_CONF_DONE	(1 << 0)
#define MBOX_CFGSTAT_STATE_CONFIG	0x10000000

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_CALL_COUNT	0x8200ff00
#define SIP_SVC_UID		0x8200ff01
#define SIP_SVC_VERSION		0x8200ff03

/* SiP Service Calls version numbers */
#define SIP_SVC_VERSION_MAJOR	0
#define SIP_SVC_VERSION_MINOR	1

/* Mailbox reconfiguration commands */
#define MBOX_RECONFIG		6
#define MBOX_RECONFIG_DATA	8
#define MBOX_RECONFIG_STATUS	9

/* Sip get memory */
#define INTEL_SIP_SMC_FPGA_CONFIG_START			0xC2000001
#define INTEL_SIP_SMC_FPGA_CONFIG_GET_MEM		0xC2000005
#define INTEL_SIP_SMC_FPGA_CONFIG_ISDONE		0xC2000004
#define INTEL_SIP_SMC_FPGA_CONFIG_WRITE			0x42000002
#define INTEL_SIP_SMC_FPGA_CONFIG_COMPLETED_WRITE	0xC2000003
#define INTEL_SIP_SMC_STATUS_OK				0
#define INTEL_SIP_SMC_STATUS_ERROR			0x4
#define INTEL_SIP_SMC_STATUS_BUSY			0x1
#define INTEL_SIP_SMC_STATUS_REJECTED			0x2
#define INTEL_SIP_SMC_FPGA_CONFIG_ADDR			0x1000
#define INTEL_SIP_SMC_FPGA_CONFIG_SIZE			16777216

void mailbox_set_int(int interrupt_input);
int mailbox_init(void);
void mailbox_set_qspi_close(void);
void mailbox_set_qspi_open(void);
void mailbox_set_qspi_direct(void);
int mailbox_send_cmd(int job_id, unsigned int cmd, uint32_t *args,
				int len, int urgent, uint32_t *response);
void mailbox_send_cmd_async(int job_id, unsigned int cmd, uint32_t *args,
				int len, int urgent);
int mailbox_read_response(int job_id, uint32_t *response);
int mailbox_get_qspi_clock(void);
void mailbox_reset_cold(void);

#endif /* SOCFPGA_MBOX_H */
