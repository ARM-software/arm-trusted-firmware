/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bl31/interrupt_mgmt.h>

#include <c1_pro.h>
#include <drivers/arm/gicv3.h>
#include <drivers/delay_timer.h>

#include <lib/cpus/cpu_ops.h>
#include <lib/cpus/errata.h>
#include <lib/el3_runtime/pubsub_events.h>
#include <lib/spinlock.h>

#include <plat/common/platform.h>

#include <smccc_helpers.h>

#define C1_PRO_CVE_2026_0995_SGI_NR	ARM_IRQ_SEC_SGI_6
#define WAIT_TIMEOUT_US			10000ULL

#define BITS_PER_BYTE			8
#define SIZEOF_IN_BITS(x)		(sizeof(x) * BITS_PER_BYTE)

#if PLATFORM_CORE_COUNT > 64
#error "Platform has too many CPUs for C1 PRO CVE-2026-0995 workaround"
#endif

/* Bytemap of active C1-Pro core state. */
static int8_t c1_pro_active_core[PLATFORM_CORE_COUNT];

/* MPIDR of all online cores. */
static u_register_t mpidr_values[PLATFORM_CORE_COUNT];

/* Global counter to serialise parallel callers. */
static uint64_t global_counter;

/* local counter values */
static uint64_t local_counters[PLATFORM_CORE_COUNT];

/* Flag to know if C1-PRO cores needs to apply the workaround, This flag usage
 * is based on assumption all C1-PRO cores in system will have the same
 * revision values and thus all C1-PRO will be all affected or all not affected.
 */
static bool c1_pro_cve_2026_0995_needed;

static int wait_for_value_with_timeout(uint64_t *ptr, uint64_t val,
				       uint64_t timeout_us)
{
	uint64_t tmp;
	uint64_t deadline_pct, deadline_wfet;

	/* Make sure delay timer driver was initialized earlier:
	 * generic_delay_timer_init() or generic_delay_timer_init_args()
	 * (done during platform init).
	 */
	deadline_pct = timeout_init_us(timeout_us);
	deadline_wfet = deadline_pct - read_cntvoff_el2();

	for (;;) {
		__asm__ volatile(
		"sevl\n"
		"wfe\n"
		"ldxr	%0, [%[p]]\n"
		"cmp	%0, %[v]\n"
		"b.hs	1f\n"
		".arch_extension wfxt\n"
		"wfet	%x[t]\n"
		".arch_extension nowfxt\n"
		"1:\n"
		: "=&r"(tmp)
		: [p]"r"(ptr), [v]"r"(val), [t]"r"(deadline_wfet)
		: "memory");

		if (tmp >= val)
			return 0;

		if (timeout_elapsed(deadline_pct))
			return -ETIMEDOUT;
	}
}

static void c1_pro_cve_2026_0995_do_workaround(void)
{
	/*
	 * On C1-PRO, if SCTLR_EL3.IESB is set, then an exception to EL3 is
	 * sufficient to mitigate the erratum. This lets us skip the dsb in
	 * c1_pro_cve_2026_0995_do_workaround().
	 */
	if (!(read_sctlr_el3() & SCTLR_IESB_BIT)) {
		dsb();
	}

	local_counters[plat_my_core_pos()] =
			__atomic_load_n(&global_counter, __ATOMIC_RELAXED);
}

static void *c1_pro_cve_2026_0995_add_active_core(const void *arg)
{
	unsigned int core_pos = plat_my_core_pos();

	if (midr_match(C1_PRO_MIDR) != 0)  {
		++c1_pro_active_core[core_pos];

		/*
		 * This is the 0 -> 1 transition for this CPU's participation in
		 * the workaround. Enable the secure SGI and alos run the local
		 * mitigation once so that its local counter/state is brought up
		 * to date before the CPU is considered active.
		 */
		if (c1_pro_active_core[core_pos] == 1) {
			plat_ic_enable_interrupt(C1_PRO_CVE_2026_0995_SGI_NR);
			/*
			 * Updates to the active core list must be visible before
			 * any SME accesses can be issued.
			 */
			dmbish();
			c1_pro_cve_2026_0995_do_workaround();
		}
	}

	return (void *)0;
}

static void *c1_pro_cve_2026_0995_remove_active_core(const void *arg)
{
	unsigned int core_pos = plat_my_core_pos();

	if (midr_match(C1_PRO_MIDR) != 0) {
		--c1_pro_active_core[core_pos];

		if (c1_pro_active_core[core_pos] == 0) {
			/*
			 * Updates to the active core list must be visible before
			 * the workaround updates with the newest counter version.
			 * The SMC handler increments the counter before it reads
			 * the active core list.
			 */
			dmbish();

			c1_pro_cve_2026_0995_do_workaround();

			/*
			 * As part of disabling the interrupt gicv3_disable_interrupt()
			 * will wait for RWP to clear meaning the SGI has been recalled
			 * from the CPU interface. If another SGI is received, it will
			 * not prevent this CPU from entering WFI for power down.
			 */
			plat_ic_disable_interrupt(C1_PRO_CVE_2026_0995_SGI_NR);
		}
	}

	return (void *)0;
}

static uint64_t c1_pro_cve_2026_0995_interrupt_handler(uint32_t id,
						      uint32_t flags,
						      void *handle,
						      void *cookie)
{
	uint32_t irq = plat_ic_acknowledge_interrupt();

	/*
	 * The loads in the workaround may be made speculatively. If two SGIs
	 * get merged the loaded value may be older than the second SGI.
	 * Ensure that global_counter is read after the IAR_EL1 above using a
	 * control + ISB dependency.
	 */
	if (irq == C1_PRO_CVE_2026_0995_SGI_NR) {
		isb();
		c1_pro_cve_2026_0995_do_workaround();
	}

	plat_ic_end_of_interrupt(irq);

	return 0U;
}

static void *c1_pro_cve_2026_0995_setup_core(const void *arg)
{
	/* The MMU must be enabled for the use of atomics */
	assert((read_sctlr_el3() & SCTLR_M_BIT) != 0U);

	/*
	 * Configure the secure SGI consistently on all cores. Non-C1-Pro cores
	 * do not participate in the workaround rendezvous, but the interrupt
	 * configuration is kept system-wide so the SGI number, type and priority
	 * are known uniformly at EL3/GIC level.
	 */
	plat_ic_set_interrupt_type(C1_PRO_CVE_2026_0995_SGI_NR, INTR_TYPE_EL3);
	plat_ic_set_interrupt_priority(C1_PRO_CVE_2026_0995_SGI_NR,
				       GIC_HIGHEST_SEC_PRIORITY);

	/* Non C1-Pro cores don't need any of this */
	if (midr_match(C1_PRO_MIDR) == 0U) {
		return NULL;
	}

	/* If this is a C1-Pro core, check if the workaround needs to be applied. */
	if (check_erratum_c1_pro_995(cpu_get_rev_var()) == ERRATA_NOT_APPLIES) {
		return NULL;
	}

	c1_pro_cve_2026_0995_needed = true;

	/* Store the MPIDR of this core */
	mpidr_values[plat_my_core_pos()] = read_mpidr();

	/* Add active core will enable the SGI */
	c1_pro_cve_2026_0995_add_active_core(NULL);

	return NULL;
}

bool c1_pro_cve_2026_0995_applies(void)
{
	return c1_pro_cve_2026_0995_needed;
}

int c1_pro_cve_2026_0995_init(void)
{
	uint64_t flags = 0U;
	int ret;

	(void) c1_pro_cve_2026_0995_setup_core(NULL);

	/* Register handler for EL3 interrupts */
	set_interrupt_rm_flag(flags, NON_SECURE);
	ret = register_interrupt_type_handler(INTR_TYPE_EL3,
					      c1_pro_cve_2026_0995_interrupt_handler,
					      flags);

	return ret;
}

int c1_pro_cve_2026_0995_smc_handler(u_register_t arg, void *handle)
{
	uint64_t gc_val, ipid_cpus = 0;
	unsigned int i;
	bool c1_pro_core = false;
	int64_t smc_ret = SMC_OK;
	unsigned int core_pos = plat_my_core_pos();

	/* Increment the global counter, and keep the value */
	gc_val = __atomic_add_fetch(&global_counter, 1, __ATOMIC_RELAXED);

	/*
	 * Complete the atomic increment to ensure global_counter update
	 * is ordered before the local_counters.
	 */
	dmbish();

	if (midr_match(C1_PRO_MIDR) != 0)
		c1_pro_core = true;

	/* Send maintenance request IPI to other affected cores */
	for (i = 0; i < ARRAY_SIZE(c1_pro_active_core); i++) {
		if (i == core_pos && c1_pro_core) {
			c1_pro_cve_2026_0995_do_workaround();
			continue;
		}

		/* Validate active core state */
		if (c1_pro_active_core[i] < -1 ||
				c1_pro_active_core[i] > 1) {
			ERROR("C1-PRO CVE-2026-0995 workaround: "
			      "Core[%d] has an invalid active core count [%d]\n",
			      i, (int)c1_pro_active_core[i]);
			continue;
		}

		/* Skip cores that are not active or participating */
		if (c1_pro_active_core[i] != 1) {
			continue;
		}

		plat_ic_raise_el3_sgi(C1_PRO_CVE_2026_0995_SGI_NR,
				      mpidr_values[i]);
		ipid_cpus |= BIT(i);
	}

	for (i = 0; i < SIZEOF_IN_BITS(ipid_cpus); i++) {
		/* Skip cores that are not participating */
		if (!(ipid_cpus & BIT(i))) {
			continue;
		}

		if (wait_for_value_with_timeout(&local_counters[i], gc_val,
						WAIT_TIMEOUT_US)) {
			ERROR("C1-PRO CVE-2026-0995 workaround timeout on core %u\n", i);
			smc_ret = SMC_UNK;
		}
	}

	SMC_RET1(handle, smc_ret);
}

/* TODO: add (new) events to also catch suspended to retention cores */
SUBSCRIBE_TO_EVENT(psci_cpu_on_finish, c1_pro_cve_2026_0995_setup_core);
SUBSCRIBE_TO_EVENT(psci_cpu_off_start, c1_pro_cve_2026_0995_remove_active_core);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_start, c1_pro_cve_2026_0995_remove_active_core);
SUBSCRIBE_TO_EVENT(psci_suspend_pwrdown_finish, c1_pro_cve_2026_0995_add_active_core);
SUBSCRIBE_TO_EVENT(cm_entering_secure_world, c1_pro_cve_2026_0995_remove_active_core);
SUBSCRIBE_TO_EVENT(cm_exited_secure_world, c1_pro_cve_2026_0995_add_active_core);
