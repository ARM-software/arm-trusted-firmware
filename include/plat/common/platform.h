/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <psci.h>
#include <stdint.h>
#include <types.h>


/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct auth_img_desc_s;
struct meminfo;
struct image_info;
struct entry_point_info;
struct bl31_params;
struct image_desc;
struct bl_load_info;
struct bl_params;
struct mmap_region;
struct secure_partition_boot_info;

/*******************************************************************************
 * plat_get_rotpk_info() flags
 ******************************************************************************/
#define ROTPK_IS_HASH			(1 << 0)
/* Flag used to skip verification of the certificate ROTPK while the platform
   ROTPK is not deployed */
#define ROTPK_NOT_DEPLOYED		(1 << 1)

/*******************************************************************************
 * Function declarations
 ******************************************************************************/
/*******************************************************************************
 * Mandatory common functions
 ******************************************************************************/
unsigned long long plat_get_syscnt_freq(void) __deprecated;
unsigned int plat_get_syscnt_freq2(void);

int plat_get_image_source(unsigned int image_id,
			uintptr_t *dev_handle,
			uintptr_t *image_spec);
uintptr_t plat_get_ns_image_entrypoint(void);
unsigned int plat_my_core_pos(void);
int plat_core_pos_by_mpidr(u_register_t mpidr);

#if STACK_PROTECTOR_ENABLED
/*
 * Return a new value to be used for the stack protection's canary.
 *
 * Ideally, this value is a random number that is impossible to predict by an
 * attacker.
 */
u_register_t plat_get_stack_protector_canary(void);
#endif /* STACK_PROTECTOR_ENABLED */

/*******************************************************************************
 * Mandatory interrupt management functions
 ******************************************************************************/
uint32_t plat_ic_get_pending_interrupt_id(void);
uint32_t plat_ic_get_pending_interrupt_type(void);
uint32_t plat_ic_acknowledge_interrupt(void);
uint32_t plat_ic_get_interrupt_type(uint32_t id);
void plat_ic_end_of_interrupt(uint32_t id);
uint32_t plat_interrupt_type_to_line(uint32_t type,
				     uint32_t security_state);

/*******************************************************************************
 * Optional interrupt management functions, depending on chosen EL3 components.
 ******************************************************************************/
unsigned int plat_ic_get_running_priority(void);
int plat_ic_is_spi(unsigned int id);
int plat_ic_is_ppi(unsigned int id);
int plat_ic_is_sgi(unsigned int id);
unsigned int plat_ic_get_interrupt_active(unsigned int id);
void plat_ic_disable_interrupt(unsigned int id);
void plat_ic_enable_interrupt(unsigned int id);
int plat_ic_has_interrupt_type(unsigned int type);
void plat_ic_set_interrupt_type(unsigned int id, unsigned int type);
void plat_ic_set_interrupt_priority(unsigned int id, unsigned int priority);
void plat_ic_raise_el3_sgi(int sgi_num, u_register_t target);
void plat_ic_set_spi_routing(unsigned int id, unsigned int routing_mode,
		u_register_t mpidr);
void plat_ic_set_interrupt_pending(unsigned int id);
void plat_ic_clear_interrupt_pending(unsigned int id);
unsigned int plat_ic_set_priority_mask(unsigned int mask);
unsigned int plat_ic_get_interrupt_id(unsigned int raw);

/*******************************************************************************
 * Optional common functions (may be overridden)
 ******************************************************************************/
uintptr_t plat_get_my_stack(void);
void plat_report_exception(unsigned int exception_type);
int plat_crash_console_init(void);
int plat_crash_console_putc(int c);
int plat_crash_console_flush(void);
void plat_error_handler(int err) __dead2;
void plat_panic_handler(void) __dead2;
const char *plat_log_get_prefix(unsigned int log_level);
void bl2_plat_preload_setup(void);
int plat_try_next_boot_source(void);

/*******************************************************************************
 * Mandatory BL1 functions
 ******************************************************************************/
void bl1_early_platform_setup(void);
void bl1_plat_arch_setup(void);
void bl1_platform_setup(void);
struct meminfo *bl1_plat_sec_mem_layout(void);

/*******************************************************************************
 * Optional EL3 component functions in BL31
 ******************************************************************************/

/* SDEI platform functions */
#if SDEI_SUPPORT
int plat_sdei_validate_entry_point(uintptr_t ep, unsigned int client_mode);
void plat_sdei_handle_masked_trigger(uint64_t mpidr, unsigned int intr);
#endif

void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags);

/*
 * The following function is mandatory when the
 * firmware update feature is used.
 */
int bl1_plat_mem_check(uintptr_t mem_base, unsigned int mem_size,
		unsigned int flags);

/*******************************************************************************
 * Optional BL1 functions (may be overridden)
 ******************************************************************************/
#if !ERROR_DEPRECATED
void bl1_init_bl2_mem_layout(const struct meminfo *bl1_mem_layout,
			     struct meminfo *bl2_mem_layout);
#endif
/*
 * The following functions are used for image loading process in BL1.
 */
void bl1_plat_set_ep_info(unsigned int image_id,
		struct entry_point_info *ep_info);
/*
 * The following functions are mandatory when firmware update
 * feature is used and optional otherwise.
 */
unsigned int bl1_plat_get_next_image_id(void);
struct image_desc *bl1_plat_get_image_desc(unsigned int image_id);

/*
 * The following functions are used by firmware update
 * feature and may optionally be overridden.
 */
__dead2 void bl1_plat_fwu_done(void *client_cookie, void *reserved);

/*
 * This BL1 function can be used by the platforms to update/use image
 * information for a given `image_id`.
 */
int bl1_plat_handle_pre_image_load(unsigned int image_id);
int bl1_plat_handle_post_image_load(unsigned int image_id);

/*******************************************************************************
 * Mandatory BL2 functions
 ******************************************************************************/
void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1, u_register_t arg2, u_register_t arg3);
#if !ERROR_DEPRECATED
void bl2_early_platform_setup(struct meminfo *mem_layout);
#endif
void bl2_plat_arch_setup(void);
void bl2_platform_setup(void);
struct meminfo *bl2_plat_sec_mem_layout(void);

#if LOAD_IMAGE_V2
/*
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 */
int bl2_plat_handle_pre_image_load(unsigned int image_id);
int bl2_plat_handle_post_image_load(unsigned int image_id);

#else /* LOAD_IMAGE_V2 */

/*
 * This function returns a pointer to the shared memory that the platform has
 * kept aside to pass trusted firmware related information that BL31
 * could need
 */
struct bl31_params *bl2_plat_get_bl31_params(void);

/*
 * This function returns a pointer to the shared memory that the platform
 * has kept to point to entry point information of BL31 to BL2
 */
struct entry_point_info *bl2_plat_get_bl31_ep_info(void);

/*
 * This function flushes to main memory all the params that are
 * passed to BL31
 */
void bl2_plat_flush_bl31_params(void);

/*
 * The next 2 functions allow the platform to change the entrypoint information
 * for the mandatory 3rd level BL images, BL31 and BL33. This is done after
 * BL2 has loaded those images into memory but before BL31 is executed.
 */
void bl2_plat_set_bl31_ep_info(struct image_info *image,
			       struct entry_point_info *ep);

void bl2_plat_set_bl33_ep_info(struct image_info *image,
			       struct entry_point_info *ep);

/* Gets the memory layout for BL33 */
void bl2_plat_get_bl33_meminfo(struct meminfo *mem_info);

/*******************************************************************************
 * Conditionally mandatory BL2 functions: must be implemented if SCP_BL2 image
 * is supported
 ******************************************************************************/
/* Gets the memory layout for SCP_BL2 */
void bl2_plat_get_scp_bl2_meminfo(struct meminfo *mem_info);

/*
 * This function is called after loading SCP_BL2 image and it is used to perform
 * any platform-specific actions required to handle the SCP firmware.
 */
int bl2_plat_handle_scp_bl2(struct image_info *scp_bl2_image_info);

/*******************************************************************************
 * Conditionally mandatory BL2 functions: must be implemented if BL32 image
 * is supported
 ******************************************************************************/
void bl2_plat_set_bl32_ep_info(struct image_info *image,
			       struct entry_point_info *ep);

/* Gets the memory layout for BL32 */
void bl2_plat_get_bl32_meminfo(struct meminfo *mem_info);

#endif /* LOAD_IMAGE_V2 */

/*******************************************************************************
 * Optional BL2 functions (may be overridden)
 ******************************************************************************/


/*******************************************************************************
 * Mandatory BL2 at EL3 functions: Must be implemented if BL2_AT_EL3 image is
 * supported
 ******************************************************************************/
void bl2_el3_early_platform_setup(u_register_t arg0, u_register_t arg1,
				  u_register_t arg2, u_register_t arg3);
void bl2_el3_plat_arch_setup(void);


/*******************************************************************************
 * Optional BL2 at EL3 functions (may be overridden)
 ******************************************************************************/
void bl2_el3_plat_prepare_exit(void);

/*******************************************************************************
 * Mandatory BL2U functions.
 ******************************************************************************/
void bl2u_early_platform_setup(struct meminfo *mem_layout,
		void *plat_info);
void bl2u_plat_arch_setup(void);
void bl2u_platform_setup(void);

/*******************************************************************************
 * Conditionally mandatory BL2U functions for CSS platforms.
 ******************************************************************************/
/*
 * This function is used to perform any platform-specific actions required to
 * handle the BL2U_SCP firmware.
 */
int bl2u_plat_handle_scp_bl2u(void);

/*******************************************************************************
 * Mandatory BL31 functions
 ******************************************************************************/
#if !ERROR_DEPRECATED
#if LOAD_IMAGE_V2
void bl31_early_platform_setup(void *from_bl2,
				void *plat_params_from_bl2);
#else
void bl31_early_platform_setup(struct bl31_params *from_bl2,
				void *plat_params_from_bl2);
#endif
#endif /* ERROR_DEPRECATED */
void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
		u_register_t arg2, u_register_t arg3);
void bl31_plat_arch_setup(void);
void bl31_platform_setup(void);
void bl31_plat_runtime_setup(void);
struct entry_point_info *bl31_plat_get_next_image_ep_info(uint32_t type);

/*******************************************************************************
 * Mandatory PSCI functions (BL31)
 ******************************************************************************/
int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops);
const unsigned char *plat_get_power_domain_tree_desc(void);

/*******************************************************************************
 * Optional PSCI functions (BL31).
 ******************************************************************************/
void plat_psci_stat_accounting_start(const psci_power_state_t *state_info);
void plat_psci_stat_accounting_stop(const psci_power_state_t *state_info);
u_register_t plat_psci_stat_get_residency(unsigned int lvl,
			const psci_power_state_t *state_info,
			int last_cpu_idx);
plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
			const plat_local_state_t *states,
			unsigned int ncpu);

/*******************************************************************************
 * Optional BL31 functions (may be overridden)
 ******************************************************************************/
void bl31_plat_enable_mmu(uint32_t flags);

/*******************************************************************************
 * Optional BL32 functions (may be overridden)
 ******************************************************************************/
void bl32_plat_enable_mmu(uint32_t flags);

/*******************************************************************************
 * Trusted Board Boot functions
 ******************************************************************************/
int plat_get_rotpk_info(void *cookie, void **key_ptr, unsigned int *key_len,
			unsigned int *flags);
int plat_get_nv_ctr(void *cookie, unsigned int *nv_ctr);
int plat_set_nv_ctr(void *cookie, unsigned int nv_ctr);
int plat_set_nv_ctr2(void *cookie, const struct auth_img_desc_s *img_desc,
		unsigned int nv_ctr);

/*******************************************************************************
 * Secure Partitions functions
 ******************************************************************************/
const struct mmap_region *plat_get_secure_partition_mmap(void *cookie);
const struct secure_partition_boot_info *plat_get_secure_partition_boot_info(
		void *cookie);

#if LOAD_IMAGE_V2
/*******************************************************************************
 * Mandatory BL image load functions(may be overridden).
 ******************************************************************************/
/*
 * This function returns pointer to the list of images that the
 * platform has populated to load.
 */
struct bl_load_info *plat_get_bl_image_load_info(void);

/*
 * This function returns a pointer to the shared memory that the
 * platform has kept aside to pass trusted firmware related
 * information that next BL image could need.
 */
struct bl_params *plat_get_next_bl_params(void);

/*
 * This function flushes to main memory all the params that are
 * passed to next image.
 */
void plat_flush_next_bl_params(void);

#endif /* LOAD_IMAGE_V2 */

#if ENABLE_PLAT_COMPAT
/*
 * The below declarations are to enable compatibility for the platform ports
 * using the old platform interface.
 */

/*******************************************************************************
 * Optional common functions (may be overridden)
 ******************************************************************************/
unsigned int platform_get_core_pos(unsigned long mpidr);

/*******************************************************************************
 * Mandatory PSCI Compatibility functions (BL31)
 ******************************************************************************/
int platform_setup_pm(const plat_pm_ops_t **);

unsigned int plat_get_aff_count(unsigned int, unsigned long);
unsigned int plat_get_aff_state(unsigned int, unsigned long);

#else /* __ENABLE_PLAT_COMPAT__ */
/*
 * The below function enable Trusted Firmware components like SPDs which
 * haven't migrated to the new platform API to compile on platforms which
 * have the compatibility layer disabled.
 */
unsigned int platform_core_pos_helper(unsigned long mpidr);
unsigned int platform_get_core_pos(unsigned long mpidr) __deprecated;

#endif /* __ENABLE_PLAT_COMPAT__ */

#endif /* __PLATFORM_H__ */

