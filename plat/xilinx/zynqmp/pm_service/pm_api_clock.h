/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * ZynqMP system level PM-API functions for clock control.
 */

#ifndef PM_API_CLOCK_H
#define PM_API_CLOCK_H

#include <lib/utils_def.h>

#include "pm_common.h"

#define CLK_NAME_LEN		U(15)
#define MAX_PARENTS		U(100)
#define CLK_NA_PARENT		-1
#define CLK_DUMMY_PARENT	-2

/* Flags for parent id */
#define PARENT_CLK_SELF		U(0)
#define PARENT_CLK_NODE1	U(1)
#define PARENT_CLK_NODE2	U(2)
#define PARENT_CLK_NODE3	U(3)
#define PARENT_CLK_NODE4	U(4)
#define PARENT_CLK_EXTERNAL	U(5)
#define PARENT_CLK_MIO0_MIO77	U(6)

#define CLK_SET_RATE_GATE	BIT(0) /* must be gated across rate change */
#define CLK_SET_PARENT_GATE	BIT(1) /* must be gated across re-parent */
#define CLK_SET_RATE_PARENT	BIT(2) /* propagate rate change up one level */
#define CLK_IGNORE_UNUSED	BIT(3) /* do not gate even if unused */
/* unused */
#define CLK_IS_BASIC		BIT(5) /* Basic clk, can't do a to_clk_foo() */
#define CLK_GET_RATE_NOCACHE	BIT(6) /* do not use the cached clk rate */
#define CLK_SET_RATE_NO_REPARENT BIT(7) /* don't re-parent on rate change */
#define CLK_GET_ACCURACY_NOCACHE BIT(8) /* do not use the cached clk accuracy */
#define CLK_RECALC_NEW_RATES	BIT(9) /* recalc rates after notifications */
#define CLK_SET_RATE_UNGATE	BIT(10) /* clock needs to run to set rate */
#define CLK_IS_CRITICAL		BIT(11) /* do not gate, ever */
/* parents need enable during gate/ungate, set rate and re-parent */
#define CLK_OPS_PARENT_ENABLE	BIT(12)

#define CLK_DIVIDER_ONE_BASED		BIT(0)
#define CLK_DIVIDER_POWER_OF_TWO	BIT(1)
#define CLK_DIVIDER_ALLOW_ZERO		BIT(2)
#define CLK_DIVIDER_HIWORD_MASK		BIT(3)
#define CLK_DIVIDER_ROUND_CLOSEST	BIT(4)
#define CLK_DIVIDER_READ_ONLY		BIT(5)
#define CLK_DIVIDER_MAX_AT_ZERO		BIT(6)
#define CLK_FRAC		BIT(8)

#define END_OF_CLK     "END_OF_CLK"

//CLock Ids
enum clock_id {
	CLK_IOPLL,
	CLK_RPLL,
	CLK_APLL,
	CLK_DPLL,
	CLK_VPLL,
	CLK_IOPLL_TO_FPD,
	CLK_RPLL_TO_FPD,
	CLK_APLL_TO_LPD,
	CLK_DPLL_TO_LPD,
	CLK_VPLL_TO_LPD,
	CLK_ACPU,
	CLK_ACPU_HALF,
	CLK_DBG_FPD,
	CLK_DBG_LPD,
	CLK_DBG_TRACE,
	CLK_DBG_TSTMP,
	CLK_DP_VIDEO_REF,
	CLK_DP_AUDIO_REF,
	CLK_DP_STC_REF,
	CLK_GDMA_REF,
	CLK_DPDMA_REF,
	CLK_DDR_REF,
	CLK_SATA_REF,
	CLK_PCIE_REF,
	CLK_GPU_REF,
	CLK_GPU_PP0_REF,
	CLK_GPU_PP1_REF,
	CLK_TOPSW_MAIN,
	CLK_TOPSW_LSBUS,
	CLK_GTGREF0_REF,
	CLK_LPD_SWITCH,
	CLK_LPD_LSBUS,
	CLK_USB0_BUS_REF,
	CLK_USB1_BUS_REF,
	CLK_USB3_DUAL_REF,
	CLK_USB0,
	CLK_USB1,
	CLK_CPU_R5,
	CLK_CPU_R5_CORE,
	CLK_CSU_SPB,
	CLK_CSU_PLL,
	CLK_PCAP,
	CLK_IOU_SWITCH,
	CLK_GEM_TSU_REF,
	CLK_GEM_TSU,
	CLK_GEM0_TX,
	CLK_GEM1_TX,
	CLK_GEM2_TX,
	CLK_GEM3_TX,
	CLK_GEM0_RX,
	CLK_GEM1_RX,
	CLK_GEM2_RX,
	CLK_GEM3_RX,
	CLK_QSPI_REF,
	CLK_SDIO0_REF,
	CLK_SDIO1_REF,
	CLK_UART0_REF,
	CLK_UART1_REF,
	CLK_SPI0_REF,
	CLK_SPI1_REF,
	CLK_NAND_REF,
	CLK_I2C0_REF,
	CLK_I2C1_REF,
	CLK_CAN0_REF,
	CLK_CAN1_REF,
	CLK_CAN0,
	CLK_CAN1,
	CLK_DLL_REF,
	CLK_ADMA_REF,
	CLK_TIMESTAMP_REF,
	CLK_AMS_REF,
	CLK_PL0_REF,
	CLK_PL1_REF,
	CLK_PL2_REF,
	CLK_PL3_REF,
	CLK_FPD_WDT,
	CLK_IOPLL_INT,
	CLK_IOPLL_PRE_SRC,
	CLK_IOPLL_HALF,
	CLK_IOPLL_INT_MUX,
	CLK_IOPLL_POST_SRC,
	CLK_RPLL_INT,
	CLK_RPLL_PRE_SRC,
	CLK_RPLL_HALF,
	CLK_RPLL_INT_MUX,
	CLK_RPLL_POST_SRC,
	CLK_APLL_INT,
	CLK_APLL_PRE_SRC,
	CLK_APLL_HALF,
	CLK_APLL_INT_MUX,
	CLK_APLL_POST_SRC,
	CLK_DPLL_INT,
	CLK_DPLL_PRE_SRC,
	CLK_DPLL_HALF,
	CLK_DPLL_INT_MUX,
	CLK_DPLL_POST_SRC,
	CLK_VPLL_INT,
	CLK_VPLL_PRE_SRC,
	CLK_VPLL_HALF,
	CLK_VPLL_INT_MUX,
	CLK_VPLL_POST_SRC,
	CLK_CAN0_MIO,
	CLK_CAN1_MIO,
	CLK_ACPU_FULL,
	CLK_GEM0_REF,
	CLK_GEM1_REF,
	CLK_GEM2_REF,
	CLK_GEM3_REF,
	CLK_GEM0_REF_UNGATED,
	CLK_GEM1_REF_UNGATED,
	CLK_GEM2_REF_UNGATED,
	CLK_GEM3_REF_UNGATED,
	CLK_LPD_WDT,
	END_OF_OUTPUT_CLKS,
};

#define CLK_MAX_OUTPUT_CLK (unsigned int)(END_OF_OUTPUT_CLKS)

//External clock ids
enum {
	EXT_CLK_PSS_REF = END_OF_OUTPUT_CLKS,
	EXT_CLK_VIDEO,
	EXT_CLK_PSS_ALT_REF,
	EXT_CLK_AUX_REF,
	EXT_CLK_GT_CRX_REF,
	EXT_CLK_SWDT0,
	EXT_CLK_SWDT1,
	EXT_CLK_GEM0_TX_EMIO,
	EXT_CLK_GEM1_TX_EMIO,
	EXT_CLK_GEM2_TX_EMIO,
	EXT_CLK_GEM3_TX_EMIO,
	EXT_CLK_GEM0_RX_EMIO,
	EXT_CLK_GEM1_RX_EMIO,
	EXT_CLK_GEM2_RX_EMIO,
	EXT_CLK_GEM3_RX_EMIO,
	EXT_CLK_MIO50_OR_MIO51,
	EXT_CLK_MIO0,
	EXT_CLK_MIO1,
	EXT_CLK_MIO2,
	EXT_CLK_MIO3,
	EXT_CLK_MIO4,
	EXT_CLK_MIO5,
	EXT_CLK_MIO6,
	EXT_CLK_MIO7,
	EXT_CLK_MIO8,
	EXT_CLK_MIO9,
	EXT_CLK_MIO10,
	EXT_CLK_MIO11,
	EXT_CLK_MIO12,
	EXT_CLK_MIO13,
	EXT_CLK_MIO14,
	EXT_CLK_MIO15,
	EXT_CLK_MIO16,
	EXT_CLK_MIO17,
	EXT_CLK_MIO18,
	EXT_CLK_MIO19,
	EXT_CLK_MIO20,
	EXT_CLK_MIO21,
	EXT_CLK_MIO22,
	EXT_CLK_MIO23,
	EXT_CLK_MIO24,
	EXT_CLK_MIO25,
	EXT_CLK_MIO26,
	EXT_CLK_MIO27,
	EXT_CLK_MIO28,
	EXT_CLK_MIO29,
	EXT_CLK_MIO30,
	EXT_CLK_MIO31,
	EXT_CLK_MIO32,
	EXT_CLK_MIO33,
	EXT_CLK_MIO34,
	EXT_CLK_MIO35,
	EXT_CLK_MIO36,
	EXT_CLK_MIO37,
	EXT_CLK_MIO38,
	EXT_CLK_MIO39,
	EXT_CLK_MIO40,
	EXT_CLK_MIO41,
	EXT_CLK_MIO42,
	EXT_CLK_MIO43,
	EXT_CLK_MIO44,
	EXT_CLK_MIO45,
	EXT_CLK_MIO46,
	EXT_CLK_MIO47,
	EXT_CLK_MIO48,
	EXT_CLK_MIO49,
	EXT_CLK_MIO50,
	EXT_CLK_MIO51,
	EXT_CLK_MIO52,
	EXT_CLK_MIO53,
	EXT_CLK_MIO54,
	EXT_CLK_MIO55,
	EXT_CLK_MIO56,
	EXT_CLK_MIO57,
	EXT_CLK_MIO58,
	EXT_CLK_MIO59,
	EXT_CLK_MIO60,
	EXT_CLK_MIO61,
	EXT_CLK_MIO62,
	EXT_CLK_MIO63,
	EXT_CLK_MIO64,
	EXT_CLK_MIO65,
	EXT_CLK_MIO66,
	EXT_CLK_MIO67,
	EXT_CLK_MIO68,
	EXT_CLK_MIO69,
	EXT_CLK_MIO70,
	EXT_CLK_MIO71,
	EXT_CLK_MIO72,
	EXT_CLK_MIO73,
	EXT_CLK_MIO74,
	EXT_CLK_MIO75,
	EXT_CLK_MIO76,
	EXT_CLK_MIO77,
	END_OF_CLKS,
};

#define CLK_MAX (unsigned int)(END_OF_CLKS)

//CLock types
#define CLK_TYPE_OUTPUT 0U
#define	CLK_TYPE_EXTERNAL  1U

//Topology types
#define TYPE_INVALID 0U
#define	TYPE_MUX 1U
#define	TYPE_PLL 2U
#define	TYPE_FIXEDFACTOR 3U
#define	TYPE_DIV1 4U
#define	TYPE_DIV2 5U
#define	TYPE_GATE 6U

struct pm_pll;
struct pm_pll *pm_clock_get_pll(enum clock_id clock_id);
struct pm_pll *pm_clock_get_pll_by_related_clk(enum clock_id clock_id);
uint8_t pm_clock_has_div(unsigned int clock_id, enum pm_clock_div_id div_id);

enum pm_ret_status pm_api_clock_get_name(unsigned int clock_id, char *name);
enum pm_ret_status pm_api_clock_get_num_clocks(unsigned int *nclocks);
enum pm_ret_status pm_api_clock_get_topology(unsigned int clock_id,
					     unsigned int index,
					     uint32_t *topology);
enum pm_ret_status pm_api_clock_get_fixedfactor_params(unsigned int clock_id,
						       uint32_t *mul,
						       uint32_t *div);
enum pm_ret_status pm_api_clock_get_parents(unsigned int clock_id,
					    unsigned int index,
					    uint32_t *parents);
enum pm_ret_status pm_api_clock_get_attributes(unsigned int clock_id,
					       uint32_t *attr);
enum pm_ret_status pm_api_clock_get_max_divisor(enum clock_id clock_id,
						uint8_t div_type,
						uint32_t *max_div);

enum pm_ret_status pm_clock_get_pll_node_id(enum clock_id clock_id,
					    enum pm_node_id *node_id);
enum pm_ret_status pm_clock_id_is_valid(unsigned int clock_id);

enum pm_ret_status pm_clock_pll_enable(struct pm_pll *pll);
enum pm_ret_status pm_clock_pll_disable(struct pm_pll *pll);
enum pm_ret_status pm_clock_pll_get_state(struct pm_pll *pll,
					  unsigned int *state);
enum pm_ret_status pm_clock_pll_set_parent(struct pm_pll *pll,
					   enum clock_id clock_id,
					   unsigned int parent_index);
enum pm_ret_status pm_clock_pll_get_parent(struct pm_pll *pll,
					   enum clock_id clock_id,
					   unsigned int *parent_index);
enum pm_ret_status pm_clock_set_pll_mode(enum clock_id clock_id,
					 unsigned int mode);
enum pm_ret_status pm_clock_get_pll_mode(enum clock_id clock_id,
					 unsigned int *mode);

#endif /* PM_API_CLOCK_H */
