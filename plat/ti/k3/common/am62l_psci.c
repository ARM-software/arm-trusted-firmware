/*
 * Copyright (c) 2025, Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <common/debug.h>
#include <device_wrapper.h>
#include <devices.h>
#include <gtc.h>
#include <k3_console.h>
#include <k3_gicv3.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lpm_stub.h>
#include <plat_scmi_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <rtc.h>
#include <stdbool.h>
#include <ti_sci.h>
#include <ti_sci_protocol.h>

/*********** PROC BOOT CODE ******************/

/* power domain indices */
#define PD_MPU_CLST     4
#define PD_MPU_CLST_CORE_0      5
#define PD_MPU_CLST_CORE_1      6

/* lpsc indices */
#define LPSC_MAIN_MPU_CLST                      38
#define LPSC_MAIN_MPU_CLST_PBIST        39
#define LPSC_MAIN_MPU_CLST_CORE_0       40
#define LPSC_MAIN_MPU_CLST_CORE_1       41

#define PSC_SYNCRESETDISABLE            		(0x0)
#define PSC_ENABLE                              (0x3)
#define PSC_PD_OFF                                      (0x0)
#define PSC_PD_ON                                       (0x1)

#define MAIN_PSC_BASE 0x00400000
#define MAIN_PSC_MDCTL_BASE 0x00400A00
#define MAIN_PSC_MDSTAT_BASE 0x00400800
#define MAIN_PSC_PDCTL_BASE 0x00400300
#define MAIN_PSC_PDSTAT_BASE 0x00400200
#define MAIN_PSC_PTSTAT (MAIN_PSC_BASE + PSC_PTSTAT)
#define MAIN_PSC_PTCMD (MAIN_PSC_BASE + PSC_PTCMD)

#define PSC_PTCMD               0x120
#define PSC_PTCMD_H             0x124
#define PSC_PTSTAT              0x128
#define PSC_PTSTAT_H            0x12C
#define PSC_PDSTAT              0x200
#define PSC_PDCTL               0x300
#define PSC_MDSTAT              0x800
#define PSC_MDCTL               0xa00

#define PDCTL_STATE_MASK                0x1
#define PDCTL_STATE_OFF                 0x0
#define PDCTL_STATE_ON                  0x1

#define MDSTAT_STATE_MASK               0x3f
#define MDSTAT_BUSY_MASK                0x30
#define MDSTAT_STATE_SWRSTDISABLE       0x0
#define MDSTAT_STATE_ENABLE             0x3

static void __unused set_main_psc_state(uint32_t pd_id, uint32_t md_id, uint32_t pd_state, uint32_t md_state)
{
	uint32_t	*mdctrl_ptr;
	volatile uint32_t	mdctrl;
	uint32_t	*mdstat_ptr;
	volatile uint32_t	mdstat;
	uint32_t	*pdctrl_ptr;
	volatile uint32_t	pdctrl;
	uint32_t	*pdstat_ptr;
	volatile uint32_t	pdstat;
	volatile uint32_t	psc_ptstat;
	volatile uint32_t	psc_ptcmd;


	mdctrl_ptr = (uint32_t*) (uint64_t) ((MAIN_PSC_MDCTL_BASE + (4*md_id)));
	mdctrl = (uint32_t) *((uint32_t*)mdctrl_ptr);
	mdstat_ptr = (uint32_t*) (uint64_t) ((MAIN_PSC_MDSTAT_BASE + (4*md_id)));
	mdstat = (uint32_t) *((uint32_t*)mdstat_ptr);
	pdctrl_ptr = (uint32_t*) (uint64_t) ((MAIN_PSC_PDCTL_BASE + (4*pd_id)));
	pdctrl = (uint32_t) *((uint32_t*)pdctrl_ptr);
	pdstat_ptr = (uint32_t*) (uint64_t) ((MAIN_PSC_PDSTAT_BASE + (4*pd_id)));
	pdstat = (uint32_t) *((uint32_t*)pdstat_ptr);

	INFO("%s: before: md_id=%d, mdstat=0x%x, pdstat=0x%x \n",__func__,md_id,mdstat,pdstat);

	if (((pdstat & 0x1) == pd_state) && ((mdstat & 0x1f) == md_state))
		return;

	// wait for GOSTAT to clear
	// may need a timeout
	psc_ptstat = *((uint32_t*) MAIN_PSC_PTSTAT);

	while ((psc_ptstat & (0x1 << pd_id)) !=0)
		psc_ptstat = *((uint32_t*) MAIN_PSC_PTSTAT);

	// Set PDCTL NEXT to new state
	*pdctrl_ptr = (pdctrl & ~(0x1)) | pd_state;

	// Set MDCTL NEXT to new state
	*mdctrl_ptr = (mdctrl & ~(0x1f)) | md_state;

	// start power transisition by setti ng PTCMD Go to 1
	psc_ptcmd = *((uint32_t*) MAIN_PSC_PTCMD);
	psc_ptcmd |= (0x1 << pd_id);

	*((uint32_t*) MAIN_PSC_PTCMD) = psc_ptcmd;

	if (!md_state)
		return;

	// wait for GOSTAT to clear
	// may need a timeout
	psc_ptstat = *((uint32_t*) MAIN_PSC_PTSTAT);
	while ((psc_ptstat & (0x1 << pd_id)) !=0)
		psc_ptstat = *((uint32_t*) MAIN_PSC_PTSTAT);

	//check states
	mdstat = (uint32_t) *((uint32_t*)mdstat_ptr);
	pdstat = (uint32_t) *((uint32_t*)pdstat_ptr);

	INFO("%s: after: md_id=%d, mdstat=0x%x, pdstat=0x%x \n",__func__,md_id,mdstat,pdstat);

}


/*********** PROC BOOT CODE ENDS******************/

#define CORE_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state) ((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state) ((state)->pwr_domain_state[PLAT_MAX_PWR_LVL])

#define WKUP_CTRL_MMR0_DEVICE_MANAGEMENT_BASE	(0x43050000UL)
#define WKUP_CTRL_MMR0_DEVICE_RESET_OFFSET	(0x4000)

uintptr_t am62l_sec_entrypoint;
uintptr_t am62l_sec_entrypoint_glob;

static int am62l_pwr_domain_on(u_register_t mpidr)
{
	int core, proc_id, ret;

	core = plat_core_pos_by_mpidr(mpidr);
	if (core < 0) {
		ERROR("Could not get target core id: %d\n", core);
		return PSCI_E_INTERN_FAIL;
	}

	proc_id = PLAT_PROC_START_ID + core;	// should be 0x21

	VERBOSE("proc_id = 0x%x\n", proc_id);

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

	set_main_psc_state(PD_MPU_CLST_CORE_0 + core, LPSC_MAIN_MPU_CLST_CORE_0 + core, PSC_PD_ON, PSC_ENABLE);

	return PSCI_E_SUCCESS;
}

static void am62l_pwr_domain_off(const psci_power_state_t *target_state)
{
	/* At very least the local core should be powering down */
	assert(CORE_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE);

	/* Prevent interrupts from spuriously waking up this cpu */
	k3_gic_cpuif_disable();
}

static void __dead2 am62l_pwr_domain_off_wfi(const psci_power_state_t *target_state)
{
	int core;

	core = plat_my_core_pos();

	/* If our cluster is not going down we stop here */
	if (CLUSTER_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE) {
		VERBOSE("%s: A53 CORE: %d OFF\n", __func__, core);
		/* Now queue up the core shutdown request */
		set_main_psc_state(PD_MPU_CLST_CORE_0 + core, LPSC_MAIN_MPU_CLST_CORE_0 + core, PSC_PD_OFF, PSC_SYNCRESETDISABLE);
	}

	while (true)
		wfi();
}

void am62l_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	k3_gic_pcpu_init();
	k3_gic_cpuif_enable();
}

static void __dead2 am62l_system_reset(void)
{
	mmio_write_32(WKUP_CTRL_MMR0_DEVICE_MANAGEMENT_BASE + WKUP_CTRL_MMR0_DEVICE_RESET_OFFSET,
		      0x6);

	ERROR("%s: Failed to reset device\n", __func__);
	while (true)
		wfi();
}

static int k3_validate_power_state(unsigned int power_state,
				   psci_power_state_t *req_state)
{
	/* TODO: perform the proper validation */

	return PSCI_E_SUCCESS;
}

#ifdef K3_AM62L_LPM
static void am62l_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	unsigned int core, proc_id;
	uint64_t  context_save_addr = 0x80A00000;
	/* TODO: Pass the mode passed from kernel using s2idle
	 * For now make mode=6 for RTC only + DDR and mdoe=0 for deepsleep
	 */
	uint32_t mode = 0;

	core = plat_my_core_pos();
	proc_id = PLAT_PROC_START_ID + core;

	/* Prevent interrupts from spuriously waking up this cpu */
	k3_gic_cpuif_disable();
	k3_gic_save_context();

	if (mode == 6) {

		k3_lpm_config_magic_words(mode);
		ti_sci_prepare_sleep(mode, context_save_addr, 0);
		INFO("sent prepare message\n");
		k3_config_wake_sources(true);
		ti_sci_enter_sleep(proc_id, mode, am62l_sec_entrypoint);
		INFO("sent enter sleep message\n");

	} else if (mode == 0) {

		k3_lpm_config_magic_words(mode);
		ti_sci_prepare_sleep(mode, context_save_addr, 0);
		INFO("sent prepare message\n");
		k3_config_wake_sources(true);
		ti_sci_enter_sleep(proc_id, mode, am62l_sec_entrypoint);
		INFO("sent enter sleep message\n");
	}

	k3_suspend_to_ram(mode);
}

static void am62l_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	/* Initialize the console to provide early debug support */
	k3_console_setup();
	k3_config_wake_sources(false);
	k3_gic_restore_context();
	k3_gic_cpuif_enable();
	ti_init_scmi_server();
	k3_lpm_stub_copy_to_sram();
	rtc_resume();
}

static void am62l_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	/* CPU & cluster off, system in retention */
	for (i = MPIDR_AFFLVL0; i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
}
#endif

static plat_psci_ops_t am62l_plat_psci_ops = {
	.pwr_domain_on = am62l_pwr_domain_on,
	.pwr_domain_off = am62l_pwr_domain_off,
	.pwr_domain_pwr_down_wfi = am62l_pwr_domain_off_wfi,
	.pwr_domain_on_finish = am62l_pwr_domain_on_finish,
#ifdef K3_AM62L_LPM
	.pwr_domain_suspend = am62l_pwr_domain_suspend,
	.pwr_domain_suspend_finish = am62l_pwr_domain_suspend_finish,
	.get_sys_suspend_power_state = am62l_get_sys_suspend_power_state,
#endif
	.system_reset = am62l_system_reset,
	.validate_power_state = k3_validate_power_state,
};

void  __aligned(16) jump_to_atf_func(void)
{
	void (*bl31_loc_warm_entry)(void) = (void *)am62l_sec_entrypoint_glob; // bl31_warm_entrypoint

	bl31_loc_warm_entry();
}

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	am62l_sec_entrypoint_glob = sec_entrypoint;
	am62l_sec_entrypoint = (unsigned long)(void *)&jump_to_atf_func;
	VERBOSE("am62l_sec_entrypoint = 0x%lx\n", am62l_sec_entrypoint);

	*psci_ops = &am62l_plat_psci_ops;

	return 0;
}
