/*
 * Copyright (c) 2018-2020, ARM Limited and Contributors. All rights reserved.
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

#define CLK_NAME_LEN		(15U)
#define MAX_PARENTS		(100U)
#define CLK_NA_PARENT		-1
#define CLK_DUMMY_PARENT	-2

/* Flags for parent id */
#define PARENT_CLK_SELF		(0U)
#define PARENT_CLK_NODE1	(1U)
#define PARENT_CLK_NODE2	(2U)
#define PARENT_CLK_NODE3	(3U)
#define PARENT_CLK_NODE4	(4U)
#define PARENT_CLK_EXTERNAL	(5U)
#define PARENT_CLK_MIO0_MIO77	(6U)

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
	CLK_IOPLL = (0U),
	CLK_RPLL  = (1U),
	CLK_APLL  = (2U),
	CLK_DPLL  = (3U),
	CLK_VPLL  = (4U),
	CLK_IOPLL_TO_FPD = (5U),
	CLK_RPLL_TO_FPD = (6U),
	CLK_APLL_TO_LPD = (7U),
	CLK_DPLL_TO_LPD = (8U),
	CLK_VPLL_TO_LPD = (9U),
	CLK_ACPU = (10U),
	CLK_ACPU_HALF = (11U),
	CLK_DBG_FPD = (12U),
	CLK_DBG_LPD = (13U),
	CLK_DBG_TRACE = (14U),
	CLK_DBG_TSTMP = (15U),
	CLK_DP_VIDEO_REF = (16U),
	CLK_DP_AUDIO_REF = (17U),
	CLK_DP_STC_REF = (18U),
	CLK_GDMA_REF = (19U),
	CLK_DPDMA_REF = (20U),
	CLK_DDR_REF = (21U),
	CLK_SATA_REF = (22U),
	CLK_PCIE_REF = (23U),
	CLK_GPU_REF = (24U),
	CLK_GPU_PP0_REF = (25U),
	CLK_GPU_PP1_REF = (26U),
	CLK_TOPSW_MAIN = (27U),
	CLK_TOPSW_LSBUS = (28U),
	CLK_GTGREF0_REF = (29U),
	CLK_LPD_SWITCH = (30U),
	CLK_LPD_LSBUS = (31U),
	CLK_USB0_BUS_REF = (32U),
	CLK_USB1_BUS_REF = (33U),
	CLK_USB3_DUAL_REF = (34U),
	CLK_USB0 = (35U),
	CLK_USB1 = (36U),
	CLK_CPU_R5 = (37U),
	CLK_CPU_R5_CORE = (38U),
	CLK_CSU_SPB = (39U),
	CLK_CSU_PLL = (40U),
	CLK_PCAP = (41U),
	CLK_IOU_SWITCH = (42U),
	CLK_GEM_TSU_REF = (43U),
	CLK_GEM_TSU = (44U),
	CLK_GEM0_TX = (45U),
	CLK_GEM1_TX = (46U),
	CLK_GEM2_TX = (47U),
	CLK_GEM3_TX = (48U),
	CLK_GEM0_RX = (49U),
	CLK_GEM1_RX = (50U),
	CLK_GEM2_RX = (51U),
	CLK_GEM3_RX = (52U),
	CLK_QSPI_REF = (53U),
	CLK_SDIO0_REF = (54U),
	CLK_SDIO1_REF = (55U),
	CLK_UART0_REF = (56U),
	CLK_UART1_REF = (57U),
	CLK_SPI0_REF = (58U),
	CLK_SPI1_REF = (59U),
	CLK_NAND_REF = (60U),
	CLK_I2C0_REF = (61U),
	CLK_I2C1_REF = (62U),
	CLK_CAN0_REF = (63U),
	CLK_CAN1_REF = (64U),
	CLK_CAN0 = (65U),
	CLK_CAN1 = (66U),
	CLK_DLL_REF = (67U),
	CLK_ADMA_REF = (68U),
	CLK_TIMESTAMP_REF = (69U),
	CLK_AMS_REF = (70U),
	CLK_PL0_REF = (71U),
	CLK_PL1_REF = (72U),
	CLK_PL2_REF = (73U),
	CLK_PL3_REF = (74U),
	CLK_FPD_WDT = (75U),
	CLK_IOPLL_INT = (76U),
	CLK_IOPLL_PRE_SRC = (77U),
	CLK_IOPLL_HALF = (78U),
	CLK_IOPLL_INT_MUX = (79U),
	CLK_IOPLL_POST_SRC = (80U),
	CLK_RPLL_INT = (81U),
	CLK_RPLL_PRE_SRC = (82U),
	CLK_RPLL_HALF = (83U),
	CLK_RPLL_INT_MUX = (84U),
	CLK_RPLL_POST_SRC = (85U),
	CLK_APLL_INT = (86U),
	CLK_APLL_PRE_SRC = (87U),
	CLK_APLL_HALF = (88U),
	CLK_APLL_INT_MUX = (89U),
	CLK_APLL_POST_SRC = (90U),
	CLK_DPLL_INT = (91U),
	CLK_DPLL_PRE_SRC = (92U),
	CLK_DPLL_HALF = (93U),
	CLK_DPLL_INT_MUX = (94U),
	CLK_DPLL_POST_SRC = (95U),
	CLK_VPLL_INT = (96U),
	CLK_VPLL_PRE_SRC = (97U),
	CLK_VPLL_HALF = (98U),
	CLK_VPLL_INT_MUX = (99U),
	CLK_VPLL_POST_SRC = (100U),
	CLK_CAN0_MIO = (101U),
	CLK_CAN1_MIO = (102U),
	CLK_ACPU_FULL = (103U),
	CLK_GEM0_REF = (104U),
	CLK_GEM1_REF = (105U),
	CLK_GEM2_REF = (106U),
	CLK_GEM3_REF = (107U),
	CLK_GEM0_REF_UNGATED = (108U),
	CLK_GEM1_REF_UNGATED = (109U),
	CLK_GEM2_REF_UNGATED = (110U),
	CLK_GEM3_REF_UNGATED = (111U),
	CLK_LPD_WDT = (112U),
	END_OF_OUTPUT_CLKS = (113U),
};

#define CLK_MAX_OUTPUT_CLK END_OF_OUTPUT_CLKS

//External clock ids
enum {
	EXT_CLK_PSS_REF = END_OF_OUTPUT_CLKS,
	EXT_CLK_VIDEO = (114U),
	EXT_CLK_PSS_ALT_REF = (115U),
	EXT_CLK_AUX_REF = (116U),
	EXT_CLK_GT_CRX_REF = (117U),
	EXT_CLK_SWDT0 = (118U),
	EXT_CLK_SWDT1 = (119U),
	EXT_CLK_GEM0_TX_EMIO = (120U),
	EXT_CLK_GEM1_TX_EMIO = (121U),
	EXT_CLK_GEM2_TX_EMIO = (122U),
	EXT_CLK_GEM3_TX_EMIO = (123U),
	EXT_CLK_GEM0_RX_EMIO = (124U),
	EXT_CLK_GEM1_RX_EMIO = (125U),
	EXT_CLK_GEM2_RX_EMIO = (126U),
	EXT_CLK_GEM3_RX_EMIO = (127U),
	EXT_CLK_MIO50_OR_MIO51 = (128U),
	EXT_CLK_MIO0 = (129U),
	EXT_CLK_MIO1 = (130U),
	EXT_CLK_MIO2 = (131U),
	EXT_CLK_MIO3 = (132U),
	EXT_CLK_MIO4 = (133U),
	EXT_CLK_MIO5 = (134U),
	EXT_CLK_MIO6 = (135U),
	EXT_CLK_MIO7 = (136U),
	EXT_CLK_MIO8 = (137U),
	EXT_CLK_MIO9 = (138U),
	EXT_CLK_MIO10 = (139U),
	EXT_CLK_MIO11 = (140U),
	EXT_CLK_MIO12 = (141U),
	EXT_CLK_MIO13 = (142U),
	EXT_CLK_MIO14 = (143U),
	EXT_CLK_MIO15 = (144U),
	EXT_CLK_MIO16 = (145U),
	EXT_CLK_MIO17 = (146U),
	EXT_CLK_MIO18 = (147U),
	EXT_CLK_MIO19 = (148U),
	EXT_CLK_MIO20 = (149U),
	EXT_CLK_MIO21 = (150U),
	EXT_CLK_MIO22 = (151U),
	EXT_CLK_MIO23 = (152U),
	EXT_CLK_MIO24 = (153U),
	EXT_CLK_MIO25 = (154U),
	EXT_CLK_MIO26 = (155U),
	EXT_CLK_MIO27 = (156U),
	EXT_CLK_MIO28 = (157U),
	EXT_CLK_MIO29 = (158U),
	EXT_CLK_MIO30 = (159U),
	EXT_CLK_MIO31 = (160U),
	EXT_CLK_MIO32 = (161U),
	EXT_CLK_MIO33 = (162U),
	EXT_CLK_MIO34 = (163U),
	EXT_CLK_MIO35 = (164U),
	EXT_CLK_MIO36 = (165U),
	EXT_CLK_MIO37 = (166U),
	EXT_CLK_MIO38 = (167U),
	EXT_CLK_MIO39 = (168U),
	EXT_CLK_MIO40 = (169U),
	EXT_CLK_MIO41 = (170U),
	EXT_CLK_MIO42 = (171U),
	EXT_CLK_MIO43 = (172U),
	EXT_CLK_MIO44 = (173U),
	EXT_CLK_MIO45 = (174U),
	EXT_CLK_MIO46 = (175U),
	EXT_CLK_MIO47 = (176U),
	EXT_CLK_MIO48 = (177U),
	EXT_CLK_MIO49 = (178U),
	EXT_CLK_MIO50 = (179U),
	EXT_CLK_MIO51 = (180U),
	EXT_CLK_MIO52 = (181U),
	EXT_CLK_MIO53 = (182U),
	EXT_CLK_MIO54 = (183U),
	EXT_CLK_MIO55 = (184U),
	EXT_CLK_MIO56 = (185U),
	EXT_CLK_MIO57 = (186U),
	EXT_CLK_MIO58 = (187U),
	EXT_CLK_MIO59 = (188U),
	EXT_CLK_MIO60 = (189U),
	EXT_CLK_MIO61 = (190U),
	EXT_CLK_MIO62 = (191U),
	EXT_CLK_MIO63 = (192U),
	EXT_CLK_MIO64 = (193U),
	EXT_CLK_MIO65 = (194U),
	EXT_CLK_MIO66 = (195U),
	EXT_CLK_MIO67 = (196U),
	EXT_CLK_MIO68 = (197U),
	EXT_CLK_MIO69 = (198U),
	EXT_CLK_MIO70 = (199U),
	EXT_CLK_MIO71 = (200U),
	EXT_CLK_MIO72 = (201U),
	EXT_CLK_MIO73 = (202U),
	EXT_CLK_MIO74 = (203U),
	EXT_CLK_MIO75 = (204U),
	EXT_CLK_MIO76 = (205U),
	EXT_CLK_MIO77 = (206U),
	END_OF_CLKS = (207U),
};

#define CLK_MAX END_OF_CLKS

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
uint8_t pm_clock_has_div(uint32_t clock_id, enum pm_clock_div_id div_id);

void pm_api_clock_get_name(uint32_t clock_id, char *name);
enum pm_ret_status pm_api_clock_get_num_clocks(uint32_t *nclocks);
enum pm_ret_status pm_api_clock_get_topology(uint32_t clock_id,
					     uint32_t index,
					     uint32_t *topology);
enum pm_ret_status pm_api_clock_get_fixedfactor_params(uint32_t clock_id,
						       uint32_t *mul,
						       uint32_t *div);
enum pm_ret_status pm_api_clock_get_parents(uint32_t clock_id,
					    uint32_t index,
					    uint32_t *parents);
enum pm_ret_status pm_api_clock_get_attributes(uint32_t clock_id,
					       uint32_t *attr);
enum pm_ret_status pm_api_clock_get_max_divisor(enum clock_id clock_id,
						uint8_t div_type,
						uint32_t *max_div);

enum pm_ret_status pm_clock_get_pll_node_id(enum clock_id clock_id,
					    enum pm_node_id *node_id);
enum pm_ret_status pm_clock_id_is_valid(uint32_t clock_id);

enum pm_ret_status pm_clock_pll_enable(struct pm_pll *pll);
enum pm_ret_status pm_clock_pll_disable(struct pm_pll *pll);
enum pm_ret_status pm_clock_pll_get_state(struct pm_pll *pll,
					  uint32_t *state);
enum pm_ret_status pm_clock_pll_set_parent(struct pm_pll *pll,
					   enum clock_id clock_id,
					   uint32_t parent_index);
enum pm_ret_status pm_clock_pll_get_parent(struct pm_pll *pll,
					   enum clock_id clock_id,
					   uint32_t *parent_index);
enum pm_ret_status pm_clock_set_pll_mode(enum clock_id clock_id,
					 uint32_t mode);
enum pm_ret_status pm_clock_get_pll_mode(enum clock_id clock_id,
					 uint32_t *mode);

#endif /* PM_API_CLOCK_H */
