/*
 * Copyright (c) 2017-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <bpmp_ipc.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <errno.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>
#include <stdbool.h>
#include <string.h>
#include <tegra_def.h>

#include "intf.h"
#include "ivc.h"

/**
 * Holds IVC channel data
 */
struct ccplex_bpmp_channel_data {
	/* Buffer for incoming data */
	struct frame_data *ib;

	/* Buffer for outgoing data */
	struct frame_data *ob;
};

static struct ccplex_bpmp_channel_data s_channel;
static struct ivc ivc_ccplex_bpmp_channel;

/*
 * Helper functions to access the HSP doorbell registers
 */
static inline uint32_t hsp_db_read(uint32_t reg)
{
	return mmio_read_32((uint32_t)(TEGRA_HSP_DBELL_BASE + reg));
}

static inline void hsp_db_write(uint32_t reg, uint32_t val)
{
	mmio_write_32((uint32_t)(TEGRA_HSP_DBELL_BASE + reg), val);
}

/*******************************************************************************
 *      IVC wrappers for CCPLEX <-> BPMP communication.
 ******************************************************************************/

static void tegra_bpmp_ring_bpmp_doorbell(void);

/*
 * Get the next frame where data can be written.
 */
static struct frame_data *tegra_bpmp_get_next_out_frame(void)
{
	struct frame_data *frame;
	const struct ivc *ch = &ivc_ccplex_bpmp_channel;

	frame = (struct frame_data *)tegra_ivc_write_get_next_frame(ch);
	if (frame == NULL) {
		ERROR("%s: Error in getting next frame, exiting\n", __func__);
	} else {
		s_channel.ob = frame;
	}

	return frame;
}

static void tegra_bpmp_signal_slave(void)
{
	(void)tegra_ivc_write_advance(&ivc_ccplex_bpmp_channel);
	tegra_bpmp_ring_bpmp_doorbell();
}

static int32_t tegra_bpmp_free_master(void)
{
	return tegra_ivc_read_advance(&ivc_ccplex_bpmp_channel);
}

static bool tegra_bpmp_slave_acked(void)
{
	struct frame_data *frame;
	bool ret = true;

	frame = (struct frame_data *)tegra_ivc_read_get_next_frame(&ivc_ccplex_bpmp_channel);
	if (frame == NULL) {
		ret = false;
	} else {
		s_channel.ib = frame;
	}

	return ret;
}

static struct frame_data *tegra_bpmp_get_cur_in_frame(void)
{
	return s_channel.ib;
}

/*
 * Enables BPMP to ring CCPlex doorbell
 */
static void tegra_bpmp_enable_ccplex_doorbell(void)
{
	uint32_t reg;

	reg = hsp_db_read(HSP_DBELL_1_ENABLE);
	reg |= HSP_MASTER_BPMP_BIT;
	hsp_db_write(HSP_DBELL_1_ENABLE, reg);
}

/*
 * CCPlex rings the BPMP doorbell
 */
static void tegra_bpmp_ring_bpmp_doorbell(void)
{
	/*
	 * Any writes to this register has the same effect,
	 * uses master ID of the write transaction and set
	 * corresponding flag.
	 */
	hsp_db_write(HSP_DBELL_3_TRIGGER, HSP_MASTER_CCPLEX_BIT);
}

/*
 * Returns true if CCPLex can ring BPMP doorbell, otherwise false.
 * This also signals that BPMP is up and ready.
 */
static bool tegra_bpmp_can_ccplex_ring_doorbell(void)
{
	uint32_t reg;

	/* check if ccplex can communicate with bpmp */
	reg = hsp_db_read(HSP_DBELL_3_ENABLE);

	return ((reg & HSP_MASTER_CCPLEX_BIT) != 0U);
}

static int32_t tegra_bpmp_wait_for_slave_ack(void)
{
	uint32_t timeout = TIMEOUT_RESPONSE_FROM_BPMP_US;

	while (!tegra_bpmp_slave_acked() && (timeout != 0U)) {
		udelay(1);
		timeout--;
	};

	return ((timeout == 0U) ? -ETIMEDOUT : 0);
}

/*
 * Notification from the ivc layer
 */
static void tegra_bpmp_ivc_notify(const struct ivc *ivc)
{
	(void)(ivc);

	tegra_bpmp_ring_bpmp_doorbell();
}

/*
 * Atomic send/receive API, which means it waits until slave acks
 */
static int32_t tegra_bpmp_ipc_send_req_atomic(uint32_t mrq, void *p_out,
			uint32_t size_out, void *p_in, uint32_t size_in)
{
	struct frame_data *frame = tegra_bpmp_get_next_out_frame();
	const struct frame_data *f_in = NULL;
	int32_t ret = 0;
	void *p_fdata;

	if ((p_out == NULL) || (size_out > IVC_DATA_SZ_BYTES) ||
	    (frame == NULL)) {
		ERROR("%s: invalid parameters, exiting\n", __func__);
		return -EINVAL;
	}

	/* prepare the command frame */
	frame->mrq = mrq;
	frame->flags = FLAG_DO_ACK;
	p_fdata = frame->data;
	(void)memcpy(p_fdata, p_out, (size_t)size_out);

	/* signal the slave */
	tegra_bpmp_signal_slave();

	/* wait for slave to ack */
	ret = tegra_bpmp_wait_for_slave_ack();
	if (ret < 0) {
		ERROR("%s: wait for slave failed (%d)\n", __func__, ret);
		return ret;
	}

	/* retrieve the response frame */
	if ((size_in <= IVC_DATA_SZ_BYTES) && (p_in != NULL)) {

		f_in = tegra_bpmp_get_cur_in_frame();
		if (f_in != NULL) {
			ERROR("Failed to get next input frame!\n");
		} else {
			(void)memcpy(p_in, p_fdata, (size_t)size_in);
		}
	}

	ret = tegra_bpmp_free_master();
	if (ret < 0) {
		ERROR("%s: free master failed (%d)\n", __func__, ret);
	}

	return ret;
}

/*
 * Initializes the BPMP<--->CCPlex communication path.
 */
int32_t tegra_bpmp_ipc_init(void)
{
	size_t msg_size;
	uint32_t frame_size, timeout;
	int32_t error = 0;

	/* allow bpmp to ring CCPLEX's doorbell */
	tegra_bpmp_enable_ccplex_doorbell();

	/* wait for BPMP to actually ring the doorbell */
	timeout = TIMEOUT_RESPONSE_FROM_BPMP_US;
	while ((timeout != 0U) && !tegra_bpmp_can_ccplex_ring_doorbell()) {
		udelay(1); /* bpmp turn-around time */
		timeout--;
	}

	if (timeout == 0U) {
		ERROR("%s: BPMP firmware is not ready\n", __func__);
		return -ENOTSUP;
	}

	INFO("%s: BPMP handshake completed\n", __func__);

	msg_size = tegra_ivc_align(IVC_CMD_SZ_BYTES);
	frame_size = (uint32_t)tegra_ivc_total_queue_size(msg_size);
	if (frame_size > TEGRA_BPMP_IPC_CH_MAP_SIZE) {
		ERROR("%s: carveout size is not sufficient\n", __func__);
		return -EINVAL;
	}

	error = tegra_ivc_init(&ivc_ccplex_bpmp_channel,
				(uint32_t)TEGRA_BPMP_IPC_RX_PHYS_BASE,
				(uint32_t)TEGRA_BPMP_IPC_TX_PHYS_BASE,
				1U, frame_size, tegra_bpmp_ivc_notify);
	if (error != 0) {

		ERROR("%s: IVC init failed (%d)\n", __func__, error);

	} else {

		/* reset channel */
		tegra_ivc_channel_reset(&ivc_ccplex_bpmp_channel);

		/* wait for notification from BPMP */
		while (tegra_ivc_channel_notified(&ivc_ccplex_bpmp_channel) != 0) {
			/*
			 * Interrupt BPMP with doorbell each time after
			 * tegra_ivc_channel_notified() returns non zero
			 * value.
			 */
			tegra_bpmp_ring_bpmp_doorbell();
		}

		INFO("%s: All communication channels initialized\n", __func__);
	}

	return error;
}

/* Handler to reset a hardware module */
int32_t tegra_bpmp_ipc_reset_module(uint32_t rst_id)
{
	int32_t ret;
	struct mrq_reset_request req = {
		.cmd = (uint32_t)CMD_RESET_MODULE,
		.reset_id = rst_id
	};

	/* only GPCDMA/XUSB_PADCTL resets are supported */
	assert((rst_id == TEGRA_RESET_ID_XUSB_PADCTL) ||
	       (rst_id == TEGRA_RESET_ID_GPCDMA));

	ret = tegra_bpmp_ipc_send_req_atomic(MRQ_RESET, &req,
			(uint32_t)sizeof(req), NULL, 0);
	if (ret != 0) {
		ERROR("%s: failed for module %d with error %d\n", __func__,
		      rst_id, ret);
	}

	return ret;
}

int tegra_bpmp_ipc_enable_clock(uint32_t clk_id)
{
	int ret;
	struct mrq_clk_request req;

	/* only SE clocks are supported */
	if (clk_id != TEGRA_CLK_SE) {
		return -ENOTSUP;
	}

	/* prepare the MRQ_CLK command */
	req.cmd_and_id = make_mrq_clk_cmd(CMD_CLK_ENABLE, clk_id);

	ret = tegra_bpmp_ipc_send_req_atomic(MRQ_CLK, &req, (uint32_t)sizeof(req),
			NULL, 0);
	if (ret != 0) {
		ERROR("%s: failed for module %d with error %d\n", __func__,
		      clk_id, ret);
	}

	return ret;
}

int tegra_bpmp_ipc_disable_clock(uint32_t clk_id)
{
	int ret;
	struct mrq_clk_request req;

	/* only SE clocks are supported */
	if (clk_id != TEGRA_CLK_SE) {
		return -ENOTSUP;
	}

	/* prepare the MRQ_CLK command */
	req.cmd_and_id = make_mrq_clk_cmd(CMD_CLK_DISABLE, clk_id);

	ret = tegra_bpmp_ipc_send_req_atomic(MRQ_CLK, &req, (uint32_t)sizeof(req),
			NULL, 0);
	if (ret != 0) {
		ERROR("%s: failed for module %d with error %d\n", __func__,
		      clk_id, ret);
	}

	return ret;
}
