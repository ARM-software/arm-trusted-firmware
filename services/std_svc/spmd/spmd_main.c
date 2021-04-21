/*
 * Copyright (c) 2020-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch_helpers.h>
#include <arch/aarch64/arch_features.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <lib/utils.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/ffa_svc.h>
#include <services/spmd_svc.h>
#include <smccc_helpers.h>
#include "spmd_private.h"

/*******************************************************************************
 * SPM Core context information.
 ******************************************************************************/
static spmd_spm_core_context_t spm_core_context[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * SPM Core attribute information read from its manifest.
 ******************************************************************************/
static spmc_manifest_attribute_t spmc_attrs;

/*******************************************************************************
 * SPM Core entry point information. Discovered on the primary core and reused
 * on secondary cores.
 ******************************************************************************/
static entry_point_info_t *spmc_ep_info;

/*******************************************************************************
 * SPM Core context on CPU based on mpidr.
 ******************************************************************************/
spmd_spm_core_context_t *spmd_get_context_by_mpidr(uint64_t mpidr)
{
	int core_idx = plat_core_pos_by_mpidr(mpidr);

	if (core_idx < 0) {
		ERROR("Invalid mpidr: %llx, returned ID: %d\n", mpidr, core_idx);
		panic();
	}

	return &spm_core_context[core_idx];
}

/*******************************************************************************
 * SPM Core context on current CPU get helper.
 ******************************************************************************/
spmd_spm_core_context_t *spmd_get_context(void)
{
	return spmd_get_context_by_mpidr(read_mpidr());
}

/*******************************************************************************
 * SPM Core entry point information get helper.
 ******************************************************************************/
entry_point_info_t *spmd_spmc_ep_info_get(void)
{
	return spmc_ep_info;
}

/*******************************************************************************
 * SPM Core ID getter.
 ******************************************************************************/
uint16_t spmd_spmc_id_get(void)
{
	return spmc_attrs.spmc_id;
}

/*******************************************************************************
 * Static function declaration.
 ******************************************************************************/
static int32_t spmd_init(void);
static int spmd_spmc_init(void *pm_addr);
static uint64_t spmd_ffa_error_return(void *handle,
				       int error_code);
static uint64_t spmd_smc_forward(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *handle);

/*******************************************************************************
 * This function takes an SPMC context pointer and performs a synchronous
 * SPMC entry.
 ******************************************************************************/
uint64_t spmd_spm_core_sync_entry(spmd_spm_core_context_t *spmc_ctx)
{
	uint64_t rc;

	assert(spmc_ctx != NULL);

	cm_set_context(&(spmc_ctx->cpu_ctx), SECURE);

	/* Restore the context assigned above */
	cm_el1_sysregs_context_restore(SECURE);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_restore(SECURE);
#endif
	cm_set_next_eret_context(SECURE);

	/* Enter SPMC */
	rc = spmd_spm_core_enter(&spmc_ctx->c_rt_ctx);

	/* Save secure state */
	cm_el1_sysregs_context_save(SECURE);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_save(SECURE);
#endif

	return rc;
}

/*******************************************************************************
 * This function returns to the place where spmd_spm_core_sync_entry() was
 * called originally.
 ******************************************************************************/
__dead2 void spmd_spm_core_sync_exit(uint64_t rc)
{
	spmd_spm_core_context_t *ctx = spmd_get_context();

	/* Get current CPU context from SPMC context */
	assert(cm_get_context(SECURE) == &(ctx->cpu_ctx));

	/*
	 * The SPMD must have initiated the original request through a
	 * synchronous entry into SPMC. Jump back to the original C runtime
	 * context with the value of rc in x0;
	 */
	spmd_spm_core_exit(ctx->c_rt_ctx, rc);

	panic();
}

/*******************************************************************************
 * Jump to the SPM Core for the first time.
 ******************************************************************************/
static int32_t spmd_init(void)
{
	spmd_spm_core_context_t *ctx = spmd_get_context();
	uint64_t rc;
	unsigned int linear_id = plat_my_core_pos();
	unsigned int core_id;

	VERBOSE("SPM Core init start.\n");
	ctx->state = SPMC_STATE_ON_PENDING;

	/* Set the SPMC context state on other CPUs to OFF */
	for (core_id = 0U; core_id < PLATFORM_CORE_COUNT; core_id++) {
		if (core_id != linear_id) {
			spm_core_context[core_id].state = SPMC_STATE_OFF;
		}
	}

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc != 0ULL) {
		ERROR("SPMC initialisation failed 0x%llx\n", rc);
		return 0;
	}

	ctx->state = SPMC_STATE_ON;

	VERBOSE("SPM Core init end.\n");

	return 1;
}

/*******************************************************************************
 * Loads SPMC manifest and inits SPMC.
 ******************************************************************************/
static int spmd_spmc_init(void *pm_addr)
{
	spmd_spm_core_context_t *spm_ctx = spmd_get_context();
	uint32_t ep_attr;
	int rc;

	/* Load the SPM Core manifest */
	rc = plat_spm_core_manifest_load(&spmc_attrs, pm_addr);
	if (rc != 0) {
		WARN("No or invalid SPM Core manifest image provided by BL2\n");
		return rc;
	}

	/*
	 * Ensure that the SPM Core version is compatible with the SPM
	 * Dispatcher version.
	 */
	if ((spmc_attrs.major_version != FFA_VERSION_MAJOR) ||
	    (spmc_attrs.minor_version > FFA_VERSION_MINOR)) {
		WARN("Unsupported FFA version (%u.%u)\n",
		     spmc_attrs.major_version, spmc_attrs.minor_version);
		return -EINVAL;
	}

	VERBOSE("FFA version (%u.%u)\n", spmc_attrs.major_version,
	     spmc_attrs.minor_version);

	VERBOSE("SPM Core run time EL%x.\n",
	     SPMD_SPM_AT_SEL2 ? MODE_EL2 : MODE_EL1);

	/* Validate the SPMC ID, Ensure high bit is set */
	if (((spmc_attrs.spmc_id >> SPMC_SECURE_ID_SHIFT) &
			SPMC_SECURE_ID_MASK) == 0U) {
		WARN("Invalid ID (0x%x) for SPMC.\n", spmc_attrs.spmc_id);
		return -EINVAL;
	}

	/* Validate the SPM Core execution state */
	if ((spmc_attrs.exec_state != MODE_RW_64) &&
	    (spmc_attrs.exec_state != MODE_RW_32)) {
		WARN("Unsupported %s%x.\n", "SPM Core execution state 0x",
		     spmc_attrs.exec_state);
		return -EINVAL;
	}

	VERBOSE("%s%x.\n", "SPM Core execution state 0x",
		spmc_attrs.exec_state);

#if SPMD_SPM_AT_SEL2
	/* Ensure manifest has not requested AArch32 state in S-EL2 */
	if (spmc_attrs.exec_state == MODE_RW_32) {
		WARN("AArch32 state at S-EL2 is not supported.\n");
		return -EINVAL;
	}

	/*
	 * Check if S-EL2 is supported on this system if S-EL2
	 * is required for SPM
	 */
	if (!is_armv8_4_sel2_present()) {
		WARN("SPM Core run time S-EL2 is not supported.\n");
		return -EINVAL;
	}
#endif /* SPMD_SPM_AT_SEL2 */

	/* Initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if ((read_sctlr_el3() & SCTLR_EE_BIT) != 0ULL) {
		ep_attr |= EP_EE_BIG;
	}

	SET_PARAM_HEAD(spmc_ep_info, PARAM_EP, VERSION_1, ep_attr);

	/*
	 * Populate SPSR for SPM Core based upon validated parameters from the
	 * manifest.
	 */
	if (spmc_attrs.exec_state == MODE_RW_32) {
		spmc_ep_info->spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
						 SPSR_E_LITTLE,
						 DAIF_FIQ_BIT |
						 DAIF_IRQ_BIT |
						 DAIF_ABT_BIT);
	} else {

#if SPMD_SPM_AT_SEL2
		static const uint32_t runtime_el = MODE_EL2;
#else
		static const uint32_t runtime_el = MODE_EL1;
#endif
		spmc_ep_info->spsr = SPSR_64(runtime_el,
					     MODE_SP_ELX,
					     DISABLE_ALL_EXCEPTIONS);
	}

	/* Initialise SPM Core context with this entry point information */
	cm_setup_context(&spm_ctx->cpu_ctx, spmc_ep_info);

	/* Reuse PSCI affinity states to mark this SPMC context as off */
	spm_ctx->state = AFF_STATE_OFF;

	INFO("SPM Core setup done.\n");

	/* Register power management hooks with PSCI */
	psci_register_spd_pm_hook(&spmd_pm);

	/* Register init function for deferred init. */
	bl31_register_bl32_init(&spmd_init);

	return 0;
}

/*******************************************************************************
 * Initialize context of SPM Core.
 ******************************************************************************/
int spmd_setup(void)
{
	void *spmc_manifest;
	int rc;

	spmc_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (spmc_ep_info == NULL) {
		WARN("No SPM Core image provided by BL2 boot loader.\n");
		return -EINVAL;
	}

	/* Under no circumstances will this parameter be 0 */
	assert(spmc_ep_info->pc != 0ULL);

	/*
	 * Check if BL32 ep_info has a reference to 'tos_fw_config'. This will
	 * be used as a manifest for the SPM Core at the next lower EL/mode.
	 */
	spmc_manifest = (void *)spmc_ep_info->args.arg0;
	if (spmc_manifest == NULL) {
		ERROR("Invalid or absent SPM Core manifest.\n");
		return -EINVAL;
	}

	/* Load manifest, init SPMC */
	rc = spmd_spmc_init(spmc_manifest);
	if (rc != 0) {
		WARN("Booting device without SPM initialization.\n");
	}

	return rc;
}

/*******************************************************************************
 * Forward SMC to the other security state
 ******************************************************************************/
static uint64_t spmd_smc_forward(uint32_t smc_fid,
				 bool secure_origin,
				 uint64_t x1,
				 uint64_t x2,
				 uint64_t x3,
				 uint64_t x4,
				 void *handle)
{
	unsigned int secure_state_in = (secure_origin) ? SECURE : NON_SECURE;
	unsigned int secure_state_out = (!secure_origin) ? SECURE : NON_SECURE;

	/* Save incoming security state */
	cm_el1_sysregs_context_save(secure_state_in);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_save(secure_state_in);
#endif

	/* Restore outgoing security state */
	cm_el1_sysregs_context_restore(secure_state_out);
#if SPMD_SPM_AT_SEL2
	cm_el2_sysregs_context_restore(secure_state_out);
#endif
	cm_set_next_eret_context(secure_state_out);

	SMC_RET8(cm_get_context(secure_state_out), smc_fid, x1, x2, x3, x4,
			SMC_GET_GP(handle, CTX_GPREG_X5),
			SMC_GET_GP(handle, CTX_GPREG_X6),
			SMC_GET_GP(handle, CTX_GPREG_X7));
}

/*******************************************************************************
 * Return FFA_ERROR with specified error code
 ******************************************************************************/
static uint64_t spmd_ffa_error_return(void *handle, int error_code)
{
	SMC_RET8(handle, (uint32_t) FFA_ERROR,
		 FFA_TARGET_INFO_MBZ, (uint32_t)error_code,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
		 FFA_PARAM_MBZ, FFA_PARAM_MBZ);
}

/*******************************************************************************
 * spmd_check_address_in_binary_image
 ******************************************************************************/
bool spmd_check_address_in_binary_image(uint64_t address)
{
	assert(!check_uptr_overflow(spmc_attrs.load_address, spmc_attrs.binary_size));

	return ((address >= spmc_attrs.load_address) &&
		(address < (spmc_attrs.load_address + spmc_attrs.binary_size)));
}

/******************************************************************************
 * spmd_is_spmc_message
 *****************************************************************************/
static bool spmd_is_spmc_message(unsigned int ep)
{
	return ((ffa_endpoint_destination(ep) == SPMD_DIRECT_MSG_ENDPOINT_ID)
		&& (ffa_endpoint_source(ep) == spmc_attrs.spmc_id));
}

/******************************************************************************
 * spmd_handle_spmc_message
 *****************************************************************************/
static int spmd_handle_spmc_message(unsigned long long msg,
		unsigned long long parm1, unsigned long long parm2,
		unsigned long long parm3, unsigned long long parm4)
{
	VERBOSE("%s %llx %llx %llx %llx %llx\n", __func__,
		msg, parm1, parm2, parm3, parm4);

	return -EINVAL;
}

/*******************************************************************************
 * This function handles all SMCs in the range reserved for FFA. Each call is
 * either forwarded to the other security state or handled by the SPM dispatcher
 ******************************************************************************/
uint64_t spmd_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags)
{
	unsigned int linear_id = plat_my_core_pos();
	spmd_spm_core_context_t *ctx = spmd_get_context();
	bool secure_origin;
	int32_t ret;
	uint32_t input_version;

	/* Determine which security state this SMC originated from */
	secure_origin = is_caller_secure(flags);

	VERBOSE("SPM(%u): 0x%x 0x%llx 0x%llx 0x%llx 0x%llx "
		"0x%llx 0x%llx 0x%llx\n",
		linear_id, smc_fid, x1, x2, x3, x4,
		SMC_GET_GP(handle, CTX_GPREG_X5),
		SMC_GET_GP(handle, CTX_GPREG_X6),
		SMC_GET_GP(handle, CTX_GPREG_X7));

	switch (smc_fid) {
	case FFA_ERROR:
		/*
		 * Check if this is the first invocation of this interface on
		 * this CPU. If so, then indicate that the SPM Core initialised
		 * unsuccessfully.
		 */
		if (secure_origin && (ctx->state == SPMC_STATE_ON_PENDING)) {
			spmd_spm_core_sync_exit(x2);
		}

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, handle);
		break; /* not reached */

	case FFA_VERSION:
		input_version = (uint32_t)(0xFFFFFFFF & x1);
		/*
		 * If caller is secure and SPMC was initialized,
		 * return FFA_VERSION of SPMD.
		 * If caller is non secure and SPMC was initialized,
		 * return SPMC's version.
		 * Sanity check to "input_version".
		 */
		if ((input_version & FFA_VERSION_BIT31_MASK) ||
			(ctx->state == SPMC_STATE_RESET)) {
			ret = FFA_ERROR_NOT_SUPPORTED;
		} else if (!secure_origin) {
			ret = MAKE_FFA_VERSION(spmc_attrs.major_version,
					       spmc_attrs.minor_version);
		} else {
			ret = MAKE_FFA_VERSION(FFA_VERSION_MAJOR,
					       FFA_VERSION_MINOR);
		}

		SMC_RET8(handle, (uint32_t)ret, FFA_TARGET_INFO_MBZ,
			 FFA_TARGET_INFO_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ,
			 FFA_PARAM_MBZ, FFA_PARAM_MBZ, FFA_PARAM_MBZ);
		break; /* not reached */

	case FFA_FEATURES:
		/*
		 * This is an optional interface. Do the minimal checks and
		 * forward to SPM Core which will handle it if implemented.
		 */

		/*
		 * Check if x1 holds a valid FFA fid. This is an
		 * optimization.
		 */
		if (!is_ffa_fid(x1)) {
			return spmd_ffa_error_return(handle,
						     FFA_ERROR_NOT_SUPPORTED);
		}

		/* Forward SMC from Normal world to the SPM Core */
		if (!secure_origin) {
			return spmd_smc_forward(smc_fid, secure_origin,
						x1, x2, x3, x4, handle);
		}

		/*
		 * Return success if call was from secure world i.e. all
		 * FFA functions are supported. This is essentially a
		 * nop.
		 */
		SMC_RET8(handle, FFA_SUCCESS_SMC32, x1, x2, x3, x4,
			 SMC_GET_GP(handle, CTX_GPREG_X5),
			 SMC_GET_GP(handle, CTX_GPREG_X6),
			 SMC_GET_GP(handle, CTX_GPREG_X7));

		break; /* not reached */

	case FFA_ID_GET:
		/*
		 * Returns the ID of the calling FFA component.
		 */
		if (!secure_origin) {
			SMC_RET8(handle, FFA_SUCCESS_SMC32,
				 FFA_TARGET_INFO_MBZ, FFA_NS_ENDPOINT_ID,
				 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				 FFA_PARAM_MBZ);
		}

		SMC_RET8(handle, FFA_SUCCESS_SMC32,
			 FFA_TARGET_INFO_MBZ, spmc_attrs.spmc_id,
			 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
			 FFA_PARAM_MBZ, FFA_PARAM_MBZ,
			 FFA_PARAM_MBZ);

		break; /* not reached */

	case FFA_SECONDARY_EP_REGISTER_SMC64:
		if (secure_origin) {
			ret = spmd_pm_secondary_ep_register(x1);

			if (ret < 0) {
				SMC_RET8(handle, FFA_ERROR_SMC64,
					FFA_TARGET_INFO_MBZ, ret,
					FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ);
			} else {
				SMC_RET8(handle, FFA_SUCCESS_SMC64,
					FFA_TARGET_INFO_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ, FFA_PARAM_MBZ,
					FFA_PARAM_MBZ);
			}
		}

		return spmd_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
		break; /* Not reached */

	case FFA_MSG_SEND_DIRECT_REQ_SMC32:
		if (secure_origin && spmd_is_spmc_message(x1)) {
			ret = spmd_handle_spmc_message(x3, x4,
				SMC_GET_GP(handle, CTX_GPREG_X5),
				SMC_GET_GP(handle, CTX_GPREG_X6),
				SMC_GET_GP(handle, CTX_GPREG_X7));

			SMC_RET8(handle, FFA_SUCCESS_SMC32,
				FFA_TARGET_INFO_MBZ, ret,
				FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				FFA_PARAM_MBZ, FFA_PARAM_MBZ,
				FFA_PARAM_MBZ);
		} else {
			/* Forward direct message to the other world */
			return spmd_smc_forward(smc_fid, secure_origin,
				x1, x2, x3, x4, handle);
		}
		break; /* Not reached */

	case FFA_MSG_SEND_DIRECT_RESP_SMC32:
		if (secure_origin && spmd_is_spmc_message(x1)) {
			spmd_spm_core_sync_exit(0);
		} else {
			/* Forward direct message to the other world */
			return spmd_smc_forward(smc_fid, secure_origin,
				x1, x2, x3, x4, handle);
		}
		break; /* Not reached */

	case FFA_RX_RELEASE:
	case FFA_RXTX_MAP_SMC32:
	case FFA_RXTX_MAP_SMC64:
	case FFA_RXTX_UNMAP:
	case FFA_PARTITION_INFO_GET:
		/*
		 * Should not be allowed to forward FFA_PARTITION_INFO_GET
		 * from Secure world to Normal world
		 *
		 * Fall through to forward the call to the other world
		 */
	case FFA_MSG_RUN:
		/* This interface must be invoked only by the Normal world */

		if (secure_origin) {
			return spmd_ffa_error_return(handle,
						     FFA_ERROR_NOT_SUPPORTED);
		}

		/* Fall through to forward the call to the other world */
	case FFA_MSG_SEND:
	case FFA_MSG_SEND_DIRECT_REQ_SMC64:
	case FFA_MSG_SEND_DIRECT_RESP_SMC64:
	case FFA_MEM_DONATE_SMC32:
	case FFA_MEM_DONATE_SMC64:
	case FFA_MEM_LEND_SMC32:
	case FFA_MEM_LEND_SMC64:
	case FFA_MEM_SHARE_SMC32:
	case FFA_MEM_SHARE_SMC64:
	case FFA_MEM_RETRIEVE_REQ_SMC32:
	case FFA_MEM_RETRIEVE_REQ_SMC64:
	case FFA_MEM_RETRIEVE_RESP:
	case FFA_MEM_RELINQUISH:
	case FFA_MEM_RECLAIM:
	case FFA_SUCCESS_SMC32:
	case FFA_SUCCESS_SMC64:
		/*
		 * TODO: Assume that no requests originate from EL3 at the
		 * moment. This will change if a SP service is required in
		 * response to secure interrupts targeted to EL3. Until then
		 * simply forward the call to the Normal world.
		 */

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, handle);
		break; /* not reached */

	case FFA_MSG_WAIT:
		/*
		 * Check if this is the first invocation of this interface on
		 * this CPU from the Secure world. If so, then indicate that the
		 * SPM Core initialised successfully.
		 */
		if (secure_origin && (ctx->state == SPMC_STATE_ON_PENDING)) {
			spmd_spm_core_sync_exit(0);
		}

		/* Fall through to forward the call to the other world */
	case FFA_INTERRUPT:
	case FFA_MSG_YIELD:
		/* This interface must be invoked only by the Secure world */
		if (!secure_origin) {
			return spmd_ffa_error_return(handle,
						      FFA_ERROR_NOT_SUPPORTED);
		}

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, handle);
		break; /* not reached */

	default:
		WARN("SPM: Unsupported call 0x%08x\n", smc_fid);
		return spmd_ffa_error_return(handle, FFA_ERROR_NOT_SUPPORTED);
	}
}
