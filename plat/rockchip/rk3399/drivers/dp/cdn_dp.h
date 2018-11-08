/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CDN_DP_H
#define CDN_DP_H

#include <plat_private.h>

enum {
	CDN_DP_HDCP_1X_KSV_LEN = 5,
	CDN_DP_HDCP_KSV_LEN = 8,
	CDN_DP_HDCP_RESERVED_LEN = 10,
	CDN_DP_HDCP_UID_LEN = 16,
	CDN_DP_HDCP_SHA_LEN = 20,
	CDN_DP_HDCP_DPK_LEN = 280,
	CDN_DP_HDCP_1X_KEYS_LEN	= 285,
	CDN_DP_HDCP_KEY_LEN = 326,
};

struct cdn_dp_hdcp_key_1x {
	uint8_t ksv[CDN_DP_HDCP_KSV_LEN];
	uint8_t device_key[CDN_DP_HDCP_DPK_LEN];
	uint8_t sha1[CDN_DP_HDCP_SHA_LEN];
	uint8_t uid[CDN_DP_HDCP_UID_LEN];
	uint16_t seed;
	uint8_t reserved[CDN_DP_HDCP_RESERVED_LEN];
};

#define HDCP_KEY_DATA_START_TRANSFER	0
#define HDCP_KEY_DATA_START_DECRYPT	1
#define HDCP_KEY_1X_STORE_DATA_ALIGN_SIZE	(6 * 64) / 8

/* Checks the cdn_dp_hdcp_key_1x must be aligned on 6 x 64-bit word boundary */
CASSERT(sizeof(struct cdn_dp_hdcp_key_1x) % HDCP_KEY_1X_STORE_DATA_ALIGN_SIZE, \
	assert_hdcp_key_1x_store_data_align_size_mismatch);

uint64_t dp_hdcp_ctrl(uint64_t type);

uint64_t dp_hdcp_store_key(uint64_t x1,
			   uint64_t x2,
			   uint64_t x3,
			   uint64_t x4,
			   uint64_t x5,
			   uint64_t x6);

#endif /* CDN_DP_H */
