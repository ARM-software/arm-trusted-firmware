/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
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
#include <errno.h>
#include <assert.h>
#include <debug.h>
#include <gicv2.h>
#include <mmio.h>
#include <plat_arm.h>
#include <platform.h>
#include <psci.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "zynqmp_private.h"

uintptr_t zynqmp_sec_entry;

void zynqmp_cpu_standby(plat_local_state_t cpu_state)
{
	VERBOSE("%s: cpu_state: 0x%x\n", __func__, cpu_state);

	dsb();
	wfi();
}

static int zynqmp_nopmu_pwr_domain_on(u_register_t mpidr)
{
	uint32_t r;
	unsigned int cpu_id = plat_core_pos_by_mpidr(mpidr);

	VERBOSE("%s: mpidr: 0x%lx\n", __func__, mpidr);

	if (cpu_id == -1)
		return PSCI_E_INTERN_FAIL;

	/* program RVBAR */
	mmio_write_32(APU_RVBAR_L_0 + (cpu_id << 3), zynqmp_sec_entry);
	mmio_write_32(APU_RVBAR_H_0 + (cpu_id << 3), zynqmp_sec_entry >> 32);

	/* clear VINITHI */
	r = mmio_read_32(APU_CONFIG_0);
	r &= ~(1 << APU_CONFIG_0_VINITHI_SHIFT << cpu_id);
	mmio_write_32(APU_CONFIG_0, r);

	/* clear power down request */
	r = mmio_read_32(APU_PWRCTL);
	r &= ~(1 << cpu_id);
	mmio_write_32(APU_PWRCTL, r);

	/* power up island */
	mmio_write_32(PMU_GLOBAL_REQ_PWRUP_EN, 1 << cpu_id);
	mmio_write_32(PMU_GLOBAL_REQ_PWRUP_TRIG, 1 << cpu_id);
	/* FIXME: we should have a way to break out */
	while (mmio_read_32(PMU_GLOBAL_REQ_PWRUP_STATUS) & (1 << cpu_id))
		;

	/* release core reset */
	r = mmio_read_32(CRF_APB_RST_FPD_APU);
	r &= ~((CRF_APB_RST_FPD_APU_ACPU_PWRON_RESET |
			CRF_APB_RST_FPD_APU_ACPU_RESET) << cpu_id);
	mmio_write_32(CRF_APB_RST_FPD_APU, r);

	return PSCI_E_SUCCESS;
}

static int zynqmp_pwr_domain_on(u_register_t mpidr)
{
	unsigned int cpu_id = plat_core_pos_by_mpidr(mpidr);
	const struct pm_proc *proc;

	VERBOSE("%s: mpidr: 0x%lx\n", __func__, mpidr);

	if (cpu_id == -1)
		return PSCI_E_INTERN_FAIL;

	proc = pm_get_proc(cpu_id);

	/* Send request to PMU to wake up selected APU CPU core */
	pm_req_wakeup(proc->node_id, 1, zynqmp_sec_entry, REQ_ACK_NO);

	return PSCI_E_SUCCESS;
}

static void zynqmp_nopmu_pwr_domain_off(const psci_power_state_t *target_state)
{
	uint32_t r;
	unsigned int cpu_id = plat_my_core_pos();

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	/* Prevent interrupts from spuriously waking up this cpu */
	gicv2_cpuif_disable();

	/* set power down request */
	r = mmio_read_32(APU_PWRCTL);
	r |= (1 << cpu_id);
	mmio_write_32(APU_PWRCTL, r);
}

static void zynqmp_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	/* Prevent interrupts from spuriously waking up this cpu */
	gicv2_cpuif_disable();

	/*
	 * Send request to PMU to power down the appropriate APU CPU
	 * core.
	 * According to PSCI specification, CPU_off function does not
	 * have resume address and CPU core can only be woken up
	 * invoking CPU_on function, during which resume address will
	 * be set.
	 */
	pm_self_suspend(proc->node_id, MAX_LATENCY, 0, 0);
}

static void zynqmp_nopmu_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint32_t r;
	unsigned int cpu_id = plat_my_core_pos();

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	/* set power down request */
	r = mmio_read_32(APU_PWRCTL);
	r |= (1 << cpu_id);
	mmio_write_32(APU_PWRCTL, r);

	/* program RVBAR */
	mmio_write_32(APU_RVBAR_L_0 + (cpu_id << 3), zynqmp_sec_entry);
	mmio_write_32(APU_RVBAR_H_0 + (cpu_id << 3), zynqmp_sec_entry >> 32);

	/* clear VINITHI */
	r = mmio_read_32(APU_CONFIG_0);
	r &= ~(1 << APU_CONFIG_0_VINITHI_SHIFT << cpu_id);
	mmio_write_32(APU_CONFIG_0, r);

	/* enable power up on IRQ */
	mmio_write_32(PMU_GLOBAL_REQ_PWRUP_EN, 1 << cpu_id);
}

static void zynqmp_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	/* Send request to PMU to suspend this core */
	pm_self_suspend(proc->node_id, MAX_LATENCY, 0, zynqmp_sec_entry);

	/* APU is to be turned off */
	if (target_state->pwr_domain_state[1] > PLAT_MAX_RET_STATE) {
		/* Power down L2 cache */
		pm_set_requirement(NODE_L2, 0, 0, REQ_ACK_NO);
		/* Send request for OCM retention state */
		set_ocm_retention();
		/* disable coherency */
		plat_arm_interconnect_exit_coherency();
	}
}

static void zynqmp_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	gicv2_cpuif_enable();
	gicv2_pcpu_distif_init();
}

static void zynqmp_nopmu_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint32_t r;
	unsigned int cpu_id = plat_my_core_pos();

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	/* disable power up on IRQ */
	mmio_write_32(PMU_GLOBAL_REQ_PWRUP_DIS, 1 << cpu_id);

	/* clear powerdown bit */
	r = mmio_read_32(APU_PWRCTL);
	r &= ~(1 << cpu_id);
	mmio_write_32(APU_PWRCTL, r);
}

static void zynqmp_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	unsigned int cpu_id = plat_my_core_pos();
	const struct pm_proc *proc = pm_get_proc(cpu_id);

	for (size_t i = 0; i <= PLAT_MAX_PWR_LVL; i++)
		VERBOSE("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);

	/* Clear the APU power control register for this cpu */
	pm_client_wakeup(proc);

	/* enable coherency */
	plat_arm_interconnect_enter_coherency();
}

/*******************************************************************************
 * ZynqMP handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 zynqmp_nopmu_system_off(void)
{
	ERROR("ZynqMP System Off: operation not handled.\n");

	/* disable coherency */
	plat_arm_interconnect_exit_coherency();

	panic();
}

static void __dead2 zynqmp_system_off(void)
{
	/* disable coherency */
	plat_arm_interconnect_exit_coherency();

	/* Send the power down request to the PMU */
	pm_system_shutdown(0);

	while (1)
		wfi();
}

static void __dead2 zynqmp_nopmu_system_reset(void)
{
	/*
	 * This currently triggers a system reset. I.e. the whole
	 * system will be reset! Including RPUs, PMU, PL, etc.
	 */

	/* disable coherency */
	plat_arm_interconnect_exit_coherency();

	/* bypass RPLL (needed on 1.0 silicon) */
	uint32_t reg = mmio_read_32(CRL_APB_RPLL_CTRL);
	reg |= CRL_APB_RPLL_CTRL_BYPASS;
	mmio_write_32(CRL_APB_RPLL_CTRL, reg);

	/* trigger system reset */
	mmio_write_32(CRL_APB_RESET_CTRL, CRL_APB_RESET_CTRL_SOFT_RESET);

	while (1)
		wfi();
}

static void __dead2 zynqmp_system_reset(void)
{
	/* disable coherency */
	plat_arm_interconnect_exit_coherency();

	/* Send the system reset request to the PMU */
	pm_system_shutdown(1);

	while (1)
		wfi();
}

int zynqmp_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	VERBOSE("%s: power_state: 0x%x\n", __func__, power_state);

	/* FIXME: populate req_state */
	return PSCI_E_SUCCESS;
}

int zynqmp_validate_ns_entrypoint(unsigned long ns_entrypoint)
{
	VERBOSE("%s: ns_entrypoint: 0x%lx\n", __func__, ns_entrypoint);

	/* FIXME: Actually validate */
	return PSCI_E_SUCCESS;
}

void zynqmp_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	req_state->pwr_domain_state[PSCI_CPU_PWR_LVL] = PLAT_MAX_OFF_STATE;
	req_state->pwr_domain_state[1] = PLAT_MAX_OFF_STATE;
}

/*******************************************************************************
 * Export the platform handlers to enable psci to invoke them
 ******************************************************************************/
static const struct plat_psci_ops zynqmp_psci_ops = {
	.cpu_standby			= zynqmp_cpu_standby,
	.pwr_domain_on			= zynqmp_pwr_domain_on,
	.pwr_domain_off			= zynqmp_pwr_domain_off,
	.pwr_domain_suspend		= zynqmp_pwr_domain_suspend,
	.pwr_domain_on_finish		= zynqmp_pwr_domain_on_finish,
	.pwr_domain_suspend_finish	= zynqmp_pwr_domain_suspend_finish,
	.system_off			= zynqmp_system_off,
	.system_reset			= zynqmp_system_reset,
	.validate_power_state		= zynqmp_validate_power_state,
	.validate_ns_entrypoint		= zynqmp_validate_ns_entrypoint,
	.get_sys_suspend_power_state	= zynqmp_get_sys_suspend_power_state,
};

static const struct plat_psci_ops zynqmp_nopmu_psci_ops = {
	.cpu_standby			= zynqmp_cpu_standby,
	.pwr_domain_on			= zynqmp_nopmu_pwr_domain_on,
	.pwr_domain_off			= zynqmp_nopmu_pwr_domain_off,
	.pwr_domain_suspend		= zynqmp_nopmu_pwr_domain_suspend,
	.pwr_domain_on_finish		= zynqmp_pwr_domain_on_finish,
	.pwr_domain_suspend_finish	= zynqmp_nopmu_pwr_domain_suspend_finish,
	.system_off			= zynqmp_nopmu_system_off,
	.system_reset			= zynqmp_nopmu_system_reset,
	.validate_power_state		= zynqmp_validate_power_state,
	.validate_ns_entrypoint		= zynqmp_validate_ns_entrypoint,
	.get_sys_suspend_power_state	= zynqmp_get_sys_suspend_power_state,
};

/*******************************************************************************
 * Export the platform specific power ops.
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	zynqmp_sec_entry = sec_entrypoint;

	if (zynqmp_is_pmu_up())
		*psci_ops = &zynqmp_psci_ops;
	else
		*psci_ops = &zynqmp_nopmu_psci_ops;

	return 0;
}
