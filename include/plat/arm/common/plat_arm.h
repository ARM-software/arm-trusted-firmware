/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __PLAT_ARM_H__
#define __PLAT_ARM_H__

#include <arm_xlat_tables.h>
#include <bakery_lock.h>
#include <cassert.h>
#include <cpu_data.h>
#include <stdint.h>
#include <utils_def.h>

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct bl31_params;
struct meminfo;
struct image_info;

#define ARM_CASSERT_MMAP						\
	CASSERT((ARRAY_SIZE(plat_arm_mmap) + ARM_BL_REGIONS)		\
		<= MAX_MMAP_REGIONS,					\
		assert_max_mmap_regions);

/*
 * Utility functions common to ARM standard platforms
 */
void arm_setup_page_tables(uintptr_t total_base,
			size_t total_size,
			uintptr_t code_start,
			uintptr_t code_limit,
			uintptr_t rodata_start,
			uintptr_t rodata_limit
#if USE_COHERENT_MEM
			, uintptr_t coh_start,
			uintptr_t coh_limit
#endif
);

#if defined(IMAGE_BL31) || (defined(AARCH32) && defined(IMAGE_BL32))
/*
 * Use this macro to instantiate lock before it is used in below
 * arm_lock_xxx() macros
 */
#define ARM_INSTANTIATE_LOCK	DEFINE_BAKERY_LOCK(arm_lock)
#define ARM_LOCK_GET_INSTANCE	(&arm_lock)
/*
 * These are wrapper macros to the Coherent Memory Bakery Lock API.
 */
#define arm_lock_init()		bakery_lock_init(&arm_lock)
#define arm_lock_get()		bakery_lock_get(&arm_lock)
#define arm_lock_release()	bakery_lock_release(&arm_lock)

#else

/*
 * Empty macros for all other BL stages other than BL31 and BL32
 */
#define ARM_INSTANTIATE_LOCK	static int arm_lock __unused
#define ARM_LOCK_GET_INSTANCE	0
#define arm_lock_init()
#define arm_lock_get()
#define arm_lock_release()

#endif /* defined(IMAGE_BL31) || (defined(AARCH32) && defined(IMAGE_BL32)) */

#if ARM_RECOM_STATE_ID_ENC
/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define ARM_LOCAL_PSTATE_WIDTH		4
#define ARM_LOCAL_PSTATE_MASK		((1 << ARM_LOCAL_PSTATE_WIDTH) - 1)

/* Macros to construct the composite power state */

/* Make composite power state parameter till power level 0 */
#if PSCI_EXTENDED_STATE_ID

#define arm_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | ((type) << PSTATE_TYPE_SHIFT))
#else
#define arm_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type) \
		(((lvl0_state) << PSTATE_ID_SHIFT) | \
		((pwr_lvl) << PSTATE_PWR_LVL_SHIFT) | \
		((type) << PSTATE_TYPE_SHIFT))
#endif /* __PSCI_EXTENDED_STATE_ID__ */

/* Make composite power state parameter till power level 1 */
#define arm_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl1_state) << ARM_LOCAL_PSTATE_WIDTH) | \
		arm_make_pwrstate_lvl0(lvl0_state, pwr_lvl, type))

/* Make composite power state parameter till power level 2 */
#define arm_make_pwrstate_lvl2(lvl2_state, lvl1_state, lvl0_state, pwr_lvl, type) \
		(((lvl2_state) << (ARM_LOCAL_PSTATE_WIDTH * 2)) | \
		arm_make_pwrstate_lvl1(lvl1_state, lvl0_state, pwr_lvl, type))

#endif /* __ARM_RECOM_STATE_ID_ENC__ */

/* ARM State switch error codes */
#define STATE_SW_E_PARAM		(-2)
#define STATE_SW_E_DENIED		(-3)

/* IO storage utility functions */
void arm_io_setup(void);

/* Security utility functions */
void arm_tzc400_setup(void);
struct tzc_dmc500_driver_data;
void arm_tzc_dmc500_setup(struct tzc_dmc500_driver_data *plat_driver_data);

/* Systimer utility function */
void arm_configure_sys_timer(void);

/* PM utility functions */
int arm_validate_power_state(unsigned int power_state,
			    psci_power_state_t *req_state);
int arm_validate_psci_entrypoint(uintptr_t entrypoint);
int arm_validate_ns_entrypoint(uintptr_t entrypoint);
void arm_system_pwr_domain_save(void);
void arm_system_pwr_domain_resume(void);
void arm_program_trusted_mailbox(uintptr_t address);
int arm_psci_read_mem_protect(int *val);
int arm_nor_psci_write_mem_protect(int val);
void arm_nor_psci_do_mem_protect(void);
int arm_psci_mem_protect_chk(uintptr_t base, u_register_t length);

/* Topology utility function */
int arm_check_mpidr(u_register_t mpidr);

/* BL1 utility functions */
void arm_bl1_early_platform_setup(void);
void arm_bl1_platform_setup(void);
void arm_bl1_plat_arch_setup(void);

/* BL2 utility functions */
void arm_bl2_early_platform_setup(struct meminfo *mem_layout);
void arm_bl2_platform_setup(void);
void arm_bl2_plat_arch_setup(void);
uint32_t arm_get_spsr_for_bl32_entry(void);
uint32_t arm_get_spsr_for_bl33_entry(void);
int arm_bl2_handle_post_image_load(unsigned int image_id);

/* BL2 at EL3 functions */
void arm_bl2_el3_early_platform_setup(void);
void arm_bl2_el3_plat_arch_setup(void);

/* BL2U utility functions */
void arm_bl2u_early_platform_setup(struct meminfo *mem_layout,
				void *plat_info);
void arm_bl2u_platform_setup(void);
void arm_bl2u_plat_arch_setup(void);

/* BL31 utility functions */
#if LOAD_IMAGE_V2
void arm_bl31_early_platform_setup(void *from_bl2,
				void *plat_params_from_bl2);
#else
void arm_bl31_early_platform_setup(struct bl31_params *from_bl2,
				void *plat_params_from_bl2);
#endif /* LOAD_IMAGE_V2 */
void arm_bl31_platform_setup(void);
void arm_bl31_plat_runtime_setup(void);
void arm_bl31_plat_arch_setup(void);

/* TSP utility functions */
void arm_tsp_early_platform_setup(void);

/* SP_MIN utility functions */
void arm_sp_min_early_platform_setup(void *from_bl2,
		void *plat_params_from_bl2);
void arm_sp_min_plat_runtime_setup(void);

/* FIP TOC validity check */
int arm_io_is_toc_valid(void);

/*
 * Mandatory functions required in ARM standard platforms
 */
unsigned int plat_arm_get_cluster_core_count(u_register_t mpidr);
void plat_arm_gic_driver_init(void);
void plat_arm_gic_init(void);
void plat_arm_gic_cpuif_enable(void);
void plat_arm_gic_cpuif_disable(void);
void plat_arm_gic_redistif_on(void);
void plat_arm_gic_redistif_off(void);
void plat_arm_gic_pcpu_init(void);
void plat_arm_gic_save(void);
void plat_arm_gic_resume(void);
void plat_arm_security_setup(void);
void plat_arm_pwrc_setup(void);
void plat_arm_interconnect_init(void);
void plat_arm_interconnect_enter_coherency(void);
void plat_arm_interconnect_exit_coherency(void);

#if ARM_PLAT_MT
unsigned int plat_arm_get_cpu_pe_count(u_register_t mpidr);
#endif

#if LOAD_IMAGE_V2
/*
 * This function is called after loading SCP_BL2 image and it is used to perform
 * any platform-specific actions required to handle the SCP firmware.
 */
int plat_arm_bl2_handle_scp_bl2(struct image_info *scp_bl2_image_info);
#endif

/*
 * Optional functions required in ARM standard platforms
 */
void plat_arm_io_setup(void);
int plat_arm_get_alt_image_source(
	unsigned int image_id,
	uintptr_t *dev_handle,
	uintptr_t *image_spec);
unsigned int plat_arm_calc_core_pos(u_register_t mpidr);
const mmap_region_t *plat_arm_get_mmap(void);

/* Allow platform to override psci_pm_ops during runtime */
const plat_psci_ops_t *plat_arm_psci_override_pm_ops(plat_psci_ops_t *ops);

/* Execution state switch in ARM platforms */
int arm_execution_state_switch(unsigned int smc_fid,
		uint32_t pc_hi,
		uint32_t pc_lo,
		uint32_t cookie_hi,
		uint32_t cookie_lo,
		void *handle);

#endif /* __PLAT_ARM_H__ */
