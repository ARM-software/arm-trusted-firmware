/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <cpucp.h>
#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>
#include <lib/spinlock.h>
#include <lib/utils_def.h>
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>

/*
 * Native (hoya/lemans) backend for the QTI PSCI power-domain hooks declared in
 * qti_plat.h. The secondary-core cold-boot bringup and reset-vector setup are
 * performed natively via the raw APSS IPM register sequence below. CPU_OFF
 * collapses the core through its IPM sequencer where the chipset provides the
 * sequence. Idle is CPU-level standby (WFI retention) only: the cluster and
 * system low-power modes (RSC/PDC) are not ported.
 */

/* Per-core APSS IPM register block. */
#define APSS_CPU_IPM_REG_BASE			0x18000000U
#define APSS_CPU_IPM_REG_OFFSET			0x10000U
#define APSS_CPU_IPM_REG(core)			(APSS_CPU_IPM_REG_BASE + \
						 ((core) * APSS_CPU_IPM_REG_OFFSET))

#define CPU_HEAD_SWITCH_CTL(core)		(APSS_CPU_IPM_REG(core) + 0x08U)
#define CPU_SEQ_FORCE_PWR_CTL_EN(core)		(APSS_CPU_IPM_REG(core) + 0x1cU)
#define CPU_SEQ_FORCE_PWR_CTL_VAL(core)		(APSS_CPU_IPM_REG(core) + 0x20U)
#define CPU_SEQ_CTL(core)			(APSS_CPU_IPM_REG(core) + 0x24U)
#define CPU_PCHANNEL_FSM_CTL(core)		(APSS_CPU_IPM_REG(core) + 0x44U)
#define CPU_PCHANNEL_CTLER_STATUS(core)		(APSS_CPU_IPM_REG(core) + 0x4cU)

#define CPU_SEQ_CTL_RESET			0x400U
#define CPU_SEQ_CTL_EN				BIT_32(0)
#define CPU_SEQ_CTL_PWR_STATE_IDX(idx)		((idx) << 8)
#define CPU_SEQ_CTL_START_ADDR(cmd)		((cmd) << 16)
#define PCHANNEL_MASK_WAKEUP			BIT_32(13)
#define CTLER_STATUS_LPM_ENTER_COND		BIT_32(12)

#define CPU_OFF_TIMEOUT_US			10000U

/* Sequencer instruction memory, four 8-bit commands per word. */
#define APSS_SEQ_COMMON_INSTR(n)		(0x180c0000U + ((n) * 4U))

static const uint32_t cpu_off_seq[] = QTI_CPU_OFF_SEQ;

/*
 * APSS cluster (L3/DSU) IPM alias register block (APSS_ALIAS_1). Used for the
 * L3 sequences of a separate gold DSU.
 */
#define APSS_CL_IPM_REG_BASE			0x18090000U

#define L3_SEQ_FORCE_PWR_CTL_EN			(APSS_CL_IPM_REG_BASE + 0x1cU)
#define L3_SEQ_FORCE_PWR_CTL_VAL		(APSS_CL_IPM_REG_BASE + 0x20U)
#define L3_SEQ_STS1				(APSS_CL_IPM_REG_BASE + 0x38U)
#define CL_PCHANNEL_FSM_CTL			(APSS_CL_IPM_REG_BASE + 0x44U)

#define L3_SEQ_STS1_MEM_REPAIR_DONE		0x40000U
#define L3_SEQ_FORCE_PWR_CTL_MEM_REPAIR		0x2000000U

/*
 * IPM sequencer that repairs the memories of the gold cores on the gold rail:
 * force power control enable and value registers, and a status register.
 */
#define CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_EN	(QTI_APC1_MEM_REPAIR_SEQ)
#define CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_VAL	(QTI_APC1_MEM_REPAIR_SEQ + 0x4U)
#define CPU_MEM_REPAIR_SEQ_STS1			(QTI_APC1_MEM_REPAIR_SEQ + 0x10U)

/* Upper bound for the memory-repair "done" handshake. */
#define MEM_REPAIR_TIMEOUT_US			10000U

/*
 * Gold-cluster (APC1) SAW4 AVS rail. The SAW4 instance for the gold rail is
 * at 0x18101000, with the AVS register region at +0x800 and the status
 * region at +0xc00.
 */
#define GOLD_SAW4_BASE				0x18101000U
#define GOLD_SAW4_VCTL				(GOLD_SAW4_BASE + 0x900U)
#define GOLD_SAW4_AVS_CTL			(GOLD_SAW4_BASE + 0x904U)
#define GOLD_SAW4_PMIC_STS			(GOLD_SAW4_BASE + 0xc18U)

#define SAW4_AVS_CTL_EN				0x1U
#define SAW4_AVS_CTL_CTL_SEL			0x10U

#define SAW4_PMIC_STS_STATE_MASK		0x30000U
#define SAW4_PMIC_STS_CURR_DATA_MASK		0xffffU

/*
 * SAW4_VCTL encodes SIZE[20], ADR_IDX[18:16] and PMIC_DATA[15:0]. The enable
 * write uses the 8-bit EN address index (3) with data 0x80; the voltage write
 * uses the 16-bit VCTL address index (0) with the boot voltage as data.
 */
#define SAW4_VCTL_ENABLE			0x30080U
#define SAW4_VCTL_SET_VOLTAGE			(0x100000U | QTI_GOLD_RAIL_BOOT_MV)

#define SAW4_PMIC_WRITE_RETRY			200U

/*
 * State of the one-time gold domain bring-up, also reported by each of its
 * steps; a domain that failed is not retried.
 */
enum gold_domain_state {
	GOLD_DOMAIN_OFF,
	GOLD_DOMAIN_ON,
	GOLD_DOMAIN_FAIL,
};

/*
 * Repair the L3 memories of the gold cluster. Performed once, before the first
 * gold core is powered on.
 */
static enum gold_domain_state l3_memory_repair(void)
{
	uint32_t val;
	uint64_t timeout;

	val = mmio_read_32(L3_SEQ_FORCE_PWR_CTL_VAL);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL,
		      val | L3_SEQ_FORCE_PWR_CTL_MEM_REPAIR);

	val = mmio_read_32(L3_SEQ_FORCE_PWR_CTL_EN);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_EN,
		      val | L3_SEQ_FORCE_PWR_CTL_MEM_REPAIR);

	timeout = timeout_init_us(MEM_REPAIR_TIMEOUT_US);
	while ((mmio_read_32(L3_SEQ_STS1) & L3_SEQ_STS1_MEM_REPAIR_DONE) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("L3 memory repair timed out\n");
			return GOLD_DOMAIN_FAIL;
		}
	}

	val = mmio_read_32(L3_SEQ_FORCE_PWR_CTL_VAL);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL,
		      val & ~L3_SEQ_FORCE_PWR_CTL_MEM_REPAIR);

	val = mmio_read_32(L3_SEQ_FORCE_PWR_CTL_EN);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_EN,
		      val & ~L3_SEQ_FORCE_PWR_CTL_MEM_REPAIR);

	return GOLD_DOMAIN_ON;
}

/* Execute the gold-cluster L3 turn-on sequence. Performed once. */
static void l3_cold_boot(void)
{
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_EN, 0x0U);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL, 0x010801a2U);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL, 0x01080082U);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL, 0x0108008aU);
	udelay(20);

	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL, 0x0108008eU);
	udelay(20);

	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL, 0x0108009eU);
	udelay(4);

	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL, 0x0108008eU);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL, 0x0108008cU);
	udelay(4);

	mmio_write_32(CL_PCHANNEL_FSM_CTL, 0x2000001U);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL, 0x8004cU);
	mmio_write_32(CL_PCHANNEL_FSM_CTL, 0x2000000U);
}

/*
 * Repair the memories of the gold cores. Performed once, after the gold rail
 * is up and before the first gold core is powered on.
 */
static enum gold_domain_state cpu_memory_repair(void)
{
	uint32_t val;
	uint64_t timeout;

	val = mmio_read_32(CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_EN);
	mmio_write_32(CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_EN,
		      val | QTI_APC1_MEM_REPAIR_REQ);

	val = mmio_read_32(CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_VAL);
	mmio_write_32(CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_VAL,
		      val | QTI_APC1_MEM_REPAIR_REQ);

	timeout = timeout_init_us(MEM_REPAIR_TIMEOUT_US);
	while ((mmio_read_32(CPU_MEM_REPAIR_SEQ_STS1) &
		QTI_APC1_MEM_REPAIR_DONE) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("Memory repair of sequencer 0x%lx timed out\n",
			      (unsigned long)QTI_APC1_MEM_REPAIR_SEQ);
			return GOLD_DOMAIN_FAIL;
		}
	}

	val = mmio_read_32(CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_VAL);
	mmio_write_32(CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_VAL,
		      val & ~QTI_APC1_MEM_REPAIR_REQ);

	val = mmio_read_32(CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_EN);
	mmio_write_32(CPU_MEM_REPAIR_SEQ_FORCE_PWR_CTL_EN,
		      val & ~QTI_APC1_MEM_REPAIR_REQ);

	return GOLD_DOMAIN_ON;
}

/*
 * Wait for the gold-cluster SAW4 PMIC write to complete. Returns true when the
 * PMIC is idle (and, if requested, matches @expect_data), false on timeout.
 */
static bool saw4_pmic_wait(uint32_t expect_data, bool check_data)
{
	unsigned int retry = SAW4_PMIC_WRITE_RETRY;

	while (retry-- > 0U) {
		uint32_t sts = mmio_read_32(GOLD_SAW4_PMIC_STS);

		if (((sts & SAW4_PMIC_STS_STATE_MASK) == 0U) &&
		    (!check_data ||
		     ((sts & SAW4_PMIC_STS_CURR_DATA_MASK) == expect_data))) {
			return true;
		}

		udelay(2);
	}

	return false;
}

/*
 * Enable the gold-cluster (APC1) SAW4 AVS rail and drive it to its boot
 * voltage; without it the gold cluster has no supply and its cores never
 * come out of reset.
 */
static enum gold_domain_state set_gold_cluster_voltage(void)
{
	uint32_t val;

	/* Let the SAW hardware adjust the rail voltage. */
	val = mmio_read_32(GOLD_SAW4_AVS_CTL);
	mmio_write_32(GOLD_SAW4_AVS_CTL, val | SAW4_AVS_CTL_EN);

	val = mmio_read_32(GOLD_SAW4_AVS_CTL);
	mmio_write_32(GOLD_SAW4_AVS_CTL, val | SAW4_AVS_CTL_CTL_SEL);

	/* Enable the PMIC rail and wait for warm-up. */
	mmio_write_32(GOLD_SAW4_VCTL, SAW4_VCTL_ENABLE);
	if (!saw4_pmic_wait(0U, false)) {
		ERROR("gold SAW4 PMIC enable timed out\n");
		return GOLD_DOMAIN_FAIL;
	}
	udelay(90);

	/* Restore the rail to its boot voltage. */
	mmio_write_32(GOLD_SAW4_VCTL, SAW4_VCTL_SET_VOLTAGE);
	if (!saw4_pmic_wait(QTI_GOLD_RAIL_BOOT_MV, true)) {
		ERROR("gold SAW4 PMIC voltage set timed out\n");
		return GOLD_DOMAIN_FAIL;
	}

	return GOLD_DOMAIN_ON;
}

/*
 * Gold domain bring-up: the gold rail, the L3 of the gold DSU where it is a
 * separate one, then the memories of the gold cores where a sequencer
 * repairs them.
 */
static enum gold_domain_state gold_domain_bring_up(void)
{
	if (set_gold_cluster_voltage() != GOLD_DOMAIN_ON) {
		return GOLD_DOMAIN_FAIL;
	}

	if (QTI_GOLD_HAS_SEPARATE_DSU != 0) {
		if (l3_memory_repair() != GOLD_DOMAIN_ON) {
			return GOLD_DOMAIN_FAIL;
		}
		l3_cold_boot();
	}

	if ((QTI_APC1_MEM_REPAIR_SEQ != 0U) &&
	    (cpu_memory_repair() != GOLD_DOMAIN_ON)) {
		return GOLD_DOMAIN_FAIL;
	}

	return GOLD_DOMAIN_ON;
}

/* One-time gold domain bring-up, run before its first core powers on. */
static enum gold_domain_state gold_domain_cold_boot(void)
{
	static spinlock_t gold_domain_lock;
	static enum gold_domain_state gold_domain;
	enum gold_domain_state ret;

	spin_lock(&gold_domain_lock);
	if (gold_domain == GOLD_DOMAIN_OFF) {
		gold_domain = gold_domain_bring_up();
	}
	ret = gold_domain;
	spin_unlock(&gold_domain_lock);

	return ret;
}

/* plat_qti_pwr_psci_init - PSCI backend init hook. */
int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;

	for (unsigned int i = 0U; i < ARRAY_SIZE(cpu_off_seq); i++) {
		mmio_write_32(APSS_SEQ_COMMON_INSTR(QTI_CPU_OFF_SEQ_START / 4U + i),
			      cpu_off_seq[i]);
	}

	return PSCI_E_SUCCESS;
}

/*
 * Wake a core its sequencer collapsed at CPU_OFF: the sequence powers it up
 * on the wake event. CPU_ON can arrive before the core has reached its last
 * WFI, so wait for the collapse first.
 */
static int cpu_wake(u_register_t mpidr, int core_pos)
{
	uint64_t timeout = timeout_init_us(CPU_OFF_TIMEOUT_US);

	while ((mmio_read_32(CPU_PCHANNEL_CTLER_STATUS(core_pos)) &
		CTLER_STATUS_LPM_ENTER_COND) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("CPU%d did not power down\n", core_pos);
			return PSCI_E_INTERN_FAIL;
		}
	}

	plat_ic_raise_el3_sgi(PLAT_INT_ID_CPU_WAKEUP_SGI, mpidr);
	mmio_clrbits_32(CPU_PCHANNEL_FSM_CTL(core_pos), PCHANNEL_MASK_WAKEUP);

	return PSCI_E_SUCCESS;
}

/*
 * plat_qti_pwr_domain_on - power on a secondary core using the raw APSS IPM
 * reset sequence.
 */
int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	if ((ARRAY_SIZE(cpu_off_seq) != 0U) &&
	    ((mmio_read_32(CPU_SEQ_CTL(core_pos)) & CPU_SEQ_CTL_EN) != 0U)) {
		return cpu_wake(mpidr, core_pos);
	}

	/* Bring up the gold domain before powering on its first core. */
	if ((core_pos >= QTI_FIRST_GOLD_CORE) &&
	    (gold_domain_cold_boot() != GOLD_DOMAIN_ON)) {
		return PSCI_E_INTERN_FAIL;
	}

	/* Program skew between en_few and en_rest. */
	mmio_write_32(CPU_HEAD_SWITCH_CTL(core_pos), 0x28U);

	/* Clear power-control enables. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_EN(core_pos), 0x0U);

	/* Close the core logic head switch. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x642U);
	udelay(2);

	/* Deassert core memory and logic clamp. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x402U);

	/* Deassert core memory slp_nret_n and slp_ret_n. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x40aU);
	udelay(4);
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x40eU);
	udelay(4);

	/* Assert and deassert wl_en_clk. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x50eU);
	udelay(2);
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x40eU);

	/* Deassert clock off. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x40cU);
	udelay(2);

	/* Assert the core P-channel power-up request. */
	mmio_write_32(CPU_PCHANNEL_FSM_CTL(core_pos), 0x1U);

	/* Deassert core reset. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x43cU);

	/* Deassert the core P-channel power-up request. */
	mmio_write_32(CPU_PCHANNEL_FSM_CTL(core_pos), 0x0U);

	/* Assert OSM core active. */
	mmio_write_32(CPU_SEQ_FORCE_PWR_CTL_VAL(core_pos), 0x443cU);

	/* Assert CPU_PWRDUP. */
	mmio_write_32(CPU_HEAD_SWITCH_CTL(core_pos), 0x428U);

	return PSCI_E_SUCCESS;
}

/*
 * plat_qti_pwr_domain_on_finish - per-core setup once it has come online.
 *
 * Initialise the GIC redistributor for the core, request CPUCP to enable
 * the core's clock domain and disable the core sequencer armed at CPU_OFF.
 */
void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)states;

	plat_qti_gic_pcpu_init();
	cpucp_clkdom_init();

	if (ARRAY_SIZE(cpu_off_seq) != 0U) {
		mmio_write_32(CPU_SEQ_CTL(core_pos), CPU_SEQ_CTL_RESET);
	}
}

/*
 * Have the core sequencer collapse the core on the power-down request it
 * raises at its last WFI, with wake-ups masked so that an interrupt cannot
 * cut the collapse short. CPU_ON then powers it up again from off.
 */
void plat_qti_pwr_domain_off(const uint8_t *states)
{
	unsigned int core;

	(void)states;

	if (ARRAY_SIZE(cpu_off_seq) == 0U) {
		return;
	}

	core = plat_my_core_pos();
	mmio_write_32(CPU_SEQ_CTL(core),
		      CPU_SEQ_CTL_START_ADDR(QTI_CPU_OFF_SEQ_START) |
		      CPU_SEQ_CTL_PWR_STATE_IDX(QTI_LOCAL_STATE_DEEPOFF) |
		      CPU_SEQ_CTL_EN);
	mmio_setbits_32(CPU_PCHANNEL_FSM_CTL(core), PCHANNEL_MASK_WAKEUP);
}

/*
 * Idle power-down is not ported: no such state is advertised by
 * plat_qti_pm_idle_states(), so the suspend hooks are empty.
 */
void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	(void)states;
}

void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	(void)states;
}

/*
 * No native EL3 handler exists for QTI-specific interrupts. The CPU wake-up
 * SGI that woke a core is expected here; drop it.
 */
void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	(void)id;
	(void)handle;
}

/*
 * plat_qti_pm_idle_states - advertise the supported CPU idle states.
 *
 * Idle power-down is not implemented. Advertise only CPU-level standby
 * (retention), entered with a simple WFI and handled by
 * qti_cpu_standby(). This is "standard Arm WFI" CPU idle and is safe alongside
 * the OP-TEE SPD, which cannot tolerate the PSCI power-down abandon path that a
 * non-collapsing WFI would otherwise trigger.
 */
const unsigned int *plat_qti_pm_idle_states(void)
{
	static const unsigned int idle_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		0,
	};

	return idle_states;
}
