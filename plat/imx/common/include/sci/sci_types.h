/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * Header file containing types used across multiple service APIs.
 */

#ifndef SCI_TYPES_H
#define SCI_TYPES_H

/* Includes */

#include <sci/sci_scfw.h>

/* Defines */

/*!
 * @name Defines for common frequencies
 */
/*@{*/
#define SC_32KHZ            32768U	/* 32KHz */
#define SC_10MHZ         10000000U	/* 10MHz */
#define SC_20MHZ         20000000U	/* 20MHz */
#define SC_25MHZ         25000000U	/* 25MHz */
#define SC_27MHZ         27000000U	/* 27MHz */
#define SC_40MHZ         40000000U	/* 40MHz */
#define SC_45MHZ         45000000U	/* 45MHz */
#define SC_50MHZ         50000000U	/* 50MHz */
#define SC_60MHZ         60000000U	/* 60MHz */
#define SC_66MHZ         66666666U	/* 66MHz */
#define SC_74MHZ         74250000U	/* 74.25MHz */
#define SC_80MHZ         80000000U	/* 80MHz */
#define SC_83MHZ         83333333U	/* 83MHz */
#define SC_84MHZ         84375000U	/* 84.37MHz */
#define SC_100MHZ       100000000U	/* 100MHz */
#define SC_125MHZ       125000000U	/* 125MHz */
#define SC_133MHZ       133333333U	/* 133MHz */
#define SC_135MHZ       135000000U	/* 135MHz */
#define SC_150MHZ       150000000U	/* 150MHz */
#define SC_160MHZ       160000000U	/* 160MHz */
#define SC_166MHZ       166666666U	/* 166MHz */
#define SC_175MHZ       175000000U	/* 175MHz */
#define SC_180MHZ       180000000U	/* 180MHz */
#define SC_200MHZ       200000000U	/* 200MHz */
#define SC_250MHZ       250000000U	/* 250MHz */
#define SC_266MHZ       266666666U	/* 266MHz */
#define SC_300MHZ       300000000U	/* 300MHz */
#define SC_312MHZ       312500000U	/* 312.5MHZ */
#define SC_320MHZ       320000000U	/* 320MHz */
#define SC_325MHZ       325000000U	/* 325MHz */
#define SC_333MHZ       333333333U	/* 333MHz */
#define SC_350MHZ       350000000U	/* 350MHz */
#define SC_372MHZ       372000000U	/* 372MHz */
#define SC_375MHZ       375000000U	/* 375MHz */
#define SC_400MHZ       400000000U	/* 400MHz */
#define SC_500MHZ       500000000U	/* 500MHz */
#define SC_594MHZ       594000000U	/* 594MHz */
#define SC_625MHZ       625000000U	/* 625MHz */
#define SC_640MHZ       640000000U	/* 640MHz */
#define SC_650MHZ       650000000U	/* 650MHz */
#define SC_667MHZ       666666667U	/* 667MHz */
#define SC_675MHZ       675000000U	/* 675MHz */
#define SC_700MHZ       700000000U	/* 700MHz */
#define SC_720MHZ       720000000U	/* 720MHz */
#define SC_750MHZ       750000000U	/* 750MHz */
#define SC_800MHZ       800000000U	/* 800MHz */
#define SC_850MHZ       850000000U	/* 850MHz */
#define SC_900MHZ       900000000U	/* 900MHz */
#define SC_1000MHZ     1000000000U	/* 1GHz */
#define SC_1056MHZ     1056000000U	/* 1.056GHz */
#define SC_1188MHZ     1188000000U	/* 1.188GHz */
#define SC_1260MHZ     1260000000U	/* 1.26GHz */
#define SC_1280MHZ     1280000000U	/* 1.28GHz */
#define SC_1300MHZ     1300000000U	/* 1.3GHz */
#define SC_1400MHZ     1400000000U	/* 1.4GHz */
#define SC_1500MHZ     1500000000U	/* 1.5GHz */
#define SC_1600MHZ     1600000000U	/* 1.6GHz */
#define SC_1800MHZ     1800000000U	/* 1.8GHz */
#define SC_2000MHZ     2000000000U	/* 2.0GHz */
#define SC_2112MHZ     2112000000U	/* 2.12GHz */
/*@}*/

/*!
 * @name Defines for 24M related frequencies
 */
/*@{*/
#define SC_8MHZ           8000000U	/* 8MHz */
#define SC_12MHZ         12000000U	/* 12MHz */
#define SC_19MHZ         19800000U	/* 19.8MHz */
#define SC_24MHZ         24000000U	/* 24MHz */
#define SC_48MHZ         48000000U	/* 48MHz */
#define SC_120MHZ       120000000U	/* 120MHz */
#define SC_132MHZ       132000000U	/* 132MHz */
#define SC_144MHZ       144000000U	/* 144MHz */
#define SC_192MHZ       192000000U	/* 192MHz */
#define SC_211MHZ       211200000U	/* 211.2MHz */
#define SC_240MHZ       240000000U	/* 240MHz */
#define SC_264MHZ       264000000U	/* 264MHz */
#define SC_352MHZ       352000000U	/* 352MHz */
#define SC_360MHZ       360000000U	/* 360MHz */
#define SC_384MHZ       384000000U	/* 384MHz */
#define SC_396MHZ       396000000U	/* 396MHz */
#define SC_432MHZ       432000000U	/* 432MHz */
#define SC_480MHZ       480000000U	/* 480MHz */
#define SC_600MHZ       600000000U	/* 600MHz */
#define SC_744MHZ       744000000U	/* 744MHz */
#define SC_792MHZ       792000000U	/* 792MHz */
#define SC_864MHZ       864000000U	/* 864MHz */
#define SC_960MHZ       960000000U	/* 960MHz */
#define SC_1056MHZ     1056000000U	/* 1056MHz */
#define SC_1200MHZ     1200000000U	/* 1.2GHz */
#define SC_1464MHZ     1464000000U	/* 1.464GHz */
#define SC_2400MHZ     2400000000U	/* 2.4GHz */
/*@}*/

/*!
 * @name Defines for A/V related frequencies
 */
/*@{*/
#define SC_62MHZ         62937500U	/* 62.9375MHz */
#define SC_755MHZ       755250000U	/* 755.25MHz */
/*@}*/

/*!
 * @name Defines for type widths
 */
/*@{*/
#define SC_FADDR_W      36U	/* Width of sc_faddr_t */
#define SC_BOOL_W       1U	/* Width of sc_bool_t */
#define SC_ERR_W        4U	/* Width of sc_err_t */
#define SC_RSRC_W       10U	/* Width of sc_rsrc_t */
#define SC_CTRL_W       6U	/* Width of sc_ctrl_t */
/*@}*/

/*!
 * @name Defines for sc_bool_t
 */
/*@{*/
#define SC_FALSE        ((sc_bool_t) 0U)	/* True */
#define SC_TRUE         ((sc_bool_t) 1U)	/* False */
/*@}*/

/*!
 * @name Defines for sc_err_t.
 */
/*@{*/
#define SC_ERR_NONE         0U	/* Success */
#define SC_ERR_VERSION      1U	/* Incompatible API version */
#define SC_ERR_CONFIG       2U	/* Configuration error */
#define SC_ERR_PARM         3U	/* Bad parameter */
#define SC_ERR_NOACCESS     4U	/* Permission error (no access) */
#define SC_ERR_LOCKED       5U	/* Permission error (locked) */
#define SC_ERR_UNAVAILABLE  6U	/* Unavailable (out of resources) */
#define SC_ERR_NOTFOUND     7U	/* Not found */
#define SC_ERR_NOPOWER      8U	/* No power */
#define SC_ERR_IPC          9U	/* Generic IPC error */
#define SC_ERR_BUSY         10U	/* Resource is currently busy/active */
#define SC_ERR_FAIL         11U	/* General I/O failure */
#define SC_ERR_LAST         12U
/*@}*/

/*!
 * @name Defines for sc_rsrc_t.
 */
/*@{*/
#define SC_R_A53                  0U
#define SC_R_A53_0                1U
#define SC_R_A53_1                2U
#define SC_R_A53_2                3U
#define SC_R_A53_3                4U
#define SC_R_A72                  5U
#define SC_R_A72_0                6U
#define SC_R_A72_1                7U
#define SC_R_A72_2                8U
#define SC_R_A72_3                9U
#define SC_R_CCI                  10U
#define SC_R_DB                   11U
#define SC_R_DRC_0                12U
#define SC_R_DRC_1                13U
#define SC_R_GIC_SMMU             14U
#define SC_R_IRQSTR_M4_0          15U
#define SC_R_IRQSTR_M4_1          16U
#define SC_R_SMMU                 17U
#define SC_R_GIC                  18U
#define SC_R_DC_0_BLIT0           19U
#define SC_R_DC_0_BLIT1           20U
#define SC_R_DC_0_BLIT2           21U
#define SC_R_DC_0_BLIT_OUT        22U
#define SC_R_DC_0_CAPTURE0        23U
#define SC_R_DC_0_CAPTURE1        24U
#define SC_R_DC_0_WARP            25U
#define SC_R_DC_0_INTEGRAL0       26U
#define SC_R_DC_0_INTEGRAL1       27U
#define SC_R_DC_0_VIDEO0          28U
#define SC_R_DC_0_VIDEO1          29U
#define SC_R_DC_0_FRAC0           30U
#define SC_R_DC_0_FRAC1           31U
#define SC_R_DC_0                 32U
#define SC_R_GPU_2_PID0           33U
#define SC_R_DC_0_PLL_0           34U
#define SC_R_DC_0_PLL_1           35U
#define SC_R_DC_1_BLIT0           36U
#define SC_R_DC_1_BLIT1           37U
#define SC_R_DC_1_BLIT2           38U
#define SC_R_DC_1_BLIT_OUT        39U
#define SC_R_DC_1_CAPTURE0        40U
#define SC_R_DC_1_CAPTURE1        41U
#define SC_R_DC_1_WARP            42U
#define SC_R_DC_1_INTEGRAL0       43U
#define SC_R_DC_1_INTEGRAL1       44U
#define SC_R_DC_1_VIDEO0          45U
#define SC_R_DC_1_VIDEO1          46U
#define SC_R_DC_1_FRAC0           47U
#define SC_R_DC_1_FRAC1           48U
#define SC_R_DC_1                 49U
#define SC_R_GPU_3_PID0           50U
#define SC_R_DC_1_PLL_0           51U
#define SC_R_DC_1_PLL_1           52U
#define SC_R_SPI_0                53U
#define SC_R_SPI_1                54U
#define SC_R_SPI_2                55U
#define SC_R_SPI_3                56U
#define SC_R_UART_0               57U
#define SC_R_UART_1               58U
#define SC_R_UART_2               59U
#define SC_R_UART_3               60U
#define SC_R_UART_4               61U
#define SC_R_EMVSIM_0             62U
#define SC_R_EMVSIM_1             63U
#define SC_R_DMA_0_CH0            64U
#define SC_R_DMA_0_CH1            65U
#define SC_R_DMA_0_CH2            66U
#define SC_R_DMA_0_CH3            67U
#define SC_R_DMA_0_CH4            68U
#define SC_R_DMA_0_CH5            69U
#define SC_R_DMA_0_CH6            70U
#define SC_R_DMA_0_CH7            71U
#define SC_R_DMA_0_CH8            72U
#define SC_R_DMA_0_CH9            73U
#define SC_R_DMA_0_CH10           74U
#define SC_R_DMA_0_CH11           75U
#define SC_R_DMA_0_CH12           76U
#define SC_R_DMA_0_CH13           77U
#define SC_R_DMA_0_CH14           78U
#define SC_R_DMA_0_CH15           79U
#define SC_R_DMA_0_CH16           80U
#define SC_R_DMA_0_CH17           81U
#define SC_R_DMA_0_CH18           82U
#define SC_R_DMA_0_CH19           83U
#define SC_R_DMA_0_CH20           84U
#define SC_R_DMA_0_CH21           85U
#define SC_R_DMA_0_CH22           86U
#define SC_R_DMA_0_CH23           87U
#define SC_R_DMA_0_CH24           88U
#define SC_R_DMA_0_CH25           89U
#define SC_R_DMA_0_CH26           90U
#define SC_R_DMA_0_CH27           91U
#define SC_R_DMA_0_CH28           92U
#define SC_R_DMA_0_CH29           93U
#define SC_R_DMA_0_CH30           94U
#define SC_R_DMA_0_CH31           95U
#define SC_R_I2C_0                96U
#define SC_R_I2C_1                97U
#define SC_R_I2C_2                98U
#define SC_R_I2C_3                99U
#define SC_R_I2C_4                100U
#define SC_R_ADC_0                101U
#define SC_R_ADC_1                102U
#define SC_R_FTM_0                103U
#define SC_R_FTM_1                104U
#define SC_R_CAN_0                105U
#define SC_R_CAN_1                106U
#define SC_R_CAN_2                107U
#define SC_R_DMA_1_CH0            108U
#define SC_R_DMA_1_CH1            109U
#define SC_R_DMA_1_CH2            110U
#define SC_R_DMA_1_CH3            111U
#define SC_R_DMA_1_CH4            112U
#define SC_R_DMA_1_CH5            113U
#define SC_R_DMA_1_CH6            114U
#define SC_R_DMA_1_CH7            115U
#define SC_R_DMA_1_CH8            116U
#define SC_R_DMA_1_CH9            117U
#define SC_R_DMA_1_CH10           118U
#define SC_R_DMA_1_CH11           119U
#define SC_R_DMA_1_CH12           120U
#define SC_R_DMA_1_CH13           121U
#define SC_R_DMA_1_CH14           122U
#define SC_R_DMA_1_CH15           123U
#define SC_R_DMA_1_CH16           124U
#define SC_R_DMA_1_CH17           125U
#define SC_R_DMA_1_CH18           126U
#define SC_R_DMA_1_CH19           127U
#define SC_R_DMA_1_CH20           128U
#define SC_R_DMA_1_CH21           129U
#define SC_R_DMA_1_CH22           130U
#define SC_R_DMA_1_CH23           131U
#define SC_R_DMA_1_CH24           132U
#define SC_R_DMA_1_CH25           133U
#define SC_R_DMA_1_CH26           134U
#define SC_R_DMA_1_CH27           135U
#define SC_R_DMA_1_CH28           136U
#define SC_R_DMA_1_CH29           137U
#define SC_R_DMA_1_CH30           138U
#define SC_R_DMA_1_CH31           139U
#define SC_R_UNUSED1              140U
#define SC_R_UNUSED2              141U
#define SC_R_UNUSED3              142U
#define SC_R_UNUSED4              143U
#define SC_R_GPU_0_PID0           144U
#define SC_R_GPU_0_PID1           145U
#define SC_R_GPU_0_PID2           146U
#define SC_R_GPU_0_PID3           147U
#define SC_R_GPU_1_PID0           148U
#define SC_R_GPU_1_PID1           149U
#define SC_R_GPU_1_PID2           150U
#define SC_R_GPU_1_PID3           151U
#define SC_R_PCIE_A               152U
#define SC_R_SERDES_0             153U
#define SC_R_MATCH_0              154U
#define SC_R_MATCH_1              155U
#define SC_R_MATCH_2              156U
#define SC_R_MATCH_3              157U
#define SC_R_MATCH_4              158U
#define SC_R_MATCH_5              159U
#define SC_R_MATCH_6              160U
#define SC_R_MATCH_7              161U
#define SC_R_MATCH_8              162U
#define SC_R_MATCH_9              163U
#define SC_R_MATCH_10             164U
#define SC_R_MATCH_11             165U
#define SC_R_MATCH_12             166U
#define SC_R_MATCH_13             167U
#define SC_R_MATCH_14             168U
#define SC_R_PCIE_B               169U
#define SC_R_SATA_0               170U
#define SC_R_SERDES_1             171U
#define SC_R_HSIO_GPIO            172U
#define SC_R_MATCH_15             173U
#define SC_R_MATCH_16             174U
#define SC_R_MATCH_17             175U
#define SC_R_MATCH_18             176U
#define SC_R_MATCH_19             177U
#define SC_R_MATCH_20             178U
#define SC_R_MATCH_21             179U
#define SC_R_MATCH_22             180U
#define SC_R_MATCH_23             181U
#define SC_R_MATCH_24             182U
#define SC_R_MATCH_25             183U
#define SC_R_MATCH_26             184U
#define SC_R_MATCH_27             185U
#define SC_R_MATCH_28             186U
#define SC_R_LCD_0                187U
#define SC_R_LCD_0_PWM_0          188U
#define SC_R_LCD_0_I2C_0          189U
#define SC_R_LCD_0_I2C_1          190U
#define SC_R_PWM_0                191U
#define SC_R_PWM_1                192U
#define SC_R_PWM_2                193U
#define SC_R_PWM_3                194U
#define SC_R_PWM_4                195U
#define SC_R_PWM_5                196U
#define SC_R_PWM_6                197U
#define SC_R_PWM_7                198U
#define SC_R_GPIO_0               199U
#define SC_R_GPIO_1               200U
#define SC_R_GPIO_2               201U
#define SC_R_GPIO_3               202U
#define SC_R_GPIO_4               203U
#define SC_R_GPIO_5               204U
#define SC_R_GPIO_6               205U
#define SC_R_GPIO_7               206U
#define SC_R_GPT_0                207U
#define SC_R_GPT_1                208U
#define SC_R_GPT_2                209U
#define SC_R_GPT_3                210U
#define SC_R_GPT_4                211U
#define SC_R_KPP                  212U
#define SC_R_MU_0A                213U
#define SC_R_MU_1A                214U
#define SC_R_MU_2A                215U
#define SC_R_MU_3A                216U
#define SC_R_MU_4A                217U
#define SC_R_MU_5A                218U
#define SC_R_MU_6A                219U
#define SC_R_MU_7A                220U
#define SC_R_MU_8A                221U
#define SC_R_MU_9A                222U
#define SC_R_MU_10A               223U
#define SC_R_MU_11A               224U
#define SC_R_MU_12A               225U
#define SC_R_MU_13A               226U
#define SC_R_MU_5B                227U
#define SC_R_MU_6B                228U
#define SC_R_MU_7B                229U
#define SC_R_MU_8B                230U
#define SC_R_MU_9B                231U
#define SC_R_MU_10B               232U
#define SC_R_MU_11B               233U
#define SC_R_MU_12B               234U
#define SC_R_MU_13B               235U
#define SC_R_ROM_0                236U
#define SC_R_FSPI_0               237U
#define SC_R_FSPI_1               238U
#define SC_R_IEE                  239U
#define SC_R_IEE_R0               240U
#define SC_R_IEE_R1               241U
#define SC_R_IEE_R2               242U
#define SC_R_IEE_R3               243U
#define SC_R_IEE_R4               244U
#define SC_R_IEE_R5               245U
#define SC_R_IEE_R6               246U
#define SC_R_IEE_R7               247U
#define SC_R_SDHC_0               248U
#define SC_R_SDHC_1               249U
#define SC_R_SDHC_2               250U
#define SC_R_ENET_0               251U
#define SC_R_ENET_1               252U
#define SC_R_MLB_0                253U
#define SC_R_DMA_2_CH0            254U
#define SC_R_DMA_2_CH1            255U
#define SC_R_DMA_2_CH2            256U
#define SC_R_DMA_2_CH3            257U
#define SC_R_DMA_2_CH4            258U
#define SC_R_USB_0                259U
#define SC_R_USB_1                260U
#define SC_R_USB_0_PHY            261U
#define SC_R_USB_2                262U
#define SC_R_USB_2_PHY            263U
#define SC_R_DTCP                 264U
#define SC_R_NAND                 265U
#define SC_R_LVDS_0               266U
#define SC_R_LVDS_0_PWM_0         267U
#define SC_R_LVDS_0_I2C_0         268U
#define SC_R_LVDS_0_I2C_1         269U
#define SC_R_LVDS_1               270U
#define SC_R_LVDS_1_PWM_0         271U
#define SC_R_LVDS_1_I2C_0         272U
#define SC_R_LVDS_1_I2C_1         273U
#define SC_R_LVDS_2               274U
#define SC_R_LVDS_2_PWM_0         275U
#define SC_R_LVDS_2_I2C_0         276U
#define SC_R_LVDS_2_I2C_1         277U
#define SC_R_M4_0_PID0            278U
#define SC_R_M4_0_PID1            279U
#define SC_R_M4_0_PID2            280U
#define SC_R_M4_0_PID3            281U
#define SC_R_M4_0_PID4            282U
#define SC_R_M4_0_RGPIO           283U
#define SC_R_M4_0_SEMA42          284U
#define SC_R_M4_0_TPM             285U
#define SC_R_M4_0_PIT             286U
#define SC_R_M4_0_UART            287U
#define SC_R_M4_0_I2C             288U
#define SC_R_M4_0_INTMUX          289U
#define SC_R_M4_0_SIM             290U
#define SC_R_M4_0_WDOG            291U
#define SC_R_M4_0_MU_0B           292U
#define SC_R_M4_0_MU_0A0          293U
#define SC_R_M4_0_MU_0A1          294U
#define SC_R_M4_0_MU_0A2          295U
#define SC_R_M4_0_MU_0A3          296U
#define SC_R_M4_0_MU_1A           297U
#define SC_R_M4_1_PID0            298U
#define SC_R_M4_1_PID1            299U
#define SC_R_M4_1_PID2            300U
#define SC_R_M4_1_PID3            301U
#define SC_R_M4_1_PID4            302U
#define SC_R_M4_1_RGPIO           303U
#define SC_R_M4_1_SEMA42          304U
#define SC_R_M4_1_TPM             305U
#define SC_R_M4_1_PIT             306U
#define SC_R_M4_1_UART            307U
#define SC_R_M4_1_I2C             308U
#define SC_R_M4_1_INTMUX          309U
#define SC_R_M4_1_SIM             310U
#define SC_R_M4_1_WDOG            311U
#define SC_R_M4_1_MU_0B           312U
#define SC_R_M4_1_MU_0A0          313U
#define SC_R_M4_1_MU_0A1          314U
#define SC_R_M4_1_MU_0A2          315U
#define SC_R_M4_1_MU_0A3          316U
#define SC_R_M4_1_MU_1A           317U
#define SC_R_SAI_0                318U
#define SC_R_SAI_1                319U
#define SC_R_SAI_2                320U
#define SC_R_IRQSTR_SCU2          321U
#define SC_R_IRQSTR_DSP           322U
#define SC_R_UNUSED5              323U
#define SC_R_OCRAM                324U
#define SC_R_AUDIO_PLL_0          325U
#define SC_R_PI_0                 326U
#define SC_R_PI_0_PWM_0           327U
#define SC_R_PI_0_PWM_1           328U
#define SC_R_PI_0_I2C_0           329U
#define SC_R_PI_0_PLL             330U
#define SC_R_PI_1                 331U
#define SC_R_PI_1_PWM_0           332U
#define SC_R_PI_1_PWM_1           333U
#define SC_R_PI_1_I2C_0           334U
#define SC_R_PI_1_PLL             335U
#define SC_R_SC_PID0              336U
#define SC_R_SC_PID1              337U
#define SC_R_SC_PID2              338U
#define SC_R_SC_PID3              339U
#define SC_R_SC_PID4              340U
#define SC_R_SC_SEMA42            341U
#define SC_R_SC_TPM               342U
#define SC_R_SC_PIT               343U
#define SC_R_SC_UART              344U
#define SC_R_SC_I2C               345U
#define SC_R_SC_MU_0B             346U
#define SC_R_SC_MU_0A0            347U
#define SC_R_SC_MU_0A1            348U
#define SC_R_SC_MU_0A2            349U
#define SC_R_SC_MU_0A3            350U
#define SC_R_SC_MU_1A             351U
#define SC_R_SYSCNT_RD            352U
#define SC_R_SYSCNT_CMP           353U
#define SC_R_DEBUG                354U
#define SC_R_SYSTEM               355U
#define SC_R_SNVS                 356U
#define SC_R_OTP                  357U
#define SC_R_VPU_PID0             358U
#define SC_R_VPU_PID1             359U
#define SC_R_VPU_PID2             360U
#define SC_R_VPU_PID3             361U
#define SC_R_VPU_PID4             362U
#define SC_R_VPU_PID5             363U
#define SC_R_VPU_PID6             364U
#define SC_R_VPU_PID7             365U
#define SC_R_VPU_UART             366U
#define SC_R_VPUCORE              367U
#define SC_R_VPUCORE_0            368U
#define SC_R_VPUCORE_1            369U
#define SC_R_VPUCORE_2            370U
#define SC_R_VPUCORE_3            371U
#define SC_R_DMA_4_CH0            372U
#define SC_R_DMA_4_CH1            373U
#define SC_R_DMA_4_CH2            374U
#define SC_R_DMA_4_CH3            375U
#define SC_R_DMA_4_CH4            376U
#define SC_R_ISI_CH0              377U
#define SC_R_ISI_CH1              378U
#define SC_R_ISI_CH2              379U
#define SC_R_ISI_CH3              380U
#define SC_R_ISI_CH4              381U
#define SC_R_ISI_CH5              382U
#define SC_R_ISI_CH6              383U
#define SC_R_ISI_CH7              384U
#define SC_R_MJPEG_DEC_S0         385U
#define SC_R_MJPEG_DEC_S1         386U
#define SC_R_MJPEG_DEC_S2         387U
#define SC_R_MJPEG_DEC_S3         388U
#define SC_R_MJPEG_ENC_S0         389U
#define SC_R_MJPEG_ENC_S1         390U
#define SC_R_MJPEG_ENC_S2         391U
#define SC_R_MJPEG_ENC_S3         392U
#define SC_R_MIPI_0               393U
#define SC_R_MIPI_0_PWM_0         394U
#define SC_R_MIPI_0_I2C_0         395U
#define SC_R_MIPI_0_I2C_1         396U
#define SC_R_MIPI_1               397U
#define SC_R_MIPI_1_PWM_0         398U
#define SC_R_MIPI_1_I2C_0         399U
#define SC_R_MIPI_1_I2C_1         400U
#define SC_R_CSI_0                401U
#define SC_R_CSI_0_PWM_0          402U
#define SC_R_CSI_0_I2C_0          403U
#define SC_R_CSI_1                404U
#define SC_R_CSI_1_PWM_0          405U
#define SC_R_CSI_1_I2C_0          406U
#define SC_R_HDMI                 407U
#define SC_R_HDMI_I2S             408U
#define SC_R_HDMI_I2C_0           409U
#define SC_R_HDMI_PLL_0           410U
#define SC_R_HDMI_RX              411U
#define SC_R_HDMI_RX_BYPASS       412U
#define SC_R_HDMI_RX_I2C_0        413U
#define SC_R_ASRC_0               414U
#define SC_R_ESAI_0               415U
#define SC_R_SPDIF_0              416U
#define SC_R_SPDIF_1              417U
#define SC_R_SAI_3                418U
#define SC_R_SAI_4                419U
#define SC_R_SAI_5                420U
#define SC_R_GPT_5                421U
#define SC_R_GPT_6                422U
#define SC_R_GPT_7                423U
#define SC_R_GPT_8                424U
#define SC_R_GPT_9                425U
#define SC_R_GPT_10               426U
#define SC_R_DMA_2_CH5            427U
#define SC_R_DMA_2_CH6            428U
#define SC_R_DMA_2_CH7            429U
#define SC_R_DMA_2_CH8            430U
#define SC_R_DMA_2_CH9            431U
#define SC_R_DMA_2_CH10           432U
#define SC_R_DMA_2_CH11           433U
#define SC_R_DMA_2_CH12           434U
#define SC_R_DMA_2_CH13           435U
#define SC_R_DMA_2_CH14           436U
#define SC_R_DMA_2_CH15           437U
#define SC_R_DMA_2_CH16           438U
#define SC_R_DMA_2_CH17           439U
#define SC_R_DMA_2_CH18           440U
#define SC_R_DMA_2_CH19           441U
#define SC_R_DMA_2_CH20           442U
#define SC_R_DMA_2_CH21           443U
#define SC_R_DMA_2_CH22           444U
#define SC_R_DMA_2_CH23           445U
#define SC_R_DMA_2_CH24           446U
#define SC_R_DMA_2_CH25           447U
#define SC_R_DMA_2_CH26           448U
#define SC_R_DMA_2_CH27           449U
#define SC_R_DMA_2_CH28           450U
#define SC_R_DMA_2_CH29           451U
#define SC_R_DMA_2_CH30           452U
#define SC_R_DMA_2_CH31           453U
#define SC_R_ASRC_1               454U
#define SC_R_ESAI_1               455U
#define SC_R_SAI_6                456U
#define SC_R_SAI_7                457U
#define SC_R_AMIX                 458U
#define SC_R_MQS_0                459U
#define SC_R_DMA_3_CH0            460U
#define SC_R_DMA_3_CH1            461U
#define SC_R_DMA_3_CH2            462U
#define SC_R_DMA_3_CH3            463U
#define SC_R_DMA_3_CH4            464U
#define SC_R_DMA_3_CH5            465U
#define SC_R_DMA_3_CH6            466U
#define SC_R_DMA_3_CH7            467U
#define SC_R_DMA_3_CH8            468U
#define SC_R_DMA_3_CH9            469U
#define SC_R_DMA_3_CH10           470U
#define SC_R_DMA_3_CH11           471U
#define SC_R_DMA_3_CH12           472U
#define SC_R_DMA_3_CH13           473U
#define SC_R_DMA_3_CH14           474U
#define SC_R_DMA_3_CH15           475U
#define SC_R_DMA_3_CH16           476U
#define SC_R_DMA_3_CH17           477U
#define SC_R_DMA_3_CH18           478U
#define SC_R_DMA_3_CH19           479U
#define SC_R_DMA_3_CH20           480U
#define SC_R_DMA_3_CH21           481U
#define SC_R_DMA_3_CH22           482U
#define SC_R_DMA_3_CH23           483U
#define SC_R_DMA_3_CH24           484U
#define SC_R_DMA_3_CH25           485U
#define SC_R_DMA_3_CH26           486U
#define SC_R_DMA_3_CH27           487U
#define SC_R_DMA_3_CH28           488U
#define SC_R_DMA_3_CH29           489U
#define SC_R_DMA_3_CH30           490U
#define SC_R_DMA_3_CH31           491U
#define SC_R_AUDIO_PLL_1          492U
#define SC_R_AUDIO_CLK_0          493U
#define SC_R_AUDIO_CLK_1          494U
#define SC_R_MCLK_OUT_0           495U
#define SC_R_MCLK_OUT_1           496U
#define SC_R_PMIC_0               497U
#define SC_R_PMIC_1               498U
#define SC_R_SECO                 499U
#define SC_R_CAAM_JR1             500U
#define SC_R_CAAM_JR2             501U
#define SC_R_CAAM_JR3             502U
#define SC_R_SECO_MU_2            503U
#define SC_R_SECO_MU_3            504U
#define SC_R_SECO_MU_4            505U
#define SC_R_HDMI_RX_PWM_0        506U
#define SC_R_A35                  507U
#define SC_R_A35_0                508U
#define SC_R_A35_1                509U
#define SC_R_A35_2                510U
#define SC_R_A35_3                511U
#define SC_R_DSP                  512U
#define SC_R_DSP_RAM              513U
#define SC_R_CAAM_JR1_OUT         514U
#define SC_R_CAAM_JR2_OUT         515U
#define SC_R_CAAM_JR3_OUT         516U
#define SC_R_VPU_DEC_0            517U
#define SC_R_VPU_ENC_0            518U
#define SC_R_CAAM_JR0             519U
#define SC_R_CAAM_JR0_OUT         520U
#define SC_R_PMIC_2               521U
#define SC_R_DBLOGIC              522U
#define SC_R_HDMI_PLL_1           523U
#define SC_R_BOARD_R0             524U
#define SC_R_BOARD_R1             525U
#define SC_R_BOARD_R2             526U
#define SC_R_BOARD_R3             527U
#define SC_R_BOARD_R4             528U
#define SC_R_BOARD_R5             529U
#define SC_R_BOARD_R6             530U
#define SC_R_BOARD_R7             531U
#define SC_R_MJPEG_DEC_MP         532U
#define SC_R_MJPEG_ENC_MP         533U
#define SC_R_VPU_TS_0             534U
#define SC_R_VPU_MU_0             535U
#define SC_R_VPU_MU_1             536U
#define SC_R_VPU_MU_2             537U
#define SC_R_VPU_MU_3             538U
#define SC_R_VPU_ENC_1            539U
#define SC_R_VPU                  540U
#define SC_R_LAST                 541U
#define SC_R_ALL                  ((sc_rsrc_t) UINT16_MAX)	/* All resources */
/*@}*/

/* NOTE - please add by replacing some of the UNUSED from above! */

/*!
 * Defnes for sc_ctrl_t.
 */
#define SC_C_TEMP                       0U
#define SC_C_TEMP_HI                    1U
#define SC_C_TEMP_LOW                   2U
#define SC_C_PXL_LINK_MST1_ADDR         3U
#define SC_C_PXL_LINK_MST2_ADDR         4U
#define SC_C_PXL_LINK_MST_ENB           5U
#define SC_C_PXL_LINK_MST1_ENB          6U
#define SC_C_PXL_LINK_MST2_ENB          7U
#define SC_C_PXL_LINK_SLV1_ADDR         8U
#define SC_C_PXL_LINK_SLV2_ADDR         9U
#define SC_C_PXL_LINK_MST_VLD           10U
#define SC_C_PXL_LINK_MST1_VLD          11U
#define SC_C_PXL_LINK_MST2_VLD          12U
#define SC_C_SINGLE_MODE                13U
#define SC_C_ID                         14U
#define SC_C_PXL_CLK_POLARITY           15U
#define SC_C_LINESTATE                  16U
#define SC_C_PCIE_G_RST                 17U
#define SC_C_PCIE_BUTTON_RST            18U
#define SC_C_PCIE_PERST                 19U
#define SC_C_PHY_RESET                  20U
#define SC_C_PXL_LINK_RATE_CORRECTION   21U
#define SC_C_PANIC                      22U
#define SC_C_PRIORITY_GROUP             23U
#define SC_C_TXCLK                      24U
#define SC_C_CLKDIV                     25U
#define SC_C_DISABLE_50                 26U
#define SC_C_DISABLE_125                27U
#define SC_C_SEL_125                    28U
#define SC_C_MODE                       29U
#define SC_C_SYNC_CTRL0                 30U
#define SC_C_KACHUNK_CNT                31U
#define SC_C_KACHUNK_SEL                32U
#define SC_C_SYNC_CTRL1                 33U
#define SC_C_DPI_RESET                  34U
#define SC_C_MIPI_RESET                 35U
#define SC_C_DUAL_MODE                  36U
#define SC_C_VOLTAGE                    37U
#define SC_C_PXL_LINK_SEL               38U
#define SC_C_OFS_SEL                    39U
#define SC_C_OFS_AUDIO                  40U
#define SC_C_OFS_PERIPH                 41U
#define SC_C_OFS_IRQ                    42U
#define SC_C_RST0                       43U
#define SC_C_RST1                       44U
#define SC_C_SEL0                       45U
#define SC_C_LAST                       46U

#define SC_P_ALL        ((sc_pad_t) UINT16_MAX)	/* All pads */

/* Types */

/*!
 * This type is used to store a boolean
 */
typedef uint8_t sc_bool_t;

/*!
 * This type is used to store a system (full-size) address.
 */
typedef uint64_t sc_faddr_t;

/*!
 * This type is used to indicate error response for most functions.
 */
typedef uint8_t sc_err_t;

/*!
 * This type is used to indicate a resource. Resources include peripherals
 * and bus masters (but not memory regions). Note items from list should
 * never be changed or removed (only added to at the end of the list).
 */
typedef uint16_t sc_rsrc_t;

/*!
 * This type is used to indicate a control.
 */
typedef uint8_t sc_ctrl_t;

/*!
 * This type is used to indicate a pad. Valid values are SoC specific.
 *
 * Refer to the SoC [Pad List](@ref PADS) for valid pad values.
 */
typedef uint16_t sc_pad_t;

/* Extra documentation of standard types */

#ifdef DOXYGEN
    /*!
     * Type used to declare an 8-bit integer.
     */
typedef __INT8_TYPE__ int8_t;

    /*!
     * Type used to declare a 16-bit integer.
     */
typedef __INT16_TYPE__ int16_t;

    /*!
     * Type used to declare a 32-bit integer.
     */
typedef __INT32_TYPE__ int32_t;

    /*!
     * Type used to declare a 64-bit integer.
     */
typedef __INT64_TYPE__ int64_t;

    /*!
     * Type used to declare an 8-bit unsigned integer.
     */
typedef __UINT8_TYPE__ uint8_t;

    /*!
     * Type used to declare a 16-bit unsigned integer.
     */
typedef __UINT16_TYPE__ uint16_t;

    /*!
     * Type used to declare a 32-bit unsigned integer.
     */
typedef __UINT32_TYPE__ uint32_t;

    /*!
     * Type used to declare a 64-bit unsigned integer.
     */
typedef __UINT64_TYPE__ uint64_t;
#endif

#endif /* SCI_TYPES_H */
