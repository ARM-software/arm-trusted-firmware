/*
 * Copyright (C) 2018-2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef A3700_PLAT_DEF_H
#define A3700_PLAT_DEF_H

#include <marvell_def.h>


#define MVEBU_MAX_CPUS_PER_CLUSTER	2

#define MVEBU_PRIMARY_CPU		0x0

/*
 * The counter on A3700 is always fed from reference 25M clock (XTAL).
 * However minimal CPU counter prescaler is 2, so the counter
 * frequency will be divided by 2, the number is 12.5M
 */
#define COUNTER_FREQUENCY		12500000

#define MVEBU_REGS_BASE			0xD0000000

/*****************************************************************************
 * MVEBU memory map related constants
 *****************************************************************************
 */
/* Aggregate of all devices in the first GB */
#define DEVICE0_BASE			MVEBU_REGS_BASE
#define DEVICE0_SIZE			0x10000000

/*****************************************************************************
 * GIC-500 & interrupt handling related constants
 *****************************************************************************
 */
/* Base MVEBU compatible GIC memory map */
#define MVEBU_GICD_BASE			0x1D00000
#define MVEBU_GICR_BASE			0x1D40000
#define MVEBU_GICC_BASE			0x1D80000

/*
 * CCI-400 base address
 * This address is absolute, not relative to MVEBU_REGS_BASE.
 * This is not the default CCI base address (that would be 0xD8000000).
 * Rather we remap CCI to this address to better utilize the address space.
 * (The remapping is done in plat/marvell/armada/a3k/common/plat_cci.c)
 */
#define MVEBU_CCI_BASE			0xFE000000

/*****************************************************************************
 * North and south bridge reset registers
 *****************************************************************************
 */
#define MVEBU_NB_RESET_REG		(MVEBU_REGS_BASE + 0x12400)
#define   MVEBU_NB_RESET_I2C1_N		(1 << 0)
#define   MVEBU_NB_RESET_1WIRE_N	(1 << 1)
#define   MVEBU_NB_RESET_SPI_N		(1 << 2)
#define   MVEBU_NB_RESET_UART_N		(1 << 3)
#define   MVEBU_NB_RESET_XTL_N		(1 << 4)
#define   MVEBU_NB_RESET_I2C2_N		(1 << 5)
#define   MVEBU_NB_RESET_UART2_N	(1 << 6)
#define   MVEBU_NB_RESET_AVS_N		(1 << 7)
#define   MVEBU_NB_RESET_DDR_N		(1 << 10)
#define   MVEBU_NB_RESET_SETM_N		(1 << 11)
#define   MVEBU_NB_RESET_DMA_N		(1 << 12)
#define   MVEBU_NB_RESET_TSECM_N	(1 << 13)
#define   MVEBU_NB_RESET_SDIO_N		(1 << 14)
#define   MVEBU_NB_RESET_SATA_N		(1 << 15)
#define   MVEBU_NB_RESET_PWRMGT_N	(1 << 16)
#define   MVEBU_NB_RESET_OTP_N		(1 << 17)
#define   MVEBU_NB_RESET_EIP_N		(1 << 18)
#define MVEBU_SB_RESET_REG		(MVEBU_REGS_BASE + 0x18600)
#define   MVEBU_SB_RESET_MCIPHY		(1 << 1)
#define   MVEBU_SB_RESET_SDIO_N		(1 << 2)
#define   MVEBU_SB_RESET_PCIE_N		(1 << 3)
#define   MVEBU_SB_RESET_GBE1_N		(1 << 4)
#define   MVEBU_SB_RESET_GBE0_N		(1 << 5)
#define   MVEBU_SB_RESET_USB2PHY	(1 << 6)
#define   MVEBU_SB_RESET_USB2HPHY	(1 << 7)
#define   MVEBU_SB_RESET_MCI_N		(1 << 8)
#define   MVEBU_SB_RESET_PWRMGT_N	(1 << 9)
#define   MVEBU_SB_RESET_EBM_N		(1 << 10)
#define   MVEBU_SB_RESET_OTP_N		(1 << 11)

/*****************************************************************************
 * North and south bridge register base
 *****************************************************************************
 */
#define MVEBU_NB_REGS_BASE			(MVEBU_REGS_BASE + 0x13000)
#define MVEBU_SB_REGS_BASE			(MVEBU_REGS_BASE + 0x18000)

/*****************************************************************************
 * GPIO registers related constants
 *****************************************************************************
 */
/* North and south bridge GPIO register base address */
#define MVEBU_NB_GPIO_REG_BASE			(MVEBU_NB_REGS_BASE + 0x800)
#define MVEBU_NB_GPIO_IRQ_REG_BASE		(MVEBU_NB_REGS_BASE + 0xC00)
#define MVEBU_SB_GPIO_REG_BASE			(MVEBU_SB_REGS_BASE + 0x800)
#define MVEBU_SB_GPIO_IRQ_REG_BASE		(MVEBU_SB_REGS_BASE + 0xC00)
#define MVEBU_NB_SB_IRQ_REG_BASE		(MVEBU_REGS_BASE + 0x8A00)

/* North Bridge GPIO selection register */
#define MVEBU_NB_GPIO_SEL_REG			(MVEBU_NB_GPIO_REG_BASE + 0x30)
#define MVEBU_NB_GPIO_OUTPUT_EN_HIGH_REG	(MVEBU_NB_GPIO_REG_BASE + 0x04)
/* I2C1 GPIO Enable bit offset */
#define MVEBU_GPIO_TW1_GPIO_EN_OFF		(10)
/* SPI pins mode bit offset */
#define MVEBU_GPIO_NB_SPI_PIN_MODE_OFF		(28)

/*****************************************************************************
 * DRAM registers related constants
 *****************************************************************************
 */
#define MVEBU_DRAM_REG_BASE			(MVEBU_REGS_BASE)

/*****************************************************************************
 * SB wake-up registers related constants
 *****************************************************************************
 */
#define MVEBU_SB_WAKEUP_REG_BASE		(MVEBU_REGS_BASE + 0x19000)

/*****************************************************************************
 * PMSU registers related constants
 *****************************************************************************
 */
#define MVEBU_PMSU_REG_BASE			(MVEBU_REGS_BASE + 0x14000)

/*****************************************************************************
 * North Bridge Step-Down Registers
 *****************************************************************************
 */
#define MVEBU_NB_STEP_DOWN_REG_BASE		(MVEBU_REGS_BASE + 0x12800)

/*****************************************************************************
 * DRAM CS memory map register base
 *****************************************************************************
 */
#define MVEBU_CS_MMAP_REG_BASE			(MVEBU_REGS_BASE + 0x200)

/*****************************************************************************
 * CPU decoder window registers related constants
 *****************************************************************************
 */
#define MVEBU_CPU_DEC_WIN_REG_BASE		(MVEBU_REGS_BASE + 0xCF00)

/*****************************************************************************
 * AVS registers related constants
 *****************************************************************************
 */
#define MVEBU_AVS_REG_BASE			(MVEBU_REGS_BASE + 0x11500)


/*****************************************************************************
 * AVS registers related constants
 *****************************************************************************
 */
#define MVEBU_COMPHY_REG_BASE			(MVEBU_REGS_BASE + 0x18300)

/*****************************************************************************
 * Cortex-M3 Secure Processor Mailbox constants
 *****************************************************************************
 */
#define MVEBU_RWTM_REG_BASE			(MVEBU_REGS_BASE + 0xB0000)

#endif /* A3700_PLAT_DEF_H */
