/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RNG_PLAT_H
#define RNG_PLAT_H

#define TRNG_TIME_OUT		1000
#define MTK_TRNG_MAX_ROUND	4

/*******************************************************************************
 * TRNG related constants
 ******************************************************************************/
#define TRNG_BASE_SIZE		0x1000
#define TRNG_CTRL		(TRNG_BASE + 0x0000)
#define TRNG_TIME		(TRNG_BASE + 0x0004)
#define TRNG_DATA		(TRNG_BASE + 0x0008)
#define TRNG_CONF		(TRNG_BASE + 0x000C)
#define TRNG_CTRL_RDY		0x80000000
#define TRNG_CTRL_START		0x00000001
#define TRNG_CONF_VON_EN	0x00000020
#define TRNG_PDN_BASE_SIZE	0x1000
#define TRNG_PDN_SET		(INFRACFG_AO_BASE + 0x0088)
#define TRNG_PDN_CLR		(INFRACFG_AO_BASE + 0x008C)
#define TRNG_PDN_STATUS		(INFRACFG_AO_BASE + 0x0094)
#define TRNG_PDN_VALUE		0x200

#endif /* RNG_PLAT_H */
