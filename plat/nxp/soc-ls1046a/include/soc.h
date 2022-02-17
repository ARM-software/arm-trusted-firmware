/*
 * Copyright 2018-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef SOC_H
#define	SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include <dcfg_lsch2.h>

#include <soc_default_base_addr.h>
#include <soc_default_helper_macros.h>

/* DDR Regions Info */
#define NUM_DRAM_REGIONS		U(3)
#define	NXP_DRAM0_ADDR			ULL(0x80000000)
#define NXP_DRAM0_MAX_SIZE		ULL(0x80000000)	/*  2 GB  */

#define	NXP_DRAM1_ADDR			ULL(0x880000000)
#define NXP_DRAM1_MAX_SIZE		ULL(0x780000000)	/* 30 GB  */

#define	NXP_DRAM2_ADDR			ULL(0x8800000000)
#define NXP_DRAM2_MAX_SIZE		ULL(0x7800000000)	/* 480 GB */

/*DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE			PLAT_DEF_DRAM0_SIZE

/*
 * SVR Definition (not include major and minor rev)
 * A: without security
 * AE: with security
 */
#define SVR_LS1026A			0x870709
#define SVR_LS1026AE			0x870708
#define SVR_LS1046A			0x870701
#define SVR_LS1046AE			0x870700

/* Number of cores in platform */
/* Used by common code for array initialization */
#define NUMBER_OF_CLUSTERS		U(1)
#define CORES_PER_CLUSTER		U(4)
#define PLATFORM_CORE_COUNT		(NUMBER_OF_CLUSTERS * CORES_PER_CLUSTER)

/*
 * Required LS standard platform porting definitions
 * for CCI-400
 */
#define NXP_CCI_CLUSTER0_SL_IFACE_IX	4


/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	U(1)
#define NXP_UART_CLK_DIVIDER		U(2)

/* set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS		U(1)

 /*
  * set this switch to 1 if you need to keep the debug block
  * clocked during system power-down
  */
#define DEBUG_ACTIVE			0

 /*
  * pwr mgmt features supported in the soc-specific code:
  *   value == 0x0  the soc code does not support this feature
  *   value != 0x0  the soc code supports this feature
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

/* Start: Macros used by lib/psci files */
#define SYSTEM_PWR_DOMAINS		1
#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CORE_COUNT + \
					NUMBER_OF_CLUSTERS  + \
					SYSTEM_PWR_DOMAINS)

/* Power state coordination occurs at the system level */
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL2

/* define retention state */
#define PLAT_MAX_RET_STATE		(PSCI_LOCAL_STATE_RUN + 1)

/* define power-down state */
#define PLAT_MAX_OFF_STATE		(PLAT_MAX_RET_STATE + 1)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 *
 * CACHE_WRITEBACK_GRANULE is defined in soc.def
 */

 /* One cache line needed for bakery locks on ARM platforms */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE	(1 * CACHE_WRITEBACK_GRANULE)

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
