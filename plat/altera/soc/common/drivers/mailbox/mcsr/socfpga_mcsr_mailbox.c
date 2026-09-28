/*
 * Copyright (c) 2024-2026, Intel Corporation. All rights reserved.
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * MCSR (multi-channel/ring based) SDM mailbox backend.
 *
 * This translation unit is only compiled for platforms that select the MCSR
 * backend (e.g. Agilex72).  It owns all descriptor-ring state and exposes the
 * mcsr_backend vtable consumed by the common core dispatcher.
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>

#include "mailbox/socfpga_mailbox_core.h"
#include <platform_def.h>
#include "socfpga_mcsr_mailbox.h"

/*
 * Message header bitfields / helpers
 */
#define MSG_HEADER_CHANNEL_NUM_BITOFFSET	28
#define MSG_HEADER_SEQUENCE_ID_BITOFFSET	24
#define MSG_HEADER_TRANSACTION_ID_BITOFFSET	24
#define MSG_HEADER_CHECKSUM_BITOFFSET		23
#define MSG_HEADER_LENGTH_BITOFFSET			12
#define MSG_HEADER_COMMAND_BITOFFSET		0

#define MSG_HEADER_TRANSACTION_ID_MASK		(0xFFu << MSG_HEADER_TRANSACTION_ID_BITOFFSET)
#define MSG_HEADER_CHANNEL_NUM_MASK			(0xFu << MSG_HEADER_CHANNEL_NUM_BITOFFSET)
#define MSG_HEADER_SEQUENCE_ID_MASK			(0xFu << MSG_HEADER_SEQUENCE_ID_BITOFFSET)
#define MSG_HEADER_CHECKSUM_MASK			(0x1u << MSG_HEADER_CHECKSUM_BITOFFSET)
#define MSG_HEADER_LENGTH_MASK				(0x7FFu << MSG_HEADER_LENGTH_BITOFFSET)
#define MSG_HEADER_COMMAND_MASK				(0xFFFu << MSG_HEADER_COMMAND_BITOFFSET)

#define MSG_HEADER_MAKE(ch, seq, cks, len, cmd) ( \
	(((uint32_t)(ch)  << MSG_HEADER_CHANNEL_NUM_BITOFFSET) & MSG_HEADER_CHANNEL_NUM_MASK) | \
	(((uint32_t)(seq) << MSG_HEADER_SEQUENCE_ID_BITOFFSET) & MSG_HEADER_SEQUENCE_ID_MASK) | \
	(((uint32_t)(cks) << MSG_HEADER_CHECKSUM_BITOFFSET) & MSG_HEADER_CHECKSUM_MASK) | \
	(((uint32_t)(len) << MSG_HEADER_LENGTH_BITOFFSET) & MSG_HEADER_LENGTH_MASK) | \
	(((uint32_t)(cmd) << MSG_HEADER_COMMAND_BITOFFSET) & MSG_HEADER_COMMAND_MASK))

#define MSG_HEADER_GET_CHANNEL_NUM(x) \
	(((x) & MSG_HEADER_CHANNEL_NUM_MASK) >> MSG_HEADER_CHANNEL_NUM_BITOFFSET)
#define MSG_HEADER_GET_SEQUENCE_ID(x) \
	(((x) & MSG_HEADER_SEQUENCE_ID_MASK) >> MSG_HEADER_SEQUENCE_ID_BITOFFSET)
#define MSG_HEADER_GET_TRANSACTION_ID(x) \
	(((x) & MSG_HEADER_TRANSACTION_ID_MASK) >> MSG_HEADER_TRANSACTION_ID_BITOFFSET)
#define MSG_HEADER_GET_CHECKSUM(x) \
	(((x) & MSG_HEADER_CHECKSUM_MASK) >> MSG_HEADER_CHECKSUM_BITOFFSET)
#define MSG_HEADER_GET_LENGTH(x) \
	(((x) & MSG_HEADER_LENGTH_MASK) >> MSG_HEADER_LENGTH_BITOFFSET)
#define MSG_HEADER_GET_CMD_RESP(x) \
	(((x) & MSG_HEADER_COMMAND_MASK) >> MSG_HEADER_COMMAND_BITOFFSET)

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX72

#define MAILBOX_MAX_QUEUE_SIZE				11
#define MAILBOX_MSG_MAX_LENGTH				64
#define MAILBOX_RESP_MAX_LENGTH				64
/*
 * On Agilex72 Simics the MCSR rings use fixed OCRAM addresses defined in
 * socfpga_plat_def.h.  These must not overlap with the baremetal image
 * (see linker_ocram.ld: DATA_ORIGIN must be >= 0x32000 for OCRAM builds).
 */
struct mailbox_message {
	uint32_t msg_header;
	uint32_t msg_data[MAILBOX_MSG_MAX_LENGTH];
};

struct mailbox_response {
	uint32_t msg_header;
	uint32_t msg_data[MAILBOX_RESP_MAX_LENGTH];
};

/*
 * MCSR SDM mailbox — Simics internal OCRAM buffers
 *
 * On Agilex72 Simics the MCSR hardware rings point to fixed OCRAM addresses.
 * message_buffer  : C2S (HPS→SDM) ring, 11 entries × 260 B =  2860 B = 0x20000..0x20B2C
 * response_buffer : S2C (SDM→HPS) ring, 11 entries × 516 B =  5676 B = 0x30000..0x3162C
 *
 * These must NOT overlap with the baremetal OCRAM image.
 * Reserve 0x00000000–0x31FFF for ATF+mailbox.  Baremetal OCRAM data
 * must start at 0x32000 or later (see linker_ocram.ld).
 */
#define AGILEX72_SIMICS_MBOX_MSG_BUF_BASE				(0x00020000)
#define AGILEX72_SIMICS_MBOX_RSP_BUF_BASE				(0x00030000)

static struct mailbox_message *message_buffer =
		(struct mailbox_message *)AGILEX72_SIMICS_MBOX_MSG_BUF_BASE;
static struct mailbox_message *response_buffer =
		(struct mailbox_message *)AGILEX72_SIMICS_MBOX_RSP_BUF_BASE;
#else
#define MAILBOX_MAX_QUEUE_SIZE				11
#define SHARED_MEM_BASE						0x0800F4000L
#define SHARED_MEM_SIZE						0x6000
#define MAILBOX_MSG_QUEUE_BASE				(SHARED_MEM_BASE)
#define MAILBOX_RSP_QUEUE_BASE				(SHARED_MEM_BASE + SHARED_MEM_SIZE)
#define MAILBOX_MSG_MAX_LENGTH				512
#define MAILBOX_RESP_MAX_LENGTH				512

/* Shared buffers (volatile) */
struct __aligned(64) mailbox_message {
	uint32_t msg_header;
	uint32_t msg_data[MAILBOX_MSG_MAX_LENGTH];
	uint8_t _pad[64 - ((sizeof(uint32_t) + MAILBOX_MSG_MAX_LENGTH * sizeof(uint32_t)) % 64)];
};

struct __aligned(64) mailbox_response {
	uint32_t msg_header;
	uint32_t msg_data[MAILBOX_RESP_MAX_LENGTH];
	uint8_t _pad[64 - ((sizeof(uint32_t) + MAILBOX_RESP_MAX_LENGTH * sizeof(uint32_t)) % 64)];
};

volatile struct mailbox_message *message_buffer =
	(struct mailbox_message *)MAILBOX_MSG_QUEUE_BASE;
volatile struct mailbox_response *response_buffer =
	(struct mailbox_response *)MAILBOX_RSP_QUEUE_BASE;

#endif

static spinlock_t mbox_db_lock;		/* Mailbox service data base lock */
static spinlock_t mbox_write_lock;	/* Hardware mailbox FIFO write lock */
static spinlock_t mbox_read_lock;	/* Hardware mailbox FIFO read lock */

static mailbox_service_t mbox_svc;	/* Mailbox service data base */

/* -------------------------------------------------------------------------
 * Initialization: clears DB, inits rings and pre-allocates S2C descriptors.
 * -------------------------------------------------------------------------
 */
int mcsr_mailbox_init(void)
{
	int status = 0;
	uint32_t count = 0U;
	struct mcsr_s2c_desc resp_desc;

	/* Clear service DB queues and all bitmap state */
	for (count = 0U; count < MBOX_SVC_RESP_QUEUE_SIZE; count++) {
		mbox_svc.resp_queue[count].flags = 0U;
	}

	for (count = 0U; count < MBOX_SVC_CMD_QUEUE_SIZE; count++) {
		mbox_svc.cmd_queue[count].flags = 0U;
	}

	memset(mbox_svc.received_bitmap, 0, sizeof(mbox_svc.received_bitmap));
	memset(mbox_svc.interrupt_bitmap, 0, sizeof(mbox_svc.interrupt_bitmap));

	mbox_svc.curr_di = -1;

	/* Initialize low-level rings */
	mcsr_c2s_init(); /* c2s init (resets hardware) */
	mcsr_s2c_init();

	/* Pre-populate S2C response descriptors so SDM can send responses */
	for (int ring_buf_index = 0; ring_buf_index < MAILBOX_MAX_QUEUE_SIZE; ring_buf_index++) {
		mcsr_s2c_desc_set(&resp_desc,
				  (uint64_t)(&response_buffer[ring_buf_index]),
				  MAILBOX_RESP_MAX_LENGTH + 1);
		mcsr_s2c_push(&resp_desc, ring_buf_index);
	}

	/* Update CIN to be COUT - 1, full buffer state */
	uint32_t snapshot_cout = mcsr_s2c_get_current_cout();

	mcsr_s2c_update_cin(mcsr_s2c_get_previous_cout(snapshot_cout));

	return status;
}

/*
 * Private helpers - descriptor DB management
 * These were present in your original code; cleaned to ATF style.
 */
static int32_t mcsr_mailbox_get_free_resp_desc(void)
{
	int32_t rsp_desc_index = -1;
	uint32_t try = 0U;

	spin_lock(&mbox_db_lock);
	for (try = 0U; try < MBOX_SVC_RESP_QUEUE_SIZE; try++) {
		if (!(mbox_svc.resp_queue[try].flags & FLAG_SDM_RESPONSE_IS_USED)) {
			rsp_desc_index = (int32_t)try;
			break;
		}
	}
	spin_unlock(&mbox_db_lock);

	return rsp_desc_index;
}

static sdm_command_t *mcsr_mailbox_get_cmd_desc(uint8_t client_id, uint8_t job_id,
						bool ignore_job_id)
{
	sdm_command_t *cmd_desc = NULL;
	uint32_t count = 0U;

	spin_lock(&mbox_db_lock);
	for (count = 0U; count < MBOX_SVC_CMD_QUEUE_SIZE; count++) {
		if ((mbox_svc.cmd_queue[count].client_id == client_id) &&
		    ((mbox_svc.cmd_queue[count].job_id == job_id) || (ignore_job_id == true)) &&
		    (mbox_svc.cmd_queue[count].flags & MBOX_SVC_CMD_IS_USED)) {
			cmd_desc = &(mbox_svc.cmd_queue[count]);
			break;
		}
	}
	spin_unlock(&mbox_db_lock);

	return cmd_desc;
}

static sdm_response_t *mcsr_mailbox_get_resp_desc(uint8_t client_id, uint8_t job_id, uint8_t *index)
{
	sdm_response_t *resp_desc = NULL;
	uint8_t transaction_id = MBOX_GET_TRANS_ID(client_id, job_id);
	uint32_t count = 0U;

	spin_lock(&mbox_db_lock);
	if ((mbox_svc.received_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &
		(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE))) == 0ULL) {
		spin_unlock(&mbox_db_lock);
		VERBOSE("MBOX: Response bitmap not set for cid %d, jid %d\n", client_id, job_id);
		return NULL;
	}

	for (count = 0U; count < MBOX_SVC_RESP_QUEUE_SIZE; count++) {
		if (mbox_svc.resp_queue[count].flags & FLAG_SDM_RESPONSE_IS_VALID) {
			if ((mbox_svc.resp_queue[count].client_id == client_id) &&
			    (mbox_svc.resp_queue[count].job_id == job_id)) {
				*index = (uint8_t)count;
				mbox_svc.received_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &=
					~(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE));
				mbox_svc.interrupt_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &=
					~(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE));
				resp_desc = &(mbox_svc.resp_queue[count]);
				break;
			}
		}
	}

	spin_unlock(&mbox_db_lock);
	return resp_desc;
}

static inline void mcsr_mailbox_free_cmd_desc(sdm_command_t *cmd_desc)
{
	if (cmd_desc == NULL) {
		return;
	}

	spin_lock(&mbox_db_lock);
	memset((void *)cmd_desc, 0, sizeof(sdm_command_t));
	spin_unlock(&mbox_db_lock);
}

static inline void mcsr_mailbox_free_resp_desc(uint8_t index)
{
	if (index >= MBOX_SVC_RESP_QUEUE_SIZE) {
		return;
	}

	spin_lock(&mbox_db_lock);
	memset((void *)&mbox_svc.resp_queue[index], 0, sizeof(sdm_response_t));
	spin_unlock(&mbox_db_lock);
}

static sdm_command_t *mcsr_mailbox_get_free_cmd_desc(void)
{
	sdm_command_t *cmd_desc = NULL;
	int cmd_index = 0;

	spin_lock(&mbox_db_lock);
	for (cmd_index = 0; cmd_index < MBOX_SVC_CMD_QUEUE_SIZE; cmd_index++) {
		if (!(mbox_svc.cmd_queue[cmd_index].flags & MBOX_SVC_CMD_IS_USED)) {
			mbox_svc.cmd_queue[cmd_index].flags |= MBOX_SVC_CMD_IS_USED;
			cmd_desc = &(mbox_svc.cmd_queue[cmd_index]);
			break;
		}
	}
	spin_unlock(&mbox_db_lock);

	return cmd_desc;
}

static sdm_response_t *mcsr_mailbox_get_resp_desc_cid(uint8_t client_id, uint8_t *index)
{
		uint32_t count = 0U;

	spin_lock(&mbox_db_lock);
	for (count = 0U; count < MBOX_SVC_RESP_QUEUE_SIZE; count++) {
		if ((mbox_svc.resp_queue[count].client_id == client_id) &&
		    (mbox_svc.resp_queue[count].flags & FLAG_SDM_RESPONSE_IS_VALID)) {
			*index = (uint8_t)count;
			uint8_t job_id = mbox_svc.resp_queue[count].job_id;
			uint8_t transaction_id = MBOX_GET_TRANS_ID(client_id, job_id);

			mbox_svc.received_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &=
				~(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE));
			mbox_svc.interrupt_bitmap[transaction_id / MBOX_TID_BITMAP_SIZE] &=
				~(1ULL << (transaction_id % MBOX_TID_BITMAP_SIZE));
			spin_unlock(&mbox_db_lock);
			return &(mbox_svc.resp_queue[count]);
		}
	}
	spin_unlock(&mbox_db_lock);

	VERBOSE("MBOX: Response descriptor not found for cid %d\n", client_id);
	return NULL;
}

/*
 * Low-level S2C response handler - processes SDM responses in LIFO order:
 *	- Takes snapshot of CIN (head/write ptr) and COUT (tail/read ptr)
 *	- Reads descriptors from S2C ring backwards from COUT to CIN+1 (LIFO)
 *	- Copies response header and payload into service DB response queue entry
 *	- Sets received bitmap to notify waiting clients
 *	- Recycles each descriptor back to S2C ring via mcsr_s2c_push()
 *	- Updates CIN to snapshot_cout - 1 to reflect processed descriptors
 *
 * Returns:
 *	MBOX_RET_OK on success, MBOX_RET_ERROR if no free response descriptors
 */
static int mcsr_mailbox_response_handler(void)
{
	int status = MBOX_RET_ERROR;
	int di = -1;
	struct mcsr_s2c_desc resp_desc;
	int i = 0;

	spin_lock(&mbox_read_lock);

	uint32_t snapshot_cin = mcsr_s2c_get_current_cin();
	uint32_t snapshot_cout = mcsr_s2c_get_current_cout();

	if (!mcsr_s2c_sw_is_full(snapshot_cin, snapshot_cout)) {
		/* Start from the last response written by SDM (COUT - 1) */
		uint32_t get_index = mcsr_s2c_get_previous_cout(snapshot_cout);
		uint8_t processed_resp_count = 0;

		/* Process backward from COUT until we reach CIN (all responses consumed) */
		while (get_index != snapshot_cin) {

			uint32_t response_header = response_buffer[get_index].msg_header;
			uint8_t trans_id;

			di = mcsr_mailbox_get_free_resp_desc();
			if (di == -1) {
				status = MBOX_RET_ERROR;
				break;
			}

			/* Populate the local response queue entry */
			mbox_svc.resp_queue[di].header = response_header;
			mbox_svc.resp_queue[di].client_id =
				MSG_HEADER_GET_CHANNEL_NUM(response_header);
			mbox_svc.resp_queue[di].job_id =
				MSG_HEADER_GET_SEQUENCE_ID(response_header);
			mbox_svc.resp_queue[di].resp_len =
				MSG_HEADER_GET_LENGTH(response_header);
			mbox_svc.resp_queue[di].flags |=
				(FLAG_SDM_RESPONSE_IS_USED | FLAG_SDM_RESPONSE_IS_IN_PROGRESS);
			mbox_svc.resp_queue[di].err_code = MSG_HEADER_GET_CMD_RESP(response_header);
			trans_id = MBOX_GET_TRANS_ID(mbox_svc.resp_queue[di].client_id,
						     mbox_svc.resp_queue[di].job_id);

			VERBOSE("MBOX: Resp Hdr: cid %d, jid %d, len %d, err_code 0x%x\n",
				mbox_svc.resp_queue[di].client_id,
				mbox_svc.resp_queue[di].job_id,
				mbox_svc.resp_queue[di].resp_len,
				mbox_svc.resp_queue[di].err_code);

			/* If there is payload, copy to client buffer or internal buffer */
			if (mbox_svc.resp_queue[di].resp_len > 0U) {
				uint32_t *read_buff;
				sdm_command_t *cmd_desc =
					mcsr_mailbox_get_cmd_desc(mbox_svc.resp_queue[di].client_id,
								  mbox_svc.resp_queue[di].job_id,
								  false);

				if ((cmd_desc != NULL) && (cmd_desc->cb_args != NULL)) {
					read_buff = cmd_desc->cb_args;
				} else {
					read_buff = (uint32_t *)mbox_svc.resp_queue[di].resp_data;
				}

				for (i = 0; i < (int)mbox_svc.resp_queue[di].resp_len; i++) {
					read_buff[i] = response_buffer[get_index].msg_data[i];
				}
			}

			mbox_svc.resp_queue[di].flags |= FLAG_SDM_RESPONSE_IS_VALID;

			/* Mark transaction-id as received (with or without payload) */
			spin_lock(&mbox_db_lock);
			mbox_svc.received_bitmap[trans_id / MBOX_TID_BITMAP_SIZE] |=
				(1ULL << (trans_id % MBOX_TID_BITMAP_SIZE));
			spin_unlock(&mbox_db_lock);

			/* Recycle S2C descriptor back to SDM */
			mcsr_s2c_desc_set(&resp_desc, (uint64_t)(&response_buffer[get_index]),
					  MAILBOX_RESP_MAX_LENGTH + 1);
			mcsr_s2c_push(&resp_desc, get_index);

			/* LIFO read */
			get_index = mcsr_s2c_get_previous_cout(get_index);
			processed_resp_count++;
		}
		/* Update the hardware CIN after processing all descriptors. */
		snapshot_cin = mcsr_s2c_get_previous_cout(snapshot_cout);
		mcsr_s2c_update_cin(snapshot_cin);
		status = MBOX_RET_OK;
		VERBOSE("MBOX: Processed %d responses from S2C ring\n", processed_resp_count);
	}
	spin_unlock(&mbox_read_lock);
	return status;
}

/*
 * Low-level C2S send helper (writes message_buffer and pushes descriptor).
 * Preserves original semantics but enforces ATF-style scoping.
 */
static int mcsr_c2s_mailbox_send(uint32_t header_cmd, uint32_t *args, unsigned int len)
{
	int status = 0;
	uint32_t i = 0U;
	uint32_t current_tx_desc_index = 0U;
	struct mcsr_c2s_desc current_desc;

	spin_lock(&mbox_write_lock);

	if ((mcsr_c2s_is_full() == true) || (len > MAILBOX_MSG_MAX_LENGTH) ||
	    ((args == NULL) && (len > 0))) {
		status = -1;
	} else {
		current_tx_desc_index = mcsr_c2s_get_current_cin();
		message_buffer[current_tx_desc_index].msg_header = header_cmd;

		if ((args != NULL) && (len > 0)) {
			for (i = 0U; i < len; i++) {
				message_buffer[current_tx_desc_index].msg_data[i] = args[i];
			}
		}

		/* Descriptor size = actual words in message (header + payload).
		 * Using the full MAILBOX_MSG_MAX_LENGTH triggers a large DMA
		 * transfer in Simics that reads beyond the valid data.
		 */
		mcsr_c2s_desc_set(&current_desc, (uint64_t)(&message_buffer[current_tx_desc_index]),
				  len + 1U);
		mcsr_c2s_push(&current_desc);
		status = 0;
	}

	spin_unlock(&mbox_write_lock);
	return status;
}

/*
 * Internal wait for response utility (retains original behaviour)
 * - polls mailbox handler until response found or timeout
 */
static int mcsr_mailbox_wait_for_response_internal(uint8_t client_id, uint32_t job_id,
						   bool ignore_job_id, uint32_t *response,
						   unsigned int *resp_len)
{
	int status = MBOX_TIMEOUT;
	sdm_response_t *resp_desc = NULL;
	sdm_command_t *cmd_desc = NULL;
	uint8_t di = 0U;
	unsigned int polling_counter = 255U;

	cmd_desc = mcsr_mailbox_get_cmd_desc(client_id, job_id, ignore_job_id);

	while (polling_counter > 0U) {
		mcsr_mailbox_response_handler();
		resp_desc = mcsr_mailbox_get_resp_desc(client_id, job_id, &di);

		if (resp_desc != NULL) {
			NOTICE("%s: Resp received for cid %d, jid %d\n",
			       __func__, resp_desc->client_id, resp_desc->job_id);

			uint16_t header = (uint16_t)resp_desc->header;

			if (resp_len != NULL) {
				*resp_len = resp_desc->rcvd_resp_len;
			}

			mcsr_mailbox_free_resp_desc(di);
			mcsr_mailbox_free_cmd_desc(cmd_desc);

			if (MBOX_RESP_ERR(header) > 0U) {
				INFO("%s: SDM err code: 0x%x\n", __func__, MBOX_RESP_ERR(header));
				status = -MBOX_RESP_ERR(header);
			} else {
				status = MBOX_RET_OK;
			}

			return status;
		}

		polling_counter--;
	}

	return MBOX_TIMEOUT;
}

/*
 * Unified high-level mailbox executor
 *
 * This is a pure "send (and optionally wait)" primitive.  It does NOT own
 * any command-descriptor state - wrappers that need a cmd_desc (e.g. for
 * async callbacks or V1 async-read tracking) must allocate it themselves
 * BEFORE calling exec and free it on failure.  Keeping ownership in the
 * wrapper makes get_free/free pairs grep-able and prevents the leaks that
 * arise when both layers try to allocate.
 *
 * flags:
 *	- MAILBOX_FLAG_ASYNC      : return immediately after send (no wait)
 *	- MAILBOX_FLAG_IGNORE_JOB : sync wait matches on client_id only
 *
 * NOTE: args points to a uint32_t word array; resp_buf is only used on the
 * synchronous path.
 */
#define MAILBOX_FLAG_ASYNC			BIT(0)
#define MAILBOX_FLAG_IGNORE_JOB			BIT(1)

int mcsr_mailbox_exec(uint8_t client_id, uint8_t job_id, uint32_t cmd_header,
		      uint32_t *args, uint32_t args_len,
		      uint32_t *resp_buf, unsigned int *resp_len,
		      uint32_t flags)
{
	int status;
	bool is_async = ((flags & MAILBOX_FLAG_ASYNC) != 0U);
	bool ignore_job = ((flags & MAILBOX_FLAG_IGNORE_JOB) != 0U);

	status = mcsr_c2s_mailbox_send(cmd_header, args, args_len);
	if (status < 0) {
		return status;
	}

	if (is_async) {
		return MBOX_RET_OK;
	}

	return mcsr_mailbox_wait_for_response_internal(client_id, job_id,
						       ignore_job,
						       resp_buf, resp_len);
}

/*
 * Backwards-compatible public APIs (wrappers)
 * These call mailbox_exec() appropriately to preserve existing semantics.
 */

int mcsr_mailbox_send_cmd(uint32_t job_id, uint32_t cmd, uint32_t *args,
			  unsigned int len, uint32_t urgent, uint32_t *response,
			  unsigned int *resp_len)
{
	int status = 0;
	uint32_t header = 0U;

	(void)urgent; /* preserved for compatibility; not used in this wrapper */

	header = (uint32_t)(MBOX_CLIENT_ID_CMD(MBOX_ATF_CLIENT_ID) |
						MBOX_JOB_ID_CMD((uint32_t)job_id) |
						MBOX_CMD_LEN_CMD(len) |
						cmd);

	status = mcsr_mailbox_exec(MBOX_ATF_CLIENT_ID, (uint8_t)job_id, header,
				   args, len, response, resp_len, 0U);

	return status;
}

int mcsr_mailbox_send_cmd_async(uint32_t *job_id, uint32_t cmd, uint32_t *args,
				unsigned int len, unsigned int indirect)
{
	sdm_command_t *cmd_desc;
	uint32_t header;
	int status;

	/*
	 * V1 async needs a cmd_desc so mcsr_mailbox_read_response_async()
	 * can later find a tracking entry for this client (ignore_job_id).
	 */
	cmd_desc = mcsr_mailbox_get_free_cmd_desc();
	if (cmd_desc == NULL) {
		return MBOX_BUFFER_FULL;
	}
	cmd_desc->client_id = MBOX_ATF_CLIENT_ID;
	cmd_desc->job_id = (uint8_t)*job_id;

	header = (uint32_t)(MBOX_CLIENT_ID_CMD(MBOX_ATF_CLIENT_ID) |
					MBOX_JOB_ID_CMD(*job_id) |
					MBOX_CMD_LEN_CMD(len) |
					MBOX_INDIRECT(indirect) |
					cmd);

	status = mcsr_mailbox_exec(MBOX_ATF_CLIENT_ID, (uint8_t)*job_id, header,
				   args, len, NULL, NULL, MAILBOX_FLAG_ASYNC);
	if (status < 0) {
		mcsr_mailbox_free_cmd_desc(cmd_desc);
		return status;
	}

	/* increment job id for legacy V1 behaviour */
	*job_id = (*job_id + 1U) % MBOX_MAX_IND_JOB_ID;
	return MBOX_RET_OK;
}

int mcsr_mailbox_send_cmd_async_ext(uint32_t header_cmd, uint32_t *args, unsigned int len)
{
	/*
	 * Raw V2 write path: fire-and-forget.  No async-read consumer exists
	 * for this entry point, so we must NOT allocate a cmd_desc - doing so
	 * would leak one slot per call (header_cmd already encodes the real
	 * client/job in its bitfield, but exec would record {ATF, 0}).
	 */
	return mcsr_mailbox_exec(MBOX_ATF_CLIENT_ID, 0, header_cmd, args, len,
				 NULL, NULL, MAILBOX_FLAG_ASYNC);
}

/* V3 async wrapper that supports callback and cb_args via cmd_desc */
int mcsr_mailbox_send_cmd_async_v3(uint8_t client_id, uint8_t job_id, uint32_t cmd,
				   uint32_t *args, uint32_t args_len, uint8_t cmd_flag,
				   sdm_command_callback cb, uint32_t *cb_args,
				   uint32_t cb_args_len)
{
	sdm_command_t *cmd_desc = NULL;
	uint32_t header = 0U;
	int status = 0;

	cmd_desc = mcsr_mailbox_get_free_cmd_desc();
	if (cmd_desc == NULL) {
		return MBOX_BUFFER_FULL;
	}

	/* record callback and arguments in descriptor */
	cmd_desc->client_id = client_id;
	cmd_desc->job_id = job_id;
	cmd_desc->cb = cb;
	cmd_desc->cb_args = cb_args;
	cmd_desc->cb_args_len = cb_args_len;

	header = (uint32_t)(MBOX_CLIENT_ID_CMD(client_id) |
						MBOX_JOB_ID_CMD(job_id) |
						MBOX_CMD_LEN_CMD(args_len) |
						(uint32_t)cmd);

	status = mcsr_mailbox_exec(client_id, job_id, header, args, args_len,
				   NULL, NULL, MAILBOX_FLAG_ASYNC);
	if (status != MBOX_RET_OK) {
		mcsr_mailbox_free_cmd_desc(cmd_desc);
	}

	(void)cmd_flag;
	return status;
}

/*
 * V1 legacy async receive.
 * This preserves original semantics: it assigns the caller buffer as cmd_desc->cb_args,
 * invokes the response handler, and returns the first available response for the fixed client id.
 */
int mcsr_mailbox_read_response_async(unsigned int *job_id, uint32_t *header,
				     uint32_t *response, unsigned int *resp_len,
				     uint8_t ignore_client_id)
{
	uint8_t client_id = MBOX_ATF_CLIENT_ID;
	uint8_t di = 0U;
	int status = MBOX_RET_OK;
	sdm_response_t *resp_desc = NULL;
	sdm_command_t *cmd_desc = NULL;

	(void)ignore_client_id;

	cmd_desc = mcsr_mailbox_get_cmd_desc(client_id, 0, true);
	if (cmd_desc == NULL) {
		return MBOX_NO_RESPONSE;
	}

	cmd_desc->cb_args = response;

	/* Ensure S2C queue is handled */
	mcsr_mailbox_response_handler();

	resp_desc = mcsr_mailbox_get_resp_desc_cid(client_id, &di);
	if (resp_desc == NULL) {
		mcsr_mailbox_free_cmd_desc(cmd_desc);
		if (resp_len != NULL) {
			*resp_len = 0U;
		}
		return MBOX_NO_RESPONSE;
	}

	*job_id = resp_desc->job_id;
	if (resp_len != NULL) {
		*resp_len = resp_desc->rcvd_resp_len;
	}
	if (header != NULL) {
		*header = resp_desc->header;
	}

	if (MBOX_RESP_ERR(resp_desc->header) > 0U) {
		INFO("MBOX: Error in async response: %x\n", resp_desc->header);
		status = -MBOX_RESP_ERR(resp_desc->header);
	}

	mcsr_mailbox_free_resp_desc(di);
	mcsr_mailbox_free_cmd_desc(cmd_desc);

	return status;
}

/*
 * Blocking read (legacy): waits for any response for ATF client (ignore job id).
 */
int mcsr_mailbox_read_response(unsigned int *job_id, uint32_t *response, unsigned int *resp_len)
{
	int status = MBOX_RET_OK;

	(void)job_id;
	status = mcsr_mailbox_wait_for_response_internal(MBOX_ATF_CLIENT_ID, 0U, true,
							 response, resp_len);
	return status;
}

/* Placeholder: not required for MCSR but retained for compatibility */
void mcsr_mailbox_set_int(uint32_t interrupt)
{
	(void)interrupt;
}

/* Placeholder: reset S2C head/tail; not implemented here */
void mcsr_mailbox_clear_response(void)
{
	/* NOP: platform-specific reset may be required after FPGA reconfig */
}

/*
 * V3 response poll: callback-invoking poll
 * returns MBOX_RET_OK if processed; MBOX_NO_RESPONSE otherwise.
 */
int mcsr_mailbox_response_poll_v3(uint8_t client_id, uint8_t job_id,
				  uint64_t *ret_args, uint32_t *ret_args_len)
{
	sdm_command_t *cmd_desc = NULL;
	sdm_response_t *resp_desc = NULL;
	uint8_t di = 0U;
	int status = MBOX_NO_RESPONSE;

	/* drain S2C and populate service queue */
	mcsr_mailbox_response_handler();

	resp_desc = mcsr_mailbox_get_resp_desc(client_id, job_id, &di);
	if (resp_desc == NULL) {
		return MBOX_NO_RESPONSE;
	}

	if (resp_desc != NULL) {
		VERBOSE("MBOX: Resp found for cid %d, jid %d\n", client_id, job_id);

		cmd_desc = mcsr_mailbox_get_cmd_desc(client_id, job_id, false);
		if ((cmd_desc != NULL) && (cmd_desc->cb != NULL)) {
			uint32_t args_len = 0U;

			args_len = (uint32_t)cmd_desc->cb(resp_desc, cmd_desc, ret_args);
			if (ret_args_len != NULL) {
				*ret_args_len = args_len;
			}
		} else if (ret_args_len != NULL) {
			*ret_args_len = 0U;
		}

		if (cmd_desc != NULL) {
			mcsr_mailbox_free_cmd_desc(cmd_desc);
		}
		mcsr_mailbox_free_resp_desc(di);

		status = MBOX_RET_OK;
	}

	return status;
}

/*
 * IRQ-based poll: called from IRQ handler context (caller provides bitmaps)
 */
int mcsr_mailbox_response_poll_on_intr_v3(uint8_t *client_id, uint8_t *job_id, uint64_t *bitmap)
{
	uint32_t i = 0U;
	int status = MBOX_RET_OK;

	status = mcsr_mailbox_response_handler();
	if (status != MBOX_RET_OK) {
		return status;
	}

	status = MBOX_NO_RESPONSE;
	spin_lock(&mbox_db_lock);
	for (i = 0U; i < MBOX_MAX_TIDS_BITMAP; i++) {
		/*
		 * Return received_bitmap directly rather than XOR-with-interrupt.
		 * The XOR approach causes stale bits: if interrupt_bitmap[i] was
		 * set equal to received_bitmap[i] on a previous poll, and
		 * received_bitmap[i] was subsequently cleared by response_poll_v3,
		 * then interrupt[i] != received[i] produces a spurious "new" bit
		 * that blocks real new responses from ever being visible (they
		 * cancel out on the next call).  In pure-polling mode, always
		 * returning received_bitmap is correct: BM sees exactly which
		 * trans-ids have arrived, and they are cleared when consumed.
		 */
#if MBOX_POLLING_MODE
		bitmap[i] = mbox_svc.received_bitmap[i];
#else
		bitmap[i] = mbox_svc.interrupt_bitmap[i] ^ mbox_svc.received_bitmap[i];
#endif

		if ((bitmap[i] != 0ULL) && (status == MBOX_NO_RESPONSE)) {
			status = MBOX_RET_OK;
		}
		mbox_svc.interrupt_bitmap[i] = 0U;
	}
	spin_unlock(&mbox_db_lock);

	return status;
}

/* -------------------------------------------------------------------------
 * MCSR backend vtable
 * -------------------------------------------------------------------------
 */
struct mailbox_backend mcsr_backend = {
	.init = mcsr_mailbox_init,
	.get_free_resp_desc = mcsr_mailbox_get_free_resp_desc,
	.get_cmd_desc = mcsr_mailbox_get_cmd_desc,
	.get_resp_desc = mcsr_mailbox_get_resp_desc,
	.free_cmd_desc = mcsr_mailbox_free_cmd_desc,
	.free_resp_desc = mcsr_mailbox_free_resp_desc,
	.get_free_cmd_desc = mcsr_mailbox_get_free_cmd_desc,
	.get_resp_desc_cid = mcsr_mailbox_get_resp_desc_cid,
	.response_handler = mcsr_mailbox_response_handler,
	.c2s_mailbox_send = mcsr_c2s_mailbox_send,
	.wait_for_response_internal = mcsr_mailbox_wait_for_response_internal,
	.mailbox_exec = mcsr_mailbox_exec,
	.send = mcsr_mailbox_send_cmd,
	.send_async = mcsr_mailbox_send_cmd_async,
	.send_async_ext = mcsr_mailbox_send_cmd_async_ext,
	.send_async_v3 = mcsr_mailbox_send_cmd_async_v3,
	.read_response_async = mcsr_mailbox_read_response_async,
	.read_response = mcsr_mailbox_read_response,
	.set_int = mcsr_mailbox_set_int,
	.clear_response = mcsr_mailbox_clear_response,
	.response_poll_v3 = mcsr_mailbox_response_poll_v3,
	.response_poll_on_intr_v3 = mcsr_mailbox_response_poll_on_intr_v3
};

/* Expose the MCSR backend to the common core dispatcher. */
struct mailbox_backend *mailbox_backend_get(enum mailbox_backend_type type)
{
	assert(type == MAILBOX_BACKEND_MCSR);
	(void)type;

	return &mcsr_backend;
}
