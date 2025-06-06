/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_MBOX_H
#define SOCFPGA_MBOX_H

#include <lib/utils_def.h>

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
#define MBOX_OFFSET					0x10a30000
#else
#define MBOX_OFFSET					0xffa30000
#endif

#define MBOX_ATF_CLIENT_ID				0x1U
#define MBOX_MAX_JOB_ID					0xFU
#define MBOX_MAX_IND_JOB_ID				(MBOX_MAX_JOB_ID - 1U)
#define MBOX_JOB_ID					MBOX_MAX_JOB_ID
#define MBOX_TEST_BIT					BIT(31)

/* Mailbox Shared Memory Register Map */
#define MBOX_CIN					0x00 /* Command valid offset, to SDM */
#define MBOX_ROUT					0x04 /* Response output offset, to SDM */
#define MBOX_URG					0x08 /* Urgent command, to SDM */
#define MBOX_INT					0x0C /* Interrupt enables, to SDM */
/* 0x10 - 0x1F, Reserved */

#define MBOX_COUT					0x20 /* Command free offset, from SDM */
#define MBOX_RIN					0x24 /* Response valid offset, from SDM */
#define MBOX_STATUS					0x2C /* Mailbox status from SDM to client */
/* 0x30 - 0x3F, Reserved */

#define MBOX_CMD_BUFFER					0x40 /* Circular buffer, cmds to SDM */
#define MBOX_RESP_BUFFER				0xC0 /* Circular buffer, resp from SDM */

#define MBOX_DOORBELL_TO_SDM				0x400 /* Doorbell from HPS to SDM */
#define MBOX_DOORBELL_FROM_SDM				0x480 /* Doorbell from SDM to HPS */

/* Mailbox commands */

#define MBOX_CMD_NOOP					0x00
#define MBOX_CMD_SYNC					0x01
#define MBOX_CMD_RESTART				0x02
#define MBOX_CMD_CANCEL					0x03
#define MBOX_CMD_VAB_SRC_CERT				0x0B
#define MBOX_CMD_GET_IDCODE				0x10
#define MBOX_CMD_GET_USERCODE				0x13
#define MBOX_CMD_GET_CHIPID				0x12
#define MBOX_CMD_FPGA_CONFIG_COMP			0x45
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
#define MBOX_CMD_QSPI_ERASE				0x38
#define MBOX_CMD_QSPI_WRITE				0x39
#define MBOX_CMD_QSPI_READ				0x3A
#define MBOX_CMD_QSPI_DIRECT				0x3B
#define MBOX_CMD_QSPI_GET_DEV_INFO			0x74

/* SEU Commands */
#define MBOX_CMD_SEU_ERR_READ				0x3C
#define MBOX_CMD_SAFE_INJECT_SEU_ERR			0x41

/* RSU Commands */
#define MBOX_GET_SUBPARTITION_TABLE			0x5A
#define MBOX_RSU_STATUS					0x5B
#define MBOX_RSU_UPDATE					0x5C
#define MBOX_HPS_STAGE_NOTIFY				0x5D
#define MBOX_RSU_GET_DEVICE_INFO			0x74

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
#define MBOX_FCS_HKDF_REQUEST				0x8B
#define MBOX_FCS_OPEN_CS_SESSION			0xA0
#define MBOX_FCS_CLOSE_CS_SESSION			0xA1
#define MBOX_FCS_IMPORT_CS_KEY				0xA5
#define MBOX_FCS_EXPORT_CS_KEY				0xA6
#define MBOX_FCS_REMOVE_CS_KEY				0xA7
#define MBOX_FCS_GET_CS_KEY_INFO			0xA8
#define MBOX_FCS_CREATE_CS_KEY				0xA9

/* PSG SIGMA Commands */
#define MBOX_PSG_SIGMA_TEARDOWN				0xD5

/* Attestation Commands */
#define MBOX_CREATE_CERT_ON_RELOAD			0x180
#define MBOX_GET_ATTESTATION_CERT			0x181
#define MBOX_ATTESTATION_SUBKEY				0x182
#define MBOX_GET_MEASUREMENT				0x183

/* Miscellaneous commands */
#define MBOX_CMD_MCTP_MSG				0x194
#define MBOX_GET_ROM_PATCH_SHA384			0x1B0
#define MBOX_CMD_GET_DEVICEID				0x500

/* Mailbox Definitions */

#define CMD_DIRECT					0
#define CMD_INDIRECT					1
#define CMD_CASUAL					0
#define CMD_URGENT					1

/* Mailbox command flags and related macros */
#define MBOX_CMD_FLAG_DIRECT				BIT(0)
#define MBOX_CMD_FLAG_INDIRECT				BIT(1)
#define MBOX_CMD_FLAG_CASUAL				BIT(2)
#define MBOX_CMD_FLAG_URGENT				BIT(3)

#define MBOX_CMD_FLAG_CASUAL_INDIRECT			(MBOX_CMD_FLAG_CASUAL | \
							 MBOX_CMD_FLAG_INDIRECT)

#define IS_CMD_SET(cmd, _type)				((((cmd) & MBOX_CMD_FLAG_##_type) != 0) ? \
								1 : 0)

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

/* Key Status */
#define MBOX_RET_SDOS_DECRYPTION_ERROR_102		-258
#define MBOX_RET_SDOS_DECRYPTION_ERROR_103		-259

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
#define MBOX_CFGSTAT_VAB_BS_PREAUTH			0x20000000
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
#define MBOX_INT_FLAG_COE				BIT(0) /* COUT update interrupt enable */
#define MBOX_INT_FLAG_RIE				BIT(1) /* RIN update interrupt enable */
#define MBOX_INT_FLAG_UAE				BIT(8) /* Urgent ACK interrupt enable */

#define MBOX_COE_BIT(INTERRUPT)				((INTERRUPT) & MBOX_INT_FLAG_COE)
#define MBOX_RIE_BIT(INTERRUPT)				((INTERRUPT) & MBOX_INT_FLAG_RIE)
#define MBOX_UAE_BIT(INTERRUPT)				((INTERRUPT) & MBOX_INT_FLAG_UAE)

/* Mailbox response and status */
#define MBOX_RESP_ERR(BUFFER)				((BUFFER) & 0x000007ff)
#define MBOX_RESP_LEN(BUFFER)				(((BUFFER) & 0x007ff000) >> 12)
#define MBOX_RESP_CLIENT_ID(BUFFER)			(((BUFFER) & 0xf0000000) >> 28)
#define MBOX_RESP_JOB_ID(BUFFER)			(((BUFFER) & 0x0f000000) >> 24)
#define MBOX_RESP_TRANSACTION_ID(BUFFER)		(((BUFFER) & 0xff000000) >> 24)
#define MBOX_STATUS_UA_MASK				(1<<8)

/* Mailbox command and response */
#define MBOX_CLIENT_ID_CMD(CLIENT_ID)			((CLIENT_ID) << 28)
#define MBOX_JOB_ID_CMD(JOB_ID)				(JOB_ID << 24)
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
#define CONFIG_STATUS_WORD_SIZE				16U
#define CONFIG_STATUS_FW_VER_OFFSET			1
#define CONFIG_STATUS_FW_VER_MASK			0x00FFFFFF

/* QSPI mailbox command macros */
#define MBOX_QSPI_SET_CS_OFFSET				(28)
#define MBOX_QSPI_SET_CS_MODE_OFFSET			(27)
#define MBOX_QSPI_SET_CS_CA_OFFSET			(26)
#define MBOX_QSPI_ERASE_SIZE_GRAN			(0x400)

#define MBOX_4K_ALIGNED_MASK				(0xFFF)
#define MBOX_IS_4K_ALIGNED(x)				((x) & MBOX_4K_ALIGNED_MASK)
#define MBOX_IS_WORD_ALIGNED(x)				(!((x) & 0x3))
#define MBOX_QSPI_RW_MAX_WORDS				(0x1000)

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
void mailbox_reset_warm(uint32_t reset_type);
void mailbox_clear_response(void);

int intel_mailbox_get_config_status(uint32_t cmd, bool init_done,
				    uint32_t *err_states);
int intel_mailbox_is_fpga_not_ready(void);

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
void intel_smmu_hps_remapper_init(uint64_t *mem);
int intel_smmu_hps_remapper_config(uint32_t remapper_bypass);
#endif

int mailbox_rsu_get_spt_offset(uint32_t *resp_buf, uint32_t resp_buf_len);
int mailbox_rsu_status(uint32_t *resp_buf, uint32_t resp_buf_len);
int mailbox_rsu_get_device_info(uint32_t *resp_buf, uint32_t resp_buf_len);
int mailbox_rsu_update(uint32_t *flash_offset);
int mailbox_hps_stage_notify(uint32_t execution_stage);
int mailbox_hwmon_readtemp(uint32_t chan, uint32_t *resp_buf);
int mailbox_hwmon_readvolt(uint32_t chan, uint32_t *resp_buf);
int mailbox_seu_err_status(uint32_t *resp_buf, uint32_t resp_buf_len);
int mailbox_safe_inject_seu_err(uint32_t *arg, unsigned int len);

int mailbox_send_fpga_config_comp(void);

#if SIP_SVC_V3
#define MBOX_CLIENT_ID_SHIFT				(28)
#define MBOX_JOB_ID_SHIFT				(24)
#define MBOX_CMD_LEN_SHIFT				(12)
#define MBOX_INDIRECT_SHIFT				(11)

#define MBOX_FRAME_CMD_HEADER(client_id, job_id, args_len, indirect, cmd)\
				((client_id << MBOX_CLIENT_ID_SHIFT) |	 \
				(job_id << MBOX_JOB_ID_SHIFT) |		 \
				(args_len << MBOX_CMD_LEN_SHIFT) |	 \
				(indirect << MBOX_INDIRECT_SHIFT) |	 \
				cmd)

#define FLAG_SDM_RESPONSE_IS_VALID			BIT(0)
#define FLAG_SDM_RESPONSE_IS_USED			BIT(1)
#define FLAG_SDM_RESPONSE_IS_IN_PROGRESS		BIT(2)
#define FLAG_SDM_RESPONSE_IS_POLL_ON_INTR		BIT(3)

/*
 * TODO: Re-visit this queue size based on the system load.
 * 4 bits for client ID and 4 bits for job ID, total 8 bits and we can have up to
 * 256 transactions. We can tune this based on our system load at any given time
 */
#define MBOX_SVC_CMD_QUEUE_SIZE				(32)
#define MBOX_SVC_RESP_QUEUE_SIZE			(32)
#define MBOX_SVC_MAX_JOB_ID				(16)
#define MBOX_SVC_CMD_ARG_SIZE				(2)
#define MBOX_SVC_CMD_IS_USED				BIT(0)
#define MBOX_SVC_CMD_CB_ARGS_SIZE			(4)
#define MBOX_SVC_MAX_CLIENTS				(16)
#define MBOX_SVC_MAX_RESP_DATA_SIZE			(32)
#define MBOX_SVC_SMC_RET_MAX_SIZE			(8)

/* Client ID(4bits) + Job ID(4bits) = Transcation ID(TID - 8bits, 256 combinations) */
#define MBOX_MAX_TIDS					(256)
/* Each transcation ID bitmap holds 64bits */
#define MBOX_TID_BITMAP_SIZE				(sizeof(uint64_t) * 8)
/* Number of transcation ID bitmaps required to hold 256 combinations */
#define MBOX_MAX_TIDS_BITMAP				(MBOX_MAX_TIDS / MBOX_TID_BITMAP_SIZE)

/* SDM Response State (SRS) enums */
typedef enum sdm_resp_state {
	SRS_WAIT_FOR_RESP = 0x00U,
	SRS_WAIT_FOR_HEADER,
	SRS_WAIT_FOR_ARGUMENTS,
	SRS_SYNC_ERROR
} sdm_resp_state_t;

/* SDM response data structure */
typedef struct sdm_response {
	bool is_poll_intr;
	uint8_t client_id;
	uint8_t job_id;
	uint16_t resp_len;
	uint16_t err_code;
	uint32_t flags;
	uint32_t header;
	uint16_t rcvd_resp_len;
	uint32_t resp_data[MBOX_SVC_MAX_RESP_DATA_SIZE];
} sdm_response_t;

/* SDM client callback template */
typedef uint8_t (*sdm_command_callback)(void *resp, void *cmd,
					uint64_t *ret_args);

/* SDM command data structure */
typedef struct sdm_command {
	uint8_t client_id;
	uint8_t job_id;
	uint32_t flags;
	sdm_command_callback cb;
	uint32_t *cb_args;
	uint8_t cb_args_len;
} sdm_command_t;

/* Get the transcation ID from client ID and job ID. */
#define MBOX_GET_TRANS_ID(cid, jib)			(((cid) << 4) | (jib))

/* Mailbox service data structure */
typedef struct mailbox_service {
	sdm_resp_state_t resp_state;
	sdm_resp_state_t next_resp_state;
	uint32_t flags;
	int curr_di;
	uint64_t received_bitmap[MBOX_MAX_TIDS_BITMAP];
	uint64_t interrupt_bitmap[MBOX_MAX_TIDS_BITMAP];
	sdm_command_t cmd_queue[MBOX_SVC_CMD_QUEUE_SIZE];
	sdm_response_t resp_queue[MBOX_SVC_RESP_QUEUE_SIZE];
} mailbox_service_t;

int mailbox_send_cmd_async_v3(uint8_t client_id, uint8_t job_id, uint32_t cmd,
			      uint32_t *args, uint32_t args_len, uint8_t cmd_flag,
			      sdm_command_callback cb, uint32_t *cb_args,
			      uint32_t cb_args_len);

int mailbox_response_poll_v3(uint8_t client_id, uint8_t job_id, uint64_t *ret_args,
			     uint32_t *ret_args_size);

int mailbox_response_poll_on_intr_v3(uint8_t *client_id, uint8_t *job_id,
				     uint64_t *bitmap);

#endif		/* #if SIP_SVC_V3 */

#endif /* SOCFPGA_MBOX_H */
