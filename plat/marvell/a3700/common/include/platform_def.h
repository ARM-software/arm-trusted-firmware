/*
 * Copyright (C) 2016 Marvell International Ltd.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#ifndef __ASSEMBLER__
#include <stdio.h>
#endif /* __ASSEMBLER__ */

#include <board_marvell_def.h>
#include <mvebu_def.h>

/*
 * Most platform porting definitions provided by included headers
 */

/*
 * DRAM Memory layout:
 *		+-----------------------+
 *		:			:
 *		:	Linux		:
 * 0x04X00000-->+-----------------------+
 *		|	BL3-3(u-boot)	|>>}>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *		|-----------------------|  }				       |
 *		|	BL3-[0,1, 2]	|  }---------------------------------> |
 *		|-----------------------|  }				||     |
 *		|	BL2		|  }->FIP (loaded by            ||     |
 *		|-----------------------|  }       BootROM to DRAM)     ||     |
 *		|	FIP_TOC		|  }                            ||     |
 * 0x04120000-->|-----------------------|				||     |
 *		|	BL1 (RO)	|				||     |
 * 0x04100000-->+-----------------------+				||     |
 *		:			:				||     |
 *		: Trusted SRAM section	:				\/     |
 * 0x04040000-->+-----------------------+  Replaced by BL2  +----------------+ |
 *		|	BL1 (RW)	|  <<<<<<<<<<<<<<<< | BL3-1 NOBITS   | |
 * 0x04037000-->|-----------------------|  <<<<<<<<<<<<<<<< |----------------| |
 *		|			|  <<<<<<<<<<<<<<<< | BL3-1 PROGBITS | |
 * 0x04023000-->|-----------------------|		    +----------------+ |
 *		|	BL2		|				       |
 *		|-----------------------|				       |
 *		|			|				       |
 * 0x04001000-->|-----------------------|				       |
 *		|	Shared		|				       |
 * 0x04000000-->+-----------------------+				       |
 *		:			:				       |
 *		:	Linux		:				       |
 *		:			:				       |
 *		|-----------------------|				       |
 *		|			|	U-Boot(BL3-3) Loaded by BL2    |
 *		|	U-Boot		|	<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
 * 0x00000000-->+-----------------------+
 *
 * Trusted SRAM section 0x4000000..0x4200000:
 * ----------------------------------------
 * SRAM_BASE		= 0x4001000
 * BL2_BASE		= 0x4006000
 * BL2_LIMIT		= BL31_BASE
 * BL31_BASE		= 0x4023000 = (64MB + 256KB - 0x1D000)
 * BL31_PROGBITS_LIMIT	= BL1_RW_BASE
 * BL1_RW_BASE		= 0x4037000 = (64MB + 256KB - 0x9000)
 * BL1_RW_LIMIT		= BL31_LIMIT = 0x4040000
 *
 *
 * PLAT_MARVELL_FIP_BASE	= 0x4120000
 */

#define PLAT_MARVELL_ATF_BASE			0x4000000
#define PLAT_MARVELL_ATF_LOAD_ADDR		\
			(PLAT_MARVELL_ATF_BASE + 0x100000)

#define PLAT_MARVELL_FIP_BASE			\
			(PLAT_MARVELL_ATF_LOAD_ADDR + 0x20000)
#define PLAT_MARVELL_FIP_MAX_SIZE		0x4000000

#define PLAT_MARVELL_CLUSTER_CORE_COUNT		2
/* DRAM[2MB..66MB] is used  as Trusted ROM */
#define PLAT_MARVELL_TRUSTED_ROM_BASE		PLAT_MARVELL_ATF_LOAD_ADDR
/* 64 MB TODO: reduce this to minimum needed according to fip image size*/
#define PLAT_MARVELL_TRUSTED_ROM_SIZE		0x04000000
/* Reserve 16M for SCP (Secure PayLoad) Trusted DRAM */
#define PLAT_MARVELL_TRUSTED_DRAM_BASE		0x04400000
#define PLAT_MARVELL_TRUSTED_DRAM_SIZE		0x01000000	/* 16 MB */

/*
 * PLAT_ARM_MAX_BL1_RW_SIZE is calculated using the current BL1 RW debug size
 * plus a little space for growth.
 */
#define PLAT_MARVELL_MAX_BL1_RW_SIZE		0xA000

/*
 * PLAT_ARM_MAX_BL2_SIZE is calculated using the current BL2 debug size plus a
 * little space for growth.
 */
#define PLAT_MARVELL_MAX_BL2_SIZE		0xF000

/*
 * PLAT_ARM_MAX_BL31_SIZE is calculated using the current BL31 debug size plus a
 * little space for growth.
 */
#define PLAT_MARVEL_MAX_BL31_SIZE		0x5D000

#define PLAT_MARVELL_CPU_ENTRY_ADDR		BL1_RO_BASE

/* GIC related definitions */
#define PLAT_MARVELL_GICD_BASE		(MVEBU_REGS_BASE + MVEBU_GICD_BASE)
#define PLAT_MARVELL_GICR_BASE		(MVEBU_REGS_BASE + MVEBU_GICR_BASE)
#define PLAT_MARVELL_GICC_BASE		(MVEBU_REGS_BASE + MVEBU_GICC_BASE)

#define PLAT_MARVELL_G0_IRQ_PROPS(grp) \
	INTR_PROP_DESC(MARVELL_IRQ_SEC_SGI_0, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(MARVELL_IRQ_SEC_SGI_6, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)

#define PLAT_MARVELL_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(MARVELL_IRQ_SEC_PHY_TIMER, \
			GIC_HIGHEST_SEC_PRIORITY, grp, GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(MARVELL_IRQ_SEC_SGI_1, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(MARVELL_IRQ_SEC_SGI_2, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(MARVELL_IRQ_SEC_SGI_3, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(MARVELL_IRQ_SEC_SGI_4, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(MARVELL_IRQ_SEC_SGI_5, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL), \
	INTR_PROP_DESC(MARVELL_IRQ_SEC_SGI_7, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)


#define PLAT_MARVELL_SHARED_RAM_CACHED		1

/* CCI related constants */
#define PLAT_MARVELL_CCI_BASE		(MVEBU_REGS_BASE + MVEBU_CCI_BASE)
#define PLAT_MARVELL_CCI_CLUSTER0_SL_IFACE_IX	3
#define PLAT_MARVELL_CCI_CLUSTER1_SL_IFACE_IX	4

/*
 * Load address of BL3-3 for this platform port
 */
#define PLAT_MARVELL_NS_IMAGE_OFFSET		0x0

/* System Reference Clock*/
#define PLAT_REF_CLK_IN_HZ			COUNTER_FREQUENCY

/*
 * PL011 related constants
 */
#define PLAT_MARVELL_BOOT_UART_BASE		(MVEBU_REGS_BASE + 0x12000)
#define PLAT_MARVELL_BOOT_UART_CLK_IN_HZ	25804800

#define PLAT_MARVELL_CRASH_UART_BASE		PLAT_MARVELL_BOOT_UART_BASE
#define PLAT_MARVELL_CRASH_UART_CLK_IN_HZ	PLAT_MARVELL_BOOT_UART_CLK_IN_HZ

#define PLAT_MARVELL_BL31_RUN_UART_BASE		PLAT_MARVELL_BOOT_UART_BASE
#define PLAT_MARVELL_BL31_RUN_UART_CLK_IN_HZ	PLAT_MARVELL_BOOT_UART_CLK_IN_HZ

/* Required platform porting definitions */
#define PLAT_MAX_PWR_LVL			MPIDR_AFFLVL1

/* System timer related constants */
#define PLAT_MARVELL_NSTIMER_FRAME_ID		1

/* Mailbox base address */
#define PLAT_MARVELL_MAILBOX_BASE		\
			(MARVELL_TRUSTED_SRAM_BASE + 0x400)
#define PLAT_MARVELL_MAILBOX_SIZE		0x100
#define PLAT_MARVELL_MAILBOX_MAGIC_NUM		0x6D72766C	/* mrvl */

/* DRAM CS memory map registers related constants */
#define MVEBU_CS_MMAP_LOW(cs_num)		\
			(MVEBU_CS_MMAP_REG_BASE + (cs_num) * 0x8)
#define MVEBU_CS_MMAP_ENABLE			0x1
#define MVEBU_CS_MMAP_AREA_LEN_OFFS		16
#define MVEBU_CS_MMAP_AREA_LEN_MASK		\
			(0x1f << MVEBU_CS_MMAP_AREA_LEN_OFFS)
#define MVEBU_CS_MMAP_START_ADDR_LOW_OFFS	23
#define MVEBU_CS_MMAP_START_ADDR_LOW_MASK	\
			(0x1ff << MVEBU_CS_MMAP_START_ADDR_LOW_OFFS)

#define MVEBU_CS_MMAP_HIGH(cs_num)		\
			(MVEBU_CS_MMAP_REG_BASE + 0x4 + (cs_num) * 0x8)

/* DRAM max CS number */
#define MVEBU_MAX_CS_MMAP_NUM			(2)

/* CPU decoder window related constants */
#define CPU_DEC_WIN_CTRL_REG(win_num)		\
			(MVEBU_CPU_DEC_WIN_REG_BASE + (win_num) * 0x10)
#define CPU_DEC_CR_WIN_ENABLE			0x1
#define CPU_DEC_CR_WIN_TARGET_OFFS		4
#define CPU_DEC_CR_WIN_TARGET_MASK		\
			(0xf << CPU_DEC_CR_WIN_TARGET_OFFS)

#define CPU_DEC_WIN_SIZE_REG(win_num)		\
			(MVEBU_CPU_DEC_WIN_REG_BASE + 0x4 + (win_num) * 0x10)
#define CPU_DEC_CR_WIN_SIZE_OFFS		0
#define CPU_DEC_CR_WIN_SIZE_MASK		\
			(0xffff << CPU_DEC_CR_WIN_SIZE_OFFS)
#define CPU_DEC_CR_WIN_SIZE_ALIGNMENT		0x10000

#define CPU_DEC_WIN_BASE_REG(win_num)		\
			(MVEBU_CPU_DEC_WIN_REG_BASE + 0x8 + (win_num) * 0x10)
#define CPU_DEC_BR_BASE_OFFS			0
#define CPU_DEC_BR_BASE_MASK			\
			(0xffff <<  CPU_DEC_BR_BASE_OFFS)

#define CPU_DEC_REMAP_LOW_REG(win_num)		\
			(MVEBU_CPU_DEC_WIN_REG_BASE + 0xC + (win_num) * 0x10)
#define CPU_DEC_RLR_REMAP_LOW_OFFS		0
#define CPU_DEC_RLR_REMAP_LOW_MASK		\
			(0xffff <<  CPU_DEC_BR_BASE_OFFS)

/* Securities */
#define IRQ_SEC_OS_TICK_INT	MARVELL_IRQ_SEC_PHY_TIMER

#define TRUSTED_DRAM_BASE	PLAT_MARVELL_TRUSTED_DRAM_BASE
#define TRUSTED_DRAM_SIZE	PLAT_MARVELL_TRUSTED_DRAM_SIZE

#ifdef BL32
#define BL32_BASE		TRUSTED_DRAM_BASE
#define BL32_LIMIT		TRUSTED_DRAM_SIZE
#endif

#endif /* PLATFORM_DEF_H */
