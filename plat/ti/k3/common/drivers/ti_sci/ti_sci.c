/*
 * Texas Instruments System Control Interface Driver
 *   Based on Linux and U-Boot implementation
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <platform_def.h>

#include <common/debug.h>
#include <sec_proxy.h>

#include "ti_sci_protocol.h"
#include "ti_sci.h"

#if USE_COHERENT_MEM
__section("tzfw_coherent_mem")
#endif
static uint8_t message_sequence;

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
	if (rx_message_size > TI_SCI_MAX_MESSAGE_SIZE ||
	    tx_message_size > TI_SCI_MAX_MESSAGE_SIZE ||
	    rx_message_size < sizeof(*hdr) ||
	    tx_message_size < sizeof(*hdr))
		return -ERANGE;

	hdr = (struct ti_sci_msg_hdr *)tx_buf;
	hdr->seq = ++message_sequence;
	hdr->type = msg_type;
	hdr->host = TI_SCI_HOST_ID;
	hdr->flags = msg_flags | TI_SCI_FLAG_REQ_ACK_ON_PROCESSED;

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
	unsigned int retry = 5;
	int ret;

	for (; retry > 0; retry--) {
		/* Receive the response */
		ret = k3_sec_proxy_recv(chan, msg);
		if (ret) {
			ERROR("Message receive failed (%d)\n", ret);
			return ret;
		}

		/* msg is updated by Secure Proxy driver */
		hdr = (struct ti_sci_msg_hdr *)msg->buf;

		/* Sanity check for message response */
		if (hdr->seq == message_sequence)
			break;
		else
			WARN("Message with sequence ID %u is not expected\n", hdr->seq);
	}
	if (!retry) {
		ERROR("Timed out waiting for message\n");
		return -EINVAL;
	}

	if (msg->len > TI_SCI_MAX_MESSAGE_SIZE) {
		ERROR("Unable to handle %lu xfer (max %d)\n",
		      msg->len, TI_SCI_MAX_MESSAGE_SIZE);
		return -EINVAL;
	}

	if (!(hdr->flags & TI_SCI_FLAG_RESP_GENERIC_ACK))
		return -ENODEV;

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

	/* Clear any spurious messages in receive queue */
	ret = k3_sec_proxy_clear_rx_thread(SP_RESPONSE);
	if (ret) {
		ERROR("Could not clear response queue (%d)\n", ret);
		return ret;
	}

	/* Send the message */
	ret = k3_sec_proxy_send(SP_HIGH_PRIORITY, msg);
	if (ret) {
		ERROR("Message sending failed (%d)\n", ret);
		return ret;
	}

	/* Get the response */
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
 * ti_sci_device_set_state() - Set device state
 *
 * @id:		Device identifier
 * @flags:	flags to setup for the device
 * @state:	State to move the device to
 *
 * Return: 0 if all goes well, else appropriate error message
 */
static int ti_sci_device_set_state(uint32_t id, uint32_t flags, uint8_t state)
{
	struct ti_sci_msg_req_set_device_state req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_SET_DEVICE_STATE, flags,
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
static int ti_sci_device_get_state(uint32_t id,  uint32_t *clcnt,
				   uint32_t *resets, uint8_t *p_state,
				   uint8_t *c_state)
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
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_get(uint32_t id)
{
	return ti_sci_device_set_state(id, 0, MSG_DEVICE_SW_STATE_ON);
}

/**
 * ti_sci_device_get_exclusive() - Exclusive request for device managed by TISCI
 *
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * NOTE: This _exclusive version of the get API is for exclusive access to the
 * device. Any other host in the system will fail to get this device after this
 * call until exclusive access is released with device_put or a non-exclusive
 * set call.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_get_exclusive(uint32_t id)
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
	return ti_sci_device_set_state(id, 0, MSG_DEVICE_SW_STATE_RETENTION);
}

/**
 * ti_sci_device_idle_exclusive() - Exclusive idle a device managed by TISCI
 *
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * NOTE: This _exclusive version of the idle API is for exclusive access to
 * the device. Any other host in the system will fail to get this device after
 * this call until exclusive access is released with device_put or a
 * non-exclusive set call.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_idle_exclusive(uint32_t id)
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
 * ti_sci_device_put_no_wait() - Release a device without requesting or waiting
 *				 for a response.
 *
 * @id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_device_put_no_wait(uint32_t id)
{
	struct ti_sci_msg_req_set_device_state req;
	struct ti_sci_msg_hdr *hdr;
	struct k3_sec_proxy_msg tx_message;
	int ret;

	/* Ensure we have sane transfer size */
	if (sizeof(req) > TI_SCI_MAX_MESSAGE_SIZE)
		return -ERANGE;

	hdr = (struct ti_sci_msg_hdr *)&req;
	hdr->seq = ++message_sequence;
	hdr->type = TI_SCI_MSG_SET_DEVICE_STATE;
	hdr->host = TI_SCI_HOST_ID;
	/* Setup with NORESPONSE flag to keep response queue clean */
	hdr->flags = TI_SCI_FLAG_REQ_GENERIC_NORESPONSE;

	req.id = id;
	req.state = MSG_DEVICE_SW_STATE_AUTO_OFF;

	tx_message.buf = (uint8_t *)&req;
	tx_message.len = sizeof(req);

	 /* Send message */
	ret = k3_sec_proxy_send(SP_HIGH_PRIORITY, &tx_message);
	if (ret) {
		ERROR("Message sending failed (%d)\n", ret);
		return ret;
	}

	/* Return without waiting for response */
	return 0;
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

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_SET_DEVICE_RESETS, 0,
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
 * ti_sci_clock_set_state() - Set clock state helper
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request,
 *		Each device has its own set of clock inputs, This indexes
 *		which clock input to modify
 * @flags:	Header flags as needed
 * @state:	State to request for the clock
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_set_state(uint32_t dev_id, uint8_t clk_id,
			   uint32_t flags, uint8_t state)
{
	struct ti_sci_msg_req_set_clock_state req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_SET_CLOCK_STATE, flags,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.request_state = state;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_clock_get_state() - Get clock state helper
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @programmed_state:	State requested for clock to move to
 * @current_state:	State that the clock is currently in
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_get_state(uint32_t dev_id, uint8_t clk_id,
			   uint8_t *programmed_state,
			   uint8_t *current_state)
{
	struct ti_sci_msg_req_get_clock_state req;
	struct ti_sci_msg_resp_get_clock_state resp;

	struct ti_sci_xfer xfer;
	int ret;

	if (!programmed_state && !current_state)
		return -EINVAL;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_GET_CLOCK_STATE, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	if (programmed_state)
		*programmed_state = resp.programmed_state;
	if (current_state)
		*current_state = resp.current_state;

	return 0;
}

/**
 * ti_sci_clock_get() - Get control of a clock from TI SCI

 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @needs_ssc: 'true' iff Spread Spectrum clock is desired
 * @can_change_freq: 'true' iff frequency change is desired
 * @enable_input_term: 'true' iff input termination is desired
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_get(uint32_t dev_id, uint8_t clk_id,
		     bool needs_ssc, bool can_change_freq,
		     bool enable_input_term)
{
	uint32_t flags = 0;

	flags |= needs_ssc ? MSG_FLAG_CLOCK_ALLOW_SSC : 0;
	flags |= can_change_freq ? MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE : 0;
	flags |= enable_input_term ? MSG_FLAG_CLOCK_INPUT_TERM : 0;

	return ti_sci_clock_set_state(dev_id, clk_id, flags,
				      MSG_CLOCK_SW_STATE_REQ);
}

/**
 * ti_sci_clock_idle() - Idle a clock which is in our control

 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 *
 * NOTE: This clock must have been requested by get_clock previously.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_idle(uint32_t dev_id, uint8_t clk_id)
{
	return ti_sci_clock_set_state(dev_id, clk_id, 0,
				      MSG_CLOCK_SW_STATE_UNREQ);
}

/**
 * ti_sci_clock_put() - Release a clock from our control
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 *
 * NOTE: This clock must have been requested by get_clock previously.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_put(uint32_t dev_id, uint8_t clk_id)
{
	return ti_sci_clock_set_state(dev_id, clk_id, 0,
				      MSG_CLOCK_SW_STATE_AUTO);
}

/**
 * ti_sci_clock_is_auto() - Is the clock being auto managed
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @req_state: state indicating if the clock is auto managed
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_is_auto(uint32_t dev_id, uint8_t clk_id, bool *req_state)
{
	uint8_t state = 0;
	int ret;

	if (!req_state)
		return -EINVAL;

	ret = ti_sci_clock_get_state(dev_id, clk_id, &state, NULL);
	if (ret)
		return ret;

	*req_state = (state == MSG_CLOCK_SW_STATE_AUTO);

	return 0;
}

/**
 * ti_sci_clock_is_on() - Is the clock ON
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @req_state: state indicating if the clock is managed by us and enabled
 * @curr_state: state indicating if the clock is ready for operation
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_is_on(uint32_t dev_id, uint8_t clk_id,
		       bool *req_state, bool *curr_state)
{
	uint8_t c_state = 0, r_state = 0;
	int ret;

	if (!req_state && !curr_state)
		return -EINVAL;

	ret = ti_sci_clock_get_state(dev_id, clk_id, &r_state, &c_state);
	if (ret)
		return ret;

	if (req_state)
		*req_state = (r_state == MSG_CLOCK_SW_STATE_REQ);
	if (curr_state)
		*curr_state = (c_state == MSG_CLOCK_HW_STATE_READY);

	return 0;
}

/**
 * ti_sci_clock_is_off() - Is the clock OFF
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @req_state: state indicating if the clock is managed by us and disabled
 * @curr_state: state indicating if the clock is NOT ready for operation
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_is_off(uint32_t dev_id, uint8_t clk_id,
			bool *req_state, bool *curr_state)
{
	uint8_t c_state = 0, r_state = 0;
	int ret;

	if (!req_state && !curr_state)
		return -EINVAL;

	ret = ti_sci_clock_get_state(dev_id, clk_id, &r_state, &c_state);
	if (ret)
		return ret;

	if (req_state)
		*req_state = (r_state == MSG_CLOCK_SW_STATE_UNREQ);
	if (curr_state)
		*curr_state = (c_state == MSG_CLOCK_HW_STATE_NOT_READY);

	return 0;
}

/**
 * ti_sci_clock_set_parent() - Set the clock source of a specific device clock
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @parent_id:	Parent clock identifier to set
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_set_parent(uint32_t dev_id, uint8_t clk_id, uint8_t parent_id)
{
	struct ti_sci_msg_req_set_clock_parent req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_SET_CLOCK_PARENT, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.parent_id = parent_id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_clock_get_parent() - Get current parent clock source
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @parent_id:	Current clock parent
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_get_parent(uint32_t dev_id, uint8_t clk_id, uint8_t *parent_id)
{
	struct ti_sci_msg_req_get_clock_parent req;
	struct ti_sci_msg_resp_get_clock_parent resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_GET_CLOCK_PARENT, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	*parent_id = resp.parent_id;

	return 0;
}

/**
 * ti_sci_clock_get_num_parents() - Get num parents of the current clk source
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @num_parents: Returns he number of parents to the current clock.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_get_num_parents(uint32_t dev_id, uint8_t clk_id,
				 uint8_t *num_parents)
{
	struct ti_sci_msg_req_get_clock_num_parents req;
	struct ti_sci_msg_resp_get_clock_num_parents resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_GET_NUM_CLOCK_PARENTS, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	*num_parents = resp.num_parents;

	return 0;
}

/**
 * ti_sci_clock_get_match_freq() - Find a good match for frequency
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @min_freq:	The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @target_freq: The target clock frequency in Hz. A frequency will be
 *		processed as close to this target frequency as possible.
 * @max_freq:	The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @match_freq:	Frequency match in Hz response.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_get_match_freq(uint32_t dev_id, uint8_t clk_id,
				uint64_t min_freq, uint64_t target_freq,
				uint64_t max_freq, uint64_t *match_freq)
{
	struct ti_sci_msg_req_query_clock_freq req;
	struct ti_sci_msg_resp_query_clock_freq resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_QUERY_CLOCK_FREQ, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.min_freq_hz = min_freq;
	req.target_freq_hz = target_freq;
	req.max_freq_hz = max_freq;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	*match_freq = resp.freq_hz;

	return 0;
}

/**
 * ti_sci_clock_set_freq() - Set a frequency for clock
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @min_freq:	The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * @target_freq: The target clock frequency in Hz. A frequency will be
 *		processed as close to this target frequency as possible.
 * @max_freq:	The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_set_freq(uint32_t dev_id, uint8_t clk_id, uint64_t min_freq,
			  uint64_t target_freq, uint64_t max_freq)
{
	struct ti_sci_msg_req_set_clock_freq req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_SET_CLOCK_FREQ, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}
	req.dev_id = dev_id;
	req.clk_id = clk_id;
	req.min_freq_hz = min_freq;
	req.target_freq_hz = target_freq;
	req.max_freq_hz = max_freq;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_clock_get_freq() - Get current frequency
 *
 * @dev_id:	Device identifier this request is for
 * @clk_id:	Clock identifier for the device for this request.
 *		Each device has its own set of clock inputs. This indexes
 *		which clock input to modify.
 * @freq:	Currently frequency in Hz
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_clock_get_freq(uint32_t dev_id, uint8_t clk_id, uint64_t *freq)
{
	struct ti_sci_msg_req_get_clock_freq req;
	struct ti_sci_msg_resp_get_clock_freq resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_GET_CLOCK_FREQ, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.dev_id = dev_id;
	req.clk_id = clk_id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	*freq = resp.freq_hz;

	return 0;
}

/**
 * ti_sci_core_reboot() - Command to request system reset
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_core_reboot(void)
{
	struct ti_sci_msg_req_reboot req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TI_SCI_MSG_SYS_RESET, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
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
 * ti_sci_proc_request() - Request a physical processor control
 *
 * @proc_id:	Processor ID this request is for
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_request(uint8_t proc_id)
{
	struct ti_sci_msg_req_proc_request req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TISCI_MSG_PROC_REQUEST, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.processor_id = proc_id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_proc_release() - Release a physical processor control
 *
 * @proc_id:	Processor ID this request is for
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_release(uint8_t proc_id)
{
	struct ti_sci_msg_req_proc_release req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TISCI_MSG_PROC_RELEASE, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.processor_id = proc_id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_proc_handover() - Handover a physical processor control to a host in
 *                          the processor's access control list.
 *
 * @proc_id:	Processor ID this request is for
 * @host_id:	Host ID to get the control of the processor
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_handover(uint8_t proc_id, uint8_t host_id)
{
	struct ti_sci_msg_req_proc_handover req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TISCI_MSG_PROC_HANDOVER, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.processor_id = proc_id;
	req.host_id = host_id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_proc_set_boot_cfg() - Set the processor boot configuration flags
 *
 * @proc_id:		Processor ID this request is for
 * @config_flags_set:	Configuration flags to be set
 * @config_flags_clear:	Configuration flags to be cleared
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_set_boot_cfg(uint8_t proc_id, uint64_t bootvector,
			     uint32_t config_flags_set,
			     uint32_t config_flags_clear)
{
	struct ti_sci_msg_req_set_proc_boot_config req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TISCI_MSG_SET_PROC_BOOT_CONFIG, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.processor_id = proc_id;
	req.bootvector_low = bootvector & TISCI_ADDR_LOW_MASK;
	req.bootvector_high = (bootvector & TISCI_ADDR_HIGH_MASK) >>
				TISCI_ADDR_HIGH_SHIFT;
	req.config_flags_set = config_flags_set;
	req.config_flags_clear = config_flags_clear;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_proc_set_boot_ctrl() - Set the processor boot control flags
 *
 * @proc_id:			Processor ID this request is for
 * @control_flags_set:		Control flags to be set
 * @control_flags_clear:	Control flags to be cleared
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_set_boot_ctrl(uint8_t proc_id, uint32_t control_flags_set,
			      uint32_t control_flags_clear)
{
	struct ti_sci_msg_req_set_proc_boot_ctrl req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TISCI_MSG_SET_PROC_BOOT_CTRL, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.processor_id = proc_id;
	req.control_flags_set = control_flags_set;
	req.control_flags_clear = control_flags_clear;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_proc_set_boot_ctrl_no_wait() - Set the processor boot control flags
 *					 without requesting or waiting for a
 *					 response.
 *
 * @proc_id:			Processor ID this request is for
 * @control_flags_set:		Control flags to be set
 * @control_flags_clear:	Control flags to be cleared
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_set_boot_ctrl_no_wait(uint8_t proc_id,
				      uint32_t control_flags_set,
				      uint32_t control_flags_clear)
{
	struct ti_sci_msg_req_set_proc_boot_ctrl req;
	struct ti_sci_msg_hdr *hdr;
	struct k3_sec_proxy_msg tx_message;
	int ret;

	/* Ensure we have sane transfer size */
	if (sizeof(req) > TI_SCI_MAX_MESSAGE_SIZE)
		return -ERANGE;

	hdr = (struct ti_sci_msg_hdr *)&req;
	hdr->seq = ++message_sequence;
	hdr->type = TISCI_MSG_SET_PROC_BOOT_CTRL;
	hdr->host = TI_SCI_HOST_ID;
	/* Setup with NORESPONSE flag to keep response queue clean */
	hdr->flags = TI_SCI_FLAG_REQ_GENERIC_NORESPONSE;

	req.processor_id = proc_id;
	req.control_flags_set = control_flags_set;
	req.control_flags_clear = control_flags_clear;

	tx_message.buf = (uint8_t *)&req;
	tx_message.len = sizeof(req);

	 /* Send message */
	ret = k3_sec_proxy_send(SP_HIGH_PRIORITY, &tx_message);
	if (ret) {
		ERROR("Message sending failed (%d)\n", ret);
		return ret;
	}

	/* Return without waiting for response */
	return 0;
}

/**
 * ti_sci_proc_auth_boot_image() - Authenticate and load image and then set the
 *                                 processor configuration flags
 *
 * @proc_id:	Processor ID this request is for
 * @cert_addr:	Memory address at which payload image certificate is located
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_auth_boot_image(uint8_t proc_id, uint64_t cert_addr)
{
	struct ti_sci_msg_req_proc_auth_boot_image req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TISCI_MSG_PROC_AUTH_BOOT_IMIAGE, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.processor_id = proc_id;
	req.cert_addr_low = cert_addr & TISCI_ADDR_LOW_MASK;
	req.cert_addr_high = (cert_addr & TISCI_ADDR_HIGH_MASK) >>
				TISCI_ADDR_HIGH_SHIFT;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_proc_get_boot_status() - Get the processor boot status
 *
 * @proc_id:	Processor ID this request is for
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_get_boot_status(uint8_t proc_id, uint64_t *bv,
				uint32_t *cfg_flags,
				uint32_t *ctrl_flags,
				uint32_t *sts_flags)
{
	struct ti_sci_msg_req_get_proc_boot_status req;
	struct ti_sci_msg_resp_get_proc_boot_status resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TISCI_MSG_GET_PROC_BOOT_STATUS, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.processor_id = proc_id;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	*bv = (resp.bootvector_low & TISCI_ADDR_LOW_MASK) |
	      (((uint64_t)resp.bootvector_high << TISCI_ADDR_HIGH_SHIFT) &
	       TISCI_ADDR_HIGH_MASK);
	*cfg_flags = resp.config_flags;
	*ctrl_flags = resp.control_flags;
	*sts_flags = resp.status_flags;

	return 0;
}

/**
 * ti_sci_proc_wait_boot_status() - Wait for a processor boot status
 *
 * @proc_id:			Processor ID this request is for
 * @num_wait_iterations		Total number of iterations we will check before
 *				we will timeout and give up
 * @num_match_iterations	How many iterations should we have continued
 *				status to account for status bits glitching.
 *				This is to make sure that match occurs for
 *				consecutive checks. This implies that the
 *				worst case should consider that the stable
 *				time should at the worst be num_wait_iterations
 *				num_match_iterations to prevent timeout.
 * @delay_per_iteration_us	Specifies how long to wait (in micro seconds)
 *				between each status checks. This is the minimum
 *				duration, and overhead of register reads and
 *				checks are on top of this and can vary based on
 *				varied conditions.
 * @delay_before_iterations_us	Specifies how long to wait (in micro seconds)
 *				before the very first check in the first
 *				iteration of status check loop. This is the
 *				minimum duration, and overhead of register
 *				reads and checks are.
 * @status_flags_1_set_all_wait	If non-zero, Specifies that all bits of the
 *				status matching this field requested MUST be 1.
 * @status_flags_1_set_any_wait	If non-zero, Specifies that at least one of the
 *				bits matching this field requested MUST be 1.
 * @status_flags_1_clr_all_wait	If non-zero, Specifies that all bits of the
 *				status matching this field requested MUST be 0.
 * @status_flags_1_clr_any_wait	If non-zero, Specifies that at least one of the
 *				bits matching this field requested MUST be 0.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_wait_boot_status(uint8_t proc_id, uint8_t num_wait_iterations,
				 uint8_t num_match_iterations,
				 uint8_t delay_per_iteration_us,
				 uint8_t delay_before_iterations_us,
				 uint32_t status_flags_1_set_all_wait,
				 uint32_t status_flags_1_set_any_wait,
				 uint32_t status_flags_1_clr_all_wait,
				 uint32_t status_flags_1_clr_any_wait)
{
	struct ti_sci_msg_req_wait_proc_boot_status req;
	struct ti_sci_msg_hdr resp;

	struct ti_sci_xfer xfer;
	int ret;

	ret = ti_sci_setup_one_xfer(TISCI_MSG_WAIT_PROC_BOOT_STATUS, 0,
				    &req, sizeof(req),
				    &resp, sizeof(resp),
				    &xfer);
	if (ret) {
		ERROR("Message alloc failed (%d)\n", ret);
		return ret;
	}

	req.processor_id = proc_id;
	req.num_wait_iterations = num_wait_iterations;
	req.num_match_iterations = num_match_iterations;
	req.delay_per_iteration_us = delay_per_iteration_us;
	req.delay_before_iterations_us = delay_before_iterations_us;
	req.status_flags_1_set_all_wait = status_flags_1_set_all_wait;
	req.status_flags_1_set_any_wait = status_flags_1_set_any_wait;
	req.status_flags_1_clr_all_wait = status_flags_1_clr_all_wait;
	req.status_flags_1_clr_any_wait = status_flags_1_clr_any_wait;

	ret = ti_sci_do_xfer(&xfer);
	if (ret) {
		ERROR("Transfer send failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * ti_sci_proc_wait_boot_status_no_wait() - Wait for a processor boot status
 *					    without requesting or waiting for
 *					    a response.
 *
 * @proc_id:			Processor ID this request is for
 * @num_wait_iterations		Total number of iterations we will check before
 *				we will timeout and give up
 * @num_match_iterations	How many iterations should we have continued
 *				status to account for status bits glitching.
 *				This is to make sure that match occurs for
 *				consecutive checks. This implies that the
 *				worst case should consider that the stable
 *				time should at the worst be num_wait_iterations
 *				num_match_iterations to prevent timeout.
 * @delay_per_iteration_us	Specifies how long to wait (in micro seconds)
 *				between each status checks. This is the minimum
 *				duration, and overhead of register reads and
 *				checks are on top of this and can vary based on
 *				varied conditions.
 * @delay_before_iterations_us	Specifies how long to wait (in micro seconds)
 *				before the very first check in the first
 *				iteration of status check loop. This is the
 *				minimum duration, and overhead of register
 *				reads and checks are.
 * @status_flags_1_set_all_wait	If non-zero, Specifies that all bits of the
 *				status matching this field requested MUST be 1.
 * @status_flags_1_set_any_wait	If non-zero, Specifies that at least one of the
 *				bits matching this field requested MUST be 1.
 * @status_flags_1_clr_all_wait	If non-zero, Specifies that all bits of the
 *				status matching this field requested MUST be 0.
 * @status_flags_1_clr_any_wait	If non-zero, Specifies that at least one of the
 *				bits matching this field requested MUST be 0.
 *
 * Return: 0 if all goes well, else appropriate error message
 */
int ti_sci_proc_wait_boot_status_no_wait(uint8_t proc_id,
					 uint8_t num_wait_iterations,
					 uint8_t num_match_iterations,
					 uint8_t delay_per_iteration_us,
					 uint8_t delay_before_iterations_us,
					 uint32_t status_flags_1_set_all_wait,
					 uint32_t status_flags_1_set_any_wait,
					 uint32_t status_flags_1_clr_all_wait,
					 uint32_t status_flags_1_clr_any_wait)
{
	struct ti_sci_msg_req_wait_proc_boot_status req;
	struct ti_sci_msg_hdr *hdr;
	struct k3_sec_proxy_msg tx_message;
	int ret;

	/* Ensure we have sane transfer size */
	if (sizeof(req) > TI_SCI_MAX_MESSAGE_SIZE)
		return -ERANGE;

	hdr = (struct ti_sci_msg_hdr *)&req;
	hdr->seq = ++message_sequence;
	hdr->type = TISCI_MSG_WAIT_PROC_BOOT_STATUS;
	hdr->host = TI_SCI_HOST_ID;
	/* Setup with NORESPONSE flag to keep response queue clean */
	hdr->flags = TI_SCI_FLAG_REQ_GENERIC_NORESPONSE;

	req.processor_id = proc_id;
	req.num_wait_iterations = num_wait_iterations;
	req.num_match_iterations = num_match_iterations;
	req.delay_per_iteration_us = delay_per_iteration_us;
	req.delay_before_iterations_us = delay_before_iterations_us;
	req.status_flags_1_set_all_wait = status_flags_1_set_all_wait;
	req.status_flags_1_set_any_wait = status_flags_1_set_any_wait;
	req.status_flags_1_clr_all_wait = status_flags_1_clr_all_wait;
	req.status_flags_1_clr_any_wait = status_flags_1_clr_any_wait;

	tx_message.buf = (uint8_t *)&req;
	tx_message.len = sizeof(req);

	 /* Send message */
	ret = k3_sec_proxy_send(SP_HIGH_PRIORITY, &tx_message);
	if (ret) {
		ERROR("Message sending failed (%d)\n", ret);
		return ret;
	}

	/* Return without waiting for response */
	return 0;
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
