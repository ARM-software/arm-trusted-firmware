/*
 * Copyright (c) 2015 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DMU_H
#define DMU_H

/* Clock field should be 2 bits only */
#define CLKCONFIG_MASK 0x3

/* argument */
struct DmuBlockEnable {
	uint32_t sotp:1;
	uint32_t pka_rng:1;
	uint32_t crypto:1;
	uint32_t spl:1;
	uint32_t cdru_vgm:1;
	uint32_t apbs_s0_idm:1;
	uint32_t smau_s0_idm:1;
};

/* prototype */
uint32_t bcm_dmu_block_enable(struct DmuBlockEnable dbe);
uint32_t bcm_dmu_block_disable(struct DmuBlockEnable dbe);
uint32_t bcm_set_ihost_pll_freq(uint32_t cluster_num, int ihost_pll_freq_sel);
uint32_t bcm_get_ihost_pll_freq(uint32_t cluster_num);

#define PLL_FREQ_BYPASS 0x0
#define PLL_FREQ_FULL  0x1
#define PLL_FREQ_HALF  0x2
#define PLL_FREQ_QRTR  0x3

#endif
