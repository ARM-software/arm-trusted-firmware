/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FLOWCTRL_H
#define FLOWCTRL_H

#include <lib/mmio.h>

#include <tegra_def.h>

#define FLOWCTRL_HALT_CPU0_EVENTS	(0x0U)
#define  FLOWCTRL_WAITEVENT		(2U << 29)
#define  FLOWCTRL_WAIT_FOR_INTERRUPT	(4U << 29)
#define  FLOWCTRL_JTAG_RESUME		(1U << 28)
#define  FLOWCTRL_HALT_SCLK		(1U << 27)
#define  FLOWCTRL_HALT_LIC_IRQ		(1U << 11)
#define  FLOWCTRL_HALT_LIC_FIQ		(1U << 10)
#define  FLOWCTRL_HALT_GIC_IRQ		(1U << 9)
#define  FLOWCTRL_HALT_GIC_FIQ		(1U << 8)
#define FLOWCTRL_HALT_BPMP_EVENTS	(0x4U)
#define FLOWCTRL_CPU0_CSR		(0x8U)
#define  FLOWCTRL_CSR_HALT_MASK		(1U << 22)
#define  FLOWCTRL_CSR_PWR_OFF_STS	(1U << 16)
#define  FLOWCTRL_CSR_INTR_FLAG		(1U << 15)
#define  FLOWCTRL_CSR_EVENT_FLAG	(1U << 14)
#define  FLOWCTRL_CSR_IMMEDIATE_WAKE	(1U << 3)
#define  FLOWCTRL_CSR_ENABLE		(1U << 0)
#define FLOWCTRL_HALT_CPU1_EVENTS	(0x14U)
#define FLOWCTRL_CPU1_CSR		(0x18U)
#define FLOW_CTLR_FLOW_DBG_QUAL		(0x50U)
#define  FLOWCTRL_FIQ2CCPLEX_ENABLE	(1U << 28)
#define FLOWCTRL_FC_SEQ_INTERCEPT	(0x5cU)
#define  INTERCEPT_IRQ_PENDING		(0xffU)
#define  INTERCEPT_HVC			(U(1) << 21)
#define  INTERCEPT_ENTRY_CC4		(U(1) << 20)
#define  INTERCEPT_ENTRY_PG_NONCPU	(U(1) << 19)
#define  INTERCEPT_EXIT_PG_NONCPU	(U(1) << 18)
#define  INTERCEPT_ENTRY_RG_CPU		(U(1) << 17)
#define  INTERCEPT_EXIT_RG_CPU		(U(1) << 16)
#define  INTERCEPT_ENTRY_PG_CORE0	(U(1) << 15)
#define  INTERCEPT_EXIT_PG_CORE0	(U(1) << 14)
#define  INTERCEPT_ENTRY_PG_CORE1	(U(1) << 13)
#define  INTERCEPT_EXIT_PG_CORE1	(U(1) << 12)
#define  INTERCEPT_ENTRY_PG_CORE2	(U(1) << 11)
#define  INTERCEPT_EXIT_PG_CORE2	(U(1) << 10)
#define  INTERCEPT_ENTRY_PG_CORE3	(U(1) << 9)
#define  INTERCEPT_EXIT_PG_CORE3	(U(1) << 8)
#define  INTERRUPT_PENDING_NONCPU	(U(1) << 7)
#define  INTERRUPT_PENDING_CRAIL	(U(1) << 6)
#define  INTERRUPT_PENDING_CORE0	(U(1) << 5)
#define  INTERRUPT_PENDING_CORE1	(U(1) << 4)
#define  INTERRUPT_PENDING_CORE2	(U(1) << 3)
#define  INTERRUPT_PENDING_CORE3	(U(1) << 2)
#define  CC4_INTERRUPT_PENDING		(U(1) << 1)
#define  HVC_INTERRUPT_PENDING		(U(1) << 0)
#define FLOWCTRL_CC4_CORE0_CTRL		(0x6cU)
#define FLOWCTRL_WAIT_WFI_BITMAP	(0x100U)
#define FLOWCTRL_L2_FLUSH_CONTROL	(0x94U)
#define FLOWCTRL_BPMP_CLUSTER_CONTROL	(0x98U)
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

void tegra_fc_bpmp_on(uint32_t entrypoint);
void tegra_fc_bpmp_off(void);
void tegra_fc_ccplex_pgexit_lock(void);
void tegra_fc_ccplex_pgexit_unlock(void);
void tegra_fc_cluster_idle(uint32_t midr);
void tegra_fc_cpu_powerdn(uint32_t mpidr);
void tegra_fc_cluster_powerdn(uint32_t midr);
void tegra_fc_cpu_on(int cpu);
void tegra_fc_cpu_off(int cpu);
void tegra_fc_disable_fiq_to_ccplex_routing(void);
void tegra_fc_enable_fiq_to_ccplex_routing(void);
bool tegra_fc_is_ccx_allowed(void);
void tegra_fc_lock_active_cluster(void);
void tegra_fc_soc_powerdn(uint32_t midr);

#endif /* FLOWCTRL_H */
