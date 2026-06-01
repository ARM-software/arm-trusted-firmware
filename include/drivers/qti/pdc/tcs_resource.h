/*
 * Copyright (c) 2026 Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TCS_RESOURCE_H
#define TCS_RESOURCE_H

/* Number of commands per TCS (same for all TCS instances) */
#define NUM_COMMANDS_PER_TCS	4

/* TCS usage */
enum tcs_usage {
	TCS_NUM_SLEEP0        = 0,	/* TCS 0 - Sleep: CX retention */
	TCS_NUM_SLEEP1,			/* TCS 1 - Sleep: CX off */
	TCS_NUM_SLEEP2,			/* TCS 2 - Sleep: unused */
	TCS_NUM_TOTAL_SLEEPS,

	TCS_NUM_WAKE0 = TCS_NUM_TOTAL_SLEEPS,	/* TCS 3 - Wake: resources to MOL */

	TCS_NUM_TOTAL,
};

/* Resource index (matches order in g_pdc_resource_list) */
enum resource_index {
	RES_IDX_CX  = 0,
	RES_IDX_MX,
	RES_IDX_XO,
	RES_IDX_SOC,

	TCS_TOTAL_RESOURCE_NUM,
};

#endif /* TCS_RESOURCE_H */
