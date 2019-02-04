/*
 * Copyright (c) 2019, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef	_HANDOFF_H_
#define	_HANDOFF_H_

#define HANDOFF_MAGIC_HEADER	0x424f4f54	/* BOOT */
#define HANDOFF_MAGIC_PINMUX_SEL	0x504d5558	/* PMUX */
#define HANDOFF_MAGIC_IOCTLR	0x494f4354	/* IOCT */
#define HANDOFF_MAGIC_FPGA		0x46504741	/* FPGA */
#define HANDOFF_MAGIC_IODELAY	0x444c4159	/* DLAY */
#define HANDOFF_MAGIC_CLOCK		0x434c4b53	/* CLKS */
#define HANDOFF_MAGIC_MISC		0x4d495343	/* MISC */

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
	uint32_t	pinmux_fpga_array[42];	/* offset, value */
	uint32_t	_pad_0x3e8_0x3f0[2];

	/* pinmux configuration - io delay */
	uint32_t	pinmux_delay_magic;
	uint32_t	pinmux_delay_length;
	uint32_t	_pad_0x3f8_0x400[2];
	uint32_t	pinmux_iodelay_array[96];	/* offset, value */

	/* clock configuration */
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

	/* misc configuration */
	uint32_t	misc_magic;
	uint32_t	misc_length;
	uint32_t	_pad_0x618_0x620[2];
	uint32_t	boot_source;
} handoff;

int verify_handoff_image(handoff *hoff_ptr, handoff *reverse_hoff_ptr);
int s10_get_handoff(handoff *hoff_ptr);

#endif


