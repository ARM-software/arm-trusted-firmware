/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


/*******************************************************************************
 * This is the Secure Payload Dispatcher (SPD). The dispatcher is meant to be a
 * plug-in component to the Secure Monitor, registered as a runtime service. The
 * SPD is expected to be a functional extension of the Secure Payload (SP) that
 * executes in Secure EL1. The Secure Monitor will delegate all SMCs targeting
 * the Trusted OS/Applications range to the dispatcher. The SPD will either
 * handle the request locally or delegate it to the Secure Payload. It is also
 * responsible for initialising and maintaining communication with the SP.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <bl31.h>
#include <tsp.h>
#include <psci.h>
#include <debug.h>
#include <uuid.h>
#include "tspd_private.h"

/*******************************************************************************
 * Single structure to hold information about the various entry points into the
 * Secure Payload. It is initialised once on the primary core after a cold boot.
 ******************************************************************************/
entry_info_t *tsp_entry_info;

/*******************************************************************************
 * Array to keep track of per-cpu Secure Payload state
 ******************************************************************************/
tsp_context_t tspd_sp_context[TSPD_CORE_COUNT];


/* TSP UID */
DEFINE_SVC_UUID(tsp_uuid,
		0x5b3056a0, 0x3291, 0x427b, 0x98, 0x11,
		0x71, 0x68, 0xca, 0x50, 0xf3, 0xfa);

int32_t tspd_init(meminfo_t *bl32_meminfo);


/*******************************************************************************
 * Secure Payload Dispatcher setup. The SPD finds out the SP entrypoint and type
 * (aarch32/aarch64) if not already known and initialises the context for entry
 * into the SP for its initialisation.
 ******************************************************************************/
int32_t tspd_setup(void)
{
	el_change_info_t *image_info;
	int32_t rc;
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id;

	linear_id = platform_get_core_pos(mpidr);

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.  TODO: Add support to
	 * conditionally include the SPD service
	 */
	image_info = bl31_get_next_image_info(SECURE);
	assert(image_info);

	/*
	 * If there's no valid entry point for SP, we return a non-zero value
	 * signalling failure initializing the service. We bail out without
	 * registering any handlers
	 */
	if (!image_info->entrypoint)
		return 1;

	/*
	 * We could inspect the SP image and determine it's execution
	 * state i.e whether AArch32 or AArch64. Assuming it's AArch64
	 * for the time being.
	 */
	rc = tspd_init_secure_context(image_info->entrypoint,
				     TSP_AARCH64,
				     mpidr,
				     &tspd_sp_context[linear_id]);
	assert(rc == 0);

	/*
	 * All TSPD initialization done. Now register our init function with
	 * BL31 for deferred invocation
	 */
	bl31_register_bl32_init(&tspd_init);

	return rc;
}

/*******************************************************************************
 * This function passes control to the Secure Payload image (BL32) for the first
 * time on the primary cpu after a cold boot. It assumes that a valid secure
 * context has already been created by tspd_setup() which can be directly used.
 * It also assumes that a valid non-secure context has been initialised by PSCI
 * so it does not need to save and restore any non-secure state. This function
 * performs a synchronous entry into the Secure payload. The SP passes control
 * back to this routine through a SMC. It also passes the extents of memory made
 * available to BL32 by BL31.
 ******************************************************************************/
int32_t tspd_init(meminfo_t *bl32_meminfo)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	uint64_t rc;
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	/*
	 * Arrange for passing a pointer to the meminfo structure
	 * describing the memory extents available to the secure
	 * payload.
	 * TODO: We are passing a pointer to BL31 internal memory
	 * whereas this structure should be copied to a communication
	 * buffer between the SP and SPD.
	 */
	write_ctx_reg(get_gpregs_ctx(&tsp_ctx->cpu_ctx),
		      CTX_GPREG_X0,
		      (uint64_t) bl32_meminfo);

	/*
	 * Arrange for an entry into the test secure payload. We expect an array
	 * of vectors in return
	 */
	rc = tspd_synchronous_sp_entry(tsp_ctx);
	assert(rc != 0);
	if (rc) {
		tsp_ctx->state = TSP_STATE_ON;

		/*
		 * TSP has been successfully initialized. Register power
		 * managemnt hooks with PSCI
		 */
		psci_register_spd_pm_hook(&tspd_pm);
	}

	return rc;
}


/*******************************************************************************
 * This function is responsible for handling all SMCs in the Trusted OS/App
 * range from the non-secure state as defined in the SMC Calling Convention
 * Document. It is also responsible for communicating with the Secure payload
 * to delegate work and return results back to the non-secure state. Lastly it
 * will also return any information that the secure payload needs to do the
 * work assigned to it.
 ******************************************************************************/
uint64_t tspd_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	cpu_context_t *ns_cpu_context;
	gp_regs_t *ns_gp_regs;
	unsigned long mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr), ns;
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	switch (smc_fid) {

	/*
	 * This function ID is used only by the SP to indicate it has
	 * finished initialising itself after a cold boot
	 */
	case TSP_ENTRY_DONE:
		if (ns)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * Stash the SP entry points information. This is done
		 * only once on the primary cpu
		 */
		assert(tsp_entry_info == NULL);
		tsp_entry_info = (entry_info_t *) x1;

		/*
		 * SP reports completion. The SPD must have initiated
		 * the original request through a synchronous entry
		 * into the SP. Jump back to the original C runtime
		 * context.
		 */
		tspd_synchronous_sp_exit(tsp_ctx, x1);

		/* Should never reach here */
		assert(0);

	/*
	 * These function IDs is used only by the SP to indicate it has
	 * finished:
	 * 1. turning itself on in response to an earlier psci
	 *    cpu_on request
	 * 2. resuming itself after an earlier psci cpu_suspend
	 *    request.
	 */
	case TSP_ON_DONE:
	case TSP_RESUME_DONE:

	/*
	 * These function IDs is used only by the SP to indicate it has
	 * finished:
	 * 1. suspending itself after an earlier psci cpu_suspend
	 *    request.
	 * 2. turning itself off in response to an earlier psci
	 *    cpu_off request.
	 */
	case TSP_OFF_DONE:
	case TSP_SUSPEND_DONE:
		if (ns)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * SP reports completion. The SPD must have initiated the
		 * original request through a synchronous entry into the SP.
		 * Jump back to the original C runtime context, and pass x1 as
		 * return value to the caller
		 */
		tspd_synchronous_sp_exit(tsp_ctx, x1);

		/* Should never reach here */
		assert(0);

		/*
		 * Request from non-secure client to perform an
		 * arithmetic operation or response from secure
		 * payload to an earlier request.
		 */
	case TSP_FID_ADD:
	case TSP_FID_SUB:
	case TSP_FID_MUL:
	case TSP_FID_DIV:
		if (ns) {
			/*
			 * This is a fresh request from the non-secure client.
			 * The parameters are in x1 and x2. Figure out which
			 * registers need to be preserved, save the non-secure
			 * state and send the request to the secure payload.
			 */
			assert(handle == cm_get_context(mpidr, NON_SECURE));
			cm_el1_sysregs_context_save(NON_SECURE);

			/* Save x1 and x2 for use by TSP_GET_ARGS call below */
			SMC_SET_GP(handle, CTX_GPREG_X1, x1);
			SMC_SET_GP(handle, CTX_GPREG_X2, x2);

			/*
			 * We are done stashing the non-secure context. Ask the
			 * secure payload to do the work now.
			 */

			/*
			 * Verify if there is a valid context to use, copy the
			 * operation type and parameters to the secure context
			 * and jump to the fast smc entry point in the secure
			 * payload. Entry into S-EL1 will take place upon exit
			 * from this function.
			 */
			assert(&tsp_ctx->cpu_ctx == cm_get_context(mpidr, SECURE));
			set_aapcs_args7(&tsp_ctx->cpu_ctx, smc_fid, x1, x2, 0, 0,
					0, 0, 0);
			cm_set_el3_elr(SECURE, (uint64_t) tsp_entry_info->fast_smc_entry);
			cm_el1_sysregs_context_restore(SECURE);
			cm_set_next_eret_context(SECURE);

			return smc_fid;
		} else {
			/*
			 * This is the result from the secure client of an
			 * earlier request. The results are in x1-x2. Copy it
			 * into the non-secure context, save the secure state
			 * and return to the non-secure state.
			 */
			assert(handle == cm_get_context(mpidr, SECURE));
			cm_el1_sysregs_context_save(SECURE);

			/* Get a reference to the non-secure context */
			ns_cpu_context = cm_get_context(mpidr, NON_SECURE);
			assert(ns_cpu_context);
			ns_gp_regs = get_gpregs_ctx(ns_cpu_context);

			/* Restore non-secure state */
			cm_el1_sysregs_context_restore(NON_SECURE);
			cm_set_next_eret_context(NON_SECURE);

			SMC_RET2(ns_gp_regs, x1, x2);
		}

		break;

		/*
		 * This is a request from the secure payload for more arguments
		 * for an ongoing arithmetic operation requested by the
		 * non-secure world. Simply return the arguments from the non-
		 * secure client in the original call.
		 */
	case TSP_GET_ARGS:
		if (ns)
			SMC_RET1(handle, SMC_UNK);

		/* Get a reference to the non-secure context */
		ns_cpu_context = cm_get_context(mpidr, NON_SECURE);
		assert(ns_cpu_context);
		ns_gp_regs = get_gpregs_ctx(ns_cpu_context);

		SMC_RET2(handle, read_ctx_reg(ns_gp_regs, CTX_GPREG_X1),
				read_ctx_reg(ns_gp_regs, CTX_GPREG_X2));

	case TOS_CALL_COUNT:
		/*
		 * Return the number of service function IDs implemented to
		 * provide service to non-secure
		 */
		SMC_RET1(handle, TSP_NUM_FID);

	case TOS_UID:
		/* Return TSP UID to the caller */
		SMC_UUID_RET(handle, tsp_uuid);

	case TOS_CALL_VERSION:
		/* Return the version of current implementation */
		SMC_RET2(handle, TSP_VERSION_MAJOR, TSP_VERSION_MINOR);

	default:
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}

/* Define a SPD runtime service descriptor */
DECLARE_RT_SVC(
	spd,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_FAST,
	tspd_setup,
	tspd_smc_handler
);
