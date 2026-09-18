/*
 * Copyright (c) 2026, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <common/debug.h>
#include <common/ep_info.h>

#include <pm_api_sys.h>
#include <pm_defs.h>
#include <pm_ipi.h>
#include <pm_tl_client.h>

/* Number of 32-bit response words returned by the PLM for the TL address. */
#define PM_TLBIN_ADDR_WORDS		2U

/*
 * Overrides the weak default in plat/amd/common/plat_xfer_list.c.
 *
 * Queries the PLM (loader module command PM_GET_TLBIN_LOAD_ADDR, 0x716) for
 * the runtime transfer list (tlbin partition) load address. The response
 * carries the 64-bit address in two words: response[0] holds the high
 * 32 bits and response[1] the low 32 bits.
 */
void plat_get_tl_fw_base_address(uintptr_t *tl_base)
{
	uint32_t payload[PAYLOAD_ARG_CNT];
	uint32_t response[PM_TLBIN_ADDR_WORDS] = {0};
	enum pm_ret_status status;

	assert(tl_base != NULL);

	PM_PACK_PAYLOAD1(payload, LOADER_MODULE_ID, SECURE, PM_GET_TLBIN_LOAD_ADDR);

	status = pm_ipi_send_sync(payload, response, PM_TLBIN_ADDR_WORDS);
	if (status == PM_RET_SUCCESS) {
		*tl_base = (uintptr_t)(((uint64_t)response[0] << 32) |
				       (uint64_t)response[1]);
	} else {
		WARN("Failed to query TL address from PLM, status: %u\n",
		     (unsigned int)status);
	}
}
