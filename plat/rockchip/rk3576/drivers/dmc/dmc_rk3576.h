/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2025, Rockchip Electronics Co., Ltd.
 */

#ifndef __PLAT_ROCKCHIP_DMC_RK3576_H__
#define __PLAT_ROCKCHIP_DMC_RK3576_H__

#define MAX_CH_NUM			(2)
#define CTL_PORT_NUM			(5)

/* DDR_GRF Register */
#define GRF_CH_CON(ch, n)		((((ch) % 2) * 0x100) + ((n) * 4))
#define DDR_GRF_CH_STATUS16(ch)		(0x220 + ((ch) * 0x100))
#define GRF_DDRPHY_CON(n)		(0x530 + ((n) * 4))
#define GRF_DDRPHY_CON0(ch)		(0x530 + (((ch) % 2) * 0x4))
#define DDR_GRF_COMMON_CON(n)		(0x540 + ((n) * 4))

/* PMUGRF Register */
#define PMUGRF_OS_REG(n)		(0x200 + ((n) * 4))

struct low_power_st {
	uint32_t pwrctl;
	uint32_t clkgatectl;
	uint32_t dfi_lp_mode_apb;
	uint32_t grf_ddr_con0;
	uint32_t grf_ddr_con1;
	uint32_t grf_ddr_con6;
	uint32_t grf_ddr_con7;
	uint32_t grf_ddr_con9;
	uint32_t grf_ddrphy_con0;
	uint32_t hwlp_0;
	uint32_t hwlp_c;
	uint32_t pcl_pd;
};

struct rk3576_dmc_config {
	struct low_power_st low_power[MAX_CH_NUM];
};

void dmc_save(void);
void dmc_restore(void);

#endif /* __PLAT_ROCKCHIP_DMC_RK3576_H__ */
