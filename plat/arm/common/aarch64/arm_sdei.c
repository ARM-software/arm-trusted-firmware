/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* SDEI configuration for ARM platforms */

#include <bl31/ehf.h>
#include <common/debug.h>
#include <services/sdei.h>

#if SDEI_IN_FCONF
#include <plat/arm/common/fconf_sdei_getter.h>
#endif
#include <plat/common/platform.h>
#include <platform_def.h>


#if SDEI_IN_FCONF
/* Private event mappings */
static sdei_ev_map_t arm_sdei_private[PLAT_SDEI_DP_EVENT_MAX_CNT + 1] = { 0 };

/* Shared event mappings */
static sdei_ev_map_t arm_sdei_shared[PLAT_SDEI_DS_EVENT_MAX_CNT] = { 0 };

void plat_sdei_setup(void)
{
	uint32_t i;

	arm_sdei_private[0] = (sdei_ev_map_t)SDEI_DEFINE_EVENT_0(ARM_SDEI_SGI);

	for (i = 0; i < FCONF_GET_PROPERTY(sdei, dyn_config, private_ev_cnt); i++) {
		arm_sdei_private[i + 1] = (sdei_ev_map_t)SDEI_PRIVATE_EVENT(
			FCONF_GET_PROPERTY(sdei, dyn_config, private_ev_nums[i]),
			FCONF_GET_PROPERTY(sdei, dyn_config, private_ev_intrs[i]),
			FCONF_GET_PROPERTY(sdei, dyn_config, private_ev_flags[i]));
	}

	for (i = 0; i < FCONF_GET_PROPERTY(sdei, dyn_config, shared_ev_cnt); i++) {
		arm_sdei_shared[i] = (sdei_ev_map_t)SDEI_SHARED_EVENT( \
			FCONF_GET_PROPERTY(sdei, dyn_config, shared_ev_nums[i]),
			FCONF_GET_PROPERTY(sdei, dyn_config, shared_ev_intrs[i]),
			FCONF_GET_PROPERTY(sdei, dyn_config, shared_ev_flags[i]));
	}
	INFO("FCONF: SDEI platform setup\n");
}
#else
/* Private event mappings */
static sdei_ev_map_t arm_sdei_private[] = {
	PLAT_ARM_PRIVATE_SDEI_EVENTS
};

/* Shared event mappings */
static sdei_ev_map_t arm_sdei_shared[] = {
	PLAT_ARM_SHARED_SDEI_EVENTS
};

void plat_sdei_setup(void)
{
	INFO("SDEI platform setup\n");
}
#endif /* SDEI_IN_FCONF */

/* Export ARM SDEI events */
REGISTER_SDEI_MAP(arm_sdei_private, arm_sdei_shared);
