/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <common/debug.h>
#include <errno.h>
#include <platform_def.h>
#include <io.h>
#include <soc.h>
#include "ddr.h"

static void cal_ddr_sdram_clk_cntl(struct ddr_cfg_regs *regs,
					 const struct memctl_opt *popts)
{
	const unsigned int clk_adj = popts->clk_adj;
	const unsigned int ss_en = 0;

	regs->clk_cntl = ((ss_en & 0x1) << 31)		|
				  ((clk_adj & 0x1F) << 22);
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
	const unsigned int wrlvl_en = 1;	/* enabled */
	const unsigned int wrlvl_mrd = 0x6;	/* > 40nCK */
	const unsigned int wrlvl_odten = 0x7;	/* 128 */
	const unsigned int wrlvl_dqsen = 0x5;	/* > 25nCK */
	const unsigned int wrlvl_wlr = 0x6;	/* > tWLO + 6 */
	const unsigned int wrlvl_smpl = popts->wrlvl_override ?
					popts->wrlvl_sample : 0xf;
	const unsigned int wrlvl_start = popts->wrlvl_start;

	regs->wrlvl_cntl[0] = ((wrlvl_en & 0x1) << 31)	|
				  ((wrlvl_mrd & 0x7) << 24)	|
				  ((wrlvl_odten & 0x7) << 20)	|
				  ((wrlvl_dqsen & 0x7) << 16)	|
				  ((wrlvl_smpl & 0xf) << 12)	|
				  ((wrlvl_wlr & 0x7) << 8)	|
				  ((wrlvl_start & 0x1F) << 0);
	regs->wrlvl_cntl[1] = popts->wrlvl_ctl_2;
	regs->wrlvl_cntl[2] = popts->wrlvl_ctl_3;
	debug("wrlvl_cntl[0] = 0x%x\n", regs->wrlvl_cntl[0]);
	debug("wrlvl_cntl[1] = 0x%x\n", regs->wrlvl_cntl[1]);
	debug("wrlvl_cntl[2] = 0x%x\n", regs->wrlvl_cntl[2]);

}

static void cal_ddr_dbg(struct ddr_cfg_regs *regs,
			const struct memctl_opt *popts)
{
	if (popts->cswl_override)
		regs->debug[18] = popts->cswl_override;

#ifdef CONFIG_SYS_FSL_DDR_EMU
	/* disable DDR training for emulator */
	regs->debug[2] = 0x00000400;
	regs->debug[4] = 0xff800800;
	regs->debug[5] = 0x08000800;
	regs->debug[6] = 0x08000800;
	regs->debug[7] = 0x08000800;
	regs->debug[8] = 0x08000800;
#endif
	if (popts->cpo_sample) {
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
