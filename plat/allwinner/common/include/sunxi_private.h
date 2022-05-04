/*
 * Copyright (c) 2017-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SUNXI_PRIVATE_H
#define SUNXI_PRIVATE_H

#include <common/fdt_fixup.h>

#include <lib/psci/psci.h>

extern const struct psci_cpu_idle_state sunxi_idle_states[];

void sunxi_configure_mmu_el3(int flags);

void sunxi_cpu_on(u_register_t mpidr);
void sunxi_cpu_power_off_others(void);
void sunxi_cpu_power_off_self(void);
void sunxi_power_down(void);

#if SUNXI_PSCI_USE_NATIVE
void sunxi_set_native_psci_ops(const plat_psci_ops_t **psci_ops);
#else
static inline void sunxi_set_native_psci_ops(const plat_psci_ops_t **psci_ops)
{
}
#endif
#if SUNXI_PSCI_USE_SCPI
bool sunxi_psci_is_scpi(void);
int sunxi_set_scpi_psci_ops(const plat_psci_ops_t **psci_ops);
#else
static inline bool sunxi_psci_is_scpi(void)
{
	return false;
}
static inline int sunxi_set_scpi_psci_ops(const plat_psci_ops_t **psci_ops)
{
	return -1;
}
#endif
int sunxi_validate_ns_entrypoint(uintptr_t ns_entrypoint);

int sunxi_pmic_setup(uint16_t socid, const void *fdt);
void sunxi_security_setup(void);

uint16_t sunxi_read_soc_id(void);
void sunxi_set_gpio_out(char port, int pin, bool level_high);
int sunxi_init_platform_r_twi(uint16_t socid, bool use_rsb);
void sunxi_execute_arisc_code(uint32_t *code, size_t size, uint16_t param);

#if SUNXI_AMEND_DTB
void sunxi_prepare_dtb(void *fdt);
#else
static inline void sunxi_prepare_dtb(void *fdt)
{
}
#endif

#endif /* SUNXI_PRIVATE_H */
