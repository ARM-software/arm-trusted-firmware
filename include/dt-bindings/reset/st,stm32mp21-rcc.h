/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/*
 * Copyright (C) STMicroelectronics 2025 - All Rights Reserved
 * Author(s): Gabriel Fernandez <gabriel.fernandez@foss.st.com>
 */

#ifndef _DT_BINDINGS_STM32MP21_RESET_H_
#define _DT_BINDINGS_STM32MP21_RESET_H_

/* TF-A use a binding required by driver, not aligned with Linux*/

#define SYS_R		8192
#define C1_R		8224
#define C2_R		8288
#define C2_HOLDBOOT_R	8608
#define C1_HOLDBOOT_R	8609
#define VSW_R		8672
#define C1MS_R		8840
#define IWDG2_KER_R	9106
#define IWDG4_KER_R	9234
#define DDRCP_R		9888
#define DDRCAPB_R	9920
#define DDRPHYCAPB_R	9952
#define DDRCFG_R	10016
#define DDR_R		10048
#define OSPI1_R		10400
#define OSPI1DLL_R	10416
#define FMC_R		10464
#define DBG_R		10508
#define GPIOA_R		10592
#define GPIOB_R		10624
#define GPIOC_R		10656
#define GPIOD_R		10688
#define GPIOE_R		10720
#define GPIOF_R		10752
#define GPIOG_R		10784
#define GPIOH_R		10816
#define GPIOI_R		10848
#define GPIOZ_R		10944
#define HPDMA1_R	10976
#define HPDMA2_R	11008
#define HPDMA3_R	11040
#define IPCC1_R		11136
#define SSMOD_R		11392
#define TIM1_R		14336
#define TIM2_R		14368
#define TIM3_R		14400
#define TIM4_R		14432
#define TIM5_R		14464
#define TIM6_R		14496
#define TIM7_R		14528
#define TIM8_R		14560
#define TIM10_R		14592
#define TIM11_R		14624
#define TIM12_R		14656
#define TIM13_R		14688
#define TIM14_R		14720
#define TIM15_R		14752
#define TIM16_R		14784
#define TIM17_R		14816
#define LPTIM1_R	14880
#define LPTIM2_R	14912
#define LPTIM3_R	14944
#define LPTIM4_R	14976
#define LPTIM5_R	15008
#define SPI1_R		15040
#define SPI2_R		15072
#define SPI3_R		15104
#define SPI4_R		15136
#define SPI5_R		15168
#define SPI6_R		15200
#define SPDIFRX_R	15296
#define USART1_R	15328
#define USART2_R	15360
#define USART3_R	15392
#define UART4_R		15424
#define UART5_R		15456
#define USART6_R	15488
#define UART7_R		15520
#define LPUART1_R	15616
#define I2C1_R		15648
#define I2C2_R		15680
#define I2C3_R		15712
#define SAI1_R		15904
#define SAI2_R		15936
#define SAI3_R		15968
#define SAI4_R		16000
#define MDF1_R		16064
#define ADF1_R		16096
#define FDCAN_R		16128
#define HDP_R		16160
#define ADC1_R		16192
#define ADC2_R		16224
#define ETH1_R		16256
#define ETH2_R		16288
#define USBH_R		16352
#define USB2PHY1_R	16384
#define OTG_R		16448
#define USB2PHY2_R	16480
#define SDMMC1_R	16768
#define SDMMC1DLL_R	16784
#define SDMMC2_R	16800
#define SDMMC2DLL_R	16816
#define SDMMC3_R	16832
#define SDMMC3DLL_R	16848
#define LTDC_R		16896
#define CSI_R		17088
#define DCMIPP_R	17120
#define DCMIPSSI_R	17152
#define RNG1_R		17280
#define RNG2_R		17312
#define PKA_R		17344
#define SAES_R		17376
#define HASH1_R		17408
#define HASH2_R		17440
#define CRYP1_R		17472
#define CRYP2_R		17504
#define WWDG1_R		17696
#define VREF_R		17760
#define DTS_R		17792
#define CRC_R		17824
#define SERC_R		17856
#define I3C1_R		17984
#define I3C2_R		18016
#define I3C3_R		18048

#define RST_SCMI_C1_R		0
#define RST_SCMI_C2_R		1
#define RST_SCMI_C1_HOLDBOOT_R	2
#define RST_SCMI_C2_HOLDBOOT_R	3
#define RST_SCMI_FMC		4
#define RST_SCMI_OSPI1		5
#define RST_SCMI_OSPI1DLL	6

#endif /* _DT_BINDINGS_STM32MP21_RESET_H_ */
