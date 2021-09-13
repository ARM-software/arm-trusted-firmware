/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_DEF_H
#define PLAT_DEF_H

#include <arch.h>
#include <cortex_a72.h>
/*
 * Required without TBBR.
 * To include the defines for DDR PHY
 * Images.
 */
#include <tbbr_img_def.h>

#include <policy.h>
#include <soc.h>


#define NXP_SYSCLK_FREQ		100000000
#define NXP_DDRCLK_FREQ		100000000

/* UART related definition */
#define NXP_CONSOLE_ADDR	NXP_UART_ADDR
#define NXP_CONSOLE_BAUDRATE	115200

#define NXP_SPD_EEPROM0		0x51

/* Size of cacheable stacks */
#if defined(IMAGE_BL2)
#if defined(TRUSTED_BOARD_BOOT)
#define PLATFORM_STACK_SIZE	0x2000
#else
#define PLATFORM_STACK_SIZE	0x1000
#endif
#elif defined(IMAGE_BL31)
#define PLATFORM_STACK_SIZE	0x1000
#endif

/* SD block buffer */
#define NXP_SD_BLOCK_BUF_SIZE	(0xC000)

#ifdef SD_BOOT
#define BL2_LIMIT		(NXP_OCRAM_ADDR + NXP_OCRAM_SIZE \
				- NXP_SD_BLOCK_BUF_SIZE)
#else
#define BL2_LIMIT		(NXP_OCRAM_ADDR + NXP_OCRAM_SIZE)
#endif
#define BL2_TEXT_LIMIT		(BL2_LIMIT)

/* IO defines as needed by IO driver framework */
#define MAX_IO_DEVICES		4
#define MAX_IO_BLOCK_DEVICES	1
#define MAX_IO_HANDLES		4

#define BL31_WDOG_SEC		89

/*
 * Define properties of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_LS_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(BL32_IRQ_SEC_PHY_TIMER, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE)

/* SGI 15 and Secure watchdog interrupts assigned to Group 0 */
#define PLAT_LS_G0_IRQ_PROPS(grp)	\
	INTR_PROP_DESC(BL31_WDOG_SEC, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_EDGE), \
	INTR_PROP_DESC(15, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)
#endif /* PLAT_DEF_H */
