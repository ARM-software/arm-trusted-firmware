/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stddef.h>
#include <device_clk.h>
#include <resource.h>
#include <clk.h>
#include <device.h>
#include <build_assert.h>
#include <soc_device.h>
#include <clk_ids.h>
#include <clocks.h>
#include <devices.h>
#include <regs.h>
#include <control.h>
#include <psc.h>

BUILD_ASSERT_GLOBAL(sizeof(dev_idx_t) == (size_t) 1, dev_idx_t_is_16bit);

#define AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0 0
#define AM62LX_PSC_PD_GP_CORE_CTL 0
#define AM62LX_PSC_PD_PD_CRYPTO 1
#define AM62LX_PSC_PD_PD_DDR 2
#define AM62LX_PSC_PD_PD_MAINIP 3
#define AM62LX_PSC_PD_PD_MPU_CLST0 4
#define AM62LX_PSC_PD_PD_MPU_CLST0_CORE0 5
#define AM62LX_PSC_PD_PD_MPU_CLST0_CORE1 6
#define AM62LX_PSC_PD_PD_MPU_CLST0_CORE2 7
#define AM62LX_PSC_PD_PD_MPU_CLST0_CORE3 8
#define AM62LX_PSC_PD_PD_PER 9
#define AM62LX_PSC_PD_PD_MCUSS0 10
#define AM62LX_PSC_PD_PD_C6DSP 11
#define AM62LX_PSC_PD_PD_ICSS 12
#define AM62LX_PSC_PD_PD_PRUSS 13
#define AM62LX_PSC_PD_PD_ISP 14
#define AM62LX_PSC_PD_PD_DLA 15
#define AM62LX_PSC_PD_PD_ENCODE 16
#define AM62LX_PSC_PD_PD_DECODE 17
#define AM62LX_PSC_PD_PD_GPUCORE 18
#define AM62LX_PSC_PD_PD_GPUCTRL 19
#define AM62LX_PSC_PD_PD_RSVD0 20
#define AM62LX_PSC_PD_PD_RSVD1 21
#define AM62LX_PSC_PD_PD_RSVD2 22
#define AM62LX_PSC_PD_PD_RSVD3 23
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_ALWAYSON 0
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_TEST 1
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_PBIST0 2
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_ISO0_N 3
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_ISO1_N 4
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_TIFS 5
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_DPHY_RX0 6
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB0 7
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB0_ISO_N 8
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB1 9
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB1_ISO_N 10
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_DPHY_TX 11
#define AM62LX_PSC_LPSC_LPSC_GP_RSVD0 12
#define AM62LX_PSC_LPSC_LPSC_GP_RSVD1 13
#define AM62LX_PSC_LPSC_LPSC_GP_RSVD2 14
#define AM62LX_PSC_LPSC_LPSC_GP_RSVD3 15
#define AM62LX_PSC_LPSC_LPSC_GP_RSVD4 16
#define AM62LX_PSC_LPSC_LPSC_GP_RSVD5 17
#define AM62LX_PSC_LPSC_LPSC_MAIN_GP_WKPERI 18
#define AM62LX_PSC_LPSC_LPSC_MAIN_CRYPTO 19
#define AM62LX_PSC_LPSC_LPSC_MAIN_CRYPTO_RSVD 20
#define AM62LX_PSC_LPSC_LPSC_MAIN_DDR_LOCAL 21
#define AM62LX_PSC_LPSC_LPSC_MAIN_DDR_CFG_ISO_N 22
#define AM62LX_PSC_LPSC_LPSC_MAIN_DDR_DATA_ISO_N 23
#define AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON 24
#define AM62LX_PSC_LPSC_LPSC_MAINIP_DSS 25
#define AM62LX_PSC_LPSC_LPSC_MAINIP_DSI 26
#define AM62LX_PSC_LPSC_LPSC_MAINIP_EMMC8B 27
#define AM62LX_PSC_LPSC_LPSC_MAINIP_EMMC4B0 28
#define AM62LX_PSC_LPSC_LPSC_MAINIP_EMMC4B1 29
#define AM62LX_PSC_LPSC_LPSC_MAINIP_CPSW 30
#define AM62LX_PSC_LPSC_LPSC_MAINIP_CSI_RX0 31
#define AM62LX_PSC_LPSC_LPSC_MAINIP_GIC 32
#define AM62LX_PSC_LPSC_LPSC_MAINIP_PBIST 33
#define AM62LX_PSC_LPSC_LPSC_MAINIP_RSVD0 34
#define AM62LX_PSC_LPSC_LPSC_MAINIP_RSVD1 35
#define AM62LX_PSC_LPSC_LPSC_MAINIP_RSVD2 36
#define AM62LX_PSC_LPSC_LPSC_MAINIP_RSVD3 37
#define AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0 38
#define AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_PBIST 39
#define AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE0 40
#define AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE1 41
#define AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE2 42
#define AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE3 43
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON 44
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCASP0 45
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCASP1 46
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCASP2 47
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_XSPI 48
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCAN0 49
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCAN1 50
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCAN2 51
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_GPMC 52
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_ADC 53
#define AM62LX_PSC_LPSC_LPSC_MAIN_PER_RSVD0 54
#define AM62LX_PSC_LPSC_LPSC_MAIN_DEBUGSS 55
#define AM62LX_PSC_LPSC_LPSC_MAIN_MCUSS0_CORE0 56
#define AM62LX_PSC_LPSC_LPSC_MAIN_MCUSS0_PBIST 57
#define AM62LX_PSC_LPSC_LPSC_MAIN_C6DSP_CORE 58
#define AM62LX_PSC_LPSC_LPSC_MAIN_C6DSP_PBIST 59
#define AM62LX_PSC_LPSC_LPSC_MAIN_ICSS 60
#define AM62LX_PSC_LPSC_LPSC_MAIN_ICSS_RSVD 61
#define AM62LX_PSC_LPSC_LPSC_MAIN_PRUSS 62
#define AM62LX_PSC_LPSC_LPSC_MAIN_PRUSS_RSVD 63
#define AM62LX_PSC_LPSC_LPSC_MAIN_ISP 64
#define AM62LX_PSC_LPSC_LPSC_MAIN_ISP_PBIST 65
#define AM62LX_PSC_LPSC_LPSC_MAIN_DLA_COMMON 66
#define AM62LX_PSC_LPSC_LPSC_MAIN_DLA_PBIST 67
#define AM62LX_PSC_LPSC_LPSC_MAIN_DLA_CORE 68
#define AM62LX_PSC_LPSC_LPSC_MAIN_ENCODE 69
#define AM62LX_PSC_LPSC_LPSC_MAIN_ENCODE_PBIST 70
#define AM62LX_PSC_LPSC_LPSC_MAIN_DECODE 71
#define AM62LX_PSC_LPSC_LPSC_MAIN_DECODE_PBIST 72
#define AM62LX_PSC_LPSC_LPSC_MAIN_GPUCORE 73
#define AM62LX_PSC_LPSC_LPSC_MAIN_GPUCTRL_COMMON 74
#define AM62LX_PSC_LPSC_LPSC_MAIN_GPUCTRL_PBIST 75
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD0_RSVD0 76
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD0_RSVD1 77
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD0_RSVD2 78
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD1_RSVD0 79
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD1_RSVD1 80
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD1_RSVD2 81
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD2_RSVD0 82
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD2_RSVD1 83
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD2_RSVD2 84
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD3_RSVD0 85
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD3_RSVD1 86
#define AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD3_RSVD2 87

#define AM62LX_DEV_ADC12_CORE_MAIN_0_CLOCKS 0
#define AM62LX_DEV_AM62L_MAIN_GPIOMUX_INTROUTER_MAIN_0_CLOCKS 7
#define AM62LX_DEV_AM62L_TIMESYNC_INTROUTER_MAIN_0_CLOCKS 8
#define AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS 9
#define AM62LX_DEV_CPT2_AGGREGATOR32_MAIN_SYSCLK2_CLOCKS 32
#define AM62LX_DEV_CPT2_AGGREGATOR32_PER_SYSCLK2_CLOCKS 33
#define AM62LX_DEV_CPT2_AGGREGATOR32_WKUP_SYSCLK2_CLOCKS 34
#define AM62LX_DEV_CXSTM500SS_MAIN_0_CLOCKS 35
#define AM62LX_DEV_DEBUGSS_K3_WRAP_CV0_MAIN_0_CLOCKS 38
#define AM62LX_DEV_DMSS_AM61_MAIN_0_BCDMA_0_CLOCKS 61
#define AM62LX_DEV_DMSS_AM61_MAIN_0_PKTDMA_0_CLOCKS 62
#define AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS 63
#define AM62LX_DEV_DMTIMER_DMC1MS_MAIN_1_CLOCKS 78
#define AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS 83
#define AM62LX_DEV_DMTIMER_DMC1MS_MAIN_3_CLOCKS 98
#define AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS 103
#define AM62LX_DEV_DMTIMER_DMC1MS_WKUP_1_CLOCKS 114
#define AM62LX_DEV_ECAP_MAIN_0_CLOCKS 119
#define AM62LX_DEV_ECAP_MAIN_1_CLOCKS 120
#define AM62LX_DEV_ECAP_MAIN_2_CLOCKS 121
#define AM62LX_DEV_ELM_MAIN_0_CLOCKS 122
#define AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS 123
#define AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS 132
#define AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS 141
#define AM62LX_DEV_EQEP_T2_MAIN_0_CLOCKS 150
#define AM62LX_DEV_EQEP_T2_MAIN_1_CLOCKS 151
#define AM62LX_DEV_EQEP_T2_MAIN_2_CLOCKS 152
#define AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS 153
#define AM62LX_DEV_GIC500SS_1_2_SPI960_MAIN_0_CLOCKS 162
#define AM62LX_DEV_GPIO_144_MAIN_0_CLOCKS 163
#define AM62LX_DEV_GPIO_144_MAIN_2_CLOCKS 164
#define AM62LX_DEV_GPIO_144_WKUP_0_CLOCKS 165
#define AM62LX_DEV_GPMC_MAIN_0_CLOCKS 170
#define AM62LX_DEV_K3_DSS_DSI_MAIN_0_CLOCKS 176
#define AM62LX_DEV_K3_DSS_NANO_MAIN_0_CLOCKS 182
#define AM62LX_DEV_K3_EPWM_MAIN_0_CLOCKS 187
#define AM62LX_DEV_K3_EPWM_MAIN_1_CLOCKS 188
#define AM62LX_DEV_K3_EPWM_MAIN_2_CLOCKS 189
#define AM62LX_DEV_K3_LED2VBUS_MAIN_0_CLOCKS 190
#define AM62LX_DEV_K3_PBIST_8C28P_4BIT_WRAP_MAIN_0_CLOCKS 192
#define AM62LX_DEV_K3_PBIST_8C28P_4BIT_WRAP_WKUP_0_CLOCKS 202
#define AM62LX_DEV_K3VTM_N16FFC_WKUP_0_CLOCKS 210
#define AM62LX_DEV_MCANSS_MAIN_0_CLOCKS 213
#define AM62LX_DEV_MCANSS_MAIN_1_CLOCKS 220
#define AM62LX_DEV_MCANSS_MAIN_2_CLOCKS 227
#define AM62LX_DEV_MCASP_MAIN_0_CLOCKS 234
#define AM62LX_DEV_MCASP_MAIN_1_CLOCKS 258
#define AM62LX_DEV_MCASP_MAIN_2_CLOCKS 272
#define AM62LX_DEV_MSHSI2C_MAIN_0_CLOCKS 286
#define AM62LX_DEV_MSHSI2C_MAIN_1_CLOCKS 290
#define AM62LX_DEV_MSHSI2C_MAIN_2_CLOCKS 294
#define AM62LX_DEV_MSHSI2C_MAIN_3_CLOCKS 298
#define AM62LX_DEV_MSHSI2C_WKUP_0_CLOCKS 302
#define AM62LX_DEV_GTC_R10_WKUP_0_CLOCKS 306
#define AM62LX_DEV_RTCSS_WKUP_0_CLOCKS 310
#define AM62LX_DEV_RTI_CFG1_MAIN_A53_0_CLOCKS 316
#define AM62LX_DEV_RTI_CFG1_MAIN_A53_1_CLOCKS 322
#define AM62LX_DEV_SAM61_DEBUG_MAIN_CELL_MAIN_0_CLOCKS 328
#define AM62LX_DEV_SAM61_MSRAM6KX128_MAIN_0_CLOCKS 331
#define AM62LX_DEV_SAM61_PSRAM16KX32_WKUP_0_CLOCKS 332
#define AM62LX_DEV_SAM61_PSROM64KX32_MAIN_0_CLOCKS 333
#define AM62LX_DEV_SAM61_WKUP_PSC_WRAP_WKUP_0_CLOCKS 334
#define AM62LX_DEV_SAM62_DM_WAKEUP_DEEPSLEEP_SOURCES_WKUP_0_CLOCKS 336
#define AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_CLOCKS 337
#define AM62LX_DEV_SAM62L_DDR_WRAP_MAIN_0_CLOCKS 342
#define AM62LX_DEV_SAM62L_DFTSS_WRAP_WKUP_0_CLOCKS 345
#define AM62LX_DEV_SPI_MAIN_0_CLOCKS 348
#define AM62LX_DEV_SPI_MAIN_1_CLOCKS 354
#define AM62LX_DEV_SPI_MAIN_2_CLOCKS 360
#define AM62LX_DEV_SPI_MAIN_3_CLOCKS 366
#define AM62LX_DEV_TRNG_DRBG_EIP76D_WRAP_MAIN_0_CLOCKS 372
#define AM62LX_DEV_USART_MAIN_1_CLOCKS 373
#define AM62LX_DEV_USART_MAIN_2_CLOCKS 377
#define AM62LX_DEV_USART_MAIN_3_CLOCKS 381
#define AM62LX_DEV_USART_MAIN_4_CLOCKS 385
#define AM62LX_DEV_USART_MAIN_5_CLOCKS 389
#define AM62LX_DEV_USART_MAIN_6_CLOCKS 393
#define AM62LX_DEV_USART_WKUP_0_CLOCKS 397
#define AM62LX_DEV_USB2SS_16FFC_MAIN_0_CLOCKS 401
#define AM62LX_DEV_USB2SS_16FFC_MAIN_1_CLOCKS 412
#define AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS 423
#define AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_CLOCKS 440
#define AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_PBIST_0_CLOCKS 443
#define AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_A53_0_CLOCKS 446
#define AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_A53_1_CLOCKS 447
#define AM62LX_DEV_USART_MAIN_0_CLOCKS 448
#define AM62LX_DEV_BOARD_0_CLOCKS 452
#define AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLOCKS 566
#define AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS 573
#define AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS 584
#define AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS 592
#define AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLOCKS 604

static const struct dev_data
am62lx_dev_adc12_core_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_ADC,
	},
	.dev_clk_idx = AM62LX_DEV_ADC12_CORE_MAIN_0_CLOCKS,
	.n_clocks = 7,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_am62l_main_gpiomux_introuter_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_AM62L_MAIN_GPIOMUX_INTROUTER_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_am62l_timesync_introuter_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_AM62L_TIMESYNC_INTROUTER_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_cpsw_3guss_am62l_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_CPSW,
	},
	.dev_clk_idx = AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
	.n_clocks = 23,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_cpt2_aggregator32_main_sysclk2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_CPT2_AGGREGATOR32_MAIN_SYSCLK2_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_cpt2_aggregator32_per_sysclk2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_CPT2_AGGREGATOR32_PER_SYSCLK2_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_cpt2_aggregator32_wkup_sysclk2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_ALWAYSON,
	},
	.dev_clk_idx = AM62LX_DEV_CPT2_AGGREGATOR32_WKUP_SYSCLK2_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_cxstm500ss_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_DEBUGSS,
	},
	.dev_clk_idx = AM62LX_DEV_CXSTM500SS_MAIN_0_CLOCKS,
	.n_clocks = 3,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_debugss_k3_wrap_cv0_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_DEBUGSS,
	},
	.dev_clk_idx = AM62LX_DEV_DEBUGSS_K3_WRAP_CV0_MAIN_0_CLOCKS,
	.n_clocks = 23,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmss_am61_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmss_am61_main_0_bcdma_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_DMSS_AM61_MAIN_0_BCDMA_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmss_am61_main_0_pktdma_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_DMSS_AM61_MAIN_0_PKTDMA_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmss_crypto_am61_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_CRYPTO,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_CRYPTO,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmss_crypto_am61_wkup_0_dthe __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmss_crypto_am61_wkup_0_xlcdma_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmtimer_dmc1ms_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
	.n_clocks = 15,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmtimer_dmc1ms_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_DMTIMER_DMC1MS_MAIN_1_CLOCKS,
	.n_clocks = 5,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmtimer_dmc1ms_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
	.n_clocks = 15,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmtimer_dmc1ms_main_3 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_DMTIMER_DMC1MS_MAIN_3_CLOCKS,
	.n_clocks = 5,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmtimer_dmc1ms_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_WKPERI,
	},
	.dev_clk_idx = AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
	.n_clocks = 11,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_dmtimer_dmc1ms_wkup_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_WKPERI,
	},
	.dev_clk_idx = AM62LX_DEV_DMTIMER_DMC1MS_WKUP_1_CLOCKS,
	.n_clocks = 5,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sms_lite_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_TIFS,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_ecap_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_ECAP_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_ecap_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_ECAP_MAIN_1_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_ecap_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_ECAP_MAIN_2_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_elm_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_GPMC,
	},
	.dev_clk_idx = AM62LX_DEV_ELM_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_emmcsd4ss_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_EMMC4B0,
	},
	.dev_clk_idx = AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS,
	.n_clocks = 9,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_emmcsd4ss_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_EMMC4B1,
	},
	.dev_clk_idx = AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS,
	.n_clocks = 9,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_emmcsd8ss_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_EMMC8B,
	},
	.dev_clk_idx = AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS,
	.n_clocks = 9,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_eqep_t2_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_EQEP_T2_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_eqep_t2_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_EQEP_T2_MAIN_1_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_eqep_t2_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_EQEP_T2_MAIN_2_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_fss_ul_128_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_XSPI,
	},
	.dev_clk_idx = AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
	.n_clocks = 9,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_gic500ss_1_2_spi960_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_GIC,
	},
	.dev_clk_idx = AM62LX_DEV_GIC500SS_1_2_SPI960_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_gpio_144_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_GPIO_144_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_gpio_144_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_GPIO_144_MAIN_2_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_gpio_144_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_ALWAYSON,
	},
	.dev_clk_idx = AM62LX_DEV_GPIO_144_WKUP_0_CLOCKS,
	.n_clocks = 5,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_gpmc_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_GPMC,
	},
	.dev_clk_idx = AM62LX_DEV_GPMC_MAIN_0_CLOCKS,
	.n_clocks = 6,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_k3_dss_dsi_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_DSI,
	},
	.dev_clk_idx = AM62LX_DEV_K3_DSS_DSI_MAIN_0_CLOCKS,
	.n_clocks = 6,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_k3_dss_nano_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_DSS,
	},
	.dev_clk_idx = AM62LX_DEV_K3_DSS_NANO_MAIN_0_CLOCKS,
	.n_clocks = 5,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_k3_epwm_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_K3_EPWM_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_k3_epwm_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_K3_EPWM_MAIN_1_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_k3_epwm_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_K3_EPWM_MAIN_2_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_k3_led2vbus_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.dev_clk_idx = AM62LX_DEV_K3_LED2VBUS_MAIN_0_CLOCKS,
	.n_clocks = 2,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_k3_pbist_8c28p_4bit_wrap_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_PBIST,
	},
	.dev_clk_idx = AM62LX_DEV_K3_PBIST_8C28P_4BIT_WRAP_MAIN_0_CLOCKS,
	.n_clocks = 10,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_k3_pbist_8c28p_4bit_wrap_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_PBIST0,
	},
	.dev_clk_idx = AM62LX_DEV_K3_PBIST_8C28P_4BIT_WRAP_WKUP_0_CLOCKS,
	.n_clocks = 8,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_k3vtm_n16ffc_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_ALWAYSON,
	},
	.dev_clk_idx = AM62LX_DEV_K3VTM_N16FFC_WKUP_0_CLOCKS,
	.n_clocks = 3,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mcanss_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCAN0,
	},
	.dev_clk_idx = AM62LX_DEV_MCANSS_MAIN_0_CLOCKS,
	.n_clocks = 7,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mcanss_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCAN1,
	},
	.dev_clk_idx = AM62LX_DEV_MCANSS_MAIN_1_CLOCKS,
	.n_clocks = 7,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mcanss_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCAN2,
	},
	.dev_clk_idx = AM62LX_DEV_MCANSS_MAIN_2_CLOCKS,
	.n_clocks = 7,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mcasp_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCASP0,
	},
	.dev_clk_idx = AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
	.n_clocks = 24,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mcasp_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCASP1,
	},
	.dev_clk_idx = AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
	.n_clocks = 14,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mcasp_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCASP2,
	},
	.dev_clk_idx = AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
	.n_clocks = 14,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mshsi2c_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_MSHSI2C_MAIN_0_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mshsi2c_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_MSHSI2C_MAIN_1_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mshsi2c_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_MSHSI2C_MAIN_2_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mshsi2c_main_3 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_MSHSI2C_MAIN_3_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_mshsi2c_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_WKPERI,
	},
	.dev_clk_idx = AM62LX_DEV_MSHSI2C_WKUP_0_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_gtc_r10_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_ALWAYSON,
	},
	.dev_clk_idx = AM62LX_DEV_GTC_R10_WKUP_0_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_rtcss_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_ALWAYSON,
	},
	.dev_clk_idx = AM62LX_DEV_RTCSS_WKUP_0_CLOCKS,
	.n_clocks = 6,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_rti_cfg1_main_a53_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MPU_CLST0_CORE0,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE0,
	},
	.dev_clk_idx = AM62LX_DEV_RTI_CFG1_MAIN_A53_0_CLOCKS,
	.n_clocks = 6,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_rti_cfg1_main_a53_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MPU_CLST0_CORE1,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE1,
	},
	.dev_clk_idx = AM62LX_DEV_RTI_CFG1_MAIN_A53_1_CLOCKS,
	.n_clocks = 6,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam61_debug_main_cell_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_DEBUGSS,
	},
	.dev_clk_idx = AM62LX_DEV_SAM61_DEBUG_MAIN_CELL_MAIN_0_CLOCKS,
	.n_clocks = 3,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam61_msram6kx128_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_SAM61_MSRAM6KX128_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam61_psram16kx32_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_ALWAYSON,
	},
	.dev_clk_idx = AM62LX_DEV_SAM61_PSRAM16KX32_WKUP_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam61_psrom64kx32_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_SAM61_PSROM64KX32_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static struct psc_data am62lx_sam61_wkup_psc_wrap_wkup_0_data __section(".bss.devgroup.MAIN");
static const struct psc_pd_data
am62lx_sam61_wkup_psc_wrap_wkup_0_pd_data[AM62LX_PSC_PD_PD_RSVD3 + 1]
__section(".const.devgroup.MAIN") = {
	[AM62LX_PSC_PD_GP_CORE_CTL] = {
		.depends = AM62LX_PSC_PD_PD_MAINIP,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_CRYPTO] = {
		.flags = PSC_PD_EXISTS,
	},
	[AM62LX_PSC_PD_PD_DDR] = {
		.depends = AM62LX_PSC_PD_PD_MAINIP,
		.flags = PSC_PD_EXISTS|PSC_PD_ALWAYSON|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_MAINIP] = {
		.depends = AM62LX_PSC_PD_GP_CORE_CTL,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_MPU_CLST0] = {
		.depends = AM62LX_PSC_PD_PD_MAINIP,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_MPU_CLST0_CORE0] = {
		.depends = AM62LX_PSC_PD_PD_MPU_CLST0,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_MPU_CLST0_CORE1] = {
		.depends = AM62LX_PSC_PD_PD_MPU_CLST0,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_MPU_CLST0_CORE2] = {
		.depends = AM62LX_PSC_PD_PD_MPU_CLST0,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_MPU_CLST0_CORE3] = {
		.depends = AM62LX_PSC_PD_PD_MPU_CLST0,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_PER] = {
		.depends = AM62LX_PSC_PD_PD_MAINIP,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_MCUSS0] = {
		.depends = AM62LX_PSC_PD_PD_PER,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_C6DSP] = {
		.depends = AM62LX_PSC_PD_PD_PER,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_ICSS] = {
		.flags = PSC_PD_EXISTS,
	},
	[AM62LX_PSC_PD_PD_PRUSS] = {
		.flags = PSC_PD_EXISTS,
	},
	[AM62LX_PSC_PD_PD_ISP] = {
		.depends = AM62LX_PSC_PD_PD_MAINIP,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_DLA] = {
		.depends = AM62LX_PSC_PD_PD_MAINIP,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_ENCODE] = {
		.depends = AM62LX_PSC_PD_PD_MAINIP,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_DECODE] = {
		.depends = AM62LX_PSC_PD_PD_MAINIP,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_GPUCORE] = {
		.depends = AM62LX_PSC_PD_PD_GPUCTRL,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_GPUCTRL] = {
		.depends = AM62LX_PSC_PD_PD_MAINIP,
		.flags = PSC_PD_EXISTS|PSC_PD_DEPENDS,
	},
	[AM62LX_PSC_PD_PD_RSVD0] = {
		.flags = PSC_PD_EXISTS,
	},
	[AM62LX_PSC_PD_PD_RSVD1] = {
		.flags = PSC_PD_EXISTS,
	},
	[AM62LX_PSC_PD_PD_RSVD2] = {
		.flags = PSC_PD_EXISTS,
	},
	[AM62LX_PSC_PD_PD_RSVD3] = {
		.flags = PSC_PD_EXISTS,
	},
};
static struct psc_pd
am62lx_sam61_wkup_psc_wrap_wkup_0_powerdomains[AM62LX_PSC_PD_PD_RSVD3 + 1]
__section(".bss.devgroup.MAIN");
static const dev_idx_t dev_list_LPSC_main_gp_alwayson[7] __section(".const.devgroup.MAIN") = {
	AM62LX_DEV_WKUP_CPT2_AGGR0,
	AM62LX_DEV_WKUP_GPIO0,
	AM62LX_DEV_WKUP_GTC0,
	AM62LX_DEV_WKUP_VTM0,
	AM62LX_DEV_WKUP_RTCSS0,
	AM62LX_DEV_WKUP_PSRAM_64K0,
	DEV_ID_NONE,
};
static const dev_idx_t dev_list_LPSC_main_per_common[36] __section(".const.devgroup.MAIN") = {
	AM62LX_DEV_MAIN_GPIOMUX_INTROUTER0,
	AM62LX_DEV_TIMESYNC_INTROUTER0,
	AM62LX_DEV_CPT2_AGGR1,
	AM62LX_DEV_DMASS0_BCDMA_0,
	AM62LX_DEV_DMASS0_PKTDMA_0,
	AM62LX_DEV_TIMER0,
	AM62LX_DEV_TIMER1,
	AM62LX_DEV_TIMER2,
	AM62LX_DEV_TIMER3,
	AM62LX_DEV_ECAP0,
	AM62LX_DEV_ECAP1,
	AM62LX_DEV_ECAP2,
	AM62LX_DEV_EQEP0,
	AM62LX_DEV_EQEP1,
	AM62LX_DEV_EQEP2,
	AM62LX_DEV_GPIO0,
	AM62LX_DEV_GPIO2,
	AM62LX_DEV_EPWM0,
	AM62LX_DEV_EPWM1,
	AM62LX_DEV_EPWM2,
	AM62LX_DEV_I2C0,
	AM62LX_DEV_I2C1,
	AM62LX_DEV_I2C2,
	AM62LX_DEV_I2C3,
	AM62LX_DEV_MCSPI0,
	AM62LX_DEV_MCSPI1,
	AM62LX_DEV_MCSPI2,
	AM62LX_DEV_MCSPI3,
	AM62LX_DEV_UART0,
	AM62LX_DEV_UART1,
	AM62LX_DEV_UART2,
	AM62LX_DEV_UART3,
	AM62LX_DEV_UART4,
	AM62LX_DEV_UART5,
	AM62LX_DEV_UART6,
	DEV_ID_NONE,
};
static const struct lpsc_module_data
am62lx_sam61_wkup_psc_wrap_wkup_0_mod_data[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD3_RSVD2 + 1]
__section(".const.devgroup.MAIN") = {
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_ALWAYSON] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_list = dev_list_LPSC_main_gp_alwayson,
		.flags = LPSC_MODULE_EXISTS|LPSC_NO_CLOCK_GATING|LPSC_NO_MODULE_RESET|LPSC_DEVICES_LIST,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_TEST] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_WKUP_DFTSS0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_PBIST0] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_WKUP_PBIST0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_ISO0_N] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS|LPSC_NO_CLOCK_GATING|LPSC_NO_MODULE_RESET,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_ISO1_N] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_NO_CLOCK_GATING|LPSC_NO_MODULE_RESET,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_TIFS] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_WKUP_SMS_LITE0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_DPHY_RX0] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB0] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_USB0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB0_ISO_N] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB0,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MAIN_USB0_ISO_VD,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS|LPSC_NO_CLOCK_GATING|LPSC_NO_MODULE_RESET,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB1] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_USB1,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB1_ISO_N] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB1,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MAIN_USB1_ISO_VD,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS|LPSC_NO_CLOCK_GATING|LPSC_NO_MODULE_RESET,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_DPHY_TX] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_DPHY_TX0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_GP_RSVD0] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_GP_RSVD1] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_GP_RSVD2] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_GP_RSVD3] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_GP_RSVD4] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_GP_RSVD5] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GP_WKPERI] = {
		.powerdomain = AM62LX_PSC_PD_GP_CORE_CTL,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_WKUP_TIMER0,
			AM62LX_DEV_WKUP_TIMER1,
			AM62LX_DEV_WKUP_I2C0,
			AM62LX_DEV_WKUP_UART0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_CRYPTO] = {
		.powerdomain = AM62LX_PSC_PD_PD_CRYPTO,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_WKUP_DMASS0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_CRYPTO_RSVD] = {
		.powerdomain = AM62LX_PSC_PD_PD_CRYPTO,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_DDR_LOCAL] = {
		.powerdomain = AM62LX_PSC_PD_PD_DDR,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_DDR16SS0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_DDR_CFG_ISO_N] = {
		.powerdomain = AM62LX_PSC_PD_PD_DDR,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_DDR_LOCAL,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_EMIF_CFG_ISO_VD,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS|LPSC_NO_CLOCK_GATING|LPSC_NO_MODULE_RESET,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_DDR_DATA_ISO_N] = {
		.powerdomain = AM62LX_PSC_PD_PD_DDR,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_DDR_CFG_ISO_N,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_EMIF_DATA_ISO_VD,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS|LPSC_NO_CLOCK_GATING|LPSC_NO_MODULE_RESET,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_GP_ISO0_N,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_CPT2_AGGR0,
			AM62LX_DEV_MSRAM_96K0,
			AM62LX_DEV_ROM0,
			AM62LX_DEV_TRNG_DRBG_EIP76D_WRAP0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_DSS] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_DSS0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_DSI] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_DSS,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_DSS_DSI0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_EMMC8B] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MMCSD0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_EMMC4B0] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MMCSD1,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_EMMC4B1] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MMCSD2,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_CPSW] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_CPSW0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_CSI_RX0] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_GIC] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_GICSS0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_PBIST] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_PBIST0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_RSVD0] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_RSVD1] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_RSVD2] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAINIP_RSVD3] = {
		.powerdomain = AM62LX_PSC_PD_PD_MAINIP,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0] = {
		.powerdomain = AM62LX_PSC_PD_PD_MPU_CLST0,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_COMPUTE_CLUSTER0_ARM_COREPACK_0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_PBIST] = {
		.powerdomain = AM62LX_PSC_PD_PD_MPU_CLST0,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_COMPUTE_CLUSTER0_PBIST_0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE0] = {
		.powerdomain = AM62LX_PSC_PD_PD_MPU_CLST0_CORE0,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_RTI0,
			AM62LX_DEV_COMPUTE_CLUSTER0_A53_0,
			DEV_ID_NONE,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS|LPSC_HAS_LOCAL_RESET,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE1] = {
		.powerdomain = AM62LX_PSC_PD_PD_MPU_CLST0_CORE1,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_RTI1,
			AM62LX_DEV_COMPUTE_CLUSTER0_A53_1,
			DEV_ID_NONE,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS|LPSC_HAS_LOCAL_RESET,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE2] = {
		.powerdomain = AM62LX_PSC_PD_PD_MPU_CLST0_CORE2,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE3] = {
		.powerdomain = AM62LX_PSC_PD_PD_MPU_CLST0_CORE3,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_GP_ISO1_N,
		.lpsc_dev.dev_list = dev_list_LPSC_main_per_common,
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS|LPSC_DEVICES_LIST,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCASP0] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MCASP0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCASP1] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MCASP1,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCASP2] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MCASP2,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_XSPI] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_FSS0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCAN0] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MCAN0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCAN1] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MCAN1,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_MCAN2] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_MCAN2,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_GPMC] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_ELM0,
			AM62LX_DEV_GPMC0,
			DEV_ID_NONE,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_ADC] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_ADC0,
			DEV_ID_NONE,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PER_RSVD0] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_DEBUGSS] = {
		.powerdomain = AM62LX_PSC_PD_PD_PER,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			AM62LX_DEV_STM0,
			AM62LX_DEV_DEBUGSS_WRAP0,
			AM62LX_DEV_DEBUGSS0,
			DEV_ID_NONE,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_MCUSS0_CORE0] = {
		.powerdomain = AM62LX_PSC_PD_PD_MCUSS0,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_MCUSS0_PBIST] = {
		.powerdomain = AM62LX_PSC_PD_PD_MCUSS0,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_C6DSP_CORE] = {
		.powerdomain = AM62LX_PSC_PD_PD_C6DSP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_C6DSP_PBIST] = {
		.powerdomain = AM62LX_PSC_PD_PD_C6DSP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_ICSS] = {
		.powerdomain = AM62LX_PSC_PD_PD_ICSS,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_ICSS_RSVD] = {
		.powerdomain = AM62LX_PSC_PD_PD_ICSS,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PRUSS] = {
		.powerdomain = AM62LX_PSC_PD_PD_PRUSS,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PRUSS_RSVD] = {
		.powerdomain = AM62LX_PSC_PD_PD_PRUSS,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_ISP] = {
		.powerdomain = AM62LX_PSC_PD_PD_ISP,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_ISP_PBIST] = {
		.powerdomain = AM62LX_PSC_PD_PD_ISP,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_DLA_COMMON] = {
		.powerdomain = AM62LX_PSC_PD_PD_DLA,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_DLA_PBIST] = {
		.powerdomain = AM62LX_PSC_PD_PD_DLA,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_DLA_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_DLA_CORE] = {
		.powerdomain = AM62LX_PSC_PD_PD_DLA,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_DLA_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_ENCODE] = {
		.powerdomain = AM62LX_PSC_PD_PD_ENCODE,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_ENCODE_PBIST] = {
		.powerdomain = AM62LX_PSC_PD_PD_ENCODE,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_DECODE] = {
		.powerdomain = AM62LX_PSC_PD_PD_DECODE,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_DECODE_PBIST] = {
		.powerdomain = AM62LX_PSC_PD_PD_DECODE,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GPUCORE] = {
		.powerdomain = AM62LX_PSC_PD_PD_GPUCORE,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_GPUCTRL_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GPUCTRL_COMMON] = {
		.powerdomain = AM62LX_PSC_PD_PD_GPUCTRL,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_GPUCTRL_PBIST] = {
		.powerdomain = AM62LX_PSC_PD_PD_GPUCTRL,
		.depends_psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.depends = AM62LX_PSC_LPSC_LPSC_MAIN_GPUCTRL_COMMON,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS|LPSC_DEPENDS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD0_RSVD0] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD0,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD0_RSVD1] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD0,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD0_RSVD2] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD0,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD1_RSVD0] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD1,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD1_RSVD1] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD1,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD1_RSVD2] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD1,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD2_RSVD0] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD2,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD2_RSVD1] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD2,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD2_RSVD2] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD2,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD3_RSVD0] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD3,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD3_RSVD1] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD3,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
	[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD3_RSVD2] = {
		.powerdomain = AM62LX_PSC_PD_PD_RSVD3,
		.lpsc_dev.dev_array = {
			DEV_ID_NONE,
			0,
			0,
			0,
		},
		.flags = LPSC_MODULE_EXISTS,
	},
};
static struct lpsc_module
am62lx_sam61_wkup_psc_wrap_wkup_0_modules[AM62LX_PSC_LPSC_LPSC_MAIN_PDRSVD3_RSVD2 + 1]
__section(".bss.devgroup.MAIN");
static const uint8_t am62lx_dev_sam61_wkup_psc_wrap_wkup_0_resources[]
__section(".const.devgroup.MAIN") = {
	RDAT_HDR(RESOURCE_MEM, 1, true),
	RDAT_MEM(0x00400000),
};
static const struct psc_drv_data am62lx_dev_sam61_wkup_psc_wrap_wkup_0
__section(".const.devgroup.MAIN") = {
	.data = &am62lx_sam61_wkup_psc_wrap_wkup_0_data,
	.pd_data = am62lx_sam61_wkup_psc_wrap_wkup_0_pd_data,
	.powerdomains = am62lx_sam61_wkup_psc_wrap_wkup_0_powerdomains,
	.pd_count = ARRAY_SIZE(am62lx_sam61_wkup_psc_wrap_wkup_0_pd_data),
	.mod_data = am62lx_sam61_wkup_psc_wrap_wkup_0_mod_data,
	.modules = am62lx_sam61_wkup_psc_wrap_wkup_0_modules,
	.module_count = ARRAY_SIZE(am62lx_sam61_wkup_psc_wrap_wkup_0_mod_data),
	.psc_idx = 0,
	.drv_data = {
		.dev_data = {
			.soc = {
				.psc_idx = PSC_DEV_NONE,
			},
			.dev_clk_idx = AM62LX_DEV_SAM61_WKUP_PSC_WRAP_WKUP_0_CLOCKS,
			.n_clocks = 2,
			.pm_devgrp = PM_DEVGRP_00,
			.flags = DEVD_FLAG_DO_INIT|DEVD_FLAG_DRV_DATA,
		},
		.drv = &psc_drv,
		.r = am62lx_dev_sam61_wkup_psc_wrap_wkup_0_resources,
	},
};
static const struct dev_data
am62lx_dev_sam62_dm_wakeup_deepsleep_sources_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.dev_clk_idx = AM62LX_DEV_SAM62_DM_WAKEUP_DEEPSLEEP_SOURCES_WKUP_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam62a_mcu_16ff_mcu_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam62l_a53_256kb_wrap_main_0_arm_corepack_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MPU_CLST0,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0,
	},
	.dev_clk_idx = AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_CLOCKS,
	.n_clocks = 5,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam62l_ddr_wrap_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_DDR,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_DDR_LOCAL,
	},
	.dev_clk_idx = AM62LX_DEV_SAM62L_DDR_WRAP_MAIN_0_CLOCKS,
	.n_clocks = 3,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam62l_dftss_wrap_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_TEST,
	},
	.dev_clk_idx = AM62LX_DEV_SAM62L_DFTSS_WRAP_WKUP_0_CLOCKS,
	.n_clocks = 3,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_spi_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_SPI_MAIN_0_CLOCKS,
	.n_clocks = 6,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_spi_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_SPI_MAIN_1_CLOCKS,
	.n_clocks = 6,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_spi_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_SPI_MAIN_2_CLOCKS,
	.n_clocks = 6,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_spi_main_3 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_SPI_MAIN_3_CLOCKS,
	.n_clocks = 6,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_trng_drbg_eip76d_wrap_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MAINIP,
		.mod = AM62LX_PSC_LPSC_LPSC_MAINIP_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_TRNG_DRBG_EIP76D_WRAP_MAIN_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usart_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_USART_MAIN_1_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usart_main_2 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_USART_MAIN_2_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usart_main_3 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_USART_MAIN_3_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usart_main_4 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_USART_MAIN_4_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usart_main_5 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_USART_MAIN_5_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usart_main_6 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_USART_MAIN_6_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usart_wkup_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_WKPERI,
	},
	.dev_clk_idx = AM62LX_DEV_USART_WKUP_0_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usb2ss_16ffc_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB0,
	},
	.dev_clk_idx = AM62LX_DEV_USB2SS_16FFC_MAIN_0_CLOCKS,
	.n_clocks = 11,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usb2ss_16ffc_main_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB1,
	},
	.dev_clk_idx = AM62LX_DEV_USB2SS_16FFC_MAIN_1_CLOCKS,
	.n_clocks = 11,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_wiz16b8m4cdt3_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_DPHY_TX,
	},
	.dev_clk_idx = AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
	.n_clocks = 17,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam62l_a53_256kb_wrap_main_0_clkdiv_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.dev_clk_idx = AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_CLOCKS,
	.n_clocks = 3,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam62l_a53_256kb_wrap_main_0_pbist_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MPU_CLST0,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_PBIST,
	},
	.dev_clk_idx = AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_PBIST_0_CLOCKS,
	.n_clocks = 3,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam62l_a53_256kb_wrap_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam62l_a53_256kb_wrap_main_0_a53_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MPU_CLST0_CORE0,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE0,
	},
	.dev_clk_idx = AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_A53_0_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_sam62l_a53_256kb_wrap_main_0_a53_1 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_MPU_CLST0_CORE1,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_MPU_CLST0_CORE1,
	},
	.dev_clk_idx = AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_A53_1_CLOCKS,
	.n_clocks = 1,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_usart_main_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_PER,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_PER_COMMON,
	},
	.dev_clk_idx = AM62LX_DEV_USART_MAIN_0_CLOCKS,
	.n_clocks = 4,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_board_0 __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.dev_clk_idx = AM62LX_DEV_BOARD_0_CLOCKS,
	.n_clocks = 114,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_wkup_gtcclk_sel_dev_VD __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.dev_clk_idx = AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLOCKS,
	.n_clocks = 7,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_wkup_obsclk_mux_sel_dev_VD __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.dev_clk_idx = AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
	.n_clocks = 11,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_wkup_clkout_sel_dev_VD __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.dev_clk_idx = AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS,
	.n_clocks = 8,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_obsclk0_mux_sel_dev_VD __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.dev_clk_idx = AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
	.n_clocks = 12,
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_main_usb0_iso_VD __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB0_ISO_N,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_main_usb1_iso_VD __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_GP_CORE_CTL,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_GP_USB1_ISO_N,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_emif_cfg_iso_VD __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_DDR,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_DDR_CFG_ISO_N,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_emif_data_iso_VD __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = AM62LX_PSC_INST_SAM61_WKUP_PSC_WRAP_WKUP_0,
		.pd = AM62LX_PSC_PD_PD_DDR,
		.mod = AM62LX_PSC_LPSC_LPSC_MAIN_DDR_DATA_ISO_N,
	},
	.pm_devgrp = PM_DEVGRP_00,
};
static const struct dev_data
am62lx_dev_clk_32k_rc_sel_dev_VD __section(".const.devgroup.MAIN") = {
	.soc = {
		.psc_idx = PSC_DEV_NONE,
	},
	.dev_clk_idx = AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLOCKS,
	.n_clocks = 5,
	.pm_devgrp = PM_DEVGRP_00,
};

static const struct dev_clk_data MAIN_dev_clk_data[609] __section(".const.devgroup.MAIN") = {
	DEV_CLK_MUX(AM62LX_DEV_ADC12_CORE_MAIN_0_CLOCKS,
		    AM62LX_DEV_ADC0_ADC_CLK,
		    CLK_AM62LX_ADC0_CLKSEL_OUT0, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_ADC12_CORE_MAIN_0_CLOCKS,
		       AM62LX_DEV_ADC0_ADC_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_ADC12_CORE_MAIN_0_CLOCKS,
		       AM62LX_DEV_ADC0_ADC_CLK_PARENT_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK12,
		       CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 12, 1),
	DEV_CLK_PARENT(AM62LX_DEV_ADC12_CORE_MAIN_0_CLOCKS,
		       AM62LX_DEV_ADC0_ADC_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_ADC12_CORE_MAIN_0_CLOCKS,
		       AM62LX_DEV_ADC0_ADC_CLK_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 3),
	DEV_CLK(AM62LX_DEV_ADC12_CORE_MAIN_0_CLOCKS,
		AM62LX_DEV_ADC0_SYS_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK(AM62LX_DEV_ADC12_CORE_MAIN_0_CLOCKS,
		AM62LX_DEV_ADC0_VBUS_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK(AM62LX_DEV_AM62L_MAIN_GPIOMUX_INTROUTER_MAIN_0_CLOCKS,
		AM62LX_DEV_MAIN_GPIOMUX_INTROUTER0_INTR_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_AM62L_TIMESYNC_INTROUTER_MAIN_0_CLOCKS,
		AM62LX_DEV_TIMESYNC_INTROUTER0_INTR_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_CPPI_CLK_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK_MUX(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		    AM62LX_DEV_CPSW0_CPTS_RFT_CLK,
		    CLK_AM62LX_MAIN_CP_GEMAC_CPTS_CLK_SEL_OUT0, 1, 8),
	DEV_CLK_PARENT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_RFT_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_RFT_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_RFT_CLK_PARENT_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		       CLK_AM62LX_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_RFT_CLK_PARENT_CLK_32K_RC_SEL_OUT0,
		       CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 3),
	DEV_CLK_PARENT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_RFT_CLK_PARENT_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_RFT_CLK_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 5),
	DEV_CLK_PARENT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_RFT_CLK_PARENT_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK,
		       CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 1, 6),
	DEV_CLK_PARENT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_RFT_CLK_PARENT_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 1, 7),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_GMII1_MR_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 10),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_GMII1_MT_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 10),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_GMII2_MR_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 10),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_GMII2_MT_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 10),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_GMII_RFT_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 2),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_RGMII_MHZ_250_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_RGMII_MHZ_50_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 5),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_RGMII_MHZ_5_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 50),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_RMII1_MHZ_50_CLK,
		CLK_AM62LX_BOARD_0_RMII1_REF_CLK_OUT, 1),
	DEV_CLK(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		AM62LX_DEV_CPSW0_RMII2_MHZ_50_CLK,
		CLK_AM62LX_BOARD_0_RMII2_REF_CLK_OUT, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_GENF0,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0),
	DEV_CLK_OUTPUT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_CPTS_GENF1,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1),
	DEV_CLK_OUTPUT(AM62LX_DEV_CPSW_3GUSS_AM62L_MAIN_0_CLOCKS,
		       AM62LX_DEV_CPSW0_MDIO_MDCLK_O,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_MDIO_MDCLK_O),
	DEV_CLK(AM62LX_DEV_CPT2_AGGREGATOR32_MAIN_SYSCLK2_CLOCKS,
		AM62LX_DEV_CPT2_AGGR0_VCLK_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_CPT2_AGGREGATOR32_PER_SYSCLK2_CLOCKS,
		AM62LX_DEV_CPT2_AGGR1_VCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK(AM62LX_DEV_CPT2_AGGREGATOR32_WKUP_SYSCLK2_CLOCKS,
		AM62LX_DEV_WKUP_CPT2_AGGR0_VCLK_CLK,
		CLK_AM62LX_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_CXSTM500SS_MAIN_0_CLOCKS,
		AM62LX_DEV_STM0_ATB_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK(AM62LX_DEV_CXSTM500SS_MAIN_0_CLOCKS,
		AM62LX_DEV_STM0_CORE_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK(AM62LX_DEV_CXSTM500SS_MAIN_0_CLOCKS,
		AM62LX_DEV_STM0_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_DEBUGSS_K3_WRAP_CV0_MAIN_0_CLOCKS,
		AM62LX_DEV_DEBUGSS_WRAP0_ATB_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK(AM62LX_DEV_DEBUGSS_K3_WRAP_CV0_MAIN_0_CLOCKS,
		AM62LX_DEV_DEBUGSS_WRAP0_CORE_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_DEBUGSS_K3_WRAP_CV0_MAIN_0_CLOCKS,
		AM62LX_DEV_DEBUGSS_WRAP0_JTAG_TCK,
		CLK_AM62LX_BOARD_0_TCK_OUT, 1),
	DEV_CLK(AM62LX_DEV_DEBUGSS_K3_WRAP_CV0_MAIN_0_CLOCKS,
		AM62LX_DEV_DEBUGSS_WRAP0_P1500_WRCK,
		CLK_AM62LX_BOARD_0_TCK_OUT, 1),
	DEV_CLK(AM62LX_DEV_DEBUGSS_K3_WRAP_CV0_MAIN_0_CLOCKS,
		AM62LX_DEV_DEBUGSS_WRAP0_TREXPT_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT9_CLK, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_DEBUGSS_K3_WRAP_CV0_MAIN_0_CLOCKS,
		       AM62LX_DEV_DEBUGSS_WRAP0_CSTPIU_TRACECLK,
		       CLK_AM62LX_DEBUGSS_K3_WRAP_CV0_MAIN_0_CSTPIU_TRACECLK),
	DEV_CLK(AM62LX_DEV_DMSS_AM61_MAIN_0_BCDMA_0_CLOCKS,
		AM62LX_DEV_DMASS0_BCDMA_0_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 1),
	DEV_CLK(AM62LX_DEV_DMSS_AM61_MAIN_0_PKTDMA_0_CLOCKS,
		AM62LX_DEV_DMASS0_PKTDMA_0_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 1),
	DEV_CLK(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		AM62LX_DEV_TIMER0_TIMER_HCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		    AM62LX_DEV_TIMER0_TIMER_TCLK_CLK,
		    CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT0, 1, 16),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_CLK_32K_RC_SEL_OUT0,
		       CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0, 1, 10),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1, 1, 11),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_GLUELOGIC_RCOSC_CLKOUT,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1, 3),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 5),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		       CLK_AM62LX_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT, 1, 7),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK, 1, 8),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_TCLK_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK, 1, 9),
	DEV_CLK_OUTPUT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_0_CLOCKS,
		       AM62LX_DEV_TIMER0_TIMER_PWM,
		       CLK_AM62LX_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM),
	DEV_CLK(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_1_CLOCKS,
		AM62LX_DEV_TIMER1_TIMER_HCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_1_CLOCKS,
		    AM62LX_DEV_TIMER1_TIMER_TCLK_CLK,
		    CLK_AM62LX_MAIN_TIMER1_CASCADE_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_1_CLOCKS,
		       AM62LX_DEV_TIMER1_TIMER_TCLK_CLK_PARENT_MAIN_TIMERCLKN_SEL_OUT1,
		       CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT1, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_1_CLOCKS,
		       AM62LX_DEV_TIMER1_TIMER_TCLK_CLK_PARENT_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM,
		       CLK_AM62LX_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM, 1, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_1_CLOCKS,
		       AM62LX_DEV_TIMER1_TIMER_PWM,
		       CLK_AM62LX_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM),
	DEV_CLK(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		AM62LX_DEV_TIMER2_TIMER_HCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		    AM62LX_DEV_TIMER2_TIMER_TCLK_CLK,
		    CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT2, 1, 16),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_CLK_32K_RC_SEL_OUT0,
		       CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0, 1, 10),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1, 1, 11),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_GLUELOGIC_RCOSC_CLKOUT,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1, 3),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 5),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		       CLK_AM62LX_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT, 1, 7),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK, 1, 8),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_TCLK_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK, 1, 9),
	DEV_CLK_OUTPUT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_2_CLOCKS,
		       AM62LX_DEV_TIMER2_TIMER_PWM,
		       CLK_AM62LX_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM),
	DEV_CLK(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_3_CLOCKS,
		AM62LX_DEV_TIMER3_TIMER_HCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_3_CLOCKS,
		    AM62LX_DEV_TIMER3_TIMER_TCLK_CLK,
		    CLK_AM62LX_MAIN_TIMER3_CASCADE_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_3_CLOCKS,
		       AM62LX_DEV_TIMER3_TIMER_TCLK_CLK_PARENT_MAIN_TIMERCLKN_SEL_OUT3,
		       CLK_AM62LX_MAIN_TIMERCLKN_SEL_OUT3, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_3_CLOCKS,
		       AM62LX_DEV_TIMER3_TIMER_TCLK_CLK_PARENT_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM,
		       CLK_AM62LX_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM, 1, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_DMTIMER_DMC1MS_MAIN_3_CLOCKS,
		       AM62LX_DEV_TIMER3_TIMER_PWM,
		       CLK_AM62LX_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM),
	DEV_CLK(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_TIMER0_TIMER_HCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		    AM62LX_DEV_WKUP_TIMER0_TIMER_TCLK_CLK,
		    CLK_AM62LX_WKUP_TIMERCLKN_SEL_OUT0, 1, 8),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER0_TIMER_TCLK_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER0_TIMER_TCLK_CLK_PARENT_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK2,
		       CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2, 1),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER0_TIMER_TCLK_CLK_PARENT_GLUELOGIC_RCOSC_CLKOUT,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER0_TIMER_TCLK_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK, 1, 3),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER0_TIMER_TCLK_CLK_PARENT_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER0_TIMER_TCLK_CLK_PARENT_CLK_32K_RC_SEL_OUT0,
		       CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 5),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER0_TIMER_TCLK_CLK_PARENT_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0, 1, 6),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER0_TIMER_TCLK_CLK_PARENT_GLUELOGIC_RCOSC_CLK_1P0V_97P65K3,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K, 3, 7),
	DEV_CLK_OUTPUT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER0_TIMER_PWM,
		       CLK_AM62LX_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM),
	DEV_CLK(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_1_CLOCKS,
		AM62LX_DEV_WKUP_TIMER1_TIMER_HCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_1_CLOCKS,
		    AM62LX_DEV_WKUP_TIMER1_TIMER_TCLK_CLK,
		    CLK_AM62LX_WKUP_TIMER1_CASCADE_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_1_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER1_TIMER_TCLK_CLK_PARENT_WKUP_TIMERCLKN_SEL_OUT1,
		       CLK_AM62LX_WKUP_TIMERCLKN_SEL_OUT1, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_DMTIMER_DMC1MS_WKUP_1_CLOCKS,
		       AM62LX_DEV_WKUP_TIMER1_TIMER_TCLK_CLK_PARENT_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM,
		       CLK_AM62LX_DMTIMER_DMC1MS_WKUP_0_TIMER_PWM, 1, 1),
	DEV_CLK(AM62LX_DEV_ECAP_MAIN_0_CLOCKS,
		AM62LX_DEV_ECAP0_VBUS_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_ECAP_MAIN_1_CLOCKS,
		AM62LX_DEV_ECAP1_VBUS_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_ECAP_MAIN_2_CLOCKS,
		AM62LX_DEV_ECAP2_VBUS_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_ELM_MAIN_0_CLOCKS,
		AM62LX_DEV_ELM0_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS,
		    AM62LX_DEV_MMCSD1_EMMCSDSS_IO_CLK_I,
		    CLK_AM62LX_MAIN_EMMCSD1_IO_CLKLB_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD1_EMMCSDSS_IO_CLK_I_PARENT_BOARD_0_MMC1_CLKLB_OUT,
		       CLK_AM62LX_BOARD_0_MMC1_CLKLB_OUT, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD1_EMMCSDSS_IO_CLK_I_PARENT_BOARD_0_MMC1_CLK_OUT,
		       CLK_AM62LX_BOARD_0_MMC1_CLK_OUT, 1, 1),
	DEV_CLK(AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS,
		AM62LX_DEV_MMCSD1_EMMCSDSS_VBUS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK_MUX(AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS,
		    AM62LX_DEV_MMCSD1_EMMCSDSS_XIN_CLK,
		    CLK_AM62LX_MAIN_EMMCSD1_REFCLK_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD1_EMMCSDSS_XIN_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD1_EMMCSDSS_XIN_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT9_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT9_CLK, 1, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_EMMCSD4SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD1_EMMCSDSS_IO_CLK_O,
		       CLK_AM62LX_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O),
	DEV_CLK_MUX(AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS,
		    AM62LX_DEV_MMCSD2_EMMCSDSS_IO_CLK_I,
		    CLK_AM62LX_MAIN_EMMCSD2_IO_CLKLB_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS,
		       AM62LX_DEV_MMCSD2_EMMCSDSS_IO_CLK_I_PARENT_BOARD_0_MMC2_CLKLB_OUT,
		       CLK_AM62LX_BOARD_0_MMC2_CLKLB_OUT, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS,
		       AM62LX_DEV_MMCSD2_EMMCSDSS_IO_CLK_I_PARENT_BOARD_0_MMC2_CLK_OUT,
		       CLK_AM62LX_BOARD_0_MMC2_CLK_OUT, 1, 1),
	DEV_CLK(AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS,
		AM62LX_DEV_MMCSD2_EMMCSDSS_VBUS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK_MUX(AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS,
		    AM62LX_DEV_MMCSD2_EMMCSDSS_XIN_CLK,
		    CLK_AM62LX_MAIN_EMMCSD2_REFCLK_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS,
		       AM62LX_DEV_MMCSD2_EMMCSDSS_XIN_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS,
		       AM62LX_DEV_MMCSD2_EMMCSDSS_XIN_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT9_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT9_CLK, 1, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_EMMCSD4SS_MAIN_1_CLOCKS,
		       AM62LX_DEV_MMCSD2_EMMCSDSS_IO_CLK_O,
		       CLK_AM62LX_EMMCSD4SS_MAIN_1_EMMCSDSS_IO_CLK_O),
	DEV_CLK_MUX(AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS,
		    AM62LX_DEV_MMCSD0_EMMCSDSS_IO_CLK_I,
		    CLK_AM62LX_MAIN_EMMCSD0_IO_CLKLB_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD0_EMMCSDSS_IO_CLK_I_PARENT_BOARD_0_MMC0_CLKLB_OUT,
		       CLK_AM62LX_BOARD_0_MMC0_CLKLB_OUT, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD0_EMMCSDSS_IO_CLK_I_PARENT_BOARD_0_MMC0_CLK_OUT,
		       CLK_AM62LX_BOARD_0_MMC0_CLK_OUT, 1, 1),
	DEV_CLK(AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS,
		AM62LX_DEV_MMCSD0_EMMCSDSS_VBUS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK_MUX(AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS,
		    AM62LX_DEV_MMCSD0_EMMCSDSS_XIN_CLK,
		    CLK_AM62LX_MAIN_EMMCSD0_REFCLK_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD0_EMMCSDSS_XIN_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT5_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD0_EMMCSDSS_XIN_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT9_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT9_CLK, 1, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_EMMCSD8SS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MMCSD0_EMMCSDSS_IO_CLK_O,
		       CLK_AM62LX_EMMCSD8SS_MAIN_0_EMMCSDSS_IO_CLK_O),
	DEV_CLK(AM62LX_DEV_EQEP_T2_MAIN_0_CLOCKS,
		AM62LX_DEV_EQEP0_VBUS_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_EQEP_T2_MAIN_1_CLOCKS,
		AM62LX_DEV_EQEP1_VBUS_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_EQEP_T2_MAIN_2_CLOCKS,
		AM62LX_DEV_EQEP2_VBUS_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
		AM62LX_DEV_FSS0_OSPI0_DQS_CLK,
		CLK_AM62LX_BOARD_0_OSPI0_DQS_OUT, 1),
	DEV_CLK_MUX(AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
		    AM62LX_DEV_FSS0_OSPI0_ICLK_CLK,
		    CLK_AM62LX_MAIN_OSPI_LOOPBACK_CLK_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
		       AM62LX_DEV_FSS0_OSPI0_ICLK_CLK_PARENT_BOARD_0_OSPI0_DQS_OUT,
		       CLK_AM62LX_BOARD_0_OSPI0_DQS_OUT, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
		       AM62LX_DEV_FSS0_OSPI0_ICLK_CLK_PARENT_BOARD_0_OSPI0_LBCLKO_OUT,
		       CLK_AM62LX_BOARD_0_OSPI0_LBCLKO_OUT, 1, 1),
	DEV_CLK_MUX(AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
		    AM62LX_DEV_FSS0_OSPI0_RCLK_CLK,
		    CLK_AM62LX_MAIN_OSPI_REF_CLK_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
		       AM62LX_DEV_FSS0_OSPI0_RCLK_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT7_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT7_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
		       AM62LX_DEV_FSS0_OSPI0_RCLK_CLK_PARENT_HSDIV4_16FFT_WKUP_0_HSDIVOUT4_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT4_CLK, 1, 1),
	DEV_CLK(AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
		AM62LX_DEV_FSS0_VBUS_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_FSS_UL_128_MAIN_0_CLOCKS,
		       AM62LX_DEV_FSS0_OSPI0_OCLK_CLK,
		       CLK_AM62LX_FSS_UL_128_MAIN_0_OSPI0_OCLK_CLK),
	DEV_CLK(AM62LX_DEV_GIC500SS_1_2_SPI960_MAIN_0_CLOCKS,
		AM62LX_DEV_GICSS0_VCLK_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_GPIO_144_MAIN_0_CLOCKS,
		AM62LX_DEV_GPIO0_MMR_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_GPIO_144_MAIN_2_CLOCKS,
		AM62LX_DEV_GPIO2_MMR_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_GPIO_144_WKUP_0_CLOCKS,
		    AM62LX_DEV_WKUP_GPIO0_MMR_CLK,
		    CLK_AM62LX_WKUP_GPIO0_CLKSEL_OUT0, 4, 4),
	DEV_CLK_PARENT(AM62LX_DEV_GPIO_144_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_GPIO0_MMR_CLK_PARENT_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK4,
		       CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 16, 0),
	DEV_CLK_PARENT(AM62LX_DEV_GPIO_144_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_GPIO0_MMR_CLK_PARENT_RTCSS_WKUP_0_OSC_32K_CLK,
		       CLK_AM62LX_RTCSS_WKUP_0_OSC_32K_CLK, 4, 1),
	DEV_CLK_PARENT(AM62LX_DEV_GPIO_144_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_GPIO0_MMR_CLK_PARENT_GLUELOGIC_RCOSC_CLK_1P0V_97P65K3,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K, 12, 2),
	DEV_CLK_PARENT(AM62LX_DEV_GPIO_144_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_GPIO0_MMR_CLK_PARENT_GLUELOGIC_RCOSC_CLKOUT,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 4, 3),
	DEV_CLK_MUX(AM62LX_DEV_GPMC_MAIN_0_CLOCKS,
		    AM62LX_DEV_GPMC0_FUNC_CLK,
		    CLK_AM62LX_MAIN_GPMC_FCLK_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_GPMC_MAIN_0_CLOCKS,
		       AM62LX_DEV_GPMC0_FUNC_CLK_PARENT_HSDIV4_16FFT_MAIN_0_HSDIVOUT3_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT3_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_GPMC_MAIN_0_CLOCKS,
		       AM62LX_DEV_GPMC0_FUNC_CLK_PARENT_HSDIV4_16FFT_WKUP_0_HSDIVOUT3_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT3_CLK, 1, 1),
	DEV_CLK(AM62LX_DEV_GPMC_MAIN_0_CLOCKS,
		AM62LX_DEV_GPMC0_PI_GPMC_RET_CLK,
		CLK_AM62LX_BOARD_0_GPMC0_CLKLB_OUT, 1),
	DEV_CLK(AM62LX_DEV_GPMC_MAIN_0_CLOCKS,
		AM62LX_DEV_GPMC0_VBUSM_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK_OUTPUT(AM62LX_DEV_GPMC_MAIN_0_CLOCKS,
		       AM62LX_DEV_GPMC0_PO_GPMC_DEV_CLK,
		       CLK_AM62LX_GPMC_MAIN_0_PO_GPMC_DEV_CLK),
	DEV_CLK(AM62LX_DEV_K3_DSS_DSI_MAIN_0_CLOCKS,
		AM62LX_DEV_DSS_DSI0_DPHY_0_RX_ESC_CLK,
		CLK_AM62LX_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_M_RXCLKESC_CLK, 1),
	DEV_CLK(AM62LX_DEV_K3_DSS_DSI_MAIN_0_CLOCKS,
		AM62LX_DEV_DSS_DSI0_DPHY_0_TX_ESC_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 6),
	DEV_CLK(AM62LX_DEV_K3_DSS_DSI_MAIN_0_CLOCKS,
		AM62LX_DEV_DSS_DSI0_DPI_0_CLK,
		CLK_AM62LX_K3_DSS_NANO_MAIN_0_DPI_0_OUT_CLK, 1),
	DEV_CLK(AM62LX_DEV_K3_DSS_DSI_MAIN_0_CLOCKS,
		AM62LX_DEV_DSS_DSI0_PLL_CTRL_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_K3_DSS_DSI_MAIN_0_CLOCKS,
		AM62LX_DEV_DSS_DSI0_PPI_0_TXBYTECLKHS_CL_CLK,
		CLK_AM62LX_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_TXBYTECLKHS_CL_CLK, 1),
	DEV_CLK(AM62LX_DEV_K3_DSS_DSI_MAIN_0_CLOCKS,
		AM62LX_DEV_DSS_DSI0_SYS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK_MUX(AM62LX_DEV_K3_DSS_NANO_MAIN_0_CLOCKS,
		    AM62LX_DEV_DSS0_DPI_0_IN_CLK,
		    CLK_AM62LX_MAIN_DSS_DPI0_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_K3_DSS_NANO_MAIN_0_CLOCKS,
		       AM62LX_DEV_DSS0_DPI_0_IN_CLK_PARENT_HSDIV0_16FFT_MAIN_17_HSDIVOUT0_CLK,
		       CLK_AM62LX_HSDIV0_16FFT_MAIN_17_HSDIVOUT0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_K3_DSS_NANO_MAIN_0_CLOCKS,
		       AM62LX_DEV_DSS0_DPI_0_IN_CLK_PARENT_BOARD_0_VOUT0_EXTPCLKIN_OUT,
		       CLK_AM62LX_BOARD_0_VOUT0_EXTPCLKIN_OUT, 1, 1),
	DEV_CLK(AM62LX_DEV_K3_DSS_NANO_MAIN_0_CLOCKS,
		AM62LX_DEV_DSS0_DSS_FUNC_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK_OUTPUT(AM62LX_DEV_K3_DSS_NANO_MAIN_0_CLOCKS,
		       AM62LX_DEV_DSS0_DPI_0_OUT_CLK,
		       CLK_AM62LX_K3_DSS_NANO_MAIN_0_DPI_0_OUT_CLK),
	DEV_CLK(AM62LX_DEV_K3_EPWM_MAIN_0_CLOCKS,
		AM62LX_DEV_EPWM0_VBUSP_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_K3_EPWM_MAIN_1_CLOCKS,
		AM62LX_DEV_EPWM1_VBUSP_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_K3_EPWM_MAIN_2_CLOCKS,
		AM62LX_DEV_EPWM2_VBUSP_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_K3_LED2VBUS_MAIN_0_CLOCKS,
		AM62LX_DEV_LED0_VBUS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_K3_PBIST_8C28P_4BIT_WRAP_MAIN_0_CLOCKS,
		AM62LX_DEV_PBIST0_CLK8_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 4),
	DEV_CLK(AM62LX_DEV_K3_PBIST_8C28P_4BIT_WRAP_MAIN_0_CLOCKS,
		AM62LX_DEV_PBIST0_TCLK_CLK,
		CLK_AM62LX_BOARD_0_TCK_OUT, 1),
	DEV_CLK(AM62LX_DEV_K3_PBIST_8C28P_4BIT_WRAP_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_PBIST0_CLK8_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_K3VTM_N16FFC_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_VTM0_FIX_REF2_CLK,
		CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1),
	DEV_CLK(AM62LX_DEV_K3VTM_N16FFC_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_VTM0_FIX_REF_CLK,
		CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1),
	DEV_CLK(AM62LX_DEV_K3VTM_N16FFC_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_VTM0_VBUSP_CLK,
		CLK_AM62LX_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_MCANSS_MAIN_0_CLOCKS,
		    AM62LX_DEV_MCAN0_MCANSS_CCLK_CLK,
		    CLK_AM62LX_MAIN_MCANN_CLK_SEL_OUT0, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCAN0_MCANSS_CCLK_CLK_PARENT_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCAN0_MCANSS_CCLK_CLK_PARENT_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCAN0_MCANSS_CCLK_CLK_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCAN0_MCANSS_CCLK_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 3),
	DEV_CLK(AM62LX_DEV_MCANSS_MAIN_0_CLOCKS,
		AM62LX_DEV_MCAN0_MCANSS_HCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_MCANSS_MAIN_1_CLOCKS,
		    AM62LX_DEV_MCAN1_MCANSS_CCLK_CLK,
		    CLK_AM62LX_MAIN_MCANN_CLK_SEL_OUT1, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCAN1_MCANSS_CCLK_CLK_PARENT_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCAN1_MCANSS_CCLK_CLK_PARENT_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCAN1_MCANSS_CCLK_CLK_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCAN1_MCANSS_CCLK_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 3),
	DEV_CLK(AM62LX_DEV_MCANSS_MAIN_1_CLOCKS,
		AM62LX_DEV_MCAN1_MCANSS_HCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_MCANSS_MAIN_2_CLOCKS,
		    AM62LX_DEV_MCAN2_MCANSS_CCLK_CLK,
		    CLK_AM62LX_MAIN_MCANN_CLK_SEL_OUT2, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCAN2_MCANSS_CCLK_CLK_PARENT_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT4_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCAN2_MCANSS_CCLK_CLK_PARENT_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCAN2_MCANSS_CCLK_CLK_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_MCANSS_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCAN2_MCANSS_CCLK_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 3),
	DEV_CLK(AM62LX_DEV_MCANSS_MAIN_2_CLOCKS,
		AM62LX_DEV_MCAN2_MCANSS_HCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		    AM62LX_DEV_MCASP0_AUX_CLK,
		    CLK_AM62LX_MCASPN_CLKSEL_AUXCLK_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_AUX_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_AUX_CLK_PARENT_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK, 1, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		AM62LX_DEV_MCASP0_MCASP_ACLKR_PIN,
		CLK_AM62LX_BOARD_0_MCASP0_ACLKR_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		AM62LX_DEV_MCASP0_MCASP_ACLKX_PIN,
		CLK_AM62LX_BOARD_0_MCASP0_ACLKX_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		AM62LX_DEV_MCASP0_MCASP_AFSR_PIN,
		CLK_AM62LX_BOARD_0_MCASP0_AFSR_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		AM62LX_DEV_MCASP0_MCASP_AFSX_PIN,
		CLK_AM62LX_BOARD_0_MCASP0_AFSX_OUT, 1),
	DEV_CLK_MUX(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		    AM62LX_DEV_MCASP0_MCASP_AHCLKR_PIN,
		    CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKR_OUT0, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKR_PIN_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKR_PIN_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKR_PIN_PARENT_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK0_OUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKR_PIN_PARENT_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK1_OUT, 1, 3),
	DEV_CLK_MUX(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		    AM62LX_DEV_MCASP0_MCASP_AHCLKX_PIN,
		    CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKX_OUT0, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKX_PIN_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKX_PIN_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKX_PIN_PARENT_BOARD_0_AUDIO_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK0_OUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKX_PIN_PARENT_BOARD_0_AUDIO_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK1_OUT, 1, 3),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		AM62LX_DEV_MCASP0_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_ACLKR_POUT,
		       CLK_AM62LX_MCASP_MAIN_0_MCASP_ACLKR_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_ACLKX_POUT,
		       CLK_AM62LX_MCASP_MAIN_0_MCASP_ACLKX_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AFSR_POUT,
		       CLK_AM62LX_MCASP_MAIN_0_MCASP_AFSR_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AFSX_POUT,
		       CLK_AM62LX_MCASP_MAIN_0_MCASP_AFSX_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKR_POUT,
		       CLK_AM62LX_MCASP_MAIN_0_MCASP_AHCLKR_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCASP0_MCASP_AHCLKX_POUT,
		       CLK_AM62LX_MCASP_MAIN_0_MCASP_AHCLKX_POUT),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		AM62LX_DEV_MCASP1_AUX_CLK,
		CLK_AM62LX_MCASPN_CLKSEL_AUXCLK_OUT1, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		AM62LX_DEV_MCASP1_MCASP_ACLKR_PIN,
		CLK_AM62LX_BOARD_0_MCASP1_ACLKR_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		AM62LX_DEV_MCASP1_MCASP_ACLKX_PIN,
		CLK_AM62LX_BOARD_0_MCASP1_ACLKX_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		AM62LX_DEV_MCASP1_MCASP_AFSR_PIN,
		CLK_AM62LX_BOARD_0_MCASP1_AFSR_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		AM62LX_DEV_MCASP1_MCASP_AFSX_PIN,
		CLK_AM62LX_BOARD_0_MCASP1_AFSX_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		AM62LX_DEV_MCASP1_MCASP_AHCLKR_PIN,
		CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKR_OUT1, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		AM62LX_DEV_MCASP1_MCASP_AHCLKX_PIN,
		CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKX_OUT1, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		AM62LX_DEV_MCASP1_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCASP1_MCASP_ACLKR_POUT,
		       CLK_AM62LX_MCASP_MAIN_1_MCASP_ACLKR_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCASP1_MCASP_ACLKX_POUT,
		       CLK_AM62LX_MCASP_MAIN_1_MCASP_ACLKX_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCASP1_MCASP_AFSR_POUT,
		       CLK_AM62LX_MCASP_MAIN_1_MCASP_AFSR_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCASP1_MCASP_AFSX_POUT,
		       CLK_AM62LX_MCASP_MAIN_1_MCASP_AFSX_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCASP1_MCASP_AHCLKR_POUT,
		       CLK_AM62LX_MCASP_MAIN_1_MCASP_AHCLKR_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCASP1_MCASP_AHCLKX_POUT,
		       CLK_AM62LX_MCASP_MAIN_1_MCASP_AHCLKX_POUT),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		AM62LX_DEV_MCASP2_AUX_CLK,
		CLK_AM62LX_MCASPN_CLKSEL_AUXCLK_OUT2, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		AM62LX_DEV_MCASP2_MCASP_ACLKR_PIN,
		CLK_AM62LX_BOARD_0_MCASP2_ACLKR_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		AM62LX_DEV_MCASP2_MCASP_ACLKX_PIN,
		CLK_AM62LX_BOARD_0_MCASP2_ACLKX_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		AM62LX_DEV_MCASP2_MCASP_AFSR_PIN,
		CLK_AM62LX_BOARD_0_MCASP2_AFSR_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		AM62LX_DEV_MCASP2_MCASP_AFSX_PIN,
		CLK_AM62LX_BOARD_0_MCASP2_AFSX_OUT, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		AM62LX_DEV_MCASP2_MCASP_AHCLKR_PIN,
		CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKR_OUT2, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		AM62LX_DEV_MCASP2_MCASP_AHCLKX_PIN,
		CLK_AM62LX_MCASPN_AHCLKSEL_AHCLKX_OUT2, 1),
	DEV_CLK(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		AM62LX_DEV_MCASP2_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCASP2_MCASP_ACLKR_POUT,
		       CLK_AM62LX_MCASP_MAIN_2_MCASP_ACLKR_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCASP2_MCASP_ACLKX_POUT,
		       CLK_AM62LX_MCASP_MAIN_2_MCASP_ACLKX_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCASP2_MCASP_AFSR_POUT,
		       CLK_AM62LX_MCASP_MAIN_2_MCASP_AFSR_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCASP2_MCASP_AFSX_POUT,
		       CLK_AM62LX_MCASP_MAIN_2_MCASP_AFSX_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCASP2_MCASP_AHCLKR_POUT,
		       CLK_AM62LX_MCASP_MAIN_2_MCASP_AHCLKR_POUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_MCASP_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCASP2_MCASP_AHCLKX_POUT,
		       CLK_AM62LX_MCASP_MAIN_2_MCASP_AHCLKX_POUT),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_0_CLOCKS,
		AM62LX_DEV_I2C0_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_0_CLOCKS,
		AM62LX_DEV_I2C0_PISCL,
		CLK_AM62LX_BOARD_0_I2C0_SCL_OUT, 1),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_0_CLOCKS,
		AM62LX_DEV_I2C0_PISYS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_MSHSI2C_MAIN_0_CLOCKS,
		       AM62LX_DEV_I2C0_PORSCL,
		       CLK_AM62LX_MSHSI2C_MAIN_0_PORSCL),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_1_CLOCKS,
		AM62LX_DEV_I2C1_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_1_CLOCKS,
		AM62LX_DEV_I2C1_PISCL,
		CLK_AM62LX_BOARD_0_I2C1_SCL_OUT, 1),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_1_CLOCKS,
		AM62LX_DEV_I2C1_PISYS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_MSHSI2C_MAIN_1_CLOCKS,
		       AM62LX_DEV_I2C1_PORSCL,
		       CLK_AM62LX_MSHSI2C_MAIN_1_PORSCL),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_2_CLOCKS,
		AM62LX_DEV_I2C2_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_2_CLOCKS,
		AM62LX_DEV_I2C2_PISCL,
		CLK_AM62LX_BOARD_0_I2C2_SCL_OUT, 1),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_2_CLOCKS,
		AM62LX_DEV_I2C2_PISYS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_MSHSI2C_MAIN_2_CLOCKS,
		       AM62LX_DEV_I2C2_PORSCL,
		       CLK_AM62LX_MSHSI2C_MAIN_2_PORSCL),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_3_CLOCKS,
		AM62LX_DEV_I2C3_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_3_CLOCKS,
		AM62LX_DEV_I2C3_PISCL,
		CLK_AM62LX_BOARD_0_I2C3_SCL_OUT, 1),
	DEV_CLK(AM62LX_DEV_MSHSI2C_MAIN_3_CLOCKS,
		AM62LX_DEV_I2C3_PISYS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_MSHSI2C_MAIN_3_CLOCKS,
		       AM62LX_DEV_I2C3_PORSCL,
		       CLK_AM62LX_MSHSI2C_MAIN_3_PORSCL),
	DEV_CLK(AM62LX_DEV_MSHSI2C_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_I2C0_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_MSHSI2C_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_I2C0_PISCL,
		CLK_AM62LX_BOARD_0_WKUP_I2C0_SCL_OUT, 4),
	DEV_CLK(AM62LX_DEV_MSHSI2C_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_I2C0_PISYS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT1_CLK, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_MSHSI2C_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_I2C0_PORSCL,
		       CLK_AM62LX_MSHSI2C_WKUP_0_PORSCL),
	DEV_CLK_MUX(AM62LX_DEV_GTC_R10_WKUP_0_CLOCKS,
		    AM62LX_DEV_WKUP_GTC0_GTC_CLK,
		    CLK_AM62LX_WKUP_GTC_OUTMUX_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_GTC_R10_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_GTC0_GTC_CLK_PARENT_WKUP_GTCCLK_SEL_OUT0,
		       CLK_AM62LX_WKUP_GTCCLK_SEL_OUT0, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_GTC_R10_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_GTC0_GTC_CLK_PARENT_CLK_32K_RC_SEL_OUT0,
		       CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 1),
	DEV_CLK(AM62LX_DEV_GTC_R10_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_GTC0_VBUSP_CLK,
		CLK_AM62LX_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK, 4),
	DEV_CLK(AM62LX_DEV_RTCSS_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_RTCSS0_ANA_OSC32K_CLK,
		CLK_AM62LX_GLUELOGIC_LFOSC0_CLK, 1),
	DEV_CLK(AM62LX_DEV_RTCSS_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_RTCSS0_AUX_32K_CLK,
		CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1),
	DEV_CLK(AM62LX_DEV_RTCSS_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_RTCSS0_JTAG_WRCK,
		CLK_AM62LX_BOARD_0_TCK_OUT, 1),
	DEV_CLK(AM62LX_DEV_RTCSS_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_RTCSS0_VCLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 8),
	DEV_CLK_OUTPUT(AM62LX_DEV_RTCSS_WKUP_0_CLOCKS,
		       AM62LX_DEV_WKUP_RTCSS0_OSC_32K_CLK,
		       CLK_AM62LX_RTCSS_WKUP_0_OSC_32K_CLK),
	DEV_CLK_MUX(AM62LX_DEV_RTI_CFG1_MAIN_A53_0_CLOCKS,
		    AM62LX_DEV_RTI0_RTI_CLK,
		    CLK_AM62LX_MAIN_WWDTCLKN_SEL_OUT0, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_RTI_CFG1_MAIN_A53_0_CLOCKS,
		       AM62LX_DEV_RTI0_RTI_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_RTI_CFG1_MAIN_A53_0_CLOCKS,
		       AM62LX_DEV_RTI0_RTI_CLK_PARENT_CLK_32K_RC_SEL_OUT0,
		       CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_RTI_CFG1_MAIN_A53_0_CLOCKS,
		       AM62LX_DEV_RTI0_RTI_CLK_PARENT_GLUELOGIC_RCOSC_CLKOUT,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_RTI_CFG1_MAIN_A53_0_CLOCKS,
		       AM62LX_DEV_RTI0_RTI_CLK_PARENT_GLUELOGIC_RCOSC_CLK_1P0V_97P65K3,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K, 3, 3),
	DEV_CLK(AM62LX_DEV_RTI_CFG1_MAIN_A53_0_CLOCKS,
		AM62LX_DEV_RTI0_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_RTI_CFG1_MAIN_A53_1_CLOCKS,
		    AM62LX_DEV_RTI1_RTI_CLK,
		    CLK_AM62LX_MAIN_WWDTCLKN_SEL_OUT1, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_RTI_CFG1_MAIN_A53_1_CLOCKS,
		       AM62LX_DEV_RTI1_RTI_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_RTI_CFG1_MAIN_A53_1_CLOCKS,
		       AM62LX_DEV_RTI1_RTI_CLK_PARENT_CLK_32K_RC_SEL_OUT0,
		       CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_RTI_CFG1_MAIN_A53_1_CLOCKS,
		       AM62LX_DEV_RTI1_RTI_CLK_PARENT_GLUELOGIC_RCOSC_CLKOUT,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_RTI_CFG1_MAIN_A53_1_CLOCKS,
		       AM62LX_DEV_RTI1_RTI_CLK_PARENT_GLUELOGIC_RCOSC_CLK_1P0V_97P65K3,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K, 3, 3),
	DEV_CLK(AM62LX_DEV_RTI_CFG1_MAIN_A53_1_CLOCKS,
		AM62LX_DEV_RTI1_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_SAM61_DEBUG_MAIN_CELL_MAIN_0_CLOCKS,
		AM62LX_DEV_DEBUGSS0_CFG_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_SAM61_DEBUG_MAIN_CELL_MAIN_0_CLOCKS,
		AM62LX_DEV_DEBUGSS0_DBG_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK(AM62LX_DEV_SAM61_DEBUG_MAIN_CELL_MAIN_0_CLOCKS,
		AM62LX_DEV_DEBUGSS0_SYS_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2),
	DEV_CLK(AM62LX_DEV_SAM61_MSRAM6KX128_MAIN_0_CLOCKS,
		AM62LX_DEV_MSRAM_96K0_VCLK_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 1),
	DEV_CLK(AM62LX_DEV_SAM61_PSRAM16KX32_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_PSRAM_64K0_CLK_CLK,
		CLK_AM62LX_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK, 1),
	DEV_CLK(AM62LX_DEV_SAM61_PSROM64KX32_MAIN_0_CLOCKS,
		AM62LX_DEV_ROM0_CLK_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_SAM61_WKUP_PSC_WRAP_WKUP_0_CLOCKS,
		AM62LX_DEV_PSC0_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_SAM61_WKUP_PSC_WRAP_WKUP_0_CLOCKS,
		AM62LX_DEV_PSC0_SLOW_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 24),
	DEV_CLK(AM62LX_DEV_SAM62_DM_WAKEUP_DEEPSLEEP_SOURCES_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_DEEPSLEEP_SOURCES0_CLK_12M_RC_CLK,
		CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1),
	DEV_CLK(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_CLOCKS,
		AM62LX_DEV_COMPUTE_CLUSTER0_ARM_COREPACK_0_COREPAC_ARM_CLK_CLK,
		CLK_AM62LX_HSDIV0_16FFT_MAIN_8_HSDIVOUT0_CLK, 1),
	DEV_CLK(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_CLOCKS,
		AM62LX_DEV_COMPUTE_CLUSTER0_ARM_COREPACK_0_PLL_CTRL_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_CLOCKS,
		       AM62LX_DEV_COMPUTE_CLUSTER0_ARM_COREPACK_0_A53_DIVH_CLK4_OBSCLK_OUT_CLK,
		       CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_A53_DIVH_CLK4_OBSCLK_OUT_CLK),
	DEV_CLK(AM62LX_DEV_SAM62L_DDR_WRAP_MAIN_0_CLOCKS,
		AM62LX_DEV_DDR16SS0_DDRSS_DDR_PLL_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT2_CLK, 1),
	DEV_CLK(AM62LX_DEV_SAM62L_DDR_WRAP_MAIN_0_CLOCKS,
		AM62LX_DEV_DDR16SS0_DDRSS_TCK,
		CLK_AM62LX_BOARD_0_TCK_OUT, 1),
	DEV_CLK(AM62LX_DEV_SAM62L_DDR_WRAP_MAIN_0_CLOCKS,
		AM62LX_DEV_DDR16SS0_PLL_CTRL_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 1),
	DEV_CLK(AM62LX_DEV_SAM62L_DFTSS_WRAP_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_DFTSS0_PLL_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 1),
	DEV_CLK(AM62LX_DEV_SAM62L_DFTSS_WRAP_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_DFTSS0_VBUSP_CLK_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_SPI_MAIN_0_CLOCKS,
		AM62LX_DEV_MCSPI0_CLKSPIREF_CLK,
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_SPI_MAIN_0_CLOCKS,
		AM62LX_DEV_MCSPI0_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_OUTPUT(AM62LX_DEV_SPI_MAIN_0_CLOCKS,
		       AM62LX_DEV_MCSPI0_IO_CLKSPIO_CLK,
		       CLK_AM62LX_SPI_MAIN_0_IO_CLKSPIO_CLK),
	DEV_CLK(AM62LX_DEV_SPI_MAIN_1_CLOCKS,
		AM62LX_DEV_MCSPI1_CLKSPIREF_CLK,
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_SPI_MAIN_1_CLOCKS,
		AM62LX_DEV_MCSPI1_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_OUTPUT(AM62LX_DEV_SPI_MAIN_1_CLOCKS,
		       AM62LX_DEV_MCSPI1_IO_CLKSPIO_CLK,
		       CLK_AM62LX_SPI_MAIN_1_IO_CLKSPIO_CLK),
	DEV_CLK(AM62LX_DEV_SPI_MAIN_2_CLOCKS,
		AM62LX_DEV_MCSPI2_CLKSPIREF_CLK,
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_SPI_MAIN_2_CLOCKS,
		AM62LX_DEV_MCSPI2_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_OUTPUT(AM62LX_DEV_SPI_MAIN_2_CLOCKS,
		       AM62LX_DEV_MCSPI2_IO_CLKSPIO_CLK,
		       CLK_AM62LX_SPI_MAIN_2_IO_CLKSPIO_CLK),
	DEV_CLK(AM62LX_DEV_SPI_MAIN_3_CLOCKS,
		AM62LX_DEV_MCSPI3_CLKSPIREF_CLK,
		CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT6_CLK, 1),
	DEV_CLK(AM62LX_DEV_SPI_MAIN_3_CLOCKS,
		AM62LX_DEV_MCSPI3_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK_OUTPUT(AM62LX_DEV_SPI_MAIN_3_CLOCKS,
		       AM62LX_DEV_MCSPI3_IO_CLKSPIO_CLK,
		       CLK_AM62LX_SPI_MAIN_3_IO_CLKSPIO_CLK),
	DEV_CLK(AM62LX_DEV_TRNG_DRBG_EIP76D_WRAP_MAIN_0_CLOCKS,
		AM62LX_DEV_TRNG_DRBG_EIP76D_WRAP0_VCLK_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_USART_MAIN_1_CLOCKS,
		AM62LX_DEV_UART1_FCLK_CLK,
		CLK_AM62LX_MAIN_USART_CLKDIV_OUT1, 1),
	DEV_CLK(AM62LX_DEV_USART_MAIN_1_CLOCKS,
		AM62LX_DEV_UART1_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_USART_MAIN_2_CLOCKS,
		AM62LX_DEV_UART2_FCLK_CLK,
		CLK_AM62LX_MAIN_USART_CLKDIV_OUT2, 1),
	DEV_CLK(AM62LX_DEV_USART_MAIN_2_CLOCKS,
		AM62LX_DEV_UART2_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_USART_MAIN_3_CLOCKS,
		AM62LX_DEV_UART3_FCLK_CLK,
		CLK_AM62LX_MAIN_USART_CLKDIV_OUT3, 1),
	DEV_CLK(AM62LX_DEV_USART_MAIN_3_CLOCKS,
		AM62LX_DEV_UART3_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_USART_MAIN_4_CLOCKS,
		AM62LX_DEV_UART4_FCLK_CLK,
		CLK_AM62LX_MAIN_USART_CLKDIV_OUT4, 1),
	DEV_CLK(AM62LX_DEV_USART_MAIN_4_CLOCKS,
		AM62LX_DEV_UART4_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_USART_MAIN_5_CLOCKS,
		AM62LX_DEV_UART5_FCLK_CLK,
		CLK_AM62LX_MAIN_USART_CLKDIV_OUT5, 1),
	DEV_CLK(AM62LX_DEV_USART_MAIN_5_CLOCKS,
		AM62LX_DEV_UART5_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_USART_MAIN_6_CLOCKS,
		AM62LX_DEV_UART6_FCLK_CLK,
		CLK_AM62LX_MAIN_USART_CLKDIV_OUT6, 1),
	DEV_CLK(AM62LX_DEV_USART_MAIN_6_CLOCKS,
		AM62LX_DEV_UART6_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_USART_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_UART0_FCLK_CLK,
		CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT2_CLK, 1),
	DEV_CLK(AM62LX_DEV_USART_WKUP_0_CLOCKS,
		AM62LX_DEV_WKUP_UART0_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_USB2SS_16FFC_MAIN_0_CLOCKS,
		AM62LX_DEV_USB0_BUS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_USB2SS_16FFC_MAIN_0_CLOCKS,
		AM62LX_DEV_USB0_CFG_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 4),
	DEV_CLK(AM62LX_DEV_USB2SS_16FFC_MAIN_0_CLOCKS,
		AM62LX_DEV_USB0_USB2_APB_PCLK_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_USB2SS_16FFC_MAIN_0_CLOCKS,
		    AM62LX_DEV_USB0_USB2_REFCLOCK_CLK,
		    CLK_AM62LX_MAIN_USB0_REFCLK_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_USB2SS_16FFC_MAIN_0_CLOCKS,
		       AM62LX_DEV_USB0_USB2_REFCLOCK_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_USB2SS_16FFC_MAIN_0_CLOCKS,
		       AM62LX_DEV_USB0_USB2_REFCLOCK_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK4,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 4, 1),
	DEV_CLK(AM62LX_DEV_USB2SS_16FFC_MAIN_0_CLOCKS,
		AM62LX_DEV_USB0_USB2_TAP_TCK,
		CLK_AM62LX_BOARD_0_TCK_OUT, 1),
	DEV_CLK(AM62LX_DEV_USB2SS_16FFC_MAIN_1_CLOCKS,
		AM62LX_DEV_USB1_BUS_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_USB2SS_16FFC_MAIN_1_CLOCKS,
		AM62LX_DEV_USB1_CFG_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 4),
	DEV_CLK(AM62LX_DEV_USB2SS_16FFC_MAIN_1_CLOCKS,
		AM62LX_DEV_USB1_USB2_APB_PCLK_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_USB2SS_16FFC_MAIN_1_CLOCKS,
		    AM62LX_DEV_USB1_USB2_REFCLOCK_CLK,
		    CLK_AM62LX_MAIN_USB1_REFCLK_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_USB2SS_16FFC_MAIN_1_CLOCKS,
		       AM62LX_DEV_USB1_USB2_REFCLOCK_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_USB2SS_16FFC_MAIN_1_CLOCKS,
		       AM62LX_DEV_USB1_USB2_REFCLOCK_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK4,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 4, 1),
	DEV_CLK(AM62LX_DEV_USB2SS_16FFC_MAIN_1_CLOCKS,
		AM62LX_DEV_USB1_USB2_TAP_TCK,
		CLK_AM62LX_BOARD_0_TCK_OUT, 1),
	DEV_CLK(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		AM62LX_DEV_DPHY_TX0_CLK,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 4),
	DEV_CLK_MUX(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		    AM62LX_DEV_DPHY_TX0_DPHY_REF_CLK,
		    CLK_AM62LX_MAIN_DPHYTX_REFCLK_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		       AM62LX_DEV_DPHY_TX0_DPHY_REF_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		       AM62LX_DEV_DPHY_TX0_DPHY_REF_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 1, 1),
	DEV_CLK(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		AM62LX_DEV_DPHY_TX0_IP1_PPI_M_TXCLKESC_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 6),
	DEV_CLK(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		AM62LX_DEV_DPHY_TX0_IP2_PPI_M_TXCLKESC_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 6),
	DEV_CLK(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		AM62LX_DEV_DPHY_TX0_IP3_PPI_M_TXCLKESC_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 6),
	DEV_CLK(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		AM62LX_DEV_DPHY_TX0_IP4_PPI_M_TXCLKESC_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 6),
	DEV_CLK(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		AM62LX_DEV_DPHY_TX0_PSM_CLK,
		CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT8_CLK, 6),
	DEV_CLK_OUTPUT(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		       AM62LX_DEV_DPHY_TX0_IP1_PPI_M_RXCLKESC_CLK,
		       CLK_AM62LX_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_M_RXCLKESC_CLK),
	DEV_CLK_OUTPUT(AM62LX_DEV_WIZ16B8M4CDT3_MAIN_0_CLOCKS,
		       AM62LX_DEV_DPHY_TX0_IP1_PPI_TXBYTECLKHS_CL_CLK,
		       CLK_AM62LX_WIZ16B8M4CDT3_MAIN_0_IP1_PPI_TXBYTECLKHS_CL_CLK),
	DEV_CLK(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_CLOCKS,
		AM62LX_DEV_COMPUTE_CLUSTER0_CLKDIV_0_FUNC_CLKIN_CLK,
		CLK_AM62LX_HSDIV0_16FFT_MAIN_8_HSDIVOUT0_CLK, 1),
	DEV_CLK_OUTPUT(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_CLOCKS,
		       AM62LX_DEV_COMPUTE_CLUSTER0_CLKDIV_0_DIVH_CLK4_CLK_CLK,
		       CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVH_CLK4_CLK_CLK),
	DEV_CLK_OUTPUT(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_CLOCKS,
		       AM62LX_DEV_COMPUTE_CLUSTER0_CLKDIV_0_DIVP_CLK1_CLK_CLK,
		       CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVP_CLK1_CLK_CLK),
	DEV_CLK(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_PBIST_0_CLOCKS,
		AM62LX_DEV_COMPUTE_CLUSTER0_PBIST_0_DIVH_CLK4_CLK_CLK,
		CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVH_CLK4_CLK_CLK, 1),
	DEV_CLK(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_PBIST_0_CLOCKS,
		AM62LX_DEV_COMPUTE_CLUSTER0_PBIST_0_DIVP_CLK1_CLK_CLK,
		CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_CLKDIV_0_DIVP_CLK1_CLK_CLK, 1),
	DEV_CLK(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_A53_0_CLOCKS,
		AM62LX_DEV_COMPUTE_CLUSTER0_A53_0_A53_CORE0_ARM_CLK_CLK,
		CLK_AM62LX_HSDIV0_16FFT_MAIN_8_HSDIVOUT0_CLK, 1),
	DEV_CLK(AM62LX_DEV_SAM62L_A53_256KB_WRAP_MAIN_0_A53_1_CLOCKS,
		AM62LX_DEV_COMPUTE_CLUSTER0_A53_1_A53_CORE1_ARM_CLK_CLK,
		CLK_AM62LX_HSDIV0_16FFT_MAIN_8_HSDIVOUT0_CLK, 1),
	DEV_CLK(AM62LX_DEV_USART_MAIN_0_CLOCKS,
		AM62LX_DEV_UART0_FCLK_CLK,
		CLK_AM62LX_MAIN_USART_CLKDIV_OUT0, 1),
	DEV_CLK(AM62LX_DEV_USART_MAIN_0_CLOCKS,
		AM62LX_DEV_UART0_VBUSP_CLK,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 4),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_AUDIO_EXT_REFCLK0_IN,
		CLK_AM62LX_AUDIO_REFCLKN_OUT0, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_AUDIO_EXT_REFCLK1_IN,
		CLK_AM62LX_AUDIO_REFCLKN_OUT1, 1),
	DEV_CLK_MUX(AM62LX_DEV_BOARD_0_CLOCKS,
		    AM62LX_DEV_BOARD0_CLKOUT0_IN,
		    CLK_AM62LX_CLKOUT0_CTRL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_CLKOUT0_IN_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK5,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 5, 0),
	DEV_CLK_PARENT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_CLKOUT0_IN_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK10,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 10, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_DDR0_CK0_IN,
		CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 2),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_GPMC0_CLKLB_IN,
		CLK_AM62LX_GPMC_MAIN_0_PO_GPMC_DEV_CLK, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_GPMC0_CLK_IN,
		CLK_AM62LX_GPMC_MAIN_0_PO_GPMC_DEV_CLK, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_GPMC0_FCLK_MUX_IN,
		CLK_AM62LX_MAIN_GPMC_FCLK_SEL_OUT0, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_I2C0_SCL_IN,
		CLK_AM62LX_MSHSI2C_MAIN_0_PORSCL, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_I2C1_SCL_IN,
		CLK_AM62LX_MSHSI2C_MAIN_1_PORSCL, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_I2C2_SCL_IN,
		CLK_AM62LX_MSHSI2C_MAIN_2_PORSCL, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_I2C3_SCL_IN,
		CLK_AM62LX_MSHSI2C_MAIN_3_PORSCL, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP0_ACLKR_IN,
		CLK_AM62LX_MCASP_MAIN_0_MCASP_ACLKR_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP0_ACLKX_IN,
		CLK_AM62LX_MCASP_MAIN_0_MCASP_ACLKX_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP0_AFSR_IN,
		CLK_AM62LX_MCASP_MAIN_0_MCASP_AFSR_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP0_AFSX_IN,
		CLK_AM62LX_MCASP_MAIN_0_MCASP_AFSX_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP1_ACLKR_IN,
		CLK_AM62LX_MCASP_MAIN_1_MCASP_ACLKR_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP1_ACLKX_IN,
		CLK_AM62LX_MCASP_MAIN_1_MCASP_ACLKX_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP1_AFSR_IN,
		CLK_AM62LX_MCASP_MAIN_1_MCASP_AFSR_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP1_AFSX_IN,
		CLK_AM62LX_MCASP_MAIN_1_MCASP_AFSX_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP2_ACLKR_IN,
		CLK_AM62LX_MCASP_MAIN_2_MCASP_ACLKR_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP2_ACLKX_IN,
		CLK_AM62LX_MCASP_MAIN_2_MCASP_ACLKX_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP2_AFSR_IN,
		CLK_AM62LX_MCASP_MAIN_2_MCASP_AFSR_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MCASP2_AFSX_IN,
		CLK_AM62LX_MCASP_MAIN_2_MCASP_AFSX_POUT, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MDIO0_MDC_IN,
		CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_MDIO_MDCLK_O, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MMC0_CLKLB_IN,
		CLK_AM62LX_EMMCSD8SS_MAIN_0_EMMCSDSS_IO_CLK_O, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MMC0_CLK_IN,
		CLK_AM62LX_EMMCSD8SS_MAIN_0_EMMCSDSS_IO_CLK_O, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MMC1_CLKLB_IN,
		CLK_AM62LX_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MMC1_CLK_IN,
		CLK_AM62LX_EMMCSD4SS_MAIN_0_EMMCSDSS_IO_CLK_O, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MMC2_CLKLB_IN,
		CLK_AM62LX_EMMCSD4SS_MAIN_1_EMMCSDSS_IO_CLK_O, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_MMC2_CLK_IN,
		CLK_AM62LX_EMMCSD4SS_MAIN_1_EMMCSDSS_IO_CLK_O, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_OBSCLK0_IN,
		CLK_AM62LX_MAIN_OBSCLK_OUTMUX_SEL_OUT0, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_OBSCLK1_IN,
		CLK_AM62LX_MAIN_OBSCLK_OUTMUX_SEL_OUT0, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_OSPI0_CLK_IN,
		CLK_AM62LX_FSS_UL_128_MAIN_0_OSPI0_OCLK_CLK, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_OSPI0_LBCLKO_IN,
		CLK_AM62LX_FSS_UL_128_MAIN_0_OSPI0_OCLK_CLK, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_SPI0_CLK_IN,
		CLK_AM62LX_SPI_MAIN_0_IO_CLKSPIO_CLK, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_SPI1_CLK_IN,
		CLK_AM62LX_SPI_MAIN_1_IO_CLKSPIO_CLK, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_SPI2_CLK_IN,
		CLK_AM62LX_SPI_MAIN_2_IO_CLKSPIO_CLK, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_SPI3_CLK_IN,
		CLK_AM62LX_SPI_MAIN_3_IO_CLKSPIO_CLK, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_TIMER_IO0_IN,
		CLK_AM62LX_DMTIMER_DMC1MS_MAIN_0_TIMER_PWM, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_TIMER_IO1_IN,
		CLK_AM62LX_DMTIMER_DMC1MS_MAIN_1_TIMER_PWM, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_TIMER_IO2_IN,
		CLK_AM62LX_DMTIMER_DMC1MS_MAIN_2_TIMER_PWM, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_TIMER_IO3_IN,
		CLK_AM62LX_DMTIMER_DMC1MS_MAIN_3_TIMER_PWM, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_TRC_CLK_IN,
		CLK_AM62LX_DEBUGSS_K3_WRAP_CV0_MAIN_0_CSTPIU_TRACECLK, 1),
	DEV_CLK_MUX(AM62LX_DEV_BOARD_0_CLOCKS,
		    AM62LX_DEV_BOARD0_WKUP_CLKOUT0_IN,
		    CLK_AM62LX_WKUP_CLKOUT_SEL_IO_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_WKUP_CLKOUT0_IN_PARENT_WKUP_CLKOUT_SEL_OUT0,
		       CLK_AM62LX_WKUP_CLKOUT_SEL_OUT0, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_WKUP_CLKOUT0_IN_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 1),
	DEV_CLK_MUX(AM62LX_DEV_BOARD_0_CLOCKS,
		    AM62LX_DEV_BOARD0_WKUP_OBSCLK0_IN,
		    CLK_AM62LX_WKUP_OBSCLK_OUTMUX_SEL_OUT0, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_WKUP_OBSCLK0_IN_PARENT_WKUP_OBSCLK_MUX_SEL_OUT0,
		       CLK_AM62LX_WKUP_OBSCLK_MUX_SEL_OUT0, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_WKUP_OBSCLK0_IN_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 1),
	DEV_CLK(AM62LX_DEV_BOARD_0_CLOCKS,
		AM62LX_DEV_BOARD0_WKUP_SYSCLKOUT0_IN,
		CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_SYSCLKOUT_CLK, 4),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_AUDIO_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK0_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_AUDIO_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_AUDIO_EXT_REFCLK1_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		       CLK_AM62LX_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_GPMC0_CLKLB_OUT,
		       CLK_AM62LX_BOARD_0_GPMC0_CLKLB_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_I2C0_SCL_OUT,
		       CLK_AM62LX_BOARD_0_I2C0_SCL_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_I2C1_SCL_OUT,
		       CLK_AM62LX_BOARD_0_I2C1_SCL_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_I2C2_SCL_OUT,
		       CLK_AM62LX_BOARD_0_I2C2_SCL_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_I2C3_SCL_OUT,
		       CLK_AM62LX_BOARD_0_I2C3_SCL_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP0_ACLKR_OUT,
		       CLK_AM62LX_BOARD_0_MCASP0_ACLKR_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP0_ACLKX_OUT,
		       CLK_AM62LX_BOARD_0_MCASP0_ACLKX_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP0_AFSR_OUT,
		       CLK_AM62LX_BOARD_0_MCASP0_AFSR_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP0_AFSX_OUT,
		       CLK_AM62LX_BOARD_0_MCASP0_AFSX_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP1_ACLKR_OUT,
		       CLK_AM62LX_BOARD_0_MCASP1_ACLKR_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP1_ACLKX_OUT,
		       CLK_AM62LX_BOARD_0_MCASP1_ACLKX_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP1_AFSR_OUT,
		       CLK_AM62LX_BOARD_0_MCASP1_AFSR_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP1_AFSX_OUT,
		       CLK_AM62LX_BOARD_0_MCASP1_AFSX_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP2_ACLKR_OUT,
		       CLK_AM62LX_BOARD_0_MCASP2_ACLKR_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP2_ACLKX_OUT,
		       CLK_AM62LX_BOARD_0_MCASP2_ACLKX_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP2_AFSR_OUT,
		       CLK_AM62LX_BOARD_0_MCASP2_AFSR_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MCASP2_AFSX_OUT,
		       CLK_AM62LX_BOARD_0_MCASP2_AFSX_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MMC0_CLKLB_OUT,
		       CLK_AM62LX_BOARD_0_MMC0_CLKLB_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MMC0_CLK_OUT,
		       CLK_AM62LX_BOARD_0_MMC0_CLK_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MMC1_CLKLB_OUT,
		       CLK_AM62LX_BOARD_0_MMC1_CLKLB_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MMC1_CLK_OUT,
		       CLK_AM62LX_BOARD_0_MMC1_CLK_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MMC2_CLKLB_OUT,
		       CLK_AM62LX_BOARD_0_MMC2_CLKLB_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_MMC2_CLK_OUT,
		       CLK_AM62LX_BOARD_0_MMC2_CLK_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_OSPI0_DQS_OUT,
		       CLK_AM62LX_BOARD_0_OSPI0_DQS_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_OSPI0_LBCLKO_OUT,
		       CLK_AM62LX_BOARD_0_OSPI0_LBCLKO_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_RMII1_REF_CLK_OUT,
		       CLK_AM62LX_BOARD_0_RMII1_REF_CLK_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_RMII2_REF_CLK_OUT,
		       CLK_AM62LX_BOARD_0_RMII2_REF_CLK_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_TCK_OUT,
		       CLK_AM62LX_BOARD_0_TCK_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_VOUT0_EXTPCLKIN_OUT,
		       CLK_AM62LX_BOARD_0_VOUT0_EXTPCLKIN_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_WKUP_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT),
	DEV_CLK_OUTPUT(AM62LX_DEV_BOARD_0_CLOCKS,
		       AM62LX_DEV_BOARD0_WKUP_I2C0_SCL_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_I2C0_SCL_OUT),
	DEV_CLK_MUX(AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLOCKS,
		    AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLK,
		    CLK_AM62LX_WKUP_GTCCLK_SEL_OUT0, 1, 8),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLK_PARENT_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_MAIN_0_HSDIVOUT6_CLK, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLK_PARENT_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT,
		       CLK_AM62LX_BOARD_0_CP_GEMAC_CPTS0_RFT_CLK_OUT, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLK_PARENT_BOARD_0_WKUP_EXT_REFCLK0_OUT,
		       CLK_AM62LX_BOARD_0_WKUP_EXT_REFCLK0_OUT, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLK_PARENT_BOARD_0_EXT_REFCLK1_OUT,
		       CLK_AM62LX_BOARD_0_EXT_REFCLK1_OUT, 1, 5),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD_CLK_PARENT_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK2,
		       CLK_AM62LX_SAM62_PLL_CTRL_WRAP_WKUP_0_CHIP_DIV1_CLK_CLK, 2, 6),
	DEV_CLK_MUX(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		    AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK,
		    CLK_AM62LX_WKUP_OBSCLK_MUX_SEL_OUT0, 1, 16),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_RCOSC_CLKOUT,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK, 1, 10),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV4_16FFT_WKUP_0_HSDIVOUT4_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT4_CLK, 1, 3),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK_DUP0,
		       CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_RCOSC_CLK_1P0V_97P65K3,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K, 3, 5),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 6),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK8,
		       CLK_AM62LX_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK, 8, 7),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK,
		       CLK_AM62LX_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK, 1, 8),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD_CLK_PARENT_CLK_32K_RC_SEL_OUT0,
		       CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 9),
	DEV_CLK_MUX(AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS,
		    AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLK,
		    CLK_AM62LX_WKUP_CLKOUT_SEL_OUT0, 1, 8),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLK_PARENT_RTCSS_WKUP_0_OSC_32K_CLK,
		       CLK_AM62LX_RTCSS_WKUP_0_OSC_32K_CLK, 1, 1),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT7_CLK, 1, 2),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLK_PARENT_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK2,
		       CLK_AM62LX_HSDIV0_16FFT_WKUP_0_HSDIVOUT0_CLK, 2, 3),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLK_PARENT_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK,
		       CLK_AM62LX_POSTDIV4_16FF_WKUP_0_HSDIVOUT8_CLK, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLK_PARENT_CLK_32K_RC_SEL_OUT0,
		       CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 5),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_RCOSC_CLKOUT,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1, 6),
	DEV_CLK_PARENT(AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 7),
	DEV_CLK_MUX(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		    AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK,
		    CLK_AM62LX_MAIN_OBSCLK0_MUX_SEL_OUT0, 1, 16),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT0_CLK, 1, 0),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF1, 1, 10),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV0_16FFT_MAIN_17_HSDIVOUT0_CLK,
		       CLK_AM62LX_HSDIV0_16FFT_MAIN_17_HSDIVOUT0_CLK, 1, 11),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_RCOSC_CLK_1P0V_97P65K3,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K, 3, 12),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_A53_DIVH_CLK4_OBSCLK_OUT_CLK,
		       CLK_AM62LX_SAM62L_A53_256KB_WRAP_MAIN_0_ARM_COREPACK_0_A53_DIVH_CLK4_OBSCLK_OUT_CLK, 1, 3),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV4_16FFT_MAIN_0_HSDIVOUT2_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_MAIN_0_HSDIVOUT2_CLK, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_RCOSC_CLKOUT,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLKOUT, 1, 5),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK8,
		       CLK_AM62LX_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK, 8, 6),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK,
		       CLK_AM62LX_HSDIV4_16FFT_WKUP_0_HSDIVOUT0_CLK, 1, 7),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_HFOSC0_CLK,
		       CLK_AM62LX_GLUELOGIC_HFOSC0_CLK, 1, 8),
	DEV_CLK_PARENT(AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD_CLK_PARENT_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0,
		       CLK_AM62LX_CPSW_3GUSS_AM62L_MAIN_0_CPTS_GENF0, 1, 9),
	DEV_CLK_MUX(AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLOCKS,
		    AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLK,
		    CLK_AM62LX_CLK_32K_RC_SEL_OUT0, 1, 4),
	DEV_CLK_PARENT(AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_RCOSC_CLK_1P0V_97P65K3,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K, 3, 0),
	DEV_CLK_PARENT(AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLK_PARENT_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK8,
		       CLK_AM62LX_HSDIV0_16FFT_WKUP_32KHZ_GEN_0_HSDIVOUT0_CLK, 8, 1),
	DEV_CLK_PARENT(AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLK_PARENT_GLUELOGIC_RCOSC_CLK_1P0V_97P65K3_DUP0,
		       CLK_AM62LX_GLUELOGIC_RCOSC_CLK_1P0V_97P65K, 3, 2),
	DEV_CLK_PARENT(AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLOCKS,
		       AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD_CLK_PARENT_RTCSS_WKUP_0_OSC_32K_CLK,
		       CLK_AM62LX_RTCSS_WKUP_0_OSC_32K_CLK, 1, 3),
};
static struct dev_clk MAIN_dev_clk[609] __section(".bss.devgroup.MAIN");

const struct devgroup soc_devgroups[AM62LX_PM_DEVGRP_RANGE_ID_MAX] = {
	[PM_DEVGRP_00] = {
		.dev_clk_data = MAIN_dev_clk_data,
		.dev_clk = MAIN_dev_clk,
		.clk_idx = 1U,
	},
};
const size_t soc_devgroup_count = ARRAY_SIZE(soc_devgroups);

const struct soc_device_data * const soc_psc_multiple_domains[1] = {
};

const struct dev_data * const soc_device_data_arr[AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD + 1U] = {
	[AM62LX_DEV_ADC0] = &am62lx_dev_adc12_core_main_0,
	[AM62LX_DEV_MAIN_GPIOMUX_INTROUTER0] = &am62lx_dev_am62l_main_gpiomux_introuter_main_0,
	[AM62LX_DEV_TIMESYNC_INTROUTER0] = &am62lx_dev_am62l_timesync_introuter_main_0,
	[AM62LX_DEV_CPSW0] = &am62lx_dev_cpsw_3guss_am62l_main_0,
	[AM62LX_DEV_CPT2_AGGR0] = &am62lx_dev_cpt2_aggregator32_main_sysclk2,
	[AM62LX_DEV_CPT2_AGGR1] = &am62lx_dev_cpt2_aggregator32_per_sysclk2,
	[AM62LX_DEV_WKUP_CPT2_AGGR0] = &am62lx_dev_cpt2_aggregator32_wkup_sysclk2,
	[AM62LX_DEV_STM0] = &am62lx_dev_cxstm500ss_main_0,
	[AM62LX_DEV_DEBUGSS_WRAP0] = &am62lx_dev_debugss_k3_wrap_cv0_main_0,
	[AM62LX_DEV_DMASS0] = &am62lx_dev_dmss_am61_main_0,
	[AM62LX_DEV_DMASS0_BCDMA_0] = &am62lx_dev_dmss_am61_main_0_bcdma_0,
	[AM62LX_DEV_DMASS0_PKTDMA_0] = &am62lx_dev_dmss_am61_main_0_pktdma_0,
	[AM62LX_DEV_WKUP_DMASS0] = &am62lx_dev_dmss_crypto_am61_wkup_0,
	[AM62LX_DEV_WKUP_DMASS0_DTHE] = &am62lx_dev_dmss_crypto_am61_wkup_0_dthe,
	[AM62LX_DEV_WKUP_DMASS0_XLCDMA_0] = &am62lx_dev_dmss_crypto_am61_wkup_0_xlcdma_0,
	[AM62LX_DEV_TIMER0] = &am62lx_dev_dmtimer_dmc1ms_main_0,
	[AM62LX_DEV_TIMER1] = &am62lx_dev_dmtimer_dmc1ms_main_1,
	[AM62LX_DEV_TIMER2] = &am62lx_dev_dmtimer_dmc1ms_main_2,
	[AM62LX_DEV_TIMER3] = &am62lx_dev_dmtimer_dmc1ms_main_3,
	[AM62LX_DEV_WKUP_TIMER0] = &am62lx_dev_dmtimer_dmc1ms_wkup_0,
	[AM62LX_DEV_WKUP_TIMER1] = &am62lx_dev_dmtimer_dmc1ms_wkup_1,
	[AM62LX_DEV_WKUP_SMS_LITE0] = &am62lx_dev_sms_lite_wkup_0,
	[AM62LX_DEV_ECAP0] = &am62lx_dev_ecap_main_0,
	[AM62LX_DEV_ECAP1] = &am62lx_dev_ecap_main_1,
	[AM62LX_DEV_ECAP2] = &am62lx_dev_ecap_main_2,
	[AM62LX_DEV_ELM0] = &am62lx_dev_elm_main_0,
	[AM62LX_DEV_MMCSD1] = &am62lx_dev_emmcsd4ss_main_0,
	[AM62LX_DEV_MMCSD2] = &am62lx_dev_emmcsd4ss_main_1,
	[AM62LX_DEV_MMCSD0] = &am62lx_dev_emmcsd8ss_main_0,
	[AM62LX_DEV_EQEP0] = &am62lx_dev_eqep_t2_main_0,
	[AM62LX_DEV_EQEP1] = &am62lx_dev_eqep_t2_main_1,
	[AM62LX_DEV_EQEP2] = &am62lx_dev_eqep_t2_main_2,
	[AM62LX_DEV_FSS0] = &am62lx_dev_fss_ul_128_main_0,
	[AM62LX_DEV_GICSS0] = &am62lx_dev_gic500ss_1_2_spi960_main_0,
	[AM62LX_DEV_GPIO0] = &am62lx_dev_gpio_144_main_0,
	[AM62LX_DEV_GPIO2] = &am62lx_dev_gpio_144_main_2,
	[AM62LX_DEV_WKUP_GPIO0] = &am62lx_dev_gpio_144_wkup_0,
	[AM62LX_DEV_GPMC0] = &am62lx_dev_gpmc_main_0,
	[AM62LX_DEV_DSS_DSI0] = &am62lx_dev_k3_dss_dsi_main_0,
	[AM62LX_DEV_DSS0] = &am62lx_dev_k3_dss_nano_main_0,
	[AM62LX_DEV_EPWM0] = &am62lx_dev_k3_epwm_main_0,
	[AM62LX_DEV_EPWM1] = &am62lx_dev_k3_epwm_main_1,
	[AM62LX_DEV_EPWM2] = &am62lx_dev_k3_epwm_main_2,
	[AM62LX_DEV_LED0] = &am62lx_dev_k3_led2vbus_main_0,
	[AM62LX_DEV_PBIST0] = &am62lx_dev_k3_pbist_8c28p_4bit_wrap_main_0,
	[AM62LX_DEV_WKUP_PBIST0] = &am62lx_dev_k3_pbist_8c28p_4bit_wrap_wkup_0,
	[AM62LX_DEV_WKUP_VTM0] = &am62lx_dev_k3vtm_n16ffc_wkup_0,
	[AM62LX_DEV_MCAN0] = &am62lx_dev_mcanss_main_0,
	[AM62LX_DEV_MCAN1] = &am62lx_dev_mcanss_main_1,
	[AM62LX_DEV_MCAN2] = &am62lx_dev_mcanss_main_2,
	[AM62LX_DEV_MCASP0] = &am62lx_dev_mcasp_main_0,
	[AM62LX_DEV_MCASP1] = &am62lx_dev_mcasp_main_1,
	[AM62LX_DEV_MCASP2] = &am62lx_dev_mcasp_main_2,
	[AM62LX_DEV_I2C0] = &am62lx_dev_mshsi2c_main_0,
	[AM62LX_DEV_I2C1] = &am62lx_dev_mshsi2c_main_1,
	[AM62LX_DEV_I2C2] = &am62lx_dev_mshsi2c_main_2,
	[AM62LX_DEV_I2C3] = &am62lx_dev_mshsi2c_main_3,
	[AM62LX_DEV_WKUP_I2C0] = &am62lx_dev_mshsi2c_wkup_0,
	[AM62LX_DEV_WKUP_GTC0] = &am62lx_dev_gtc_r10_wkup_0,
	[AM62LX_DEV_WKUP_RTCSS0] = &am62lx_dev_rtcss_wkup_0,
	[AM62LX_DEV_RTI0] = &am62lx_dev_rti_cfg1_main_a53_0,
	[AM62LX_DEV_RTI1] = &am62lx_dev_rti_cfg1_main_a53_1,
	[AM62LX_DEV_DEBUGSS0] = &am62lx_dev_sam61_debug_main_cell_main_0,
	[AM62LX_DEV_MSRAM_96K0] = &am62lx_dev_sam61_msram6kx128_main_0,
	[AM62LX_DEV_WKUP_PSRAM_64K0] = &am62lx_dev_sam61_psram16kx32_wkup_0,
	[AM62LX_DEV_ROM0] = &am62lx_dev_sam61_psrom64kx32_main_0,
	[AM62LX_DEV_PSC0] = &am62lx_dev_sam61_wkup_psc_wrap_wkup_0.drv_data.dev_data,
	[AM62LX_DEV_WKUP_DEEPSLEEP_SOURCES0] = &am62lx_dev_sam62_dm_wakeup_deepsleep_sources_wkup_0,
	[AM62LX_DEV_MCU_MCU_16FF0] = &am62lx_dev_sam62a_mcu_16ff_mcu_0,
	[AM62LX_DEV_COMPUTE_CLUSTER0_ARM_COREPACK_0] =
	&am62lx_dev_sam62l_a53_256kb_wrap_main_0_arm_corepack_0,
	[AM62LX_DEV_DDR16SS0] = &am62lx_dev_sam62l_ddr_wrap_main_0,
	[AM62LX_DEV_WKUP_DFTSS0] = &am62lx_dev_sam62l_dftss_wrap_wkup_0,
	[AM62LX_DEV_MCSPI0] = &am62lx_dev_spi_main_0,
	[AM62LX_DEV_MCSPI1] = &am62lx_dev_spi_main_1,
	[AM62LX_DEV_MCSPI2] = &am62lx_dev_spi_main_2,
	[AM62LX_DEV_MCSPI3] = &am62lx_dev_spi_main_3,
	[AM62LX_DEV_TRNG_DRBG_EIP76D_WRAP0] = &am62lx_dev_trng_drbg_eip76d_wrap_main_0,
	[AM62LX_DEV_UART1] = &am62lx_dev_usart_main_1,
	[AM62LX_DEV_UART2] = &am62lx_dev_usart_main_2,
	[AM62LX_DEV_UART3] = &am62lx_dev_usart_main_3,
	[AM62LX_DEV_UART4] = &am62lx_dev_usart_main_4,
	[AM62LX_DEV_UART5] = &am62lx_dev_usart_main_5,
	[AM62LX_DEV_UART6] = &am62lx_dev_usart_main_6,
	[AM62LX_DEV_WKUP_UART0] = &am62lx_dev_usart_wkup_0,
	[AM62LX_DEV_USB0] = &am62lx_dev_usb2ss_16ffc_main_0,
	[AM62LX_DEV_USB1] = &am62lx_dev_usb2ss_16ffc_main_1,
	[AM62LX_DEV_DPHY_TX0] = &am62lx_dev_wiz16b8m4cdt3_main_0,
	[AM62LX_DEV_COMPUTE_CLUSTER0_CLKDIV_0] = &am62lx_dev_sam62l_a53_256kb_wrap_main_0_clkdiv_0,
	[AM62LX_DEV_COMPUTE_CLUSTER0_PBIST_0] = &am62lx_dev_sam62l_a53_256kb_wrap_main_0_pbist_0,
	[AM62LX_DEV_COMPUTE_CLUSTER0] = &am62lx_dev_sam62l_a53_256kb_wrap_main_0,
	[AM62LX_DEV_COMPUTE_CLUSTER0_A53_0] = &am62lx_dev_sam62l_a53_256kb_wrap_main_0_a53_0,
	[AM62LX_DEV_COMPUTE_CLUSTER0_A53_1] = &am62lx_dev_sam62l_a53_256kb_wrap_main_0_a53_1,
	[AM62LX_DEV_UART0] = &am62lx_dev_usart_main_0,
	[AM62LX_DEV_BOARD0] = &am62lx_dev_board_0,
	[AM62LX_DEV_WKUP_GTCCLK_SEL_DEV_VD] = &am62lx_dev_wkup_gtcclk_sel_dev_VD,
	[AM62LX_DEV_WKUP_OBSCLK_MUX_SEL_DEV_VD] = &am62lx_dev_wkup_obsclk_mux_sel_dev_VD,
	[AM62LX_DEV_WKUP_CLKOUT_SEL_DEV_VD] = &am62lx_dev_wkup_clkout_sel_dev_VD,
	[AM62LX_DEV_OBSCLK0_MUX_SEL_DEV_VD] = &am62lx_dev_obsclk0_mux_sel_dev_VD,
	[AM62LX_DEV_MAIN_USB0_ISO_VD] = &am62lx_dev_main_usb0_iso_VD,
	[AM62LX_DEV_MAIN_USB1_ISO_VD] = &am62lx_dev_main_usb1_iso_VD,
	[AM62LX_DEV_EMIF_CFG_ISO_VD] = &am62lx_dev_emif_cfg_iso_VD,
	[AM62LX_DEV_EMIF_DATA_ISO_VD] = &am62lx_dev_emif_data_iso_VD,
	[AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD] = &am62lx_dev_clk_32k_rc_sel_dev_VD,
};

struct device soc_devices[AM62LX_DEV_CLK_32K_RC_SEL_DEV_VD + 1U];
const size_t soc_device_count = ARRAY_SIZE(soc_device_data_arr);

struct device * const this_dev = soc_devices;
