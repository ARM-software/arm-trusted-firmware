/*
 * Copyright (c) 2020-2026, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_CPUCFG_H
#define SUNXI_CPUCFG_H

#include <plat/common/platform.h>
#include <sunxi_mmap.h>

/* c = cluster, n = core */
#define SUNXI_CPUCFG_CLS_CTRL_REG0(c)	(SUNXI_CPUCFG_BASE + 0x0010)
#define SUNXI_CPUCFG_CLS_CTRL_REG1(c)	(SUNXI_CPUCFG_BASE + 0x0014)
#define SUNXI_CPUCFG_CACHE_CFG_REG	(SUNXI_CPUCFG_BASE + 0x0024)
#define SUNXI_CPUCFG_DBG_REG0		(SUNXI_CPUCFG_BASE + 0x00c0)

#define SUNXI_CPUCFG_RST_CTRL_REG(c)	(SUNXI_CPUCFG_BASE + 0x0000)
#define SUNXI_CPUCFG_GEN_CTRL_REG0(c)	(SUNXI_CPUSUBSYS_BASE + 0x0000)

#define SUNXI_C0_CPU_CTRL_REG(n)	(SUNXI_CPUCFG_BASE + 0x0060 + (n) * 4)

#define SUNXI_CPU_CTRL_REG(n)		(SUNXI_CPUSUBSYS_BASE + 0x20 + (n) * 4)
#define SUNXI_ALT_RVBAR_LO_REG(n)	(SUNXI_CPUSUBSYS_BASE + 0x40 + (n) * 8)
#define SUNXI_ALT_RVBAR_HI_REG(n)	(SUNXI_CPUSUBSYS_BASE + 0x44 + (n) * 8)
#define SUNXI_CPUCFG_RVBAR_LO_REG	SUNXI_ALT_RVBAR_LO_REG
#define SUNXI_CPUCFG_RVBAR_HI_REG	SUNXI_ALT_RVBAR_HI_REG

#define SUNXI_POWERON_RST_REG(c)	(SUNXI_R_CPUCFG_BASE + 0x0040 + (c) * 4)
#define SUNXI_POWEROFF_GATING_REG(c)	(SUNXI_R_CPUCFG_BASE + 0x0044 + (c) * 4)
#define SUNXI_CPU_POWER_CLAMP_REG(c, n)	(SUNXI_R_CPUCFG_BASE + 0x0050 + \
					(c) * 0x10 + (n) * 4)
#define SUNXI_CPU_UNK_REG(n)		(SUNXI_R_CPUCFG_BASE + 0x0070 + (n) * 4)

#define SUNXI_CPUIDLE_EN_REG		(SUNXI_R_CPUCFG_BASE + 0x0100)
#define SUNXI_CORE_CLOSE_REG		(SUNXI_R_CPUCFG_BASE + 0x0104)

#define SUNXI_AA64nAA32_REG		SUNXI_CPUCFG_GEN_CTRL_REG0
#define SUNXI_AA64nAA32_OFFSET		4

static inline bool sunxi_cpucfg_has_per_cluster_regs(void)
{
	return plat_get_soc_revision() < 5;
}

#endif /* SUNXI_CPUCFG_H */
