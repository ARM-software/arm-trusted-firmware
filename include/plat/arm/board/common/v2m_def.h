/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __V2M_DEF_H__
#define __V2M_DEF_H__

#include <arm_xlat_tables.h>


/* V2M motherboard system registers & offsets */
#define V2M_SYSREGS_BASE		0x1c010000
#define V2M_SYS_ID			0x0
#define V2M_SYS_SWITCH			0x4
#define V2M_SYS_LED			0x8
#define V2M_SYS_NVFLAGS			0x38
#define V2M_SYS_NVFLAGSSET		0x38
#define V2M_SYS_NVFLAGSCLR		0x3c
#define V2M_SYS_CFGDATA			0xa0
#define V2M_SYS_CFGCTRL			0xa4
#define V2M_SYS_CFGSTATUS		0xa8

#define V2M_CFGCTRL_START		(1 << 31)
#define V2M_CFGCTRL_RW			(1 << 30)
#define V2M_CFGCTRL_FUNC_SHIFT		20
#define V2M_CFGCTRL_FUNC(fn)		(fn << V2M_CFGCTRL_FUNC_SHIFT)
#define V2M_FUNC_CLK_GEN		0x01
#define V2M_FUNC_TEMP			0x04
#define V2M_FUNC_DB_RESET		0x05
#define V2M_FUNC_SCC_CFG		0x06
#define V2M_FUNC_SHUTDOWN		0x08
#define V2M_FUNC_REBOOT			0x09

/*
 * V2M sysled bit definitions. The values written to this
 * register are defined in arch.h & runtime_svc.h. Only
 * used by the primary cpu to diagnose any cold boot issues.
 *
 * SYS_LED[0]   - Security state (S=0/NS=1)
 * SYS_LED[2:1] - Exception Level (EL3-EL0)
 * SYS_LED[7:3] - Exception Class (Sync/Async & origin)
 *
 */
#define V2M_SYS_LED_SS_SHIFT		0x0
#define V2M_SYS_LED_EL_SHIFT		0x1
#define V2M_SYS_LED_EC_SHIFT		0x3

#define V2M_SYS_LED_SS_MASK		0x1
#define V2M_SYS_LED_EL_MASK		0x3
#define V2M_SYS_LED_EC_MASK		0x1f

/* V2M sysid register bits */
#define V2M_SYS_ID_REV_SHIFT		28
#define V2M_SYS_ID_HBI_SHIFT		16
#define V2M_SYS_ID_BLD_SHIFT		12
#define V2M_SYS_ID_ARCH_SHIFT		8
#define V2M_SYS_ID_FPGA_SHIFT		0

#define V2M_SYS_ID_REV_MASK		0xf
#define V2M_SYS_ID_HBI_MASK		0xfff
#define V2M_SYS_ID_BLD_MASK		0xf
#define V2M_SYS_ID_ARCH_MASK		0xf
#define V2M_SYS_ID_FPGA_MASK		0xff

#define V2M_SYS_ID_BLD_LENGTH		4


/* NOR Flash */
#define V2M_FLASH0_BASE			0x08000000
#define V2M_FLASH0_SIZE			0x04000000
#define V2M_FLASH_BLOCK_SIZE		0x00040000	/* 256 KB */

#define V2M_IOFPGA_BASE			0x1c000000
#define V2M_IOFPGA_SIZE			0x03000000

/* PL011 UART related constants */
#define V2M_IOFPGA_UART0_BASE		0x1c090000
#define V2M_IOFPGA_UART1_BASE		0x1c0a0000
#define V2M_IOFPGA_UART2_BASE		0x1c0b0000
#define V2M_IOFPGA_UART3_BASE		0x1c0c0000

#define V2M_IOFPGA_UART0_CLK_IN_HZ	24000000
#define V2M_IOFPGA_UART1_CLK_IN_HZ	24000000
#define V2M_IOFPGA_UART2_CLK_IN_HZ	24000000
#define V2M_IOFPGA_UART3_CLK_IN_HZ	24000000

/* SP804 timer related constants */
#define V2M_SP804_TIMER0_BASE		0x1C110000
#define V2M_SP804_TIMER1_BASE		0x1C120000

/* SP810 controller */
#define V2M_SP810_BASE			0x1c020000
#define V2M_SP810_CTRL_TIM0_SEL		(1 << 15)
#define V2M_SP810_CTRL_TIM1_SEL		(1 << 17)
#define V2M_SP810_CTRL_TIM2_SEL		(1 << 19)
#define V2M_SP810_CTRL_TIM3_SEL		(1 << 21)

/*
 * The flash can be mapped either as read-only or read-write.
 *
 * If it is read-write then it should also be mapped as device memory because
 * NOR flash programming involves sending a fixed, ordered sequence of commands.
 *
 * If it is read-only then it should also be mapped as:
 * - Normal memory, because reading from NOR flash is transparent, it is like
 *   reading from RAM.
 * - Non-executable by default. If some parts of the flash need to be executable
 *   then platform code is responsible for re-mapping the appropriate portion
 *   of it as executable.
 */
#define V2M_MAP_FLASH0_RW		MAP_REGION_FLAT(V2M_FLASH0_BASE,\
						V2M_FLASH0_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

#define V2M_MAP_FLASH0_RO		MAP_REGION_FLAT(V2M_FLASH0_BASE,\
						V2M_FLASH0_SIZE,	\
						MT_RO_DATA | MT_SECURE)

#define V2M_MAP_IOFPGA			MAP_REGION_FLAT(V2M_IOFPGA_BASE,\
						V2M_IOFPGA_SIZE,		\
						MT_DEVICE | MT_RW | MT_SECURE)

/* Region equivalent to V2M_MAP_IOFPGA suitable for mapping at EL0 */
#define V2M_MAP_IOFPGA_EL0		MAP_REGION_FLAT(		\
						V2M_IOFPGA_BASE,	\
						V2M_IOFPGA_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE | MT_USER)


#endif /* __V2M_DEF_H__ */
