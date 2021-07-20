/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _SOC_H
#define	_SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include <dcfg_lsch3.h>
#include <soc_default_base_addr.h>
#include <soc_default_helper_macros.h>


#define NUM_DRAM_REGIONS		3
#define	NXP_DRAM0_ADDR			0x80000000
#define NXP_DRAM0_MAX_SIZE		0x80000000	/*  2 GB  */

#define NXP_DRAM1_ADDR			0x2080000000
#define NXP_DRAM1_MAX_SIZE		0x1F80000000	/* 126 G */

#define NXP_DRAM2_ADDR			0x6000000000
#define NXP_DRAM2_MAX_SIZE		0x2000000000	/* 128G */

/*DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE			PLAT_DEF_DRAM0_SIZE

#define DDR_PLL_FIX
#define NXP_DDR_PHY1_ADDR		0x01400000
#define NXP_DDR_PHY2_ADDR		0x01600000

#if defined(IMAGE_BL31)
#define LS_SYS_TIMCTL_BASE		0x2890000

#ifdef LS_SYS_TIMCTL_BASE
#define PLAT_LS_NSTIMER_FRAME_ID	0
#define LS_CONFIG_CNTACR		1
#endif
#endif

/* Start: Macros used by soc.c: get_boot_dev */
#define PORSR1_RCW_MASK		0x07800000
#define PORSR1_RCW_SHIFT	23

#define SDHC1_VAL		0x8
#define SDHC2_VAL		0x9
#define I2C1_VAL		0xa
#define FLEXSPI_NAND2K_VAL	0xc
#define FLEXSPI_NAND4K_VAL	0xd
#define FLEXSPI_NOR		0xf
/* End: Macros used by soc.c: get_boot_dev */

/* SVR Definition (not include major and minor rev) */
#define SVR_LX2160A		0x873601
#define SVR_LX2120A		0x873621
#define SVR_LX2080A		0x873603

/* Number of cores in platform */
/* Used by common code for array initialization */
#define NUMBER_OF_CLUSTERS		8
#define CORES_PER_CLUSTER		2
#define PLATFORM_CORE_COUNT		NUMBER_OF_CLUSTERS * CORES_PER_CLUSTER

/*
 * Required LS standard platform porting definitions
 * for CCN-508
 */
#define PLAT_CLUSTER_TO_CCN_ID_MAP 11, 15, 27, 31, 12, 28, 16, 0
#define PLAT_6CLUSTER_TO_CCN_ID_MAP 11, 15, 27, 31, 12, 28


/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	2
#define NXP_UART_CLK_DIVIDER		4

/* Start: Macros used by lx2160a.S */
#define MPIDR_AFFINITY0_MASK			0x00FF
#define MPIDR_AFFINITY1_MASK			0xFF00
#define CPUECTLR_DISABLE_TWALK_PREFETCH		0x4000000000
#define CPUECTLR_INS_PREFETCH_MASK		0x1800000000
#define CPUECTLR_DAT_PREFETCH_MASK		0x0300000000
#define CPUECTLR_RET_8CLK			0x2
#define OSDLR_EL1_DLK_LOCK			0x1
#define CNTP_CTL_EL0_EN				0x1
#define CNTP_CTL_EL0_IMASK			0x2
/* set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS			1
/* End: Macros used by lx2160a.S */

/* Start: Macros used by lib/psci files */
#define SYSTEM_PWR_DOMAINS 1
#define PLAT_NUM_PWR_DOMAINS   (PLATFORM_CORE_COUNT + \
				NUMBER_OF_CLUSTERS  + \
				SYSTEM_PWR_DOMAINS)

/* Power state coordination occurs at the system level */
#define PLAT_MAX_PWR_LVL  MPIDR_AFFLVL2

/* define retention state */
#define PLAT_MAX_RET_STATE  (PSCI_LOCAL_STATE_RUN + 1)

/* define power-down state */
#define PLAT_MAX_OFF_STATE  (PLAT_MAX_RET_STATE + 1)
/* End: Macros used by lib/psci files */

/* Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 *
 * CACHE_WRITEBACK_GRANULE is defined in soc.def
 *
 * One cache line needed for bakery locks on ARM platforms
 */
#define PLAT_PERCPU_BAKERY_LOCK_SIZE (1 * CACHE_WRITEBACK_GRANULE)

#ifndef WDOG_RESET_FLAG
#define WDOG_RESET_FLAG DEFAULT_SET_VALUE
#endif

#ifndef WARM_BOOT_SUCCESS
#define WARM_BOOT_SUCCESS DEFAULT_SET_VALUE
#endif

#ifndef __ASSEMBLER__

void set_base_freq_CNTFID0(void);
void soc_init_start(void);
void soc_init_finish(void);
void soc_init_percpu(void);
void _soc_set_start_addr(unsigned long addr);
void _set_platform_security(void);

#endif

#endif /* _SOC_H */
