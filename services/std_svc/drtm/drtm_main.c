/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * DRTM service
 *
 * Authors:
 *	Lucian Paul-Trifu <lucian.paultrifu@gmail.com>
 *	Brian Nezvadovitz <brinez@microsoft.com> 2021-02-01
 */

#include <stdint.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/auth/crypto_mod.h>
#include "drtm_main.h"
#include "drtm_remediation.h"
#include <lib/psci/psci_lib.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <services/drtm_svc.h>
#include <platform_def.h>

/* Structure to store DRTM features specific to the platform. */
static drtm_features_t plat_drtm_features;

/* DRTM-formatted memory map. */
static drtm_memory_region_descriptor_table_t *plat_drtm_mem_map;

int drtm_setup(void)
{
	bool rc;
	const plat_drtm_tpm_features_t *plat_tpm_feat;
	const plat_drtm_dma_prot_features_t *plat_dma_prot_feat;
	uint64_t dlme_data_min_size;

	INFO("DRTM service setup\n");

	/* Read boot PE ID from MPIDR */
	plat_drtm_features.boot_pe_id = read_mpidr_el1() & MPIDR_AFFINITY_MASK;

	rc = drtm_dma_prot_init();
	if (rc) {
		return INTERNAL_ERROR;
	}

	/*
	 * initialise the platform supported crypto module that will
	 * be used by the DRTM-service to calculate hash of DRTM-
	 * implementation specific components
	 */
	crypto_mod_init();

	/* Build DRTM-compatible address map. */
	plat_drtm_mem_map = drtm_build_address_map();
	if (plat_drtm_mem_map == NULL) {
		return INTERNAL_ERROR;
	}

	/* Get DRTM features from platform hooks. */
	plat_tpm_feat = plat_drtm_get_tpm_features();
	if (plat_tpm_feat == NULL) {
		return INTERNAL_ERROR;
	}

	plat_dma_prot_feat = plat_drtm_get_dma_prot_features();
	if (plat_dma_prot_feat == NULL) {
		return INTERNAL_ERROR;
	}

	/*
	 * Add up minimum DLME data memory.
	 *
	 * For systems with complete DMA protection there is only one entry in
	 * the protected regions table.
	 */
	if (plat_dma_prot_feat->dma_protection_support ==
			ARM_DRTM_DMA_PROT_FEATURES_DMA_SUPPORT_COMPLETE) {
		dlme_data_min_size =
			sizeof(drtm_memory_region_descriptor_table_t) +
			sizeof(drtm_mem_region_t);
	} else {
		/*
		 * TODO set protected regions table size based on platform DMA
		 * protection configuration
		 */
		panic();
	}

	dlme_data_min_size += (drtm_get_address_map_size() +
			       PLAT_DRTM_EVENT_LOG_MAX_SIZE +
			       plat_drtm_get_tcb_hash_table_size() +
			       plat_drtm_get_imp_def_dlme_region_size());

	dlme_data_min_size = page_align(dlme_data_min_size, UP)/PAGE_SIZE;

	/* Fill out platform DRTM features structure */
	/* Only support default PCR schema (0x1) in this implementation. */
	ARM_DRTM_TPM_FEATURES_SET_PCR_SCHEMA(plat_drtm_features.tpm_features,
		ARM_DRTM_TPM_FEATURES_PCR_SCHEMA_DEFAULT);
	ARM_DRTM_TPM_FEATURES_SET_TPM_HASH(plat_drtm_features.tpm_features,
		plat_tpm_feat->tpm_based_hash_support);
	ARM_DRTM_TPM_FEATURES_SET_FW_HASH(plat_drtm_features.tpm_features,
		plat_tpm_feat->firmware_hash_algorithm);
	ARM_DRTM_MIN_MEM_REQ_SET_MIN_DLME_DATA_SIZE(plat_drtm_features.minimum_memory_requirement,
		dlme_data_min_size);
	ARM_DRTM_MIN_MEM_REQ_SET_DCE_SIZE(plat_drtm_features.minimum_memory_requirement,
		plat_drtm_get_min_size_normal_world_dce());
	ARM_DRTM_DMA_PROT_FEATURES_SET_MAX_REGIONS(plat_drtm_features.dma_prot_features,
		plat_dma_prot_feat->max_num_mem_prot_regions);
	ARM_DRTM_DMA_PROT_FEATURES_SET_DMA_SUPPORT(plat_drtm_features.dma_prot_features,
		plat_dma_prot_feat->dma_protection_support);
	ARM_DRTM_TCB_HASH_FEATURES_SET_MAX_NUM_HASHES(plat_drtm_features.tcb_hash_features,
		plat_drtm_get_tcb_hash_features());

	return 0;
}

static inline uint64_t drtm_features_tpm(void *ctx)
{
	SMC_RET2(ctx, 1ULL, /* TPM feature is supported */
		 plat_drtm_features.tpm_features);
}

static inline uint64_t drtm_features_mem_req(void *ctx)
{
	SMC_RET2(ctx, 1ULL, /* memory req Feature is supported */
		 plat_drtm_features.minimum_memory_requirement);
}

static inline uint64_t drtm_features_boot_pe_id(void *ctx)
{
	SMC_RET2(ctx, 1ULL, /* Boot PE feature is supported */
		 plat_drtm_features.boot_pe_id);
}

static inline uint64_t drtm_features_dma_prot(void *ctx)
{
	SMC_RET2(ctx, 1ULL, /* DMA protection feature is supported */
		 plat_drtm_features.dma_prot_features);
}

static inline uint64_t drtm_features_tcb_hashes(void *ctx)
{
	SMC_RET2(ctx, 1ULL, /* TCB hash feature is supported */
		 plat_drtm_features.tcb_hash_features);
}

static enum drtm_retc drtm_dl_check_caller_el(void *ctx)
{
	uint64_t spsr_el3 = read_ctx_reg(get_el3state_ctx(ctx), CTX_SPSR_EL3);
	uint64_t dl_caller_el;
	uint64_t dl_caller_aarch;

	dl_caller_el = spsr_el3 >> MODE_EL_SHIFT & MODE_EL_MASK;
	dl_caller_aarch = spsr_el3 >> MODE_RW_SHIFT & MODE_RW_MASK;

	/* Caller's security state is checked from drtm_smc_handle function */

	/* Caller can be NS-EL2/EL1 */
	if (dl_caller_el == MODE_EL3) {
		ERROR("DRTM: invalid launch from EL3\n");
		return DENIED;
	}

	if (dl_caller_aarch != MODE_RW_64) {
		ERROR("DRTM: invalid launch from non-AArch64 execution state\n");
		return DENIED;
	}

	return SUCCESS;
}

static enum drtm_retc drtm_dl_check_cores(void)
{
	bool running_on_single_core;
	uint64_t this_pe_aff_value = read_mpidr_el1() & MPIDR_AFFINITY_MASK;

	if (this_pe_aff_value != plat_drtm_features.boot_pe_id) {
		ERROR("DRTM: invalid launch on a non-boot PE\n");
		return DENIED;
	}

	running_on_single_core = psci_is_last_on_cpu_safe();
	if (!running_on_single_core) {
		ERROR("DRTM: invalid launch due to non-boot PE not being turned off\n");
		return DENIED;
	}

	return SUCCESS;
}

static enum drtm_retc drtm_dl_prepare_dlme_data(const struct_drtm_dl_args *args,
						size_t *dlme_data_size_out)
{
	size_t dlme_data_total_bytes_req = 0;

	*dlme_data_size_out = dlme_data_total_bytes_req;

	return SUCCESS;
}

/*
 * Note: accesses to the dynamic launch args, and to the DLME data are
 * little-endian as required, thanks to TF-A BL31 init requirements.
 */
static enum drtm_retc drtm_dl_check_args(uint64_t x1,
					 struct_drtm_dl_args *a_out)
{
	uint64_t dlme_start, dlme_end;
	uint64_t dlme_img_start, dlme_img_ep, dlme_img_end;
	uint64_t dlme_data_start, dlme_data_end;
	uintptr_t args_mapping;
	size_t args_mapping_size;
	struct_drtm_dl_args *a;
	struct_drtm_dl_args args_buf;
	size_t dlme_data_size_req;
	int rc;

	if (x1 % DRTM_PAGE_SIZE != 0) {
		ERROR("DRTM: parameters structure is not "
		      DRTM_PAGE_SIZE_STR "-aligned\n");
		return INVALID_PARAMETERS;
	}

	args_mapping_size = ALIGNED_UP(sizeof(struct_drtm_dl_args), DRTM_PAGE_SIZE);
	rc = mmap_add_dynamic_region_alloc_va(x1, &args_mapping, args_mapping_size,
					      MT_MEMORY | MT_NS | MT_RO |
					      MT_SHAREABILITY_ISH);
	if (rc != 0) {
		WARN("DRTM: %s: mmap_add_dynamic_region() failed rc=%d\n",
		      __func__, rc);
		return INTERNAL_ERROR;
	}
	a = (struct_drtm_dl_args *)args_mapping;
	/*
	 * TODO: invalidate all data cache before reading the data passed by the
	 * DCE Preamble.  This is required to avoid / defend against racing with
	 * cache evictions.
	 */
	args_buf = *a;

	rc = mmap_remove_dynamic_region(args_mapping, args_mapping_size);
	if (rc) {
		ERROR("%s(): mmap_remove_dynamic_region() failed unexpectedly"
		      " rc=%d\n", __func__, rc);
		panic();
	}
	a = &args_buf;

	if (a->version != 1) {
		ERROR("DRTM: parameters structure incompatible with major version %d\n",
		      ARM_DRTM_VERSION_MAJOR);
		return NOT_SUPPORTED;
	}

	if (!(a->dlme_img_off < a->dlme_size &&
	      a->dlme_data_off < a->dlme_size)) {
		ERROR("DRTM: argument offset is outside of the DLME region\n");
		return INVALID_PARAMETERS;
	}
	dlme_start = a->dlme_paddr;
	dlme_end = a->dlme_paddr + a->dlme_size;
	dlme_img_start = a->dlme_paddr + a->dlme_img_off;
	dlme_img_ep = dlme_img_start + a->dlme_img_ep_off;
	dlme_img_end = dlme_img_start + a->dlme_img_size;
	dlme_data_start = a->dlme_paddr + a->dlme_data_off;
	dlme_data_end = dlme_end;

	/*
	 * TODO: validate that the DLME physical address range is all NS memory,
	 * return INVALID_PARAMETERS if it is not.
	 * Note that this check relies on platform-specific information. For
	 * examples, see psci_plat_pm_ops->validate_ns_entrypoint() or
	 * arm_validate_ns_entrypoint().
	 */

	/* Check the DLME regions arguments. */
	if ((dlme_start % DRTM_PAGE_SIZE) != 0) {
		ERROR("DRTM: argument DLME region is not "
		      DRTM_PAGE_SIZE_STR "-aligned\n");
		return INVALID_PARAMETERS;
	}

	if (!(dlme_start < dlme_end &&
	      dlme_start <= dlme_img_start && dlme_img_start < dlme_img_end &&
	      dlme_start <= dlme_data_start && dlme_data_start < dlme_data_end)) {
		ERROR("DRTM: argument DLME region is discontiguous\n");
		return INVALID_PARAMETERS;
	}

	if (dlme_img_start < dlme_data_end && dlme_data_start < dlme_img_end) {
		ERROR("DRTM: argument DLME regions overlap\n");
		return INVALID_PARAMETERS;
	}

	/* Check the DLME image region arguments. */
	if ((dlme_img_start % DRTM_PAGE_SIZE) != 0) {
		ERROR("DRTM: argument DLME image region is not "
		      DRTM_PAGE_SIZE_STR "-aligned\n");
		return INVALID_PARAMETERS;
	}

	if (!(dlme_img_start <= dlme_img_ep && dlme_img_ep < dlme_img_end)) {
		ERROR("DRTM: DLME entry point is outside of the DLME image region\n");
		return INVALID_PARAMETERS;
	}

	if ((dlme_img_ep % 4) != 0) {
		ERROR("DRTM: DLME image entry point is not 4-byte-aligned\n");
		return INVALID_PARAMETERS;
	}

	/* Check the DLME data region arguments. */
	if ((dlme_data_start % DRTM_PAGE_SIZE) != 0) {
		ERROR("DRTM: argument DLME data region is not "
		      DRTM_PAGE_SIZE_STR "-aligned\n");
		return INVALID_PARAMETERS;
	}

	rc = drtm_dl_prepare_dlme_data(NULL, &dlme_data_size_req);
	if (rc) {
		ERROR("%s: drtm_dl_prepare_dlme_data() failed unexpectedly rc=%d\n",
		      __func__, rc);
		panic();
	}
	if (dlme_data_end - dlme_data_start < dlme_data_size_req) {
		ERROR("DRTM: argument DLME data region is short of %lu bytes\n",
		      dlme_data_size_req - (size_t)(dlme_data_end - dlme_data_start));
		return INVALID_PARAMETERS;
	}

	/* Check the Normal World DCE region arguments. */
	if (a->dce_nwd_paddr != 0) {
		uint32_t dce_nwd_start = a->dce_nwd_paddr;
		uint32_t dce_nwd_end = dce_nwd_start + a->dce_nwd_size;

		if (!(dce_nwd_start < dce_nwd_end)) {
			ERROR("DRTM: argument Normal World DCE region is dicontiguous\n");
			return INVALID_PARAMETERS;
		}

		if (dce_nwd_start < dlme_end && dlme_start < dce_nwd_end) {
			ERROR("DRTM: argument Normal World DCE regions overlap\n");
			return INVALID_PARAMETERS;
		}
	}

	*a_out = *a;
	return SUCCESS;
}

static uint64_t drtm_dynamic_launch(uint64_t x1, void *handle)
{
	enum drtm_retc ret = SUCCESS;
	struct_drtm_dl_args args;

	/* Ensure that only boot PE is powered on */
	ret = drtm_dl_check_cores();
	if (ret != SUCCESS) {
		SMC_RET1(handle, ret);
	}

	/*
	 * Ensure that execution state is AArch64 and the caller
	 * is highest non-secure exception level
	 */
	ret = drtm_dl_check_caller_el(handle);
	if (ret != SUCCESS) {
		SMC_RET1(handle, ret);
	}

	ret = drtm_dl_check_args(x1, &args);
	if (ret != SUCCESS) {
		SMC_RET1(handle, ret);
	}

	SMC_RET1(handle, ret);
}

uint64_t drtm_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	/* Check that the SMC call is from the Normal World. */
	if (!is_caller_non_secure(flags)) {
		SMC_RET1(handle, NOT_SUPPORTED);
	}

	switch (smc_fid) {
	case ARM_DRTM_SVC_VERSION:
		INFO("DRTM service handler: version\n");
		/* Return the version of current implementation */
		SMC_RET1(handle, ARM_DRTM_VERSION);
		break;	/* not reached */

	case ARM_DRTM_SVC_FEATURES:
		if (((x1 >> ARM_DRTM_FUNC_SHIFT) & ARM_DRTM_FUNC_MASK) ==
		    ARM_DRTM_FUNC_ID) {
			/* Dispatch function-based queries. */
			switch (x1 & FUNCID_MASK) {
			case ARM_DRTM_SVC_VERSION:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_FEATURES:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_UNPROTECT_MEM:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_DYNAMIC_LAUNCH:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_CLOSE_LOCALITY:
				WARN("ARM_DRTM_SVC_CLOSE_LOCALITY feature %s",
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);
				break;	/* not reached */

			case ARM_DRTM_SVC_GET_ERROR:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_SET_ERROR:
				SMC_RET1(handle, SUCCESS);
				break;	/* not reached */

			case ARM_DRTM_SVC_SET_TCB_HASH:
				WARN("ARM_DRTM_SVC_TCB_HASH feature %s",
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);
				break;	/* not reached */

			case ARM_DRTM_SVC_LOCK_TCB_HASH:
				WARN("ARM_DRTM_SVC_LOCK_TCB_HASH feature %s",
				     "is not supported\n");
				SMC_RET1(handle, NOT_SUPPORTED);
				break;	/* not reached */

			default:
				ERROR("Unknown DRTM service function\n");
				SMC_RET1(handle, NOT_SUPPORTED);
				break;	/* not reached */
			}
		} else {
			/* Dispatch feature-based queries. */
			switch (x1 & ARM_DRTM_FEAT_ID_MASK) {
			case ARM_DRTM_FEATURES_TPM:
				INFO("++ DRTM service handler: TPM features\n");
				return drtm_features_tpm(handle);
				break;	/* not reached */

			case ARM_DRTM_FEATURES_MEM_REQ:
				INFO("++ DRTM service handler: Min. mem."
				     " requirement features\n");
				return drtm_features_mem_req(handle);
				break;	/* not reached */

			case ARM_DRTM_FEATURES_DMA_PROT:
				INFO("++ DRTM service handler: "
				     "DMA protection features\n");
				return drtm_features_dma_prot(handle);
				break;	/* not reached */

			case ARM_DRTM_FEATURES_BOOT_PE_ID:
				INFO("++ DRTM service handler: "
				     "Boot PE ID features\n");
				return drtm_features_boot_pe_id(handle);
				break;	/* not reached */

			case ARM_DRTM_FEATURES_TCB_HASHES:
				INFO("++ DRTM service handler: "
				     "TCB-hashes features\n");
				return drtm_features_tcb_hashes(handle);
				break;	/* not reached */

			default:
				ERROR("Unknown ARM DRTM service feature\n");
				SMC_RET1(handle, NOT_SUPPORTED);
				break;	/* not reached */
			}
		}

	case ARM_DRTM_SVC_UNPROTECT_MEM:
		INFO("DRTM service handler: unprotect mem\n");
		SMC_RET1(handle, SMC_OK);
		break;	/* not reached */

	case ARM_DRTM_SVC_DYNAMIC_LAUNCH:
		INFO("DRTM service handler: dynamic launch\n");
		return drtm_dynamic_launch(x1, handle);
		break;	/* not reached */

	case ARM_DRTM_SVC_CLOSE_LOCALITY:
		WARN("DRTM service handler: close locality %s\n",
		     "is not supported");
		SMC_RET1(handle, NOT_SUPPORTED);
		break;	/* not reached */

	case ARM_DRTM_SVC_GET_ERROR:
		INFO("DRTM service handler: get error\n");
		drtm_get_error(handle);
		break;	/* not reached */

	case ARM_DRTM_SVC_SET_ERROR:
		INFO("DRTM service handler: set error\n");
		drtm_set_error(x1, handle);
		break;	/* not reached */

	case ARM_DRTM_SVC_SET_TCB_HASH:
		WARN("DRTM service handler: set TCB hash %s\n",
		     "is not supported");
		SMC_RET1(handle, NOT_SUPPORTED);
		break;  /* not reached */

	case ARM_DRTM_SVC_LOCK_TCB_HASH:
		WARN("DRTM service handler: lock TCB hash %s\n",
		     "is not supported");
		SMC_RET1(handle, NOT_SUPPORTED);
		break;  /* not reached */

	default:
		ERROR("Unknown DRTM service function: 0x%x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
		break;	/* not reached */
	}

	/* not reached */
	SMC_RET1(handle, SMC_UNK);
}
