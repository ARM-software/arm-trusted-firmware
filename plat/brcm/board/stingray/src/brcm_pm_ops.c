/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/arm/ccn.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>

#include <brcm_scpi.h>
#include <chimp.h>
#include <cmn_plat_util.h>
#include <plat_brcm.h>
#include <platform_def.h>
#include <sr_utils.h>

#include "m0_cfg.h"


#define CORE_PWR_STATE(state)	((state)->pwr_domain_state[MPIDR_AFFLVL0])
#define CLUSTER_PWR_STATE(state)	\
			((state)->pwr_domain_state[MPIDR_AFFLVL1])
#define SYSTEM_PWR_STATE(state)	((state)->pwr_domain_state[MPIDR_AFFLVL2])

#define VENDOR_RST_TYPE_SHIFT	4

#if HW_ASSISTED_COHERENCY
/*
 * On systems where participant CPUs are cache-coherent, we can use spinlocks
 * instead of bakery locks.
 */
spinlock_t event_lock;
#define event_lock_get(_lock) spin_lock(&_lock)
#define event_lock_release(_lock) spin_unlock(&_lock)

#else
/*
 * Use bakery locks for state coordination as not all participants are
 * cache coherent now.
 */
DEFINE_BAKERY_LOCK(event_lock);
#define event_lock_get(_lock) bakery_lock_get(&_lock)
#define event_lock_release(_lock) bakery_lock_release(&_lock)
#endif

static int brcm_pwr_domain_on(u_register_t mpidr)
{
	/*
	 * SCP takes care of powering up parent power domains so we
	 * only need to care about level 0
	 */
	scpi_set_brcm_power_state(mpidr, scpi_power_on, scpi_power_on,
				  scpi_power_on);

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Handler called when a power level has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from. This handler would never be invoked with
 * the system power domain uninitialized as either the primary would have taken
 * care of it as part of cold boot or the first core awakened from system
 * suspend would have already initialized it.
 ******************************************************************************/
static void brcm_pwr_domain_on_finish(const psci_power_state_t *target_state)
{
	unsigned long cluster_id = MPIDR_AFFLVL1_VAL(read_mpidr());

	/* Assert that the system power domain need not be initialized */
	assert(SYSTEM_PWR_STATE(target_state) == PLAT_LOCAL_STATE_RUN);

	assert(CORE_PWR_STATE(target_state) == PLAT_LOCAL_STATE_OFF);

	/*
	 * Perform the common cluster specific operations i.e enable coherency
	 * if this cluster was off.
	 */
	if (CLUSTER_PWR_STATE(target_state) == PLAT_LOCAL_STATE_OFF) {
		INFO("Cluster #%lu entering to snoop/dvm domain\n", cluster_id);
		ccn_enter_snoop_dvm_domain(1 << cluster_id);
	}

	/* Program the gic per-cpu distributor or re-distributor interface */
	plat_brcm_gic_pcpu_init();

	/* Enable the gic cpu interface */
	plat_brcm_gic_cpuif_enable();
}

static void brcm_power_down_common(void)
{
	unsigned int standbywfil2, standbywfi;
	uint64_t mpidr = read_mpidr_el1();

	switch (MPIDR_AFFLVL1_VAL(mpidr)) {
	case 0x0:
		standbywfi = CDRU_PROC_EVENT_CLEAR__IH0_CDRU_STANDBYWFI;
		standbywfil2 = CDRU_PROC_EVENT_CLEAR__IH0_CDRU_STANDBYWFIL2;
		break;
	case 0x1:
		standbywfi = CDRU_PROC_EVENT_CLEAR__IH1_CDRU_STANDBYWFI;
		standbywfil2 = CDRU_PROC_EVENT_CLEAR__IH1_CDRU_STANDBYWFIL2;
		break;
	case 0x2:
		standbywfi = CDRU_PROC_EVENT_CLEAR__IH2_CDRU_STANDBYWFI;
		standbywfil2 = CDRU_PROC_EVENT_CLEAR__IH2_CDRU_STANDBYWFIL2;
		break;
	case 0x3:
		standbywfi = CDRU_PROC_EVENT_CLEAR__IH3_CDRU_STANDBYWFI;
		standbywfil2 = CDRU_PROC_EVENT_CLEAR__IH3_CDRU_STANDBYWFIL2;
		break;
	default:
		ERROR("Invalid cluster #%" PRIx64 "\n", MPIDR_AFFLVL1_VAL(mpidr));
		return;
	}
	/* Clear the WFI status bit */
	event_lock_get(event_lock);
	mmio_setbits_32(CDRU_PROC_EVENT_CLEAR,
			(1 << (standbywfi + MPIDR_AFFLVL0_VAL(mpidr))) |
			(1 << standbywfil2));
	event_lock_release(event_lock);
}

/*
 * Helper function to inform power down state to SCP.
 */
static void brcm_scp_suspend(const psci_power_state_t *target_state)
{
	uint32_t cluster_state = scpi_power_on;
	uint32_t system_state = scpi_power_on;

	/* Check if power down at system power domain level is requested */
	if (SYSTEM_PWR_STATE(target_state) == PLAT_LOCAL_STATE_OFF)
		system_state = scpi_power_retention;

	/* Check if Cluster is to be turned off */
	if (CLUSTER_PWR_STATE(target_state) == PLAT_LOCAL_STATE_OFF)
		cluster_state = scpi_power_off;

	/*
	 * Ask the SCP to power down the appropriate components depending upon
	 * their state.
	 */
	scpi_set_brcm_power_state(read_mpidr_el1(),
				  scpi_power_off,
				  cluster_state,
				  system_state);
}

/*
 * Helper function to turn off a CPU power domain and its parent power domains
 * if applicable. Since SCPI doesn't differentiate between OFF and suspend, we
 * call the suspend helper here.
 */
static void brcm_scp_off(const psci_power_state_t *target_state)
{
	brcm_scp_suspend(target_state);
}

static void brcm_pwr_domain_off(const psci_power_state_t *target_state)
{
	unsigned long cluster_id = MPIDR_AFFLVL1_VAL(read_mpidr_el1());

	assert(CORE_PWR_STATE(target_state) == PLAT_LOCAL_STATE_OFF);
	/* Prevent interrupts from spuriously waking up this cpu */
	plat_brcm_gic_cpuif_disable();

	/* Turn redistributor off */
	plat_brcm_gic_redistif_off();

	/* If Cluster is to be turned off, disable coherency */
	if (CLUSTER_PWR_STATE(target_state) == PLAT_LOCAL_STATE_OFF)
		ccn_exit_snoop_dvm_domain(1 << cluster_id);

	brcm_power_down_common();

	brcm_scp_off(target_state);
}

/*******************************************************************************
 * Handler called when the CPU power domain is about to enter standby.
 ******************************************************************************/
static void brcm_cpu_standby(plat_local_state_t cpu_state)
{
	unsigned int scr;

	assert(cpu_state == PLAT_LOCAL_STATE_RET);

	scr = read_scr_el3();
	/*
	 * Enable the Non secure interrupt to wake the CPU.
	 * In GICv3 affinity routing mode, the non secure group1 interrupts use
	 * the PhysicalFIQ at EL3 whereas in GICv2, it uses the PhysicalIRQ.
	 * Enabling both the bits works for both GICv2 mode and GICv3 affinity
	 * routing mode.
	 */
	write_scr_el3(scr | SCR_IRQ_BIT | SCR_FIQ_BIT);
	isb();
	dsb();
	wfi();

	/*
	 * Restore SCR to the original value, synchronisation of scr_el3 is
	 * done by eret while el3_exit to save some execution cycles.
	 */
	write_scr_el3(scr);
}

/*
 * Helper function to shutdown the system via SCPI.
 */
static void __dead2 brcm_scp_sys_shutdown(void)
{
	/*
	 * Disable GIC CPU interface to prevent pending interrupt
	 * from waking up the AP from WFI.
	 */
	plat_brcm_gic_cpuif_disable();

	/* Flush and invalidate data cache */
	dcsw_op_all(DCCISW);

	/* Bring Cluster out of coherency domain as its going to die */
	plat_brcm_interconnect_exit_coherency();

	brcm_power_down_common();

	/* Send the power down request to the SCP */
	scpi_sys_power_state(scpi_system_shutdown);

	wfi();
	ERROR("BRCM System Off: operation not handled.\n");
	panic();
}

/*
 * Helper function to reset the system
 */
static void __dead2 brcm_scp_sys_reset(unsigned int reset_type)
{
	/*
	 * Disable GIC CPU interface to prevent pending interrupt
	 * from waking up the AP from WFI.
	 */
	plat_brcm_gic_cpuif_disable();

	/* Flush and invalidate data cache */
	dcsw_op_all(DCCISW);

	/* Bring Cluster out of coherency domain as its going to die */
	plat_brcm_interconnect_exit_coherency();

	brcm_power_down_common();

	/* Send the system reset request to the SCP
	 *
	 * As per PSCI spec system power state could be
	 * 0-> Shutdown
	 * 1-> Reboot- Board level Reset
	 * 2-> Reset - SoC level Reset
	 *
	 * Spec allocates 8 bits, 2 nibble, for this. One nibble is sufficient
	 * for sending the state hence We are utilizing 2nd nibble for vendor
	 * define reset type.
	 */
	scpi_sys_power_state((reset_type << VENDOR_RST_TYPE_SHIFT) |
			     scpi_system_reboot);

	wfi();
	ERROR("BRCM System Reset: operation not handled.\n");
	panic();
}

static void __dead2 brcm_system_reset(void)
{
	unsigned int reset_type;

	if (bcm_chimp_is_nic_mode())
		reset_type = SOFT_RESET_L3;
	else
		reset_type = SOFT_SYS_RESET_L1;

	brcm_scp_sys_reset(reset_type);
}

static int brcm_system_reset2(int is_vendor, int reset_type,
		      u_register_t cookie)
{
	if (!is_vendor) {
		/* Architectural warm boot: only warm reset is supported */
		reset_type = SOFT_RESET_L3;
	} else {
		uint32_t boot_source = (uint32_t)cookie;

		boot_source &= BOOT_SOURCE_MASK;
		brcm_stingray_set_straps(boot_source);
	}
	brcm_scp_sys_reset(reset_type);

	/*
	 * brcm_scp_sys_reset cannot return (it is a __dead function),
	 * but brcm_system_reset2 has to return some value, even in
	 * this case.
	 */
	return 0;
}

static int brcm_validate_ns_entrypoint(uintptr_t entrypoint)
{
	/*
	 * Check if the non secure entrypoint lies within the non
	 * secure DRAM.
	 */
	if ((entrypoint >= BRCM_NS_DRAM1_BASE) &&
	    (entrypoint < (BRCM_NS_DRAM1_BASE + BRCM_NS_DRAM1_SIZE)))
		return PSCI_E_SUCCESS;
#ifdef __aarch64__
	if ((entrypoint >= BRCM_DRAM2_BASE) &&
	    (entrypoint < (BRCM_DRAM2_BASE + BRCM_DRAM2_SIZE)))
		return PSCI_E_SUCCESS;

	if ((entrypoint >= BRCM_DRAM3_BASE) &&
	    (entrypoint < (BRCM_DRAM3_BASE + BRCM_DRAM3_SIZE)))
		return PSCI_E_SUCCESS;
#endif

	return PSCI_E_INVALID_ADDRESS;
}

/*******************************************************************************
 * ARM standard platform handler called to check the validity of the power state
 * parameter.
 ******************************************************************************/
static int brcm_validate_power_state(unsigned int power_state,
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
					PLAT_LOCAL_STATE_RET;
	} else {
		for (i = MPIDR_AFFLVL0; i <= pwr_lvl; i++)
			req_state->pwr_domain_state[i] =
					PLAT_LOCAL_STATE_OFF;
	}

	/*
	 * We expect the 'state id' to be zero.
	 */
	if (psci_get_pstate_id(power_state))
		return PSCI_E_INVALID_PARAMS;

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Export the platform handlers via plat_brcm_psci_pm_ops. The ARM Standard
 * platform will take care of registering the handlers with PSCI.
 ******************************************************************************/
plat_psci_ops_t plat_brcm_psci_pm_ops = {
	.pwr_domain_on		= brcm_pwr_domain_on,
	.pwr_domain_on_finish	= brcm_pwr_domain_on_finish,
	.pwr_domain_off		= brcm_pwr_domain_off,
	.cpu_standby		= brcm_cpu_standby,
	.system_off		= brcm_scp_sys_shutdown,
	.system_reset		= brcm_system_reset,
	.system_reset2		= brcm_system_reset2,
	.validate_ns_entrypoint = brcm_validate_ns_entrypoint,
	.validate_power_state	= brcm_validate_power_state,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	*psci_ops = &plat_brcm_psci_pm_ops;

	/* Setup mailbox with entry point. */
	mmio_write_64(CRMU_CFG_BASE + offsetof(M0CFG, core_cfg.rvbar),
		      sec_entrypoint);

	return 0;
}
