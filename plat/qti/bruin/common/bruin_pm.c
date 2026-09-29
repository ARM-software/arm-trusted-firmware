/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <drivers/delay_timer.h>
#include <lib/mmio.h>
#include <lib/psci/psci.h>

#include <platform_def.h>
#include <qti_plat.h>

#define APSS_CPU_IPM_REG_BASE		0x0F800000U
#define APSS_CPU_IPM_REG_OFFSET		0x10000U
#define APSS_CPU_IPM_REG(core)		(APSS_CPU_IPM_REG_BASE + \
					 ((core) * APSS_CPU_IPM_REG_OFFSET))

#define CPU_HEAD_SWITCH_CTL(core)	(APSS_CPU_IPM_REG(core) + 0x08U)
#define CPU_SEQ_FORCE_PWR_CTL_EN(core)	(APSS_CPU_IPM_REG(core) + 0x1cU)
#define CPU_SEQ_FORCE_PWR_CTL_VAL(core)	(APSS_CPU_IPM_REG(core) + 0x20U)
#define CPU_PCHANNEL_FSM_CTL(core)	(APSS_CPU_IPM_REG(core) + 0x44U)

/*
 * plat_qti_pwr_psci_init - PSCI backend init hook.
 *
 * The secondary-core reset vector (RVBAR) is programmed by
 * qti_sec_core_remap(), invoked from plat_setup_psci_ops() ahead of this hook,
 * so there is nothing to do here.
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
 */
void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	(void)states;

	plat_qti_gic_pcpu_init();
}

/*
 * The native port only supports CPU-level standby (WFI retention). The
 * firmware power-collapse / LPM node handling (PCU/RSC/PDC/RPM) is not ported.
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

void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	(void)id;
	(void)handle;
}

const unsigned int *plat_qti_pm_idle_states(void)
{
	static const unsigned int idle_states[] = {
		qti_make_pwrstate_lvl0(QTI_LOCAL_STATE_STB,
				       PSTATE_TYPE_STANDBY),
		0,
	};

	return idle_states;
}
