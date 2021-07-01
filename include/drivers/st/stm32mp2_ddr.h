/*
 * Copyright (C) 2021-2024, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#ifndef STM32MP2_DDR_H
#define STM32MP2_DDR_H

#include <stdbool.h>

#include <ddrphy_phyinit_struct.h>

#include <drivers/st/stm32mp_ddr.h>

struct stm32mp2_ddrctrl_reg {
	uint32_t mstr;
	uint32_t mrctrl0;
	uint32_t mrctrl1;
	uint32_t mrctrl2;
	uint32_t derateen;
	uint32_t derateint;
	uint32_t deratectl;
	uint32_t pwrctl;
	uint32_t pwrtmg;
	uint32_t hwlpctl;
	uint32_t rfshctl0;
	uint32_t rfshctl1;
	uint32_t rfshctl3;
	uint32_t crcparctl0;
	uint32_t crcparctl1;
	uint32_t init0;
	uint32_t init1;
	uint32_t init2;
	uint32_t init3;
	uint32_t init4;
	uint32_t init5;
	uint32_t init6;
	uint32_t init7;
	uint32_t dimmctl;
	uint32_t rankctl;
	uint32_t rankctl1;
	uint32_t zqctl0;
	uint32_t zqctl1;
	uint32_t zqctl2;
	uint32_t dfitmg0;
	uint32_t dfitmg1;
	uint32_t dfilpcfg0;
	uint32_t dfilpcfg1;
	uint32_t dfiupd0;
	uint32_t dfiupd1;
	uint32_t dfiupd2;
	uint32_t dfimisc;
	uint32_t dfitmg2;
	uint32_t dfitmg3;
	uint32_t dbictl;
	uint32_t dfiphymstr;
	uint32_t dbg0;
	uint32_t dbg1;
	uint32_t dbgcmd;
	uint32_t swctl;
	uint32_t swctlstatic;
	uint32_t poisoncfg;
	uint32_t pccfg;
};

struct stm32mp2_ddrctrl_timing {
	uint32_t rfshtmg;
	uint32_t rfshtmg1;
	uint32_t dramtmg0;
	uint32_t dramtmg1;
	uint32_t dramtmg2;
	uint32_t dramtmg3;
	uint32_t dramtmg4;
	uint32_t dramtmg5;
	uint32_t dramtmg6;
	uint32_t dramtmg7;
	uint32_t dramtmg8;
	uint32_t dramtmg9;
	uint32_t dramtmg10;
	uint32_t dramtmg11;
	uint32_t dramtmg12;
	uint32_t dramtmg13;
	uint32_t dramtmg14;
	uint32_t dramtmg15;
	uint32_t odtcfg;
	uint32_t odtmap;
};

struct stm32mp2_ddrctrl_map {
	uint32_t addrmap0;
	uint32_t addrmap1;
	uint32_t addrmap2;
	uint32_t addrmap3;
	uint32_t addrmap4;
	uint32_t addrmap5;
	uint32_t addrmap6;
	uint32_t addrmap7;
	uint32_t addrmap8;
	uint32_t addrmap9;
	uint32_t addrmap10;
	uint32_t addrmap11;
};

struct stm32mp2_ddrctrl_perf {
	uint32_t sched;
	uint32_t sched1;
	uint32_t perfhpr1;
	uint32_t perflpr1;
	uint32_t perfwr1;
	uint32_t sched3;
	uint32_t sched4;
	uint32_t pcfgr_0;
	uint32_t pcfgw_0;
	uint32_t pctrl_0;
	uint32_t pcfgqos0_0;
	uint32_t pcfgqos1_0;
	uint32_t pcfgwqos0_0;
	uint32_t pcfgwqos1_0;
#if STM32MP_DDR_DUAL_AXI_PORT
	uint32_t pcfgr_1;
	uint32_t pcfgw_1;
	uint32_t pctrl_1;
	uint32_t pcfgqos0_1;
	uint32_t pcfgqos1_1;
	uint32_t pcfgwqos0_1;
	uint32_t pcfgwqos1_1;
#endif /* STM32MP_DDR_DUAL_AXI_PORT */
};

struct stm32mp_ddr_config {
	struct stm32mp_ddr_info info;
	struct stm32mp2_ddrctrl_reg c_reg;
	struct stm32mp2_ddrctrl_timing c_timing;
	struct stm32mp2_ddrctrl_map c_map;
	struct stm32mp2_ddrctrl_perf c_perf;
	bool self_refresh;
	uint32_t zdata;
	struct user_input_basic uib;
	struct user_input_advanced uia;
	struct user_input_mode_register uim;
	struct user_input_swizzle uis;
};

void stm32mp2_ddr_init(struct stm32mp_ddr_priv *priv, struct stm32mp_ddr_config *config);

#endif /* STM32MP2_DDR_H */
