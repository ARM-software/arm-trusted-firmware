/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TEGRA_PRIVATE_H__
#define __TEGRA_PRIVATE_H__

#include <arch.h>
#include <platform_def.h>
#include <psci.h>
#include <xlat_tables.h>

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
} plat_params_from_bl2_t;

/*******************************************************************************
 * Per-CPU struct describing FIQ state to be stored
 ******************************************************************************/
typedef struct pcpu_fiq_state {
	uint64_t elr_el3;
	uint64_t spsr_el3;
} pcpu_fiq_state_t;

/*******************************************************************************
 * Struct describing per-FIQ configuration settings
 ******************************************************************************/
typedef struct irq_sec_cfg {
	/* IRQ number */
	unsigned int irq;
	/* Target CPUs servicing this interrupt */
	unsigned int target_cpus;
	/* type = INTR_TYPE_S_EL1 or INTR_TYPE_EL3 */
	uint32_t type;
} irq_sec_cfg_t;

/* Declarations for plat_psci_handlers.c */
int32_t tegra_soc_validate_power_state(unsigned int power_state,
		psci_power_state_t *req_state);

/* Declarations for plat_setup.c */
const mmap_region_t *plat_get_mmio_map(void);
uint32_t plat_get_console_from_id(int id);
void plat_gic_setup(void);
bl31_params_t *plat_get_bl31_params(void);
plat_params_from_bl2_t *plat_get_bl31_plat_params(void);

/* Declarations for plat_secondary.c */
void plat_secondary_setup(void);
int plat_lock_cpu_vectors(void);

/* Declarations for tegra_fiq_glue.c */
void tegra_fiq_handler_setup(void);
int tegra_fiq_get_intr_context(void);
void tegra_fiq_set_ns_entrypoint(uint64_t entrypoint);

/* Declarations for tegra_gic.c */
void tegra_gic_cpuif_deactivate(void);
void tegra_gic_setup(const irq_sec_cfg_t *irq_sec_ptr, uint32_t num_irqs);

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

#endif /* __TEGRA_PRIVATE_H__ */
