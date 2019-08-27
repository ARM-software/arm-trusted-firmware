/*
 * Copyright (c) 2015-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA_PRIVATE_H
#define TEGRA_PRIVATE_H

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <drivers/ti/uart/uart_16550.h>
#include <lib/psci/psci.h>
#include <lib/xlat_tables/xlat_tables_v2.h>

#include <tegra_gic.h>

/*******************************************************************************
 * Tegra DRAM memory base address
 ******************************************************************************/
#define TEGRA_DRAM_BASE		ULL(0x80000000)
#define TEGRA_DRAM_END		ULL(0x27FFFFFFF)

/*******************************************************************************
 * Implementation defined ACTLR_EL1 bit definitions
 ******************************************************************************/
#define ACTLR_EL1_PMSTATE_MASK		(ULL(0xF) << 0)

/*******************************************************************************
 * Implementation defined ACTLR_EL2 bit definitions
 ******************************************************************************/
#define ACTLR_EL2_PMSTATE_MASK		(ULL(0xF) << 0)

/*******************************************************************************
 * Struct for parameters received from BL2
 ******************************************************************************/
typedef struct plat_params_from_bl2 {
	/* TZ memory size */
	uint64_t tzdram_size;
	/* TZ memory base */
	uint64_t tzdram_base;
	/* UART port ID */
	int32_t uart_id;
	/* L2 ECC parity protection disable flag */
	int32_t l2_ecc_parity_prot_dis;
	/* SHMEM base address for storing the boot logs */
	uint64_t boot_profiler_shmem_base;
	/* System Suspend Entry Firmware size */
	uint64_t sc7entry_fw_size;
	/* System Suspend Entry Firmware base address */
	uint64_t sc7entry_fw_base;
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
void plat_early_platform_setup(void);
void plat_late_platform_setup(void);

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
int32_t tegra_system_suspended(void);
int32_t tegra_soc_cpu_standby(plat_local_state_t cpu_state);
int32_t tegra_soc_pwr_domain_suspend(const psci_power_state_t *target_state);
int32_t tegra_soc_pwr_domain_on(u_register_t mpidr);
int32_t tegra_soc_pwr_domain_off(const psci_power_state_t *target_state);
int32_t tegra_soc_pwr_domain_on_finish(const psci_power_state_t *target_state);
int32_t tegra_soc_pwr_domain_power_down_wfi(const psci_power_state_t *target_state);
int32_t tegra_soc_prepare_system_reset(void);
__dead2 void tegra_soc_prepare_system_off(void);
plat_local_state_t tegra_soc_get_target_pwr_state(uint32_t lvl,
					     const plat_local_state_t *states,
					     uint32_t ncpu);
void tegra_get_sys_suspend_power_state(psci_power_state_t *req_state);
void tegra_cpu_standby(plat_local_state_t cpu_state);
int32_t tegra_pwr_domain_on(u_register_t mpidr);
void tegra_pwr_domain_off(const psci_power_state_t *target_state);
void tegra_pwr_domain_suspend(const psci_power_state_t *target_state);
void __dead2 tegra_pwr_domain_power_down_wfi(const psci_power_state_t *target_state);
void tegra_pwr_domain_on_finish(const psci_power_state_t *target_state);
void tegra_pwr_domain_suspend_finish(const psci_power_state_t *target_state);
__dead2 void tegra_system_off(void);
__dead2 void tegra_system_reset(void);
int32_t tegra_validate_power_state(uint32_t power_state,
				   psci_power_state_t *req_state);
int32_t tegra_validate_ns_entrypoint(uintptr_t entrypoint);

/* Declarations for tegraXXX_pm.c */
int tegra_prepare_cpu_suspend(unsigned int id, unsigned int afflvl);
int tegra_prepare_cpu_on_finish(unsigned long mpidr);

/* Declarations for tegra_bl31_setup.c */
plat_params_from_bl2_t *bl31_get_plat_params(void);
int32_t bl31_check_ns_address(uint64_t base, uint64_t size_in_bytes);

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
