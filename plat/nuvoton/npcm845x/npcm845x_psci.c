/*
 * Copyright (c) 2015-2023, ARM Limited and Contributors. All rights reserved.
 *
 * Copyright (C) 2017-2023 Nuvoton Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/semihosting.h>
#include <npcm845x_clock.h>
#include <plat/arm/common/plat_arm.h>
#include <plat/common/platform.h>
#include <plat_npcm845x.h>

#define ADP_STOPPED_APPLICATION_EXIT 0x20026

/* Make composite power state parameter till power level 0 */
#if PSCI_EXTENDED_STATE_ID
/* Not Extended */
#define npcm845x_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		 ((type) << PSTATE_TYPE_SHIFT))
#else
#define npcm845x_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		 ((pwr_lvl) << PSTATE_PWR_LVL_SHIFT) | \
		 ((type) << PSTATE_TYPE_SHIFT))
#endif /* PSCI_EXTENDED_STATE_ID */

#define npcm845x_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl1_state) << PLAT_LOCAL_PSTATE_WIDTH) | \
		 npcm845x_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type))

/*
 * The table storing the valid idle power states. Ensure that the
 * array entries are populated in ascending order of state-id to
 * enable us to use binary search during power state validation.
 * The table must be terminated by a NULL entry.
 */
static const unsigned int npcm845x_pm_idle_states[] = {
/*
 * Cluster = 0 (RUN) CPU=1 (RET, higest in idle) -
 * Retention. The Power state is Stand-by
 */

/* State-id - 0x01 */
	npcm845x_make_pwrstate_lvl1(PLAT_LOCAL_STATE_RUN, PLAT_LOCAL_STATE_RET,
				MPIDR_AFFLVL0, PSTATE_TYPE_STANDBY),

/*
 * For testing purposes.
 * Only CPU suspend to standby is supported by NPCM845x
 */
	/* State-id - 0x02 */
	npcm845x_make_pwrstate_lvl1(PLAT_LOCAL_STATE_RUN, PLAT_LOCAL_STATE_OFF,
				MPIDR_AFFLVL0, PSTATE_TYPE_POWERDOWN),
	0,
};

/*******************************************************************************
 * Platform handler called to check the validity of the non secure
 * entrypoint.
 ******************************************************************************/
int npcm845x_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	NOTICE("%s() nuvoton_psci\n", __func__);
#ifdef PLAT_ARM_TRUSTED_DRAM_BASE
	if ((entrypoint >= PLAT_ARM_TRUSTED_DRAM_BASE) &&
		(entrypoint < (PLAT_ARM_TRUSTED_DRAM_BASE +
		PLAT_ARM_TRUSTED_DRAM_SIZE))) {
		return PSCI_E_INVALID_ADDRESS;
	}
#endif /* PLAT_ARM_TRUSTED_DRAM_BASE */
	/* For TFTS purposes, '0' is also illegal */
	#ifdef SPD_tspd
		if (entrypoint == 0) {
			return PSCI_E_INVALID_ADDRESS;
		}
	#endif /* SPD_tspd */
	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler called when a CPU is about to enter standby.
 ******************************************************************************/
void npcm845x_cpu_standby(plat_local_state_t cpu_state)
{
	NOTICE("%s() nuvoton_psci\n", __func__);

	uint64_t scr;

	scr = read_scr_el3();
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);

	/*
	 * Enter standby state
	 * dsb is good practice before using wfi to enter low power states
	 */
	isb();
	dsb();
	wfi();

	/* Once awake */
	write_scr_el3(scr);
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
int npcm845x_pwr_domain_on(u_register_t mpidr)
{
	int rc = PSCI_E_SUCCESS;
	int cpu_id = plat_core_pos_by_mpidr(mpidr);

	if ((unsigned int)cpu_id >= PLATFORM_CORE_COUNT) {
		ERROR("%s()  CPU 0x%X\n", __func__, cpu_id);
		return PSCI_E_INVALID_PARAMS;
	}

	if (cpu_id == -1) {
		/* domain on was not called by a CPU */
		ERROR("%s() was not per CPU 0x%X\n", __func__, cpu_id);
		return PSCI_E_INVALID_PARAMS;
	}

	unsigned int pos = (unsigned int)plat_core_pos_by_mpidr(mpidr);
	uintptr_t hold_base = PLAT_NPCM_TM_HOLD_BASE;

	assert(pos < PLATFORM_CORE_COUNT);

	hold_base += pos * PLAT_NPCM_TM_HOLD_ENTRY_SIZE;

	mmio_write_64(hold_base, PLAT_NPCM_TM_HOLD_STATE_GO);
	/* No cache maintenance here, hold_base is mapped as device memory. */

	/* Make sure that the write has completed */
	dsb();
	isb();

	sev();

	return rc;
}


/*******************************************************************************
 * Platform handler called when a power domain is about to be suspended. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void npcm845x_pwr_domain_suspend(const psci_power_state_t *target_state)
{
	NOTICE("%s() nuvoton_psci\n", __func__);

	for (size_t i = 0; (uint64_t)i <= PLAT_MAX_PWR_LVL; i++) {
		INFO("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	gicv2_cpuif_disable();

	NOTICE("%s() Out of suspend\n", __func__);
}


/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void npcm845x_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	NOTICE("%s() nuvoton_psci\n", __func__);

	for (size_t i = 0; (uint64_t)i <= PLAT_MAX_PWR_LVL; i++) {
		INFO("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
			PLAT_LOCAL_STATE_OFF);

	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}


/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * having been suspended earlier. The target_state encodes the low power state
 * that each level has woken up from.
 ******************************************************************************/
void npcm845x_pwr_domain_suspend_finish(const psci_power_state_t *target_state)
{
	NOTICE("%s() nuvoton_psci\n", __func__);

	for (size_t i = 0; (uint64_t)i <= PLAT_MAX_PWR_LVL; i++) {
		INFO("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	assert(target_state->pwr_domain_state[MPIDR_AFFLVL0] ==
			PLAT_LOCAL_STATE_OFF);

	gicv2_pcpu_distif_init();
	gicv2_cpuif_enable();
}


void __dead2 npcm845x_system_reset(void)
{
	uintptr_t RESET_BASE_ADDR;
	uint32_t val;

	NOTICE("%s() nuvoton_psci\n", __func__);
	console_flush();

	dsbsy();
	isb();

	/*
	 * In future - support all reset types. For now, SW1 reset
	 * Enable software reset 1 to reboot the BMC
	 */
	RESET_BASE_ADDR = (uintptr_t)0xF0801000;

	/* Read SW1 control register */
	val = mmio_read_32(RESET_BASE_ADDR + 0x44);
	/* Keep SPI BMC & MC persist*/
	val &= 0xFBFFFFDF;
	/* Setting SW1 control register */
	mmio_write_32(RESET_BASE_ADDR + 0x44, val);
	/* Set SW1 reset */
	mmio_write_32(RESET_BASE_ADDR + 0x14, 0x8);
	dsb();

	while (1) {
		;
	}
}

int npcm845x_validate_power_state(unsigned int power_state,
			 psci_power_state_t *req_state)
{
	unsigned int state_id;
	int i;

	NOTICE("%s() nuvoton_psci\n", __func__);
	assert(req_state);

	/*
	 *  Currently we are using a linear search for finding the matching
	 *  entry in the idle power state array. This can be made a binary
	 *  search if the number of entries justify the additional complexity.
	 */
	for (i = 0; !!npcm845x_pm_idle_states[i]; i++) {
		if (power_state == npcm845x_pm_idle_states[i]) {
			break;
		}
	}

	/* Return error if entry not found in the idle state array */
	if (!npcm845x_pm_idle_states[i]) {
		return PSCI_E_INVALID_PARAMS;
	}

	i = 0;
	state_id = psci_get_pstate_id(power_state);

	/* Parse the State ID and populate the state info parameter */
	while (state_id) {
		req_state->pwr_domain_state[i++] = (uint8_t)state_id &
						PLAT_LOCAL_PSTATE_MASK;
		state_id >>= PLAT_LOCAL_PSTATE_WIDTH;
	}

	return PSCI_E_SUCCESS;
}

/*
 * The NPCM845 doesn't truly support power management at SYSTEM power domain.
 * The SYSTEM_SUSPEND will be down-graded to the cluster level within
 * the platform layer. The `fake` SYSTEM_SUSPEND allows us to validate
 * some of the driver save and restore sequences on FVP.
 */
#if !ARM_BL31_IN_DRAM
void npcm845x_get_sys_suspend_power_state(psci_power_state_t *req_state)
{
	unsigned int i;

	NOTICE("%s() nuvoton_psci\n", __func__);

	for (i = ARM_PWR_LVL0; (uint64_t)i <= PLAT_MAX_PWR_LVL; i++) {
		req_state->pwr_domain_state[i] = (uint8_t)PLAT_LOCAL_STATE_OFF;
	}
}
#endif /* !ARM_BL31_IN_DRAM */

/*
 * The rest of the PSCI implementation are for testing purposes only.
 * Not supported in Arbel
 */
void __dead2 npcm845x_system_off(void)
{
	console_flush();

	dsbsy();
	isb();

	/* NPCM845 doesn't allow real system off, Do reaset instead */
	/* Do reset here TBD which, in the meanwhile SW1 reset */
	for (;;) {
		wfi();
	}
}

void __dead2 plat_secondary_cold_boot_setup(void);

void __dead2 npcm845x_pwr_down_wfi(
		const psci_power_state_t *target_state)
{
	uintptr_t hold_base = PLAT_NPCM_TM_HOLD_BASE;
	unsigned int pos = plat_my_core_pos();

	if (pos == 0) {
		/*
		 * The secondaries will always be in a wait
		 * for warm boot on reset, but the BSP needs
		 * to be able to distinguish between waiting
		 * for warm boot (e.g. after psci_off, waiting
		 * for psci_on) and a cold boot.
		 */
		mmio_write_64(hold_base, PLAT_NPCM_TM_HOLD_STATE_BSP_OFF);
		/* No cache maintenance here, we run with caches off already. */
		dsb();
		isb();
	}

	wfe();

	while (1) {
		;
	}
}

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned off. The
 * target_state encodes the power state that each level should transition to.
 ******************************************************************************/
void npcm845x_pwr_domain_off(const psci_power_state_t *target_state)
{
	NOTICE("%s() nuvoton_psci\n", __func__);

	for (size_t i = 0; (uint64_t)i <= PLAT_MAX_PWR_LVL; i++) {
		INFO("%s: target_state->pwr_domain_state[%lu]=%x\n",
			__func__, i, target_state->pwr_domain_state[i]);
	}

	plat_secondary_cold_boot_setup();
}

static const plat_psci_ops_t npcm845x_plat_psci_ops = {
	.cpu_standby = npcm845x_cpu_standby,
	.pwr_domain_on = npcm845x_pwr_domain_on,
	.pwr_domain_suspend = npcm845x_pwr_domain_suspend,
	.pwr_domain_on_finish = npcm845x_pwr_domain_on_finish,
	.pwr_domain_suspend_finish = npcm845x_pwr_domain_suspend_finish,
	.system_reset = npcm845x_system_reset,
	.validate_power_state = npcm845x_validate_power_state,
	.validate_ns_entrypoint = npcm845x_validate_ns_entrypoint,

	/* For testing purposes only This PSCI states are not supported */
	.pwr_domain_off = npcm845x_pwr_domain_off,
	.pwr_domain_pwr_down_wfi = npcm845x_pwr_down_wfi,
};

/* For reference only
 * typedef struct plat_psci_ops {
 *	void (*cpu_standby)(plat_local_state_t cpu_state);
 *	int (*pwr_domain_on)(u_register_t mpidr);
 *	void (*pwr_domain_off)(const psci_power_state_t *target_state);
 *	void (*pwr_domain_suspend_pwrdown_early)(
 *				const psci_power_state_t *target_state);
 *	void (*pwr_domain_suspend)(const psci_power_state_t *target_state);
 *	void (*pwr_domain_on_finish)(const psci_power_state_t *target_state);
 *	void (*pwr_domain_on_finish_late)(
 *				const psci_power_state_t *target_state);
 *	void (*pwr_domain_suspend_finish)(
 *				const psci_power_state_t *target_state);
 *	void __dead2 (*pwr_domain_pwr_down_wfi)(
 *				const psci_power_state_t *target_state);
 *	void __dead2 (*system_off)(void);
 *	void __dead2 (*system_reset)(void);
 *	int (*validate_power_state)(unsigned int power_state,
 *				psci_power_state_t *req_state);
 *	int (*validate_ns_entrypoint)(uintptr_t ns_entrypoint);
 *	void (*get_sys_suspend_power_state)(
 *				psci_power_state_t *req_state);
 *	int (*get_pwr_lvl_state_idx)(plat_local_state_t pwr_domain_state,
 *				int pwrlvl);
 *	int (*translate_power_state_by_mpidr)(u_register_t mpidr,
 *				unsigned int power_state,
 *				psci_power_state_t *output_state);
 *	int (*get_node_hw_state)(u_register_t mpidr, unsigned int power_level);
 *	int (*mem_protect_chk)(uintptr_t base, u_register_t length);
 *	int (*read_mem_protect)(int *val);
 *	int (*write_mem_protect)(int val);
 *	int (*system_reset2)(int is_vendor,
 *				int reset_type, u_register_t cookie);
 * } plat_psci_ops_t;
 */

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const plat_psci_ops_t **psci_ops)
{
	uintptr_t *entrypoint = (void *)PLAT_NPCM_TM_ENTRYPOINT;

	*entrypoint = sec_entrypoint;

	*psci_ops = &npcm845x_plat_psci_ops;

	return 0;
}
