/*
 * Copyright (C) 2015, Fuzhou Rockchip Electronics Co., Ltd
 * Author: Tony.Xie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SOC_H__
#define __SOC_H__

#ifndef BIT
#define BIT(nr)			(1 << (nr))
#endif

/*****************************************************************************
 * secure timer
 *****************************************************************************/
#define TIMER_LOADE_COUNT0	0x00
#define TIMER_LOADE_COUNT1	0x04
#define TIMER_CURRENT_VALUE0	0x08
#define TIMER_CURRENT_VALUE1	0x0C
#define TIMER_CONTROL_REG	0x10
#define TIMER_INTSTATUS		0x18

#define STIMER1_BASE		(STIME_BASE + 0x20)

/*****************************************************************************
 * sgrf reg, offset
 *****************************************************************************/
#define SGRF_SOC_CON(n)		(0x0 + (n) * 4)
#define SGRF_BUSDMAC_CON(n)	(0x100 + (n) * 4)
#define SGRF_SOC_CON(n)		(0x0 + (n) * 4)
#define SGRF_BUSDMAC_CON(n)	(0x100 + (n) * 4)

/*****************************************************************************
 * cru reg, offset
 *****************************************************************************/
#define CRU_SOFTRST_CON		0x300
#define CRU_SOFTRSTS_CON(n)	(CRU_SOFTRST_CON + ((n) * 4))

void platform_soc_init(void);

#endif /* __SOC_H__ */
