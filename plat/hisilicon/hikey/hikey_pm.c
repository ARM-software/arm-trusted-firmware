/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/arm/gicv2.h>
#include <drivers/arm/sp804_delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <hi6220.h>
#include <hikey_def.h>
#include <hisi_ipc.h>
#include <hisi_pwrc.h>
#include <hisi_sram_map.h>

#define CORE_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) \
	((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) \
	((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

static uintptr_t hikey_sec_entrypoint;

static int hikey_pwr_domain_on(u_register_t mpidr)
{
	int cpu, cluster;
	int curr_cluster;

	cluster = MPIDR_AFFLVL1_VAL(mpidr);
	cpu = MPIDR_AFFLVL0_VAL(mpidr);
	curr_cluster = MPIDR_AFFLVL1_VAL(read_mpidr());
	if (cluster != curr_cluster)
		hisi_ipc_cluster_on(cpu, cluster);

	hisi_pwrc_set_core_bx_addr(cpu, cluster, hikey_sec_entrypoint);
	hisi_pwrc_enable_debug(cpu, cluster);
	hisi_ipc_cpu_on(cpu, cluster);

	return 0;
}

static void hikey_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	unsigned long mpidr;
	int cpu, cluster;

	mpidr = read_mpidr();
	cluster = MPIDR_AFFLVL1_VAL(mpidr);
	cpu = MPIDR_AFFLVL0_VAL(mpidr);


	/*
	 * Enable CCI coherency for this cluster.
	 * No need for locks as no other cpu is active at the moment.
	 */
	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE)
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

	/* Zero the jump address in the mailbox for this cpu */
	hisi_pwrc_set_core_bx_addr(cpu, cluster, 0);

	/* Program the GIC per-cpu distributor or re-distributor interface */
	gicv2_pcpu_distif_init();
	/* Enable the GIC cpu interface */
	gicv2_cpuif_enable();
}

void hikey_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned long mpidr;
	int cpu, cluster;

	mpidr = read_mpidr();
	cluster = MPIDR_AFFLVL1_VAL(mpidr);
	cpu = MPIDR_AFFLVL0_VAL(mpidr);

	gicv2_cpuif_disable();
	hisi_ipc_cpu_off(cpu, cluster);

	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		hisi_ipc_spin_lock(HISI_IPC_SEM_CPUIDLE);
		cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));
		hisi_ipc_spin_unlock(HISI_IPC_SEM_CPUIDLE);

		hisi_ipc_cluster_off(cpu, cluster);
	}
}

static void hikey_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();
	unsigned int cpu = mpidr & MPIDR_CPU_MASK;
	unsigned int cluster =
		(mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFFINITY_BITS;

	if (CORE_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE)
		return;

	if (CORE_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {

		/* Program the jump address for the target cpu */
		hisi_pwrc_set_core_bx_addr(cpu, cluster, hikey_sec_entrypoint);

		gicv2_cpuif_disable();

		if (SYSTEM_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE)
			hisi_ipc_cpu_suspend(cpu, cluster);
	}

	/* Perform the common cluster specific operations */
	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		hisi_ipc_spin_lock(HISI_IPC_SEM_CPUIDLE);
		cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));
		hisi_ipc_spin_unlock(HISI_IPC_SEM_CPUIDLE);

		if (SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
			hisi_pwrc_set_cluster_wfi(1);
			hisi_pwrc_set_cluster_wfi(0);
			hisi_ipc_psci_system_off();
		} else
			hisi_ipc_cluster_suspend(cpu, cluster);
	}
}

static void hikey_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	unsigned long mpidr;
	unsigned int cluster, cpu;

	/* Nothing to be done on waking up from retention from CPU level */
	if (CORE_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE)
		return;

	/* Get the mpidr for this cpu */
	mpidr = read_mpidr_el1();
	cluster = (mpidr & MPIDR_CLUSTER_MASK) >> MPIDR_AFF1_SHIFT;
	cpu = mpidr & MPIDR_CPU_MASK;

	/* Enable CCI coherency for cluster */
	if (CLUSTER_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE)
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(mpidr));

	hisi_pwrc_set_core_bx_addr(cpu, cluster, 0);

	if (SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		gicv2_distif_init();
		gicv2_pcpu_distif_init();
		gicv2_cpuif_enable();
	} else {
		gicv2_pcpu_distif_init();
		gicv2_cpuif_enable();
	}
}

static void hikey_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	int i;

	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++)
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
}

static void __dead2 hikey_system_off(void)
{
	NOTICE("%s: off system\n", __func__);

	/* Pull down GPIO_0_0 to trigger PMIC shutdown */
	mmio_write_32(0xF8001810, 0x2); /* Pinmux */
	mmio_write_8(0xF8011400, 1);	/* Pin direction */
	mmio_write_8(0xF8011004, 0);	/* Pin output value */

	/* Wait for 2s to power off system by PMIC */
	sp804_timer_init(SP804_TIMER0_BASE, 10, 192);
	mdelay(2000);

	/*
	 * PMIC shutdown depends on two conditions: GPIO_0_0 (PWR_HOLD) low,
	 * and VBUS_DET < 3.6V. For HiKey, VBUS_DET is connected to VDD_4V2
	 * through Jumper 1-2. So, to complete shutdown, user needs to manually
	 * remove Jumper 1-2.
	 */
	NOTICE("+------------------------------------------+\n");
	NOTICE("| IMPORTANT: Remove Jumper 1-2 to shutdown |\n");
	NOTICE("| DANGER:    SoC is still burning. DANGER! |\n");
	NOTICE("| Board will be reboot to avoid overheat   |\n");
	NOTICE("+------------------------------------------+\n");

	/* Send the system reset request */
	mmio_write_32(AO_SC_SYS_STAT0, 0x48698284);

	wfi();
	panic();
}

static void __dead2 hikey_system_reset(void)
{
	/* Send the system reset request */
	mmio_write_32(AO_SC_SYS_STAT0, 0x48698284);
	isb();
	dsb();

	wfi();
	panic();
}

int hikey_validate_power_state(unsigned int power_state,
			       psci_power_state_t *req_state)
{
	int pstate = psci_get_pstate_type(power_state);
	int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
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

static int hikey_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint > DDR_BASE) && (entrypoint < (DDR_BASE + DDR_SIZE)))
		return PSCI_E_SUCCESS;

	return PSCI_E_INVALID_ADDRESS;
}

static const plat_psci_ops_t hikey_psci_ops = {
	.cpu_standby			= NULL,
	.pwr_domain_on			= hikey_pwr_domain_on,
	.pwr_domain_on_finish		= hikey_pwr_domain_on_finish,
	.pwr_domain_off			= hikey_pwr_domain_off,
	.pwr_domain_suspend		= hikey_pwr_domain_suspend,
	.pwr_domain_suspend_finish	= hikey_pwr_domain_suspend_finish,
	.system_off			= hikey_system_off,
	.system_reset			= hikey_system_reset,
	.validate_power_state		= hikey_validate_power_state,
	.validate_ns_entrypoint		= hikey_validate_ns_entrypoint,
	.get_sys_suspend_power_state	= hikey_get_sys_suspend_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	hikey_sec_entrypoint = sec_entrypoint;

	/*
	 * Initialize PSCI ops struct
	 */
	*psci_ops = &hikey_psci_ops;
	return 0;
}
