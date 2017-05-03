/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __FLOWCTRL_H__
#define __FLOWCTRL_H__

#include <mmio.h>
#include <tegra_def.h>

#define FLOWCTRL_HALT_CPU0_EVENTS	0x0U
#define  FLOWCTRL_WAITEVENT		(2U << 29)
#define  FLOWCTRL_WAIT_FOR_INTERRUPT	(4U << 29)
#define  FLOWCTRL_JTAG_RESUME		(1U << 28)
#define  FLOWCTRL_HALT_SCLK		(1U << 27)
#define  FLOWCTRL_HALT_LIC_IRQ		(1U << 11)
#define  FLOWCTRL_HALT_LIC_FIQ		(1U << 10)
#define  FLOWCTRL_HALT_GIC_IRQ		(1U << 9)
#define  FLOWCTRL_HALT_GIC_FIQ		(1U << 8)
#define FLOWCTRL_HALT_BPMP_EVENTS	0x4U
#define FLOWCTRL_CPU0_CSR		0x8U
#define  FLOW_CTRL_CSR_PWR_OFF_STS	(1U << 16)
#define  FLOWCTRL_CSR_INTR_FLAG		(1U << 15)
#define  FLOWCTRL_CSR_EVENT_FLAG	(1U << 14)
#define  FLOWCTRL_CSR_IMMEDIATE_WAKE	(1U << 3)
#define  FLOWCTRL_CSR_ENABLE		(1U << 0)
#define FLOWCTRL_HALT_CPU1_EVENTS	0x14U
#define FLOWCTRL_CPU1_CSR		0x18U
#define FLOWCTRL_CC4_CORE0_CTRL		0x6cU
#define FLOWCTRL_WAIT_WFI_BITMAP	0x100U
#define FLOWCTRL_L2_FLUSH_CONTROL	0x94U
#define FLOWCTRL_BPMP_CLUSTER_CONTROL	0x98U
#define  FLOWCTRL_BPMP_CLUSTER_PWRON_LOCK	(1U << 2)

#define FLOWCTRL_ENABLE_EXT		12U
#define FLOWCTRL_ENABLE_EXT_MASK	3U
#define FLOWCTRL_PG_CPU_NONCPU		0x1U
#define FLOWCTRL_TURNOFF_CPURAIL	0x2U

static inline uint32_t tegra_fc_read_32(uint32_t off)
{
	return mmio_read_32(TEGRA_FLOWCTRL_BASE + off);
}

static inline void tegra_fc_write_32(uint32_t off, uint32_t val)
{
	mmio_write_32(TEGRA_FLOWCTRL_BASE + off, val);
}

void tegra_fc_cluster_idle(uint32_t midr);
void tegra_fc_cpu_powerdn(uint32_t mpidr);
void tegra_fc_cluster_powerdn(uint32_t midr);
void tegra_fc_soc_powerdn(uint32_t midr);
void tegra_fc_cpu_on(int cpu);
void tegra_fc_cpu_off(int cpu);
void tegra_fc_lock_active_cluster(void);
void tegra_fc_reset_bpmp(void);

#endif /* __FLOWCTRL_H__ */
