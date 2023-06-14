/*
 * Copyright (c) 2023, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef STM32MP2_DEF_H
#define STM32MP2_DEF_H

#include <common/tbbr/tbbr_img_def.h>
#ifndef __ASSEMBLER__
#include <drivers/st/bsec.h>
#endif
#include <dt-bindings/clock/stm32mp25-clks.h>
#include <dt-bindings/clock/stm32mp25-clksrc.h>
#include <dt-bindings/reset/stm32mp25-resets.h>

#ifndef __ASSEMBLER__
#include <boot_api.h>
#include <stm32mp_common.h>
#include <stm32mp_dt.h>
#include <stm32mp_shared_resources.h>
#endif

/*******************************************************************************
 * STM32MP2 memory map related constants
 ******************************************************************************/
#define STM32MP_SYSRAM_BASE			U(0x0E000000)
#define STM32MP_SYSRAM_SIZE			U(0x00040000)

#define STM32MP_SEC_SYSRAM_BASE			STM32MP_SYSRAM_BASE
#define STM32MP_SEC_SYSRAM_SIZE			STM32MP_SYSRAM_SIZE

/* DDR configuration */
#define STM32MP_DDR_BASE			U(0x80000000)
#define STM32MP_DDR_MAX_SIZE			UL(0x100000000)	/* Max 4GB */

/* DDR power initializations */
#ifndef __ASSEMBLER__
enum ddr_type {
	STM32MP_DDR3,
	STM32MP_DDR4,
	STM32MP_LPDDR4
};
#endif

#define STM32MP_BL2_SIZE			U(0x0002A000) /* 168 KB for BL2 */

#define STM32MP_BL2_BASE			(STM32MP_SEC_SYSRAM_BASE + \
						 STM32MP_SEC_SYSRAM_SIZE - \
						 STM32MP_BL2_SIZE)

/* BL2 and BL32/sp_min require 4 tables */
#define MAX_XLAT_TABLES				U(4)	/* 16 KB for mapping */

/*
 * MAX_MMAP_REGIONS is usually:
 * BL stm32mp2_mmap size + mmap regions in *_plat_arch_setup
 */
#define MAX_MMAP_REGIONS			6

#define STM32MP_BL33_BASE			(STM32MP_DDR_BASE + U(0x04000000))
#define STM32MP_BL33_MAX_SIZE			U(0x400000)

/*******************************************************************************
 * STM32MP2 RCC
 ******************************************************************************/
#define RCC_BASE				U(0x44200000)

/*******************************************************************************
 * STM32MP2 PWR
 ******************************************************************************/
#define PWR_BASE				U(0x44210000)

/*******************************************************************************
 * STM32MP2 SDMMC
 ******************************************************************************/
#define STM32MP_SDMMC1_BASE			U(0x48220000)
#define STM32MP_SDMMC2_BASE			U(0x48230000)
#define STM32MP_SDMMC3_BASE			U(0x48240000)

/*******************************************************************************
 * STM32MP2 TAMP
 ******************************************************************************/
#define PLAT_MAX_TAMP_INT			U(5)
#define PLAT_MAX_TAMP_EXT			U(3)
#define TAMP_BASE				U(0x46010000)
#define TAMP_SMCR				(TAMP_BASE + U(0x20))
#define TAMP_BKP_REGISTER_BASE			(TAMP_BASE + U(0x100))
#define TAMP_BKP_REG_CLK			CK_BUS_RTC
#define TAMP_BKP_SEC_NUMBER			U(10)
#define TAMP_COUNTR				U(0x40)

#if !(defined(__LINKER__) || defined(__ASSEMBLER__))
static inline uintptr_t tamp_bkpr(uint32_t idx)
{
	return TAMP_BKP_REGISTER_BASE + (idx << 2);
}
#endif

/*******************************************************************************
 * STM32MP2 DDRCTRL
 ******************************************************************************/
#define DDRCTRL_BASE				U(0x48040000)

/*******************************************************************************
 * STM32MP2 DDRDBG
 ******************************************************************************/
#define DDRDBG_BASE				U(0x48050000)

/*******************************************************************************
 * STM32MP2 DDRPHYC
 ******************************************************************************/
#define DDRPHYC_BASE				U(0x48C00000)

/*******************************************************************************
 * Miscellaneous STM32MP1 peripherals base address
 ******************************************************************************/
#define BSEC_BASE				U(0x44000000)
#define DBGMCU_BASE				U(0x4A010000)
#define HASH_BASE				U(0x42010000)
#define RTC_BASE				U(0x46000000)
#define STGEN_BASE				U(0x48080000)
#define SYSCFG_BASE				U(0x44230000)

/*******************************************************************************
 * REGULATORS
 ******************************************************************************/
/* 3 PWR + 1 VREFBUF + 14 PMIC regulators + 1 FIXED */
#define PLAT_NB_RDEVS				U(19)
/* 2 FIXED */
#define PLAT_NB_FIXED_REGUS			U(2)
/* No GPIO regu */
#define PLAT_NB_GPIO_REGUS			U(0)

/*******************************************************************************
 * Device Tree defines
 ******************************************************************************/
#define DT_BSEC_COMPAT				"st,stm32mp25-bsec"
#define DT_DDR_COMPAT				"st,stm32mp2-ddr"
#define DT_PWR_COMPAT				"st,stm32mp25-pwr"
#define DT_RCC_CLK_COMPAT			"st,stm32mp25-rcc"
#define DT_UART_COMPAT				"st,stm32h7-uart"

#endif /* STM32MP2_DEF_H */
