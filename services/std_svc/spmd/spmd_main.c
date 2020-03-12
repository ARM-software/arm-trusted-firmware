/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <platform_def.h>
#include <services/spci_svc.h>
#include <services/spmd_svc.h>
#include <smccc_helpers.h>
#include "spmd_private.h"

/*******************************************************************************
 * SPM Core context information.
 ******************************************************************************/
spmd_spm_core_context_t spm_core_context[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * SPM Core attribute information read from its manifest.
 ******************************************************************************/
static spmc_manifest_sect_attribute_t spmc_attrs;

/*******************************************************************************
 * SPM Core entry point information. Discovered on the primary core and reused
 * on secondary cores.
 ******************************************************************************/
static entry_point_info_t *spmc_ep_info;

/*******************************************************************************
 * Static function declaration.
 ******************************************************************************/
static int32_t	spmd_init(void);
static int	spmd_spmc_init(void *rd_base, size_t rd_size);
static uint64_t	spmd_spci_error_return(void *handle, int error_code);
static uint64_t	spmd_smc_forward(uint32_t smc_fid, bool secure_origin,
				 uint64_t x1, uint64_t x2, uint64_t x3,
				 uint64_t x4, void *handle);

/*******************************************************************************
 * This function takes an SP context pointer and performs a synchronous entry
 * into it.
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
 * This function returns to the place where spm_sp_synchronous_entry() was
 * called originally.
 ******************************************************************************/
__dead2 void spmd_spm_core_sync_exit(uint64_t rc)
{
	spmd_spm_core_context_t *ctx = &spm_core_context[plat_my_core_pos()];

	/* Get context of the SP in use by this CPU. */
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
 * Jump to the SPM core for the first time.
 ******************************************************************************/
static int32_t spmd_init(void)
{
	uint64_t rc = 0;
	spmd_spm_core_context_t *ctx = &spm_core_context[plat_my_core_pos()];

	INFO("SPM Core init start.\n");
	ctx->state = SPMC_STATE_RESET;

	rc = spmd_spm_core_sync_entry(ctx);
	if (rc) {
		ERROR("SPMC initialisation failed 0x%llx\n", rc);
		panic();
	}

	ctx->state = SPMC_STATE_IDLE;
	INFO("SPM Core init end.\n");

	return 1;
}

/*******************************************************************************
 * Load SPMC manifest, init SPMC.
 ******************************************************************************/
static int spmd_spmc_init(void *rd_base, size_t rd_size)
{
	int rc;
	uint32_t ep_attr;
	unsigned int linear_id = plat_my_core_pos();
	spmd_spm_core_context_t *spm_ctx = &spm_core_context[linear_id];

	/* Load the SPM core manifest */
	rc = plat_spm_core_manifest_load(&spmc_attrs, rd_base, rd_size);
	if (rc != 0) {
		WARN("No or invalid SPM core manifest image provided by BL2 "
		     "boot loader. ");
		return 1;
	}

	/*
	 * Ensure that the SPM core version is compatible with the SPM
	 * dispatcher version
	 */
	if ((spmc_attrs.major_version != SPCI_VERSION_MAJOR) ||
	    (spmc_attrs.minor_version > SPCI_VERSION_MINOR)) {
		WARN("Unsupported SPCI version (%x.%x) specified in SPM core "
		     "manifest image provided by BL2 boot loader.\n",
		     spmc_attrs.major_version, spmc_attrs.minor_version);
		return 1;
	}

	INFO("SPCI version (%x.%x).\n", spmc_attrs.major_version,
	     spmc_attrs.minor_version);

	INFO("SPM core run time EL%x.\n",
	     SPMD_SPM_AT_SEL2 ? MODE_EL2 : MODE_EL1);

	/* Validate the SPMC ID, Ensure high bit is set */
	if (!(spmc_attrs.spmc_id >> SPMC_SECURE_ID_SHIFT) &
			SPMC_SECURE_ID_MASK) {
		WARN("Invalid ID (0x%x) for SPMC.\n",
		     spmc_attrs.spmc_id);
		return 1;
	}

	INFO("SPMC ID %x.\n", spmc_attrs.spmc_id);

	/* Validate the SPM core execution state */
	if ((spmc_attrs.exec_state != MODE_RW_64) &&
	    (spmc_attrs.exec_state != MODE_RW_32)) {
		WARN("Unsupported SPM core execution state %x specified in "
		     "manifest image provided by BL2 boot loader.\n",
		     spmc_attrs.exec_state);
		return 1;
	}

	INFO("SPM core execution state %x.\n", spmc_attrs.exec_state);

#if SPMD_SPM_AT_SEL2
	/* Ensure manifest has not requested AArch32 state in S-EL2 */
	if (spmc_attrs.exec_state == MODE_RW_32) {
		WARN("AArch32 state at S-EL2 is not supported.\n");
		return 1;
	}

	/*
	 * Check if S-EL2 is supported on this system if S-EL2
	 * is required for SPM
	 */
	uint64_t sel2 = read_id_aa64pfr0_el1();

	sel2 >>= ID_AA64PFR0_SEL2_SHIFT;
	sel2 &= ID_AA64PFR0_SEL2_MASK;

	if (!sel2) {
		WARN("SPM core run time S-EL2 is not supported.");
		return 1;
	}
#endif /* SPMD_SPM_AT_SEL2 */

	/* Initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if (read_sctlr_el3() & SCTLR_EE_BIT) {
		ep_attr |= EP_EE_BIG;
	}

	SET_PARAM_HEAD(spmc_ep_info, PARAM_EP, VERSION_1, ep_attr);
	assert(spmc_ep_info->pc == BL32_BASE);

	/*
	 * Populate SPSR for SPM core based upon validated parameters from the
	 * manifest
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

	/* Initialise SPM core context with this entry point information */
	cm_setup_context(&spm_ctx->cpu_ctx, spmc_ep_info);

	/* Reuse PSCI affinity states to mark this SPMC context as off */
	spm_ctx->state = AFF_STATE_OFF;

	INFO("SPM core setup done.\n");

	/* Register init function for deferred init.  */
	bl31_register_bl32_init(&spmd_init);

	return 0;
}

/*******************************************************************************
 * Initialize context of SPM core.
 ******************************************************************************/
int spmd_setup(void)
{
	int rc;
	void *rd_base;
	size_t rd_size;
	uintptr_t rd_base_align;
	uintptr_t rd_size_align;

	spmc_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!spmc_ep_info) {
		WARN("No SPM core image provided by BL2 boot loader, Booting "
		     "device without SP initialization. SMC`s destined for SPM "
		     "core will return SMC_UNK\n");
		return 1;
	}

	/* Under no circumstances will this parameter be 0 */
	assert(spmc_ep_info->pc != 0U);

	/*
	 * Check if BL32 ep_info has a reference to 'tos_fw_config'. This will
	 * be used as a manifest for the SPM core at the next lower EL/mode.
	 */
	if (spmc_ep_info->args.arg0 == 0U || spmc_ep_info->args.arg2 == 0U) {
		ERROR("Invalid or absent SPM core manifest\n");
		panic();
	}

	/* Obtain whereabouts of SPM core manifest */
	rd_base = (void *) spmc_ep_info->args.arg0;
	rd_size = spmc_ep_info->args.arg2;

	rd_base_align = page_align((uintptr_t) rd_base, DOWN);
	rd_size_align = page_align((uintptr_t) rd_size, UP);

	/* Map the manifest in the SPMD translation regime first */
	VERBOSE("SPM core manifest base : 0x%lx\n", rd_base_align);
	VERBOSE("SPM core manifest size : 0x%lx\n", rd_size_align);
	rc = mmap_add_dynamic_region((unsigned long long) rd_base_align,
				     (uintptr_t) rd_base_align,
				     rd_size_align,
				     MT_RO_DATA);
	if (rc != 0) {
		ERROR("Error while mapping SPM core manifest (%d).\n", rc);
		panic();
	}

	/* Load manifest, init SPMC */
	rc = spmd_spmc_init(rd_base, rd_size);
	if (rc != 0) {
		int mmap_rc;

		WARN("Booting device without SPM initialization. "
		     "SPCI SMCs destined for SPM core will return "
		     "ENOTSUPPORTED\n");

		mmap_rc = mmap_remove_dynamic_region(rd_base_align,
						     rd_size_align);
		if (mmap_rc != 0) {
			ERROR("Error while unmapping SPM core manifest (%d).\n",
			      mmap_rc);
			panic();
		}

		return rc;
	}

	return 0;
}

/*******************************************************************************
 * Forward SMC to the other security state
 ******************************************************************************/
static uint64_t spmd_smc_forward(uint32_t smc_fid, bool secure_origin,
				 uint64_t x1, uint64_t x2, uint64_t x3,
				 uint64_t x4, void *handle)
{
	uint32_t secure_state_in = (secure_origin) ? SECURE : NON_SECURE;
	uint32_t secure_state_out = (!secure_origin) ? SECURE : NON_SECURE;

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
 * Return SPCI_ERROR with specified error code
 ******************************************************************************/
static uint64_t spmd_spci_error_return(void *handle, int error_code)
{
	SMC_RET8(handle, SPCI_ERROR,
		 SPCI_TARGET_INFO_MBZ, error_code,
		 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
		 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ);
}

/*******************************************************************************
 * This function handles all SMCs in the range reserved for SPCI. Each call is
 * either forwarded to the other security state or handled by the SPM dispatcher
 ******************************************************************************/
uint64_t spmd_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			  uint64_t x3, uint64_t x4, void *cookie, void *handle,
			  uint64_t flags)
{
	spmd_spm_core_context_t *ctx = &spm_core_context[plat_my_core_pos()];
	bool secure_origin;
	int32_t ret;

	/* Determine which security state this SMC originated from */
	secure_origin = is_caller_secure(flags);

	INFO("SPM: 0x%x, 0x%llx, 0x%llx, 0x%llx, 0x%llx, "
	     "0x%llx, 0x%llx, 0x%llx\n",
	     smc_fid, x1, x2, x3, x4, SMC_GET_GP(handle, CTX_GPREG_X5),
	     SMC_GET_GP(handle, CTX_GPREG_X6),
	     SMC_GET_GP(handle, CTX_GPREG_X7));

	switch (smc_fid) {
	case SPCI_ERROR:
		/*
		 * Check if this is the first invocation of this interface on
		 * this CPU. If so, then indicate that the SPM core initialised
		 * unsuccessfully.
		 */
		if (secure_origin && (ctx->state == SPMC_STATE_RESET)) {
			spmd_spm_core_sync_exit(x2);
		}

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, handle);
		break; /* not reached */

	case SPCI_VERSION:
		/*
		 * TODO: This is an optimization that the version information
		 * provided by the SPM core manifest is returned by the SPM
		 * dispatcher. It might be a better idea to simply forward this
		 * call to the SPM core and wash our hands completely.
		 */
		ret = MAKE_SPCI_VERSION(spmc_attrs.major_version,
					spmc_attrs.minor_version);
		SMC_RET8(handle, SPCI_SUCCESS_SMC32, SPCI_TARGET_INFO_MBZ, ret,
			 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
			 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ);
		break; /* not reached */

	case SPCI_FEATURES:
		/*
		 * This is an optional interface. Do the minimal checks and
		 * forward to SPM core which will handle it if implemented.
		 */

		/*
		 * Check if x1 holds a valid SPCI fid. This is an
		 * optimization.
		 */
		if (!is_spci_fid(x1)) {
			return spmd_spci_error_return(handle,
						      SPCI_ERROR_NOT_SUPPORTED);
		}

		/* Forward SMC from Normal world to the SPM core */
		if (!secure_origin) {
			return spmd_smc_forward(smc_fid, secure_origin,
						x1, x2, x3, x4, handle);
		} else {
			/*
			 * Return success if call was from secure world i.e. all
			 * SPCI functions are supported. This is essentially a
			 * nop.
			 */
			SMC_RET8(handle, SPCI_SUCCESS_SMC32, x1, x2, x3, x4,
				 SMC_GET_GP(handle, CTX_GPREG_X5),
				 SMC_GET_GP(handle, CTX_GPREG_X6),
				 SMC_GET_GP(handle, CTX_GPREG_X7));
		}

		break; /* not reached */

	case SPCI_ID_GET:
		/*
		 * Returns the ID of the calling SPCI component.
		*/
		if (!secure_origin) {
			SMC_RET8(handle, SPCI_SUCCESS_SMC32,
				 SPCI_TARGET_INFO_MBZ, SPCI_NS_ENDPOINT_ID,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
				 SPCI_PARAM_MBZ);
		} else {
			SMC_RET8(handle, SPCI_SUCCESS_SMC32,
				 SPCI_TARGET_INFO_MBZ, spmc_attrs.spmc_id,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
				 SPCI_PARAM_MBZ);
		}

		break; /* not reached */

	case SPCI_RX_RELEASE:
	case SPCI_RXTX_MAP_SMC32:
	case SPCI_RXTX_MAP_SMC64:
	case SPCI_RXTX_UNMAP:
	case SPCI_MSG_RUN:
		/* This interface must be invoked only by the Normal world */
		if (secure_origin) {
			return spmd_spci_error_return(handle,
						      SPCI_ERROR_NOT_SUPPORTED);
		}

		/* Fall through to forward the call to the other world */

	case SPCI_PARTITION_INFO_GET:
	case SPCI_MSG_SEND:
	case SPCI_MSG_SEND_DIRECT_REQ_SMC32:
	case SPCI_MSG_SEND_DIRECT_REQ_SMC64:
	case SPCI_MSG_SEND_DIRECT_RESP_SMC32:
	case SPCI_MSG_SEND_DIRECT_RESP_SMC64:
	case SPCI_MEM_DONATE_SMC32:
	case SPCI_MEM_DONATE_SMC64:
	case SPCI_MEM_LEND_SMC32:
	case SPCI_MEM_LEND_SMC64:
	case SPCI_MEM_SHARE_SMC32:
	case SPCI_MEM_SHARE_SMC64:
	case SPCI_MEM_RETRIEVE_REQ_SMC32:
	case SPCI_MEM_RETRIEVE_REQ_SMC64:
	case SPCI_MEM_RETRIEVE_RESP:
	case SPCI_MEM_RELINQUISH:
	case SPCI_MEM_RECLAIM:
	case SPCI_SUCCESS_SMC32:
	case SPCI_SUCCESS_SMC64:
		/*
		 * TODO: Assume that no requests originate from EL3 at the
		 * moment. This will change if a SP service is required in
		 * response to secure interrupts targeted to EL3. Until then
		 * simply forward the call to the Normal world.
		 */

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, handle);
		break; /* not reached */

	case SPCI_MSG_WAIT:
		/*
		 * Check if this is the first invocation of this interface on
		 * this CPU from the Secure world. If so, then indicate that the
		 * SPM core initialised successfully.
		 */
		if (secure_origin && (ctx->state == SPMC_STATE_RESET)) {
			spmd_spm_core_sync_exit(0);
		}

		/* Fall through to forward the call to the other world */

	case SPCI_MSG_YIELD:
		/* This interface must be invoked only by the Secure world */
		if (!secure_origin) {
			return spmd_spci_error_return(handle,
						      SPCI_ERROR_NOT_SUPPORTED);
		}

		return spmd_smc_forward(smc_fid, secure_origin,
					x1, x2, x3, x4, handle);
		break; /* not reached */

	default:
		WARN("SPM: Unsupported call 0x%08x\n", smc_fid);
		return spmd_spci_error_return(handle, SPCI_ERROR_NOT_SUPPORTED);
	}
}
