/*
 * Copyright 2017-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define	SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include "dcfg_lsch2.h"
#include "soc_default_base_addr.h"
#include "soc_default_helper_macros.h"

/* DDR Regions Info */
#define NUM_DRAM_REGIONS	3
#define	NXP_DRAM0_ADDR		0x80000000
#define NXP_DRAM0_MAX_SIZE	0x80000000	/*  2 GB  */

#define	NXP_DRAM1_ADDR		0x880000000
#define NXP_DRAM1_MAX_SIZE	0x780000000	/* 30 GB  */

#define	NXP_DRAM2_ADDR		0x8800000000
#define NXP_DRAM2_MAX_SIZE	0x7800000000	/* 480 GB */
/* DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE		PLAT_DEF_DRAM0_SIZE

/*
 * P23: 23 x 23 package
 * A: without security
 * AE: with security
 * SVR Definition (not include major and minor rev)
 */
#define SVR_LS1023A		0x879209
#define SVR_LS1023AE		0x879208
#define SVR_LS1023A_P23		0x87920B
#define SVR_LS1023AE_P23	0x87920A
#define SVR_LS1043A		0x879201
#define SVR_LS1043AE		0x879200
#define SVR_LS1043A_P23		0x879203
#define SVR_LS1043AE_P23	0x879202

/* Number of cores in platform */
#define PLATFORM_CORE_COUNT	4
#define NUMBER_OF_CLUSTERS	1
#define CORES_PER_CLUSTER	4

/* set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS			1

/*
 * Required LS standard platform porting definitions
 * for CCI-400
 */
#define NXP_CCI_CLUSTER0_SL_IFACE_IX	4

/* ls1043 version info for GIC configuration */
#define REV1_0			0x10
#define REV1_1			0x11
#define GIC_ADDR_BIT		31

/* Errata */
#define NXP_ERRATUM_A009663
#define NXP_ERRATUM_A009942

#define NUM_OF_DDRC	1

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	1
#define NXP_UART_CLK_DIVIDER		1

/*
 * Set this switch to 1 if you need to keep the debug block
 * clocked during system power-down.
 */
#define DEBUG_ACTIVE  0

#define IPPDEXPCR_MAC1_1          0x80000000    // DEVDISR2_FMAN1_MAC1
#define IPPDEXPCR_MAC1_2          0x40000000    // DEVDISR2_FMAN1_MAC2
#define IPPDEXPCR_MAC1_3          0x20000000    // DEVDISR2_FMAN1_MAC3
#define IPPDEXPCR_MAC1_4          0x10000000    // DEVDISR2_FMAN1_MAC4
#define IPPDEXPCR_MAC1_5          0x08000000    // DEVDISR2_FMAN1_MAC5
#define IPPDEXPCR_MAC1_6          0x04000000    // DEVDISR2_FMAN1_MAC6
#define IPPDEXPCR_MAC1_9          0x00800000    // DEVDISR2_FMAN1_MAC9
#define IPPDEXPCR_I2C1            0x00080000    // DEVDISR5_I2C_1
#define IPPDEXPCR_LPUART1         0x00040000    // DEVDISR5_LPUART1
#define IPPDEXPCR_FLX_TMR1        0x00020000    // DEVDISR5_FLX_TMR
#define IPPDEXPCR_OCRAM1          0x00010000    // DEVDISR5_OCRAM1
#define IPPDEXPCR_GPIO1           0x00000040    // DEVDISR5_GPIO
#define IPPDEXPCR_FM1             0x00000008    // DEVDISR2_FMAN1

#define IPPDEXPCR_MASK1           0xFC800008    // overrides for DEVDISR2
#define IPPDEXPCR_MASK2           0x000F0040    // overriddes for DEVDISR5

#define IPSTPCR0_VALUE            0xA000C201
#define IPSTPCR1_VALUE            0x00000080
#define IPSTPCR2_VALUE            0x000C0000
#define IPSTPCR3_VALUE            0x38000000
#if (DEBUG_ACTIVE)
  #define IPSTPCR4_VALUE          0x10833BFC
#else
  #define IPSTPCR4_VALUE          0x10A33BFC
#endif

#define DEVDISR1_QE               0x00000001
#define DEVDISR1_SEC              0x00000200
#define DEVDISR1_USB1             0x00004000
#define DEVDISR1_SATA             0x00008000
#define DEVDISR1_USB2             0x00010000
#define DEVDISR1_USB3             0x00020000
#define DEVDISR1_DMA2             0x00400000
#define DEVDISR1_DMA1             0x00800000
#define DEVDISR1_ESDHC            0x20000000
#define DEVDISR1_PBL              0x80000000

#define DEVDISR2_FMAN1            0x00000080
#define DEVDISR2_FMAN1_MAC9       0x00800000
#define DEVDISR2_FMAN1_MAC6       0x04000000
#define DEVDISR2_FMAN1_MAC5       0x08000000
#define DEVDISR2_FMAN1_MAC4       0x10000000
#define DEVDISR2_FMAN1_MAC3       0x20000000
#define DEVDISR2_FMAN1_MAC2       0x40000000
#define DEVDISR2_FMAN1_MAC1       0x80000000

#define DEVDISR3_BMAN             0x00040000
#define DEVDISR3_QMAN             0x00080000
#define DEVDISR3_PEX3             0x20000000
#define DEVDISR3_PEX2             0x40000000
#define DEVDISR3_PEX1             0x80000000

#define DEVDISR4_QSPI             0x08000000
#define DEVDISR4_DUART2           0x10000000
#define DEVDISR4_DUART1           0x20000000

#define DEVDISR5_ICMMU            0x00000001
#define DEVDISR5_I2C_1            0x00000002
#define DEVDISR5_I2C_2            0x00000004
#define DEVDISR5_I2C_3            0x00000008
#define DEVDISR5_I2C_4            0x00000010
#define DEVDISR5_WDG_5            0x00000020
#define DEVDISR5_WDG_4            0x00000040
#define DEVDISR5_WDG_3            0x00000080
#define DEVDISR5_DSPI1            0x00000100
#define DEVDISR5_WDG_2            0x00000200
#define DEVDISR5_FLX_TMR          0x00000400
#define DEVDISR5_WDG_1            0x00000800
#define DEVDISR5_LPUART6          0x00001000
#define DEVDISR5_LPUART5          0x00002000
#define DEVDISR5_LPUART3          0x00008000
#define DEVDISR5_LPUART2          0x00010000
#define DEVDISR5_LPUART1          0x00020000
#define DEVDISR5_DBG              0x00200000
#define DEVDISR5_GPIO             0x00400000
#define DEVDISR5_IFC              0x00800000
#define DEVDISR5_OCRAM2           0x01000000
#define DEVDISR5_OCRAM1           0x02000000
#define DEVDISR5_LPUART4          0x10000000
#define DEVDISR5_DDR              0x80000000
#define DEVDISR5_MEM              0x80000000

#define DEVDISR1_VALUE            0xA0C3C201
#define DEVDISR2_VALUE            0xCC0C0080
#define DEVDISR3_VALUE            0xE00C0000
#define DEVDISR4_VALUE            0x38000000
#if (DEBUG_ACTIVE)
  #define DEVDISR5_VALUE          0x10833BFC
#else
  #define DEVDISR5_VALUE          0x10A33BFC
#endif

/*
 * PWR mgmt features supported in the soc-specific code:
 *   value == 0x0  the soc code does not support this feature
 *   value != 0x0  the soc code supports this feature
 */
#define SOC_CORE_RELEASE       0x1
#define SOC_CORE_RESTART       0x1
#define SOC_CORE_OFF           0x1
#define SOC_CORE_STANDBY       0x1
#define SOC_CORE_PWR_DWN       0x1
#define SOC_CLUSTER_STANDBY    0x1
#define SOC_CLUSTER_PWR_DWN    0x1
#define SOC_SYSTEM_STANDBY     0x1
#define SOC_SYSTEM_PWR_DWN     0x1
#define SOC_SYSTEM_OFF         0x1
#define SOC_SYSTEM_RESET       0x1

/* PSCI-specific defines */
#define SYSTEM_PWR_DOMAINS 1
#define PLAT_NUM_PWR_DOMAINS   (PLATFORM_CORE_COUNT + \
				NUMBER_OF_CLUSTERS  + \
				SYSTEM_PWR_DOMAINS)

/* Power state coordination occurs at the system level */
#define PLAT_PD_COORD_LVL MPIDR_AFFLVL2
#define PLAT_MAX_PWR_LVL  PLAT_PD_COORD_LVL

/* Local power state for power domains in Run state */
#define LS_LOCAL_STATE_RUN  PSCI_LOCAL_STATE_RUN

/* define retention state */
#define PLAT_MAX_RET_STATE  (PSCI_LOCAL_STATE_RUN + 1)
#define LS_LOCAL_STATE_RET  PLAT_MAX_RET_STATE

/* define power-down state */
#define PLAT_MAX_OFF_STATE  (PLAT_MAX_RET_STATE + 1)
#define LS_LOCAL_STATE_OFF  PLAT_MAX_OFF_STATE

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 * CACHE_WRITEBACK_GRANULE is defined in soc.def
 */

/* One cache line needed for bakery locks on ARM platforms */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE (1 * CACHE_WRITEBACK_GRANULE)

#ifndef __ASSEMBLER__
/* CCI slave interfaces */
static const int cci_map[] = {
	NXP_CCI_CLUSTER0_SL_IFACE_IX,
};
void soc_init_lowlevel(void);
void soc_init_percpu(void);
void _soc_set_start_addr(unsigned long addr);

#endif

#endif /* SOC_H */
