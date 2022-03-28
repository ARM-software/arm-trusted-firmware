/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define	SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include "dcfg_lsch3.h"
#include "soc_default_base_addr.h"
#include "soc_default_helper_macros.h"

/*
 * SVR Definition of LS1088A
 * A: without security
 * AE: with security
 * (not include major and minor rev)
 */
#define SVR_LS1044A			0x870323
#define SVR_LS1044AE			0x870322
#define SVR_LS1048A			0x870321
#define SVR_LS1048AE			0x870320
#define SVR_LS1084A			0x870303
#define SVR_LS1084AE			0x870302
#define SVR_LS1088A			0x870301
#define SVR_LS1088AE			0x870300

#define SVR_WO_E			0xFFFFFE

/* Number of cores in platform */
#define NUMBER_OF_CLUSTERS		2
#define CORES_PER_CLUSTER		4
#define PLATFORM_CORE_COUNT		(NUMBER_OF_CLUSTERS * CORES_PER_CLUSTER)

/* set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS		1


#define NUM_DRAM_REGIONS		2
#define	NXP_DRAM0_ADDR			0x80000000
#define NXP_DRAM0_MAX_SIZE		0x80000000	/*  2 GB  */

#define NXP_DRAM1_ADDR			0x8080000000
#define NXP_DRAM1_MAX_SIZE		0x7F80000000	/* 510 G */

/* DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE			PLAT_DEF_DRAM0_SIZE

#define NXP_POWMGTDCR			0x700123C20

/* epu register offsets and values */
#define EPU_EPGCR_OFFSET		0x0
#define EPU_EPIMCR10_OFFSET		0x128
#define EPU_EPCTR10_OFFSET		0xa28
#define EPU_EPCCR10_OFFSET		0x828

#ifdef EPU_EPCCR10_VAL
#undef EPU_EPCCR10_VAL
#endif
#define EPU_EPCCR10_VAL			0xf2800000

#define EPU_EPIMCR10_VAL		0xba000000
#define EPU_EPCTR10_VAL			0x0
#define EPU_EPGCR_VAL			(1 << 31)

/* pmu register offsets and values */
#define PMU_PCPW20SR_OFFSET		0x830
#define PMU_CLAINACTSETR_OFFSET		0x1100
#define PMU_CLAINACTCLRR_OFFSET		0x1104
#define PMU_CLSINACTSETR_OFFSET		0x1108
#define PMU_CLSINACTCLRR_OFFSET		0x110C
#define PMU_CLL2FLUSHSETR_OFFSET	0x1110
#define PMU_CLSL2FLUSHCLRR_OFFSET	0x1114
#define PMU_CLL2FLUSHSR_OFFSET		0x1118
#define PMU_POWMGTCSR_OFFSET		0x4000
#define PMU_IPPDEXPCR0_OFFSET		0x4040
#define PMU_IPPDEXPCR1_OFFSET		0x4044
#define PMU_IPPDEXPCR2_OFFSET		0x4048
#define PMU_IPPDEXPCR3_OFFSET		0x404C
#define PMU_IPPDEXPCR4_OFFSET		0x4050
#define PMU_IPPDEXPCR5_OFFSET		0x4054
#define PMU_IPSTPCR0_OFFSET		0x4120
#define PMU_IPSTPCR1_OFFSET		0x4124
#define PMU_IPSTPCR2_OFFSET		0x4128
#define PMU_IPSTPCR3_OFFSET		0x412C
#define PMU_IPSTPCR4_OFFSET		0x4130
#define PMU_IPSTPCR5_OFFSET		0x4134
#define PMU_IPSTPCR6_OFFSET		0x4138
#define PMU_IPSTPACK0_OFFSET		0x4140
#define PMU_IPSTPACK1_OFFSET		0x4144
#define PMU_IPSTPACK2_OFFSET		0x4148
#define PMU_IPSTPACK3_OFFSET		0x414C
#define PMU_IPSTPACK4_OFFSET		0x4150
#define PMU_IPSTPACK5_OFFSET		0x4154
#define PMU_IPSTPACK6_OFFSET		0x4158
#define PMU_POWMGTCSR_VAL		(1 << 20)

#define IPPDEXPCR0_MASK			0xFFFFFFFF
#define IPPDEXPCR1_MASK			0xFFFFFFFF
#define IPPDEXPCR2_MASK			0xFFFFFFFF
#define IPPDEXPCR3_MASK			0xFFFFFFFF
#define IPPDEXPCR4_MASK			0xFFFFFFFF
#define IPPDEXPCR5_MASK			0xFFFFFFFF

/* DEVDISR5_FLX_TMR */
#define IPPDEXPCR_FLX_TMR		0x00004000
#define DEVDISR5_FLX_TMR		0x00004000

#define IPSTPCR0_VALUE			0x0041310C
#define IPSTPCR1_VALUE			0x000003FF
#define IPSTPCR2_VALUE			0x00013006

/* Dont' stop UART */
#define IPSTPCR3_VALUE			0x0000033A

#define IPSTPCR4_VALUE			0x00103300
#define IPSTPCR5_VALUE			0x00000001
#define IPSTPCR6_VALUE			0x00000000


#define TZPC_BLOCK_SIZE			0x1000

/* PORSR1 */
#define PORSR1_RCW_MASK			0xFF800000
#define PORSR1_RCW_SHIFT		23

/* CFG_RCW_SRC[6:0] */
#define RCW_SRC_TYPE_MASK		0x70

/* RCW SRC NOR */
#define	NOR_16B_VAL			0x20

/*
 * RCW SRC Serial Flash
 * 1. SERAIL NOR (QSPI)
 * 2. OTHERS (SD/MMC, SPI, I2C1)
 */
#define RCW_SRC_SERIAL_MASK		0x7F
#define QSPI_VAL			0x62
#define SDHC_VAL			0x40
#define EMMC_VAL			0x41

/*
 * Required LS standard platform porting definitions
 * for CCN-504 - Read from RN-F node ID register
 */
#define PLAT_CLUSTER_TO_CCN_ID_MAP 1, 9, 11, 19

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 40)

/*
 * Clock Divisors
 */
#define NXP_PLATFORM_CLK_DIVIDER	1
#define NXP_UART_CLK_DIVIDER		2

/* dcfg register offsets and values */
#define DCFG_DEVDISR1_OFFSET		0x70
#define DCFG_DEVDISR2_OFFSET		0x74
#define DCFG_DEVDISR3_OFFSET		0x78
#define DCFG_DEVDISR5_OFFSET		0x80
#define DCFG_DEVDISR6_OFFSET		0x84

#define DCFG_DEVDISR1_SEC		(1 << 22)
#define DCFG_DEVDISR3_QBMAIN		(1 << 12)
#define DCFG_DEVDISR4_SPI_QSPI		(1 << 4 | 1 << 5)
#define DCFG_DEVDISR5_MEM		(1 << 0)

#define DEVDISR1_VALUE			0x0041310c
#define DEVDISR2_VALUE			0x000003ff
#define DEVDISR3_VALUE			0x00013006
#define DEVDISR4_VALUE			0x0000033e
#define DEVDISR5_VALUE			0x00103300
#define DEVDISR6_VALUE			0x00000001

/*
 * pwr mgmt features supported in the soc-specific code:
 * value == 0x0, the soc code does not support this feature
 * value != 0x0, the soc code supports this feature
 */
#define SOC_CORE_RELEASE		0x1
#define SOC_CORE_RESTART		0x1
#define SOC_CORE_OFF			0x1
#define SOC_CORE_STANDBY		0x1
#define SOC_CORE_PWR_DWN		0x1
#define SOC_CLUSTER_STANDBY		0x1
#define SOC_CLUSTER_PWR_DWN		0x1
#define SOC_SYSTEM_STANDBY		0x1
#define SOC_SYSTEM_PWR_DWN		0x1
#define SOC_SYSTEM_OFF			0x1
#define SOC_SYSTEM_RESET		0x1

#define SYSTEM_PWR_DOMAINS		1
#define PLAT_NUM_PWR_DOMAINS	(PLATFORM_CORE_COUNT + \
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

#ifndef __ASSEMBLER__
/* CCI slave interfaces */
static const int cci_map[] = {
	3,
	4,
};
void soc_init_lowlevel(void);
void soc_init_percpu(void);
void _soc_set_start_addr(unsigned long addr);
void _set_platform_security(void);
#endif

#endif /* SOC_H */
