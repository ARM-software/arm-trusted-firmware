/*
 * Copyright (c) 2025, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <bl31/ehf.h>
#include <common/debug.h>
#include <plat/common/platform.h>
#include <services/sdei.h>

#include <platform_def.h>

int arm_validate_ns_entrypoint(uintptr_t entrypoint)
{
	int ret;
	uintptr_t base = BL31_BASE;
	uintptr_t limit = BL31_LIMIT;

	ret = ((entrypoint < base) || (entrypoint > limit)) ? 0 : -1;
	return ret;
}

/* Private event mappings */
static sdei_ev_map_t versal_net_sdei_private[] = {
	SDEI_DEFINE_EVENT_0(VERSAL_NET_SDEI_SGI_PRIVATE),
	SDEI_PRIVATE_EVENT(VERSAL_NET_SDEI_PRV_EV, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
};

/* Shared event mappings */
static sdei_ev_map_t versal_net_sdei_shared[] = {
	SDEI_SHARED_EVENT(VERSAL_NET_SDEI_SH_EVENT_0, SDEI_DYN_IRQ, SDEI_MAPF_DYNAMIC),
};

void plat_sdei_setup(void)
{
	INFO("SDEI platform setup\n");
}

/* Export ARM SDEI events */
REGISTER_SDEI_MAP(versal_net_sdei_private, versal_net_sdei_shared);
