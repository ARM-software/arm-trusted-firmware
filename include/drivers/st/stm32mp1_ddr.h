/*
 * Copyright (C) 2018-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
 */

#ifndef STM32MP1_DDR_H
#define STM32MP1_DDR_H

#include <stdbool.h>
#include <stdint.h>

#define DT_DDR_COMPAT	"st,stm32mp1-ddr"

struct stm32mp1_ddr_size {
	uint64_t base;
	uint64_t size;
};

/**
 * struct ddr_info
 *
 * @dev: pointer for the device
 * @info: UCLASS RAM information
 * @ctl: DDR controleur base address
 * @phy: DDR PHY base address
 * @syscfg: syscfg base address
 */
struct ddr_info {
	struct stm32mp1_ddr_size info;
	struct stm32mp1_ddrctl *ctl;
	struct stm32mp1_ddrphy *phy;
	uintptr_t pwr;
	uintptr_t rcc;
};

struct stm32mp1_ddrctrl_reg {
	uint32_t mstr;
	uint32_t mrctrl0;
	uint32_t mrctrl1;
	uint32_t derateen;
	uint32_t derateint;
	uint32_t pwrctl;
	uint32_t pwrtmg;
	uint32_t hwlpctl;
	uint32_t rfshctl0;
	uint32_t rfshctl3;
	uint32_t crcparctl0;
	uint32_t zqctl0;
	uint32_t dfitmg0;
	uint32_t dfitmg1;
	uint32_t dfilpcfg0;
	uint32_t dfiupd0;
	uint32_t dfiupd1;
	uint32_t dfiupd2;
	uint32_t dfiphymstr;
	uint32_t odtmap;
	uint32_t dbg0;
	uint32_t dbg1;
	uint32_t dbgcmd;
	uint32_t poisoncfg;
	uint32_t pccfg;
};

struct stm32mp1_ddrctrl_timing {
	uint32_t rfshtmg;
	uint32_t dramtmg0;
	uint32_t dramtmg1;
	uint32_t dramtmg2;
	uint32_t dramtmg3;
	uint32_t dramtmg4;
	uint32_t dramtmg5;
	uint32_t dramtmg6;
	uint32_t dramtmg7;
	uint32_t dramtmg8;
	uint32_t dramtmg14;
	uint32_t odtcfg;
};

struct stm32mp1_ddrctrl_map {
	uint32_t addrmap1;
	uint32_t addrmap2;
	uint32_t addrmap3;
	uint32_t addrmap4;
	uint32_t addrmap5;
	uint32_t addrmap6;
	uint32_t addrmap9;
	uint32_t addrmap10;
	uint32_t addrmap11;
};

struct stm32mp1_ddrctrl_perf {
	uint32_t sched;
	uint32_t sched1;
	uint32_t perfhpr1;
	uint32_t perflpr1;
	uint32_t perfwr1;
	uint32_t pcfgr_0;
	uint32_t pcfgw_0;
	uint32_t pcfgqos0_0;
	uint32_t pcfgqos1_0;
	uint32_t pcfgwqos0_0;
	uint32_t pcfgwqos1_0;
	uint32_t pcfgr_1;
	uint32_t pcfgw_1;
	uint32_t pcfgqos0_1;
	uint32_t pcfgqos1_1;
	uint32_t pcfgwqos0_1;
	uint32_t pcfgwqos1_1;
};

struct stm32mp1_ddrphy_reg {
	uint32_t pgcr;
	uint32_t aciocr;
	uint32_t dxccr;
	uint32_t dsgcr;
	uint32_t dcr;
	uint32_t odtcr;
	uint32_t zq0cr1;
	uint32_t dx0gcr;
	uint32_t dx1gcr;
	uint32_t dx2gcr;
	uint32_t dx3gcr;
};

struct stm32mp1_ddrphy_timing {
	uint32_t ptr0;
	uint32_t ptr1;
	uint32_t ptr2;
	uint32_t dtpr0;
	uint32_t dtpr1;
	uint32_t dtpr2;
	uint32_t mr0;
	uint32_t mr1;
	uint32_t mr2;
	uint32_t mr3;
};

struct stm32mp1_ddrphy_cal {
	uint32_t dx0dllcr;
	uint32_t dx0dqtr;
	uint32_t dx0dqstr;
	uint32_t dx1dllcr;
	uint32_t dx1dqtr;
	uint32_t dx1dqstr;
	uint32_t dx2dllcr;
	uint32_t dx2dqtr;
	uint32_t dx2dqstr;
	uint32_t dx3dllcr;
	uint32_t dx3dqtr;
	uint32_t dx3dqstr;
};

struct stm32mp1_ddr_info {
	const char *name;
	uint32_t speed; /* in kHZ */
	uint32_t size;  /* Memory size in byte = col * row * width */
};

struct stm32mp1_ddr_config {
	struct stm32mp1_ddr_info info;
	struct stm32mp1_ddrctrl_reg c_reg;
	struct stm32mp1_ddrctrl_timing c_timing;
	struct stm32mp1_ddrctrl_map c_map;
	struct stm32mp1_ddrctrl_perf c_perf;
	struct stm32mp1_ddrphy_reg p_reg;
	struct stm32mp1_ddrphy_timing p_timing;
	struct stm32mp1_ddrphy_cal p_cal;
};

int stm32mp1_ddr_clk_enable(struct ddr_info *priv, uint32_t mem_speed);
void stm32mp1_ddr_init(struct ddr_info *priv,
		       struct stm32mp1_ddr_config *config);
#endif /* STM32MP1_DDR_H */
