/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* resources that are going to stay in secure partition */
sc_rsrc_t secure_rsrcs[] = {
	SC_R_MU_0A,
	SC_R_A35,
	SC_R_A35_0,
	SC_R_A35_1,
	SC_R_A35_2,
	SC_R_A35_3,
	SC_R_GIC,
	SC_R_SYSTEM,
	SC_R_IRQSTR_SCU2,
	SC_R_GPT_0
};

/* resources that have register access for non-secure domain */
sc_rsrc_t ns_access_allowed[] = {
	SC_R_GIC,
	SC_R_GPT_0
};
