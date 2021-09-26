/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOC_H
#define	SOC_H

/* Chassis specific defines - common across SoC's of a particular platform */
#include <dcfg_lsch3.h>
#include <soc_default_base_addr.h>
#include <soc_default_helper_macros.h>

/*
 * SVR Definition of LS1028A
 * (not include major and minor rev)
 * These info is listed in Table B-6. DCFG differences
 * between LS1028A and LS1027A of LS1028ARM(Reference Manual)
 */
#define SVR_LS1017AN		0x870B25
#define SVR_LS1017AE		0x870B24
#define SVR_LS1018AN		0x870B21
#define SVR_LS1018AE		0x870B20
#define SVR_LS1027AN		0x870B05
#define SVR_LS1027AE		0x870B04
#define SVR_LS1028AN		0x870B01
#define SVR_LS1028AE		0x870B00

/* Number of cores in platform */
#define PLATFORM_CORE_COUNT		2
#define NUMBER_OF_CLUSTERS		1
#define CORES_PER_CLUSTER		2

/* Set to 0 if the clusters are not symmetrical */
#define SYMMETRICAL_CLUSTERS		1

#define NUM_DRAM_REGIONS		3

#define	NXP_DRAM0_ADDR			0x80000000
#define NXP_DRAM0_MAX_SIZE		0x80000000	/* 2GB */

#define NXP_DRAM1_ADDR			0x2080000000
#define NXP_DRAM1_MAX_SIZE		0x1F80000000	/* 126G */

#define NXP_DRAM2_ADDR			0x6000000000
#define NXP_DRAM2_MAX_SIZE		0x2000000000	/* 128G */

/* DRAM0 Size defined in platform_def.h */
#define	NXP_DRAM0_SIZE			PLAT_DEF_DRAM0_SIZE

/* CCSR space memory Map  */
#undef NXP_UART_ADDR
#define NXP_UART_ADDR			0x021C0500

#undef NXP_UART1_ADDR
#define NXP_UART1_ADDR			0x021C0600

#undef NXP_WDOG1_TZ_ADDR
#define NXP_WDOG1_TZ_ADDR		0x023C0000

#undef NXP_GICR_ADDR
#define NXP_GICR_ADDR			0x06040000

#undef NXP_GICR_SGI_ADDR
#define NXP_GICR_SGI_ADDR		0x06050000

/* EPU register offsets and values */
#define EPU_EPGCR_OFFSET              0x0
#define EPU_EPIMCR10_OFFSET           0x128
#define EPU_EPCTR10_OFFSET            0xa28
#define EPU_EPCCR10_OFFSET            0x828
#define EPU_EPCCR10_VAL               0xb2800000
#define EPU_EPIMCR10_VAL              0xba000000
#define EPU_EPCTR10_VAL               0x0
#define EPU_EPGCR_VAL                 (1 << 31)

/* PORSR1 */
#define PORSR1_RCW_MASK		0x07800000
#define PORSR1_RCW_SHIFT	23

#define SDHC1_VAL		0x8
#define SDHC2_VAL		0x9
#define I2C1_VAL		0xa
#define FLEXSPI_NAND2K_VAL	0xc
#define FLEXSPI_NAND4K_VAL	0xd
#define FLEXSPI_NOR		0xf

/*
 * Required LS standard platform porting definitions
 * for CCI-400
 */
#define NXP_CCI_CLUSTER0_SL_IFACE_IX	4

/* Defines required for using XLAT tables from ARM common code */
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ull << 40)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ull << 40)

/* Clock Divisors */
#define NXP_PLATFORM_CLK_DIVIDER	1
#define NXP_UART_CLK_DIVIDER		2

/* dcfg register offsets and values */
#define DCFG_DEVDISR2_ENETC		(1 << 31)

#define MPIDR_AFFINITY0_MASK		0x00FF
#define MPIDR_AFFINITY1_MASK		0xFF00
#define CPUECTLR_DISABLE_TWALK_PREFETCH	0x4000000000
#define CPUECTLR_INS_PREFETCH_MASK	0x1800000000
#define CPUECTLR_DAT_PREFETCH_MASK	0x0300000000
#define OSDLR_EL1_DLK_LOCK		0x1
#define CNTP_CTL_EL0_EN			0x1
#define CNTP_CTL_EL0_IMASK		0x2

#define SYSTEM_PWR_DOMAINS	1
#define PLAT_NUM_PWR_DOMAINS	(PLATFORM_CORE_COUNT + \
				 NUMBER_OF_CLUSTERS  + \
				 SYSTEM_PWR_DOMAINS)

/* Power state coordination occurs at the system level */
#define PLAT_PD_COORD_LVL	MPIDR_AFFLVL2
#define PLAT_MAX_PWR_LVL	PLAT_PD_COORD_LVL

/* Local power state for power domains in Run state */
#define LS_LOCAL_STATE_RUN	PSCI_LOCAL_STATE_RUN

/* define retention state */
#define PLAT_MAX_RET_STATE	(PSCI_LOCAL_STATE_RUN + 1)
#define LS_LOCAL_STATE_RET	PLAT_MAX_RET_STATE

/* define power-down state */
#define PLAT_MAX_OFF_STATE	(PLAT_MAX_RET_STATE + 1)
#define LS_LOCAL_STATE_OFF	PLAT_MAX_OFF_STATE

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
void _set_platform_security(void);
#endif

#endif /* SOC_H */
