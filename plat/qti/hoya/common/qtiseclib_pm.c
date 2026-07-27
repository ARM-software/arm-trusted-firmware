/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include <lib/psci/psci.h>

#include <qti_plat.h>
#include <qtiseclib_interface.h>

/*
 * QTISECLIB backend for the QTI PSCI / interrupt-dispatch hooks (see
 * qti_plat.h). The QTISECLIB-based hoya platforms compile this file to forward
 * the qti_pm.c / qti_interrupt_svc.c hooks to the QTISECLIB blob (or its stub).
 */

int plat_qti_pwr_domain_on(u_register_t mpidr, int core_pos)
{
	(void)core_pos;
	return qtiseclib_psci_node_power_on(mpidr);
}

void plat_qti_pwr_domain_on_finish(int core_pos, const uint8_t *states)
{
	(void)core_pos;
	qtiseclib_psci_node_on_finish(states);
}

void plat_qti_pwr_domain_off(const uint8_t *states)
{
	qtiseclib_psci_node_power_off(states);
}

void plat_qti_pwr_domain_suspend(const uint8_t *states)
{
	qtiseclib_psci_node_suspend(states);
}

void plat_qti_pwr_domain_suspend_finish(const uint8_t *states)
{
	qtiseclib_psci_node_suspend_finish(states);
}

int plat_qti_pwr_psci_init(uintptr_t warmboot_entry)
{
	return qtiseclib_psci_init(warmboot_entry);
}

void plat_qti_invoke_unhandled_isr(uint32_t id, void *handle)
{
	qtiseclib_invoke_isr(id, handle);
}
