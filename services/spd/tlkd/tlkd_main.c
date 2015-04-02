/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <runtime_svc.h>
#include <stddef.h>
#include <tlk.h>
#include <uuid.h>
#include "tlkd_private.h"

extern const spd_pm_ops_t tlkd_pm_ops;

/*******************************************************************************
 * Array to keep track of per-cpu Secure Payload state
 ******************************************************************************/
static tlk_context_t tlk_ctx;

/* TLK UID: RFC-4122 compliant UUID (version-5, sha-1) */
DEFINE_SVC_UUID(tlk_uuid,
		0xbd11e9c9, 0x2bba, 0x52ee, 0xb1, 0x72,
		0x46, 0x1f, 0xba, 0x97, 0x7f, 0x63);

int32_t tlkd_init(void);

/*
 * The number of arguments/results to save during a SMC call for TLK.
 */
#define TLK_SHDBUF_SIZE		4

/*******************************************************************************
 * Shared memory buffer for passing SMC args/results to TLK
 ******************************************************************************/
typedef struct tlk_args_results {
	uint64_t args[TLK_SHDBUF_SIZE];
} tlk_args_results_t;

static tlk_args_results_t *tlk_args_results_buf;

/*
 * Helper function to store args from TLK and pass results back
 */
static inline void store_tlk_args_results(uint64_t x0, uint64_t x1, uint64_t x2,
	uint64_t x3)
{
	/* store arguments sent by TLK */
	tlk_args_results_buf->args[0] = x0;
	tlk_args_results_buf->args[1] = x1;
	tlk_args_results_buf->args[2] = x2;
	tlk_args_results_buf->args[3] = x3;

	flush_dcache_range((uint64_t)tlk_args_results_buf,
		sizeof(tlk_args_results_t));
}

/*******************************************************************************
 * Secure Payload Dispatcher setup. The SPD finds out the SP entrypoint and type
 * (aarch32/aarch64) if not already known and initialises the context for entry
 * into the SP for its initialisation.
 ******************************************************************************/
int32_t tlkd_setup(void)
{
	entry_point_info_t *tlk_ep_info;

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.
	 */
	tlk_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!tlk_ep_info) {
		WARN("No SP provided. Booting device without SP"
			" initialization. SMC`s destined for SP"
			" will return SMC_UNK\n");
		return 1;
	}

	/*
	 * If there's no valid entry point for SP, we return a non-zero value
	 * signalling failure initializing the service. We bail out without
	 * registering any handlers
	 */
	if (!tlk_ep_info->pc)
		return 1;

	/*
	 * Inspect the SP image's SPSR and determine it's execution state
	 * i.e whether AArch32 or AArch64.
	 */
	tlkd_init_tlk_ep_state(tlk_ep_info,
		(tlk_ep_info->spsr >> MODE_RW_SHIFT) & MODE_RW_MASK,
		tlk_ep_info->pc,
		&tlk_ctx);

	/*
	 * All TLK SPD initialization done. Now register our init function
	 * with BL31 for deferred invocation
	 */
	bl31_register_bl32_init(&tlkd_init);

	return 0;
}

/*******************************************************************************
 * This function passes control to the Secure Payload image (BL32) for the first
 * time on the primary cpu after a cold boot. It assumes that a valid secure
 * context has already been created by tlkd_setup() which can be directly
 * used. This function performs a synchronous entry into the Secure payload.
 * The SP passes control back to this routine through a SMC.
 ******************************************************************************/
int32_t tlkd_init(void)
{
	uint64_t mpidr = read_mpidr();
	entry_point_info_t *tlk_entry_point;

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.
	 */
	tlk_entry_point = bl31_plat_get_next_image_ep_info(SECURE);
	assert(tlk_entry_point);

	cm_init_context(mpidr, tlk_entry_point);

	/*
	 * Arrange for an entry into the test secure payload.
	 */
	return tlkd_synchronous_sp_entry(&tlk_ctx);
}

/*******************************************************************************
 * This function is responsible for handling all SMCs in the Trusted OS/App
 * range from the non-secure state as defined in the SMC Calling Convention
 * Document. It is also responsible for communicating with the Secure payload
 * to delegate work and return results back to the non-secure state. Lastly it
 * will also return any information that the secure payload needs to do the
 * work assigned to it.
 ******************************************************************************/
uint64_t tlkd_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	cpu_context_t *ns_cpu_context;
	uint32_t ns;
	uint64_t vaddr, type, par;

	/* Passing a NULL context is a critical programming error */
	assert(handle);

	/* These SMCs are only supported by CPU0 */
	if ((read_mpidr() & MPIDR_CPU_MASK) != 0)
		SMC_RET1(handle, SMC_UNK);

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	switch (smc_fid) {

	/*
	 * This function ID is used by SP to indicate that it was
	 * preempted by a non-secure world IRQ.
	 */
	case TLK_PREEMPTED:

		if (ns)
			SMC_RET1(handle, SMC_UNK);

		assert(handle == cm_get_context(SECURE));
		cm_el1_sysregs_context_save(SECURE);

		/* Get a reference to the non-secure context */
		ns_cpu_context = cm_get_context(NON_SECURE);
		assert(ns_cpu_context);

		/*
		 * Restore non-secure state. There is no need to save the
		 * secure system register context since the SP was supposed
		 * to preserve it during S-EL1 interrupt handling.
		 */
		cm_el1_sysregs_context_restore(NON_SECURE);
		cm_set_next_eret_context(NON_SECURE);

		SMC_RET1(ns_cpu_context, tlk_args_results_buf->args[0]);

	/*
	 * Request from non secure world to resume the preempted
	 * Standard SMC call.
	 */
	case TLK_RESUME_FID:

		/* RESUME should be invoked only by normal world */
		if (!ns)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * This is a resume request from the non-secure client.
		 * save the non-secure state and send the request to
		 * the secure payload.
		 */
		assert(handle == cm_get_context(NON_SECURE));

		/* Check if we are already preempted before resume */
		if (!get_std_smc_active_flag(tlk_ctx.state))
			SMC_RET1(handle, SMC_UNK);

		cm_el1_sysregs_context_save(NON_SECURE);

		/*
		 * We are done stashing the non-secure context. Ask the
		 * secure payload to do the work now.
		 */

		/* We just need to return to the preempted point in
		 * SP and the execution will resume as normal.
		 */
		cm_el1_sysregs_context_restore(SECURE);
		cm_set_next_eret_context(SECURE);
		SMC_RET0(handle);

	/*
	 * This is a request from the non-secure context to:
	 *
	 * a. register shared memory with the SP for storing it's
	 *    activity logs.
	 * b. register shared memory with the SP for passing args
	 *    required for maintaining sessions with the Trusted
	 *    Applications.
	 * c. open/close sessions
	 * d. issue commands to the Trusted Apps
	 */
	case TLK_REGISTER_LOGBUF:
	case TLK_REGISTER_REQBUF:
	case TLK_OPEN_TA_SESSION:
	case TLK_CLOSE_TA_SESSION:
	case TLK_TA_LAUNCH_OP:
	case TLK_TA_SEND_EVENT:

		if (!ns || !tlk_args_results_buf)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * This is a fresh request from the non-secure client.
		 * The parameters are in x1 and x2. Figure out which
		 * registers need to be preserved, save the non-secure
		 * state and send the request to the secure payload.
		 */
		assert(handle == cm_get_context(NON_SECURE));

		/* Check if we are already preempted */
		if (get_std_smc_active_flag(tlk_ctx.state))
			SMC_RET1(handle, SMC_UNK);

		cm_el1_sysregs_context_save(NON_SECURE);

		/*
		 * Verify if there is a valid context to use.
		 */
		assert(&tlk_ctx.cpu_ctx == cm_get_context(SECURE));

		/*
		 * Mark the SP state as active.
		 */
		set_std_smc_active_flag(tlk_ctx.state);

		/* Save args for use by the SP on return */
		store_tlk_args_results(smc_fid, x1, x2, x3);

		/*
		 * We are done stashing the non-secure context. Ask the
		 * secure payload to do the work now.
		 */
		cm_el1_sysregs_context_restore(SECURE);
		cm_set_next_eret_context(SECURE);
		SMC_RET0(&tlk_ctx.cpu_ctx);

	/*
	 * Translate NS/EL1-S virtual addresses
	 */
	case TLK_VA_TRANSLATE:
		if (ns || !tlk_args_results_buf)
			SMC_RET1(handle, SMC_UNK);

		/* virtual address and type: ns/s */
		vaddr = tlk_args_results_buf->args[0];
		type = tlk_args_results_buf->args[1];

		par = tlkd_va_translate(vaddr, type);

		/* Save PA for use by the SP on return */
		store_tlk_args_results(par, 0, 0, 0);

		SMC_RET0(handle);

	/*
	 * This is a request from the SP to mark completion of
	 * a standard function ID.
	 */
	case TLK_REQUEST_DONE:
		if (ns || !tlk_args_results_buf)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * Mark the SP state as inactive.
		 */
		clr_std_smc_active_flag(tlk_ctx.state);

		/* Get a reference to the non-secure context */
		ns_cpu_context = cm_get_context(NON_SECURE);
		assert(ns_cpu_context);

		/*
		 * This is a request completion SMC and we must switch to
		 * the non-secure world to pass the result.
		 */
		cm_el1_sysregs_context_save(SECURE);

		/*
		 * We are done stashing the secure context. Switch to the
		 * non-secure context and return the result.
		 */
		cm_el1_sysregs_context_restore(NON_SECURE);
		cm_set_next_eret_context(NON_SECURE);
		SMC_RET1(ns_cpu_context, tlk_args_results_buf->args[0]);

	/*
	 * This function ID is used only by the SP to indicate it has
	 * finished initialising itself after a cold boot
	 */
	case TLK_ENTRY_DONE:
		if (ns || !tlk_args_results_buf)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * SP has been successfully initialized. Register power
		 * managemnt hooks with PSCI
		 */
		psci_register_spd_pm_hook(&tlkd_pm_ops);

		/*
		 * TLK reports completion. The SPD must have initiated
		 * the original request through a synchronous entry
		 * into the SP. Jump back to the original C runtime
		 * context.
		 */
		tlkd_synchronous_sp_exit(&tlk_ctx, tlk_args_results_buf->args[0]);

	/*
	 * This is a request from the secure payload to register
	 * shared memory to pass SMC args/results between EL1, EL3.
	 */
	case TLK_FID_SHARED_MEMBUF:
		if (ns || !x1)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * TODO: Check if the passed memory pointer is valid. Might
		 * require a call into the platform code.
		 */

		tlk_args_results_buf = (tlk_args_results_t *)x1;
		SMC_RET0(handle);

	/*
	 * Return the number of service function IDs implemented to
	 * provide service to non-secure
	 */
	case TOS_CALL_COUNT:
		SMC_RET1(handle, TLK_NUM_FID);

	/*
	 * Return TLK's UID to the caller
	 */
	case TOS_UID:
		SMC_UUID_RET(handle, tlk_uuid);

	/*
	 * Return the version of current implementation
	 */
	case TOS_CALL_VERSION:
		SMC_RET2(handle, TLK_VERSION_MAJOR, TLK_VERSION_MINOR);

	default:
		break;
	}

	SMC_RET1(handle, SMC_UNK);
}

/* Define a SPD runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	tlkd_tos_fast,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_FAST,
	tlkd_setup,
	tlkd_smc_handler
);

/* Define a SPD runtime service descriptor for standard SMC calls */
DECLARE_RT_SVC(
	tlkd_tos_std,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_STD,
	NULL,
	tlkd_smc_handler
);

/* Define a SPD runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	tlkd_tap_fast,

	OEN_TAP_START,
	OEN_TAP_END,
	SMC_TYPE_FAST,
	NULL,
	tlkd_smc_handler
);

/* Define a SPD runtime service descriptor for standard SMC calls */
DECLARE_RT_SVC(
	tlkd_tap_std,

	OEN_TAP_START,
	OEN_TAP_END,
	SMC_TYPE_STD,
	NULL,
	tlkd_smc_handler
);
