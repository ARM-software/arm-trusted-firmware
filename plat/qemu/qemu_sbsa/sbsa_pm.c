/*
 * Copyright (c) 2020, Nuvia Inc
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <arch_helpers.h>
#include <assert.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include "sbsa_private.h"

#define ADP_STOPPED_APPLICATION_EXIT 0x20026

/*
 * Define offset and commands for the fake EC device
 */
#define SBSA_SECURE_EC_OFFSET 0x50000000

#define SBSA_SECURE_EC_CMD_SHUTDOWN 0x01
#define SBSA_SECURE_EC_CMD_REBOOT   0x02

/*
 * The secure entry point to be used on warm reset.
 */
static unsigned long secure_entrypoint;

/* Make composite power state parameter till power level 0 */
#if PSCI_EXTENDED_STATE_ID

#define qemu_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		 ((type) << PSTATE_TYPE_SHIFT))
#else
#define qemu_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		 ((pwr_lvl) << PSTATE_PWR_LVL_SHIFT) | \
		 ((type) << PSTATE_TYPE_SHIFT))
#endif /* PSCI_EXTENDED_STATE_ID */


#define qemu_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl1_state) << PLAT_LOCAL_PSTATE_WIDTH) | \
		 qemu_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type))



/*
 *  The table storing the valid idle power states. Ensure that the
 *  array entries are populated in ascending order of state-id to
 *  enable us to use binary search during power state validation.
 *  The table must be terminated by a NULL entry.
 */
static const unsigned int qemu_pm_idle_states[] = {
	/* State-id - 0x01 */
	qemu_make_pwrstate_lvl1(PLAT_LOCAL_STATE_RUN, PLAT_LOCAL_STATE_RET,
				MPIDR_AFFLVL0, PSTATE_TYPE_STANDBY),
	/* State-id - 0x02 */
	qemu_make_pwrstate_lvl1(PLAT_LOCAL_STATE_RUN, PLAT_LOCAL_STATE_OFF,
				MPIDR_AFFLVL0, PSTATE_TYPE_POWERDOWN),
	/* State-id - 0x22 */
	qemu_make_pwrstate_lvl1(PLAT_LOCAL_STATE_OFF, PLAT_LOCAL_STATE_OFF,
				MPIDR_AFFLVL1, PSTATE_TYPE_POWERDOWN),
	0
};

/*******************************************************************************
 * Platform handler called to check the validity of the power state
 * parameter. The power state parameter has to be a composite power state.
 ******************************************************************************/
static int qemu_validate_power_state(unsigned int power_state,
				psci_power_state_t *req_state)
{
	unsigned int state_id;
	unsigned int i;

	assert(req_state != NULL);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justifies the additional complexity.
	 */
	for (i = 0U; qemu_pm_idle_states[i] != 0U; i++) {
		if (power_state == qemu_pm_idle_states[i]) {
			break;
		}
	}

	/* Return error if entry not found in the idle state array */
	if (qemu_pm_idle_states[i] == 0U) {
		return PSCI_E_INVALID_PARAMS;
	}

	i = 0U;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id != 0U) {
		req_state->pwr_domain_state[i++] = state_id &
						PLAT_LOCAL_PSTATE_MASK;
		state_id >>= PLAT_LOCAL_PSTATE_WIDTH;
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler called when a CPU is about to enter standby.
 ******************************************************************************/
static void qemu_cpu_standby(plat_local_state_t cpu_state)
{

	assert(cpu_state == PLAT_LOCAL_STATE_RET);

	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
static int qemu_pwr_domain_on(u_register_t mpidr)
{
	int pos = plat_core_pos_by_mpidr(mpidr);
	uint64_t *hold_base = (uint64_t *)PLAT_QEMU_HOLD_BASE;

	if (pos < 0) {
		return PSCI_E_INVALID_PARAMS;
	}

	hold_base[pos] = PLAT_QEMU_HOLD_STATE_GO;
	dsb();
	sev();

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
static void qemu_pwr_domain_off(const psci_power_state_t *target_state)
{
	qemu_pwr_gic_off();
}

void __dead2 plat_secondary_cold_boot_setup(void);

static void __dead2
qemu_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{
	disable_mmu_el3();
	plat_secondary_cold_boot_setup();
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void qemu_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	assert(false);
}

/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void qemu_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					PLAT_LOCAL_STATE_OFF);

	qemu_pwr_gic_on_finish();
}

/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 ******************************************************************************/
void qemu_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	assert(false);
}

/*******************************************************************************
 * Platform handlers to shutdown/reboot the system
 ******************************************************************************/
static void __dead2 qemu_system_off(void)
{
	mmio_write_32(SBSA_SECURE_EC_OFFSET, SBSA_SECURE_EC_CMD_SHUTDOWN);
	panic();
}

static void __dead2 qemu_system_reset(void)
{
	mmio_write_32(SBSA_SECURE_EC_OFFSET, SBSA_SECURE_EC_CMD_REBOOT);
	panic();
}

static const plat_psci_ops_t plat_qemu_psci_pm_ops = {
	.cpu_standby = qemu_cpu_standby,
	.pwr_domain_on = qemu_pwr_domain_on,
	.pwr_domain_off = qemu_pwr_domain_off,
	.pwr_domain_pwr_down_wfi = qemu_pwr_domain_pwr_down_wfi,
	.pwr_domain_suspend = qemu_pwr_domain_suspend,
	.pwr_domain_on_finish = qemu_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = qemu_pwr_domain_suspend_finish,
	.system_off = qemu_system_off,
	.system_reset = qemu_system_reset,
	.validate_power_state = qemu_validate_power_state
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	uintptr_t *mailbox = (uintptr_t *)PLAT_QEMU_TRUSTED_MAILBOX_BASE;

	*mailbox = sec_entrypoint;
	secure_entrypoint = (unsigned long)sec_entrypoint;
	*psci_ops = &plat_qemu_psci_pm_ops;

	return 0;
}
