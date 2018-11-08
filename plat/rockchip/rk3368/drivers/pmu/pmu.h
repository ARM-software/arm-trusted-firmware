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
#define PMU_WKUP_CFG0		0x0
#define PMU_WKUP_CFG1		0x4
#define PMU_WKUP_CFG2		0x8
#define PMU_TIMEOUT_CNT		0x7c
#define PMU_PWRDN_CON		0xc
#define PMU_PWRDN_ST		0x10
#define PMU_CORE_PWR_ST		0x38

#define PMU_PWRMD_CORE		0x14
#define PMU_PWRMD_COM		0x18
#define PMU_SFT_CON			0x1c
#define PMU_BUS_IDE_REQ		0x3c
#define PMU_BUS_IDE_ST		0x40
#define PMU_OSC_CNT			0x48
#define PMU_PLLLOCK_CNT		0x4c
#define PMU_PLLRST_CNT		0x50
#define PMU_STABLE_CNT		0x54
#define PMU_DDRIO_PWR_CNT	0x58
#define PMU_WKUPRST_CNT		0x5c

enum pmu_powermode_core {
	pmu_mdcr_global_int_dis = 0,
	pmu_mdcr_core_src_gt,
	pmu_mdcr_clr_cci,
	pmu_mdcr_cpu0_pd,
	pmu_mdcr_clr_clst_l = 4,
	pmu_mdcr_clr_core,
	pmu_mdcr_scu_l_pd,
	pmu_mdcr_core_pd,
	pmu_mdcr_l2_idle = 8,
	pmu_mdcr_l2_flush
};

/*
 * the shift of bits for cores status
 */
enum pmu_core_pwrst_shift {
	clstl_cpu_wfe = 2,
	clstl_cpu_wfi = 6,
	clstb_cpu_wfe = 12,
	clstb_cpu_wfi = 16
};

enum pmu_pdid {
	PD_CPUL0 = 0,
	PD_CPUL1,
	PD_CPUL2,
	PD_CPUL3,
	PD_SCUL,
	PD_CPUB0 = 5,
	PD_CPUB1,
	PD_CPUB2,
	PD_CPUB3,
	PD_SCUB = 9,
	PD_PERI = 13,
	PD_VIDEO,
	PD_VIO,
	PD_GPU0,
	PD_GPU1,
	PD_END
};

enum pmu_bus_ide {
	bus_ide_req_clst_l = 0,
	bus_ide_req_clst_b,
	bus_ide_req_gpu,
	bus_ide_req_core,
	bus_ide_req_bus = 4,
	bus_ide_req_dma,
	bus_ide_req_peri,
	bus_ide_req_video,
	bus_ide_req_vio = 8,
	bus_ide_req_res0,
	bus_ide_req_cxcs,
	bus_ide_req_alive,
	bus_ide_req_pmu = 12,
	bus_ide_req_msch,
	bus_ide_req_cci,
	bus_ide_req_cci400 = 15,
	bus_ide_req_end
};

enum pmu_powermode_common {
	pmu_mode_en = 0,
	pmu_mode_res0,
	pmu_mode_bus_pd,
	pmu_mode_wkup_rst,
	pmu_mode_pll_pd = 4,
	pmu_mode_pwr_off,
	pmu_mode_pmu_use_if,
	pmu_mode_pmu_alive_use_if,
	pmu_mode_osc_dis = 8,
	pmu_mode_input_clamp,
	pmu_mode_sref_enter,
	pmu_mode_ddrc_gt,
	pmu_mode_ddrio_ret = 12,
	pmu_mode_ddrio_ret_deq,
	pmu_mode_clr_pmu,
	pmu_mode_clr_alive,
	pmu_mode_clr_bus = 16,
	pmu_mode_clr_dma,
	pmu_mode_clr_msch,
	pmu_mode_clr_peri,
	pmu_mode_clr_video = 20,
	pmu_mode_clr_vio,
	pmu_mode_clr_gpu,
	pmu_mode_clr_mcu,
	pmu_mode_clr_cxcs = 24,
	pmu_mode_clr_cci400,
	pmu_mode_res1,
	pmu_mode_res2,
	pmu_mode_res3 = 28,
	pmu_mode_mclst
};

enum pmu_core_power_st {
	clst_l_cpu_wfe = 2,
	clst_l_cpu_wfi = 6,
	clst_b_l2_flsh_done = 10,
	clst_b_l2_wfi = 11,
	clst_b_cpu_wfe = 12,
	clst_b_cpu_wfi = 16,
	mcu_sleeping = 20,
};

enum pmu_sft_con {
	pmu_sft_acinactm_clst_b = 5,
	pmu_sft_l2flsh_clst_b,
	pmu_sft_glbl_int_dis_b = 9,
	pmu_sft_ddrio_ret_cfg = 11,
};

enum pmu_wkup_cfg2 {
	pmu_cluster_l_wkup_en = 0,
	pmu_cluster_b_wkup_en,
	pmu_gpio_wkup_en,
	pmu_sdio_wkup_en,
	pmu_sdmmc_wkup_en,
	pmu_sim_wkup_en,
	pmu_timer_wkup_en,
	pmu_usbdev_wkup_en,
	pmu_sft_wkup_en,
	pmu_wdt_mcu_wkup_en,
	pmu_timeout_wkup_en,
};

enum pmu_bus_idle_st {
	pmu_idle_ack_cluster_l = 0,
	pmu_idle_ack_cluster_b,
	pmu_idle_ack_gpu,
	pmu_idle_ack_core,
	pmu_idle_ack_bus,
	pmu_idle_ack_dma,
	pmu_idle_ack_peri,
	pmu_idle_ack_video,
	pmu_idle_ack_vio,
	pmu_idle_ack_cci = 10,
	pmu_idle_ack_msch,
	pmu_idle_ack_alive,
	pmu_idle_ack_pmu,
	pmu_idle_ack_cxcs,
	pmu_idle_ack_cci400,
	pmu_inactive_cluster_l,
	pmu_inactive_cluster_b,
	pmu_idle_gpu,
	pmu_idle_core,
	pmu_idle_bus,
	pmu_idle_dma,
	pmu_idle_peri,
	pmu_idle_video,
	pmu_idle_vio,
	pmu_idle_cci = 26,
	pmu_idle_msch,
	pmu_idle_alive,
	pmu_idle_pmu,
	pmu_active_cxcs,
	pmu_active_cci,
};

#define PM_PWRDM_CPUSB_MSK (0xf << 5)

#define CKECK_WFE_MSK		0x1
#define CKECK_WFI_MSK		0x10
#define CKECK_WFEI_MSK		0x11

#define PD_CTR_LOOP		500
#define CHK_CPU_LOOP		500

#define MAX_WAIT_CONUT 1000

#endif /* PMU_H */
