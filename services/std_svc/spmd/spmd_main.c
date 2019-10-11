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
spmc_manifest_sect_attribute_t spmc_attrs;

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
	cm_set_next_eret_context(SECURE);

	/* Invalidate TLBs at EL1. */
	tlbivmalle1();
	dsbish();

	/* Enter Secure Partition */
	rc = spmd_spm_core_enter(&spmc_ctx->c_rt_ctx);

	/* Save secure state */
	cm_el1_sysregs_context_save(SECURE);

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
 * Initialize context of SPM core.
 ******************************************************************************/
int32_t spmd_setup(void)
{
	int rc;
	void *rd_base;
	size_t rd_size;
	entry_point_info_t *spmc_ep_info;
	uintptr_t rd_base_align;
	uintptr_t rd_size_align;
	uint32_t ep_attr;

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
	if (rc < 0) {
		ERROR("Error while mapping SPM core manifest (%d).\n", rc);
		panic();
	}

	/* Load the SPM core manifest */
	rc = plat_spm_core_manifest_load(&spmc_attrs, rd_base, rd_size);
	if (rc < 0) {
		WARN("No or invalid SPM core manifest image provided by BL2 "
		     "boot loader. ");
		goto error;
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
		goto error;
	}

	INFO("SPCI version (%x.%x).\n", spmc_attrs.major_version,
	     spmc_attrs.minor_version);

	/* Validate the SPM core runtime EL */
	if ((spmc_attrs.runtime_el != MODE_EL1) &&
	    (spmc_attrs.runtime_el != MODE_EL2)) {
		WARN("Unsupported SPM core run time EL%x specified in "
		     "manifest image provided by BL2 boot loader.\n",
		     spmc_attrs.runtime_el);
		goto error;
	}

	INFO("SPM core run time EL%x.\n", spmc_attrs.runtime_el);

	/* Validate the SPM core execution state */
	if ((spmc_attrs.exec_state != MODE_RW_64) &&
	    (spmc_attrs.exec_state != MODE_RW_32)) {
		WARN("Unsupported SPM core execution state %x specified in "
		     "manifest image provided by BL2 boot loader.\n",
		     spmc_attrs.exec_state);
		goto error;
	}

	INFO("SPM core execution state %x.\n", spmc_attrs.exec_state);

	/* Ensure manifest has not requested S-EL2 in AArch32 state */
	if ((spmc_attrs.exec_state == MODE_RW_32) &&
	    (spmc_attrs.runtime_el == MODE_EL2)) {
		WARN("Invalid combination of SPM core execution state (%x) "
		     "and run time EL (%x).\n", spmc_attrs.exec_state,
		     spmc_attrs.runtime_el);
		goto error;
	}

	/*
	 * Check if S-EL2 is supported on this system if S-EL2
	 * is required for SPM
	 */
	if (spmc_attrs.runtime_el == MODE_EL2) {
		uint64_t sel2 = read_id_aa64pfr0_el1();

		sel2 >>= ID_AA64PFR0_SEL2_SHIFT;
		sel2 &= ID_AA64PFR0_SEL2_MASK;

		if (!sel2) {
			WARN("SPM core run time EL: S-EL%x is not supported "
			     "but specified in manifest image provided by "
			     "BL2 boot loader.\n", spmc_attrs.runtime_el);
			goto error;
		}
	}

	/* Initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if (read_sctlr_el3() & SCTLR_EE_BIT)
		ep_attr |= EP_EE_BIG;
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
		spmc_ep_info->spsr = SPSR_64(spmc_attrs.runtime_el,
					     MODE_SP_ELX,
					     DISABLE_ALL_EXCEPTIONS);
	}

	/* Initialise SPM core context with this entry point information */
	cm_setup_context(&(spm_core_context[plat_my_core_pos()].cpu_ctx),
			 spmc_ep_info);

	INFO("SPM core setup done.\n");

	/* Register init function for deferred init.  */
	bl31_register_bl32_init(&spmd_init);

	return 0;

error:
	WARN("Booting device without SPM initialization. "
	     "SPCI SMCs destined for SPM core will return "
	     "ENOTSUPPORTED\n");

	rc = mmap_remove_dynamic_region(rd_base_align, rd_size_align);
	if (rc < 0) {
		ERROR("Error while unmapping SPM core manifest (%d).\n",
		      rc);
		panic();
	}

	return 1;
}

/*******************************************************************************
 * This function handles all SMCs in the range reserved for SPCI. Each call is
 * either forwarded to the other security state or handled by the SPM dispatcher
 ******************************************************************************/
uint64_t spmd_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			  uint64_t x3, uint64_t x4, void *cookie, void *handle,
			  uint64_t flags)
{
	uint32_t in_sstate;
	uint32_t out_sstate;
	int32_t ret;
	spmd_spm_core_context_t *ctx = &spm_core_context[plat_my_core_pos()];

	/* Determine which security state this SMC originated from */
	if (is_caller_secure(flags)) {
		in_sstate = SECURE;
		out_sstate = NON_SECURE;
	} else {
		in_sstate = NON_SECURE;
		out_sstate = SECURE;
	}

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
		if ((in_sstate == SECURE) && (ctx->state == SPMC_STATE_RESET))
			spmd_spm_core_sync_exit(x2);

		/* Save incoming security state */
		cm_el1_sysregs_context_save(in_sstate);

		/* Restore outgoing security state */
		cm_el1_sysregs_context_restore(out_sstate);
		cm_set_next_eret_context(out_sstate);

		SMC_RET8(cm_get_context(out_sstate), smc_fid, x1, x2, x3, x4,
			 SMC_GET_GP(handle, CTX_GPREG_X5),
			 SMC_GET_GP(handle, CTX_GPREG_X6),
			 SMC_GET_GP(handle, CTX_GPREG_X7));
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
		 * Check if w1 holds a valid SPCI fid. This is an
		 * optimization.
		 */
		if (!is_spci_fid(x1))
			SMC_RET8(handle, SPCI_ERROR,
				 SPCI_TARGET_INFO_MBZ, SPCI_ERROR_NOT_SUPPORTED,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ);

		/* Forward SMC from Normal world to the SPM core */
		if (in_sstate == NON_SECURE) {
			/* Save incoming security state */
			cm_el1_sysregs_context_save(in_sstate);

			/* Restore outgoing security state */
			cm_el1_sysregs_context_restore(out_sstate);
			cm_set_next_eret_context(out_sstate);

			SMC_RET8(cm_get_context(out_sstate), smc_fid,
				 x1, x2, x3, x4,
				 SMC_GET_GP(handle, CTX_GPREG_X5),
				 SMC_GET_GP(handle, CTX_GPREG_X6),
				 SMC_GET_GP(handle, CTX_GPREG_X7));
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

	case SPCI_RX_RELEASE:
	case SPCI_RXTX_MAP_SMC32:
	case SPCI_RXTX_MAP_SMC64:
	case SPCI_RXTX_UNMAP:
	case SPCI_MSG_RUN:
		/* This interface must be invoked only by the Normal world */
		if (in_sstate == SECURE) {
			SMC_RET8(handle, SPCI_ERROR,
				 SPCI_TARGET_INFO_MBZ, SPCI_ERROR_NOT_SUPPORTED,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ);
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

		/* Save incoming security state */
		cm_el1_sysregs_context_save(in_sstate);

		/* Restore outgoing security state */
		cm_el1_sysregs_context_restore(out_sstate);
		cm_set_next_eret_context(out_sstate);

		SMC_RET8(cm_get_context(out_sstate), smc_fid, x1, x2, x3, x4,
			 SMC_GET_GP(handle, CTX_GPREG_X5),
			 SMC_GET_GP(handle, CTX_GPREG_X6),
			 SMC_GET_GP(handle, CTX_GPREG_X7));
		break; /* not reached */

	case SPCI_MSG_WAIT:
		/*
		 * Check if this is the first invocation of this interface on
		 * this CPU from the Secure world. If so, then indicate that the
		 * SPM core initialised successfully.
		 */
		if ((in_sstate == SECURE) && (ctx->state == SPMC_STATE_RESET)) {
			spmd_spm_core_sync_exit(0);
		}

		/* Intentional fall-through */

	case SPCI_MSG_YIELD:
		/* This interface must be invoked only by the Secure world */
		if (in_sstate == NON_SECURE) {
			SMC_RET8(handle, SPCI_ERROR,
				 SPCI_TARGET_INFO_MBZ, SPCI_ERROR_NOT_SUPPORTED,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
				 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ);
		}

		/* Save incoming security state */
		cm_el1_sysregs_context_save(in_sstate);

		/* Restore outgoing security state */
		cm_el1_sysregs_context_restore(out_sstate);
		cm_set_next_eret_context(out_sstate);

		SMC_RET8(cm_get_context(out_sstate), smc_fid, x1, x2, x3, x4,
			 SMC_GET_GP(handle, CTX_GPREG_X5),
			 SMC_GET_GP(handle, CTX_GPREG_X6),
			 SMC_GET_GP(handle, CTX_GPREG_X7));
		break; /* not reached */

	default:
		WARN("SPM: Unsupported call 0x%08x\n", smc_fid);
		SMC_RET8(handle, SPCI_ERROR,
			 SPCI_TARGET_INFO_MBZ, SPCI_ERROR_NOT_SUPPORTED,
			 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ, SPCI_PARAM_MBZ,
			 SPCI_PARAM_MBZ, SPCI_PARAM_MBZ);
	}
}
