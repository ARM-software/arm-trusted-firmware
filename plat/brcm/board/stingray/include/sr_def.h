/*
 * Copyright (c) 2016-2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SR_DEF_H
#define SR_DEF_H

#ifndef __ASSEMBLER__
#include <lib/mmio.h>
#endif

#include <common/interrupt_props.h>
#include <drivers/arm/gic_common.h>

#include <crmu_def.h>

/* Special value used to verify platform parameters from BL2 to BL3-1 */
#define BRCM_BL31_PLAT_PARAM_VAL	ULL(0x0f1e2d3c4b5a6978)

#define MHB_BASE_ADDR		0x60000000
#define PLAT_BRCM_CCN_BASE	0x61000000
#define CORESIGHT_BASE_ADDR	0x62000000
#define SMMU_BASE		0x64000000

/* memory map entries*/
/* Grouping block device for bigger MMU region */
/* covers MHB, CNN, coresight, GIC, MMU, APB, CRMU */
#define PERIPH0_BASE	MHB_BASE_ADDR
#define PERIPH0_SIZE	0x06d00000

#define PERIPH1_BASE	0x66d80000
#define PERIPH1_SIZE	0x00f80000

#define HSLS_BASE_ADDR	0x68900000
#define HSLS_SIZE	0x04500000

#define GIC500_BASE	0x63c00000
#define GIC500_SIZE	0x400000

/*******************************************************************************
 * CCN related constants
 ******************************************************************************/
#define OLY_MN_REGISTERS_NODE0_SECURE_ACCESS	(PLAT_BRCM_CCN_BASE + 0x0)

#define OLY_RNI3PDVM_REGISTERS_NODE8_AUX_CTL	(PLAT_BRCM_CCN_BASE + 0x880500)

/* Used for acceleration of coherent ordered writes */
#define OLY_RNI3PDVM_REGISTERS_NODE8_AUX_CTL_WUO  BIT(4)
/* Wait for completion of requests at RN-I */
#define OLY_RNI3PDVM_REGISTERS_NODE8_AUX_CTL_WFC  BIT(3)

/*
 * Forces all reads from the RN-I to be sent with the request order bit set
 * and this ensures ordered allocation of read data buffers in the RN-I
 */
#define OLY_RNI3PDVM_REGISTERS_NODE8_AUX_CTL_RQO  BIT(5)

#define OLY_RNI3PDVM_REGISTERS_NODE14_AUX_CTL	(PLAT_BRCM_CCN_BASE + 0x8e0500)

/* Wait for completion of requests at RN-I */
#define OLY_RNI3PDVM_REGISTERS_NODE14_AUX_CTL_WFC BIT(3)

#define OLY_HNI_REGISTERS_NODE0_POS_CONTROL	  (PLAT_BRCM_CCN_BASE + 0x80000)
#define POS_CONTROL_HNI_POS_EN			  BIT(0)

#define OLY_HNI_REGISTERS_NODE0_PCIERC_RNI_NODEID_LIST \
						  (PLAT_BRCM_CCN_BASE + 0x80008)
/* PAXB and PAXC connected to 8th Node */
#define SR_RNI_PCIE_CONNECTED			  BIT(8)
/* PAXB connected to 6th Node */
#define SRP_RNI_PCIE_CONNECTED			  BIT(6)

#define OLY_HNI_REGISTERS_NODE0_SA_AUX_CTL	  (PLAT_BRCM_CCN_BASE + 0x80500)
#define SA_AUX_CTL_POS_EARLY_WR_COMP_EN		  BIT(5)
#define SA_AUX_CTL_SER_DEVNE_WR			  BIT(9)

/*******************************************************************************
 * Coresight related constants
 ******************************************************************************/
#define CORESIGHT_BASE_ADDR	0x62000000

#define IHOST0_BASE			0x66000000
#define IHOST_ADDR_SPACE		0x2000

/*******************************************************************************
 * SCR related constants
 ******************************************************************************/
#define SCR_BASE			0x6600a000
#define SCR_ARCACHE_OFFSET		4
#define SCR_ARCACHE_MASK		(0x3 << SCR_ARCACHE_OFFSET)
#define SCR_AWCACHE_OFFSET		6
#define SCR_AWCACHE_MASK		(0x3 << SCR_AWCACHE_OFFSET)
#define SCR_AXCACHE_CONFIG_MASK		(SCR_ARCACHE_MASK | SCR_AWCACHE_MASK)
#define SCR_TBUX_AXCACHE_CONFIG		((0x1 << SCR_AWCACHE_OFFSET) | \
					 (0x1 << SCR_ARCACHE_OFFSET))

#define SCR_REGS_SCR_SOFT_RESET		(SCR_BASE + 0x1c)
#define SCR_REGS_GIC_SOFT_RESET		BIT(0)

#define SCR_GPV_BASE			0x66100000
#define SCR_NOC_SECURITY0		(SCR_GPV_BASE + 0x08)
#define SCR_NOC_DDR_REGISTER_ACCESS	(SCR_GPV_BASE + 0x30)

/*******************************************************************************
 * MEMC and DDR related constants
 ******************************************************************************/
#define DDR0_CONTROL_ROOT	0x66200000
#define EMEM_SS_CFG_0_ROOT	0x66202000
#define EMEM_SYS_IF_0_ROOT	0x66204000
#define DDR_PHY0_ROOT		0x66240000

#define DDR1_CONTROL_ROOT	0x66280000
#define EMEM_SS_CFG_1_ROOT	0x66282000
#define EMEM_SYS_IF_1_ROOT	0x66284000
#define DDR_PHY1_ROOT		0x662c0000

#define DDR2_CONTROL_ROOT	0x66300000
#define EMEM_SS_CFG_2_ROOT	0x66302000
#define EMEM_SYS_IF_2_ROOT	0x66304000
#define DDR_PHY2_ROOT		0x66340000

/*******************************************************************************
 * TZC400 related constants
 ******************************************************************************/
#define TZC_400_BASE		0x66d84000

/*******************************************************************************
 * FS4 related constants
 ******************************************************************************/
#define FS4_SRAM_IDM_IO_CONTROL_DIRECT	 0x66d8a408

#define FS4_CRYPTO_IDM_IO_CONTROL_DIRECT 0x66d8e408
#define FS4_CRYPTO_IDM_RESET_CONTROL	 0x66d8e800
#define FS4_CRYPTO_BASE			 0x67000000
#define FS4_CRYPTO_DME_BASE		 (FS4_CRYPTO_BASE + 0x280000)

#define FS4_RAID_IDM_IO_CONTROL_DIRECT	 0x66d8f408
#define FS4_RAID_IDM_IO_STATUS		 0x66d8f500
#define FS4_RAID_IDM_RESET_CONTROL	 0x66d8f800
#define FS4_RAID_BASE			 0x67400000
#define FS4_RAID_DME_BASE		 (FS4_RAID_BASE + 0x280000)

#define FS4_CRYPTO_GPV_BASE		 0x67300000
#define FS4_RAID_GPV_BASE		 0x67700000

#define FS6_PKI_BASE			0x67400000
#define FS6_PKI_DME_BASE		0x66D90000

#define TZC400_FS_SRAM_ROOT		 0x66d84000
#define GATE_KEEPER_OFFSET		 0x8
#define REGION_ATTRIBUTES_0_OFFSET	 0x110
#define REGION_ID_ACCESS_0_OFFSET	 0x114

#define NIC400_FS_NOC_ROOT		 0x66e00000
#define NIC400_FS_NOC_SECURITY2_OFFSET	 0x10
#define NIC400_FS_NOC_SECURITY4_OFFSET	 0x18
#define NIC400_FS_NOC_SECURITY7_OFFSET	 0x24

/*******************************************************************************
 * SATA PHY related constants
 ******************************************************************************/
#define SATA_BASE	0x67d00000

/*******************************************************************************
 * USB related constants
 ******************************************************************************/
#define USB_BASE	0x68500000
#define USB_SIZE	0x00400000
#define XHC_BASE	(USB_BASE + 0x11000)
#define MAX_USB_PORTS	3

/*******************************************************************************
 * HSLS related constants
 ******************************************************************************/
#define IPROC_ROOT		0x68900000
#define HSLS_ICFG_REGS_BASE	IPROC_ROOT
#define HSLS_IDM_REGS_BASE	0x68e00000
#define HSLS_MODE_SEL_CONTROL	0x68a40000
#define HSLS_TZPC_BASE		0x68b40000
#define HSLS_GPV_BASE		0x6cd00000

/*******************************************************************************
 * Chip ID related constants
 ******************************************************************************/
#define ICFG_CHIP_ID		HSLS_ICFG_REGS_BASE
#define CHIP_ID_SR		0xd730
#define CHIP_ID_NS3Z		0xe56d
#define CHIP_ID_MASK		0xf000
#define ICFG_CHIP_REVISION_ID	(HSLS_ICFG_REGS_BASE + 0x4)
#define PLAT_CHIP_ID_GET	(mmio_read_32(ICFG_CHIP_ID))
#define PLAT_CHIP_REV_GET	(mmio_read_32(ICFG_CHIP_REVISION_ID))

/*******************************************************************************
 * Timers related constants
 ******************************************************************************/
/* ChipcommonG_tim0_TIM_TIMER1Load 0x68930000 */
#define SP804_TIMER0_BASE	0x68930000
#define SP804_TIMER1_BASE	0x68940000
#define SP804_TIMER0_TIMER_VAL_REG_OFFSET 0x4
#define SP804_TIMER0_CLKMULT	2
#define SP804_TIMER0_CLKDIV	25

/*******************************************************************************
 * GPIO related constants
 ******************************************************************************/
#define IPROC_GPIO_NS_BASE	0x689d0000
#define IPROC_GPIO_S_BASE	0x68b00000
#define IPROC_GPIO_NR		151
#define GPIO_S_CNTRL_REG	0x68b60000

/*******************************************************************************
 * I2C SMBUS related constants
 ******************************************************************************/
#define SMBUS0_REGS_BASE	0x689b0000
#define SMBUS1_REGS_BASE	0x689e0000

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define ChipcommonG_UART0_UART_RBR_THR_DLL	0x68a00000
#define ChipcommonG_UART1_UART_RBR_THR_DLL	0x68a10000
#define ChipcommonG_UART2_UART_RBR_THR_DLL	0x68a20000
#define ChipcommonG_UART3_UART_RBR_THR_DLL	0x68a30000

#define UART0_BASE_ADDR		ChipcommonG_UART0_UART_RBR_THR_DLL
#define UART1_BASE_ADDR		ChipcommonG_UART1_UART_RBR_THR_DLL
#define UART2_BASE_ADDR		ChipcommonG_UART2_UART_RBR_THR_DLL
#define UART3_BASE_ADDR		ChipcommonG_UART3_UART_RBR_THR_DLL

#define UART_SPR_OFFSET		0x1c    /* Scratch Pad Register */

#define LOG_LEVEL_REGISTER	CRMU_SPARE_REG_3
#define GET_LOG_LEVEL()		(mmio_read_32(LOG_LEVEL_REGISTER))
#define SET_LOG_LEVEL(x)	(mmio_write_32(LOG_LEVEL_REGISTER, x))

#define IO_RETRY_REGISTER	CRMU_SPARE_REG_4

#define DWC_UART_REFCLK		(25 * 1000 * 1000)
#define DWC_UART_REFCLK_DIV	16
/* Baud rate in emulation will vary based on setting of 25MHz SCLK */
#define DWC_UART_BAUDRATE	115200

#define BRCM_CRASH_CONSOLE_BASE		UART1_BASE_ADDR
#define BRCM_CRASH_CONSOLE_REFCLK	DWC_UART_REFCLK
#define BRCM_CRASH_CONSOLE_BAUDRATE	DWC_UART_BAUDRATE

#ifdef BOARD_CONSOLE_UART
#define PLAT_BRCM_BOOT_UART_BASE	BOARD_CONSOLE_UART
#else
#define PLAT_BRCM_BOOT_UART_BASE	UART1_BASE_ADDR
#endif
#define CONSOLE_UART_ID	((PLAT_BRCM_BOOT_UART_BASE >> 16) & 0x3)

#define PLAT_BRCM_BOOT_UART_CLK_IN_HZ	DWC_UART_REFCLK
#define BRCM_CONSOLE_BAUDRATE		DWC_UART_BAUDRATE

#define PLAT_BRCM_BL31_RUN_UART_BASE	 PLAT_BRCM_BOOT_UART_BASE
#define PLAT_BRCM_BL31_RUN_UART_CLK_IN_HZ PLAT_BRCM_BOOT_UART_CLK_IN_HZ

/*******************************************************************************
 * IOMUX related constants
 ******************************************************************************/
#define HSLS_IOPAD_BASE			HSLS_MODE_SEL_CONTROL
#define MODE_SEL_CONTROL_FSEL_MASK	0x7
#define MODE_SEL_CONTROL_FSEL_MODE0	0x0
#define MODE_SEL_CONTROL_FSEL_MODE1	0x1
#define MODE_SEL_CONTROL_FSEL_MODE2	0x2
#define MODE_SEL_CONTROL_FSEL_MODE3	0x3
#define MODE_SEL_CONTROL_FSEL_DEBUG	0x4
#define IPROC_IOPAD_MODE_BASE		(HSLS_MODE_SEL_CONTROL + 0x29c)
#define UART0_SIN_MODE_SEL_CONTROL	(HSLS_MODE_SEL_CONTROL + 0x4a8)
#define UART0_SOUT_MODE_SEL_CONTROL	(HSLS_MODE_SEL_CONTROL + 0x4ac)
#define UART1_SIN_MODE_SEL_CONTROL	(HSLS_MODE_SEL_CONTROL + 0x3b8)
#define UART1_SOUT_MODE_SEL_CONTROL	(HSLS_MODE_SEL_CONTROL + 0x3bc)
#define UARTx_SIN_MODE_SEL_CONTROL_FSEL		0
#define UARTx_SOUT_MODE_SEL_CONTROL_FSEL	0

/*******************************************************************************
 * PKA constants
 ******************************************************************************/
#define ICFG_PKA_MEM_PWR_CTRL			(HSLS_ICFG_REGS_BASE + 0xac0)
#define ICFG_PKA_MEM_PWR_CTRL__POWERONIN	BIT(0)
#define ICFG_PKA_MEM_PWR_CTRL__POWEROKIN	BIT(1)
#define ICFG_PKA_MEM_PWR_CTRL__ARRPOWERONIN	BIT(2)
#define ICFG_PKA_MEM_PWR_CTRL__ARRPOWEROKIN	BIT(3)
#define ICFG_PKA_MEM_PWR_CTRL__POWERONOUT	BIT(4)
#define ICFG_PKA_MEM_PWR_CTRL__POWEROKOUT	BIT(5)
#define ICFG_PKA_MEM_PWR_CTRL__ARRPOWERONOUT	BIT(6)
#define ICFG_PKA_MEM_PWR_CTRL__ARRPOWEROKOUT	BIT(7)
#define ICFG_PKA_MEM_PWR_CTRL__ISO		BIT(8)

/*******************************************************************************
 * RNG constants
 ******************************************************************************/
#define RNG_BASE_ADDR			0x68b20000

/*******************************************************************************
 * Trusted Watchdog constants
 ******************************************************************************/
#define ARM_SP805_TWDG_BASE		0x68b30000
#define ARM_SP805_TWDG_CLK_HZ		((25 * 1000 * 1000) / 2)
/*
 * The TBBR document specifies a watchdog timeout of 256 seconds. SP805
 * asserts reset after two consecutive countdowns (2 x 128 = 256 sec)
 */
#define ARM_TWDG_TIMEOUT_SEC		128
#define ARM_TWDG_LOAD_VAL		(ARM_SP805_TWDG_CLK_HZ * \
					 ARM_TWDG_TIMEOUT_SEC)

/*******************************************************************************
 * SOTP related constants
 ******************************************************************************/
#define SOTP_REGS_OTP_BASE		0x68b50000
#define SOTP_CHIP_CTRL			(SOTP_REGS_OTP_BASE + 0x4c)
#define SOTP_CLEAR_SYSCTRL_ALL_MASTER_NS  0

/*******************************************************************************
 * DMAC/PL330 related constants
 ******************************************************************************/
#define DMAC_M0_IDM_IO_CONTROL_DIRECT	(HSLS_IDM_REGS_BASE + 0x408)
#define BOOT_MANAGER_NS			BIT(25)
#define DMAC_M0_IDM_RESET_CONTROL	(HSLS_IDM_REGS_BASE + 0x800)
#define ICFG_DMAC_CONFIG_0		(HSLS_ICFG_REGS_BASE + 0x190)
#define ICFG_DMAC_CONFIG_1		(HSLS_ICFG_REGS_BASE + 0x194)
#define ICFG_DMAC_CONFIG_2		(HSLS_ICFG_REGS_BASE + 0x198)
#define BOOT_PERIPHERAL_NS		0xffffffff
#define ICFG_DMAC_CONFIG_3		(HSLS_ICFG_REGS_BASE + 0x19c)
#define BOOT_IRQ_NS			0x0000ffff
#define ICFG_DMAC_SID_ARADDR_CONTROL	(HSLS_ICFG_REGS_BASE + 0xaf0)
#define ICFG_DMAC_SID_AWADDR_CONTROL	(HSLS_ICFG_REGS_BASE + 0xaf4)
#define ICFG_DMAC_MEM_PWR_CTRL__POWERONIN	BIT(0)
#define ICFG_DMAC_MEM_PWR_CTRL__POWEROKIN	BIT(1)
#define ICFG_DMAC_MEM_PWR_CTRL__ARRPOWERONIN	BIT(2)
#define ICFG_DMAC_MEM_PWR_CTRL__ARRPOWEROKIN	BIT(3)
#define ICFG_DMAC_MEM_PWR_CTRL__POWERONOUT	BIT(4)
#define ICFG_DMAC_MEM_PWR_CTRL__POWEROKOUT	BIT(5)
#define ICFG_DMAC_MEM_PWR_CTRL__ARRPOWERONOUT	BIT(6)
#define ICFG_DMAC_MEM_PWR_CTRL__ARRPOWEROKOUT	BIT(7)
#define ICFG_DMAC_MEM_PWR_CTRL__ISO		BIT(8)
#define ICFG_DMAC_MEM_PWR_CTRL		(HSLS_ICFG_REGS_BASE + 0xadc)

/*******************************************************************************
 * PNOR related constants
 ******************************************************************************/
#define PNOR_ICFG_BASE			(HSLS_ICFG_REGS_BASE + 0x780)
#define PNOR_ICFG_CS_0			PNOR_ICFG_BASE
#define PNOR_ICFG_CS_1			(PNOR_ICFG_BASE + 0x4)
#define PNOR_ICFG_CS_2			(PNOR_ICFG_BASE + 0x8)
#define PNOR_ICFG_CS_x_MASK0_MASK	0xff
#define PNOR_ICFG_CS_x_MASK0_SHIFT	8
#define PNOR_ICFG_CS_x_MATCH0_MASK	0xff
#define PNOR_ICFG_CS_x_MATCH0_SHIFT	0

#define PNOR_IDM_BASE			(HSLS_IDM_REGS_BASE + 0xb000)
#define PNOR_IDM_IO_CONTROL_DIRECT	(PNOR_IDM_BASE + 0x408)
#define PNOR_IDM_IO_RESET_CONTROL	(PNOR_IDM_BASE + 0x800)

#define PNOR_REG_BASE			0x68c50000
#define PNOR_REG_DIRECT_CMD		(PNOR_REG_BASE + 0x010)
#define PNOR_REG_SET_CYCLES		(PNOR_REG_BASE + 0x014)
#define PNOR_REG_SET_OPMODE		(PNOR_REG_BASE + 0x018)
#define PNOR_REG_REFRESH_0		(PNOR_REG_BASE + 0x020)
#define PNOR_REG_PERIPH_ID0		(PNOR_REG_BASE + 0xfe0)
#define PNOR_REG_PERIPH_ID1		(PNOR_REG_BASE + 0xfe4)
#define PNOR_REG_PERIPH_ID2		(PNOR_REG_BASE + 0xfe8)
#define PNOR_REG_PERIPH_ID3		(PNOR_REG_BASE + 0xfec)
#define PNOR_REG_PERIPH_IDx_MASK	0xff

/*******************************************************************************
 * NAND related constants
 ******************************************************************************/
#define NAND_FLASH_REVISION		0x68c60000
#define NAND_IDM_IDM_IO_CONTROL_DIRECT	(HSLS_IDM_REGS_BASE + 0xa408)
#define NAND_IDM_IDM_RESET_CONTROL	(HSLS_IDM_REGS_BASE + 0xa800)

/*******************************************************************************
 * eMMC related constants
 ******************************************************************************/
#define PLAT_SD_MAX_READ_LENGTH		0x400

#define SDIO0_EMMCSDXC_SYSADDR		0x68cf1000
#define SDIO_IDM0_IO_CONTROL_DIRECT	(HSLS_IDM_REGS_BASE + 0x2408)
#define SDIO_IDM1_IO_CONTROL_DIRECT	(HSLS_IDM_REGS_BASE + 0x3408)
#define SDIO_IDM0_IDM_RESET_CONTROL	(HSLS_IDM_REGS_BASE + 0x2800)
#define ICFG_SDIO0_BASE			(HSLS_ICFG_REGS_BASE + 0x6e4)
#define ICFG_SDIO1_BASE			(HSLS_ICFG_REGS_BASE + 0x734)
#define ICFG_SDIO0_CAP0			(ICFG_SDIO0_BASE + 0x10)
#define ICFG_SDIO0_CAP1			(ICFG_SDIO0_BASE + 0x14)
#define ICFG_SDIO0_SID			(HSLS_ICFG_REGS_BASE + 0xb00)
#define ICFG_SDIO1_SID			(HSLS_ICFG_REGS_BASE + 0xb08)

/*******************************************************************************
 * Bootstrap related constants
 ******************************************************************************/
#define ROM_S0_IDM_IO_STATUS		(HSLS_IDM_REGS_BASE + 0x9500)

/*******************************************************************************
 * ROM related constants
 ******************************************************************************/
#define ROM_BASE_ADDR		0x6ce00000
#define ROM_VERSION_STRING_ADDR	(ROM_BASE_ADDR + 0x28000)
#define ROM_BUILD_MESSAGE_ADDR	(ROM_BASE_ADDR + 0x28018)

/*******************************************************************************
 * Boot source peripheral related constants
 ******************************************************************************/
#define QSPI_CTRL_BASE_ADDR	0x68c70000
#define QSPI_BASE_ADDR		0x70000000
#define QSPI_SIZE		0x08000000
#define NOR_BASE_ADDR		0x74000000
#define NOR_SIZE		0x04000000
#define NAND_BASE_ADDR		0x78000000
#define NAND_SIZE		0x08000000

#define QSPI_IDM_RESET_CONTROL		(HSLS_IDM_REGS_BASE + 0xc800)

#define APBR_IDM_RESET_CONTROL		(HSLS_IDM_REGS_BASE + 0xe800)
#define APBS_IDM_IDM_RESET_CONTROL	(HSLS_IDM_REGS_BASE + 0xf800)

#define APBX_IDM_IDM_IO_CONTROL_DIRECT	(HSLS_IDM_REGS_BASE + 0x10408)
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_CLK_ENABLE	0
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_WDOG_SCLK_SEL	2
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_TIM0_SCLK_SEL	4
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_TIM1_SCLK_SEL	6
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_TIM2_SCLK_SEL	8
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_TIM3_SCLK_SEL	10
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_TIM4_SCLK_SEL	12
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_TIM5_SCLK_SEL	13
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_TIM6_SCLK_SEL	14
#define APBX_IDM_IDM_IO_CONTROL_DIRECT_TIM7_SCLK_SEL	15

#define APBY_IDM_IDM_IO_CONTROL_DIRECT	(HSLS_IDM_REGS_BASE + 0x11408)
#define APBY_IDM_IDM_IO_CONTROL_DIRECT_CLK_ENABLE	0
#define APBY_IDM_IDM_IO_CONTROL_DIRECT_UART0_SCLK_SEL	2
#define APBY_IDM_IDM_IO_CONTROL_DIRECT_UART1_SCLK_SEL	4
#define APBY_IDM_IDM_IO_CONTROL_DIRECT_UART2_SCLK_SEL	6
#define APBY_IDM_IDM_IO_CONTROL_DIRECT_UART3_SCLK_SEL	8

#define APBZ_IDM_IDM_IO_CONTROL_DIRECT	(HSLS_IDM_REGS_BASE + 0x12408)
#define APBZ_IDM_IDM_IO_CONTROL_DIRECT_CLK_ENABLE	0
#define APBZ_IDM_IDM_IO_CONTROL_DIRECT_WDOG_SCLK_SEL	2

/*******************************************************************************
 * Stingray memory map related constants
 ******************************************************************************/

/* The last 4KB of Trusted SRAM are used as shared memory */
#define BRCM_SHARED_RAM_SIZE		0x0
#define BRCM_SHARED_RAM_BASE		(PLAT_BRCM_TRUSTED_SRAM_BASE + \
					 PLAT_BRCM_TRUSTED_SRAM_SIZE - \
					 BRCM_SHARED_RAM_SIZE)

/* Reserve 4 KB to store error logs in BL2 */
#define BCM_ELOG_BL2_SIZE		0x00001000
#define BCM_ELOG_BL2_BASE		BL1_RW_LIMIT

/* The remaining Trusted SRAM is used to load the BL images */
#define BRCM_BL_RAM_BASE		(PLAT_BRCM_TRUSTED_SRAM_BASE)
#define BRCM_BL_RAM_SIZE		(PLAT_BRCM_TRUSTED_SRAM_SIZE - \
					 BRCM_SHARED_RAM_SIZE)

/* DDR Address where TMON temperature values are written */
#define TMON_SHARED_DDR_ADDRESS		0x8f100000

/* Reserve 4 kB to pass data to BL33 */
#define BL33_SHARED_DDR_BASE		0x8f102000
#define BL33_SHARED_DDR_SIZE		0x1000

/* Default AP error logging base addr */
#ifndef ELOG_AP_UART_LOG_BASE
#define ELOG_AP_UART_LOG_BASE		0x8f110000
#endif

/* Reserve 16 to store error logs in BL31 */
#define BCM_ELOG_BL31_BASE		ELOG_AP_UART_LOG_BASE
#define BCM_ELOG_BL31_SIZE		0x4000

/*******************************************************************************
 * Non-secure DDR Map
 ******************************************************************************/
#define BRCM_DRAM1_BASE		ULL(0x80000000)
#define BRCM_DRAM1_SIZE		ULL(0x10000000)
#define BRCM_DRAM2_BASE		ULL(0x880000000)
#define BRCM_DRAM2_SIZE		ULL(0x780000000)
#define BRCM_DRAM3_BASE		ULL(0x8800000000)
#define BRCM_DRAM3_SIZE		ULL(0x7800000000)
#define BRCM_SHARED_DRAM_BASE	BL33_SHARED_DDR_BASE
#define BRCM_SHARED_DRAM_SIZE	BL33_SHARED_DDR_SIZE
#define BRCM_EXT_SRAM_BASE	ULL(0x74000000)
#define BRCM_EXT_SRAM_SIZE	ULL(0x4000000)

/* Priority levels for platforms */
#define PLAT_RAS_PRI			0x10
#define PLAT_SDEI_CRITICAL_PRI		0x60
#define PLAT_SDEI_NORMAL_PRI		0x70

/* Define a list of Group 1 Secure and Group 0 interrupts as per GICv3 */
#define BRCM_IRQ_SEC_SGI_0	14
#define BRCM_IRQ_SEC_SGI_1	15

/* RTC periodic interrupt */
#define BRCM_IRQ_SEC_SPI_0	49

/*
 *  Macros for local power states in SR platforms encoded by State-ID field
 *  within the power-state parameter.
 */

/* Local power state for power domains in Run state. */
#define PLAT_LOCAL_STATE_RUN	0

/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET	1

/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains.
 */
#define PLAT_LOCAL_STATE_OFF	2

/*
 * This macro defines the deepest retention state possible. A higher state
 * id will represent an invalid or a power down state.
 */
#define PLAT_MAX_RET_STATE	PLAT_LOCAL_STATE_RET

/*
 * This macro defines the deepest power down states possible. Any state ID
 * higher than this is invalid.
 */
#define PLAT_MAX_OFF_STATE	PLAT_LOCAL_STATE_OFF

/* ChiMP-related constants */

#define NITRO_TZPC_TZPCDECPROT0clr		0x60c01808
#define NITRO_TZPC_TZPCDECPROT0clr__DECPROT0_chimp_m_clr_R		1

#define NIC400_NITRO_CHIMP_S_IDM_IO_CONTROL_DIRECT		0x60e00408

#define CHIMP_INDIRECT_ADDR_MASK		0x3fffff
#define CHIMP_INDIRECT_BASE		0x60800000

#define CHIMP_REG_ECO_RESERVED		0x3042400

#define CHIMP_FLASH_ACCESS_DONE_BIT		2

/* indicate FRU table programming is done successfully */
#define CHIMP_FRU_PROG_DONE_BIT			9

#define CHIMP_REG_CTRL_BPE_MODE_REG		0x0
#define CHIMP_REG_CTRL_BPE_STAT_REG		0x4
#define CHIMP_REG_CTRL_FSTBOOT_PTR_REG		0x8
#define CHIMP_REG_CHIMP_REG_CTRL_BPE_MODE_REG__cm3_rst_L		1
#define CHIMP_REG_CHIMP_REG_CTRL_BPE_MODE_REG__cm3_rst_R		1
#define CHIMP_REG_CTRL_BASE		0x3040000
#define CHIMP_FAST_BOOT_MODE_BIT		2
#define CHIMP_REG_CHIMP_APE_SCPAD		0x3300000
#define CHIMP_REG_CHIMP_SCPAD		0x3100000

/* Chimp health status offset in scratch pad ram */
#define CHIMP_HEALTH_STATUS_OFFSET	0x8
/*
 * If not in NIC mode then FASTBOOT can be enabled.
 *  "Not in NIC mode" means that FORCE_FASTBOOT is set
 *  and a valid (1 or 2) fastboot type is specified.
 *
 *  Three types of fastboot are supported:
 *  0 = No fastboot. Boots Nitro/ChiMP and lets ROM loader
 *		initialize ChiMP from NVRAM (QSPI).
 *
 *  1 = Jump in place (need a flat image)
 *		This is intended to speedup Nitro FW boot on Palladium,
 *		can be used with a real chip as well.
 *  2 = Jump normally with decompression
 *		Modus operandi for a real chip. Works also on Palladium
 *		Note: image decompressing takes time on Palladium.
 *  3 = No fastboot support. No ChiMP bringup
 *		(use only for AP debug or for ChiMP's deferred setup).
 */
#define CHIMP_FASTBOOT_JUMP_DECOMPRESS		2
#define CHIMP_FASTBOOT_JUMP_IN_PLACE		1
#define CHIMP_FASTBOOT_NITRO_RESET		0
/*
 * Definitions for a non-Nitro access
 * to QSPI PAD after the handshake
 */
#define	QSPI_HOLD_N_MODE_SEL_CONTROL		(HSLS_MODE_SEL_CONTROL + 0x3e8)
#define QSPI_WP_N_MODE_SEL_CONTROL		(HSLS_MODE_SEL_CONTROL + 0x3ec)
#define QSPI_SCK_MODE_SEL_CONTROL		(HSLS_MODE_SEL_CONTROL + 0x3f0)
#define QSPI_CS_N_MODE_SEL_CONTROL		(HSLS_MODE_SEL_CONTROL + 0x3f4)
#define QSPI_MOSI_MODE_SEL_CONTROL		(HSLS_MODE_SEL_CONTROL + 0x3f8)
#define QSPI_MISO_MODE_SEL_CONTROL		(HSLS_MODE_SEL_CONTROL + 0x3fc)

/*******************************************************************************
 * Stream IDs for different blocks of SR
 * block_id for different blocks is as follows:
 * PCIE		: 0x0
 * PAXC		: 0x1
 * FS4		: 0x2
 * Rest of the masters(includes MHB via RNI): 0x3
 ******************************************************************************/
#define SR_SID_VAL(block_id, subblock_id, device_num)	((block_id << 13) | \
							(subblock_id << 11) | \
							(device_num))

#define CRMU_STREAM_ID		SR_SID_VAL(0x3, 0x0, 0x7)
#define CRMU_SID_SHIFT		5

#define DMAC_STREAM_ID		SR_SID_VAL(0x3, 0x0, 0x0)
#define DMAC_SID_SHIFT		5

/* DDR SHMOO Values defines */
#define IDRAM_SHMOO_VALUES_ADDR CRMU_IDRAM_BASE_ADDR
#define DDR_SHMOO_VALUES_ADDR 0x8f103000
#define SHMOO_SIZE_PER_CHANNEL 0x1000

#endif /* SR_DEF_H */
