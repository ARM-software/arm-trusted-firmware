/*
 * Copyright (c) 2017-2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BPMP_IVC_H
#define BPMP_IVC_H

#include <lib/utils_def.h>
#include <stdint.h>
#include <stddef.h>

#define IVC_ALIGN		U(64)
#define IVC_CHHDR_TX_FIELDS	U(16)
#define IVC_CHHDR_RX_FIELDS	U(16)

struct ivc_channel_header;

struct ivc {
	struct ivc_channel_header *rx_channel;
	struct ivc_channel_header *tx_channel;
	uint32_t w_pos;
	uint32_t r_pos;
	void (*notify)(const struct ivc *);
	uint32_t nframes;
	uint32_t frame_size;
};

/* callback handler for notify on receiving a response */
typedef void (* ivc_notify_function)(const struct ivc *);

int32_t tegra_ivc_init(struct ivc *ivc, uintptr_t rx_base, uintptr_t tx_base,
		uint32_t nframes, uint32_t frame_size,
		ivc_notify_function notify);
size_t tegra_ivc_total_queue_size(size_t queue_size);
size_t tegra_ivc_align(size_t size);
int32_t tegra_ivc_channel_notified(struct ivc *ivc);
void tegra_ivc_channel_reset(const struct ivc *ivc);
int32_t tegra_ivc_write_advance(struct ivc *ivc);
void *tegra_ivc_write_get_next_frame(const struct ivc *ivc);
int32_t tegra_ivc_write(struct ivc *ivc, const void *buf, size_t size);
int32_t tegra_ivc_read_advance(struct ivc *ivc);
void *tegra_ivc_read_get_next_frame(const struct ivc *ivc);
int32_t tegra_ivc_read(struct ivc *ivc, void *buf, size_t max_read);
bool tegra_ivc_tx_empty(const struct ivc *ivc);
bool tegra_ivc_can_write(const struct ivc *ivc);
bool tegra_ivc_can_read(const struct ivc *ivc);

#endif /* BPMP_IVC_H */
