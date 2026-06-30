/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/build_message.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <services/lfa_svc.h>

/*
 * These variables are used pre and post live activation so place them in the
 * relocatable data section to ensure they don't get moved around and persist
 * across live activation.
 */
static lfa_mailbox_message_t lfa_mailbox[PLATFORM_CORE_COUNT] __attribute__((section(".lfa_relocatable_data")));
static const plat_psci_ops_t *lfa_psci_ops __attribute__((section(".lfa_relocatable_data")));
static uintptr_t plat_wr_ep __attribute__((section(".lfa_relocatable_data")));
static uint64_t reset_cpu_count __attribute__((section(".lfa_relocatable_data")));
static spinlock_t cpucountlock __attribute__((section(".lfa_relocatable_data")));

extern plat_psci_ops_t *psci_plat_pm_ops;
extern int psci_validate_entry_point(entry_point_info_t *ep,
			      uintptr_t entrypoint,
			      u_register_t context_id);
extern void bl31_lfa_entrypoint(void);
extern void bl31_warm_entrypoint(void);

static void add_reset_cpu(void)
{
	spin_lock(&cpucountlock);
	reset_cpu_count++;
	spin_unlock(&cpucountlock);
}

static uint64_t remove_reset_cpu(void)
{
	uint64_t cpu_count;

	spin_lock(&cpucountlock);
	if (reset_cpu_count > 0) {
		reset_cpu_count--;
	}
	cpu_count = reset_cpu_count;
	spin_unlock(&cpucountlock);

	return cpu_count;
}

/*
 * Prepare CPU warm reset. This functioned should be called before entering
 * relocatable code for each CPU.
 */
uint64_t prepare_warm_reset(uintptr_t lfa_ns_ep, uint64_t context_id, uint64_t cpu_spinlock)
{
	const psci_power_state_t state_info = { {ARM_LOCAL_STATE_OFF} };
	unsigned int core_pos = plat_my_core_pos();
	lfa_mailbox_message_t mmesg;
	int rc;
	static spinlock_t setuplock;

	spin_lock(&setuplock);
	if (plat_wr_ep == 0 && plat_lfa_mailbox_base()) {
		plat_wr_ep = *((uintptr_t *)plat_lfa_mailbox_base());
	}

	if (lfa_psci_ops == NULL) {
		plat_setup_psci_ops((uintptr_t)bl31_lfa_entrypoint, &lfa_psci_ops);
	}
	spin_unlock(&setuplock);

	rc = psci_validate_entry_point(&mmesg.ep, lfa_ns_ep, (u_register_t)context_id);
	if (rc != PSCI_E_SUCCESS) {
		ERROR("BL31 LFA: Invalid NS entrypoint! (error %d)\n", rc);
		return LFA_CRITICAL_ERROR;
	}

	/* Stash the holding pen lock so we can release it after reboot. */
	mmesg.spinlock_p = (uint64_t)cpu_spinlock;

	/* Stash the platform warmboot entry point so we can restore it. */
	mmesg.plat_wr_mailbox = plat_wr_ep;
	memcpy((void *)&lfa_mailbox[core_pos], &mmesg, sizeof(lfa_mailbox_message_t));

	/*
	 * TODO: refactor PSCI suspend API to be usable here. The main issue is that
	 * PSCI suspend will put the core down immediately, but the use case here is
	 * slightly different since we have to do the firmware copy operation at the
	 * very end right before putting the core down for reset.
	 */
	if (lfa_psci_ops->pwr_domain_suspend != NULL) {
		/*
		 * We can't use the proper PSCI API since it will put the core
		 * down immediately, we still need to copy BL31.
		 */
#if USE_GIC_DRIVER
		gic_cpuif_disable(core_pos);
#endif
		lfa_psci_ops->pwr_domain_suspend(&state_info);
	} else {
		ERROR("BL31: PSCI power domain suspend not supported!\n");
		return LFA_CRITICAL_ERROR;
	}

	add_reset_cpu();

	cm_el2_sysregs_context_save(NON_SECURE);

	return LFA_SUCCESS;
}

/*
 * Entrypoint function for CPU warm reset case.
 */
void __no_pauth lfa_warm_reset_entrypoint(void)
{
	static spinlock_t resetlock;
	uint32_t counter_freq;
	const psci_power_state_t state_info = { {ARM_LOCAL_STATE_OFF} };
	lfa_mailbox_message_t mmesg;
	unsigned int core_pos = plat_my_core_pos();

#if ENABLE_FEAT_RME
	/*
	 * At warm boot GPT data structures have already been initialized in RAM
	 * but the sysregs for this CPU need to be initialized. Note that the GPT
	 * accesses are controlled attributes in GPCCR and do not depend on the
	 * SCR_EL3.C bit.
	 */
	if (gpt_enable() != 0) {
		panic();
	}
#endif

	/* TODO see if we can use psci_warmboot_entrypoint here. */
	psci_plat_pm_ops->pwr_domain_suspend_finish(&state_info);
#if USE_GIC_DRIVER
	gic_cpuif_enable(core_pos);
#endif

	/* Init registers that never change for the lifetime of TF-A */
	cm_manage_extensions_el3(core_pos);

	/* Re-init the cntfrq_el0 register */
	counter_freq = plat_get_syscnt_freq2();
	write_cntfrq_el0(counter_freq);

	memcpy(&mmesg, (void *)&lfa_mailbox[core_pos], sizeof(lfa_mailbox_message_t));

	spin_lock(&resetlock);
	NOTICE("BL31 LFA Warm Reset on CPU %u\n", core_pos);
	NOTICE("  Version : %s\n", build_version_string);
	NOTICE("  %s\n", build_message);
	console_flush();
	spin_unlock(&resetlock);

	/* The last CPU to reach here restores the warm entrypoint. */
	if (remove_reset_cpu() == 0) {
		plat_arm_program_trusted_mailbox(mmesg.plat_wr_mailbox);
		plat_wr_ep = 0;
		lfa_psci_ops = NULL;
	}

	spin_unlock((spinlock_t *)mmesg.spinlock_p);

	cm_init_my_context(&mmesg.ep);

	cm_prepare_el3_exit_ns();
}
