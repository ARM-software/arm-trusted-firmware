/*
 * Copyright (c) 2017-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>
#include <drivers/delay_timer.h>

#include "scmi_private.h"

#if HW_ASSISTED_COHERENCY
#define scmi_lock_init(lock)
#define scmi_lock_get(lock)		spin_lock(lock)
#define scmi_lock_release(lock)		spin_unlock(lock)
#else
#define scmi_lock_init(lock)		bakery_lock_init(lock)
#define scmi_lock_get(lock)		bakery_lock_get(lock)
#define scmi_lock_release(lock)		bakery_lock_release(lock)
#endif


/*
 * Private helper function to get exclusive access to SCMI channel.
 */
void scmi_get_channel(scmi_channel_t *ch)
{
	assert(ch->lock);
	scmi_lock_get(ch->lock);

	/* Make sure any previous command has finished */
	assert(SCMI_IS_CHANNEL_FREE(
			((mailbox_mem_t *)(ch->info->scmi_mbx_mem))->status));
}

/*
 * Private helper function to transfer ownership of channel from AP to SCP.
 */
void scmi_send_sync_command(scmi_channel_t *ch)
{
	mailbox_mem_t *mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	uint64_t timeout;

	SCMI_MARK_CHANNEL_BUSY(mbx_mem->status);

	/*
	 * Ensure that any write to the SCMI payload area is seen by SCP before
	 * we write to the doorbell register. If these 2 writes were reordered
	 * by the CPU then SCP would read stale payload data
	 */
	dmbst();

	ch->info->ring_doorbell(ch->info);
	/*
	 * Ensure that the write to the doorbell register is ordered prior to
	 * checking whether the channel is free.
	 */
	dmbsy();

	/* Wait for channel to be free */
	if (ch->info->timeout != 0U) {
		timeout = timeout_init_us(ch->info->timeout);
		while (!SCMI_IS_CHANNEL_FREE(mbx_mem->status) &&
		       !timeout_elapsed(timeout)) {
			if (ch->info->delay != 0) {
				udelay(ch->info->delay);
			}
		}

		/* Ensure command completed within timeout */
		if (!SCMI_IS_CHANNEL_FREE(mbx_mem->status)) {
			ERROR("SCMI channel timed out (%" PRIu32 " us)\n",
			      ch->info->timeout);
			panic();
		}
	} else {
		while (!SCMI_IS_CHANNEL_FREE(mbx_mem->status)) {
			if (ch->info->delay != 0) {
				udelay(ch->info->delay);
			}
		}
	}

	/*
	 * Ensure that any read to the SCMI payload area is done after reading
	 * mailbox status. If these 2 reads were reordered then the CPU would
	 * read invalid payload data
	 */
	dmbld();
}

/*
 * Private helper function to release exclusive access to SCMI channel.
 */
void scmi_put_channel(scmi_channel_t *ch)
{
	/* Make sure any previous command has finished */
	assert(SCMI_IS_CHANNEL_FREE(
			((mailbox_mem_t *)(ch->info->scmi_mbx_mem))->status));

	assert(ch->lock);
	scmi_lock_release(ch->lock);
}

/*
 * API to query the SCMI protocol version.
 */
int scmi_proto_version(void *p, uint32_t proto_id, uint32_t *version)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(proto_id, SCMI_PROTO_VERSION_MSG,
							token);
	mbx_mem->len = SCMI_PROTO_VERSION_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL2(mbx_mem->payload, ret, *version);
	assert(mbx_mem->len == SCMI_PROTO_VERSION_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

/*
 * API to query the protocol message attributes for a SCMI protocol.
 */
int scmi_proto_msg_attr(void *p, uint32_t proto_id,
		uint32_t command_id, uint32_t *attr)
{
	mailbox_mem_t *mbx_mem;
	unsigned int token = 0;
	int ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(proto_id,
				SCMI_PROTO_MSG_ATTR_MSG, token);
	mbx_mem->len = SCMI_PROTO_MSG_ATTR_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;
	SCMI_PAYLOAD_ARG1(mbx_mem->payload, command_id);

	scmi_send_sync_command(ch);

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL2(mbx_mem->payload, ret, *attr);
	assert(mbx_mem->len == SCMI_PROTO_MSG_ATTR_RESP_LEN);
	assert(token == SCMI_MSG_GET_TOKEN(mbx_mem->msg_header));

	scmi_put_channel(ch);

	return ret;
}

/*
 * Initialize an SCMI channel.
 *
 * Initializes the SCMI channel provided by `ch`, with an optional callback
 * responsible for initializing individual SCMI protocols. This callback is
 * expected to retrun `0` on success, o a non-zero error code on failure.
 *
 * This function returns a handle to the initialized SCMI channel, or NULL if
 * initialization failed for any reason.
 */
void *scmi_init(scmi_channel_t *ch, scmi_protocol_init_fn_t init_protocols)
{
	assert(ch && ch->info);
	assert(ch->info->db_reg_addr);
	assert(ch->info->db_modify_mask);
	assert(ch->info->db_preserve_mask);
	assert(ch->info->ring_doorbell != NULL);

	assert(ch->lock);

	scmi_lock_init(ch->lock);

	ch->is_initialized = 1;

	if ((init_protocols != NULL) && (init_protocols(ch) != 0)) {
		goto error;
	}

	INFO("SCMI driver initialized\n");

	return (void *)ch;

error:
	ch->is_initialized = 0;
	return NULL;
}
