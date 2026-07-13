/*
 * Copyright 2026 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>
#include <stdint.h>

#include <common/debug.h>
#include <drivers/scmi-msg.h>
#include <drivers/scmi.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <platform_def.h>
#include <s32g_scmi.h>

static struct scmi_msg_channel scmi_channel[S32G_SCMI_AGENT_COUNT] = {
	[S32G_SCMI_AGENT_OSPM] = {
		.shm_addr = S32G_SCMI_SHMEM_BASE,
		.shm_size = S32G_SCMI_SHMEM_SIZE,
		.busy = false,
	},
};

struct scmi_msg_channel *plat_scmi_get_channel(unsigned int agent_id)
{
	if (agent_id >= ARRAY_SIZE(scmi_channel)) {
		return NULL;
	}

	return &scmi_channel[agent_id];
}

/*
 * Protocols served by this SCMI server, in addition to the Base protocol which
 * is always implicitly present. The list is null-terminated. Entries are added
 * as the corresponding backends are implemented.
 */
static const uint8_t s32g_scmi_protocols[] = {
	0U /* Null termination */
};

const char *plat_scmi_vendor_name(void)
{
	return "NXP";
}

const char *plat_scmi_sub_vendor_name(void)
{
	return "S32G274A";
}

size_t plat_scmi_protocol_count(void)
{
	return ARRAY_SIZE(s32g_scmi_protocols) - 1U;
}

const uint8_t *plat_scmi_protocol_list(unsigned int agent_id __unused)
{
	return s32g_scmi_protocols;
}

void s32g_scmi_setup(void)
{
	size_t i;
	int ret;

	/*
	 * Map the SMT buffer as non-cacheable so that writes are immediately
	 * visible to the non-secure agent, which accesses this "no-map" region
	 * uncached.
	 */
	ret = mmap_add_dynamic_region(S32G_SCMI_SHMEM_BASE, S32G_SCMI_SHMEM_BASE,
				      PAGE_SIZE,
				      MT_NON_CACHEABLE | MT_RW | MT_SECURE);
	if (ret != 0) {
		ERROR("Failed to map the SCMI shared memory\n");
		panic();
	}

	for (i = 0U; i < ARRAY_SIZE(scmi_channel); i++) {
		scmi_smt_init_agent_channel(&scmi_channel[i]);
	}
}
