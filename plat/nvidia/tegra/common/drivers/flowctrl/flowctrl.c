/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <delay_timer.h>
#include <mmio.h>
#include <pmc.h>
#include <cortex_a53.h>
#include <flowctrl.h>
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
 * Reset BPMP processor
 ******************************************************************************/
void tegra_fc_reset_bpmp(void)
{
	uint32_t val;

	/* halt BPMP */
	tegra_fc_write_32(FLOWCTRL_HALT_BPMP_EVENTS, FLOWCTRL_WAITEVENT);

	/* Assert BPMP reset */
	mmio_write_32(TEGRA_CAR_RESET_BASE + CLK_RST_DEV_L_SET, CLK_BPMP_RST);

	/* Restore reset address (stored in PMC_SCRATCH39) */
	val = tegra_pmc_read_32(PMC_SCRATCH39);
	mmio_write_32(TEGRA_EVP_BASE + EVP_BPMP_RESET_VECTOR, val);
	while (val != mmio_read_32(TEGRA_EVP_BASE + EVP_BPMP_RESET_VECTOR))
		; /* wait till value reaches EVP_BPMP_RESET_VECTOR */

	/* Wait for 2us before de-asserting the reset signal. */
	udelay(2);

	/* De-assert BPMP reset */
	mmio_write_32(TEGRA_CAR_RESET_BASE + CLK_RST_DEV_L_CLR, CLK_BPMP_RST);

	/* Un-halt BPMP */
	tegra_fc_write_32(FLOWCTRL_HALT_BPMP_EVENTS, 0);
}
