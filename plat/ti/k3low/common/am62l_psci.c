/*
 * Copyright (c) 2025, Texas Instruments Incorporated - https://www.ti.com/
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>
#include <ti_sci.h>
#include <ti_sci_protocol.h>

#include <k3_gicv3.h>
#include <platform_def.h>

uintptr_t am62l_sec_entrypoint;
uintptr_t am62l_sec_entrypoint_glob;
void  __aligned(16) jump_to_atf_func(void *unused);

static int am62l_pwr_domain_on(u_register_t mpidr)
{
	int32_t core, ret;
	uint8_t proc_id;

	core = plat_core_pos_by_mpidr(mpidr);
	if (core < 0) {
		ERROR("Could not get target core id: %d\n", core);
		return PSCI_E_INTERN_FAIL;
	}

	proc_id = (uint8_t)(PLAT_PROC_START_ID + (uint32_t)core);

	ret = ti_sci_proc_request(proc_id);
	if (ret != 0) {
		ERROR("Request for processor ID 0x%x failed: %d\n",
				proc_id, ret);
		return PSCI_E_INTERN_FAIL;
	}

	ret = ti_sci_proc_set_boot_cfg(proc_id, am62l_sec_entrypoint, 0, 0);
	if (ret != 0) {
		ERROR("Request to set core boot address failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	/* sanity check these are off before starting a core */
	ret = ti_sci_proc_set_boot_ctrl(proc_id,
			0, PROC_BOOT_CTRL_FLAG_ARMV8_L2FLUSHREQ |
			PROC_BOOT_CTRL_FLAG_ARMV8_AINACTS |
			PROC_BOOT_CTRL_FLAG_ARMV8_ACINACTM);
	if (ret != 0) {
		ERROR("Request to clear boot config failed: %d\n", ret);
		return PSCI_E_INTERN_FAIL;
	}

	/*
	 * TODO: Add the actual PM operation call
	 * to turn on the core here
	 */
	return PSCI_E_SUCCESS;
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
	/* TODO: Add the actual pm operation call to turn off the core */
}

void am62l_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	k3_gic_pcpu_init();
	k3_gic_cpuif_enable();
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

static plat_psci_ops_t am62l_plat_psci_ops = {
	.pwr_domain_on = am62l_pwr_domain_on,
	.pwr_domain_off = am62l_pwr_domain_off,
	.pwr_domain_pwr_down = am62l_pwr_down_domain,
	.pwr_domain_on_finish = am62l_pwr_domain_on_finish,
	.system_reset = am62l_system_reset,
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
