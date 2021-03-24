/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <common/debug.h>
#include <ddr.h>

static void cal_ddr_sdram_clk_cntl(struct ddr_cfg_regs *regs,
					 const struct memctl_opt *popts)
{
	const unsigned int clk_adj = popts->clk_adj;
	const unsigned int ss_en = 0U;

	regs->clk_cntl = ((ss_en & U(0x1)) << 31U)		|
				  ((clk_adj & U(0x1F)) << 22U);
	debug("clk_cntl = 0x%x\n", regs->clk_cntl);
}

static void cal_ddr_cdr(struct ddr_cfg_regs *regs,
			const struct memctl_opt *popts)
{
	regs->cdr[0] = popts->ddr_cdr1;
	regs->cdr[1] = popts->ddr_cdr2;
	debug("cdr[0] = 0x%x\n", regs->cdr[0]);
	debug("cdr[1] = 0x%x\n", regs->cdr[1]);
}

static void cal_ddr_wrlvl_cntl(struct ddr_cfg_regs *regs,
				const struct memctl_opt *popts)
{
	const unsigned int wrlvl_en = 1U;	/* enabled */
	const unsigned int wrlvl_mrd = U(0x6);	/* > 40nCK */
	const unsigned int wrlvl_odten = U(0x7);	/* 128 */
	const unsigned int wrlvl_dqsen = U(0x5);	/* > 25nCK */
	const unsigned int wrlvl_wlr = U(0x6);	/* > tWLO + 6 */
	const unsigned int wrlvl_smpl = popts->wrlvl_override ?
					popts->wrlvl_sample : U(0xf);
	const unsigned int wrlvl_start = popts->wrlvl_start;

	regs->wrlvl_cntl[0] = ((wrlvl_en & U(0x1)) << 31U)	|
				  ((wrlvl_mrd & U(0x7)) << 24U)	|
				  ((wrlvl_odten & U(0x7)) << 20U)	|
				  ((wrlvl_dqsen & U(0x7)) << 16U)	|
				  ((wrlvl_smpl & U(0xf)) << 12U)	|
				  ((wrlvl_wlr & U(0x7)) << 8U)	|
				  ((wrlvl_start & U(0x1F)) << 0U);
	regs->wrlvl_cntl[1] = popts->wrlvl_ctl_2;
	regs->wrlvl_cntl[2] = popts->wrlvl_ctl_3;
	debug("wrlvl_cntl[0] = 0x%x\n", regs->wrlvl_cntl[0]);
	debug("wrlvl_cntl[1] = 0x%x\n", regs->wrlvl_cntl[1]);
	debug("wrlvl_cntl[2] = 0x%x\n", regs->wrlvl_cntl[2]);

}

static void cal_ddr_dbg(struct ddr_cfg_regs *regs,
			const struct memctl_opt *popts)
{
	if (popts->cswl_override != 0) {
		regs->debug[18] = popts->cswl_override;
	}

#ifdef CONFIG_SYS_FSL_DDR_EMU
	/* disable DDR training for emulator */
	regs->debug[2] = U(0x00000400);
	regs->debug[4] = U(0xff800800);
	regs->debug[5] = U(0x08000800);
	regs->debug[6] = U(0x08000800);
	regs->debug[7] = U(0x08000800);
	regs->debug[8] = U(0x08000800);
#endif
	if (popts->cpo_sample != 0U) {
		regs->debug[28] = popts->cpo_sample;
		debug("debug[28] = 0x%x\n", regs->debug[28]);
	}
}

int compute_ddr_phy(struct ddr_info *priv)
{
	const struct memctl_opt *popts = &priv->opt;
	struct ddr_cfg_regs *regs = &priv->ddr_reg;

	cal_ddr_sdram_clk_cntl(regs, popts);
	cal_ddr_cdr(regs, popts);
	cal_ddr_wrlvl_cntl(regs, popts);
	cal_ddr_dbg(regs, popts);

	return 0;
}
