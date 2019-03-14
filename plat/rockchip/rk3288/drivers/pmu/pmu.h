/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PMU_H
#define PMU_H

/* Allocate sp reginon in pmusram */
#define PSRAM_SP_SIZE		0x80
#define PSRAM_SP_BOTTOM		(PSRAM_SP_TOP - PSRAM_SP_SIZE)

/*****************************************************************************
 * pmu con,reg
 *****************************************************************************/
#define PMU_WAKEUP_CFG0		0x0
#define PMU_WAKEUP_CFG1		0x4
#define PMU_PWRDN_CON		0x8
#define PMU_PWRDN_ST		0xc

#define PMU_PWRMODE_CON		0x18
#define PMU_BUS_IDE_REQ		0x10
#define PMU_BUS_IDE_ST		0x14

#define PMU_OSC_CNT		0x20
#define PMU_PLL_CNT		0x24
#define PMU_STABL_CNT		0x28
#define PMU_DDRIO0_PWR_CNT	0x2c
#define PMU_DDRIO1_PWR_CNT	0x30
#define PMU_WKUPRST_CNT		0x44
#define PMU_SFT_CON		0x48
#define PMU_PWRMODE_CON1	0x90

enum pmu_pdid {
	PD_CPU0 = 0,
	PD_CPU1,
	PD_CPU2,
	PD_CPU3,
	PD_BUS = 5,
	PD_PERI,
	PD_VIO,
	PD_VIDEO,
	PD_GPU,
	PD_SCU = 11,
	PD_HEVC = 14,
	PD_END
};

enum pmu_bus_ide {
	bus_ide_req_bus = 0,
	bus_ide_req_peri,
	bus_ide_req_gpu,
	bus_ide_req_video,
	bus_ide_req_vio,
	bus_ide_req_core,
	bus_ide_req_alive,
	bus_ide_req_dma,
	bus_ide_req_cpup,
	bus_ide_req_hevc,
	bus_ide_req_end
};

enum pmu_pwrmode {
	pmu_mode_en = 0,
	pmu_mode_core_src_gt,
	pmu_mode_glb_int_dis,
	pmu_mode_l2_flush_en,
	pmu_mode_bus_pd,
	pmu_mode_cpu0_pd,
	pmu_mode_scu_pd,
	pmu_mode_pll_pd = 7,
	pmu_mode_chip_pd,
	pmu_mode_pwr_off_comb,
	pmu_mode_pmu_alive_use_lf,
	pmu_mode_pmu_use_lf,
	pmu_mode_osc_dis = 12,
	pmu_mode_input_clamp,
	pmu_mode_wkup_rst,
	pmu_mode_sref0_enter,
	pmu_mode_sref1_enter,
	pmu_mode_ddrio0_ret,
	pmu_mode_ddrio1_ret,
	pmu_mode_ddrc0_gt,
	pmu_mode_ddrc1_gt,
	pmu_mode_ddrio0_ret_deq,
	pmu_mode_ddrio1_ret_deq,
};

enum pmu_pwrmode1 {
	pmu_mode_clr_bus = 0,
	pmu_mode_clr_core,
	pmu_mode_clr_cpup,
	pmu_mode_clr_alive,
	pmu_mode_clr_dma,
	pmu_mode_clr_peri,
	pmu_mode_clr_gpu,
	pmu_mode_clr_video,
	pmu_mode_clr_hevc,
	pmu_mode_clr_vio
};

enum pmu_sft_con {
	pmu_sft_ddrio0_ret_cfg = 6,
	pmu_sft_ddrio1_ret_cfg = 9,
	pmu_sft_l2flsh = 15,
};

enum pmu_wakeup_cfg1 {
	pmu_armint_wakeup_en = 0,
	pmu_gpio_wakeup_negedge,
	pmu_sdmmc0_wakeup_en,
	pmu_gpioint_wakeup_en,
};

enum pmu_bus_idle_st {
	pmu_idle_bus = 0,
	pmu_idle_peri,
	pmu_idle_gpu,
	pmu_idle_video,
	pmu_idle_vio,
	pmu_idle_core,
	pmu_idle_alive,
	pmu_idle_dma,
	pmu_idle_cpup,
	pmu_idle_hevc,
	pmu_idle_ack_bus = 16,
	pmu_idle_ack_peri,
	pmu_idle_ack_gpu,
	pmu_idle_ack_video,
	pmu_idle_ack_vio,
	pmu_idle_ack_core,
	pmu_idle_ack_alive,
	pmu_idle_ack_dma,
	pmu_idle_ack_cpup,
	pmu_idle_ack_hevc,
};

#define CHECK_CPU_WFIE_BASE		(0)

#define clstl_cpu_wfe		-1
#define clstb_cpu_wfe		-1
#define CKECK_WFEI_MSK		0


#define PD_CTR_LOOP		500
#define CHK_CPU_LOOP		500

#define MAX_WAIT_CONUT 1000

#endif /* PMU_H */
