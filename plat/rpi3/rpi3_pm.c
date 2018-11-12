/*
 * Copyright (c) 2015-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <platform_def.h>
#include <platform.h>
#include <psci.h>

#include "rpi3_hw.h"

/* Make composite power state parameter till power level 0 */
#if PSCI_EXTENDED_STATE_ID

#define rpi3_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		 ((type) << PSTATE_TYPE_SHIFT))

#else

#define rpi3_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		 ((pwr_lvl) << PSTATE_PWR_LVL_SHIFT) | \
		 ((type) << PSTATE_TYPE_SHIFT))

#endif /* PSCI_EXTENDED_STATE_ID */

#define rpi3_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl1_state) << PLAT_LOCAL_PSTATE_WIDTH) | \
		 rpi3_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type))

/*
 *  The table storing the valid idle power states. Ensure that the
 *  array entries are populated in ascending order of state-id to
 *  enable us to use binary search during power state validation.
 *  The table must be terminated by a NULL entry.
 */
static const unsigned int rpi3_pm_idle_states[] = {
	/* State-id - 0x01 */
	rpi3_make_pwrstate_lvl1(PLAT_LOCAL_STATE_RUN, PLAT_LOCAL_STATE_RET,
				MPIDR_AFFLVL0, PSTATE_TYPE_STANDBY),
	/* State-id - 0x02 */
	rpi3_make_pwrstate_lvl1(PLAT_LOCAL_STATE_RUN, PLAT_LOCAL_STATE_OFF,
				MPIDR_AFFLVL0, PSTATE_TYPE_POWERDOWN),
	/* State-id - 0x22 */
	rpi3_make_pwrstate_lvl1(PLAT_LOCAL_STATE_OFF, PLAT_LOCAL_STATE_OFF,
				MPIDR_AFFLVL1, PSTATE_TYPE_POWERDOWN),
	0,
};

/*******************************************************************************
 * Platform handler called to check the validity of the power state
 * parameter. The power state parameter has to be a composite power state.
 ******************************************************************************/
static int rpi3_validate_power_state(unsigned int power_state,
				     psci_power_state_t *req_state)
{
	unsigned int state_id;
	int i;

	assert(req_state != 0);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; rpi3_pm_idle_states[i] != 0; i++) {
		if (power_state == rpi3_pm_idle_states[i]) {
			break;
		}
	}

	/* Return error if entry not found in the idle state array */
	if (!rpi3_pm_idle_states[i]) {
		return PSCI_E_INVALID_PARAMS;
	}

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] = state_id &
						PLAT_LOCAL_PSTATE_MASK;
		state_id >>= PLAT_LOCAL_PSTATE_WIDTH;
	}

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler called when a CPU is about to enter standby.
 ******************************************************************************/
static void rpi3_cpu_standby(plat_local_state_t cpu_state)
{
	assert(cpu_state == PLAT_LOCAL_STATE_RET);

	/*
	 * Enter standby state.
	 * dsb is good practice before using wfi to enter low power states
	 */
	dsb();
	wfi();
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
static int rpi3_pwr_domain_on(u_register_t mpidr)
{
	int rc = PSCI_E_SUCCESS;
	unsigned int pos = plat_core_pos_by_mpidr(mpidr);
	uint64_t *hold_base = (uint64_t *)PLAT_RPI3_TM_HOLD_BASE;

	assert(pos < PLATFORM_CORE_COUNT);

	hold_base[pos] = PLAT_RPI3_TM_HOLD_STATE_GO;

	/* Make sure that the write has completed */
	dsb();
	isb();

	sev();

	return rc;
}

/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void rpi3_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
					PLAT_LOCAL_STATE_OFF);
}

/*******************************************************************************
 * Platform handlers for system reset and system off.
 ******************************************************************************/

/* 10 ticks (Watchdog timer = Timer clock / 16) */
#define RESET_TIMEOUT	U(10)

static void __dead2 rpi3_watchdog_reset(void)
{
	uint32_t rstc;

#if CONSOLE_FLUSH_ON_POWEROFF
	console_flush();
#endif /* CONSOLE_FLUSH_ON_POWEROFF */

	dsbsy();
	isb();

	mmio_write_32(RPI3_PM_BASE + RPI3_PM_WDOG_OFFSET,
		      RPI3_PM_PASSWORD | RESET_TIMEOUT);

	rstc = mmio_read_32(RPI3_PM_BASE + RPI3_PM_RSTC_OFFSET);
	rstc &= ~RPI3_PM_RSTC_WRCFG_MASK;
	rstc |= RPI3_PM_PASSWORD | RPI3_PM_RSTC_WRCFG_FULL_RESET;
	mmio_write_32(RPI3_PM_BASE + RPI3_PM_RSTC_OFFSET, rstc);

	for (;;) {
		wfi();
	}
}

static void __dead2 rpi3_system_reset(void)
{
	INFO("rpi3: PSCI_SYSTEM_RESET: Invoking watchdog reset\n");

	rpi3_watchdog_reset();
}

static void __dead2 rpi3_system_off(void)
{
	uint32_t rsts;

	INFO("rpi3: PSCI_SYSTEM_OFF: Invoking watchdog reset\n");

	/*
	 * This function doesn't actually make the Raspberry Pi turn itself off,
	 * the hardware doesn't allow it. It simply reboots it and the RSTS
	 * value tells the bootcode.bin firmware not to continue the regular
	 * bootflow and to stay in a low power mode.
	 */

	rsts = mmio_read_32(RPI3_PM_BASE + RPI3_PM_RSTS_OFFSET);
	rsts |= RPI3_PM_PASSWORD | RPI3_PM_RSTS_WRCFG_HALT;
	mmio_write_32(RPI3_PM_BASE + RPI3_PM_RSTS_OFFSET, rsts);

	rpi3_watchdog_reset();
}

/*******************************************************************************
 * Platform handlers and setup function.
 ******************************************************************************/
static const plat_psci_ops_t plat_rpi3_psci_pm_ops = {
	.cpu_standby = rpi3_cpu_standby,
	.pwr_domain_on = rpi3_pwr_domain_on,
	.pwr_domain_on_finish = rpi3_pwr_domain_on_finish,
	.system_off = rpi3_system_off,
	.system_reset = rpi3_system_reset,
	.validate_power_state = rpi3_validate_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	uintptr_t *entrypoint = (void *) PLAT_RPI3_TM_ENTRYPOINT;

	*entrypoint = sec_entrypoint;
	*psci_ops = &plat_rpi3_psci_pm_ops;

	return 0;
}
