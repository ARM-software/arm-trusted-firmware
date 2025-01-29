/*
 * Copyright (c) 2021-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include <arch_helpers.h>
#include <arch_features.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/cpu_data.h>
#include <lib/el3_runtime/pubsub.h>
#include <lib/extensions/mpam.h>
#include <lib/extensions/pmuv3.h>
#include <lib/extensions/sys_reg_trace.h>
#include <lib/gpt_rme/gpt_rme.h>
#include <lib/per_cpu/per_cpu.h>

#include <lib/spinlock.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/rmmd_svc.h>
#include <smccc_helpers.h>
#include <lib/extensions/sme.h>
#include <lib/extensions/sve.h>
#include <lib/extensions/spe.h>
#include <lib/extensions/trbe.h>
#include "rmmd_private.h"

#define MECID_SHIFT			U(32)
#define MECID_MASK			0xFFFFU

#define MEC_REFRESH_REASON_SHIFT	U(0)
#define MEC_REFRESH_REASON_MASK		BIT(0)

/*******************************************************************************
 * RMM boot failure flag
 ******************************************************************************/
static bool rmm_boot_failed;

/*******************************************************************************
 * RMM context information.
 ******************************************************************************/
PER_CPU_DEFINE(rmmd_rmm_context_t, rmm_context);

/*******************************************************************************
 * RMM entry point information. Discovered on the primary core and reused
 * on secondary cores.
 ******************************************************************************/
static entry_point_info_t *rmm_ep_info;

/*******************************************************************************
 * Static function declaration.
 ******************************************************************************/
static int32_t rmm_init(void);

/*******************************************************************************
 * This function takes an RMM context pointer and performs a synchronous entry
 * into it.
 ******************************************************************************/
uint64_t rmmd_rmm_sync_entry(rmmd_rmm_context_t *rmm_ctx)
{
	uint64_t rc;

	assert(rmm_ctx != NULL);

	cm_set_context(&(rmm_ctx->cpu_ctx), REALM);

	/* Restore the realm context assigned above */
	cm_el2_sysregs_context_restore(REALM);
	cm_set_next_eret_context(REALM);

	/* Enter RMM */
	rc = rmmd_rmm_enter(&rmm_ctx->c_rt_ctx);

	/*
	 * Save realm context. EL2 Non-secure context will be restored
	 * before exiting Non-secure world, therefore there is no need
	 * to clear EL2 context registers.
	 */
	cm_el2_sysregs_context_save(REALM);

	return rc;
}

/*******************************************************************************
 * This function returns to the place where rmmd_rmm_sync_entry() was
 * called originally.
 ******************************************************************************/
__dead2 void rmmd_rmm_sync_exit(uint64_t rc)
{
	rmmd_rmm_context_t *ctx = PER_CPU_CUR(rmm_context);

	/* Get context of the RMM in use by this CPU. */
	assert(cm_get_context(REALM) == &(ctx->cpu_ctx));

	/*
	 * The RMMD must have initiated the original request through a
	 * synchronous entry into RMM. Jump back to the original C runtime
	 * context with the value of rc in x0;
	 */
	rmmd_rmm_exit(ctx->c_rt_ctx, rc);

	panic();
}

/*******************************************************************************
 * Jump to the RMM for the first time.
 ******************************************************************************/
static int32_t rmm_init(void)
{
	long rc;
	rmmd_rmm_context_t *ctx = PER_CPU_CUR(rmm_context);

	INFO("RMM init start.\n");

	rc = rmmd_rmm_sync_entry(ctx);
	if (rc != E_RMM_BOOT_SUCCESS) {
		ERROR("RMM init failed: %ld\n", rc);
		/* Mark the boot as failed for all the CPUs */
		rmm_boot_failed = true;
		return 0;
	}

	INFO("RMM init end.\n");

	return 1;
}

/*******************************************************************************
 * Load and read RMM manifest, setup RMM.
 ******************************************************************************/
int rmmd_setup(void)
{
	size_t shared_buf_size __unused;
	uintptr_t shared_buf_base;
	uint32_t ep_attr;
	unsigned int linear_id = plat_my_core_pos();

	rmmd_rmm_context_t *rmm_ctx = PER_CPU_CUR(rmm_context);
	struct rmm_manifest *manifest;
	int rc;

	/* Make sure RME is supported. */
	if (is_feat_rme_present() == 0U) {
		/* Mark the RMM boot as failed for all the CPUs */
		rmm_boot_failed = true;
		return -ENOTSUP;
	}

	rmm_ep_info = bl31_plat_get_next_image_ep_info(REALM);
	if ((rmm_ep_info == NULL) || (rmm_ep_info->pc == 0)) {
		WARN("No RMM image provided by BL2 boot loader, Booting "
		     "device without RMM initialization. SMCs destined for "
		     "RMM will return SMC_UNK\n");

		/* Mark the boot as failed for all the CPUs */
		rmm_boot_failed = true;
		return -ENOENT;
	}

	/* Initialise an entrypoint to set up the CPU context */
	ep_attr = EP_REALM;
	if ((read_sctlr_el3() & SCTLR_EE_BIT) != 0U) {
		ep_attr |= EP_EE_BIG;
	}

	SET_PARAM_HEAD(rmm_ep_info, PARAM_EP, VERSION_1, ep_attr);
	rmm_ep_info->spsr = SPSR_64(MODE_EL2,
					MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS);

	shared_buf_size =
			plat_rmmd_get_el3_rmm_shared_mem(&shared_buf_base);

	assert((shared_buf_size == SZ_4K) &&
					((void *)shared_buf_base != NULL));

	/* Zero out and load the boot manifest at the beginning of the share area */
	manifest = (struct rmm_manifest *)shared_buf_base;
	(void)memset((void *)manifest, 0, sizeof(struct rmm_manifest));

	rc = plat_rmmd_load_manifest(manifest);
	if (rc != 0) {
		ERROR("Error loading RMM Boot Manifest (%i)\n", rc);
		/* Mark the boot as failed for all the CPUs */
		rmm_boot_failed = true;
		return rc;
	}
	flush_dcache_range((uintptr_t)shared_buf_base, shared_buf_size);

	/*
	 * Prepare coldboot arguments for RMM:
	 * arg0: This CPUID (primary processor).
	 * arg1: Version for this Boot Interface.
	 * arg2: PLATFORM_CORE_COUNT.
	 * arg3: Base address for the EL3 <-> RMM shared area. The boot
	 *       manifest will be stored at the beginning of this area.
	 * arg4: opaque activation token, as returned by previous calls
	 */
	rmm_ep_info->args.arg0 = linear_id;
	rmm_ep_info->args.arg1 = RMM_EL3_INTERFACE_VERSION;
	rmm_ep_info->args.arg2 = PLATFORM_CORE_COUNT;
	rmm_ep_info->args.arg3 = shared_buf_base;
	rmm_ep_info->args.arg4 = rmm_ctx->activation_token;

	/* Initialise RMM context with this entry point information */
	cm_setup_context(&rmm_ctx->cpu_ctx, rmm_ep_info);

	INFO("RMM setup done.\n");

	/* Register init function for deferred init.  */
	bl31_register_rmm_init(&rmm_init);

	return 0;
}

/*******************************************************************************
 * Forward SMC to the other security state
 ******************************************************************************/
static uint64_t	rmmd_smc_forward(uint32_t src_sec_state,
				 uint32_t dst_sec_state, uint64_t x0,
				 uint64_t x1, uint64_t x2, uint64_t x3,
				 uint64_t x4, void *handle)
{
	cpu_context_t *ctx = cm_get_context(dst_sec_state);

	/* Save incoming security state */
	cm_el2_sysregs_context_save(src_sec_state);

	/* Restore outgoing security state */
	cm_el2_sysregs_context_restore(dst_sec_state);
	cm_set_next_eret_context(dst_sec_state);

	/*
	 * As per SMCCCv1.2, we need to preserve x4 to x7 unless
	 * being used as return args. Hence we differentiate the
	 * onward and backward path. Support upto 8 args in the
	 * onward path and 4 args in return path.
	 * Register x4 will be preserved by RMM in case it is not
	 * used in return path.
	 */
	if (src_sec_state == NON_SECURE) {
		SMC_RET8(ctx, x0, x1, x2, x3, x4,
			 SMC_GET_GP(handle, CTX_GPREG_X5),
			 SMC_GET_GP(handle, CTX_GPREG_X6),
			 SMC_GET_GP(handle, CTX_GPREG_X7));
	}

	SMC_RET5(ctx, x0, x1, x2, x3, x4);
}

/*******************************************************************************
 * This function handles all SMCs in the range reserved for RMI. Each call is
 * either forwarded to the other security state or handled by the RMM dispatcher
 ******************************************************************************/
uint64_t rmmd_rmi_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			  uint64_t x3, uint64_t x4, void *cookie,
			  void *handle, uint64_t flags)
{
	uint32_t src_sec_state;

	/* If RMM failed to boot, treat any RMI SMC as unknown */
	if (rmm_boot_failed) {
		WARN("RMMD: Failed to boot up RMM. Ignoring RMI call\n");
		SMC_RET1(handle, SMC_UNK);
	}

	/* Determine which security state this SMC originated from */
	src_sec_state = caller_sec_state(flags);

	/* RMI must not be invoked by the Secure world */
	if (src_sec_state == SMC_FROM_SECURE) {
		WARN("RMMD: RMI invoked by secure world.\n");
		SMC_RET1(handle, SMC_UNK);
	}

	/*
	 * Forward an RMI call from the Normal world to the Realm world as it
	 * is.
	 */
	if (src_sec_state == SMC_FROM_NON_SECURE) {
		/*
		 * If SVE hint bit is set in the flags then update the SMC
		 * function id and pass it on to the lower EL.
		 */
		if (is_sve_hint_set(flags)) {
			smc_fid |= (FUNCID_SVE_HINT_MASK <<
				    FUNCID_SVE_HINT_SHIFT);
		}
		VERBOSE("RMMD: RMI call from non-secure world.\n");
		return rmmd_smc_forward(NON_SECURE, REALM, smc_fid,
					x1, x2, x3, x4, handle);
	}

	if (src_sec_state != SMC_FROM_REALM) {
		SMC_RET1(handle, SMC_UNK);
	}

	switch (smc_fid) {
	case RMM_RMI_REQ_COMPLETE: {
		uint64_t x5 = SMC_GET_GP(handle, CTX_GPREG_X5);

		return rmmd_smc_forward(REALM, NON_SECURE, x1,
					x2, x3, x4, x5, handle);
	}
	default:
		WARN("RMMD: Unsupported RMM call 0x%08x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/*******************************************************************************
 * This cpu has been turned on. Enter RMM to initialise R-EL2.  Entry into RMM
 * is done after initialising minimal architectural state that guarantees safe
 * execution.
 ******************************************************************************/
static void *rmmd_cpu_on_finish_handler(const void *arg)
{
	long rc;
	uint32_t linear_id = plat_my_core_pos();
	rmmd_rmm_context_t *ctx = PER_CPU_CUR(rmm_context);
	/* Create a local copy of ep info to avoid race conditions */
	entry_point_info_t local_rmm_ep_info = *rmm_ep_info;

	if (rmm_boot_failed) {
		/* RMM Boot failed on a previous CPU. Abort. */
		ERROR("RMM Failed to initialize. Ignoring for CPU%d\n",
								linear_id);
		return NULL;
	}

	/*
	 * Prepare warmboot arguments for RMM:
	 * arg0: This CPUID.
	 * arg1: opaque activation token, as returned by previous calls
	 * arg2 to arg3: Not used.
	 */
	local_rmm_ep_info.args.arg0 = linear_id;
	local_rmm_ep_info.args.arg1 = ctx->activation_token;
	local_rmm_ep_info.args.arg2 = 0ULL;
	local_rmm_ep_info.args.arg3 = 0ULL;

	/* Initialise RMM context with this entry point information */
	cm_setup_context(&ctx->cpu_ctx, &local_rmm_ep_info);

	rc = rmmd_rmm_sync_entry(ctx);

	if (rc != E_RMM_BOOT_SUCCESS) {
		ERROR("RMM init failed on CPU%d: %ld\n", linear_id, rc);
		/*
		 * TODO: Investigate handling of rmm_boot_failed under
		 * concurrent access, or explore alternative approaches
		 * to fixup the logic.
		 */
		rmm_boot_failed = true;
	}

	return NULL;
}

/* Subscribe to PSCI CPU on to initialize RMM on secondary */
SUBSCRIBE_TO_EVENT(psci_cpu_on_finish, rmmd_cpu_on_finish_handler);

/* Convert GPT lib error to RMMD GTS error */
static int gpt_to_gts_error(int error, uint32_t smc_fid, uint64_t address)
{
	int ret;

	if (error == 0) {
		return E_RMM_OK;
	}

	if (error == -EINVAL) {
		ret = E_RMM_BAD_ADDR;
	} else {
		/* This is the only other error code we expect */
		assert(error == -EPERM);
		ret = E_RMM_BAD_PAS;
	}

	ERROR("RMMD: PAS Transition failed. GPT ret = %d, PA: 0x%"PRIx64 ", FID = 0x%x\n",
				error, address, smc_fid);
	return ret;
}

static int rmm_el3_ifc_get_feat_register(uint64_t feat_reg_idx,
					 uint64_t *feat_reg)
{
	if (feat_reg_idx != RMM_EL3_FEAT_REG_0_IDX) {
		ERROR("RMMD: Failed to get feature register %ld\n", feat_reg_idx);
		return E_RMM_INVAL;
	}

	*feat_reg = 0UL;
#if RMMD_ENABLE_EL3_TOKEN_SIGN
	*feat_reg |= RMM_EL3_FEAT_REG_0_EL3_TOKEN_SIGN_MASK;
#endif
	return E_RMM_OK;
}

/*
 * Update encryption key associated with mecid included in x1.
 */
static int rmmd_mecid_key_update(uint64_t x1)
{
	uint64_t mecid_width, mecid_width_mask;
	uint16_t mecid;
	unsigned int reason;
	int ret;

	/*
	 * Check whether FEAT_MEC is supported by the hardware. If not, return
	 * unknown SMC.
	 */
	if (is_feat_mec_supported() == false) {
		return E_RMM_UNK;
	}

	/*
	 * Check whether the mecid parameter is at most MECIDR_EL2.MECIDWidthm1 + 1
	 * in length.
	 */
	mecid_width = ((read_mecidr_el2() >> MECIDR_EL2_MECIDWidthm1_SHIFT) &
		MECIDR_EL2_MECIDWidthm1_MASK) + 1UL;
	mecid_width_mask = ((1UL << mecid_width) - 1UL);

	mecid = (x1 >> MECID_SHIFT) & MECID_MASK;
	if ((mecid & ~mecid_width_mask) != 0U) {
		return E_RMM_INVAL;
	}

	reason = (x1 >> MEC_REFRESH_REASON_SHIFT) & MEC_REFRESH_REASON_MASK;
	ret = plat_rmmd_mecid_key_update(mecid, reason);

	if (ret != 0) {
		return E_RMM_UNK;
	}
	return E_RMM_OK;
}

/*******************************************************************************
 * This function handles RMM-EL3 interface SMCs
 ******************************************************************************/
uint64_t rmmd_rmm_el3_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
				uint64_t x3, uint64_t x4, void *cookie,
				void *handle, uint64_t flags)
{
	uint64_t remaining_len = 0UL;
	uint32_t src_sec_state;
	int ret;

	/* If RMM failed to boot, treat any RMM-EL3 interface SMC as unknown */
	if (rmm_boot_failed) {
		WARN("RMMD: Failed to boot up RMM. Ignoring RMM-EL3 call\n");
		SMC_RET1(handle, SMC_UNK);
	}

	/* Determine which security state this SMC originated from */
	src_sec_state = caller_sec_state(flags);

	if (src_sec_state != SMC_FROM_REALM) {
		WARN("RMMD: RMM-EL3 call originated from secure or normal world\n");
		SMC_RET1(handle, SMC_UNK);
	}

	switch (smc_fid) {
	case RMM_GTSI_DELEGATE:
		ret = gpt_delegate_pas(x1, PAGE_SIZE_4KB, SMC_FROM_REALM);
		SMC_RET1(handle, gpt_to_gts_error(ret, smc_fid, x1));
	case RMM_GTSI_UNDELEGATE:
		ret = gpt_undelegate_pas(x1, PAGE_SIZE_4KB, SMC_FROM_REALM);
		SMC_RET1(handle, gpt_to_gts_error(ret, smc_fid, x1));
	case RMM_ATTEST_GET_REALM_KEY:
		ret = rmmd_attest_get_signing_key(x1, &x2, x3);
		SMC_RET2(handle, ret, x2);
	case RMM_ATTEST_GET_PLAT_TOKEN:
		ret = rmmd_attest_get_platform_token(x1, &x2, x3, &remaining_len);
		SMC_RET3(handle, ret, x2, remaining_len);
	case RMM_EL3_FEATURES:
		ret = rmm_el3_ifc_get_feat_register(x1, &x2);
		SMC_RET2(handle, ret, x2);
#if RMMD_ENABLE_EL3_TOKEN_SIGN
	case RMM_EL3_TOKEN_SIGN:
		return rmmd_el3_token_sign(handle, x1, x2, x3, x4);
#endif

#if RMMD_ENABLE_IDE_KEY_PROG
	case RMM_IDE_KEY_PROG:
	{
		rp_ide_key_info_t ide_key_info;

		ide_key_info.keyqw0 = x4;
		ide_key_info.keyqw1 = SMC_GET_GP(handle, CTX_GPREG_X5);
		ide_key_info.keyqw2 = SMC_GET_GP(handle, CTX_GPREG_X6);
		ide_key_info.keyqw3 = SMC_GET_GP(handle, CTX_GPREG_X7);
		ide_key_info.ifvqw0 = SMC_GET_GP(handle, CTX_GPREG_X8);
		ide_key_info.ifvqw1 = SMC_GET_GP(handle, CTX_GPREG_X9);
		uint64_t x10 = SMC_GET_GP(handle, CTX_GPREG_X10);
		uint64_t x11 = SMC_GET_GP(handle, CTX_GPREG_X11);

		ret = rmmd_el3_ide_key_program(x1, x2, x3, &ide_key_info, x10, x11);
		SMC_RET1(handle, ret);
	}
	case RMM_IDE_KEY_SET_GO:
		ret = rmmd_el3_ide_key_set_go(x1, x2, x3, x4, SMC_GET_GP(handle, CTX_GPREG_X5));
		SMC_RET1(handle, ret);
	case RMM_IDE_KEY_SET_STOP:
		ret = rmmd_el3_ide_key_set_stop(x1, x2, x3, x4, SMC_GET_GP(handle, CTX_GPREG_X5));
		SMC_RET1(handle, ret);
	case RMM_IDE_KM_PULL_RESPONSE: {
		uint64_t req_resp = 0, req_id = 0, cookie_var = 0;

		ret = rmmd_el3_ide_km_pull_response(x1, x2, &req_resp, &req_id, &cookie_var);
		SMC_RET4(handle, ret, req_resp, req_id, cookie_var);
	}
#endif /* RMMD_ENABLE_IDE_KEY_PROG */
	case RMM_RESERVE_MEMORY:
		ret = rmmd_reserve_memory(x1, &x2);
		SMC_RET2(handle, ret, x2);

	case RMM_BOOT_COMPLETE:
	{
		rmmd_rmm_context_t *ctx = PER_CPU_CUR(rmm_context);

		ctx->activation_token = x2;
		VERBOSE("RMMD: running rmmd_rmm_sync_exit\n");
		rmmd_rmm_sync_exit(x1);
	}
	case RMM_MEC_REFRESH:
		ret = rmmd_mecid_key_update(x1);
		SMC_RET1(handle, ret);
	default:
		WARN("RMMD: Unsupported RMM-EL3 call 0x%08x\n", smc_fid);
		SMC_RET1(handle, SMC_UNK);
	}
}

/**
 * Helper to activate Primary CPU with the updated RMM, mainly used during
 * LFA of RMM.
 */
int rmmd_primary_activate(void)
{
	int rc;

	rc = rmmd_setup();
	if (rc != 0) {
		ERROR("rmmd_setup failed during LFA: %d\n", rc);
		return rc;
	}

	rc = rmm_init();
	if (rc == 0) {
		ERROR("rmm_init failed during LFA: %d\n", rc);
		return rc;
	}

	INFO("RMM warm reset done on primary during LFA. \n");

	return 0;
}

/**
 * Helper to activate Primary CPU with the updated RMM, mainly used during
 * LFA of RMM.
 */
int rmmd_secondary_activate(void)
{
	rmmd_cpu_on_finish_handler(NULL);

	return 0;
}
