/*
 * Copyright 2016-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef DDR_H
#define DDR_H

#include <platform_def.h>
#include "immap.h"
#include "dimm.h"

#ifndef DDRC_NUM_CS
#define DDRC_NUM_CS 4
#endif

#include "opts.h"
#include "regs.h"
#include "utility.h"

#ifdef DDR_DEBUG
#define debug(...) INFO(__VA_ARGS__)
#else
#define debug(...) VERBOSE(__VA_ARGS__)
#endif

#ifndef DDRC_NUM_DIMM
#define DDRC_NUM_DIMM 1
#endif
#define CONFIG_CS_PER_SLOT \
	(DDRC_NUM_CS / DDRC_NUM_DIMM)

struct ddr_conf {
	int dimm_in_use[DDRC_NUM_DIMM];
	int cs_in_use;	/* bitmask, bit 0 for cs0, bit 1 for cs1, etc. */
	int cs_on_dimm[DDRC_NUM_DIMM];	/* bitmask */
	unsigned long long cs_base_addr[DDRC_NUM_CS];
	unsigned long long cs_size[DDRC_NUM_CS];
	unsigned long long base_addr;
	unsigned long long total_mem;
};

struct ddr_info {
	unsigned long clk;
	unsigned long long mem_base;
	unsigned int num_ctlrs;
	unsigned int dimm_on_ctlr;
	struct dimm_params dimm;
	struct memctl_opt opt;
	struct ddr_conf conf;
	struct ddr_cfg_regs ddr_reg;
	struct ccsr_ddr *ddr[NUM_OF_DDRC];
	uint16_t *phy[NUM_OF_DDRC];
	int *spd_addr;
	unsigned int ip_rev;
};

struct rc_timing {
	unsigned int speed_bin;
	unsigned int clk_adj;
	unsigned int wrlvl;
};

struct board_timing {
	unsigned int rc;
	struct rc_timing const *p;
	unsigned int add1;
	unsigned int add2;
};

int disable_unused_ddrc(struct ddr_info *priv, int mask);
int ddr_board_options(struct ddr_info *priv);
int compute_ddrc(const unsigned long clk,
		 const struct memctl_opt *popts,
		 const struct ddr_conf *conf,
		 struct ddr_cfg_regs *ddr,
		 const struct dimm_params *dimm_params,
		 const unsigned int ip_rev);
int compute_ddr_phy(struct ddr_info *priv);
int ddrc_set_regs(const unsigned long clk,
		  const struct ddr_cfg_regs *regs,
		  const struct ccsr_ddr *ddr,
		  int twopass);
int cal_board_params(struct ddr_info *priv,
		     const struct board_timing *dimm,
		     int len);
/* return bit mask of used DIMM(s) */
int ddr_get_ddr_params(struct dimm_params *pdimm, struct ddr_conf *conf);
long long dram_init(struct ddr_info *info);
long long board_static_ddr(struct ddr_info *info);

#endif	/* DDR_H */
