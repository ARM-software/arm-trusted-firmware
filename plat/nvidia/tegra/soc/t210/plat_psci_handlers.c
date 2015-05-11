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
#include <mmio.h>
#include <platform.h>
#include <tegra_config.h>
#include <platform_def.h>
#include <psci.h>
#include <cortex_a53.h>
#include <tegra_def.h>
#include <tegra_private.h>

#define PLAT_MAX_CPUS		PLATFORM_MAX_CPUS_PER_CLUSTER

#define FLOWCTRL_HALT_CPU0_EVENTS	0x0
#define  FLOWCTRL_WAITEVENT		(2 << 29)
#define  FLOWCTRL_WAIT_FOR_INTERRUPT	(4 << 29)
#define  FLOWCTRL_JTAG_RESUME		(1 << 28)
#define  FLOWCTRL_HALT_SCLK		(1 << 27)
#define  FLOWCTRL_HALT_LIC_IRQ		(1 << 11)
#define  FLOWCTRL_HALT_LIC_FIQ		(1 << 10)
#define  FLOWCTRL_HALT_GIC_IRQ		(1 << 9)
#define  FLOWCTRL_HALT_GIC_FIQ		(1 << 8)
#define FLOWCTRL_HALT_COP_EVENTS	0x4
#define FLOWCTRL_CPU0_CSR		0x8
#define  FLOW_CTRL_CSR_PWR_OFF_STS	(1 << 16)
#define  FLOWCTRL_CSR_INTR_FLAG		(1 << 15)
#define  FLOWCTRL_CSR_EVENT_FLAG	(1 << 14)
#define  FLOWCTRL_CSR_IMMEDIATE_WAKE	(1 << 3)
#define  FLOWCTRL_CSR_ENABLE		(1 << 0)
#define FLOWCTRL_HALT_CPU1_EVENTS	0x14
#define FLOWCTRL_CPU1_CSR		0x18
#define FLOWCTRL_CC4_CORE0_CTRL		0x6c
#define FLOWCTRL_WAIT_WFI_BITMAP	0x100
#define FLOWCTRL_L2_FLUSH_CONTROL	0x94
#define FLOWCTRL_BPMP_CLUSTER_CONTROL	0x98
#define  FLOWCTRL_BPMP_CLUSTER_PWRON_LOCK	(1 << 2)

#define FLOWCTRL_ENABLE_EXT		12
#define FLOWCTRL_ENABLE_EXT_MASK	3
#define FLOWCTRL_PG_CPU_NONCPU		0x1
#define FLOWCTRL_TURNOFF_CPURAIL	0x2

#define PMC_SCRATCH39			0x138
#define CLK_RST_DEV_L_SET		0x300
#define CLK_RST_DEV_L_CLR		0x304
#define  CLK_COP_RST			(1 << 1)

#define EVP_COP_RESET_VECTOR		0x200

#define PMC_PWRGATE_STATUS		0x38
#define PMC_PWRGATE_TOGGLE		0x30
#define  PMC_TOGGLE_START		0x100

/* Power down state IDs */
#define PSTATE_ID_CORE_POWERDN		7
#define PSTATE_ID_CLUSTER_IDLE		16
#define PSTATE_ID_CLUSTER_POWERDN	17
#define PSTATE_ID_SOC_POWERDN		27

static int cpu_powergate_mask[PLAT_MAX_CPUS];
static int pmc_cpu_powergate_id[PLAT_MAX_CPUS] = {0 , 9, 10, 11};

static uint64_t flowctrl_offset_cpu_csr[PLAT_MAX_CPUS] = {
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CPU0_CSR),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CPU1_CSR),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CPU1_CSR + 8),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CPU1_CSR + 16)
};

static uint64_t flowctrl_offset_halt_cpu[PLAT_MAX_CPUS] = {
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_HALT_CPU0_EVENTS),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_HALT_CPU1_EVENTS),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_HALT_CPU1_EVENTS + 8),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_HALT_CPU1_EVENTS + 16)
};

static uint64_t flowctrl_offset_cc4_ctrl[PLAT_MAX_CPUS] = {
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CC4_CORE0_CTRL),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CC4_CORE0_CTRL + 4),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CC4_CORE0_CTRL + 8),
	(TEGRA_FLOWCTRL_BASE + FLOWCTRL_CC4_CORE0_CTRL + 12)
};

static inline void flowctrl_write_cc4_ctrl(int cpu_id, uint32_t val)
{
	mmio_write_32(flowctrl_offset_cc4_ctrl[cpu_id], val);
	val = mmio_read_32(flowctrl_offset_cc4_ctrl[cpu_id]);
}

static inline void flowctrl_write_cpu_csr(int cpu_id, uint32_t val)
{
	mmio_write_32(flowctrl_offset_cpu_csr[cpu_id], val);
	val = mmio_read_32(flowctrl_offset_cpu_csr[cpu_id]);
}

static inline void flowctrl_write_halt_cpu(int cpu_id, uint32_t val)
{
	mmio_write_32(flowctrl_offset_halt_cpu[cpu_id], val);
	val = mmio_read_32(flowctrl_offset_halt_cpu[cpu_id]);
}

static inline void flowctrl_lock_active_cluster(void)
{
	uint32_t val;

	/* inform the COP that we completed the cluster power up */
	val = mmio_read_32(TEGRA_FLOWCTRL_BASE + FLOWCTRL_BPMP_CLUSTER_CONTROL);
	val |= FLOWCTRL_BPMP_CLUSTER_PWRON_LOCK;
	mmio_write_32(TEGRA_FLOWCTRL_BASE + FLOWCTRL_BPMP_CLUSTER_CONTROL, val);
	val =mmio_read_32(TEGRA_FLOWCTRL_BASE + FLOWCTRL_BPMP_CLUSTER_CONTROL);
}

static int soc_prepare_suspend(int cpu_id, uint32_t csr)
{
	uint32_t val;

	val = FLOWCTRL_HALT_GIC_IRQ | FLOWCTRL_HALT_GIC_FIQ |
	      FLOWCTRL_HALT_LIC_IRQ | FLOWCTRL_HALT_LIC_FIQ |
	      FLOWCTRL_WAITEVENT;
	flowctrl_write_halt_cpu(cpu_id, val);

	val = FLOWCTRL_CSR_INTR_FLAG | FLOWCTRL_CSR_EVENT_FLAG |
	      FLOWCTRL_CSR_ENABLE | (FLOWCTRL_WAIT_WFI_BITMAP << cpu_id);
	flowctrl_write_cpu_csr(cpu_id, val | csr);

	return PSCI_E_SUCCESS;
}

int tegra_prepare_cpu_suspend(unsigned int id, unsigned int afflvl)
{
	int cpu = read_mpidr() & MPIDR_CPU_MASK;
	uint32_t midr = read_midr() & ~TEGRA_MIDR_MASK;
	uint32_t fc_flushctl = TEGRA_FLOWCTRL_BASE + FLOWCTRL_L2_FLUSH_CONTROL;
	uint32_t val;

	/* There's nothing to be done for affinity level 1 */
	if (afflvl == MPIDR_AFFLVL1)
		return PSCI_E_SUCCESS;

	switch (id) {
	/*
	 * Prepare for cpu idle state entry.
	 */
	case PSTATE_ID_CORE_POWERDN:
		VERBOSE("CPU%d powering down...\n", cpu);
		return soc_prepare_suspend(cpu, 0);

	/*
	 * Prepare for cluster idle state entry.
	 */
	case PSTATE_ID_CLUSTER_IDLE:
	case PSTATE_ID_CLUSTER_POWERDN:
		VERBOSE("Entering cluster %s state...\n",
			(id == PSTATE_ID_CLUSTER_IDLE) ? "idle" : "powerdn");

		flowctrl_write_cc4_ctrl(cpu, 0);

		/* hardware L2 flush is faster for A53 only */
		mmio_write_32(fc_flushctl, midr == CORTEX_A53_MIDR);

		val = ((id == PSTATE_ID_CLUSTER_IDLE) ? FLOWCTRL_PG_CPU_NONCPU :
			FLOWCTRL_TURNOFF_CPURAIL) << FLOWCTRL_ENABLE_EXT;

		return soc_prepare_suspend(cpu, val);

	/*
	 * Prepare for system idle state entry.
	 */
	case PSTATE_ID_SOC_POWERDN:
		VERBOSE("SoC powering down...\n");

		/* Enter system suspend state */
		tegra_pm_system_suspend_entry();

		flowctrl_write_cc4_ctrl(cpu, 0);

		mmio_write_32(fc_flushctl, 1);

		/* CC7 configuration */
		val = FLOWCTRL_TURNOFF_CPURAIL << FLOWCTRL_ENABLE_EXT;
		soc_prepare_suspend(cpu, val);

		/* overwrite HALT register */
		flowctrl_write_halt_cpu(cpu, FLOWCTRL_WAITEVENT);

		return PSCI_E_SUCCESS;

	default:
		ERROR("Unknown state id (%d)\n", id);
		break;
	}

	return PSCI_E_NOT_SUPPORTED;
}

int tegra_prepare_cpu_on_finish(unsigned long mpidr)
{
	uintptr_t evp_base = TEGRA_EVP_BASE;
	uintptr_t fc_base = TEGRA_FLOWCTRL_BASE;
	uintptr_t car_base = TEGRA_CAR_RESET_BASE;
	uint32_t val;

	/*
	 * Check if we are exiting from SOC_POWERDN.
	 */
	if (tegra_system_suspended()) {

		/*
		 * Restore Power Co-processor (COP) reset address and reset it.
		 */

		/*
		 * Halt COP
		 */
		mmio_write_32(fc_base + FLOWCTRL_HALT_COP_EVENTS,
			FLOWCTRL_WAITEVENT);

		/*
		 * Assert COP reset
		 */
		mmio_write_32(car_base + CLK_RST_DEV_L_SET, CLK_COP_RST);

		/*
		 * Restore reset address (stored in PMC_SCRATCH39)
		 */
		val = mmio_read_32(TEGRA_PMC_BASE + PMC_SCRATCH39);
		mmio_write_32(evp_base + EVP_COP_RESET_VECTOR, val);
		while (val != mmio_read_32(evp_base + EVP_COP_RESET_VECTOR))
			; // wait till value reaches EVP_COP_RESET_VECTOR

		/*
		 * Wait for 2us before de-asserting the reset signal.
		 */
		val = mmio_read_32(TEGRA_TMRUS_BASE);
		val += 2;
		while (val > mmio_read_32(TEGRA_TMRUS_BASE));

		/*
		 * De-assert COP reset
		 */
		mmio_write_32(car_base + CLK_RST_DEV_L_CLR, CLK_COP_RST);

		/*
		 * Un-halt COP
		 */
		mmio_write_32(fc_base + FLOWCTRL_HALT_COP_EVENTS, 0);

		/*
		 * System resume complete.
		 */
		tegra_pm_system_suspend_exit();
	}

	/*
	 * T210 has a dedicated ARMv7 boot and power mgmt processor, BPMP. It
	 * used for power management and boot purposes. Inform the BPMP that
	 * we have completed the cluster power up.
	 */
	if (psci_get_max_phys_off_afflvl() == MPIDR_AFFLVL1)
		flowctrl_lock_active_cluster();

	return PSCI_E_SUCCESS;
}

int tegra_prepare_cpu_on(unsigned long mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;
	uint32_t val;

	/* Turn on CPU using flow controller or PMC */

	if (cpu_powergate_mask[cpu] == 0) {
		/* use PMC */

		/*
		 * The PMC deasserts the START bit when it starts the power
		 * ungate process. Loop till powergate START bit is asserted.
		 */
		do {
			val = mmio_read_32(TEGRA_PMC_BASE + PMC_PWRGATE_TOGGLE);
		} while (val & (1 << 8));

		cpu_powergate_mask[cpu] = 1;

		/*
		 * Start the power ungate procedure
		 */
		val = pmc_cpu_powergate_id[cpu] | PMC_TOGGLE_START;
		mmio_write_32(TEGRA_PMC_BASE + PMC_PWRGATE_TOGGLE, val);

		/*
		 * The PMC deasserts the START bit when it starts the power
		 * ungate process. Loop till powergate START bit is asserted.
		 */
		do {
			val = mmio_read_32(TEGRA_PMC_BASE + PMC_PWRGATE_TOGGLE);
		} while (val & (1 << 8));

		/* loop till the CPU is power ungated */
		do {
			val = mmio_read_32(TEGRA_PMC_BASE + PMC_PWRGATE_STATUS);
		} while ((val & (1 << pmc_cpu_powergate_id[cpu])) == 0);

	} else {
		/* pwoer up CPU using Flow Controller */
		flowctrl_write_cpu_csr(cpu, FLOWCTRL_CSR_ENABLE);
		flowctrl_write_halt_cpu(cpu, FLOWCTRL_WAITEVENT |
			FLOWCTRL_HALT_SCLK);
	}

	return PSCI_E_SUCCESS;
}

int tegra_prepare_cpu_off(unsigned long mpidr)
{
	int cpu = mpidr & MPIDR_CPU_MASK;
	uint32_t val;

	/*
	 * Flow controller powers down the CPU. The CPU would be powered
	 * on when it receives any interrupt
	 */
	val = FLOWCTRL_CSR_INTR_FLAG | FLOWCTRL_CSR_EVENT_FLAG |
		FLOWCTRL_CSR_ENABLE | (FLOWCTRL_WAIT_WFI_BITMAP << cpu);
	flowctrl_write_cpu_csr(cpu, val);
	flowctrl_write_halt_cpu(cpu, FLOWCTRL_WAITEVENT);
	flowctrl_write_cc4_ctrl(cpu, 0);

	return PSCI_E_SUCCESS;
}
