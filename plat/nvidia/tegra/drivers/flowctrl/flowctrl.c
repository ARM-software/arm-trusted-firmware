/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <cortex_a53.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>

#include <flowctrl.h>
#include <lib/utils_def.h>
#include <pmc.h>
#include <tegra_def.h>

#define CLK_RST_DEV_L_SET		0x300
#define CLK_RST_DEV_L_CLR		0x304
#define  CLK_BPMP_RST			(1 << 1)

#define EVP_BPMP_RESET_VECTOR		0x200

static const uint64_t flowctrl_offset_cpu_csr[4] = {
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CPU0_CSR),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CPU1_CSR),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CPU1_CSR + 8),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CPU1_CSR + 16)
};

static const uint64_t flowctrl_offset_halt_cpu[4] = {
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_HALT_CPU0_EVENTS),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_HALT_CPU1_EVENTS),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_HALT_CPU1_EVENTS + 8),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_HALT_CPU1_EVENTS + 16)
};

static const uint64_t flowctrl_offset_cc4_ctrl[4] = {
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CC4_CORE0_CTRL),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CC4_CORE0_CTRL + 4),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CC4_CORE0_CTRL + 8),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CC4_CORE0_CTRL + 12)
};

static inline void tegra_fc_cc4_ctrl(int cpu_id, uint32_t val)
{
	mmio_write_32(flowctrl_offset_cc4_ctrl[cpu_id], val);
	val = mmio_read_32(flowctrl_offset_cc4_ctrl[cpu_id]);
}

static inline void tegra_fc_cpu_csr(int cpu_id, uint32_t val)
{
	mmio_write_32(flowctrl_offset_cpu_csr[cpu_id], val);
	val = mmio_read_32(flowctrl_offset_cpu_csr[cpu_id]);
}

static inline void tegra_fc_halt_cpu(int cpu_id, uint32_t val)
{
	mmio_write_32(flowctrl_offset_halt_cpu[cpu_id], val);
	val = mmio_read_32(flowctrl_offset_halt_cpu[cpu_id]);
}

static void tegra_fc_prepare_suspend(int cpu_id, uint32_t csr)
{
	uint32_t val;

	val = FLOWCTRL_HALT_GIC_IRQ | FLOWCTRL_HALT_GIC_FIQ |
	      FLOWCTRL_HALT_LIC_IRQ | FLOWCTRL_HALT_LIC_FIQ |
	      FLOWCTRL_WAITEVENT;
	tegra_fc_halt_cpu(cpu_id, val);

	val = FLOWCTRL_CSR_INTR_FLAG | FLOWCTRL_CSR_EVENT_FLAG |
	      FLOWCTRL_CSR_ENABLE | (FLOWCTRL_WAIT_WFI_BITMAP << cpu_id);
	tegra_fc_cpu_csr(cpu_id, val | csr);
}

/*******************************************************************************
 * After this, no core can wake from C7 until the action is reverted.
 * If a wake up event is asserted, the FC state machine will stall until
 * the action is reverted.
 ******************************************************************************/
void tegra_fc_ccplex_pgexit_lock(void)
{
	unsigned int i, cpu = read_mpidr() & MPIDR_CPU_MASK;
	uint32_t flags = tegra_fc_read_32(FLOWCTRL_FC_SEQ_INTERCEPT) & ~INTERCEPT_IRQ_PENDING;;
	uint32_t icept_cpu_flags[] = {
		INTERCEPT_EXIT_PG_CORE0,
		INTERCEPT_EXIT_PG_CORE1,
		INTERCEPT_EXIT_PG_CORE2,
		INTERCEPT_EXIT_PG_CORE3
	};

	/* set the intercept flags */
	for (i = 0; i < ARRAY_SIZE(icept_cpu_flags); i++) {

		/* skip current CPU */
		if (i == cpu)
			continue;

		/* enable power gate exit intercept locks */
		flags |= icept_cpu_flags[i];
	}

	tegra_fc_write_32(FLOWCTRL_FC_SEQ_INTERCEPT, flags);
	(void)tegra_fc_read_32(FLOWCTRL_FC_SEQ_INTERCEPT);
}

/*******************************************************************************
 * Revert the ccplex powergate exit locks
 ******************************************************************************/
void tegra_fc_ccplex_pgexit_unlock(void)
{
	/* clear lock bits, clear pending interrupts */
	tegra_fc_write_32(FLOWCTRL_FC_SEQ_INTERCEPT, INTERCEPT_IRQ_PENDING);
	(void)tegra_fc_read_32(FLOWCTRL_FC_SEQ_INTERCEPT);
}

/*******************************************************************************
 * Powerdn the current CPU
 ******************************************************************************/
void tegra_fc_cpu_powerdn(uint32_t mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;

	VERBOSE("CPU%d powering down...\n", cpu);
	tegra_fc_prepare_suspend(cpu, 0);
}

/*******************************************************************************
 * Suspend the current CPU cluster
 ******************************************************************************/
void tegra_fc_cluster_idle(uint32_t mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;
	uint32_t val;

	VERBOSE("Entering cluster idle state...\n");

	tegra_fc_cc4_ctrl(cpu, 0);

	/* hardware L2 flush is faster for A53 only */
	tegra_fc_write_32(FLOWCTRL_L2_FLUSH_CONTROL,
		!!MPIDR_AFFLVL1_VAL(mpidr));

	/* suspend the CPU cluster */
	val = FLOWCTRL_PG_CPU_NONCPU << FLOWCTRL_ENABLE_EXT;
	tegra_fc_prepare_suspend(cpu, val);
}

/*******************************************************************************
 * Power down the current CPU cluster
 ******************************************************************************/
void tegra_fc_cluster_powerdn(uint32_t mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;
	uint32_t val;

	VERBOSE("Entering cluster powerdn state...\n");

	tegra_fc_cc4_ctrl(cpu, 0);

	/* hardware L2 flush is faster for A53 only */
	tegra_fc_write_32(FLOWCTRL_L2_FLUSH_CONTROL,
		read_midr() == CORTEX_A53_MIDR);

	/* power down the CPU cluster */
	val = FLOWCTRL_TURNOFF_CPURAIL << FLOWCTRL_ENABLE_EXT;
	tegra_fc_prepare_suspend(cpu, val);
}

/*******************************************************************************
 * Check if cluster idle or power down state is allowed from this CPU
 ******************************************************************************/
bool tegra_fc_is_ccx_allowed(void)
{
	unsigned int i, cpu = read_mpidr() & MPIDR_CPU_MASK;
	uint32_t val;
	bool ccx_allowed = true;

	for (i = 0; i < ARRAY_SIZE(flowctrl_offset_cpu_csr); i++) {

		/* skip current CPU */
		if (i == cpu)
			continue;

		/* check if all other CPUs are already halted */
		val = mmio_read_32(flowctrl_offset_cpu_csr[i]);
		if ((val & FLOWCTRL_CSR_HALT_MASK) == 0U) {
			ccx_allowed = false;
		}
	}

	return ccx_allowed;
}

/*******************************************************************************
 * Suspend the entire SoC
 ******************************************************************************/
void tegra_fc_soc_powerdn(uint32_t mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;
	uint32_t val;

	VERBOSE("Entering SoC powerdn state...\n");

	tegra_fc_cc4_ctrl(cpu, 0);

	tegra_fc_write_32(FLOWCTRL_L2_FLUSH_CONTROL, 1);

	val = FLOWCTRL_TURNOFF_CPURAIL << FLOWCTRL_ENABLE_EXT;
	tegra_fc_prepare_suspend(cpu, val);

	/* overwrite HALT register */
	tegra_fc_halt_cpu(cpu, FLOWCTRL_WAITEVENT);
}

/*******************************************************************************
 * Power up the CPU
 ******************************************************************************/
void tegra_fc_cpu_on(int cpu)
{
	tegra_fc_cpu_csr(cpu, FLOWCTRL_CSR_ENABLE);
	tegra_fc_halt_cpu(cpu, FLOWCTRL_WAITEVENT | FLOWCTRL_HALT_SCLK);
}

/*******************************************************************************
 * Power down the CPU
 ******************************************************************************/
void tegra_fc_cpu_off(int cpu)
{
	uint32_t val;

	/*
	 * Flow controller powers down the CPU during wfi. The CPU would be
	 * powered on when it receives any interrupt.
	 */
	val = FLOWCTRL_CSR_INTR_FLAG | FLOWCTRL_CSR_EVENT_FLAG |
		FLOWCTRL_CSR_ENABLE | (FLOWCTRL_WAIT_WFI_BITMAP << cpu);
	tegra_fc_cpu_csr(cpu, val);
	tegra_fc_halt_cpu(cpu, FLOWCTRL_WAITEVENT);
	tegra_fc_cc4_ctrl(cpu, 0);
}

/*******************************************************************************
 * Inform the BPMP that we have completed the cluster power up
 ******************************************************************************/
void tegra_fc_lock_active_cluster(void)
{
	uint32_t val;

	val = tegra_fc_read_32(FLOWCTRL_BPMP_CLUSTER_CONTROL);
	val |= FLOWCTRL_BPMP_CLUSTER_PWRON_LOCK;
	tegra_fc_write_32(FLOWCTRL_BPMP_CLUSTER_CONTROL, val);
	val = tegra_fc_read_32(FLOWCTRL_BPMP_CLUSTER_CONTROL);
}

/*******************************************************************************
 * Power ON BPMP processor
 ******************************************************************************/
void tegra_fc_bpmp_on(uint32_t entrypoint)
{
	/* halt BPMP */
	tegra_fc_write_32(FLOWCTRL_HALT_BPMP_EVENTS, FLOWCTRL_WAITEVENT);

	/* Assert BPMP reset */
	mmio_write_32(TEGRA_CAR_RESET_BASE + CLK_RST_DEV_L_SET, CLK_BPMP_RST);

	/* Set reset address (stored in PMC_SCRATCH39) */
	mmio_write_32(TEGRA_EVP_BASE + EVP_BPMP_RESET_VECTOR, entrypoint);
	while (entrypoint != mmio_read_32(TEGRA_EVP_BASE + EVP_BPMP_RESET_VECTOR))
		; /* wait till value reaches EVP_BPMP_RESET_VECTOR */

	/* Wait for 2us before de-asserting the reset signal. */
	udelay(2);

	/* De-assert BPMP reset */
	mmio_write_32(TEGRA_CAR_RESET_BASE + CLK_RST_DEV_L_CLR, CLK_BPMP_RST);

	/* Un-halt BPMP */
	tegra_fc_write_32(FLOWCTRL_HALT_BPMP_EVENTS, 0);
}

/*******************************************************************************
 * Power OFF BPMP processor
 ******************************************************************************/
void tegra_fc_bpmp_off(void)
{
	/* halt BPMP */
	tegra_fc_write_32(FLOWCTRL_HALT_BPMP_EVENTS, FLOWCTRL_WAITEVENT);

	/* Assert BPMP reset */
	mmio_write_32(TEGRA_CAR_RESET_BASE + CLK_RST_DEV_L_SET, CLK_BPMP_RST);

	/* Clear reset address */
	mmio_write_32(TEGRA_EVP_BASE + EVP_BPMP_RESET_VECTOR, 0);
	while (0 != mmio_read_32(TEGRA_EVP_BASE + EVP_BPMP_RESET_VECTOR))
		; /* wait till value reaches EVP_BPMP_RESET_VECTOR */
}

/*******************************************************************************
 * Route legacy FIQ to the GICD
 ******************************************************************************/
void tegra_fc_enable_fiq_to_ccplex_routing(void)
{
	uint32_t val = tegra_fc_read_32(FLOW_CTLR_FLOW_DBG_QUAL);

	/* set the bit to pass FIQs to the GICD */
	tegra_fc_write_32(FLOW_CTLR_FLOW_DBG_QUAL, val | FLOWCTRL_FIQ2CCPLEX_ENABLE);
}

/*******************************************************************************
 * Disable routing legacy FIQ to the GICD
 ******************************************************************************/
void tegra_fc_disable_fiq_to_ccplex_routing(void)
{
	uint32_t val = tegra_fc_read_32(FLOW_CTLR_FLOW_DBG_QUAL);

	/* clear the bit to pass FIQs to the GICD */
	tegra_fc_write_32(FLOW_CTLR_FLOW_DBG_QUAL, val & ~FLOWCTRL_FIQ2CCPLEX_ENABLE);
}
