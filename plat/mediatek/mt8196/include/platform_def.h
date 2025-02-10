/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

#include <arch.h>
#include <plat/common/common_def.h>

#include <arch_def.h>

#define PLAT_PRIMARY_CPU	(0x0)

#define MT_GIC_BASE		(0x0C400000)
#define MCUCFG_BASE		(0x0C000000)
#define MCUCFG_REG_SIZE		(0x50000)
#define IO_PHYS			(0x10000000)

#define MT_UTILITYBUS_BASE	(0x0C800000)
#define MT_UTILITYBUS_SIZE	(0x800000)

/* Aggregate of all devices for MMU mapping */
#define MTK_DEV_RNG1_BASE	(IO_PHYS)
#define MTK_DEV_RNG1_SIZE	(0x10000000)

#define TOPCKGEN_BASE		(IO_PHYS)

/*******************************************************************************
 * AUDIO related constants
 ******************************************************************************/
#define AUDIO_BASE		(IO_PHYS + 0x0a110000)

/*******************************************************************************
 * APUSYS related constants
 ******************************************************************************/
#define APUSYS_BASE			(IO_PHYS + 0x09000000)
#define APU_MD32_SYSCTRL		(IO_PHYS + 0x09001000)
#define APU_MD32_WDT			(IO_PHYS + 0x09002000)
#define APU_LOGTOP			(IO_PHYS + 0x09024000)
#define APUSYS_CTRL_DAPC_RCX_BASE	(IO_PHYS + 0x09030000)
#define APU_REVISER			(IO_PHYS + 0x0903C000)
#define APU_RCX_UPRV_TCU		(IO_PHYS + 0x09060000)
#define APU_RCX_EXTM_TCU		(IO_PHYS + 0x09061000)
#define APU_CMU_TOP			(IO_PHYS + 0x09067000)
#define APUSYS_CE_BASE			(IO_PHYS + 0x090B0000)
#define APU_ARE_REG_BASE		(IO_PHYS + 0x090B0000)
#define APU_RCX_VCORE_CONFIG		(IO_PHYS + 0x090E0000)
#define APU_AO_CTRL			(IO_PHYS + 0x090F2000)
#define APU_SEC_CON			(IO_PHYS + 0x090F5000)
#define APUSYS_CTRL_DAPC_AO_BASE	(IO_PHYS + 0x090FC000)

#define APU_MBOX0			(0x4C200000)
#define APU_MD32_TCM			(0x4D000000)

#define APU_MD32_TCM_SZ			(0x50000)
#define APU_MBOX0_SZ			(0x100000)
#define APU_INFRA_BASE			(0x1002C000)
#define APU_INFRA_SZ			(0x1000)

#define APU_RESERVE_MEMORY		(0x95000000)
#define APU_SEC_INFO_OFFSET		(0x100000)
#define APU_RESERVE_SIZE		(0x1400000)

/*******************************************************************************
 * SPM related constants
 ******************************************************************************/
#define SPM_BASE		(IO_PHYS + 0x0C004000)
#define SPM_REG_SIZE		(0x1000)
#define SPM_SRAM_BASE		(IO_PHYS + 0x0C00C000)
#define SPM_SRAM_REG_SIZE	(0x1000)
#define SPM_PBUS_BASE		(IO_PHYS + 0x0C00D000)
#define SPM_PBUS_REG_SIZE	(0x1000)

#ifdef SPM_BASE
#define SPM_EXT_INT_WAKEUP_REQ		(SPM_BASE + 0x210)
#define SPM_EXT_INT_WAKEUP_REQ_SET	(SPM_BASE + 0x214)
#define SPM_EXT_INT_WAKEUP_REQ_CLR	(SPM_BASE + 0x218)
#define SPM_CPU_BUCK_ISO_CON		(SPM_BASE + 0xEF8)
#define SPM_CPU_BUCK_ISO_DEFAUT		(0x0)
#define SPM_AUDIO_PWR_CON		(SPM_BASE + 0xE4C)
#endif

/*******************************************************************************
 * GPIO related constants
 ******************************************************************************/
#define GPIO_BASE		(IO_PHYS + 0x0002D000)
#define RGU_BASE		(IO_PHYS + 0x0C010000)
#define DRM_BASE		(IO_PHYS + 0x0000D000)
#define IOCFG_RT_BASE		(IO_PHYS + 0x02000000)
#define IOCFG_RM1_BASE		(IO_PHYS + 0x02020000)
#define IOCFG_RM2_BASE		(IO_PHYS + 0x02040000)
#define IOCFG_RB_BASE		(IO_PHYS + 0x02060000)
#define IOCFG_BM1_BASE		(IO_PHYS + 0x02820000)
#define IOCFG_BM2_BASE		(IO_PHYS + 0x02840000)
#define IOCFG_BM3_BASE		(IO_PHYS + 0x02860000)
#define IOCFG_LT_BASE		(IO_PHYS + 0x03000000)
#define IOCFG_LM1_BASE		(IO_PHYS + 0x03020000)
#define IOCFG_LM2_BASE		(IO_PHYS + 0x03040000)
#define IOCFG_LB1_BASE		(IO_PHYS + 0x030f0000)
#define IOCFG_LB2_BASE		(IO_PHYS + 0x03110000)
#define IOCFG_TM1_BASE		(IO_PHYS + 0x03800000)
#define IOCFG_TM2_BASE		(IO_PHYS + 0x03820000)
#define IOCFG_TM3_BASE		(IO_PHYS + 0x03860000)

/*******************************************************************************
 * UART related constants
 ******************************************************************************/
#define UART0_BASE	(IO_PHYS + 0x06000000)
#define UART_BAUDRATE	(115200)

/*******************************************************************************
 * PMIF address
 ******************************************************************************/
#define PMIF_SPMI_M_BASE	(IO_PHYS + 0x0C01A000)
#define PMIF_SPMI_P_BASE	(IO_PHYS + 0x0C018000)
#define PMIF_SPMI_SIZE		0x1000

/*******************************************************************************
 * SPMI address
 ******************************************************************************/
#define SPMI_MST_M_BASE		(IO_PHYS + 0x0C01C000)
#define SPMI_MST_P_BASE		(IO_PHYS + 0x0C01C800)
#define SPMI_MST_SIZE		0x1000

/*******************************************************************************
 * Infra IOMMU related constants
 ******************************************************************************/
#define INFRACFG_AO_BASE	(IO_PHYS + 0x00001000)
#define INFRACFG_AO_MEM_BASE	(IO_PHYS + 0x00404000)
#define PERICFG_AO_BASE		(IO_PHYS + 0x06630000)
#define PERICFG_AO_REG_SIZE	(0x1000)

/*******************************************************************************
 * GIC-600 & interrupt handling related constants
 ******************************************************************************/
/* Base MTK_platform compatible GIC memory map */
#define BASE_GICD_BASE		(MT_GIC_BASE)
#define MT_GIC_RDIST_BASE	(MT_GIC_BASE + 0x40000)
#define MTK_GIC_REG_SIZE	0x400000
#define SGI_MASK		0xffff
#define DEV_IRQ_ID		982

#define PLATFORM_G1S_PROPS(grp) \
	INTR_PROP_DESC(DEV_IRQ_ID, GIC_HIGHEST_SEC_PRIORITY, grp, \
			GIC_INTR_CFG_LEVEL)

/*******************************************************************************
 * CIRQ related constants
 ******************************************************************************/
#define SYS_CIRQ_BASE		(IO_PHYS + 0x1CB000)
#define MD_WDT_IRQ_BIT_ID	(397)
#define CIRQ_REG_NUM		(26)
#define CIRQ_SPI_START		(128)
#define CIRQ_IRQ_NUM		(831)

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
 * APMIXEDSYS related constants
 ******************************************************************************/
#define APMIXEDSYS		(IO_PHYS + 0x0000C000)

/*******************************************************************************
 * VPPSYS related constants
 ******************************************************************************/
#define VPPSYS0_BASE		(IO_PHYS + 0x04000000)
#define VPPSYS1_BASE		(IO_PHYS + 0x04f00000)

/*******************************************************************************
 * VDOSYS related constants
 ******************************************************************************/
#define VDOSYS0_BASE		(IO_PHYS + 0x0C01D000)
#define VDOSYS1_BASE		(IO_PHYS + 0x0C100000)

/*******************************************************************************
 * DP related constants
 ******************************************************************************/
#define EDP_SEC_BASE		(IO_PHYS + 0x2EC54000)
#define DP_SEC_BASE		(IO_PHYS + 0x2EC14000)
#define EDP_SEC_SIZE		(0x1000)
#define DP_SEC_SIZE		(0x1000)

/*******************************************************************************
 * EMI MPU related constants
 *******************************************************************************/
#define EMI_MPU_BASE			(IO_PHYS + 0x00428000)
#define SUB_EMI_MPU_BASE		(IO_PHYS + 0x00528000)
#define EMI_SLB_BASE			(IO_PHYS + 0x0042e000)
#define SUB_EMI_SLB_BASE		(IO_PHYS + 0x0052e000)
#define CHN0_EMI_APB_BASE		(IO_PHYS + 0x00201000)
#define CHN1_EMI_APB_BASE		(IO_PHYS + 0x00205000)
#define CHN2_EMI_APB_BASE		(IO_PHYS + 0x00209000)
#define CHN3_EMI_APB_BASE		(IO_PHYS + 0x0020D000)
#define EMI_APB_BASE			(IO_PHYS + 0x00429000)
#define INFRA_EMI_DEBUG_CFG_BASE	(IO_PHYS + 0x00425000)
#define NEMI_SMPU_BASE			(IO_PHYS + 0x0042f000)
#define SEMI_SMPU_BASE			(IO_PHYS + 0x0052f000)
#define SUB_EMI_APB_BASE		(IO_PHYS + 0x00529000)
#define SUB_INFRA_EMI_DEBUG_CFG_BASE	(IO_PHYS + 0x00525000)
#define SUB_INFRACFG_AO_MEM_BASE	(IO_PHYS + 0x00504000)

/*******************************************************************************
 * System counter frequency related constants
 ******************************************************************************/
#define SYS_COUNTER_FREQ_IN_HZ	(13000000)
#define SYS_COUNTER_FREQ_IN_MHZ	(13)

/*******************************************************************************
 * Generic platform constants
 ******************************************************************************/
#define PLATFORM_STACK_SIZE		(0x800)
#define SOC_CHIP_ID			U(0x8196)

/*******************************************************************************
 * Platform memory map related constants
 ******************************************************************************/
#define TZRAM_BASE			(0x94600000)
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
 * Platform specific page table and MMU setup constants
 ******************************************************************************/
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 39)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 39)
#define MAX_XLAT_TABLES			(128)
#define MAX_MMAP_REGIONS		(512)

/*******************************************************************************
 * CPU_EB TCM handling related constants
 ******************************************************************************/
#define CPU_EB_TCM_BASE		0x0C2CF000
#define CPU_EB_TCM_SIZE		0x1000
#define CPU_EB_TCM_CNT_BASE	0x0C2CC000

/*******************************************************************************
 * CPU PM definitions
 ******************************************************************************/
#define PLAT_CPU_PM_B_BUCK_ISO_ID	(6)
#define PLAT_CPU_PM_ILDO_ID		(6)

/*******************************************************************************
 * SYSTIMER related definitions
 ******************************************************************************/
#define SYSTIMER_BASE		(0x1C400000)

/*******************************************************************************
 * CKSYS related constants
 ******************************************************************************/
#define CKSYS_BASE		(IO_PHYS)

/*******************************************************************************
 * VLP AO related constants
 ******************************************************************************/
#define VLPCFG_BUS_BASE		(IO_PHYS + 0x0C001000)
#define VLPCFG_BUS_SIZE		(0x1000)
#define VLP_AO_DEVAPC_APB_BASE	(IO_PHYS + 0x0C550000)
#define VLP_AO_DEVAPC_APB_SIZE	(0x1000)

/*******************************************************************************
 * SCP registers
 ******************************************************************************/
#define SCP_CLK_CTRL_BASE	(IO_PHYS + 0x0CF21000)
#define SCP_CLK_CTRL_SIZE	(0x1000)

#define SCP_CFGREG_BASE		(IO_PHYS + 0x0CF24000)
#define SCP_CFGREG_SIZE		(0x1000)

/*******************************************************************************
 * VLP CKSYS related constants
 ******************************************************************************/
#define VLP_CKSYS_BASE		(IO_PHYS + 0x0C016000)
#define VLP_CKSYS_SIZE		0x1000

/*******************************************************************************
 * PERI related constants use PERI secure address to garuantee access
 ******************************************************************************/
#define PERICFG_AO_SIZE		0x1000
#define PERI_CG0_STA		(PERICFG_AO_BASE + 0x10)
#define PERI_CLK_CON		(PERICFG_AO_BASE + 0x20)
#define PERI_CG1_CLR		(PERICFG_AO_BASE + 0x30)

/******************************************************************************
 * LPM syssram related constants
 *****************************************************************************/
#define MTK_LPM_SRAM_BASE	0x11B000
#define MTK_LPM_SRAM_MAP_SIZE	0x1000

/*******************************************************************************
 * SSPM_MBOX_3 related constants
 ******************************************************************************/
#define SSPM_MBOX_3_BASE	(IO_PHYS + 0x0C380000)
#define SSPM_MBOX_3_SIZE	0x1000

/*******************************************************************************
 * SSPM related constants
 ******************************************************************************/
#define SSPM_REG_OFFSET		(0x40000)
#define SSPM_CFGREG_BASE	(IO_PHYS + 0x0C300000 + SSPM_REG_OFFSET)
#define SSPM_CFGREG_SIZE	(0x1000)

#endif /* PLATFORM_DEF_H */
