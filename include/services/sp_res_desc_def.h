/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPM_RES_DESC_DEFS_H
#define SPM_RES_DESC_DEFS_H

#include <lib/utils_def.h>

/*******************************************************************************
 * Attribute Section
 ******************************************************************************/

#define RD_ATTR_TYPE_UP_MIGRATABLE	U(0)
#define RD_ATTR_TYPE_UP_PINNED		U(2)
#define RD_ATTR_TYPE_MP			U(1)

#define RD_ATTR_RUNTIME_SEL0		U(0)
#define RD_ATTR_RUNTIME_SEL1		U(1)

#define RD_ATTR_INIT_ONLY		U(0)
#define RD_ATTR_RUNTIME			U(1)

#define RD_ATTR_PANIC_RESTART		U(0)
#define RD_ATTR_PANIC_ONESHOT		U(1)

#define RD_ATTR_XLAT_GRANULE_4KB	U(0)
#define RD_ATTR_XLAT_GRANULE_16KB	U(1)
#define RD_ATTR_XLAT_GRANULE_64KB	U(2)

/*******************************************************************************
 * Memory Region Section
 ******************************************************************************/

#define RD_MEM_REGION_NAME_LEN		U(32)

#define RD_MEM_DEVICE			U(0)
#define RD_MEM_NORMAL_CODE		U(1)
#define RD_MEM_NORMAL_DATA		U(2)
#define RD_MEM_NORMAL_BSS		U(3)
#define RD_MEM_NORMAL_RODATA		U(4)
#define RD_MEM_NORMAL_SPM_SP_SHARED_MEM	U(5)
#define RD_MEM_NORMAL_CLIENT_SHARED_MEM	U(6)
#define RD_MEM_NORMAL_MISCELLANEOUS	U(7)

#define RD_MEM_MASK			U(15)

#define RD_MEM_IS_PIE			(U(1) << 4)

/*******************************************************************************
 * Notification Section
 ******************************************************************************/

#define RD_NOTIF_TYPE_PLATFORM		(U(0) << 31)
#define RD_NOTIF_TYPE_INTERRUPT		(U(1) << 31)

#define RD_NOTIF_PLAT_ID_MASK		U(0xFFFF)
#define RD_NOTIF_PLAT_ID_SHIFT		U(0)

#define RD_NOTIF_PLATFORM(id)						\
	(RD_NOTIF_TYPE_PLATFORM						\
	| (((id) & RD_NOTIF_PLAT_ID_MASK) << RD_NOTIF_PLAT_ID_SHIFT))

#define RD_NOTIF_IRQ_NUM_MASK		U(0xFFFF)
#define RD_NOTIF_IRQ_NUM_SHIFT		U(0)
#define RD_NOTIF_IRQ_PRIO_MASK		U(0xFF)
#define RD_NOTIF_IRQ_PRIO_SHIFT		U(16)

#define RD_NOTIF_IRQ_EDGE_FALLING	U(0)
#define RD_NOTIF_IRQ_EDGE_RISING	U(2)
#define RD_NOTIF_IRQ_LEVEL_LOW		U(1)
#define RD_NOTIF_IRQ_LEVEL_HIGH		U(3)
#define RD_NOTIF_IRQ_TRIGGER_SHIFT	U(24)

#define RD_NOTIF_IRQ(num, prio, trig)					\
	(RD_NOTIF_TYPE_IRQ						\
	| (((num) & RD_NOTIF_IRQ_NUM_MASK) << RD_NOTIF_IRQ_NUM_SHIFT)	\
	| (((prio) & RD_NOTIF_IRQ_PRIO_MASK) << RD_NOTIF_IRQ_PRIO_SHIFT) \
	| (((trig) << RD_NOTIF_IRQ_TRIGGER_SHIFT)))

/*******************************************************************************
 * Service Description Section
 ******************************************************************************/

#define RD_SERV_ACCESS_SECURE		(U(1) << 0)
#define RD_SERV_ACCESS_EL3		(U(1) << 1)
#define RD_SERV_ACCESS_NORMAL		(U(1) << 2)

#define RD_SERV_SUPPORT_BLOCKING	(U(1) << 0)
#define RD_SERV_SUPPORT_NON_BLOCKING	(U(1) << 0)

#endif /* SPM_RES_DESC_DEFS_H */
