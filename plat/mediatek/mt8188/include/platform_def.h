/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch_def.h>

#define PLAT_PRIMARY_CPU	(0x0)

#define MT_GIC_BASE		(0x0C000000)
#define MCUCFG_BASE		(0x0C530000)
#define MCUCFG_REG_SIZE		(0x10000)
#define IO_PHYS			(0x10000000)

/* Aggregate of all devices for MMU mapping */
#define MTK_DEV_RNG0_BASE	(MT_GIC_BASE)
#define MTK_DEV_RNG0_SIZE	(0x600000)
#define MTK_DEV_RNG1_BASE	(IO_PHYS)
#define MTK_DEV_RNG1_SIZE	(0x10000000)

/*******************************************************************************
 * AUDIO related constants
 ******************************************************************************/
#define AUDIO_BASE		(IO_PHYS + 0x00b10000)

/*******************************************************************************
 * SPM related constants
 ******************************************************************************/
#define SPM_BASE		(IO_PHYS + 0x00006000)

/*******************************************************************************
 * GPIO related constants
 ******************************************************************************/
#define GPIO_BASE		(IO_PHYS + 0x00005000)
#define RGU_BASE		(IO_PHYS + 0x00007000)
#define DRM_BASE		(IO_PHYS + 0x0000D000)
#define IOCFG_RM_BASE		(IO_PHYS + 0x01C00000)
#define IOCFG_LT_BASE		(IO_PHYS + 0x01E10000)
#define IOCFG_LM_BASE		(IO_PHYS + 0x01E20000)
#define IOCFG_RT_BASE		(IO_PHYS + 0x01EA0000)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE	(IO_PHYS + 0x01002000)
#define UART_BAUDRATE	(115200)

/*******************************************************************************
 * PMIC related constants
 ******************************************************************************/
#define PMIC_WRAP_BASE		(IO_PHYS + 0x00024000)

/*******************************************************************************
 * Infra IOMMU related constants
 ******************************************************************************/
#define PERICFG_AO_BASE		(IO_PHYS + 0x01003000)
#define PERICFG_AO_REG_SIZE	(0x1000)

/*******************************************************************************
 * GIC-600 & interrupt handling related constants
 ******************************************************************************/
/* Base MTK_platform compatible GIC memory map */
#define BASE_GICD_BASE		(MT_GIC_BASE)
#define MT_GIC_RDIST_BASE	(MT_GIC_BASE + 0x40000)

/*******************************************************************************
 * CIRQ related constants
 ******************************************************************************/
#define SYS_CIRQ_BASE		(IO_PHYS + 0x204000)
#define MD_WDT_IRQ_BIT_ID	(141)
#define CIRQ_IRQ_NUM		(730)
#define CIRQ_REG_NUM		(23)
#define CIRQ_SPI_START		(96)

/*******************************************************************************
 * MM IOMMU & SMI related constants
 ******************************************************************************/
#define SMI_LARB_0_BASE		(IO_PHYS + 0x0c022000)
#define SMI_LARB_1_BASE		(IO_PHYS + 0x0c023000)
#define SMI_LARB_2_BASE		(IO_PHYS + 0x0c102000)
#define SMI_LARB_3_BASE		(IO_PHYS + 0x0c103000)
#define SMI_LARB_4_BASE		(IO_PHYS + 0x04013000)
#define SMI_LARB_5_BASE		(IO_PHYS + 0x04f02000)
#define SMI_LARB_6_BASE		(IO_PHYS + 0x04f03000)
#define SMI_LARB_7_BASE		(IO_PHYS + 0x04e04000)
#define SMI_LARB_9_BASE		(IO_PHYS + 0x05001000)
#define SMI_LARB_10_BASE	(IO_PHYS + 0x05120000)
#define SMI_LARB_11A_BASE	(IO_PHYS + 0x05230000)
#define SMI_LARB_11B_BASE	(IO_PHYS + 0x05530000)
#define SMI_LARB_11C_BASE	(IO_PHYS + 0x05630000)
#define SMI_LARB_12_BASE	(IO_PHYS + 0x05340000)
#define SMI_LARB_13_BASE	(IO_PHYS + 0x06001000)
#define SMI_LARB_14_BASE	(IO_PHYS + 0x06002000)
#define SMI_LARB_15_BASE	(IO_PHYS + 0x05140000)
#define SMI_LARB_16A_BASE	(IO_PHYS + 0x06008000)
#define SMI_LARB_16B_BASE	(IO_PHYS + 0x0600a000)
#define SMI_LARB_17A_BASE	(IO_PHYS + 0x06009000)
#define SMI_LARB_17B_BASE	(IO_PHYS + 0x0600b000)
#define SMI_LARB_19_BASE	(IO_PHYS + 0x0a010000)
#define SMI_LARB_21_BASE	(IO_PHYS + 0x0802e000)
#define SMI_LARB_23_BASE	(IO_PHYS + 0x0800d000)
#define SMI_LARB_27_BASE	(IO_PHYS + 0x07201000)
#define SMI_LARB_28_BASE	(IO_PHYS + 0x00000000)
#define SMI_LARB_REG_RNG_SIZE	(0x1000)

/*******************************************************************************
 * DP related constants
 ******************************************************************************/
#define EDP_SEC_BASE		(IO_PHYS + 0x0C504000)
#define DP_SEC_BASE		(IO_PHYS + 0x0C604000)
#define EDP_SEC_SIZE		(0x1000)
#define DP_SEC_SIZE		(0x1000)

/*******************************************************************************
 * EMI MPU related constants
 *******************************************************************************/
#define EMI_MPU_BASE		(IO_PHYS + 0x00226000)
#define SUB_EMI_MPU_BASE	(IO_PHYS + 0x00225000)

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_HZ	(13000000)
#define SYS_COUNTER_FREQ_IN_MHZ	(13)

/*******************************************************************************
 * Platform binary types for linking
 ******************************************************************************/
#define PLATFORM_LINKER_FORMAT		"elf64-littleaarch64"
#define PLATFORM_LINKER_ARCH		aarch64

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
#define PLATFORM_STACK_SIZE		(0x800)
#define FIRMWARE_WELCOME_STR		"Booting Trusted Firmware\n"
#define SOC_CHIP_ID			U(0x8188)

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define TZRAM_BASE			(0x54600000)
#define TZRAM_SIZE			(0x00030000)

/*******************************************************************************
 * BL31 specific defines.
 ******************************************************************************/
/*
 * Put BL3-1 at the top of the Trusted SRAM (just below the shared memory, if
 * present). BL31_BASE is calculated using the current BL3-1 debug size plus a
 * little space for growth.
 */
#define BL31_BASE			(TZRAM_BASE + 0x1000)
#define BL31_LIMIT			(TZRAM_BASE + TZRAM_SIZE)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_XLAT_TABLES			(16)
#define MAX_MMAP_REGIONS		(16)

/*******************************************************************************
 * CPU_EB TCM handling related constants
 ******************************************************************************/
#define CPU_EB_TCM_BASE		(0x0C550000)
#define CPU_EB_TCM_SIZE		(0x10000)
#define CPU_EB_MBOX3_OFFSET	(0xFCE0)

/*******************************************************************************
 * CPU PM definitions
 *******************************************************************************/
#define PLAT_CPU_PM_B_BUCK_ISO_ID	(6)
#define PLAT_CPU_PM_ILDO_ID		(6)
#define CPU_IDLE_SRAM_BASE		(0x11B000)
#define CPU_IDLE_SRAM_SIZE		(0x1000)

#endif /* PLATFORM_DEF_H */
