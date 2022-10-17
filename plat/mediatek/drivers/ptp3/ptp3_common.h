/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PTP3_COMMON_H
#define PTP3_COMMON_H

/* config enum */
enum PTP3_CFG {
	PTP3_CFG_ADDR,
	PTP3_CFG_VALUE,
	NR_PTP3_CFG,
};

/* prototype */
void ptp3_core_init(unsigned int core);
void ptp3_core_deinit(unsigned int core);

#endif /* PTP3_COMMON_H */
