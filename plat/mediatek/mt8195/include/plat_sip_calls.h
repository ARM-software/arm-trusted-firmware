/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SIP_CALLS_H
#define PLAT_SIP_CALLS_H

/*******************************************************************************
 * Plat SiP function constants
 ******************************************************************************/
#define MTK_PLAT_SIP_NUM_CALLS    4

/* DFD */
#define MTK_SIP_KERNEL_DFD_AARCH32	0x82000205
#define MTK_SIP_KERNEL_DFD_AARCH64	0xC2000205

/* DP/eDP */
#define MTK_SIP_DP_CONTROL_AARCH32	0x82000523
#define MTK_SIP_DP_CONTROL_AARCH64	0xC2000523

#endif /* PLAT_SIP_CALLS_H */
