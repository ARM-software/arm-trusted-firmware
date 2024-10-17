/*
 * Copyright (c) 2019-2023, Intel Corporation. All rights reserved.
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef	HANDOFF_H
#define	HANDOFF_H

#define HANDOFF_MAGIC_HEADER			0x424f4f54	/* BOOT */
#define HANDOFF_MAGIC_PINMUX_SEL		0x504d5558	/* PMUX */
#define HANDOFF_MAGIC_IOCTLR			0x494f4354	/* IOCT */
#define HANDOFF_MAGIC_FPGA			0x46504741	/* FPGA */
#define HANDOFF_MAGIC_IODELAY			0x444c4159	/* DLAY */
#define HANDOFF_MAGIC_CLOCK			0x434c4b53	/* CLKS */
#define HANDOFF_MAGIC_MISC			0x4d495343	/* MISC */
#define HANDOFF_MAGIC_PERIPHERAL		0x50455249	/* PERIPHERAL */
#define HANDOFF_MAGIC_DDR			0x5344524d	/* DDR */

#include <socfpga_plat_def.h>

typedef struct handoff_t {
	/* header */
	uint32_t	header_magic;
	uint32_t	header_device;
	uint32_t	_pad_0x08_0x10[2];

	/* pinmux configuration - select */
	uint32_t	pinmux_sel_magic;
	uint32_t	pinmux_sel_length;
	uint32_t	_pad_0x18_0x20[2];
	uint32_t	pinmux_sel_array[96];	/* offset, value */

	/* pinmux configuration - io control */
	uint32_t	pinmux_io_magic;
	uint32_t	pinmux_io_length;
	uint32_t	_pad_0x1a8_0x1b0[2];
	uint32_t	pinmux_io_array[96];	/* offset, value */

	/* pinmux configuration - use fpga switch */
	uint32_t	pinmux_fpga_magic;
	uint32_t	pinmux_fpga_length;
	uint32_t	_pad_0x338_0x340[2];
	uint32_t	pinmux_fpga_array[44];	/* offset, value */
	/* TODO: Temp remove due to add in extra handoff data */
	// uint32_t	_pad_0x3e8_0x3f0[2];

	/* pinmux configuration - io delay */
	uint32_t	pinmux_delay_magic;
	uint32_t	pinmux_delay_length;
	uint32_t	_pad_0x3f8_0x400[2];
	uint32_t	pinmux_iodelay_array[96];	/* offset, value */

	/* clock configuration */
#if PLATFORM_MODEL == PLAT_SOCFPGA_STRATIX10
	uint32_t	clock_magic;
	uint32_t	clock_length;
	uint32_t	_pad_0x588_0x590[2];
	uint32_t	main_pll_mpuclk;
	uint32_t	main_pll_nocclk;
	uint32_t	main_pll_cntr2clk;
	uint32_t	main_pll_cntr3clk;
	uint32_t	main_pll_cntr4clk;
	uint32_t	main_pll_cntr5clk;
	uint32_t	main_pll_cntr6clk;
	uint32_t	main_pll_cntr7clk;
	uint32_t	main_pll_cntr8clk;
	uint32_t	main_pll_cntr9clk;
	uint32_t	main_pll_nocdiv;
	uint32_t	main_pll_pllglob;
	uint32_t	main_pll_fdbck;
	uint32_t	main_pll_pllc0;
	uint32_t	main_pll_pllc1;
	uint32_t	_pad_0x5cc_0x5d0[1];
	uint32_t	per_pll_cntr2clk;
	uint32_t	per_pll_cntr3clk;
	uint32_t	per_pll_cntr4clk;
	uint32_t	per_pll_cntr5clk;
	uint32_t	per_pll_cntr6clk;
	uint32_t	per_pll_cntr7clk;
	uint32_t	per_pll_cntr8clk;
	uint32_t	per_pll_cntr9clk;
	uint32_t	per_pll_emacctl;
	uint32_t	per_pll_gpiodiv;
	uint32_t	per_pll_pllglob;
	uint32_t	per_pll_fdbck;
	uint32_t	per_pll_pllc0;
	uint32_t	per_pll_pllc1;
	uint32_t	hps_osc_clk_h;
	uint32_t	fpga_clk_hz;
#elif PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX
	uint32_t	clock_magic;
	uint32_t	clock_length;
	uint32_t	_pad_0x588_0x590[2];
	uint32_t	main_pll_mpuclk;
	uint32_t	main_pll_nocclk;
	uint32_t	main_pll_nocdiv;
	uint32_t	main_pll_pllglob;
	uint32_t	main_pll_fdbck;
	uint32_t	main_pll_pllc0;
	uint32_t	main_pll_pllc1;
	uint32_t	main_pll_pllc2;
	uint32_t	main_pll_pllc3;
	uint32_t	main_pll_pllm;
	uint32_t	per_pll_emacctl;
	uint32_t	per_pll_gpiodiv;
	uint32_t	per_pll_pllglob;
	uint32_t	per_pll_fdbck;
	uint32_t	per_pll_pllc0;
	uint32_t	per_pll_pllc1;
	uint32_t	per_pll_pllc2;
	uint32_t	per_pll_pllc3;
	uint32_t	per_pll_pllm;
	uint32_t	alt_emacactr;
	uint32_t	alt_emacbctr;
	uint32_t	alt_emacptpctr;
	uint32_t	alt_gpiodbctr;
	uint32_t	alt_sdmmcctr;
	uint32_t	alt_s2fuser0ctr;
	uint32_t	alt_s2fuser1ctr;
	uint32_t	alt_psirefctr;
	uint32_t	hps_osc_clk_h;
	uint32_t	fpga_clk_hz;
	uint32_t	_pad_0x604_0x610[3];
#elif PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	uint32_t	clock_magic;
	uint32_t	clock_length;
	uint32_t	_pad_0x588_0x590[2];

	/* main group PLL */
	uint32_t	main_pll_nocclk;
	uint32_t	main_pll_nocdiv;
	uint32_t	main_pll_pllglob;
	uint32_t	main_pll_fdbck;
	uint32_t	main_pll_pllc0;
	uint32_t	main_pll_pllc1;
	uint32_t	main_pll_pllc2;
	uint32_t	main_pll_pllc3;
	uint32_t	main_pll_pllm;

	/* peripheral group PLL */
	uint32_t	per_pll_emacctl;
	uint32_t	per_pll_gpiodiv;
	uint32_t	per_pll_pllglob;
	uint32_t	per_pll_fdbck;
	uint32_t	per_pll_pllc0;
	uint32_t	per_pll_pllc1;
	uint32_t	per_pll_pllc2;
	uint32_t	per_pll_pllc3;
	uint32_t	per_pll_pllm;

	/* control group */
	uint32_t	alt_emacactr;
	uint32_t	alt_emacbctr;
	uint32_t	alt_emacptpctr;
	uint32_t	alt_gpiodbctr;
	uint32_t	alt_s2fuser0ctr;
	uint32_t	alt_s2fuser1ctr;
	uint32_t	alt_psirefctr;
	uint32_t	alt_usb31ctr;
	uint32_t	alt_dsuctr;
	uint32_t	alt_core01ctr;
	uint32_t	alt_core23ctr;
	uint32_t	alt_core2ctr;
	uint32_t	alt_core3ctr;
	uint32_t	hps_osc_clk_hz;
	uint32_t	fpga_clk_hz;
	uint32_t	_pad_0x604_0x610[3];
#endif

#if PLATFORM_MODEL == PLAT_SOCFPGA_AGILEX5
	/* peripheral configuration - select */
	uint32_t	peripheral_pwr_gate_magic;
	uint32_t	peripheral_pwr_gate_length;
	uint32_t	_pad_0x08_0x0C[2];
	uint32_t	peripheral_pwr_gate_array;	/* offset, value */

	/* ddr configuration - select */
	uint32_t	ddr_magic;
	uint32_t	ddr_length;
	uint32_t	_pad_0x1C_0x20[2];
	uint32_t	ddr_config;	/* BIT[0]-Dual Port. BIT[1]-Dual EMIF */
#endif
} handoff;

int verify_handoff_image(handoff *hoff_ptr, handoff *reverse_hoff_ptr);
int socfpga_get_handoff(handoff *hoff_ptr);

#endif
