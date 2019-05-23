/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/css/scmi.h>

#include "scmi_private.h"
#include "scmi_sq.h"

#include <sq_common.h>

/* SCMI messge ID to get the available DRAM region */
#define SCMI_VENDOR_EXT_MEMINFO_GET_MSG		0x3

#define SCMI_VENDOR_EXT_MEMINFO_GET_MSG_LEN	4

/*
 * API to get the available DRAM region
 */
int scmi_get_draminfo(void *p, struct draminfo *info)
{
	mailbox_mem_t *mbx_mem;
	int token = 0, ret;
	scmi_channel_t *ch = (scmi_channel_t *)p;
	struct dram_info_resp response;

	validate_scmi_channel(ch);

	scmi_get_channel(ch);

	mbx_mem = (mailbox_mem_t *)(ch->info->scmi_mbx_mem);
	mbx_mem->msg_header = SCMI_MSG_CREATE(SCMI_SYS_VENDOR_EXT_PROTO_ID,
			SCMI_VENDOR_EXT_MEMINFO_GET_MSG, token);
	mbx_mem->len = SCMI_VENDOR_EXT_MEMINFO_GET_MSG_LEN;
	mbx_mem->flags = SCMI_FLAG_RESP_POLL;

	scmi_send_sync_command(ch);

	/*
	 * Ensure that any read to the SCPI payload area is done after reading
	 * the MHU register. If these 2 reads were reordered then the CPU would
	 * read invalid payload data
	 */
	dmbld();

	/* Get the return values */
	SCMI_PAYLOAD_RET_VAL1(mbx_mem->payload, ret);

	memcpy(&response, (void *)mbx_mem->payload, sizeof(response));

	scmi_put_channel(ch);

	*info = response.info;

	return ret;
}
