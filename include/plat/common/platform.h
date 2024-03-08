/*
 * Copyright (c) 2013-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>

#include <lib/psci/psci.h>
#if defined(SPD_spmd)
#include <services/spm_core_manifest.h>
#endif
#if ENABLE_RME
#include <services/rmm_core_manifest.h>
#endif
#include <drivers/fwu/fwu_metadata.h>
#if TRNG_SUPPORT
#include "plat_trng.h"
#endif /* TRNG_SUPPORT */
#if DRTM_SUPPORT
#include "plat_drtm.h"
#endif /* DRTM_SUPPORT */

/*******************************************************************************
 * Forward declarations
 ******************************************************************************/
struct auth_img_desc_s;
struct meminfo;
struct image_info;
struct entry_point_info;
struct image_desc;
struct bl_load_info;
struct bl_params;
struct mmap_region;
struct spm_mm_boot_info;
struct sp_res_desc;
struct rmm_manifest;
enum fw_enc_status_t;

/*******************************************************************************
 * plat_get_rotpk_info() flags
 ******************************************************************************/
#define ROTPK_IS_HASH			(1 << 0)

/* Flag used to skip verification of the certificate ROTPK while the platform
   ROTPK is not deployed */
#define ROTPK_NOT_DEPLOYED		(1 << 1)

static inline bool is_rotpk_flags_valid(unsigned int flags)
{
	unsigned int valid_flags = ROTPK_IS_HASH;
	return (flags == ROTPK_NOT_DEPLOYED) || ((flags & ~valid_flags) == 0);
}

/*******************************************************************************
 * plat_get_enc_key_info() flags
 ******************************************************************************/
/*
 * Flag used to notify caller that information provided in key buffer is an
 * identifier rather than an actual key.
 */
#define ENC_KEY_IS_IDENTIFIER		(1 << 0)

/*******************************************************************************
 * Function declarations
 ******************************************************************************/
/*******************************************************************************
 * Mandatory common functions
 ******************************************************************************/
unsigned int plat_get_syscnt_freq2(void);

int plat_get_image_source(unsigned int image_id,
			uintptr_t *dev_handle,
			uintptr_t *image_spec);
uintptr_t plat_get_ns_image_entrypoint(void);
unsigned int plat_my_core_pos(void);
int plat_core_pos_by_mpidr(u_register_t mpidr);
int plat_get_mbedtls_heap(void **heap_addr, size_t *heap_size);

/*******************************************************************************
 * Simple routine to determine whether a mpidr is valid or not.
 ******************************************************************************/
static inline bool is_valid_mpidr(u_register_t mpidr)
{
	int pos = plat_core_pos_by_mpidr(mpidr);

	if ((pos < 0) || ((unsigned int)pos >= PLATFORM_CORE_COUNT)) {
		return false;
	}

	return true;
}

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
bool plat_ic_has_interrupt_type(unsigned int type);
void plat_ic_set_interrupt_type(unsigned int id, unsigned int type);
void plat_ic_set_interrupt_priority(unsigned int id, unsigned int priority);
void plat_ic_raise_el3_sgi(int sgi_num, u_register_t target);
void plat_ic_raise_ns_sgi(int sgi_num, u_register_t target);
void plat_ic_raise_s_el1_sgi(int sgi_num, u_register_t target);
void plat_ic_set_spi_routing(unsigned int id, unsigned int routing_mode,
		u_register_t mpidr);
void plat_ic_set_interrupt_pending(unsigned int id);
void plat_ic_clear_interrupt_pending(unsigned int id);
unsigned int plat_ic_set_priority_mask(unsigned int mask);
unsigned int plat_ic_deactivate_priority(unsigned int mask);
unsigned int plat_ic_get_interrupt_id(unsigned int raw);

/*******************************************************************************
 * Optional common functions (may be overridden)
 ******************************************************************************/
uintptr_t plat_get_my_stack(void);
void plat_report_exception(unsigned int exception_type);
void plat_report_prefetch_abort(unsigned int fault_address);
void plat_report_data_abort(unsigned int fault_address);
int plat_crash_console_init(void);
int plat_crash_console_putc(int c);
void plat_crash_console_flush(void);
void plat_error_handler(int err) __dead2;
void plat_panic_handler(void) __dead2;
void plat_system_reset(void) __dead2;
const char *plat_log_get_prefix(unsigned int log_level);
void bl2_plat_preload_setup(void);
int plat_try_next_boot_source(void);

#if MEASURED_BOOT
int plat_mboot_measure_image(unsigned int image_id, image_info_t *image_data);
int plat_mboot_measure_critical_data(unsigned int critical_data_id,
				     const void *base,
				     size_t size);
int plat_mboot_measure_key(const void *pk_oid, const void *pk_ptr,
			   size_t pk_len);
#else
static inline int plat_mboot_measure_image(unsigned int image_id __unused,
					   image_info_t *image_data __unused)
{
	return 0;
}
static inline int plat_mboot_measure_critical_data(
					unsigned int critical_data_id __unused,
					const void *base __unused,
					size_t size __unused)
{
	return 0;
}
static inline int plat_mboot_measure_key(const void *pk_oid __unused,
					 const void *pk_ptr __unused,
					 size_t pk_len __unused)
{
	return 0;
}
#endif /* MEASURED_BOOT */

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
void plat_sdei_setup(void);
int plat_sdei_validate_entry_point(uintptr_t ep, unsigned int client_mode);
void plat_sdei_handle_masked_trigger(uint64_t mpidr, unsigned int intr);
#endif

void plat_default_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags);
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

#if MEASURED_BOOT
void bl1_plat_mboot_init(void);
void bl1_plat_mboot_finish(void);
#else
static inline void bl1_plat_mboot_init(void)
{
}
static inline void bl1_plat_mboot_finish(void)
{
}
#endif /* MEASURED_BOOT */

/*******************************************************************************
 * Mandatory BL2 functions
 ******************************************************************************/
void bl2_early_platform_setup2(u_register_t arg0, u_register_t arg1, u_register_t arg2, u_register_t arg3);
void bl2_plat_arch_setup(void);
void bl2_platform_setup(void);
struct meminfo *bl2_plat_sec_mem_layout(void);

/*
 * This function can be used by the platforms to update/use image
 * information for given `image_id`.
 */
int bl2_plat_handle_pre_image_load(unsigned int image_id);
int bl2_plat_handle_post_image_load(unsigned int image_id);

/*******************************************************************************
 * Optional BL2 functions (may be overridden)
 ******************************************************************************/
#if MEASURED_BOOT
void bl2_plat_mboot_init(void);
void bl2_plat_mboot_finish(void);
#else
static inline void bl2_plat_mboot_init(void)
{
}
static inline void bl2_plat_mboot_finish(void)
{
}
#endif /* MEASURED_BOOT */

/*******************************************************************************
 * Mandatory BL2 at EL3 functions: Must be implemented
 * if RESET_TO_BL2 image is supported
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
			unsigned int last_cpu_idx);
plat_local_state_t plat_get_target_pwr_state(unsigned int lvl,
			const plat_local_state_t *states,
			unsigned int ncpu);

/*******************************************************************************
 * Mandatory BL31 functions when ENABLE_RME=1
 ******************************************************************************/
#if ENABLE_RME
int plat_rmmd_get_cca_attest_token(uintptr_t buf, size_t *len,
				   uintptr_t hash, size_t hash_size);
int plat_rmmd_get_cca_realm_attest_key(uintptr_t buf, size_t *len,
				       unsigned int type);
size_t plat_rmmd_get_el3_rmm_shared_mem(uintptr_t *shared);
int plat_rmmd_load_manifest(struct rmm_manifest *manifest);
#endif

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
int get_mbedtls_heap_helper(void **heap_addr, size_t *heap_size);
int plat_get_enc_key_info(enum fw_enc_status_t fw_enc_status, uint8_t *key,
			  size_t *key_len, unsigned int *flags,
			  const uint8_t *img_id, size_t img_id_len);

/*******************************************************************************
 * Secure Partitions functions
 ******************************************************************************/
const struct mmap_region *plat_get_secure_partition_mmap(void *cookie);
const struct spm_mm_boot_info *plat_get_secure_partition_boot_info(
		void *cookie);
int plat_spm_sp_rd_load(struct sp_res_desc *rd, const void *ptr, size_t size);
int plat_spm_sp_get_next_address(void **sp_base, size_t *sp_size,
				 void **rd_base, size_t *rd_size);
#if defined(SPD_spmd)
int plat_spm_core_manifest_load(spmc_manifest_attribute_t *manifest,
				const void *pm_addr);
#endif
#if defined(SPMC_AT_EL3)
int plat_spmc_shmem_datastore_get(uint8_t **datastore, size_t *size);
#endif

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

/*
 * The below function enable Trusted Firmware components like SPDs which
 * haven't migrated to the new platform API to compile on platforms which
 * have the compatibility layer disabled.
 */
unsigned int platform_core_pos_helper(unsigned long mpidr);

/*
 * Optional function to get SOC version
 */
int32_t plat_get_soc_version(void);

/*
 * Optional function to get SOC revision
 */
int32_t plat_get_soc_revision(void);

/*
 * Optional function to check for SMCCC function availability for platform
 */
int32_t plat_is_smccc_feature_available(u_register_t fid);

/*******************************************************************************
 * FWU platform specific functions
 ******************************************************************************/
int plat_fwu_set_metadata_image_source(unsigned int image_id,
				       uintptr_t *dev_handle,
				       uintptr_t *image_spec);
void plat_fwu_set_images_source(const struct fwu_metadata *metadata);
uint32_t plat_fwu_get_boot_idx(void);

/*
 * Optional function to indicate if cache management operations can be
 * performed.
 */
#if CONDITIONAL_CMO
uint64_t plat_can_cmo(void);
#else
static inline uint64_t plat_can_cmo(void)
{
	return 1;
}
#endif /* CONDITIONAL_CMO */

#endif /* PLATFORM_H */
