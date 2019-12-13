/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>
#include <stdlib.h>
#include <string.h>

#include <lib/smccc.h>

#include <cdn_dp.h>

__asm__(
	".pushsection .text.hdcp_handler, \"ax\", %progbits\n"
	".global hdcp_handler\n"
	".balign 4\n"
	"hdcp_handler:\n"
	".incbin \"" HDCPFW "\"\n"
	".type hdcp_handler, %function\n"
	".size hdcp_handler, .- hdcp_handler\n"
	".popsection\n"
);

static uint64_t *hdcp_key_pdata;
static struct cdn_dp_hdcp_key_1x key;

int hdcp_handler(struct cdn_dp_hdcp_key_1x *key);

uint64_t dp_hdcp_ctrl(uint64_t type)
{
	switch (type) {
	case HDCP_KEY_DATA_START_TRANSFER:
		memset(&key, 0x00, sizeof(key));
		hdcp_key_pdata = (uint64_t *)&key;
		return 0;
	case HDCP_KEY_DATA_START_DECRYPT:
		if (hdcp_key_pdata == (uint64_t *)(&key + 1))
			return hdcp_handler(&key);
		else
			return PSCI_E_INVALID_PARAMS;
		assert(0); /* Unreachable */
	default:
		return SMC_UNK;
	}
}

uint64_t dp_hdcp_store_key(uint64_t x1,
			   uint64_t x2,
			   uint64_t x3,
			   uint64_t x4,
			   uint64_t x5,
			   uint64_t x6)
{
	if (hdcp_key_pdata < (uint64_t *)&key ||
		hdcp_key_pdata + 6 > (uint64_t *)(&key + 1))
		return PSCI_E_INVALID_PARAMS;

	hdcp_key_pdata[0] = x1;
	hdcp_key_pdata[1] = x2;
	hdcp_key_pdata[2] = x3;
	hdcp_key_pdata[3] = x4;
	hdcp_key_pdata[4] = x5;
	hdcp_key_pdata[5] = x6;
	hdcp_key_pdata += 6;

	return 0;
}
