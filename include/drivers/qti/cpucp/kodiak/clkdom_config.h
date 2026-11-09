/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QTI_CPUCP_KODIAK_CLKDOM_CONFIG_H
#define QTI_CPUCP_KODIAK_CLKDOM_CONFIG_H

/*
 * In CPUCP SCMI clock ID order: the L3, the Cortex-A55 cores 0 to 3, the
 * Cortex-A78 cores 4 to 6 and the gold+ Cortex-A78 core 7.
 */
enum clock_domain_id {
	CD_L3,
	CD_SILVER,
	CD_GOLD,
	CD_GOLDPLUS,
	CD_MAX
};

#endif /* QTI_CPUCP_KODIAK_CLKDOM_CONFIG_H */
