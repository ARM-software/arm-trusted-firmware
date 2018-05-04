/*
 * Texas Instruments System Control Interface Driver
 *   Based on Linux and U-Boot implementation
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <debug.h>
#include <errno.h>
#include <platform_def.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <sec_proxy.h>

#include "ti_sci_protocol.h"
#include "ti_sci.h"

/**
 * struct ti_sci_desc - Description of SoC integration
 * @host_id:		Host identifier representing the compute entity
 * @max_msg_size:	Maximum size of data per message that can be handled
 */
struct ti_sci_desc {
	uint8_t host_id;
	int max_msg_size;
};

/**
 * struct ti_sci_info - Structure representing a TI SCI instance
 * @desc:	SoC description for this instance
 * @seq:	Seq id used for verification for tx and rx message
 */
struct ti_sci_info {
	const struct ti_sci_desc desc;
	uint8_t seq;
};

static struct ti_sci_info info = {
	.desc = {
		.host_id = TI_SCI_HOST_ID,
		.max_msg_size = TI_SCI_MAX_MESSAGE_SIZE,
	},
	.seq = 0x0a,
};

/**
 * struct ti_sci_xfer - Structure representing a message flow
 * @tx_message:	Transmit message
 * @rx_message:	Receive message
 */
struct ti_sci_xfer {
	struct k3_sec_proxy_msg tx_message;
	struct k3_sec_proxy_msg rx_message;
};

/**
 * ti_sci_setup_one_xfer() - Setup one message type
 *
 * @msg_type:	Message type
 * @msg_flags:	Flag to set for the message
 * @tx_buf:	Buffer to be sent to mailbox channel
 * @tx_message_size: transmit message size
 * @rx_buf:	Buffer to be received from mailbox channel
 * @rx_message_size: receive message size
 *
 * Helper function which is used by various command functions that are
 * exposed to clients of this driver for allocating a message traffic event.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
static int ti_sci_setup_one_xfer(uint16_t msg_type, uint32_t msg_flags,
				 void *tx_buf,
				 size_t tx_message_size,
				 void *rx_buf,
				 size_t rx_message_size,
				 struct ti_sci_xfer *xfer)
{
	struct ti_sci_msg_hdr *hdr;

	/* Ensure we have sane transfer sizes */
	if (rx_message_size > info.desc.max_msg_size ||
	    tx_message_size > info.desc.max_msg_size ||
	    rx_message_size < sizeof(*hdr) ||
	    tx_message_size < sizeof(*hdr))
		return -ERANGE;

	info.seq++;

	hdr = (struct ti_sci_msg_hdr *)tx_buf;
	hdr->seq = info.seq;
	hdr->type = msg_type;
	hdr->host = info.desc.host_id;
	hdr->flags = msg_flags;

	xfer->tx_message.buf = tx_buf;
	xfer->tx_message.len = tx_message_size;

	xfer->rx_message.buf = rx_buf;
	xfer->rx_message.len = rx_message_size;

	return 0;
}

/**
 * ti_sci_get_response() - Receive response from mailbox channel
 *
 * @xfer:	Transfer to initiate and wait for response
 * @chan:	Channel to receive the response
 *
 * Return: 0 if all goes well, else appropriate error message
 */
static inline int ti_sci_get_response(struct ti_sci_xfer *xfer,
				      enum k3_sec_proxy_chan_id chan)
{
	struct k3_sec_proxy_msg *msg = &xfer->rx_message;
	struct ti_sci_msg_hdr *hdr;
	int ret;

	/* Receive the response */
	ret = k3_sec_proxy_recv(chan, msg);
	if (ret) {
		ERROR("Message receive failed (%d)\n", ret);
		return ret;
	}

	/* msg is updated by Secure Proxy driver */
	hdr = (struct ti_sci_msg_hdr *)msg->buf;

	/* Sanity check for message response */
	if (hdr->seq != info.seq) {
		ERROR("Message for %d is not expected\n", hdr->seq);
		return -EINVAL;
	}

	if (msg->len > info.desc.max_msg_size) {
		ERROR("Unable to handle %lu xfer (max %d)\n",
		      msg->len, info.desc.max_msg_size);
		return -EINVAL;
	}

	return 0;
}

/**
 * ti_sci_do_xfer() - Do one transfer
 *
 * @xfer:	Transfer to initiate and wait for response
 *
 * Return: 0 if all goes well, else appropriate error message
 */
static inline int ti_sci_do_xfer(struct ti_sci_xfer *xfer)
{
	struct k3_sec_proxy_msg *msg = &xfer->tx_message;
	int ret;

	/* Send the message */
	ret = k3_sec_proxy_send(SP_HIGH_PRIORITY, msg);
	if (ret) {
		ERROR("Message sending failed (%d)\n", ret);
		return ret;
	}

	ret = ti_sci_get_response(xfer, SP_RESPONSE);
	if (ret) {
		ERROR("Failed to get response (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_get_revision() - Get the revision of the SCI entity
 *
 * Updates the SCI information in the internal data structure.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_get_revision(struct ti_sci_msg_resp_version *rev_info)
{
	struct ti_sci_msg_hdr hdr;
	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_VERSION, 0x0,
				    &hdr, sizeof(hdr),
				    rev_info, sizeof(*rev_info),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_is_response_ack() - Generic ACK/NACK message check
 *
 * @r:	pointer to response buffer
 *
 * Return: true if the response was an ACK, else returns false
 */
static inline bool ti_sci_is_response_ack(void *r)
{
	struct ti_sci_msg_hdr *hdr = r;

	return hdr->flags & TI_SCI_FLAG_RESP_GENERIC_ACK ? true : false;
}

/**
 * ti_sci_device_set_state() - Set device state
 *
 * @id:		Device identifier
 * @flags:	flags to setup for the device
 * @state:	State to move the device to
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_set_state(uint32_t id, uint32_t flags, uint8_t state)
{
	struct ti_sci_msg_req_set_device_state req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_SET_DEVICE_STATE,
				    flags | TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.id = id;
	req.state = state;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	if (!ti_sci_is_response_ack(&resp))
		return -ENODEV;

	return 0;
}

/**
 * ti_sci_device_get_state() - Get device state
 *
 * @id:		Device Identifier
 * @clcnt:	Pointer to Context Loss Count
 * @resets:	pointer to resets
 * @p_state:	pointer to p_state
 * @c_state:	pointer to c_state
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_get_state(uint32_t id,  uint32_t *clcnt,  uint32_t *resets,
			    uint8_t *p_state,  uint8_t *c_state)
{
	struct ti_sci_msg_req_get_device_state req;
	struct ti_sci_msg_resp_get_device_state resp;

	struct ti_sci_xfer xfer;
	int ret;

	if (!clcnt && !resets && !p_state && !c_state)
		return -EINVAL;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_GET_DEVICE_STATE, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.id = id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	if (!ti_sci_is_response_ack(&resp))
		return -ENODEV;

	if (clcnt)
		*clcnt = resp.context_loss_count;
	if (resets)
		*resets = resp.resets;
	if (p_state)
		*p_state = resp.programmed_state;
	if (c_state)
		*c_state = resp.current_state;

	return 0;
}

/**
 * ti_sci_device_get() - Request for device managed by TISCI
 *
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * NOTE: The request is for exclusive access for the processor.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_get(uint32_t id)
{
	return ti_sci_device_set_state(id,
				       MSG_FLAG_DEVICE_EXCLUSIVE,
				       MSG_DEVICE_SW_STATE_ON);
}

/**
 * ti_sci_device_idle() - Idle a device managed by TISCI
 *
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_idle(uint32_t id)
{
	return ti_sci_device_set_state(id,
				       MSG_FLAG_DEVICE_EXCLUSIVE,
				       MSG_DEVICE_SW_STATE_RETENTION);
}

/**
 * ti_sci_device_put() - Release a device managed by TISCI
 *
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_put(uint32_t id)
{
	return ti_sci_device_set_state(id, 0, MSG_DEVICE_SW_STATE_AUTO_OFF);
}

/**
 * ti_sci_device_is_valid() - Is the device valid
 *
 * @id:		Device Identifier
 *
 * Return: 0 if all goes well and the device ID is valid, else return
 *         appropriate error
 */
int ti_sci_device_is_valid(uint32_t id)
{
	uint8_t unused;

	/* check the device state which will also tell us if the ID is valid */
	return ti_sci_device_get_state(id, NULL, NULL, NULL, &unused);
}

/**
 * ti_sci_device_get_clcnt() - Get context loss counter
 *
 * @id:		Device Identifier
 * @count:	Pointer to Context Loss counter to populate
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_get_clcnt(uint32_t id, uint32_t *count)
{
	return ti_sci_device_get_state(id, count, NULL, NULL, NULL);
}

/**
 * ti_sci_device_is_idle() - Check if the device is requested to be idle
 *
 * @id:		Device Identifier
 * @r_state:	true if requested to be idle
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_is_idle(uint32_t id, bool *r_state)
{
	int ret;
	uint8_t state;

	if (!r_state)
		return -EINVAL;

	ret = ti_sci_device_get_state(id, NULL, NULL, &state, NULL);
	if (ret)
		return ret;

	*r_state = (state == MSG_DEVICE_SW_STATE_RETENTION);

	return 0;
}

/**
 * ti_sci_device_is_stop() - Check if the device is requested to be stopped
 *
 * @id:		Device Identifier
 * @r_state:	true if requested to be stopped
 * @curr_state:	true if currently stopped
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_is_stop(uint32_t id, bool *r_state,  bool *curr_state)
{
	int ret;
	uint8_t p_state, c_state;

	if (!r_state && !curr_state)
		return -EINVAL;

	ret = ti_sci_device_get_state(id, NULL, NULL, &p_state, &c_state);
	if (ret)
		return ret;

	if (r_state)
		*r_state = (p_state == MSG_DEVICE_SW_STATE_AUTO_OFF);
	if (curr_state)
		*curr_state = (c_state == MSG_DEVICE_HW_STATE_OFF);

	return 0;
}

/**
 * ti_sci_device_is_on() - Check if the device is requested to be ON
 *
 * @id:		Device Identifier
 * @r_state:	true if requested to be ON
 * @curr_state:	true if currently ON and active
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_is_on(uint32_t id, bool *r_state,  bool *curr_state)
{
	int ret;
	uint8_t p_state, c_state;

	if (!r_state && !curr_state)
		return -EINVAL;

	ret =
	    ti_sci_device_get_state(id, NULL, NULL, &p_state, &c_state);
	if (ret)
		return ret;

	if (r_state)
		*r_state = (p_state == MSG_DEVICE_SW_STATE_ON);
	if (curr_state)
		*curr_state = (c_state == MSG_DEVICE_HW_STATE_ON);

	return 0;
}

/**
 * ti_sci_device_is_trans() - Check if the device is currently transitioning
 *
 * @id:		Device Identifier
 * @curr_state:	true if currently transitioning
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_is_trans(uint32_t id, bool *curr_state)
{
	int ret;
	uint8_t state;

	if (!curr_state)
		return -EINVAL;

	ret = ti_sci_device_get_state(id, NULL, NULL, NULL, &state);
	if (ret)
		return ret;

	*curr_state = (state == MSG_DEVICE_HW_STATE_TRANS);

	return 0;
}

/**
 * ti_sci_device_set_resets() - Set resets for device managed by TISCI
 *
 * @id:			Device Identifier
 * @reset_state:	Device specific reset bit field
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_set_resets(uint32_t id, uint32_t reset_state)
{
	struct ti_sci_msg_req_set_device_resets req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_SET_DEVICE_RESETS,
				    TI_SCI_FLAG_REQ_ACK_ON_PROCESSED,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.id = id;
	req.resets = reset_state;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	if (!ti_sci_is_response_ack(&resp))
		return -ENODEV;

	return 0;
}

/**
 * ti_sci_device_get_resets() - Get reset state for device managed by TISCI
 *
 * @id:			Device Identifier
 * @reset_state:	Pointer to reset state to populate
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_get_resets(uint32_t id, uint32_t *reset_state)
{
	return ti_sci_device_get_state(id, NULL, reset_state, NULL, NULL);
}

/**
 * ti_sci_init() - Basic initialization
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_init(void)
{
	struct ti_sci_msg_resp_version rev_info;
	int ret;

	ret = ti_sci_get_revision(&rev_info);
	if (ret) {
		ERROR("Unable to communicate with control firmware (%d)\n", ret);
		return ret;
	}

	INFO("SYSFW ABI: %d.%d (firmware rev 0x%04x '%s')\n",
	     rev_info.abi_major, rev_info.abi_minor,
	     rev_info.firmware_revision,
	     rev_info.firmware_description);

	return 0;
}
