/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA_PRIVATE_H
#define TEGRA_PRIVATE_H

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <lib/psci/psci.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <tegra_gic.h>

/*******************************************************************************
 * Tegra DRAM memory base address
 ******************************************************************************/
#define TEGRA_DRAM_BASE		ULL(0x80000000)
#define TEGRA_DRAM_END		ULL(0x27FFFFFFF)

/*******************************************************************************
 * Struct for parameters received from BL2
 ******************************************************************************/
typedef struct plat_params_from_bl2 {
	/* TZ memory size */
	uint64_t tzdram_size;
	/* TZ memory base */
	uint64_t tzdram_base;
	/* UART port ID */
	int uart_id;
	/* L2 ECC parity protection disable flag */
	int l2_ecc_parity_prot_dis;
} plat_params_from_bl2_t;

/*******************************************************************************
 * Helper function to access l2ctlr_el1 register on Cortex-A57 CPUs
 ******************************************************************************/
DEFINE_RENAME_SYSREG_RW_FUNCS(l2ctlr_el1, CORTEX_A57_L2CTLR_EL1)

/*******************************************************************************
 * Struct describing parameters passed to bl31
 ******************************************************************************/
struct tegra_bl31_params {
       param_header_t h;
       image_info_t *bl31_image_info;
       entry_point_info_t *bl32_ep_info;
       image_info_t *bl32_image_info;
       entry_point_info_t *bl33_ep_info;
       image_info_t *bl33_image_info;
};

/* Declarations for plat_psci_handlers.c */
int32_t tegra_soc_validate_power_state(uint32_t power_state,
		psci_power_state_t *req_state);

/* Declarations for plat_setup.c */
const mmap_region_t *plat_get_mmio_map(void);
uint32_t plat_get_console_from_id(int32_t id);
void plat_gic_setup(void);
struct tegra_bl31_params *plat_get_bl31_params(void);
plat_params_from_bl2_t *plat_get_bl31_plat_params(void);

/* Declarations for plat_secondary.c */
void plat_secondary_setup(void);
int32_t plat_lock_cpu_vectors(void);

/* Declarations for tegra_fiq_glue.c */
void tegra_fiq_handler_setup(void);
int tegra_fiq_get_intr_context(void);
void tegra_fiq_set_ns_entrypoint(uint64_t entrypoint);

/* Declarations for tegra_security.c */
void tegra_security_setup(void);
void tegra_security_setup_videomem(uintptr_t base, uint64_t size);

/* Declarations for tegra_pm.c */
extern uint8_t tegra_fake_system_suspend;

void tegra_pm_system_suspend_entry(void);
void tegra_pm_system_suspend_exit(void);
int tegra_system_suspended(void);

/* Declarations for tegraXXX_pm.c */
int tegra_prepare_cpu_suspend(unsigned int id, unsigned int afflvl);
int tegra_prepare_cpu_on_finish(unsigned long mpidr);

/* Declarations for tegra_bl31_setup.c */
plat_params_from_bl2_t *bl31_get_plat_params(void);
int bl31_check_ns_address(uint64_t base, uint64_t size_in_bytes);
void plat_early_platform_setup(void);

/* Declarations for tegra_delay_timer.c */
void tegra_delay_timer_init(void);

void tegra_secure_entrypoint(void);
void tegra186_cpu_reset_handler(void);

/* Declarations for tegra_sip_calls.c */
uintptr_t tegra_sip_handler(uint32_t smc_fid,
			    u_register_t x1,
			    u_register_t x2,
			    u_register_t x3,
			    u_register_t x4,
			    void *cookie,
			    void *handle,
			    u_register_t flags);
int plat_sip_handler(uint32_t smc_fid,
		     uint64_t x1,
		     uint64_t x2,
		     uint64_t x3,
		     uint64_t x4,
		     const void *cookie,
		     void *handle,
		     uint64_t flags);

#endif /* TEGRA_PRIVATE_H */
