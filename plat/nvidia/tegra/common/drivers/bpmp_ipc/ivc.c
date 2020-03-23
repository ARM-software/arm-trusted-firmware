/*
 * Copyright (c) 2017-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "ivc.h"

/*
 * IVC channel reset protocol.
 *
 * Each end uses its tx_channel.state to indicate its synchronization state.
 */
enum {
	/*
	 * This value is zero for backwards compatibility with services that
	 * assume channels to be initially zeroed. Such channels are in an
	 * initially valid state, but cannot be asynchronously reset, and must
	 * maintain a valid state at all times.
	 *
	 * The transmitting end can enter the established state from the sync or
	 * ack state when it observes the receiving endpoint in the ack or
	 * established state, indicating that has cleared the counters in our
	 * rx_channel.
	 */
	ivc_state_established = U(0),

	/*
	 * If an endpoint is observed in the sync state, the remote endpoint is
	 * allowed to clear the counters it owns asynchronously with respect to
	 * the current endpoint. Therefore, the current endpoint is no longer
	 * allowed to communicate.
	 */
	ivc_state_sync = U(1),

	/*
	 * When the transmitting end observes the receiving end in the sync
	 * state, it can clear the w_count and r_count and transition to the ack
	 * state. If the remote endpoint observes us in the ack state, it can
	 * return to the established state once it has cleared its counters.
	 */
	ivc_state_ack = U(2)
};

/*
 * This structure is divided into two-cache aligned parts, the first is only
 * written through the tx_channel pointer, while the second is only written
 * through the rx_channel pointer. This delineates ownership of the cache lines,
 * which is critical to performance and necessary in non-cache coherent
 * implementations.
 */
struct ivc_channel_header {
	struct {
		/* fields owned by the transmitting end */
		uint32_t w_count;
		uint32_t state;
		uint32_t w_rsvd[IVC_CHHDR_TX_FIELDS - 2];
	};
	struct {
		/* fields owned by the receiving end */
		uint32_t r_count;
		uint32_t r_rsvd[IVC_CHHDR_RX_FIELDS - 1];
	};
};

static inline bool ivc_channel_empty(const struct ivc *ivc,
		volatile const struct ivc_channel_header *ch)
{
	/*
	 * This function performs multiple checks on the same values with
	 * security implications, so sample the counters' current values in
	 * shared memory to ensure that these checks use the same values.
	 */
	uint32_t wr_count = ch->w_count;
	uint32_t rd_count = ch->r_count;
	bool ret = false;

	(void)ivc;

	/*
	 * Perform an over-full check to prevent denial of service attacks where
	 * a server could be easily fooled into believing that there's an
	 * extremely large number of frames ready, since receivers are not
	 * expected to check for full or over-full conditions.
	 *
	 * Although the channel isn't empty, this is an invalid case caused by
	 * a potentially malicious peer, so returning empty is safer, because it
	 * gives the impression that the channel has gone silent.
	 */
	if (((wr_count - rd_count) > ivc->nframes) || (wr_count == rd_count)) {
		ret = true;
	}

	return ret;
}

static inline bool ivc_channel_full(const struct ivc *ivc,
		volatile const struct ivc_channel_header *ch)
{
	uint32_t wr_count = ch->w_count;
	uint32_t rd_count = ch->r_count;

	(void)ivc;

	/*
	 * Invalid cases where the counters indicate that the queue is over
	 * capacity also appear full.
	 */
	return ((wr_count - rd_count) >= ivc->nframes);
}

static inline uint32_t ivc_channel_avail_count(const struct ivc *ivc,
		volatile const struct ivc_channel_header *ch)
{
	uint32_t wr_count = ch->w_count;
	uint32_t rd_count = ch->r_count;

	(void)ivc;

	/*
	 * This function isn't expected to be used in scenarios where an
	 * over-full situation can lead to denial of service attacks. See the
	 * comment in ivc_channel_empty() for an explanation about special
	 * over-full considerations.
	 */
	return (wr_count - rd_count);
}

static inline void ivc_advance_tx(struct ivc *ivc)
{
	ivc->tx_channel->w_count++;

	if (ivc->w_pos == (ivc->nframes - (uint32_t)1U)) {
		ivc->w_pos = 0U;
	} else {
		ivc->w_pos++;
	}
}

static inline void ivc_advance_rx(struct ivc *ivc)
{
	ivc->rx_channel->r_count++;

	if (ivc->r_pos == (ivc->nframes - (uint32_t)1U)) {
		ivc->r_pos = 0U;
	} else {
		ivc->r_pos++;
	}
}

static inline int32_t ivc_check_read(const struct ivc *ivc)
{
	/*
	 * tx_channel->state is set locally, so it is not synchronized with
	 * state from the remote peer. The remote peer cannot reset its
	 * transmit counters until we've acknowledged its synchronization
	 * request, so no additional synchronization is required because an
	 * asynchronous transition of rx_channel->state to ivc_state_ack is not
	 * allowed.
	 */
	if (ivc->tx_channel->state != ivc_state_established) {
		return -ECONNRESET;
	}

	/*
	* Avoid unnecessary invalidations when performing repeated accesses to
	* an IVC channel by checking the old queue pointers first.
	* Synchronization is only necessary when these pointers indicate empty
	* or full.
	*/
	if (!ivc_channel_empty(ivc, ivc->rx_channel)) {
		return 0;
	}

	return ivc_channel_empty(ivc, ivc->rx_channel) ? -ENOMEM : 0;
}

static inline int32_t ivc_check_write(const struct ivc *ivc)
{
	if (ivc->tx_channel->state != ivc_state_established) {
		return -ECONNRESET;
	}

	if (!ivc_channel_full(ivc, ivc->tx_channel)) {
		return 0;
	}

	return ivc_channel_full(ivc, ivc->tx_channel) ? -ENOMEM : 0;
}

bool tegra_ivc_can_read(const struct ivc *ivc)
{
	return ivc_check_read(ivc) == 0;
}

bool tegra_ivc_can_write(const struct ivc *ivc)
{
	return ivc_check_write(ivc) == 0;
}

bool tegra_ivc_tx_empty(const struct ivc *ivc)
{
	return ivc_channel_empty(ivc, ivc->tx_channel);
}

static inline uintptr_t calc_frame_offset(uint32_t frame_index,
	uint32_t frame_size, uint32_t frame_offset)
{
    return ((uintptr_t)frame_index * (uintptr_t)frame_size) +
	    (uintptr_t)frame_offset;
}

static void *ivc_frame_pointer(const struct ivc *ivc,
				volatile const struct ivc_channel_header *ch,
				uint32_t frame)
{
	assert(frame < ivc->nframes);
	return (void *)((uintptr_t)(&ch[1]) +
		calc_frame_offset(frame, ivc->frame_size, 0));
}

int32_t tegra_ivc_read(struct ivc *ivc, void *buf, size_t max_read)
{
	const void *src;
	int32_t result;

	if (buf == NULL) {
		return -EINVAL;
	}

	if (max_read > ivc->frame_size) {
		return -E2BIG;
	}

	result = ivc_check_read(ivc);
	if (result != 0) {
		return result;
	}

	/*
	 * Order observation of w_pos potentially indicating new data before
	 * data read.
	 */
	dmbish();

	src = ivc_frame_pointer(ivc, ivc->rx_channel, ivc->r_pos);

	(void)memcpy(buf, src, max_read);

	ivc_advance_rx(ivc);

	/*
	 * Ensure our write to r_pos occurs before our read from w_pos.
	 */
	dmbish();

	/*
	 * Notify only upon transition from full to non-full.
	 * The available count can only asynchronously increase, so the
	 * worst possible side-effect will be a spurious notification.
	 */
	if (ivc_channel_avail_count(ivc, ivc->rx_channel) == (ivc->nframes - (uint32_t)1U)) {
		ivc->notify(ivc);
	}

	return (int32_t)max_read;
}

/* directly peek at the next frame rx'ed */
void *tegra_ivc_read_get_next_frame(const struct ivc *ivc)
{
	if (ivc_check_read(ivc) != 0) {
		return NULL;
	}

	/*
	 * Order observation of w_pos potentially indicating new data before
	 * data read.
	 */
	dmbld();

	return ivc_frame_pointer(ivc, ivc->rx_channel, ivc->r_pos);
}

int32_t tegra_ivc_read_advance(struct ivc *ivc)
{
	/*
	 * No read barriers or synchronization here: the caller is expected to
	 * have already observed the channel non-empty. This check is just to
	 * catch programming errors.
	 */
	int32_t result = ivc_check_read(ivc);
	if (result != 0) {
		return result;
	}

	ivc_advance_rx(ivc);

	/*
	 * Ensure our write to r_pos occurs before our read from w_pos.
	 */
	dmbish();

	/*
	 * Notify only upon transition from full to non-full.
	 * The available count can only asynchronously increase, so the
	 * worst possible side-effect will be a spurious notification.
	 */
	if (ivc_channel_avail_count(ivc, ivc->rx_channel) == (ivc->nframes - (uint32_t)1U)) {
		ivc->notify(ivc);
	}

	return 0;
}

int32_t tegra_ivc_write(struct ivc *ivc, const void *buf, size_t size)
{
	void *p;
	int32_t result;

	if ((buf == NULL) || (ivc == NULL)) {
		return -EINVAL;
	}

	if (size > ivc->frame_size) {
		return -E2BIG;
	}

	result = ivc_check_write(ivc);
	if (result != 0) {
		return result;
	}

	p = ivc_frame_pointer(ivc, ivc->tx_channel, ivc->w_pos);

	(void)memset(p, 0, ivc->frame_size);
	(void)memcpy(p, buf, size);

	/*
	 * Ensure that updated data is visible before the w_pos counter
	 * indicates that it is ready.
	 */
	dmbst();

	ivc_advance_tx(ivc);

	/*
	 * Ensure our write to w_pos occurs before our read from r_pos.
	 */
	dmbish();

	/*
	 * Notify only upon transition from empty to non-empty.
	 * The available count can only asynchronously decrease, so the
	 * worst possible side-effect will be a spurious notification.
	 */
	if (ivc_channel_avail_count(ivc, ivc->tx_channel) == 1U) {
		ivc->notify(ivc);
	}

	return (int32_t)size;
}

/* directly poke at the next frame to be tx'ed */
void *tegra_ivc_write_get_next_frame(const struct ivc *ivc)
{
	if (ivc_check_write(ivc) != 0) {
		return NULL;
	}

	return ivc_frame_pointer(ivc, ivc->tx_channel, ivc->w_pos);
}

/* advance the tx buffer */
int32_t tegra_ivc_write_advance(struct ivc *ivc)
{
	int32_t result = ivc_check_write(ivc);

	if (result != 0) {
		return result;
	}

	/*
	 * Order any possible stores to the frame before update of w_pos.
	 */
	dmbst();

	ivc_advance_tx(ivc);

	/*
	 * Ensure our write to w_pos occurs before our read from r_pos.
	 */
	dmbish();

	/*
	 * Notify only upon transition from empty to non-empty.
	 * The available count can only asynchronously decrease, so the
	 * worst possible side-effect will be a spurious notification.
	 */
	if (ivc_channel_avail_count(ivc, ivc->tx_channel) == (uint32_t)1U) {
		ivc->notify(ivc);
	}

	return 0;
}

void tegra_ivc_channel_reset(const struct ivc *ivc)
{
	ivc->tx_channel->state = ivc_state_sync;
	ivc->notify(ivc);
}

/*
 * ===============================================================
 *  IVC State Transition Table - see tegra_ivc_channel_notified()
 * ===============================================================
 *
 *	local	remote	action
 *	-----	------	-----------------------------------
 *	SYNC	EST	<none>
 *	SYNC	ACK	reset counters; move to EST; notify
 *	SYNC	SYNC	reset counters; move to ACK; notify
 *	ACK	EST	move to EST; notify
 *	ACK	ACK	move to EST; notify
 *	ACK	SYNC	reset counters; move to ACK; notify
 *	EST	EST	<none>
 *	EST	ACK	<none>
 *	EST	SYNC	reset counters; move to ACK; notify
 *
 * ===============================================================
 */
int32_t tegra_ivc_channel_notified(struct ivc *ivc)
{
	uint32_t peer_state;

	/* Copy the receiver's state out of shared memory. */
	peer_state = ivc->rx_channel->state;

	if (peer_state == (uint32_t)ivc_state_sync) {
		/*
		 * Order observation of ivc_state_sync before stores clearing
		 * tx_channel.
		 */
		dmbld();

		/*
		 * Reset tx_channel counters. The remote end is in the SYNC
		 * state and won't make progress until we change our state,
		 * so the counters are not in use at this time.
		 */
		ivc->tx_channel->w_count = 0U;
		ivc->rx_channel->r_count = 0U;

		ivc->w_pos = 0U;
		ivc->r_pos = 0U;

		/*
		 * Ensure that counters appear cleared before new state can be
		 * observed.
		 */
		dmbst();

		/*
		 * Move to ACK state. We have just cleared our counters, so it
		 * is now safe for the remote end to start using these values.
		 */
		ivc->tx_channel->state = ivc_state_ack;

		/*
		 * Notify remote end to observe state transition.
		 */
		ivc->notify(ivc);

	} else if ((ivc->tx_channel->state == (uint32_t)ivc_state_sync) &&
			(peer_state == (uint32_t)ivc_state_ack)) {
		/*
		 * Order observation of ivc_state_sync before stores clearing
		 * tx_channel.
		 */
		dmbld();

		/*
		 * Reset tx_channel counters. The remote end is in the ACK
		 * state and won't make progress until we change our state,
		 * so the counters are not in use at this time.
		 */
		ivc->tx_channel->w_count = 0U;
		ivc->rx_channel->r_count = 0U;

		ivc->w_pos = 0U;
		ivc->r_pos = 0U;

		/*
		 * Ensure that counters appear cleared before new state can be
		 * observed.
		 */
		dmbst();

		/*
		 * Move to ESTABLISHED state. We know that the remote end has
		 * already cleared its counters, so it is safe to start
		 * writing/reading on this channel.
		 */
		ivc->tx_channel->state = ivc_state_established;

		/*
		 * Notify remote end to observe state transition.
		 */
		ivc->notify(ivc);

	} else if (ivc->tx_channel->state == (uint32_t)ivc_state_ack) {
		/*
		 * At this point, we have observed the peer to be in either
		 * the ACK or ESTABLISHED state. Next, order observation of
		 * peer state before storing to tx_channel.
		 */
		dmbld();

		/*
		 * Move to ESTABLISHED state. We know that we have previously
		 * cleared our counters, and we know that the remote end has
		 * cleared its counters, so it is safe to start writing/reading
		 * on this channel.
		 */
		ivc->tx_channel->state = ivc_state_established;

		/*
		 * Notify remote end to observe state transition.
		 */
		ivc->notify(ivc);

	} else {
		/*
		 * There is no need to handle any further action. Either the
		 * channel is already fully established, or we are waiting for
		 * the remote end to catch up with our current state. Refer
		 * to the diagram in "IVC State Transition Table" above.
		 */
	}

	return ((ivc->tx_channel->state == (uint32_t)ivc_state_established) ? 0 : -EAGAIN);
}

size_t tegra_ivc_align(size_t size)
{
	return (size + (IVC_ALIGN - 1U)) & ~(IVC_ALIGN - 1U);
}

size_t tegra_ivc_total_queue_size(size_t queue_size)
{
	if ((queue_size & (IVC_ALIGN - 1U)) != 0U) {
		ERROR("queue_size (%d) must be %d-byte aligned\n",
				(int32_t)queue_size, IVC_ALIGN);
		return 0;
	}
	return queue_size + sizeof(struct ivc_channel_header);
}

static int32_t check_ivc_params(uintptr_t queue_base1, uintptr_t queue_base2,
		uint32_t nframes, uint32_t frame_size)
{
	assert((offsetof(struct ivc_channel_header, w_count)
				& (IVC_ALIGN - 1U)) == 0U);
	assert((offsetof(struct ivc_channel_header, r_count)
				& (IVC_ALIGN - 1U)) == 0U);
	assert((sizeof(struct ivc_channel_header) & (IVC_ALIGN - 1U)) == 0U);

	if (((uint64_t)nframes * (uint64_t)frame_size) >= 0x100000000ULL) {
		ERROR("nframes * frame_size overflows\n");
		return -EINVAL;
	}

	/*
	 * The headers must at least be aligned enough for counters
	 * to be accessed atomically.
	 */
	if ((queue_base1 & (IVC_ALIGN - 1U)) != 0U) {
		ERROR("ivc channel start not aligned: %lx\n", queue_base1);
		return -EINVAL;
	}
	if ((queue_base2 & (IVC_ALIGN - 1U)) != 0U) {
		ERROR("ivc channel start not aligned: %lx\n", queue_base2);
		return -EINVAL;
	}

	if ((frame_size & (IVC_ALIGN - 1U)) != 0U) {
		ERROR("frame size not adequately aligned: %u\n",
				frame_size);
		return -EINVAL;
	}

	if (queue_base1 < queue_base2) {
		if ((queue_base1 + ((uint64_t)frame_size * nframes)) > queue_base2) {
			ERROR("queue regions overlap: %lx + %x, %x\n",
					queue_base1, frame_size,
					frame_size * nframes);
			return -EINVAL;
		}
	} else {
		if ((queue_base2 + ((uint64_t)frame_size * nframes)) > queue_base1) {
			ERROR("queue regions overlap: %lx + %x, %x\n",
					queue_base2, frame_size,
					frame_size * nframes);
			return -EINVAL;
		}
	}

	return 0;
}

int32_t tegra_ivc_init(struct ivc *ivc, uintptr_t rx_base, uintptr_t tx_base,
		uint32_t nframes, uint32_t frame_size,
		ivc_notify_function notify)
{
	int32_t result;

	/* sanity check input params */
	if ((ivc == NULL) || (notify == NULL)) {
		return -EINVAL;
	}

	result = check_ivc_params(rx_base, tx_base, nframes, frame_size);
	if (result != 0) {
		return result;
	}

	/*
	 * All sizes that can be returned by communication functions should
	 * fit in a 32-bit integer.
	 */
	if (frame_size > (1u << 31)) {
		return -E2BIG;
	}

	ivc->rx_channel = (struct ivc_channel_header *)rx_base;
	ivc->tx_channel = (struct ivc_channel_header *)tx_base;
	ivc->notify = notify;
	ivc->frame_size = frame_size;
	ivc->nframes = nframes;
	ivc->w_pos = 0U;
	ivc->r_pos = 0U;

	INFO("%s: done\n", __func__);

	return 0;
}
