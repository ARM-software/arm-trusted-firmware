/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2024, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <hoya_def.h>

/*----------------------------------------------------------------------------*/
/* UART related constants. */
/*----------------------------------------------------------------------------*/
#define PLAT_QTI_UART_BASE			0x880000

/*----------------------------------------------------------------------------*/
/* Peripherals base addresses */
/*----------------------------------------------------------------------------*/
#define QTI_SEC_PRNG_BASE			0x790000

/*----------------------------------------------------------------------------*/
/* Device address space for mapping. Excluding starting 4K */
/*----------------------------------------------------------------------------*/
#define QTI_DEVICE_BASE				0x1000
#define QTI_DEVICE_SIZE				(0x80000000 - QTI_DEVICE_BASE)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL31 at DDR as per memory map. BL31_BASE is calculated using the
 * current BL31 debug size plus a little space for growth.
 */
#define BL31_BASE				0x86200000
#define BL31_SIZE				0x00100000
#define BL31_LIMIT				(BL31_BASE + BL31_SIZE)

/*----------------------------------------------------------------------------*/
/* AOP CMD DB  address space for mapping */
/*----------------------------------------------------------------------------*/
#define QTI_AOP_CMD_DB_BASE			0x85F20000
#define QTI_AOP_CMD_DB_SIZE			0x00020000
/*----------------------------------------------------------------------------*/
/* SMEM base address */
/*----------------------------------------------------------------------------*/
#define QTI_SMEM_BASE				0x86000000
#define QTI_SMEM_SIZE				0x00200000
/*----------------------------------------------------------------------------*/
/* LC PON register offsets */
/*----------------------------------------------------------------------------*/
#define PON_PS_HOLD_RESET_CTL			0x85a
#define PON_PS_HOLD_RESET_CTL2			0x85b

/*----------------------------------------------------------------------------*/
/* Chipset specific NOC error interrupt IDs.                                  */
/*----------------------------------------------------------------------------*/
#define PLAT_INT_ID_A1_NOC_ERROR		(0x18B)
#define PLAT_INT_ID_SYSTEM_NOC_ERROR		(0xC6)

#endif /* PLATFORM_DEF_H */
