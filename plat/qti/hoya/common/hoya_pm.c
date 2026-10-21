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
#include <plat/common/platform.h>

#include <platform_def.h>
#include <qti_plat.h>

/*
 * Native (hoya/lemans) backend for the QTI PSCI power-domain hooks declared in
 * qti_plat.h. The secondary-core cold-boot bringup and reset-vector setup are
 * performed natively via the raw APSS IPM register sequence below. Only
 * CPU-level standby (WFI retention) is supported; the firmware
 * low-power-mode / suspend node handling (PCU/RSC/PDC) is not ported, so the
 * power-down and suspend hooks are empty stubs.
 */

/* Per-core APSS IPM register block. */
#define APSS_CPU_IPM_REG_BASE			0x18000000U
#define APSS_CPU_IPM_REG_OFFSET			0x10000U
#define APSS_CPU_IPM_REG(core)			(APSS_CPU_IPM_REG_BASE + \
						 ((core) * APSS_CPU_IPM_REG_OFFSET))

#define CPU_HEAD_SWITCH_CTL(core)		(APSS_CPU_IPM_REG(core) + 0x08U)
#define CPU_SEQ_FORCE_PWR_CTL_EN(core)		(APSS_CPU_IPM_REG(core) + 0x1cU)
#define CPU_SEQ_FORCE_PWR_CTL_VAL(core)		(APSS_CPU_IPM_REG(core) + 0x20U)
#define CPU_PCHANNEL_FSM_CTL(core)		(APSS_CPU_IPM_REG(core) + 0x44U)

/*
 * APSS cluster (L3/DSU) IPM alias register block (APSS_ALIAS_1). Used for the
 * gold-cluster L3 turn-on and memory-repair sequences.
 */
#define APSS_CL_IPM_REG_BASE			0x18090000U

#define L3_SEQ_FORCE_PWR_CTL_EN			(APSS_CL_IPM_REG_BASE + 0x1cU)
#define L3_SEQ_FORCE_PWR_CTL_VAL		(APSS_CL_IPM_REG_BASE + 0x20U)
#define L3_SEQ_STS1				(APSS_CL_IPM_REG_BASE + 0x38U)
#define CL_PCHANNEL_FSM_CTL			(APSS_CL_IPM_REG_BASE + 0x44U)
#define GOLD_PLL_SEQ_FORCE_PWR_CTL_EN		(APSS_CL_IPM_REG_BASE + 0xecU)
#define GOLD_PLL_SEQ_FORCE_PWR_CTL_VAL		(APSS_CL_IPM_REG_BASE + 0xf0U)
#define GOLD_PLL_SEQ_STS1			(APSS_CL_IPM_REG_BASE + 0xfcU)

#define L3_SEQ_STS1_MEM_REPAIR_DONE		0x40000U
#define L3_SEQ_FORCE_PWR_CTL_MEM_REPAIR		0x2000000U
#define GOLD_PLL_SEQ_STS1_MEM_REPAIR_DONE	0x40U
#define GOLD_PLL_SEQ_FORCE_PWR_CTL_MEM_REPAIR	0x4000U

/* Upper bound for the memory-repair "done" handshake. */
#define MEM_REPAIR_TIMEOUT_US			10000U

/*
 * Gold-cluster (APC1) SAW4 AVS rail. The lemans SAW4 instance for the gold
 * rail is at 0x18101000, with the AVS register region at +0x800 and the status
 * region at +0xc00 (see qtiseclib HAL_avs_SecondaryRailInit / saw_v4.c).
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
#define GOLD_SAW4_BOOT_VOLTAGE			828U
#define SAW4_VCTL_SET_VOLTAGE			(0x100000U | GOLD_SAW4_BOOT_VOLTAGE)

#define SAW4_PMIC_WRITE_RETRY			200U

/* The first gold-cluster (DSU1) core in the lemans CPU topology. */
#define QTI_FIRST_GOLD_CORE			4

/*
 * Repair the L3 memories of the gold cluster. Performed once, before the first
 * gold core is powered on.
 */
static void l3_memory_repair(void)
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
			panic();
		}
	}

	val = mmio_read_32(L3_SEQ_FORCE_PWR_CTL_VAL);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_VAL,
		      val & ~L3_SEQ_FORCE_PWR_CTL_MEM_REPAIR);

	val = mmio_read_32(L3_SEQ_FORCE_PWR_CTL_EN);
	mmio_write_32(L3_SEQ_FORCE_PWR_CTL_EN,
		      val & ~L3_SEQ_FORCE_PWR_CTL_MEM_REPAIR);
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
 * Repair the gold-cluster PLL memories. Performed once, before the first gold
 * core is powered on.
 */
static void cpu_memory_repair(void)
{
	uint32_t val;
	uint64_t timeout;

	val = mmio_read_32(GOLD_PLL_SEQ_FORCE_PWR_CTL_EN);
	mmio_write_32(GOLD_PLL_SEQ_FORCE_PWR_CTL_EN,
		      val | GOLD_PLL_SEQ_FORCE_PWR_CTL_MEM_REPAIR);

	val = mmio_read_32(GOLD_PLL_SEQ_FORCE_PWR_CTL_VAL);
	mmio_write_32(GOLD_PLL_SEQ_FORCE_PWR_CTL_VAL,
		      val | GOLD_PLL_SEQ_FORCE_PWR_CTL_MEM_REPAIR);

	timeout = timeout_init_us(MEM_REPAIR_TIMEOUT_US);
	while ((mmio_read_32(GOLD_PLL_SEQ_STS1) &
		GOLD_PLL_SEQ_STS1_MEM_REPAIR_DONE) == 0U) {
		if (timeout_elapsed(timeout)) {
			ERROR("Gold PLL memory repair timed out\n");
			panic();
		}
	}

	val = mmio_read_32(GOLD_PLL_SEQ_FORCE_PWR_CTL_VAL);
	mmio_write_32(GOLD_PLL_SEQ_FORCE_PWR_CTL_VAL,
		      val & ~GOLD_PLL_SEQ_FORCE_PWR_CTL_MEM_REPAIR);

	val = mmio_read_32(GOLD_PLL_SEQ_FORCE_PWR_CTL_EN);
	mmio_write_32(GOLD_PLL_SEQ_FORCE_PWR_CTL_EN,
		      val & ~GOLD_PLL_SEQ_FORCE_PWR_CTL_MEM_REPAIR);
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
 * voltage. This mirrors qtiseclib's HAL_avs_SecondaryRailInit(); without it the
 * gold cluster has no supply and its cores never come out of reset.
 */
static void set_gold_cluster_voltage(void)
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
		WARN("gold SAW4 PMIC enable timed out\n");
	}
	udelay(90);

	/* Restore the rail to its boot voltage. */
	mmio_write_32(GOLD_SAW4_VCTL, SAW4_VCTL_SET_VOLTAGE);
	if (!saw4_pmic_wait(GOLD_SAW4_BOOT_VOLTAGE, true)) {
		WARN("gold SAW4 PMIC voltage set timed out\n");
	}
}

/* One-time gold-cluster (DSU1) bringup, run before its first core powers on. */
static void gold_cluster_cold_boot(void)
{
	static spinlock_t gold_cluster_lock;
	static bool gold_cluster_booted;

	spin_lock(&gold_cluster_lock);
	if (gold_cluster_booted) {
		spin_unlock(&gold_cluster_lock);
		return;
	}

	set_gold_cluster_voltage();
	l3_memory_repair();
	l3_cold_boot();
	cpu_memory_repair();

	gold_cluster_booted = true;
	spin_unlock(&gold_cluster_lock);
}

/*
 * plat_qti_pwr_psci_init - PSCI backend init hook.
 *
 * The secondary-core reset vector (RVBAR) is programmed by
 * qti_sec_core_remap(), invoked from plat_setup_psci_ops() ahead of this hook,
 * so there is nothing left to do here.
 */
int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	(void)warmboot_entry;

	return PSCI_E_SUCCESS;
}

/*
 * plat_qti_pwr_domain_on - power on a secondary core using the raw APSS IPM
 * reset sequence.
 */
int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	(void)mpidr;

	/* Bring up the gold cluster before powering on its first core. */
	if (core_pos >= QTI_FIRST_GOLD_CORE) {
		gold_cluster_cold_boot();
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
 * Initialise the GIC redistributor for the core and request CPUCP to enable
 * the core's clock domain on cold boot.
 */
void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;

	plat_qti_gic_pcpu_init();
	cpucp_clkdom_init();
}

/*
 * The native port only supports CPU-level standby (WFI retention); the firmware
 * power-collapse / low-power-mode node handling (PCU/RSC/PDC) is not ported.
 * These power-down and suspend node hooks are therefore empty stubs, and no
 * corresponding idle states are advertised by plat_qti_pm_idle_states().
 */
void plat_qti_pwr_domain_off(const uint8_t *states)
{
	(void)states;
}

void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	(void)states;
}

void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	(void)states;
}

/*
 * No native EL3 handler exists for QTI-specific interrupts; an unhandled
 * interrupt reaching EL3 is not expected, so drop it.
 */
void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	(void)id;
	(void)handle;
}

/*
 * plat_qti_pm_idle_states - advertise the supported CPU idle states.
 *
 * The native port does not implement the firmware power-collapse (LPM)
 * sequence, so a CPU cannot be physically powered down by EL3 alone. Advertise
 * only CPU-level standby (retention), entered with a simple WFI and handled by
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
