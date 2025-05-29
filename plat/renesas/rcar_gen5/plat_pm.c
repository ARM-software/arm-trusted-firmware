/*
 * Copyright (c) 2025, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <drivers/arm/cci.h>
#include <drivers/arm/gicv3.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <plat/common/platform.h>
#include "pwrc.h"
#include "timer.h"

#include "platform_def.h"
#include "rcar_def.h"
#include "rcar_private.h"

#define SYSTEM_PWR_STATE(s)	((s)->pwr_domain_state[PLAT_MAX_PWR_LVL])
#define CORE_PWR_STATE(s)	((s)->pwr_domain_state[MPIDR_AFFLVL0])

static uintptr_t rcar_sec_entrypoint;
static gicv3_redist_ctx_t rdist_ctx[PLATFORM_CORE_COUNT];
static gicv3_dist_ctx_t dist_ctx;

static void rcar_program_mailbox(u_register_t mpidr, uintptr_t address)
{
	uintptr_t range;
	mailbox_t *rcar_mboxes = (mailbox_t *) MBOX_BASE;
	int linear_id = plat_core_pos_by_mpidr(mpidr);

	if (linear_id < 0) {
		ERROR("BL3-1 : The value of passed MPIDR is invalid.");
		panic();
	}
	rcar_mboxes[linear_id].value = address;
	range = (uintptr_t)(&rcar_mboxes[linear_id]);

	flush_dcache_range(range, sizeof(mailbox_t));
}

static void rcar_cpu_standby(plat_local_state_t cpu_state)
{
	u_register_t scr_el3 = read_scr_el3();

	write_scr_el3(scr_el3 | SCR_IRQ_BIT);
	dsb();
	wfi();
	write_scr_el3(scr_el3);
}

static int rcar_pwr_domain_on(u_register_t mpidr)
{
	rcar_program_mailbox(mpidr, rcar_sec_entrypoint);
	rcar_scmi_cpuon(mpidr);

	return PSCI_E_SUCCESS;
}

static void rcar_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();

	rcar_pwrc_disable_interrupt_wakeup(mpidr);
	rcar_program_mailbox(mpidr, 0U);
}

static void rcar_pwr_domain_off(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();

	rcar_pwrc_disable_interrupt_wakeup(mpidr);

	rcar_scmi_cpuoff(target_state);
}

static void rcar_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	u_register_t mpidr = read_mpidr_el1();

	if (CORE_PWR_STATE(target_state) != PLAT_MAX_OFF_STATE) {
		return;
	}

	rcar_program_mailbox(mpidr, rcar_sec_entrypoint);
	rcar_pwrc_enable_interrupt_wakeup(mpidr);

	if (SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		for (unsigned int i = 0U; i < PLATFORM_CORE_COUNT; i++)
			gicv3_rdistif_save(i, &rdist_ctx[i]);
		gicv3_distif_save(&dist_ctx);
	}
}

static void rcar_pwr_domain_suspend_finish(const psci_power_state_t
					   *target_state)
{
	u_register_t mpidr = read_mpidr_el1();

	if (SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		rcar_pwrc_restore_timer_state();

		plat_rcar_scmi_setup();
	}

	rcar_pwrc_disable_interrupt_wakeup(mpidr);
	rcar_program_mailbox(mpidr, 0U);
	if (SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		gicv3_distif_init_restore(&dist_ctx);
		for (unsigned int i = 0U; i < PLATFORM_CORE_COUNT; i++)
			gicv3_rdistif_init_restore(i, &rdist_ctx[i]);
	}
}

static void __dead2 rcar_system_off(void)
{
	u_register_t mpidr = read_mpidr_el1();
	uint32_t rtn_on;

	rtn_on = rcar_pwrc_cpu_on_check(mpidr);

	if (rtn_on > 0U) {
		panic();
	}

	rcar_scmi_sys_shutdown();

	wfi();
	ERROR("RCAR System Off: operation not handled.\n");
	panic();
}

static void __dead2 rcar_system_reset(void)
{
	rcar_scmi_sys_reboot();

	wfi();

	ERROR("RCAR System Reset: operation not handled.\n");
	panic();
}

static void __dead2 rcar_pwr_domain_pwr_down_wfi(const psci_power_state_t *target_state)
{

	if (SYSTEM_PWR_STATE(target_state) == PLAT_MAX_OFF_STATE) {
		rcar_pwrc_suspend_to_ram();
	}

	wfi();

	ERROR("RCAR Power Down: operation not handled.\n");
	panic();
}

static int rcar_validate_power_state(unsigned int power_state,
				    psci_power_state_t *req_state)
{
	uint32_t pwr_lvl = psci_get_pstate_pwrlvl(power_state);
	uint32_t pstate = psci_get_pstate_type(power_state);
	uint64_t i;

	if (pstate == PSTATE_TYPE_STANDBY) {
		if (pwr_lvl != MPIDR_AFFLVL0) {
			return PSCI_E_INVALID_PARAMS;
		}

		req_state->pwr_domain_state[MPIDR_AFFLVL0] = PLAT_MAX_RET_STATE;
	} else {
		for (i = MPIDR_AFFLVL0; i <= (uint64_t)pwr_lvl; i++) {
			req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
		}
	}

	if (psci_get_pstate_id(power_state) != 0U) {
		return PSCI_E_INVALID_PARAMS;
	}

	return PSCI_E_SUCCESS;
}

static void rcar_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	uint64_t i;

	for (i = MPIDR_AFFLVL0; i <= (uint64_t)PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = PLAT_MAX_OFF_STATE;
	}
}

static plat_psci_ops_t rcar_plat_psci_ops = {
	.cpu_standby			= rcar_cpu_standby,
	.pwr_domain_on			= rcar_pwr_domain_on,
	.pwr_domain_off			= rcar_pwr_domain_off,
	.pwr_domain_suspend		= rcar_pwr_domain_suspend,
	.pwr_domain_on_finish		= rcar_pwr_domain_on_finish,
	.pwr_domain_suspend_finish	= rcar_pwr_domain_suspend_finish,
	.system_off			= rcar_system_off,
	.system_reset			= rcar_system_reset,
	.validate_power_state		= rcar_validate_power_state,
	.pwr_domain_pwr_down		= rcar_pwr_domain_pwr_down_wfi,
	.get_sys_suspend_power_state	= rcar_get_sys_suspend_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint, const plat_psci_ops_t **psci_ops)
{
	*psci_ops = plat_rcar_psci_override_pm_ops(&rcar_plat_psci_ops);
	rcar_sec_entrypoint = sec_entrypoint;

	return 0;
}
