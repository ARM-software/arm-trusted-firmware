/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_SIP_CALLS_H
#define PLAT_SIP_CALLS_H

/*******************************************************************************
 * Plat SiP function constants
 ******************************************************************************/
#define MTK_PLAT_SIP_NUM_CALLS	6

#define MTK_SIP_PWR_ON_MTCMOS			0x82000402
#define MTK_SIP_PWR_OFF_MTCMOS			0x82000403
#define MTK_SIP_PWR_MTCMOS_SUPPORT		0x82000404
#define MTK_SIP_SET_HDCP_KEY_NUM		0x82000405
#define MTK_SIP_CLR_HDCP_KEY			0x82000406
#define MTK_SIP_SET_HDCP_KEY_EX			0x82000407

#endif /* PLAT_SIP_CALLS_H */
