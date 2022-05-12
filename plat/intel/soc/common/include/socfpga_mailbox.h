/*
 * Copyright (c) 2019-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_MBOX_H
#define SOCFPGA_MBOX_H

#include <lib/utils_def.h>


#define MBOX_OFFSET					0xffa30000

#define MBOX_ATF_CLIENT_ID				0x1U
#define MBOX_MAX_JOB_ID					0xFU
#define MBOX_MAX_IND_JOB_ID				(MBOX_MAX_JOB_ID - 1U)
#define MBOX_JOB_ID					MBOX_MAX_JOB_ID
#define MBOX_TEST_BIT					BIT(31)

/* Mailbox Shared Memory Register Map */
#define MBOX_CIN					0x00
#define MBOX_ROUT					0x04
#define MBOX_URG					0x08
#define MBOX_INT					0x0C
#define MBOX_COUT					0x20
#define MBOX_RIN					0x24
#define MBOX_STATUS					0x2C
#define MBOX_CMD_BUFFER					0x40
#define MBOX_RESP_BUFFER				0xC0

/* Mailbox SDM doorbell */
#define MBOX_DOORBELL_TO_SDM				0x400
#define MBOX_DOORBELL_FROM_SDM				0x480


/* Mailbox commands */

#define MBOX_CMD_NOOP					0x00
#define MBOX_CMD_SYNC					0x01
#define MBOX_CMD_RESTART				0x02
#define MBOX_CMD_CANCEL					0x03
#define MBOX_CMD_VAB_SRC_CERT				0x0B
#define MBOX_CMD_GET_IDCODE				0x10
#define MBOX_CMD_GET_USERCODE				0x13
#define MBOX_CMD_GET_CHIPID				0x12
#define MBOX_CMD_REBOOT_HPS				0x47

/* Reconfiguration Commands */
#define MBOX_CONFIG_STATUS				0x04
#define MBOX_RECONFIG					0x06
#define MBOX_RECONFIG_DATA				0x08
#define MBOX_RECONFIG_STATUS				0x09

/* HWMON Commands */
#define MBOX_HWMON_READVOLT				0x18
#define MBOX_HWMON_READTEMP				0x19


/* QSPI Commands */
#define MBOX_CMD_QSPI_OPEN				0x32
#define MBOX_CMD_QSPI_CLOSE				0x33
#define MBOX_CMD_QSPI_SET_CS				0x34
#define MBOX_CMD_QSPI_DIRECT				0x3B

/* RSU Commands */
#define MBOX_GET_SUBPARTITION_TABLE			0x5A
#define MBOX_RSU_STATUS					0x5B
#define MBOX_RSU_UPDATE					0x5C
#define MBOX_HPS_STAGE_NOTIFY				0x5D

/* FCS Command */
#define MBOX_FCS_GET_PROVISION				0x7B
#define MBOX_FCS_CNTR_SET_PREAUTH			0x7C
#define MBOX_FCS_ENCRYPT_REQ				0x7E
#define MBOX_FCS_DECRYPT_REQ				0x7F
#define MBOX_FCS_RANDOM_GEN				0x80
#define MBOX_FCS_AES_CRYPT_REQ				0x81
#define MBOX_FCS_GET_DIGEST_REQ				0x82
#define MBOX_FCS_MAC_VERIFY_REQ				0x83
#define MBOX_FCS_ECDSA_HASH_SIGN_REQ			0x84
#define MBOX_FCS_ECDSA_SHA2_DATA_SIGN_REQ		0x85
#define MBOX_FCS_ECDSA_HASH_SIG_VERIFY			0x86
#define MBOX_FCS_ECDSA_SHA2_DATA_SIGN_VERIFY		0x87
#define MBOX_FCS_ECDSA_GET_PUBKEY			0x88
#define MBOX_FCS_ECDH_REQUEST				0x89
#define MBOX_FCS_OPEN_CS_SESSION			0xA0
#define MBOX_FCS_CLOSE_CS_SESSION			0xA1
#define MBOX_FCS_IMPORT_CS_KEY				0xA5
#define MBOX_FCS_EXPORT_CS_KEY				0xA6
#define MBOX_FCS_REMOVE_CS_KEY				0xA7
#define MBOX_FCS_GET_CS_KEY_INFO			0xA8

/* PSG SIGMA Commands */
#define MBOX_PSG_SIGMA_TEARDOWN				0xD5

/* Attestation Commands */
#define MBOX_CREATE_CERT_ON_RELOAD			0x180
#define MBOX_GET_ATTESTATION_CERT			0x181
#define MBOX_ATTESTATION_SUBKEY				0x182
#define MBOX_GET_MEASUREMENT				0x183

/* Miscellaneous commands */
#define MBOX_GET_ROM_PATCH_SHA384	0x1B0

/* Mailbox Definitions */

#define CMD_DIRECT					0
#define CMD_INDIRECT					1
#define CMD_CASUAL					0
#define CMD_URGENT					1

#define MBOX_WORD_BYTE					4U
#define MBOX_RESP_BUFFER_SIZE				16
#define MBOX_CMD_BUFFER_SIZE				32
#define MBOX_INC_HEADER_MAX_WORD_SIZE			1024U

/* Execution states for HPS_STAGE_NOTIFY */
#define HPS_EXECUTION_STATE_FSBL			0
#define HPS_EXECUTION_STATE_SSBL			1
#define HPS_EXECUTION_STATE_OS				2

/* Status Response */
#define MBOX_RET_OK					0
#define MBOX_RET_ERROR					-1
#define MBOX_NO_RESPONSE				-2
#define MBOX_WRONG_ID					-3
#define MBOX_BUFFER_FULL				-4
#define MBOX_BUSY					-5
#define MBOX_TIMEOUT					-2047

/* Reconfig Status Response */
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


/* Mailbox Macros */

#define MBOX_ENTRY_TO_ADDR(_buf, ptr)			(MBOX_OFFSET + (MBOX_##_buf##_BUFFER) \
								+ MBOX_WORD_BYTE * (ptr))

/* Mailbox interrupt flags and masks */
#define MBOX_INT_FLAG_COE				0x1
#define MBOX_INT_FLAG_RIE				0x2
#define MBOX_INT_FLAG_UAE				0x100
#define MBOX_COE_BIT(INTERRUPT)				((INTERRUPT) & 0x3)
#define MBOX_UAE_BIT(INTERRUPT)				(((INTERRUPT) & (1<<8)))

/* Mailbox response and status */
#define MBOX_RESP_ERR(BUFFER)				((BUFFER) & 0x000007ff)
#define MBOX_RESP_LEN(BUFFER)				(((BUFFER) & 0x007ff000) >> 12)
#define MBOX_RESP_CLIENT_ID(BUFFER)			(((BUFFER) & 0xf0000000) >> 28)
#define MBOX_RESP_JOB_ID(BUFFER)			(((BUFFER) & 0x0f000000) >> 24)
#define MBOX_STATUS_UA_MASK				(1<<8)

/* Mailbox command and response */
#define MBOX_CLIENT_ID_CMD(CLIENT_ID)			((CLIENT_ID) << 28)
#define MBOX_JOB_ID_CMD(JOB_ID)				(JOB_ID<<24)
#define MBOX_CMD_LEN_CMD(CMD_LEN)			((CMD_LEN) << 12)
#define MBOX_INDIRECT(val)				((val) << 11)
#define MBOX_CMD_MASK(header)				((header) & 0x7ff)

/* Mailbox payload */
#define MBOX_DATA_MAX_LEN				0x3ff
#define MBOX_PAYLOAD_FLAG_BUSY				BIT(0)

/* RSU Macros */
#define RSU_VERSION_ACMF				BIT(8)
#define RSU_VERSION_ACMF_MASK				0xff00

/* Config Status Macros */
#define CONFIG_STATUS_WORD_SIZE		16U
#define CONFIG_STATUS_FW_VER_OFFSET	1
#define CONFIG_STATUS_FW_VER_MASK	0x00FFFFFF

/* Data structure */

typedef struct mailbox_payload {
	uint32_t header;
	uint32_t data[MBOX_DATA_MAX_LEN];
} mailbox_payload_t;

typedef struct mailbox_container {
	uint32_t flag;
	uint32_t index;
	mailbox_payload_t *payload;
} mailbox_container_t;

/* Mailbox Function Definitions */

void mailbox_set_int(uint32_t interrupt_input);
int mailbox_init(void);
void mailbox_set_qspi_close(void);
void mailbox_hps_qspi_enable(void);

int mailbox_send_cmd(uint32_t job_id, uint32_t cmd, uint32_t *args,
			unsigned int len, uint32_t urgent, uint32_t *response,
			unsigned int *resp_len);
int mailbox_send_cmd_async(uint32_t *job_id, uint32_t cmd, uint32_t *args,
			unsigned int len, unsigned int indirect);
int mailbox_send_cmd_async_ext(uint32_t header_cmd, uint32_t *args,
			unsigned int len);
int mailbox_read_response(uint32_t *job_id, uint32_t *response,
			unsigned int *resp_len);
int mailbox_read_response_async(uint32_t *job_id, uint32_t *header,
			uint32_t *response, unsigned int *resp_len,
			uint8_t ignore_client_id);
int iterate_resp(uint32_t mbox_resp_len, uint32_t *resp_buf,
			unsigned int *resp_len);

void mailbox_reset_cold(void);
void mailbox_clear_response(void);

int intel_mailbox_get_config_status(uint32_t cmd, bool init_done);
int intel_mailbox_is_fpga_not_ready(void);

int mailbox_rsu_get_spt_offset(uint32_t *resp_buf, uint32_t resp_buf_len);
int mailbox_rsu_status(uint32_t *resp_buf, uint32_t resp_buf_len);
int mailbox_rsu_update(uint32_t *flash_offset);
int mailbox_hps_stage_notify(uint32_t execution_stage);
int mailbox_hwmon_readtemp(uint32_t chan, uint32_t *resp_buf);
int mailbox_hwmon_readvolt(uint32_t chan, uint32_t *resp_buf);

#endif /* SOCFPGA_MBOX_H */
