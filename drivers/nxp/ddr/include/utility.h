/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef UTILITY_H
#define UTILITY_H

#include <dcfg.h>

#if defined(NXP_HAS_CCN504) || defined(NXP_HAS_CCN508)
#define CCN_HN_F_SAM_CTL		0x8
#define CCN_HN_F_REGION_SIZE		0x10000
#endif

unsigned long get_ddr_freq(struct sysinfo *sys, int ctrl_num);
unsigned int get_memory_clk_ps(unsigned long clk);
unsigned int picos_to_mclk(unsigned long data_rate, unsigned int picos);
unsigned int get_ddrc_version(const struct ccsr_ddr *ddr);
void print_ddr_info(struct ccsr_ddr *ddr);

#endif
