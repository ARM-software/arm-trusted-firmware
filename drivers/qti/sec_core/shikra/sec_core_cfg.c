/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include <drivers/qti/sec_core/sec_core.h>
#include <lib/utils_def.h>
#include <sec_core_defs.h>

#define APSS_ALIAS_APC_SECURE(n)					\
	{ APSS_ALIAS_0_APC_SECURE_ADDR +				\
		  ((n) * APSS_ALIASn_APC_SECURE_OFFSET_TO_NEXT),	\
	  APSS_ALIAS_0_APC_SECURE_RMSK }

const struct sec_core_cfg_s qti_sec_core_cfg[] = {
	{APSS_WDT_TMR1_WDOG_SECURE_ADDR,	APSS_WDT_TMR1_WDOG_SECURE_RMSK},
	APSS_ALIAS_APC_SECURE(0),
	APSS_ALIAS_APC_SECURE(1),
	APSS_ALIAS_APC_SECURE(2),
	APSS_ALIAS_APC_SECURE(3),
	{APSS_CL_SECURE_ADDR,	APSS_CL_SECURE_RMSK},
	{APSS_BANKED_APC_SECURE_ADDR,	APSS_BANKED_APC_SECURE_RMSK},
	{APSS_SHARED_SHR_SECURE_ADDR,	0x0},
};

const size_t qti_sec_core_cfg_count = ARRAY_SIZE(qti_sec_core_cfg);

const uintptr_t qti_sec_core_rvbaraddr_lo =
	APSS_SHARED_KRYO_RVBARADDR_LO_ADDR;
const uintptr_t qti_sec_core_rvbaraddr_hi =
	APSS_SHARED_KRYO_RVBARADDR_HI_ADDR;
