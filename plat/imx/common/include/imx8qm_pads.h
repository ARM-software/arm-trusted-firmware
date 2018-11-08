/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * Header file used to configure SoC pad list.
 */

#ifndef IMX8QM_PADS_H
#define IMX8QM_PADS_H

/* Includes */

/* Defines */

/*!
 * @name Pad Definitions
 */
/*@{*/
#define SC_P_SIM0_CLK                            0	/* DMA.SIM0.CLK, LSIO.GPIO0.IO00 */
#define SC_P_SIM0_RST                            1	/* DMA.SIM0.RST, LSIO.GPIO0.IO01 */
#define SC_P_SIM0_IO                             2	/* DMA.SIM0.IO, LSIO.GPIO0.IO02 */
#define SC_P_SIM0_PD                             3	/* DMA.SIM0.PD, DMA.I2C3.SCL, LSIO.GPIO0.IO03 */
#define SC_P_SIM0_POWER_EN                       4	/* DMA.SIM0.POWER_EN, DMA.I2C3.SDA, LSIO.GPIO0.IO04 */
#define SC_P_SIM0_GPIO0_00                       5	/* DMA.SIM0.POWER_EN, LSIO.GPIO0.IO05 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_SIM           6	/*  */
#define SC_P_M40_I2C0_SCL                        7	/* M40.I2C0.SCL, M40.UART0.RX, M40.GPIO0.IO02, LSIO.GPIO0.IO06 */
#define SC_P_M40_I2C0_SDA                        8	/* M40.I2C0.SDA, M40.UART0.TX, M40.GPIO0.IO03, LSIO.GPIO0.IO07 */
#define SC_P_M40_GPIO0_00                        9	/* M40.GPIO0.IO00, M40.TPM0.CH0, DMA.UART4.RX, LSIO.GPIO0.IO08 */
#define SC_P_M40_GPIO0_01                        10	/* M40.GPIO0.IO01, M40.TPM0.CH1, DMA.UART4.TX, LSIO.GPIO0.IO09 */
#define SC_P_M41_I2C0_SCL                        11	/* M41.I2C0.SCL, M41.UART0.RX, M41.GPIO0.IO02, LSIO.GPIO0.IO10 */
#define SC_P_M41_I2C0_SDA                        12	/* M41.I2C0.SDA, M41.UART0.TX, M41.GPIO0.IO03, LSIO.GPIO0.IO11 */
#define SC_P_M41_GPIO0_00                        13	/* M41.GPIO0.IO00, M41.TPM0.CH0, DMA.UART3.RX, LSIO.GPIO0.IO12 */
#define SC_P_M41_GPIO0_01                        14	/* M41.GPIO0.IO01, M41.TPM0.CH1, DMA.UART3.TX, LSIO.GPIO0.IO13 */
#define SC_P_GPT0_CLK                            15	/* LSIO.GPT0.CLK, DMA.I2C1.SCL, LSIO.KPP0.COL4, LSIO.GPIO0.IO14 */
#define SC_P_GPT0_CAPTURE                        16	/* LSIO.GPT0.CAPTURE, DMA.I2C1.SDA, LSIO.KPP0.COL5, LSIO.GPIO0.IO15 */
#define SC_P_GPT0_COMPARE                        17	/* LSIO.GPT0.COMPARE, LSIO.PWM3.OUT, LSIO.KPP0.COL6, LSIO.GPIO0.IO16 */
#define SC_P_GPT1_CLK                            18	/* LSIO.GPT1.CLK, DMA.I2C2.SCL, LSIO.KPP0.COL7, LSIO.GPIO0.IO17 */
#define SC_P_GPT1_CAPTURE                        19	/* LSIO.GPT1.CAPTURE, DMA.I2C2.SDA, LSIO.KPP0.ROW4, LSIO.GPIO0.IO18 */
#define SC_P_GPT1_COMPARE                        20	/* LSIO.GPT1.COMPARE, LSIO.PWM2.OUT, LSIO.KPP0.ROW5, LSIO.GPIO0.IO19 */
#define SC_P_UART0_RX                            21	/* DMA.UART0.RX, SCU.UART0.RX, LSIO.GPIO0.IO20 */
#define SC_P_UART0_TX                            22	/* DMA.UART0.TX, SCU.UART0.TX, LSIO.GPIO0.IO21 */
#define SC_P_UART0_RTS_B                         23	/* DMA.UART0.RTS_B, LSIO.PWM0.OUT, DMA.UART2.RX, LSIO.GPIO0.IO22 */
#define SC_P_UART0_CTS_B                         24	/* DMA.UART0.CTS_B, LSIO.PWM1.OUT, DMA.UART2.TX, LSIO.GPIO0.IO23 */
#define SC_P_UART1_TX                            25	/* DMA.UART1.TX, DMA.SPI3.SCK, LSIO.GPIO0.IO24 */
#define SC_P_UART1_RX                            26	/* DMA.UART1.RX, DMA.SPI3.SDO, LSIO.GPIO0.IO25 */
#define SC_P_UART1_RTS_B                         27	/* DMA.UART1.RTS_B, DMA.SPI3.SDI, DMA.UART1.CTS_B, LSIO.GPIO0.IO26 */
#define SC_P_UART1_CTS_B                         28	/* DMA.UART1.CTS_B, DMA.SPI3.CS0, DMA.UART1.RTS_B, LSIO.GPIO0.IO27 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOLH        29	/*  */
#define SC_P_SCU_PMIC_MEMC_ON                    30	/* SCU.GPIO0.IOXX_PMIC_MEMC_ON */
#define SC_P_SCU_WDOG_OUT                        31	/* SCU.WDOG0.WDOG_OUT */
#define SC_P_PMIC_I2C_SDA                        32	/* SCU.PMIC_I2C.SDA */
#define SC_P_PMIC_I2C_SCL                        33	/* SCU.PMIC_I2C.SCL */
#define SC_P_PMIC_EARLY_WARNING                  34	/* SCU.PMIC_EARLY_WARNING */
#define SC_P_PMIC_INT_B                          35	/* SCU.DSC.PMIC_INT_B */
#define SC_P_SCU_GPIO0_00                        36	/* SCU.GPIO0.IO00, SCU.UART0.RX, LSIO.GPIO0.IO28 */
#define SC_P_SCU_GPIO0_01                        37	/* SCU.GPIO0.IO01, SCU.UART0.TX, LSIO.GPIO0.IO29 */
#define SC_P_SCU_GPIO0_02                        38	/* SCU.GPIO0.IO02, SCU.GPIO0.IOXX_PMIC_GPU0_ON, LSIO.GPIO0.IO30 */
#define SC_P_SCU_GPIO0_03                        39	/* SCU.GPIO0.IO03, SCU.GPIO0.IOXX_PMIC_GPU1_ON, LSIO.GPIO0.IO31 */
#define SC_P_SCU_GPIO0_04                        40	/* SCU.GPIO0.IO04, SCU.GPIO0.IOXX_PMIC_A72_ON, LSIO.GPIO1.IO00 */
#define SC_P_SCU_GPIO0_05                        41	/* SCU.GPIO0.IO05, SCU.GPIO0.IOXX_PMIC_A53_ON, LSIO.GPIO1.IO01 */
#define SC_P_SCU_GPIO0_06                        42	/* SCU.GPIO0.IO06, SCU.TPM0.CH0, LSIO.GPIO1.IO02 */
#define SC_P_SCU_GPIO0_07                        43	/* SCU.GPIO0.IO07, SCU.TPM0.CH1, SCU.DSC.RTC_CLOCK_OUTPUT_32K, LSIO.GPIO1.IO03 */
#define SC_P_SCU_BOOT_MODE0                      44	/* SCU.DSC.BOOT_MODE0 */
#define SC_P_SCU_BOOT_MODE1                      45	/* SCU.DSC.BOOT_MODE1 */
#define SC_P_SCU_BOOT_MODE2                      46	/* SCU.DSC.BOOT_MODE2 */
#define SC_P_SCU_BOOT_MODE3                      47	/* SCU.DSC.BOOT_MODE3 */
#define SC_P_SCU_BOOT_MODE4                      48	/* SCU.DSC.BOOT_MODE4, SCU.PMIC_I2C.SCL */
#define SC_P_SCU_BOOT_MODE5                      49	/* SCU.DSC.BOOT_MODE5, SCU.PMIC_I2C.SDA */
#define SC_P_LVDS0_GPIO00                        50	/* LVDS0.GPIO0.IO00, LVDS0.PWM0.OUT, LSIO.GPIO1.IO04 */
#define SC_P_LVDS0_GPIO01                        51	/* LVDS0.GPIO0.IO01, LSIO.GPIO1.IO05 */
#define SC_P_LVDS0_I2C0_SCL                      52	/* LVDS0.I2C0.SCL, LVDS0.GPIO0.IO02, LSIO.GPIO1.IO06 */
#define SC_P_LVDS0_I2C0_SDA                      53	/* LVDS0.I2C0.SDA, LVDS0.GPIO0.IO03, LSIO.GPIO1.IO07 */
#define SC_P_LVDS0_I2C1_SCL                      54	/* LVDS0.I2C1.SCL, DMA.UART2.TX, LSIO.GPIO1.IO08 */
#define SC_P_LVDS0_I2C1_SDA                      55	/* LVDS0.I2C1.SDA, DMA.UART2.RX, LSIO.GPIO1.IO09 */
#define SC_P_LVDS1_GPIO00                        56	/* LVDS1.GPIO0.IO00, LVDS1.PWM0.OUT, LSIO.GPIO1.IO10 */
#define SC_P_LVDS1_GPIO01                        57	/* LVDS1.GPIO0.IO01, LSIO.GPIO1.IO11 */
#define SC_P_LVDS1_I2C0_SCL                      58	/* LVDS1.I2C0.SCL, LVDS1.GPIO0.IO02, LSIO.GPIO1.IO12 */
#define SC_P_LVDS1_I2C0_SDA                      59	/* LVDS1.I2C0.SDA, LVDS1.GPIO0.IO03, LSIO.GPIO1.IO13 */
#define SC_P_LVDS1_I2C1_SCL                      60	/* LVDS1.I2C1.SCL, DMA.UART3.TX, LSIO.GPIO1.IO14 */
#define SC_P_LVDS1_I2C1_SDA                      61	/* LVDS1.I2C1.SDA, DMA.UART3.RX, LSIO.GPIO1.IO15 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_LVDSGPIO      62	/*  */
#define SC_P_MIPI_DSI0_I2C0_SCL                  63	/* MIPI_DSI0.I2C0.SCL, LSIO.GPIO1.IO16 */
#define SC_P_MIPI_DSI0_I2C0_SDA                  64	/* MIPI_DSI0.I2C0.SDA, LSIO.GPIO1.IO17 */
#define SC_P_MIPI_DSI0_GPIO0_00                  65	/* MIPI_DSI0.GPIO0.IO00, MIPI_DSI0.PWM0.OUT, LSIO.GPIO1.IO18 */
#define SC_P_MIPI_DSI0_GPIO0_01                  66	/* MIPI_DSI0.GPIO0.IO01, LSIO.GPIO1.IO19 */
#define SC_P_MIPI_DSI1_I2C0_SCL                  67	/* MIPI_DSI1.I2C0.SCL, LSIO.GPIO1.IO20 */
#define SC_P_MIPI_DSI1_I2C0_SDA                  68	/* MIPI_DSI1.I2C0.SDA, LSIO.GPIO1.IO21 */
#define SC_P_MIPI_DSI1_GPIO0_00                  69	/* MIPI_DSI1.GPIO0.IO00, MIPI_DSI1.PWM0.OUT, LSIO.GPIO1.IO22 */
#define SC_P_MIPI_DSI1_GPIO0_01                  70	/* MIPI_DSI1.GPIO0.IO01, LSIO.GPIO1.IO23 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_MIPIDSIGPIO   71	/*  */
#define SC_P_MIPI_CSI0_MCLK_OUT                  72	/* MIPI_CSI0.ACM.MCLK_OUT, LSIO.GPIO1.IO24 */
#define SC_P_MIPI_CSI0_I2C0_SCL                  73	/* MIPI_CSI0.I2C0.SCL, LSIO.GPIO1.IO25 */
#define SC_P_MIPI_CSI0_I2C0_SDA                  74	/* MIPI_CSI0.I2C0.SDA, LSIO.GPIO1.IO26 */
#define SC_P_MIPI_CSI0_GPIO0_00                  75	/* MIPI_CSI0.GPIO0.IO00, DMA.I2C0.SCL, MIPI_CSI1.I2C0.SCL, LSIO.GPIO1.IO27 */
#define SC_P_MIPI_CSI0_GPIO0_01                  76	/* MIPI_CSI0.GPIO0.IO01, DMA.I2C0.SDA, MIPI_CSI1.I2C0.SDA, LSIO.GPIO1.IO28 */
#define SC_P_MIPI_CSI1_MCLK_OUT                  77	/* MIPI_CSI1.ACM.MCLK_OUT, LSIO.GPIO1.IO29 */
#define SC_P_MIPI_CSI1_GPIO0_00                  78	/* MIPI_CSI1.GPIO0.IO00, DMA.UART4.RX, LSIO.GPIO1.IO30 */
#define SC_P_MIPI_CSI1_GPIO0_01                  79	/* MIPI_CSI1.GPIO0.IO01, DMA.UART4.TX, LSIO.GPIO1.IO31 */
#define SC_P_MIPI_CSI1_I2C0_SCL                  80	/* MIPI_CSI1.I2C0.SCL, LSIO.GPIO2.IO00 */
#define SC_P_MIPI_CSI1_I2C0_SDA                  81	/* MIPI_CSI1.I2C0.SDA, LSIO.GPIO2.IO01 */
#define SC_P_HDMI_TX0_TS_SCL                     82	/* HDMI_TX0.I2C0.SCL, DMA.I2C0.SCL, LSIO.GPIO2.IO02 */
#define SC_P_HDMI_TX0_TS_SDA                     83	/* HDMI_TX0.I2C0.SDA, DMA.I2C0.SDA, LSIO.GPIO2.IO03 */
#define SC_P_COMP_CTL_GPIO_3V3_HDMIGPIO          84	/*  */
#define SC_P_ESAI1_FSR                           85	/* AUD.ESAI1.FSR, LSIO.GPIO2.IO04 */
#define SC_P_ESAI1_FST                           86	/* AUD.ESAI1.FST, AUD.SPDIF0.EXT_CLK, LSIO.GPIO2.IO05 */
#define SC_P_ESAI1_SCKR                          87	/* AUD.ESAI1.SCKR, LSIO.GPIO2.IO06 */
#define SC_P_ESAI1_SCKT                          88	/* AUD.ESAI1.SCKT, AUD.SAI2.RXC, AUD.SPDIF0.EXT_CLK, LSIO.GPIO2.IO07 */
#define SC_P_ESAI1_TX0                           89	/* AUD.ESAI1.TX0, AUD.SAI2.RXD, AUD.SPDIF0.RX, LSIO.GPIO2.IO08 */
#define SC_P_ESAI1_TX1                           90	/* AUD.ESAI1.TX1, AUD.SAI2.RXFS, AUD.SPDIF0.TX, LSIO.GPIO2.IO09 */
#define SC_P_ESAI1_TX2_RX3                       91	/* AUD.ESAI1.TX2_RX3, AUD.SPDIF0.RX, LSIO.GPIO2.IO10 */
#define SC_P_ESAI1_TX3_RX2                       92	/* AUD.ESAI1.TX3_RX2, AUD.SPDIF0.TX, LSIO.GPIO2.IO11 */
#define SC_P_ESAI1_TX4_RX1                       93	/* AUD.ESAI1.TX4_RX1, LSIO.GPIO2.IO12 */
#define SC_P_ESAI1_TX5_RX0                       94	/* AUD.ESAI1.TX5_RX0, LSIO.GPIO2.IO13 */
#define SC_P_SPDIF0_RX                           95	/* AUD.SPDIF0.RX, AUD.MQS.R, AUD.ACM.MCLK_IN1, LSIO.GPIO2.IO14 */
#define SC_P_SPDIF0_TX                           96	/* AUD.SPDIF0.TX, AUD.MQS.L, AUD.ACM.MCLK_OUT1, LSIO.GPIO2.IO15 */
#define SC_P_SPDIF0_EXT_CLK                      97	/* AUD.SPDIF0.EXT_CLK, DMA.DMA0.REQ_IN0, LSIO.GPIO2.IO16 */
#define SC_P_SPI3_SCK                            98	/* DMA.SPI3.SCK, LSIO.GPIO2.IO17 */
#define SC_P_SPI3_SDO                            99	/* DMA.SPI3.SDO, DMA.FTM.CH0, LSIO.GPIO2.IO18 */
#define SC_P_SPI3_SDI                            100	/* DMA.SPI3.SDI, DMA.FTM.CH1, LSIO.GPIO2.IO19 */
#define SC_P_SPI3_CS0                            101	/* DMA.SPI3.CS0, DMA.FTM.CH2, LSIO.GPIO2.IO20 */
#define SC_P_SPI3_CS1                            102	/* DMA.SPI3.CS1, LSIO.GPIO2.IO21 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHB       103	/*  */
#define SC_P_ESAI0_FSR                           104	/* AUD.ESAI0.FSR, LSIO.GPIO2.IO22 */
#define SC_P_ESAI0_FST                           105	/* AUD.ESAI0.FST, LSIO.GPIO2.IO23 */
#define SC_P_ESAI0_SCKR                          106	/* AUD.ESAI0.SCKR, LSIO.GPIO2.IO24 */
#define SC_P_ESAI0_SCKT                          107	/* AUD.ESAI0.SCKT, LSIO.GPIO2.IO25 */
#define SC_P_ESAI0_TX0                           108	/* AUD.ESAI0.TX0, LSIO.GPIO2.IO26 */
#define SC_P_ESAI0_TX1                           109	/* AUD.ESAI0.TX1, LSIO.GPIO2.IO27 */
#define SC_P_ESAI0_TX2_RX3                       110	/* AUD.ESAI0.TX2_RX3, LSIO.GPIO2.IO28 */
#define SC_P_ESAI0_TX3_RX2                       111	/* AUD.ESAI0.TX3_RX2, LSIO.GPIO2.IO29 */
#define SC_P_ESAI0_TX4_RX1                       112	/* AUD.ESAI0.TX4_RX1, LSIO.GPIO2.IO30 */
#define SC_P_ESAI0_TX5_RX0                       113	/* AUD.ESAI0.TX5_RX0, LSIO.GPIO2.IO31 */
#define SC_P_MCLK_IN0                            114	/* AUD.ACM.MCLK_IN0, AUD.ESAI0.RX_HF_CLK, AUD.ESAI1.RX_HF_CLK, LSIO.GPIO3.IO00 */
#define SC_P_MCLK_OUT0                           115	/* AUD.ACM.MCLK_OUT0, AUD.ESAI0.TX_HF_CLK, AUD.ESAI1.TX_HF_CLK, LSIO.GPIO3.IO01 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHC       116	/*  */
#define SC_P_SPI0_SCK                            117	/* DMA.SPI0.SCK, AUD.SAI0.RXC, LSIO.GPIO3.IO02 */
#define SC_P_SPI0_SDO                            118	/* DMA.SPI0.SDO, AUD.SAI0.TXD, LSIO.GPIO3.IO03 */
#define SC_P_SPI0_SDI                            119	/* DMA.SPI0.SDI, AUD.SAI0.RXD, LSIO.GPIO3.IO04 */
#define SC_P_SPI0_CS0                            120	/* DMA.SPI0.CS0, AUD.SAI0.RXFS, LSIO.GPIO3.IO05 */
#define SC_P_SPI0_CS1                            121	/* DMA.SPI0.CS1, AUD.SAI0.TXC, LSIO.GPIO3.IO06 */
#define SC_P_SPI2_SCK                            122	/* DMA.SPI2.SCK, LSIO.GPIO3.IO07 */
#define SC_P_SPI2_SDO                            123	/* DMA.SPI2.SDO, LSIO.GPIO3.IO08 */
#define SC_P_SPI2_SDI                            124	/* DMA.SPI2.SDI, LSIO.GPIO3.IO09 */
#define SC_P_SPI2_CS0                            125	/* DMA.SPI2.CS0, LSIO.GPIO3.IO10 */
#define SC_P_SPI2_CS1                            126	/* DMA.SPI2.CS1, AUD.SAI0.TXFS, LSIO.GPIO3.IO11 */
#define SC_P_SAI1_RXC                            127	/* AUD.SAI1.RXC, AUD.SAI0.TXD, LSIO.GPIO3.IO12 */
#define SC_P_SAI1_RXD                            128	/* AUD.SAI1.RXD, AUD.SAI0.TXFS, LSIO.GPIO3.IO13 */
#define SC_P_SAI1_RXFS                           129	/* AUD.SAI1.RXFS, AUD.SAI0.RXD, LSIO.GPIO3.IO14 */
#define SC_P_SAI1_TXC                            130	/* AUD.SAI1.TXC, AUD.SAI0.TXC, LSIO.GPIO3.IO15 */
#define SC_P_SAI1_TXD                            131	/* AUD.SAI1.TXD, AUD.SAI1.RXC, LSIO.GPIO3.IO16 */
#define SC_P_SAI1_TXFS                           132	/* AUD.SAI1.TXFS, AUD.SAI1.RXFS, LSIO.GPIO3.IO17 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIORHT       133	/*  */
#define SC_P_ADC_IN7                             134	/* DMA.ADC1.IN3, DMA.SPI1.CS1, LSIO.KPP0.ROW3, LSIO.GPIO3.IO25 */
#define SC_P_ADC_IN6                             135	/* DMA.ADC1.IN2, DMA.SPI1.CS0, LSIO.KPP0.ROW2, LSIO.GPIO3.IO24 */
#define SC_P_ADC_IN5                             136	/* DMA.ADC1.IN1, DMA.SPI1.SDI, LSIO.KPP0.ROW1, LSIO.GPIO3.IO23 */
#define SC_P_ADC_IN4                             137	/* DMA.ADC1.IN0, DMA.SPI1.SDO, LSIO.KPP0.ROW0, LSIO.GPIO3.IO22 */
#define SC_P_ADC_IN3                             138	/* DMA.ADC0.IN3, DMA.SPI1.SCK, LSIO.KPP0.COL3, LSIO.GPIO3.IO21 */
#define SC_P_ADC_IN2                             139	/* DMA.ADC0.IN2, LSIO.KPP0.COL2, LSIO.GPIO3.IO20 */
#define SC_P_ADC_IN1                             140	/* DMA.ADC0.IN1, LSIO.KPP0.COL1, LSIO.GPIO3.IO19 */
#define SC_P_ADC_IN0                             141	/* DMA.ADC0.IN0, LSIO.KPP0.COL0, LSIO.GPIO3.IO18 */
#define SC_P_MLB_SIG                             142	/* CONN.MLB.SIG, AUD.SAI3.RXC, LSIO.GPIO3.IO26 */
#define SC_P_MLB_CLK                             143	/* CONN.MLB.CLK, AUD.SAI3.RXFS, LSIO.GPIO3.IO27 */
#define SC_P_MLB_DATA                            144	/* CONN.MLB.DATA, AUD.SAI3.RXD, LSIO.GPIO3.IO28 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOLHT       145	/*  */
#define SC_P_FLEXCAN0_RX                         146	/* DMA.FLEXCAN0.RX, LSIO.GPIO3.IO29 */
#define SC_P_FLEXCAN0_TX                         147	/* DMA.FLEXCAN0.TX, LSIO.GPIO3.IO30 */
#define SC_P_FLEXCAN1_RX                         148	/* DMA.FLEXCAN1.RX, LSIO.GPIO3.IO31 */
#define SC_P_FLEXCAN1_TX                         149	/* DMA.FLEXCAN1.TX, LSIO.GPIO4.IO00 */
#define SC_P_FLEXCAN2_RX                         150	/* DMA.FLEXCAN2.RX, LSIO.GPIO4.IO01 */
#define SC_P_FLEXCAN2_TX                         151	/* DMA.FLEXCAN2.TX, LSIO.GPIO4.IO02 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOTHR       152	/*  */
#define SC_P_USB_SS3_TC0                         153	/* DMA.I2C1.SCL, CONN.USB_OTG1.PWR, LSIO.GPIO4.IO03 */
#define SC_P_USB_SS3_TC1                         154	/* DMA.I2C1.SCL, CONN.USB_OTG2.PWR, LSIO.GPIO4.IO04 */
#define SC_P_USB_SS3_TC2                         155	/* DMA.I2C1.SDA, CONN.USB_OTG1.OC, LSIO.GPIO4.IO05 */
#define SC_P_USB_SS3_TC3                         156	/* DMA.I2C1.SDA, CONN.USB_OTG2.OC, LSIO.GPIO4.IO06 */
#define SC_P_COMP_CTL_GPIO_3V3_USB3IO            157	/*  */
#define SC_P_USDHC1_RESET_B                      158	/* CONN.USDHC1.RESET_B, LSIO.GPIO4.IO07 */
#define SC_P_USDHC1_VSELECT                      159	/* CONN.USDHC1.VSELECT, LSIO.GPIO4.IO08 */
#define SC_P_USDHC2_RESET_B                      160	/* CONN.USDHC2.RESET_B, LSIO.GPIO4.IO09 */
#define SC_P_USDHC2_VSELECT                      161	/* CONN.USDHC2.VSELECT, LSIO.GPIO4.IO10 */
#define SC_P_USDHC2_WP                           162	/* CONN.USDHC2.WP, LSIO.GPIO4.IO11 */
#define SC_P_USDHC2_CD_B                         163	/* CONN.USDHC2.CD_B, LSIO.GPIO4.IO12 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_VSELSEP       164	/*  */
#define SC_P_ENET0_MDIO                          165	/* CONN.ENET0.MDIO, DMA.I2C4.SDA, LSIO.GPIO4.IO13 */
#define SC_P_ENET0_MDC                           166	/* CONN.ENET0.MDC, DMA.I2C4.SCL, LSIO.GPIO4.IO14 */
#define SC_P_ENET0_REFCLK_125M_25M               167	/* CONN.ENET0.REFCLK_125M_25M, CONN.ENET0.PPS, LSIO.GPIO4.IO15 */
#define SC_P_ENET1_REFCLK_125M_25M               168	/* CONN.ENET1.REFCLK_125M_25M, CONN.ENET1.PPS, LSIO.GPIO4.IO16 */
#define SC_P_ENET1_MDIO                          169	/* CONN.ENET1.MDIO, DMA.I2C4.SDA, LSIO.GPIO4.IO17 */
#define SC_P_ENET1_MDC                           170	/* CONN.ENET1.MDC, DMA.I2C4.SCL, LSIO.GPIO4.IO18 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_GPIOCT        171	/*  */
#define SC_P_QSPI1A_SS0_B                        172	/* LSIO.QSPI1A.SS0_B, LSIO.GPIO4.IO19 */
#define SC_P_QSPI1A_SS1_B                        173	/* LSIO.QSPI1A.SS1_B, LSIO.QSPI1A.SCLK2, LSIO.GPIO4.IO20 */
#define SC_P_QSPI1A_SCLK                         174	/* LSIO.QSPI1A.SCLK, LSIO.GPIO4.IO21 */
#define SC_P_QSPI1A_DQS                          175	/* LSIO.QSPI1A.DQS, LSIO.GPIO4.IO22 */
#define SC_P_QSPI1A_DATA3                        176	/* LSIO.QSPI1A.DATA3, DMA.I2C1.SDA, CONN.USB_OTG1.OC, LSIO.GPIO4.IO23 */
#define SC_P_QSPI1A_DATA2                        177	/* LSIO.QSPI1A.DATA2, DMA.I2C1.SCL, CONN.USB_OTG2.PWR, LSIO.GPIO4.IO24 */
#define SC_P_QSPI1A_DATA1                        178	/* LSIO.QSPI1A.DATA1, DMA.I2C1.SDA, CONN.USB_OTG2.OC, LSIO.GPIO4.IO25 */
#define SC_P_QSPI1A_DATA0                        179	/* LSIO.QSPI1A.DATA0, LSIO.GPIO4.IO26 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_QSPI1         180	/*  */
#define SC_P_QSPI0A_DATA0                        181	/* LSIO.QSPI0A.DATA0 */
#define SC_P_QSPI0A_DATA1                        182	/* LSIO.QSPI0A.DATA1 */
#define SC_P_QSPI0A_DATA2                        183	/* LSIO.QSPI0A.DATA2 */
#define SC_P_QSPI0A_DATA3                        184	/* LSIO.QSPI0A.DATA3 */
#define SC_P_QSPI0A_DQS                          185	/* LSIO.QSPI0A.DQS */
#define SC_P_QSPI0A_SS0_B                        186	/* LSIO.QSPI0A.SS0_B */
#define SC_P_QSPI0A_SS1_B                        187	/* LSIO.QSPI0A.SS1_B, LSIO.QSPI0A.SCLK2 */
#define SC_P_QSPI0A_SCLK                         188	/* LSIO.QSPI0A.SCLK */
#define SC_P_QSPI0B_SCLK                         189	/* LSIO.QSPI0B.SCLK */
#define SC_P_QSPI0B_DATA0                        190	/* LSIO.QSPI0B.DATA0 */
#define SC_P_QSPI0B_DATA1                        191	/* LSIO.QSPI0B.DATA1 */
#define SC_P_QSPI0B_DATA2                        192	/* LSIO.QSPI0B.DATA2 */
#define SC_P_QSPI0B_DATA3                        193	/* LSIO.QSPI0B.DATA3 */
#define SC_P_QSPI0B_DQS                          194	/* LSIO.QSPI0B.DQS */
#define SC_P_QSPI0B_SS0_B                        195	/* LSIO.QSPI0B.SS0_B */
#define SC_P_QSPI0B_SS1_B                        196	/* LSIO.QSPI0B.SS1_B, LSIO.QSPI0B.SCLK2 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_QSPI0         197	/*  */
#define SC_P_PCIE_CTRL0_CLKREQ_B                 198	/* HSIO.PCIE0.CLKREQ_B, LSIO.GPIO4.IO27 */
#define SC_P_PCIE_CTRL0_WAKE_B                   199	/* HSIO.PCIE0.WAKE_B, LSIO.GPIO4.IO28 */
#define SC_P_PCIE_CTRL0_PERST_B                  200	/* HSIO.PCIE0.PERST_B, LSIO.GPIO4.IO29 */
#define SC_P_PCIE_CTRL1_CLKREQ_B                 201	/* HSIO.PCIE1.CLKREQ_B, DMA.I2C1.SDA, CONN.USB_OTG2.OC, LSIO.GPIO4.IO30 */
#define SC_P_PCIE_CTRL1_WAKE_B                   202	/* HSIO.PCIE1.WAKE_B, DMA.I2C1.SCL, CONN.USB_OTG2.PWR, LSIO.GPIO4.IO31 */
#define SC_P_PCIE_CTRL1_PERST_B                  203	/* HSIO.PCIE1.PERST_B, DMA.I2C1.SCL, CONN.USB_OTG1.PWR, LSIO.GPIO5.IO00 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_PCIESEP       204	/*  */
#define SC_P_USB_HSIC0_DATA                      205	/* CONN.USB_HSIC0.DATA, DMA.I2C1.SDA, LSIO.GPIO5.IO01 */
#define SC_P_USB_HSIC0_STROBE                    206	/* CONN.USB_HSIC0.STROBE, DMA.I2C1.SCL, LSIO.GPIO5.IO02 */
#define SC_P_CALIBRATION_0_HSIC                  207	/*  */
#define SC_P_CALIBRATION_1_HSIC                  208	/*  */
#define SC_P_EMMC0_CLK                           209	/* CONN.EMMC0.CLK, CONN.NAND.READY_B */
#define SC_P_EMMC0_CMD                           210	/* CONN.EMMC0.CMD, CONN.NAND.DQS, AUD.MQS.R, LSIO.GPIO5.IO03 */
#define SC_P_EMMC0_DATA0                         211	/* CONN.EMMC0.DATA0, CONN.NAND.DATA00, LSIO.GPIO5.IO04 */
#define SC_P_EMMC0_DATA1                         212	/* CONN.EMMC0.DATA1, CONN.NAND.DATA01, LSIO.GPIO5.IO05 */
#define SC_P_EMMC0_DATA2                         213	/* CONN.EMMC0.DATA2, CONN.NAND.DATA02, LSIO.GPIO5.IO06 */
#define SC_P_EMMC0_DATA3                         214	/* CONN.EMMC0.DATA3, CONN.NAND.DATA03, LSIO.GPIO5.IO07 */
#define SC_P_EMMC0_DATA4                         215	/* CONN.EMMC0.DATA4, CONN.NAND.DATA04, LSIO.GPIO5.IO08 */
#define SC_P_EMMC0_DATA5                         216	/* CONN.EMMC0.DATA5, CONN.NAND.DATA05, LSIO.GPIO5.IO09 */
#define SC_P_EMMC0_DATA6                         217	/* CONN.EMMC0.DATA6, CONN.NAND.DATA06, LSIO.GPIO5.IO10 */
#define SC_P_EMMC0_DATA7                         218	/* CONN.EMMC0.DATA7, CONN.NAND.DATA07, LSIO.GPIO5.IO11 */
#define SC_P_EMMC0_STROBE                        219	/* CONN.EMMC0.STROBE, CONN.NAND.CLE, LSIO.GPIO5.IO12 */
#define SC_P_EMMC0_RESET_B                       220	/* CONN.EMMC0.RESET_B, CONN.NAND.WP_B, CONN.USDHC1.VSELECT, LSIO.GPIO5.IO13 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_SD1FIX        221	/*  */
#define SC_P_USDHC1_CLK                          222	/* CONN.USDHC1.CLK, AUD.MQS.R */
#define SC_P_USDHC1_CMD                          223	/* CONN.USDHC1.CMD, AUD.MQS.L, LSIO.GPIO5.IO14 */
#define SC_P_USDHC1_DATA0                        224	/* CONN.USDHC1.DATA0, CONN.NAND.RE_N, LSIO.GPIO5.IO15 */
#define SC_P_USDHC1_DATA1                        225	/* CONN.USDHC1.DATA1, CONN.NAND.RE_P, LSIO.GPIO5.IO16 */
#define SC_P_CTL_NAND_RE_P_N                     226	/*  */
#define SC_P_USDHC1_DATA2                        227	/* CONN.USDHC1.DATA2, CONN.NAND.DQS_N, LSIO.GPIO5.IO17 */
#define SC_P_USDHC1_DATA3                        228	/* CONN.USDHC1.DATA3, CONN.NAND.DQS_P, LSIO.GPIO5.IO18 */
#define SC_P_CTL_NAND_DQS_P_N                    229	/*  */
#define SC_P_USDHC1_DATA4                        230	/* CONN.USDHC1.DATA4, CONN.NAND.CE0_B, AUD.MQS.R, LSIO.GPIO5.IO19 */
#define SC_P_USDHC1_DATA5                        231	/* CONN.USDHC1.DATA5, CONN.NAND.RE_B, AUD.MQS.L, LSIO.GPIO5.IO20 */
#define SC_P_USDHC1_DATA6                        232	/* CONN.USDHC1.DATA6, CONN.NAND.WE_B, CONN.USDHC1.WP, LSIO.GPIO5.IO21 */
#define SC_P_USDHC1_DATA7                        233	/* CONN.USDHC1.DATA7, CONN.NAND.ALE, CONN.USDHC1.CD_B, LSIO.GPIO5.IO22 */
#define SC_P_USDHC1_STROBE                       234	/* CONN.USDHC1.STROBE, CONN.NAND.CE1_B, CONN.USDHC1.RESET_B, LSIO.GPIO5.IO23 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_VSEL2         235	/*  */
#define SC_P_USDHC2_CLK                          236	/* CONN.USDHC2.CLK, AUD.MQS.R, LSIO.GPIO5.IO24 */
#define SC_P_USDHC2_CMD                          237	/* CONN.USDHC2.CMD, AUD.MQS.L, LSIO.GPIO5.IO25 */
#define SC_P_USDHC2_DATA0                        238	/* CONN.USDHC2.DATA0, DMA.UART4.RX, LSIO.GPIO5.IO26 */
#define SC_P_USDHC2_DATA1                        239	/* CONN.USDHC2.DATA1, DMA.UART4.TX, LSIO.GPIO5.IO27 */
#define SC_P_USDHC2_DATA2                        240	/* CONN.USDHC2.DATA2, DMA.UART4.CTS_B, LSIO.GPIO5.IO28 */
#define SC_P_USDHC2_DATA3                        241	/* CONN.USDHC2.DATA3, DMA.UART4.RTS_B, LSIO.GPIO5.IO29 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_VSEL3         242	/*  */
#define SC_P_ENET0_RGMII_TXC                     243	/* CONN.ENET0.RGMII_TXC, CONN.ENET0.RCLK50M_OUT, CONN.ENET0.RCLK50M_IN, LSIO.GPIO5.IO30 */
#define SC_P_ENET0_RGMII_TX_CTL                  244	/* CONN.ENET0.RGMII_TX_CTL, LSIO.GPIO5.IO31 */
#define SC_P_ENET0_RGMII_TXD0                    245	/* CONN.ENET0.RGMII_TXD0, LSIO.GPIO6.IO00 */
#define SC_P_ENET0_RGMII_TXD1                    246	/* CONN.ENET0.RGMII_TXD1, LSIO.GPIO6.IO01 */
#define SC_P_ENET0_RGMII_TXD2                    247	/* CONN.ENET0.RGMII_TXD2, DMA.UART3.TX, VPU.TSI_S1.VID, LSIO.GPIO6.IO02 */
#define SC_P_ENET0_RGMII_TXD3                    248	/* CONN.ENET0.RGMII_TXD3, DMA.UART3.RTS_B, VPU.TSI_S1.SYNC, LSIO.GPIO6.IO03 */
#define SC_P_ENET0_RGMII_RXC                     249	/* CONN.ENET0.RGMII_RXC, DMA.UART3.CTS_B, VPU.TSI_S1.DATA, LSIO.GPIO6.IO04 */
#define SC_P_ENET0_RGMII_RX_CTL                  250	/* CONN.ENET0.RGMII_RX_CTL, VPU.TSI_S0.VID, LSIO.GPIO6.IO05 */
#define SC_P_ENET0_RGMII_RXD0                    251	/* CONN.ENET0.RGMII_RXD0, VPU.TSI_S0.SYNC, LSIO.GPIO6.IO06 */
#define SC_P_ENET0_RGMII_RXD1                    252	/* CONN.ENET0.RGMII_RXD1, VPU.TSI_S0.DATA, LSIO.GPIO6.IO07 */
#define SC_P_ENET0_RGMII_RXD2                    253	/* CONN.ENET0.RGMII_RXD2, CONN.ENET0.RMII_RX_ER, VPU.TSI_S0.CLK, LSIO.GPIO6.IO08 */
#define SC_P_ENET0_RGMII_RXD3                    254	/* CONN.ENET0.RGMII_RXD3, DMA.UART3.RX, VPU.TSI_S1.CLK, LSIO.GPIO6.IO09 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETB    255	/*  */
#define SC_P_ENET1_RGMII_TXC                     256	/* CONN.ENET1.RGMII_TXC, CONN.ENET1.RCLK50M_OUT, CONN.ENET1.RCLK50M_IN, LSIO.GPIO6.IO10 */
#define SC_P_ENET1_RGMII_TX_CTL                  257	/* CONN.ENET1.RGMII_TX_CTL, LSIO.GPIO6.IO11 */
#define SC_P_ENET1_RGMII_TXD0                    258	/* CONN.ENET1.RGMII_TXD0, LSIO.GPIO6.IO12 */
#define SC_P_ENET1_RGMII_TXD1                    259	/* CONN.ENET1.RGMII_TXD1, LSIO.GPIO6.IO13 */
#define SC_P_ENET1_RGMII_TXD2                    260	/* CONN.ENET1.RGMII_TXD2, DMA.UART3.TX, VPU.TSI_S1.VID, LSIO.GPIO6.IO14 */
#define SC_P_ENET1_RGMII_TXD3                    261	/* CONN.ENET1.RGMII_TXD3, DMA.UART3.RTS_B, VPU.TSI_S1.SYNC, LSIO.GPIO6.IO15 */
#define SC_P_ENET1_RGMII_RXC                     262	/* CONN.ENET1.RGMII_RXC, DMA.UART3.CTS_B, VPU.TSI_S1.DATA, LSIO.GPIO6.IO16 */
#define SC_P_ENET1_RGMII_RX_CTL                  263	/* CONN.ENET1.RGMII_RX_CTL, VPU.TSI_S0.VID, LSIO.GPIO6.IO17 */
#define SC_P_ENET1_RGMII_RXD0                    264	/* CONN.ENET1.RGMII_RXD0, VPU.TSI_S0.SYNC, LSIO.GPIO6.IO18 */
#define SC_P_ENET1_RGMII_RXD1                    265	/* CONN.ENET1.RGMII_RXD1, VPU.TSI_S0.DATA, LSIO.GPIO6.IO19 */
#define SC_P_ENET1_RGMII_RXD2                    266	/* CONN.ENET1.RGMII_RXD2, CONN.ENET1.RMII_RX_ER, VPU.TSI_S0.CLK, LSIO.GPIO6.IO20 */
#define SC_P_ENET1_RGMII_RXD3                    267	/* CONN.ENET1.RGMII_RXD3, DMA.UART3.RX, VPU.TSI_S1.CLK, LSIO.GPIO6.IO21 */
#define SC_P_COMP_CTL_GPIO_1V8_3V3_ENET_ENETA    268	/*  */
/*@}*/

#endif /* IMX8QM_PADS_H */
