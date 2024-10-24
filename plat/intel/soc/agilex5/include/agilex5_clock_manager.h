/*
 * Copyright (c) 2019-2022, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLOCKMANAGER_H
#define CLOCKMANAGER_H

#include "socfpga_handoff.h"

/* Clock Manager Registers */
#define CLKMGR_OFFSET					0x10d10000

#define CLKMGR_CTRL					0x0
#define CLKMGR_STAT					0x4
#define CLKMGR_TESTIOCTROL				0x8
#define CLKMGR_INTRGEN					0xc
#define CLKMGR_INTRMSK					0x10
#define CLKMGR_INTRCLR					0x14
#define CLKMGR_INTRSTS					0x18
#define CLKMGR_INTRSTK					0x1c
#define CLKMGR_INTRRAW					0x20

/* Main PLL Group */
#define CLKMGR_MAINPLL					0x10d10024
#define CLKMGR_MAINPLL_EN				0x0
#define CLKMGR_MAINPLL_ENS				0x4
#define CLKMGR_MAINPLL_BYPASS				0xc
#define CLKMGR_MAINPLL_BYPASSS				0x10
#define CLKMGR_MAINPLL_BYPASSR				0x14
#define CLKMGR_MAINPLL_NOCCLK				0x1c
#define CLKMGR_MAINPLL_NOCDIV				0x20
#define CLKMGR_MAINPLL_PLLGLOB				0x24
#define CLKMGR_MAINPLL_FDBCK				0x28
#define CLKMGR_MAINPLL_MEM				0x2c
#define CLKMGR_MAINPLL_MEMSTAT				0x30
#define CLKMGR_MAINPLL_VCOCALIB				0x34
#define CLKMGR_MAINPLL_PLLC0				0x38
#define CLKMGR_MAINPLL_PLLC1				0x3c
#define CLKMGR_MAINPLL_PLLC2				0x40
#define CLKMGR_MAINPLL_PLLC3				0x44
#define CLKMGR_MAINPLL_PLLM				0x48
#define CLKMGR_MAINPLL_FHOP				0x4c
#define CLKMGR_MAINPLL_SSC				0x50
#define CLKMGR_MAINPLL_LOSTLOCK				0x54

/* Peripheral PLL Group */
#define CLKMGR_PERPLL					0x10d1007c
#define CLKMGR_PERPLL_EN				0x0
#define CLKMGR_PERPLL_ENS				0x4
#define CLKMGR_PERPLL_BYPASS				0xc
#define CLKMGR_PERPLL_EMACCTL				0x18
#define CLKMGR_PERPLL_GPIODIV				0x1c
#define CLKMGR_PERPLL_PLLGLOB				0x20
#define CLKMGR_PERPLL_FDBCK				0x24
#define CLKMGR_PERPLL_MEM				0x28
#define CLKMGR_PERPLL_MEMSTAT				0x2c
#define CLKMGR_PERPLL_PLLC0				0x30
#define CLKMGR_PERPLL_PLLC1				0x34
#define CLKMGR_PERPLL_VCOCALIB				0x38
#define CLKMGR_PERPLL_PLLC2				0x3c
#define CLKMGR_PERPLL_PLLC3				0x40
#define CLKMGR_PERPLL_PLLM				0x44
#define CLKMGR_PERPLL_LOSTLOCK				0x50

/* Altera Group */
#define CLKMGR_ALTERA					0x10d100d0
#define CLKMGR_ALTERA_JTAG				0x0
#define CLKMGR_ALTERA_EMACACTR				0x4
#define CLKMGR_ALTERA_EMACBCTR				0x8
#define CLKMGR_ALTERA_EMACPTPCTR			0xc
#define CLKMGR_ALTERA_GPIODBCTR				0x10
#define CLKMGR_ALTERA_S2FUSER0CTR			0x18
#define CLKMGR_ALTERA_S2FUSER1CTR			0x1c
#define CLKMGR_ALTERA_PSIREFCTR				0x20
#define CLKMGR_ALTERA_EXTCNTRST				0x24
#define CLKMGR_ALTERA_USB31CTR				0x28
#define CLKMGR_ALTERA_DSUCTR				0x2c
#define CLKMGR_ALTERA_CORE01CTR				0x30
#define CLKMGR_ALTERA_CORE23CTR				0x34
#define CLKMGR_ALTERA_CORE2CTR				0x38
#define CLKMGR_ALTERA_CORE3CTR				0x3c

/* Membus */
#define CLKMGR_MEM_REQ					BIT(24)
#define CLKMGR_MEM_WR					BIT(25)
#define CLKMGR_MEM_ERR					BIT(26)
#define CLKMGR_MEM_WDAT_OFFSET				16
#define CLKMGR_MEM_ADDR					0x4027
#define CLKMGR_MEM_WDAT					0x80

/* Clock Manager Macros */
#define CLKMGR_CTRL_BOOTMODE_SET_MSK			0x00000001
#define CLKMGR_STAT_BUSY_E_BUSY				0x1
#define CLKMGR_STAT_BUSY(x)				(((x) & 0x00000001) >> 0)
#define CLKMGR_STAT_MAINPLLLOCKED(x)			(((x) & 0x00000100) >> 8)
#define CLKMGR_STAT_PERPLLLOCKED(x)			(((x) & 0x00010000) >> 16)
#define CLKMGR_INTRCLR_MAINLOCKLOST_SET_MSK		0x00000004
#define CLKMGR_INTRCLR_PERLOCKLOST_SET_MSK		0x00000008
#define CLKMGR_INTOSC_HZ				460000000

/* Main PLL Macros */
#define CLKMGR_MAINPLL_EN_RESET				0x0000005e
#define CLKMGR_MAINPLL_ENS_RESET			0x0000005e

/* Peripheral PLL Macros */
#define CLKMGR_PERPLL_EN_RESET				0x040007FF
#define CLKMGR_PERPLL_ENS_RESET			0x040007FF

#define CLKMGR_PERPLL_EN_SDMMCCLK			BIT(5)
#define CLKMGR_PERPLL_GPIODIV_GPIODBCLK_SET(x)		(((x) << 0) & 0x0000ffff)

/* Altera Macros */
#define CLKMGR_ALTERA_EXTCNTRST_RESET			0xff

/* Shared Macros */
#define CLKMGR_PSRC(x)					(((x) & 0x00030000) >> 16)
#define CLKMGR_PSRC_MAIN				0
#define CLKMGR_PSRC_PER					1

#define CLKMGR_PLLGLOB_PSRC_EOSC1			0x0
#define CLKMGR_PLLGLOB_PSRC_INTOSC			0x1
#define CLKMGR_PLLGLOB_PSRC_F2S				0x2

#define CLKMGR_PLLM_MDIV(x)				((x) & 0x000003ff)
#define CLKMGR_PLLGLOB_PD_SET_MSK			0x00000001
#define CLKMGR_PLLGLOB_RST_SET_MSK			0x00000002

#define CLKMGR_PLLGLOB_REFCLKDIV(x)			(((x) & 0x00003f00) >> 8)
#define CLKMGR_PLLGLOB_AREFCLKDIV(x)			(((x) & 0x00000f00) >> 8)
#define CLKMGR_PLLGLOB_DREFCLKDIV(x)			(((x) & 0x00003000) >> 12)

#define CLKMGR_VCOCALIB_HSCNT_SET(x)			(((x) << 0) & 0x000003ff)
#define CLKMGR_VCOCALIB_MSCNT_SET(x)			(((x) << 16) & 0x00ff0000)

#define CLKMGR_CLR_LOSTLOCK_BYPASS			0x20000000

typedef struct {
	uint32_t  clk_freq_of_eosc1;
	uint32_t  clk_freq_of_f2h_free;
	uint32_t  clk_freq_of_cb_intosc_ls;
} CLOCK_SOURCE_CONFIG;

int config_clkmgr_handoff(handoff *hoff_ptr);
uint32_t get_wdt_clk(void);
uint32_t get_uart_clk(void);
uint32_t get_mmc_clk(void);

#endif
