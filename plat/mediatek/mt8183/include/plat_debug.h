/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEBUG_H
#define PLATFORM_DEBUG_H

#define sync_writel(addr, val) \
	do { mmio_write_32((addr), (val)); dsbsy(); } while (0)

#define MCU_BIU_BASE			0x0c530000
#define MISC1_CFG_BASE			0xb00
#define CA15M_CFG_BASE			0x2000
#define DFD_INTERNAL_CTL		(MCU_BIU_BASE + MISC1_CFG_BASE + 0x00)
#define CA15M_DBG_CONTROL		(MCU_BIU_BASE + CA15M_CFG_BASE + 0x728)
#define CA15M_PWR_RST_CTL		(MCU_BIU_BASE + CA15M_CFG_BASE + 0x08)
#define VPROC_EXT_CTL			0x10006290

#define CFG_SF_CTRL			0x0c510014
#define CFG_SF_INI			0x0c510010

#define BIT_CA15M_L2PARITY_EN		(1 << 1)
#define BIT_CA15M_LASTPC_DIS		(1 << 8)

#define MCU_ALL_PWR_ON_CTRL		0x0c530b58
#define PLAT_MTK_CIRCULAR_BUFFER_UNLOCK	0xefab4133
#define PLAT_MTK_CIRCULAR_BUFFER_LOCK	0xefab4134

extern void circular_buffer_setup(void);
extern void l2c_parity_check_setup(void);
extern void clear_all_on_mux(void);
#endif /* PLATFORM_DEBUG_H */
