/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
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

#define TOPCKGEN_BASE		(IO_PHYS)

/*******************************************************************************
 * GPIO related constants
 ******************************************************************************/
#define GPIO_BASE		(IO_PHYS + 0x00005000)
#define IOCFG_LM_BASE		(IO_PHYS + 0x01B50000)
#define IOCFG_RB0_BASE		(IO_PHYS + 0x01C50000)
#define IOCFG_RB1_BASE		(IO_PHYS + 0x01C60000)
#define IOCFG_BM0_BASE		(IO_PHYS + 0x01D20000)
#define IOCFG_BM1_BASE		(IO_PHYS + 0x01D30000)
#define IOCFG_BM2_BASE		(IO_PHYS + 0x01D40000)
#define IOCFG_LT0_BASE		(IO_PHYS + 0x01E20000)
#define IOCFG_LT1_BASE		(IO_PHYS + 0x01E30000)
#define IOCFG_RT_BASE		(IO_PHYS + 0x01F20000)

/*******************************************************************************
 * SPM related constants
 ******************************************************************************/
#define SPM_BASE	(IO_PHYS + 0x0C001000)
#define INFRACFG_AO_BASE	(IO_PHYS + 0x00001000)
#define CKSYS_BASE	(IO_PHYS)

/*******************************************************************************
 * dvfsrc related constants
 ******************************************************************************/
#define DVFSRC_BASE	(IO_PHYS + 0x0C00F000)

#define MTK_LPM_SRAM_BASE	(0x11B000)

/*******************************************************************************
 * mfgsys related constants
 ******************************************************************************/
#define MFGSYS_BASE	(IO_PHYS + 0x03000000)

/*******************************************************************************
 * PERI related constants
 ******************************************************************************/
#define PERICFG_AO_BASE	(IO_PHYS + 0x01036000)
#define PERICFG_AO_SIZE	(0x1000)

/*******************************************************************************
 * APMIX related constants
 ******************************************************************************/
#define APMIXEDSYS	(IO_PHYS + 0x0000C000)
#define APMIXEDSYS_REG_SIZE	0x1000

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE	(IO_PHYS + 0x01002000)
#define UART_BAUDRATE	(115200)

/*******************************************************************************
 * SSPM CFGREG related constants
 ******************************************************************************/
#define SSPM_REG_OFFSET	(0x40000)
#define SSPM_CFGREG_BASE	(IO_PHYS + 0x0C300000 + SSPM_REG_OFFSET)
#define SSPM_CFGREG_SIZE	(0x1000)

/*******************************************************************************
 * SSPM_MBOX_3 related constants
 ******************************************************************************/
#define SSPM_MBOX_3_BASE	(IO_PHYS + 0x0C380000)
#define SSPM_MBOX_3_SIZE	(0x1000)

/*******************************************************************************
 * LPM syssram related constants
 ******************************************************************************/
#define MTK_LPM_SRAM_BASE	(0x11B000)
#define MTK_LPM_SRAM_MAP_SIZE	(0x1000)

/*******************************************************************************
 * Infra IOMMU related constants
 ******************************************************************************/
#define PERICFG_AO_BASE		(IO_PHYS + 0x01036000)
#define PERICFG_AO_REG_SIZE	(0x1000)

/*******************************************************************************
 * CIRQ related constants
 ******************************************************************************/
#define SYS_CIRQ_BASE		(IO_PHYS + 204000)
#define MD_WDT_IRQ_BIT_ID	(519)
#define CIRQ_REG_NUM		(19)
#define CIRQ_SPI_START		(128)
#define CIRQ_IRQ_NUM		(598)

/*******************************************************************************
 * MM IOMMU & SMI related constants
 ******************************************************************************/
#define SMI_LARB_0_BASE		(IO_PHYS + 0x0401c000)
#define SMI_LARB_1_BASE		(IO_PHYS + 0x0401d000)
#define SMI_LARB_2_BASE		(IO_PHYS + 0x0f002000)
#define SMI_LARB_4_BASE		(IO_PHYS + 0x0602e000)
#define SMI_LARB_7_BASE		(IO_PHYS + 0x07010000)
#define SMI_LARB_9_BASE		(IO_PHYS + 0x0502e000)
#define SMI_LARB_11_BASE	(IO_PHYS + 0x0582e000)
#define SMI_LARB_13_BASE	(IO_PHYS + 0x0a001000)
#define SMI_LARB_14_BASE	(IO_PHYS + 0x0a002000)
#define SMI_LARB_16_BASE	(IO_PHYS + 0x0a00f000)
#define SMI_LARB_17_BASE	(IO_PHYS + 0x0a010000)
#define SMI_LARB_19_BASE	(IO_PHYS + 0x0b10f000)
#define SMI_LARB_20_BASE	(IO_PHYS + 0x0b00f000)
#define SMI_LARB_REG_RNG_SIZE	(0x1000)
#define MMSYS_CONFIG_BASE	(IO_PHYS + 0x04000000)
#define DISP_MUTEX_BASE		(IO_PHYS + 0x04001000)
#define DISP_OVL0_BASE		(IO_PHYS + 0x04002000)
#define DISP_OVL1_BASE		(IO_PHYS + 0x04003000)
#define DISP_RDMA0_BASE		(IO_PHYS + 0x04006000)
#define DISP_COLOR0_BASE	(IO_PHYS + 0x04008000)
#define DISP_CCORR0_BASE	(IO_PHYS + 0x0400A000)
#define DISP_CCORR2_BASE	(IO_PHYS + 0x0400C000)
#define DISP_AAL0_BASE		(IO_PHYS + 0x0400E000)
#define DISP_GAMMA0_BASE	(IO_PHYS + 0x04010000)
#define DISP_DITHER0_BASE	(IO_PHYS + 0x04012000)
#define MM_IOMMU_BASE		(IO_PHYS + 0x0e802000 + 0x4000)
#define APU_IOMMU_BASE		(IO_PHYS + 0x09010000)

#define IOMMU_REG_RNG_SIZE	(0x5000)

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
#define SOC_CHIP_ID			U(0x8189)

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define TZRAM_BASE			(0x54600000)
#define TZRAM_SIZE			(0x00200000)

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
 * GIC-600 & interrupt handling related constants
 ******************************************************************************/
/* Base MTK_platform compatible GIC memory map */
#define BASE_GICD_BASE		(MT_GIC_BASE)
#define MT_GIC_RDIST_BASE	(MT_GIC_BASE + 0x40000)
#define DEV_IRQ_ID		300

#define PLAT_MTK_G1S_IRQ_PROPS(grp) \
	INTR_PROP_DESC(DEV_IRQ_ID, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)

/*******************************************************************************
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)
#define MAX_XLAT_TABLES			(16)
#define MAX_MMAP_REGIONS		(16)

/*******************************************************************************
 * SYSTIMER related definitions
 ******************************************************************************/
#define SYSTIMER_BASE		(IO_PHYS + 0x0CC10000)

/*******************************************************************************
 * SPMI related definitions
 ******************************************************************************/
#define SPMI_MST_P_BASE			(IO_PHYS + 0x0CC00000)
#define PMIF_SPMI_P_BASE		(IO_PHYS + 0x0CC06000)
#define SPMI_MST_P_SIZE			(0x1000)

/*******************************************************************************
 * PWRAP related definitions
 ******************************************************************************/
#define PMICSPI_MST_BASE		(IO_PHYS + 0x0c013000)
#define PMICSPI_MST_SIZE		(0x1000)
#define PMIC_WRAP_BASE			(IO_PHYS + 0x0CC04000)
#define PMIF_SPI_BASE			(0x1CC04000)
#define PWRAP_REG_BASE			(0x1C013000)
#define PWRAP_WRAP_EN			(PWRAP_REG_BASE + 0x14)

/*******************************************************************************
 * PMIC regsister related definitions
 ******************************************************************************/
#define PMIC_REG_BASE			(0x0000)
#define PWRAP_SIZE			(0x1000)
#define DEW_READ_TEST			(PMIC_REG_BASE + 0x040e)
#define DEW_WRITE_TEST			(PMIC_REG_BASE + 0x0410)

/*******************************************************************************
 * Differentiate between 3G and 2.6G-related definitions
 ******************************************************************************/
#define EFUSEC_BASE			(IO_PHYS + 0x01F10000)
#define CHIP_ID_REG			(EFUSEC_BASE + 0x7A0)
#define CPU_SEG_ID_REG			(EFUSEC_BASE + 0x7E0)

#define MTK_CPU_ID_MT8189		0x81890000
#define MTK_CPU_SEG_ID_MT8189G		0x20
#define MTK_CPU_SEG_ID_MT8189H		0x21

/*******************************************************************************
 * Thermal related constants
 ******************************************************************************/
#define INFRACFG_BASE			(IO_PHYS + 0x0020E000)
#define THERM_CTRL_AP_BASE		(IO_PHYS + 0x00315000)
#define THERM_AP_REG_SIZE		(0x1000)
#define THERM_CTRL_MCU_BASE		(IO_PHYS + 0x00316000)
#define THERM_MCU_REG_SIZE		(0x1000)

#define THERMAL_CSRAM_BASE		(0x00102500)
#define THERMAL_CSRAM_SIZE		(0x400)

/*******************************************************************************
 * CPU PM definitions
 ******************************************************************************/
#define PLAT_CPU_PM_B_BUCK_ISO_ID	(6)
#define PLAT_CPU_PM_ILDO_ID		(6)
#define CPU_IDLE_SRAM_BASE		(0x11B000)
#define CPU_IDLE_SRAM_SIZE		(0x1000)

/*******************************************************************************
 * SPM related constants
 ******************************************************************************/
#define SPM_BASE		(IO_PHYS + 0x0C001000)
#define SPM_REG_SIZE		(0x1000)

/*******************************************************************************
 * CPU_EB related constants
 ******************************************************************************/
#define CPU_EB_TCM_BASE		(0x0C56F000)
#define CPU_EB_TCM_SIZE		(0x1000)
#define CPU_EB_MBOX3_OFFSET	(0xCE0)

#endif /* PLATFORM_DEF_H */
