/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <cci.h>
#include <console.h>
#include <debug.h>
#include <gicv2.h>
#include <hi3660.h>
#include <hi3660_crg.h>
#include <mmio.h>
#include <psci.h>
#include "drivers/pwrc/hisi_pwrc.h"

#include "hikey960_def.h"
#include "hikey960_private.h"

#define CORE_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) \
	((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

#define DMAC_GLB_REG_SEC	0x694
#define AXI_CONF_BASE		0x820

static uintptr_t hikey960_sec_entrypoint;

static void hikey960_pwr_domain_standby(plat_local_state_t cpu_state)
{
	unsigned long scr;

	scr = read_scr_el3();

	/* Enable Physical IRQ and FIQ to wake the CPU */
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);

	/* Add barrier before CPU enter WFI state */
	isb();
	dsb();
	wfi();

	/*
	 * Restore SCR to the original value, synchronisazion of
	 * scr_el3 is done by eret while el3_exit to save some
	 * execution cycles.
	 */
	write_scr_el3(scr);
}

static int hikey960_pwr_domain_on(u_register_t mpidr)
{
	unsigned int core = mpidr & MPIDR_CPU_MASK;
	unsigned int cluster =
		(mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;
	int cluster_stat = cluster_is_powered_on(cluster);

	hisi_set_cpu_boot_flag(cluster, core);

	mmio_write_32(CRG_REG_BASE + CRG_RVBAR(cluster, core),
		      hikey960_sec_entrypoint >> 2);

	if (cluster_stat)
		hisi_powerup_core(cluster, core);
	else
		hisi_powerup_cluster(cluster, core);

	return PSCI_E_SUCCESS;
}

static void
hikey960_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE)
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));

	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}

void hikey960_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned long mpidr = read_mpidr_el1();
	unsigned int core = mpidr & MPIDR_CPU_MASK;
	unsigned int cluster =
		(mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;

	clr_ex();
	isb();
	dsbsy();

	gicv2_cpuif_disable();

	hisi_clear_cpu_boot_flag(cluster, core);
	hisi_powerdn_core(cluster, core);

	/* check if any core is powered up */
	if (hisi_test_cpu_down(cluster, core)) {

		cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));

		isb();
		dsbsy();

		hisi_powerdn_cluster(cluster, core);
	}
}

static void __dead2 hikey960_system_reset(void)
{
	mmio_write_32(SCTRL_SCPEREN1_REG,
		      SCPEREN1_WAIT_DDR_SELFREFRESH_DONE_BYPASS);
	mmio_write_32(SCTRL_SCSYSSTAT_REG, 0xdeadbeef);
	panic();
}

int hikey960_validate_power_state(unsigned int power_state,
			       psci_power_state_t *req_state)
{
	unsigned int pstate = psci_get_pstate_type(power_state);
	unsigned int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	int i;

	assert(req_state);

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	/* Sanity check the requested state */
	if (pstate == PSTATE_TYPE_STANDBY) {
		/*
		 * It's possible to enter standby only on power level 0
		 * Ignore any other power level.
		 */
		if (pwr_lvl != MPIDR_AFFLVL0)
			return PSCI_E_INVALID_PARAMS;

		req_state->pwr_domain_state[MPIDR_AFFLVL0] =
					PLAT_MAX_RET_STATE;
	} else {
		for (i = MPIDR_AFFLVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					PLAT_MAX_OFF_STATE;
	}

	/*
	 * We expect the 'state id' to be zero.
	 */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

static int hikey960_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint > DDR_BASE) && (entrypoint < (DDR_BASE + DDR_SIZE)))
		return PSCI_E_SUCCESS;

	return PSCI_E_INVALID_ADDRESS;
}

static void hikey960_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int core = mpidr & MPIDR_CPU_MASK;
	unsigned int cluster =
		(mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;

	if (CORE_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE)
		return;

	if (CORE_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		clr_ex();
		isb();
		dsbsy();

		gicv2_cpuif_disable();

		hisi_cpuidle_lock(cluster, core);
		hisi_set_cpuidle_flag(cluster, core);
		hisi_cpuidle_unlock(cluster, core);

		mmio_write_32(CRG_REG_BASE + CRG_RVBAR(cluster, core),
		      hikey960_sec_entrypoint >> 2);

		hisi_enter_core_idle(cluster, core);
	}

	/* Perform the common cluster specific operations */
	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		hisi_cpuidle_lock(cluster, core);
		hisi_disable_pdc(cluster);

		/* check if any core is powered up */
		if (hisi_test_pwrdn_allcores(cluster, core)) {

			cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

			isb();
			dsbsy();

			/* mask the pdc wakeup irq, then
			 * enable pdc to power down the core
			 */
			hisi_pdc_mask_cluster_wakeirq(cluster);
			hisi_enable_pdc(cluster);

			hisi_cpuidle_unlock(cluster, core);

			/* check the SR flag bit to determine
			 * CLUSTER_IDLE_IPC or AP_SR_IPC to send
			 */
			if (hisi_test_ap_suspend_flag(cluster))
				hisi_enter_ap_suspend(cluster, core);
			else
				hisi_enter_cluster_idle(cluster, core);
		} else {
			/* enable pdc */
			hisi_enable_pdc(cluster);
			hisi_cpuidle_unlock(cluster, core);
		}
	}
}

static void hikey960_sr_dma_reinit(void)
{
	unsigned int ctr = 0;

	mmio_write_32(DMAC_BASE + DMAC_GLB_REG_SEC, 0x3);

	/* 1~15 channel is set non_secure */
	for (ctr = 1; ctr <= 15; ctr++)
		mmio_write_32(DMAC_BASE + AXI_CONF_BASE + ctr * (0x40),
			      (1 << 6) | (1 << 18));
}

static void
hikey960_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	unsigned long mpidr = read_mpidr_el1();
	unsigned int core = mpidr & MPIDR_CPU_MASK;
	unsigned int cluster =
		(mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;

	/* Nothing to be done on waking up from retention from CPU level */
	if (CORE_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE)
		return;

	hisi_cpuidle_lock(cluster, core);
	hisi_clear_cpuidle_flag(cluster, core);
	hisi_cpuidle_unlock(cluster, core);

	if (hisi_test_ap_suspend_flag(cluster)) {
		hikey960_sr_dma_reinit();
		gicv2_cpuif_enable();
		console_init(PL011_UART6_BASE, PL011_UART_CLK_IN_HZ,
			     PL011_BAUDRATE);
	}

	hikey960_pwr_domain_on_finish(target_state);
}

static void hikey960_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	int i;

	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

static const plat_psci_ops_t hikey960_psci_ops = {
	.cpu_standby			= hikey960_pwr_domain_standby,
	.pwr_domain_on			= hikey960_pwr_domain_on,
	.pwr_domain_on_finish		= hikey960_pwr_domain_on_finish,
	.pwr_domain_off			= hikey960_pwr_domain_off,
	.pwr_domain_suspend		= hikey960_pwr_domain_suspend,
	.pwr_domain_suspend_finish	= hikey960_pwr_domain_suspend_finish,
	.system_off			= NULL,
	.system_reset			= hikey960_system_reset,
	.validate_power_state		= hikey960_validate_power_state,
	.validate_ns_entrypoint		= hikey960_validate_ns_entrypoint,
	.get_sys_suspend_power_state	= hikey960_get_sys_suspend_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	hikey960_sec_entrypoint = sec_entrypoint;

	INFO("%s: sec_entrypoint=0x%lx\n", __func__,
	     (unsigned long)hikey960_sec_entrypoint);

	/*
	 * Initialize PSCI ops struct
	 */
	*psci_ops = &hikey960_psci_ops;
	return 0;
}
