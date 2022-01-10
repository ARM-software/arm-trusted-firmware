/*
 * Copyright (c) 2020-2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "mhu_v2_x.h"

#define MHU_V2_X_MAX_CHANNELS		124
#define MHU_V2_1_MAX_CHCOMB_INT		4
#define ENABLE				0x1
#define DISABLE				0x0
#define CLEAR_INTR			0x1
#define CH_PER_CH_COMB			0x20
#define SEND_FRAME(p_mhu)		((struct mhu_v2_x_send_frame_t *)p_mhu)
#define RECV_FRAME(p_mhu)		((struct mhu_v2_x_recv_frame_t *)p_mhu)

#define MHU_MAJOR_REV_V2		0x1u
#define MHU_MINOR_REV_2_0		0x0u
#define MHU_MINOR_REV_2_1		0x1u

struct mhu_v2_x_send_ch_window_t {
	/* Offset: 0x00 (R/ ) Channel Status */
	volatile uint32_t ch_st;
	/* Offset: 0x04 (R/ ) Reserved */
	volatile uint32_t reserved_0;
	/* Offset: 0x08 (R/ ) Reserved */
	volatile uint32_t reserved_1;
	/* Offset: 0x0C ( /W) Channel Set */
	volatile uint32_t ch_set;
	/* Offset: 0x10 (R/ ) Channel Interrupt Status (Reserved in 2.0) */
	volatile uint32_t ch_int_st;
	/* Offset: 0x14 ( /W) Channel Interrupt Clear  (Reserved in 2.0) */
	volatile uint32_t ch_int_clr;
	/* Offset: 0x18 (R/W) Channel Interrupt Enable (Reserved in 2.0) */
	volatile uint32_t ch_int_en;
	/* Offset: 0x1C (R/ ) Reserved */
	volatile uint32_t reserved_2;
};

struct mhu_v2_x_send_frame_t {
	/* Offset: 0x000 ( / ) Sender Channel Window 0 -123 */
	struct mhu_v2_x_send_ch_window_t send_ch_window[MHU_V2_X_MAX_CHANNELS];
	/* Offset: 0xF80 (R/ ) Message Handling Unit Configuration */
	volatile uint32_t mhu_cfg;
	/* Offset: 0xF84 (R/W) Response Configuration */
	volatile uint32_t resp_cfg;
	/* Offset: 0xF88 (R/W) Access Request */
	volatile uint32_t access_request;
	/* Offset: 0xF8C (R/ ) Access Ready */
	volatile uint32_t access_ready;
	/* Offset: 0xF90 (R/ ) Interrupt Status */
	volatile uint32_t int_st;
	/* Offset: 0xF94 ( /W) Interrupt Clear */
	volatile uint32_t int_clr;
	/* Offset: 0xF98 (R/W) Interrupt Enable */
	volatile uint32_t int_en;
	/* Offset: 0xF9C (R/ ) Reserved */
	volatile uint32_t reserved_0;
	/* Offset: 0xFA0 (R/W) Channel Combined IRQ Stat (Reserved in 2.0) */
	volatile uint32_t ch_comb_int_st[MHU_V2_1_MAX_CHCOMB_INT];
	/* Offset: 0xFC4 (R/ ) Reserved */
	volatile uint32_t reserved_1[6];
	/* Offset: 0xFC8 (R/ ) Implementer Identification Register */
	volatile uint32_t iidr;
	/* Offset: 0xFCC (R/ ) Architecture Identification Register */
	volatile uint32_t aidr;
	/* Offset: 0xFD0 (R/ )  */
	volatile uint32_t pid_1[4];
	/* Offset: 0xFE0 (R/ )  */
	volatile uint32_t pid_0[4];
	/* Offset: 0xFF0 (R/ )  */
	volatile uint32_t cid[4];
};

struct mhu_v2_x_rec_ch_window_t {
	/* Offset: 0x00 (R/ ) Channel Status */
	volatile uint32_t ch_st;
	/* Offset: 0x04 (R/ ) Channel Status Masked */
	volatile uint32_t ch_st_msk;
	/* Offset: 0x08 ( /W) Channel Clear */
	volatile uint32_t ch_clr;
	/* Offset: 0x0C (R/ ) Reserved */
	volatile uint32_t reserved_0;
	/* Offset: 0x10 (R/ ) Channel Mask Status */
	volatile uint32_t ch_msk_st;
	/* Offset: 0x14 ( /W) Channel Mask Set */
	volatile uint32_t ch_msk_set;
	/* Offset: 0x18 ( /W) Channel Mask Clear */
	volatile uint32_t ch_msk_clr;
	/* Offset: 0x1C (R/ ) Reserved */
	volatile uint32_t reserved_1;
};

struct mhu_v2_x_recv_frame_t {
	/* Offset: 0x000 ( / ) Receiver Channel Window 0 -123 */
	struct mhu_v2_x_rec_ch_window_t rec_ch_window[MHU_V2_X_MAX_CHANNELS];
	/* Offset: 0xF80 (R/ ) Message Handling Unit Configuration */
	volatile uint32_t mhu_cfg;
	/* Offset: 0xF84 (R/ ) Reserved */
	volatile uint32_t reserved_0[3];
	/* Offset: 0xF90 (R/ ) Interrupt Status (Reserved in 2.0) */
	volatile uint32_t int_st;
	/* Offset: 0xF94 (R/ ) Interrupt Clear  (Reserved in 2.0) */
	volatile uint32_t int_clr;
	/* Offset: 0xF98 (R/W) Interrupt Enable (Reserved in 2.0) */
	volatile uint32_t int_en;
	/* Offset: 0xF9C (R/ ) Reserved  */
	volatile uint32_t reserved_1;
	/* Offset: 0xFA0 (R/ ) Channel Combined IRQ Stat (Reserved in 2.0) */
	volatile uint32_t ch_comb_int_st[MHU_V2_1_MAX_CHCOMB_INT];
	/* Offset: 0xFB0 (R/ ) Reserved */
	volatile uint32_t reserved_2[6];
	/* Offset: 0xFC8 (R/ ) Implementer Identification Register */
	volatile uint32_t iidr;
	/* Offset: 0xFCC (R/ ) Architecture Identification Register */
	volatile uint32_t aidr;
	/* Offset: 0xFD0 (R/ )  */
	volatile uint32_t pid_1[4];
	/* Offset: 0xFE0 (R/ )  */
	volatile uint32_t pid_0[4];
	/* Offset: 0xFF0 (R/ )  */
	volatile uint32_t cid[4];
};

union mhu_v2_x_frame {
	struct mhu_v2_x_send_frame_t send_frame;
	struct mhu_v2_x_recv_frame_t recv_frame;
};

enum mhu_v2_x_error_t mhu_v2_x_driver_init(struct mhu_v2_x_dev_t *dev,
	 enum mhu_v2_x_supported_revisions rev)
{
	uint32_t AIDR = 0;
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (dev->is_initialized) {
		return MHU_V_2_X_ERR_ALREADY_INIT;
	}

	if (rev == MHU_REV_READ_FROM_HW) {
		/* Read revision from HW */
		if (dev->frame == MHU_V2_X_RECEIVER_FRAME) {
			AIDR = p_mhu->recv_frame.aidr;
		} else {
			AIDR = p_mhu->send_frame.aidr;
		}

		/* Get bits 7:4 to read major revision */
		if (((AIDR >> 4) & 0b1111) != MHU_MAJOR_REV_V2) {
			/* Unsupported MHU version */
			return MHU_V_2_X_ERR_UNSUPPORTED_VERSION;
		} /* No need to save major version, driver only supports MHUv2 */

		/* Get bits 3:0 to read minor revision */
		dev->subversion = AIDR & 0b1111;

		if (dev->subversion != MHU_MINOR_REV_2_0 &&
			dev->subversion != MHU_MINOR_REV_2_1) {
			/* Unsupported subversion */
			return MHU_V_2_X_ERR_UNSUPPORTED_VERSION;
		}
	} else {
		/* Revisions were provided by caller */
		if (rev == MHU_REV_2_0) {
			dev->subversion = MHU_MINOR_REV_2_0;
		} else if (rev == MHU_REV_2_1) {
			dev->subversion = MHU_MINOR_REV_2_1;
		} else {
			/* Unsupported subversion */
			return MHU_V_2_X_ERR_UNSUPPORTED_VERSION;
		} /* No need to save major version, driver only supports MHUv2 */
	}

	dev->is_initialized = true;

	return MHU_V_2_X_ERR_NONE;
}

uint32_t mhu_v2_x_get_num_channel_implemented(const struct mhu_v2_x_dev_t *dev)
{
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (!(dev->is_initialized)) {
		return MHU_V_2_X_ERR_NOT_INIT;
	}

	if (dev->frame == MHU_V2_X_SENDER_FRAME) {
		return (SEND_FRAME(p_mhu))->mhu_cfg;
	} else {
		assert(dev->frame == MHU_V2_X_RECEIVER_FRAME);
		return (RECV_FRAME(p_mhu))->mhu_cfg;
	}
}

enum mhu_v2_x_error_t mhu_v2_x_channel_send(const struct mhu_v2_x_dev_t *dev,
	 uint32_t channel, uint32_t val)
{
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (!(dev->is_initialized)) {
		return MHU_V_2_X_ERR_NOT_INIT;
	}

	if (dev->frame == MHU_V2_X_SENDER_FRAME) {
		(SEND_FRAME(p_mhu))->send_ch_window[channel].ch_set = val;
		return MHU_V_2_X_ERR_NONE;
	} else {
		return MHU_V_2_X_ERR_INVALID_ARG;
	}
}

enum mhu_v2_x_error_t mhu_v2_x_channel_poll(const struct mhu_v2_x_dev_t *dev,
	 uint32_t channel, uint32_t *value)
{
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (!(dev->is_initialized)) {
		return MHU_V_2_X_ERR_NOT_INIT;
	}

	if (dev->frame == MHU_V2_X_SENDER_FRAME) {
		*value = (SEND_FRAME(p_mhu))->send_ch_window[channel].ch_st;
		return MHU_V_2_X_ERR_NONE;
	} else {
		return MHU_V_2_X_ERR_INVALID_ARG;
	}
}

enum mhu_v2_x_error_t mhu_v2_x_channel_clear(const struct mhu_v2_x_dev_t *dev,
	 uint32_t channel)
{
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (!(dev->is_initialized)) {
		return MHU_V_2_X_ERR_NOT_INIT;
	}

	if (dev->frame == MHU_V2_X_RECEIVER_FRAME) {
		(RECV_FRAME(p_mhu))->rec_ch_window[channel].ch_clr = UINT32_MAX;
		return MHU_V_2_X_ERR_NONE;
	} else {
		return MHU_V_2_X_ERR_INVALID_ARG;
	}
}

enum mhu_v2_x_error_t mhu_v2_x_channel_receive(
	 const struct mhu_v2_x_dev_t *dev, uint32_t channel, uint32_t *value)
{
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (!(dev->is_initialized)) {
		return MHU_V_2_X_ERR_NOT_INIT;
	}

	if (dev->frame == MHU_V2_X_RECEIVER_FRAME) {
		*value = (RECV_FRAME(p_mhu))->rec_ch_window[channel].ch_st;
		return MHU_V_2_X_ERR_NONE;
	} else {
		return MHU_V_2_X_ERR_INVALID_ARG;
	}
}

enum mhu_v2_x_error_t mhu_v2_x_channel_mask_set(
	 const struct mhu_v2_x_dev_t *dev, uint32_t channel, uint32_t mask)
{
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (!(dev->is_initialized)) {
		return MHU_V_2_X_ERR_NOT_INIT;
	}

	if (dev->frame == MHU_V2_X_RECEIVER_FRAME) {
		(RECV_FRAME(p_mhu))->rec_ch_window[channel].ch_msk_set = mask;
		return MHU_V_2_X_ERR_NONE;
	} else {
		return MHU_V_2_X_ERR_INVALID_ARG;
	}
}

enum mhu_v2_x_error_t mhu_v2_x_channel_mask_clear(
	 const struct mhu_v2_x_dev_t *dev, uint32_t channel, uint32_t mask)
{
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (!(dev->is_initialized)) {
		return MHU_V_2_X_ERR_NOT_INIT;
	}

	if (dev->frame == MHU_V2_X_RECEIVER_FRAME) {
		(RECV_FRAME(p_mhu))->rec_ch_window[channel].ch_msk_clr = mask;
		return MHU_V_2_X_ERR_NONE;
	} else {
		return MHU_V_2_X_ERR_INVALID_ARG;
	}
}
enum mhu_v2_x_error_t mhu_v2_x_initiate_transfer(
	 const struct mhu_v2_x_dev_t *dev)
{
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (!(dev->is_initialized)) {
		return MHU_V_2_X_ERR_NOT_INIT;
	}

	if (dev->frame != MHU_V2_X_SENDER_FRAME) {
		return MHU_V_2_X_ERR_INVALID_ARG;
	}

	(SEND_FRAME(p_mhu))->access_request = ENABLE;

	while (!((SEND_FRAME(p_mhu))->access_ready)) {
		/* Wait in a loop for access ready signal to be high */
		;
	}

	return MHU_V_2_X_ERR_NONE;
}

enum mhu_v2_x_error_t mhu_v2_x_close_transfer(const struct mhu_v2_x_dev_t *dev)
{
	union mhu_v2_x_frame *p_mhu;

	assert(dev != NULL);

	p_mhu = (union mhu_v2_x_frame *)dev->base;

	if (!(dev->is_initialized)) {
		return MHU_V_2_X_ERR_NOT_INIT;
	}

	if (dev->frame != MHU_V2_X_SENDER_FRAME) {
		return MHU_V_2_X_ERR_INVALID_ARG;
	}

	(SEND_FRAME(p_mhu))->access_request = DISABLE;

	return MHU_V_2_X_ERR_NONE;
}
