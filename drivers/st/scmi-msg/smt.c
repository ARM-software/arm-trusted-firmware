// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2020, Linaro Limited
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <drivers/st/scmi-msg.h>
#include <drivers/st/scmi.h>
#include <lib/cassert.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <lib/utils.h>
#include <plat/common/platform.h>

#include "common.h"

/* Legacy SMT/SCMI messages are 128 bytes at most including SMT header */
#define SCMI_PLAYLOAD_MAX		92U
#define SCMI_PLAYLOAD_U32_MAX		(SCMI_PLAYLOAD_MAX / sizeof(uint32_t))

/**
 * struct smt_header - SMT formatted header for SMT base shared memory transfer
 *
 * @status: Bit flags, see SMT_STATUS_*
 * @flags: Bit flags, see SMT_FLAG_*
 * @length: Byte size of message payload (variable) + ::message_header (32bit)
 * payload: SCMI message payload data
 */
struct smt_header {
	uint32_t reserved0;
	uint32_t status;
	uint64_t reserved1;
	uint32_t flags;
	uint32_t length; /* message_header + payload */
	uint32_t message_header;
	uint32_t payload[];
};

CASSERT(SCMI_PLAYLOAD_MAX + sizeof(struct smt_header) <= SMT_BUF_SLOT_SIZE,
	assert_scmi_message_max_length_fits_in_smt_buffer_slot);

/* Flag set in smt_header::status when SMT does not contain pending message */
#define SMT_STATUS_FREE			BIT(0)
/* Flag set in smt_header::status when SMT reports an error */
#define SMT_STATUS_ERROR		BIT(1)

/* Flag set in smt_header::flags when SMT uses interrupts */
#define SMT_FLAG_INTR_ENABLED		BIT(1)

/* Bit fields packed in smt_header::message_header */
#define SMT_MSG_ID_MASK			GENMASK_32(7, 0)
#define SMT_HDR_MSG_ID(_hdr)		((_hdr) & SMT_MSG_ID_MASK)

#define SMT_MSG_TYPE_MASK		GENMASK_32(9, 8)
#define SMT_HDR_TYPE_ID(_hdr)		(((_hdr) & SMT_MSG_TYPE_MASK) >> 8)

#define SMT_MSG_PROT_ID_MASK		GENMASK_32(17, 10)
#define SMT_HDR_PROT_ID(_hdr)		(((_hdr) & SMT_MSG_PROT_ID_MASK) >> 10)

/*
 * Provision input message payload buffers for fastcall SMC context entries
 * and for interrupt context execution entries.
 */
static uint32_t fast_smc_payload[PLATFORM_CORE_COUNT][SCMI_PLAYLOAD_U32_MAX];
static uint32_t interrupt_payload[PLATFORM_CORE_COUNT][SCMI_PLAYLOAD_U32_MAX];

/* SMP protection on channel access */
static struct spinlock smt_channels_lock;

/* If channel is not busy, set busy and return true, otherwise return false */
static bool channel_set_busy(struct scmi_msg_channel *chan)
{
	bool channel_is_busy;

	spin_lock(&smt_channels_lock);

	channel_is_busy = chan->busy;

	if (!channel_is_busy) {
		chan->busy = true;
	}

	spin_unlock(&smt_channels_lock);

	return !channel_is_busy;
}

static void channel_release_busy(struct scmi_msg_channel *chan)
{
	chan->busy = false;
}

static struct smt_header *channel_to_smt_hdr(struct scmi_msg_channel *chan)
{
	return (struct smt_header *)chan->shm_addr;
}

/*
 * Creates a SCMI message instance in secure memory and pushes it in the SCMI
 * message drivers. Message structure contains SCMI protocol meta-data and
 * references to input payload in secure memory and output message buffer
 * in shared memory.
 */
static void scmi_proccess_smt(unsigned int agent_id, uint32_t *payload_buf)
{
	struct scmi_msg_channel *chan;
	struct smt_header *smt_hdr;
	size_t in_payload_size;
	uint32_t smt_status;
	struct scmi_msg msg;
	bool error = true;

	chan = plat_scmi_get_channel(agent_id);
	if (chan == NULL) {
		return;
	}

	smt_hdr = channel_to_smt_hdr(chan);
	assert(smt_hdr);

	smt_status = __atomic_load_n(&smt_hdr->status, __ATOMIC_RELAXED);

	if (!channel_set_busy(chan)) {
		VERBOSE("SCMI channel %u busy", agent_id);
		goto out;
	}

	in_payload_size = __atomic_load_n(&smt_hdr->length, __ATOMIC_RELAXED) -
			  sizeof(smt_hdr->message_header);

	if (in_payload_size > SCMI_PLAYLOAD_MAX) {
		VERBOSE("SCMI payload too big %u", in_payload_size);
		goto out;
	}

	if ((smt_status & (SMT_STATUS_ERROR | SMT_STATUS_FREE)) != 0U) {
		VERBOSE("SCMI channel bad status 0x%x",
			smt_hdr->status & (SMT_STATUS_ERROR | SMT_STATUS_FREE));
		goto out;
	}

	/* Fill message */
	zeromem(&msg, sizeof(msg));
	msg.in = (char *)payload_buf;
	msg.in_size = in_payload_size;
	msg.out = (char *)smt_hdr->payload;
	msg.out_size = chan->shm_size - sizeof(*smt_hdr);

	assert((msg.out != NULL) && (msg.out_size >= sizeof(int32_t)));

	/* Here the payload is copied in secure memory */
	memcpy(msg.in, smt_hdr->payload, in_payload_size);

	msg.protocol_id = SMT_HDR_PROT_ID(smt_hdr->message_header);
	msg.message_id = SMT_HDR_MSG_ID(smt_hdr->message_header);
	msg.agent_id = agent_id;

	scmi_process_message(&msg);

	/* Update message length with the length of the response message */
	smt_hdr->length = msg.out_size_out + sizeof(smt_hdr->message_header);

	channel_release_busy(chan);
	error = false;

out:
	if (error) {
		VERBOSE("SCMI error");
		smt_hdr->status |= SMT_STATUS_ERROR | SMT_STATUS_FREE;
	} else {
		smt_hdr->status |= SMT_STATUS_FREE;
	}
}

void scmi_smt_fastcall_smc_entry(unsigned int agent_id)
{
	scmi_proccess_smt(agent_id,
			  fast_smc_payload[plat_my_core_pos()]);
}

void scmi_smt_interrupt_entry(unsigned int agent_id)
{
	scmi_proccess_smt(agent_id,
			  interrupt_payload[plat_my_core_pos()]);
}

/* Init a SMT header for a shared memory buffer: state it a free/no-error */
void scmi_smt_init_agent_channel(struct scmi_msg_channel *chan)
{
	if (chan != NULL) {
		struct smt_header *smt_header = channel_to_smt_hdr(chan);

		if (smt_header != NULL) {
			memset(smt_header, 0, sizeof(*smt_header));
			smt_header->status = SMT_STATUS_FREE;

			return;
		}
	}

	panic();
}
