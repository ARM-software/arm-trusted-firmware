/*
 * Copyright (c) 2024, Rockchip, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __CLOCK_H__
#define __CLOCK_H__

/* scmi-clocks indices */

#define SCMI_CLK_CPUL			0
#define SCMI_CLK_DSU			1
#define SCMI_CLK_CPUB01			2
#define SCMI_CLK_CPUB23			3
#define SCMI_CLK_DDR			4
#define SCMI_CLK_GPU			5
#define SCMI_CLK_NPU			6
#define SCMI_CLK_SBUS			7
#define SCMI_PCLK_SBUS			8
#define SCMI_CCLK_SD			9
#define SCMI_DCLK_SD			10
#define SCMI_ACLK_SECURE_NS		11
#define SCMI_HCLK_SECURE_NS		12
#define SCMI_TCLK_WDT			13
#define SCMI_KEYLADDER_CORE		14
#define SCMI_KEYLADDER_RNG		15
#define SCMI_ACLK_SECURE_S		16
#define SCMI_HCLK_SECURE_S		17
#define SCMI_PCLK_SECURE_S		18
#define SCMI_CRYPTO_RNG			19
#define SCMI_CRYPTO_CORE		20
#define SCMI_CRYPTO_PKA			21
#define SCMI_SPLL			22
#define SCMI_HCLK_SD			23
#define SCMI_CRYPTO_RNG_S		24
#define SCMI_CRYPTO_CORE_S		25
#define SCMI_CRYPTO_PKA_S		26
#define SCMI_A_CRYPTO_S			27
#define SCMI_H_CRYPTO_S			28
#define SCMI_P_CRYPTO_S			29
#define SCMI_A_KEYLADDER_S		30
#define SCMI_H_KEYLADDER_S		31
#define SCMI_P_KEYLADDER_S		32
#define SCMI_TRNG_S			33
#define SCMI_H_TRNG_S			34
#define SCMI_P_OTPC_S			35
#define SCMI_OTPC_S			36
#define SCMI_OTP_PHY			37
#define SCMI_OTPC_AUTO_RD		38
#define SCMI_OTPC_ARB			39

/******** DSUCRU **************************************/
#define DSUCRU_CLKSEL_CON(n)		(0x0300 + (n) * 4)

/********Name=DSUCRU_CLKSEL_CON04,Offset=0x310********/
#define PCLK_DSU_ROOT_SEL_SHIFT		5
#define PCLK_DSU_ROOT_SEL_MASK		0x3
#define PCLK_DSU_ROOT_SEL_GPLL		0x3

/********Name=SECURE_SOFTRST_CON00,Offset=0xA00********/
#define SRST_A_SECURE_NS_BIU		10
#define SRST_H_SECURE_NS_BIU		11
#define SRST_A_SECURE_S_BIU		12
#define SRST_H_SECURE_S_BIU		13
#define SRST_P_SECURE_S_BIU		14
#define SRST_CRYPTO_CORE		15
/********Name=SECURE_SOFTRST_CON01,Offset=0xA04********/
#define SRST_CRYPTO_PKA			16
#define SRST_CRYPTO_RNG			17
#define SRST_A_CRYPTO			18
#define SRST_H_CRYPTO			19
#define SRST_KEYLADDER_CORE		25
#define SRST_KEYLADDER_RNG		26
#define SRST_A_KEYLADDER		27
#define SRST_H_KEYLADDER		28
#define SRST_P_OTPC_S			29
#define SRST_OTPC_S			30
#define SRST_WDT_S			31
/********Name=SECURE_SOFTRST_CON02,Offset=0xA08********/
#define SRST_T_WDT_S			32
#define SRST_H_BOOTROM			33
#define SRST_A_DCF			34
#define SRST_P_DCF			35
#define SRST_H_BOOTROM_NS		37
#define SRST_P_KEYLADDER		46
#define SRST_H_TRNG_S			47
/********Name=SECURE_SOFTRST_CON03,Offset=0xA0C********/
#define SRST_H_TRNG_NS			48
#define SRST_D_SDMMC_BUFFER		49
#define SRST_H_SDMMC			50
#define SRST_H_SDMMC_BUFFER		51
#define SRST_SDMMC			52
#define SRST_P_TRNG_CHK			53
#define SRST_TRNG_S			54

#define SRST_INVALID			55

void pvtplls_suspend(void);
void pvtplls_resume(void);

void rockchip_clock_init(void);

#endif
