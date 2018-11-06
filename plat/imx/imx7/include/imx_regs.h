/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IMX_REGS_H
#define IMX_REGS_H

/* Define the processor memory map */

#define OCRAM_S_ALIAS_BASE		0x00000000	/* CM4 Alias Code */
#define ROM_HIGH_BASE			0x00008000	/* ROM high 64k */
#define ROM_HIGH_PROT_BASE		0x00017000	/* ROM high 64k protected */
#define CAAM_BASE			0x00020000	/* CAAM block base address */
#define OCRAM_S_BASE			0x00180000	/* OCRAM_S  */
#define ROM_LOW_BASE			0x007f8000	/* ROM low 64k */
#define OCRAM_BASE			0x00900000	/* OCRAM base */
#define CM4_ALIAS_CODE_BASE		0x04000000	/* CM4 alias code */
#define TCM_BASE			0x1fff0000	/* TCM */
#define BOOTROM_CP_BASE			0x20020000	/* Boot ROM (all 96KB) */
#define CM4_ALIAS_SYSTEM_BASE		0x20100000	/* CM4 Alias system */
#define EIM_BASE			0x28000000	/* EIM */

/* BootROM absolute base address */
#define BOOTROM_BASE			0x00000000	/* BootROM */

/* Peripherals like GPIO live in the AIPS range */
#define AIPS1_BASE			0x30000000	/* AIPS1 */
#define AIPS2_BASE			0x30400000	/* AIPS2 */
#define AIPS3_BASE			0x30800000	/* AIPS3 */
#define AIPS4_BASE			0x30c00000	/* AIPS4 */

/* ARM peripherals like GIC */
#define ARM_PERIPHERAL_GIC_BASE		0x31000000	/* GIC */

/* Configuration ports */
#define GPV0_BASE			0x32000000	/* Main config port */
#define GPV1_BASE			0x32100000	/* Wakeup config port */
#define GPV2_BASE			0x32200000	/* Per_s config port */
#define GPV3_BASE			0x32300000	/* Per_m config port */
#define GPV4_BASE			0x32400000	/* Enet config port */
#define GPV5_BASE			0x32500000	/* Display config port */
#define GPV6_BASE			0x32600000	/* M4 conig port */

/* MMAP peripherals - like APBH DMA */
#define APBH_DMA_BASE			0x33000000	/* APBH DMA block */

/* QSPI RX BUFFERS */
#define QSPI_RX_BUFFER_BASE		0x34000000	/* QSPI RX buffers */

/* QSPI1 FLASH */
#define QSPI_FLASH_BASE			0x60000000	/* QSPI1 flash */

/* AIPS1 block addresses */
#define AIPSTZ_CONFIG_OFFSET		0x001f0000
#define CCM_BASE			(AIPS1_BASE + 0x380000)

/* Define the maximum number of UART blocks on this SoC */
#define MXC_UART1_BASE			(AIPS3_BASE + 0x060000)
#define MXC_UART2_BASE			(AIPS3_BASE + 0x070000)
#define MXC_UART3_BASE			(AIPS3_BASE + 0x080000)
#define MXC_UART4_BASE			(AIPS3_BASE + 0x260000)
#define MXC_UART5_BASE			(AIPS3_BASE + 0x270000)
#define MXC_UART6_BASE			(AIPS3_BASE + 0x280000)
#define MXC_UART7_BASE			(AIPS3_BASE + 0x290000)
#define MXC_MAX_UART_NUM		0x07

/* Define the maximum number of USDHCI blocks on this SoC */
#define MXC_MAX_USDHC_NUM		3

/* Define the number of CSU registers for this SoC */
#define MXC_MAX_CSU_REGS		0x40
#define CSU_BASE			(AIPS1_BASE + 0x3E0000)

/* IO Mux block base */
#define MXC_IO_MUXC_BASE		(AIPS1_BASE + 0x330000)

/* SNVS base */
#define SNVS_BASE			(AIPS1_BASE + 0x370000)

/* GP Timer base */
#define GPT1_BASE_ADDR			(AIPS1_BASE + 0x2d0000)

/* MMC base */
#define USDHC1_BASE			(AIPS1_BASE + 0xb40000)
#define USDHC2_BASE			(AIPS1_BASE + 0xb50000)
#define USDHC3_BASE			(AIPS1_BASE + 0xb60000)

/* Arm optional memory mapped counter module base address */
#define SYS_CNTCTL_BASE			(AIPS2_BASE + 0x2c0000)

/* Define CAAM AIPS offset */
#define CAAM_AIPS_BASE			(AIPS3_BASE + 0x100000)
#define CAAM_NUM_JOB_RINGS		0x03
#define CAAM_NUM_RTIC			0x04
#define CAAM_NUM_DECO			0x01

/* Define watchdog base addresses */
#define WDOG1_BASE			(AIPS1_BASE + 0x280000)
#define WDOG2_BASE			(AIPS1_BASE + 0x290000)
#define WDOG3_BASE			(AIPS1_BASE + 0x2A0000)
#define WDOG4_BASE			(AIPS1_BASE + 0x280000)

/* Define the maximum number of WDOG blocks on this SoC */
#define MXC_MAX_WDOG_NUM		0x04

#endif /* IMX_REGS_H */
