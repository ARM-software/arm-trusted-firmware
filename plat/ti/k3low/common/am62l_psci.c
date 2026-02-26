/*
 * Copyright (c) 2025-2026, Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <ti_clk.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <firewall.h>
#include <gtc.h>
#include <k3_console.h>
#include <ti_devices.h>
#include <ti_device_handler.h>
#include <k3_gicv3.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lpm_stub.h>
#include <plat_scmi_def.h>
#include <plat/common/platform.h>
#include <ti_sci.h>
#include <ti_sci_protocol.h>

#include <k3_gicv3.h>
#include <platform_def.h>
#include <standby.h>

volatile unsigned int val_mdctl;
volatile unsigned int val_mdstat;

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

#define PWR_LVL_STATE(state, lvl) ((state) >> (4 * (lvl)) & 0xfU)

#define PMCTRL_SYS					(0x80)

uintptr_t am62l_sec_entrypoint;
uintptr_t am62l_sec_entrypoint_glob;
void  __aligned(16) jump_to_atf_func(void *unused);

static void am62l_cpu_standby(plat_local_state_t cpu_state)
{
	u_register_t scr;
	scr = read_scr_el3();
	/* Enable the Non secure interrupt to wake the CPU */
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);
	isb();
	/* dsb is good practice before using wfi to enter low power states */
	dsb();
	/* Enter standby state */
	wfi();
	/* Restore SCR */
	write_scr_el3(scr);
}

static int __maybe_unused am62l_core_pwr_domain_on(int core) {
	int proc_id = PLAT_PROC_START_ID + core;	// should be 0x21
	int ret;

	INFO("loc_pwr proc_id = 0x%x\n", proc_id);

	ret = ti_sci_proc_request(proc_id);
	if (ret) {
		ERROR("Request for processor failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	ret = ti_sci_proc_set_boot_cfg(proc_id, am62l_sec_entrypoint, 0, 0);
	if (ret) {
		ERROR("Request to set core boot address failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	/* sanity check these are off before starting a core */
	ret = ti_sci_proc_set_boot_ctrl(proc_id,
					0, PROC_BOOT_CTRL_FLAG_ARMV8_L2FLUSHREQ |
					PROC_BOOT_CTRL_FLAG_ARMV8_AINACTS |
					PROC_BOOT_CTRL_FLAG_ARMV8_ACINACTM);
	if (ret) {
		ERROR("Request to clear boot configuration failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	set_main_psc_state(PD_MPU_CLST_CORE_0 + core, LPSC_MAIN_MPU_CLST_CORE_0 + core,
			   PSC_PD_ON, PSC_ENABLE);

	return PSCI_E_SUCCESS;

}

static void am62l_core_pwr_domain_off(int core) {
	set_main_psc_state(PD_MPU_CLST_CORE_0 + core, LPSC_MAIN_MPU_CLST_CORE_0 + core,
			PSC_PD_OFF, PSC_SYNCRESETDISABLE);
}

static int am62l_pwr_domain_on(u_register_t mpidr)
{
	int32_t core;

	core = plat_core_pos_by_mpidr(mpidr);
	if (core < 0) {
		ERROR("Could not get target core id: %d\n", core);
		return PSCI_E_INTERN_FAIL;
	}

	return am62l_core_pwr_domain_on(core);
}

static void am62l_pwr_domain_off(const psci_power_state_t *target_state)
{
	/* At very least the local core should be powering down */
	assert(((target_state)->pwr_domain_state[MPIDR_AFFLVL0]) == PLAT_MAX_OFF_STATE);

	/* Prevent interrupts from spuriously waking up this cpu */
	k3_gic_cpuif_disable();
}

static void am62l_pwr_down_domain(const psci_power_state_t *target_state)
{
	int core;

	core = plat_my_core_pos();

	/* If our cluster is not going down we stop here */
	if (SYSTEM_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE) {
		VERBOSE("%s: A53 CORE: %d OFF\n", __func__, core);
		am62l_core_pwr_domain_off(core);
	}
}

void am62l_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	k3_gic_pcpu_init();
	k3_gic_cpuif_enable();
}

static void __dead2 am62l_system_off(void)
{
	INFO("%s: Initiating system poweroff sequence\n", __func__);

	/* Notify TIFS to prepare for poweroff */
	ti_sci_prepare_sleep(TI_K3_SLEEP_MODE_RTC_ONLY, 0, 0);

	/* Enter poweroff by configuring PMIC control register */
	mmio_write_32(WKUP_CTRL_MMR_SEC_5_BASE + PMCTRL_SYS, 0x0U);
	dsb();
	isb();

	INFO("%s: PMIC control configured, waiting for poweroff\n", __func__);

	/* Cannot safely recover - enter infinite WFI loop */
	while (true)
		wfi();
}

static void am62l_system_reset(void)
{
	mmio_write_32(WKUP_CTRL_MMR0_BASE + WKUP_CTRL_MMR0_DEVICE_RESET_OFFSET,
		      0x6);

	/* Wait for reset to complete for 500ms before printing error */
	mdelay(500);

	/* Ideally we should not reach here */
	ERROR("%s: Failed to reset device\n", __func__);
}

static int am62l_validate_power_state(unsigned int power_state,
				   psci_power_state_t *req_state)
{
	unsigned int pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	unsigned int pstate = psci_get_pstate_type(power_state);
	int i;

	if (pwr_lvl > PLAT_MAX_PWR_LVL)
		return PSCI_E_INVALID_PARAMS;

	if (pstate == PSTATE_TYPE_STANDBY) {
		CORE_PWR_STATE(req_state) = PWR_LVL_STATE(power_state, MPIDR_AFFLVL0);
		CLUSTER_PWR_STATE(req_state) = PWR_LVL_STATE(power_state, MPIDR_AFFLVL1);
		SYSTEM_PWR_STATE(req_state) = PWR_LVL_STATE(power_state, PLAT_MAX_PWR_LVL);
	} else if (pstate == PSTATE_TYPE_POWERDOWN) {
		for (i = MPIDR_AFFLVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
#if PSCI_OS_INIT_MODE
	req_state->last_at_pwrlvl = pwr_lvl;
#endif
	return PSCI_E_SUCCESS;
}

static void am62l_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	uint32_t core, proc_id;
	uint32_t mode = 0;
	core = plat_my_core_pos();
	uint64_t context_save_addr = 0x80A00000;

	assert(core < 2U);

	/* Entering cluster standby sequence */
	if (CORE_PWR_STATE(target_state) == CORE_IDLE_STATE) {
		uint32_t cluster_pwr_state = CLUSTER_PWR_STATE(target_state);
		am62l_enter_standby(core, cluster_pwr_state);
		return;
	}

	proc_id = PLAT_PROC_START_ID + core;

	/* Prevent interrupts from spuriously waking up this cpu */
	k3_gic_cpuif_disable();
	k3_gic_save_context();
	ti_clks_suspend();
	INFO("Started Suspend Sequence in ATF\n");
	/* Isolate the I/Os to allow I/O Daisy chain wakeup */
	k3low_lpm_set_io_isolation(true);
	k3low_lpm_config_magic_words(mode);
	ti_sci_prepare_sleep(mode, context_save_addr, 0);
	INFO("sent prepare message\n");
	k3low_config_wake_sources(true);
	ti_sci_enter_sleep(proc_id, mode, am62l_sec_entrypoint);
	INFO("sent enter sleep message\n");
	k3low_suspend_to_ram(mode);
}

static void am62l_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	uint32_t core;
	core = plat_my_core_pos();

	assert(core < 2U);

	/* Exiting cluster standby sequence */
	if (CORE_PWR_STATE(target_state) == CORE_IDLE_STATE) {
		uint32_t cluster_pwr_state = CLUSTER_PWR_STATE(target_state);
		am62l_exit_standby(core, cluster_pwr_state);
		return;
	}

	/* Update firewall configurations */
	update_fwl_configs();
	/* Remove the I/O isolation */
	k3low_lpm_set_io_isolation(false);
	/* Initialize the console to provide early debug support */
	k3_console_setup();
	k3low_config_wake_sources(false);
	k3_gic_restore_context();
	k3_gic_cpuif_enable();
	ti_init_scmi_server();
	k3low_lpm_stub_copy_to_sram();
	ti_clks_resume();
}

static void am62l_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	/* CPU & cluster off, system in retention */
	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
}

static plat_psci_ops_t am62l_plat_psci_ops = {
	.cpu_standby = am62l_cpu_standby,
	.pwr_domain_on = am62l_pwr_domain_on,
	.pwr_domain_off = am62l_pwr_domain_off,
	.pwr_domain_pwr_down = am62l_pwr_down_domain,
	.pwr_domain_on_finish = am62l_pwr_domain_on_finish,
	.system_reset = am62l_system_reset,
	.pwr_domain_suspend = am62l_pwr_domain_suspend,
	.pwr_domain_suspend_finish = am62l_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = am62l_get_sys_suspend_power_state,
	.validate_power_state = am62l_validate_power_state,
	.system_off = am62l_system_off,
};

void  __aligned(16) jump_to_atf_func(void *unused)
{
	/*
	 * MISRA Deviation observed:
	 * Rule 11.1 (MISRA C:2012) Prohibits conversion performed between a
	 * pointer to a function and another incompatible type.
	 * This conversion is required for handling secure boot entry points.
	 * The conversion is safe as the address is verified before execution.
	 */
	void (*bl31_loc_warm_entry)(void) = (void *)am62l_sec_entrypoint_glob;

	bl31_loc_warm_entry();
}

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	am62l_sec_entrypoint_glob = sec_entrypoint;
	/* Note that boot vector reg in sec mmr requires 16B aligned start address */
	am62l_sec_entrypoint = (uint64_t)(void *)&jump_to_atf_func;
	VERBOSE("am62l_sec_entrypoint = 0x%lx\n", am62l_sec_entrypoint);

	*psci_ops = &am62l_plat_psci_ops;

	return 0;
}

plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
					     const plat_local_state_t *states,
					     unsigned int ncpu)
{
	plat_local_state_t target = PLAT_MAX_OFF_STATE, temp;
	const plat_local_state_t *st = states;
	unsigned int n = ncpu;

	assert(ncpu > 0U);

	do {
		temp = *st;
		st++;
		/*  The power state of the CPU STANDBY called by fast path in psci_cpu_suspend()
		 *	is RUN and the power states are in an increasing order of power saved.
		 *	Thus the target power state for the cluster is the minimum of the power states
		 *	requested by all the cores that is not RUN.
		 */
		if ((temp < target) && ((temp != PSCI_LOCAL_STATE_RUN) || (lvl != MPIDR_AFFLVL1)))
			target = temp;
		n--;
	} while (n > 0U);

	return target;
}