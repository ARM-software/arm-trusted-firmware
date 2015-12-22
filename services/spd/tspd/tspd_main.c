/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
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
#include <string.h>
#include <tsp.h>
#include <uuid.h>
#include "tspd_private.h"

/*******************************************************************************
 * Address of the entrypoint vector table in the Secure Payload. It is
 * initialised once on the primary core after a cold boot.
 ******************************************************************************/
tsp_vectors_t *tsp_vectors;

/*******************************************************************************
 * Array to keep track of per-cpu Secure Payload state
 ******************************************************************************/
tsp_context_t tspd_sp_context[TSPD_CORE_COUNT];


/* TSP UID */
DEFINE_SVC_UUID(tsp_uuid,
		0x5b3056a0, 0x3291, 0x427b, 0x98, 0x11,
		0x71, 0x68, 0xca, 0x50, 0xf3, 0xfa);

int32_t tspd_init(void);

/*
 * This helper function handles Secure EL1 preemption. The preemption could be
 * due Non Secure interrupts or EL3 interrupts. In both the cases we context
 * switch to the normal world and in case of EL3 interrupts, it will again be
 * routed to EL3 which will get handled at the exception vectors.
 */
uint64_t tspd_handle_sp_preemption(void *handle)
{
	cpu_context_t *ns_cpu_context;

	assert(handle == cm_get_context(SECURE));
	cm_el1_sysregs_context_save(SECURE);
	/* Get a reference to the non-secure context */
	ns_cpu_context = cm_get_context(NON_SECURE);
	assert(ns_cpu_context);

	/*
	 * To allow Secure EL1 interrupt handler to re-enter TSP while TSP
	 * is preempted, the secure system register context which will get
	 * overwritten must be additionally saved. This is currently done
	 * by the TSPD S-EL1 interrupt handler.
	 */

	/*
	 * Restore non-secure state.
	 */
	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	/*
	 * The TSP was preempted during STD SMC execution.
	 * Return back to the normal world with SMC_PREEMPTED as error
	 * code in x0.
	 */
	SMC_RET1(ns_cpu_context, SMC_PREEMPTED);
}

/*******************************************************************************
 * This function is the handler registered for S-EL1 interrupts by the TSPD. It
 * validates the interrupt and upon success arranges entry into the TSP at
 * 'tsp_sel1_intr_entry()' for handling the interrupt.
 ******************************************************************************/
static uint64_t tspd_sel1_interrupt_handler(uint32_t id,
					    uint32_t flags,
					    void *handle,
					    void *cookie)
{
	uint32_t linear_id;
	tsp_context_t *tsp_ctx;

	/* Check the security state when the exception was generated */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

	/* Sanity check the pointer to this cpu's context */
	assert(handle == cm_get_context(NON_SECURE));

	/* Save the non-secure context before entering the TSP */
	cm_el1_sysregs_context_save(NON_SECURE);

	/* Get a reference to this cpu's TSP context */
	linear_id = plat_my_core_pos();
	tsp_ctx = &tspd_sp_context[linear_id];
	assert(&tsp_ctx->cpu_ctx == cm_get_context(SECURE));

	/*
	 * Determine if the TSP was previously preempted. Its last known
	 * context has to be preserved in this case.
	 * The TSP should return control to the TSPD after handling this
	 * S-EL1 interrupt. Preserve essential EL3 context to allow entry into
	 * the TSP at the S-EL1 interrupt entry point using the 'cpu_context'
	 * structure. There is no need to save the secure system register
	 * context since the TSP is supposed to preserve it during S-EL1
	 * interrupt handling.
	 */
	if (get_std_smc_active_flag(tsp_ctx->state)) {
		tsp_ctx->saved_spsr_el3 = SMC_GET_EL3(&tsp_ctx->cpu_ctx,
						      CTX_SPSR_EL3);
		tsp_ctx->saved_elr_el3 = SMC_GET_EL3(&tsp_ctx->cpu_ctx,
						     CTX_ELR_EL3);
#if TSP_NS_INTR_ASYNC_PREEMPT
		/*Need to save the previously interrupted secure context */
		memcpy(&tsp_ctx->sp_ctx, &tsp_ctx->cpu_ctx, TSPD_SP_CTX_SIZE);
#endif
	}

	cm_el1_sysregs_context_restore(SECURE);
	cm_set_elr_spsr_el3(SECURE, (uint64_t) &tsp_vectors->sel1_intr_entry,
		    SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS));

	cm_set_next_eret_context(SECURE);

	/*
	 * Tell the TSP that it has to handle a S-EL1 interrupt synchronously.
	 * Also the instruction in normal world where the interrupt was
	 * generated is passed for debugging purposes. It is safe to retrieve
	 * this address from ELR_EL3 as the secure context will not take effect
	 * until el3_exit().
	 */
	SMC_RET2(&tsp_ctx->cpu_ctx, TSP_HANDLE_SEL1_INTR_AND_RETURN, read_elr_el3());
}

#if TSP_NS_INTR_ASYNC_PREEMPT
/*******************************************************************************
 * This function is the handler registered for Non secure interrupts by the
 * TSPD. It validates the interrupt and upon success arranges entry into the
 * normal world for handling the interrupt.
 ******************************************************************************/
static uint64_t tspd_ns_interrupt_handler(uint32_t id,
					    uint32_t flags,
					    void *handle,
					    void *cookie)
{
	/* Check the security state when the exception was generated */
	assert(get_interrupt_src_ss(flags) == SECURE);

	/*
	 * Disable the routing of NS interrupts from secure world to EL3 while
	 * interrupted on this core.
	 */
	disable_intr_rm_local(INTR_TYPE_NS, SECURE);

	return tspd_handle_sp_preemption(handle);
}
#endif

/*******************************************************************************
 * Secure Payload Dispatcher setup. The SPD finds out the SP entrypoint and type
 * (aarch32/aarch64) if not already known and initialises the context for entry
 * into the SP for its initialisation.
 ******************************************************************************/
int32_t tspd_setup(void)
{
	entry_point_info_t *tsp_ep_info;
	uint32_t linear_id;

	linear_id = plat_my_core_pos();

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.  TODO: Add support to
	 * conditionally include the SPD service
	 */
	tsp_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!tsp_ep_info) {
		WARN("No TSP provided by BL2 boot loader, Booting device"
			" without TSP initialization. SMC`s destined for TSP"
			" will return SMC_UNK\n");
		return 1;
	}

	/*
	 * If there's no valid entry point for SP, we return a non-zero value
	 * signalling failure initializing the service. We bail out without
	 * registering any handlers
	 */
	if (!tsp_ep_info->pc)
		return 1;

	/*
	 * We could inspect the SP image and determine its execution
	 * state i.e whether AArch32 or AArch64. Assuming it's AArch64
	 * for the time being.
	 */
	tspd_init_tsp_ep_state(tsp_ep_info,
				TSP_AARCH64,
				tsp_ep_info->pc,
				&tspd_sp_context[linear_id]);

#if TSP_INIT_ASYNC
	bl31_set_next_image_type(SECURE);
#else
	/*
	 * All TSPD initialization done. Now register our init function with
	 * BL31 for deferred invocation
	 */
	bl31_register_bl32_init(&tspd_init);
#endif
	return 0;
}

/*******************************************************************************
 * This function passes control to the Secure Payload image (BL32) for the first
 * time on the primary cpu after a cold boot. It assumes that a valid secure
 * context has already been created by tspd_setup() which can be directly used.
 * It also assumes that a valid non-secure context has been initialised by PSCI
 * so it does not need to save and restore any non-secure state. This function
 * performs a synchronous entry into the Secure payload. The SP passes control
 * back to this routine through a SMC.
 ******************************************************************************/
int32_t tspd_init(void)
{
	uint32_t linear_id = plat_my_core_pos();
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];
	entry_point_info_t *tsp_entry_point;
	uint64_t rc;

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.
	 */
	tsp_entry_point = bl31_plat_get_next_image_ep_info(SECURE);
	assert(tsp_entry_point);

	cm_init_my_context(tsp_entry_point);

	/*
	 * Arrange for an entry into the test secure payload. It will be
	 * returned via TSP_ENTRY_DONE case
	 */
	rc = tspd_synchronous_sp_entry(tsp_ctx);
	assert(rc != 0);

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
	uint32_t linear_id = plat_my_core_pos(), ns;
	tsp_context_t *tsp_ctx = &tspd_sp_context[linear_id];
	uint64_t rc;
#if TSP_INIT_ASYNC
	entry_point_info_t *next_image_info;
#endif

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	switch (smc_fid) {

	/*
	 * This function ID is used by TSP to indicate that it was
	 * preempted by a normal world IRQ.
	 *
	 */
	case TSP_PREEMPTED:
		if (ns)
			SMC_RET1(handle, SMC_UNK);

		return tspd_handle_sp_preemption(handle);

	/*
	 * This function ID is used only by the TSP to indicate that it has
	 * finished handling a S-EL1 interrupt or was preempted by a higher
	 * priority pending EL3 interrupt. Execution should resume
	 * in the normal world.
	 */
	case TSP_HANDLED_S_EL1_INTR:
		if (ns)
			SMC_RET1(handle, SMC_UNK);

		assert(handle == cm_get_context(SECURE));

		/*
		 * Restore the relevant EL3 state which saved to service
		 * this SMC.
		 */
		if (get_std_smc_active_flag(tsp_ctx->state)) {
			SMC_SET_EL3(&tsp_ctx->cpu_ctx,
				    CTX_SPSR_EL3,
				    tsp_ctx->saved_spsr_el3);
			SMC_SET_EL3(&tsp_ctx->cpu_ctx,
				    CTX_ELR_EL3,
				    tsp_ctx->saved_elr_el3);
#if TSP_NS_INTR_ASYNC_PREEMPT
			/*
			 * Need to restore the previously interrupted
			 * secure context.
			 */
			memcpy(&tsp_ctx->cpu_ctx, &tsp_ctx->sp_ctx,
				TSPD_SP_CTX_SIZE);
#endif
		}

		/* Get a reference to the non-secure context */
		ns_cpu_context = cm_get_context(NON_SECURE);
		assert(ns_cpu_context);

		/*
		 * Restore non-secure state. There is no need to save the
		 * secure system register context since the TSP was supposed
		 * to preserve it during S-EL1 interrupt handling.
		 */
		cm_el1_sysregs_context_restore(NON_SECURE);
		cm_set_next_eret_context(NON_SECURE);

		SMC_RET0((uint64_t) ns_cpu_context);

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
		assert(tsp_vectors == NULL);
		tsp_vectors = (tsp_vectors_t *) x1;

		if (tsp_vectors) {
			set_tsp_pstate(tsp_ctx->state, TSP_PSTATE_ON);

			/*
			 * TSP has been successfully initialized. Register power
			 * managemnt hooks with PSCI
			 */
			psci_register_spd_pm_hook(&tspd_pm);

			/*
			 * Register an interrupt handler for S-EL1 interrupts
			 * when generated during code executing in the
			 * non-secure state.
			 */
			flags = 0;
			set_interrupt_rm_flag(flags, NON_SECURE);
			rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
						tspd_sel1_interrupt_handler,
						flags);
			if (rc)
				panic();

#if TSP_NS_INTR_ASYNC_PREEMPT
			/*
			 * Register an interrupt handler for NS interrupts when
			 * generated during code executing in secure state are
			 * routed to EL3.
			 */
			flags = 0;
			set_interrupt_rm_flag(flags, SECURE);

			rc = register_interrupt_type_handler(INTR_TYPE_NS,
						tspd_ns_interrupt_handler,
						flags);
			if (rc)
				panic();

			/*
			 * Disable the NS interrupt locally.
			 */
			disable_intr_rm_local(INTR_TYPE_NS, SECURE);
#endif
		}


#if TSP_INIT_ASYNC
		/* Save the Secure EL1 system register context */
		assert(cm_get_context(SECURE) == &tsp_ctx->cpu_ctx);
		cm_el1_sysregs_context_save(SECURE);

		/* Program EL3 registers to enable entry into the next EL */
		next_image_info = bl31_plat_get_next_image_ep_info(NON_SECURE);
		assert(next_image_info);
		assert(NON_SECURE ==
				GET_SECURITY_STATE(next_image_info->h.attr));

		cm_init_my_context(next_image_info);
		cm_prepare_el3_exit(NON_SECURE);
		SMC_RET0(cm_get_context(NON_SECURE));
#else
		/*
		 * SP reports completion. The SPD must have initiated
		 * the original request through a synchronous entry
		 * into the SP. Jump back to the original C runtime
		 * context.
		 */
		tspd_synchronous_sp_exit(tsp_ctx, x1);
#endif

	/*
	 * These function IDs are used only by the SP to indicate it has
	 * finished:
	 * 1. turning itself on in response to an earlier psci
	 *    cpu_on request
	 * 2. resuming itself after an earlier psci cpu_suspend
	 *    request.
	 */
	case TSP_ON_DONE:
	case TSP_RESUME_DONE:

	/*
	 * These function IDs are used only by the SP to indicate it has
	 * finished:
	 * 1. suspending itself after an earlier psci cpu_suspend
	 *    request.
	 * 2. turning itself off in response to an earlier psci
	 *    cpu_off request.
	 */
	case TSP_OFF_DONE:
	case TSP_SUSPEND_DONE:
	case TSP_SYSTEM_OFF_DONE:
	case TSP_SYSTEM_RESET_DONE:
		if (ns)
			SMC_RET1(handle, SMC_UNK);

		/*
		 * SP reports completion. The SPD must have initiated the
		 * original request through a synchronous entry into the SP.
		 * Jump back to the original C runtime context, and pass x1 as
		 * return value to the caller
		 */
		tspd_synchronous_sp_exit(tsp_ctx, x1);

		/*
		 * Request from non-secure client to perform an
		 * arithmetic operation or response from secure
		 * payload to an earlier request.
		 */
	case TSP_FAST_FID(TSP_ADD):
	case TSP_FAST_FID(TSP_SUB):
	case TSP_FAST_FID(TSP_MUL):
	case TSP_FAST_FID(TSP_DIV):

	case TSP_STD_FID(TSP_ADD):
	case TSP_STD_FID(TSP_SUB):
	case TSP_STD_FID(TSP_MUL):
	case TSP_STD_FID(TSP_DIV):
		if (ns) {
			/*
			 * This is a fresh request from the non-secure client.
			 * The parameters are in x1 and x2. Figure out which
			 * registers need to be preserved, save the non-secure
			 * state and send the request to the secure payload.
			 */
			assert(handle == cm_get_context(NON_SECURE));

			/* Check if we are already preempted */
			if (get_std_smc_active_flag(tsp_ctx->state))
				SMC_RET1(handle, SMC_UNK);

			cm_el1_sysregs_context_save(NON_SECURE);

			/* Save x1 and x2 for use by TSP_GET_ARGS call below */
			store_tsp_args(tsp_ctx, x1, x2);

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
			assert(&tsp_ctx->cpu_ctx == cm_get_context(SECURE));

			/* Set appropriate entry for SMC.
			 * We expect the TSP to manage the PSTATE.I and PSTATE.F
			 * flags as appropriate.
			 */
			if (GET_SMC_TYPE(smc_fid) == SMC_TYPE_FAST) {
				cm_set_elr_el3(SECURE, (uint64_t)
						&tsp_vectors->fast_smc_entry);
			} else {
				set_std_smc_active_flag(tsp_ctx->state);
				cm_set_elr_el3(SECURE, (uint64_t)
						&tsp_vectors->std_smc_entry);
#if TSP_NS_INTR_ASYNC_PREEMPT
				/*
				 * Enable the routing of NS interrupts to EL3
				 * during STD SMC processing on this core.
				 */
				enable_intr_rm_local(INTR_TYPE_NS, SECURE);
#endif
			}

			cm_el1_sysregs_context_restore(SECURE);
			cm_set_next_eret_context(SECURE);
			SMC_RET3(&tsp_ctx->cpu_ctx, smc_fid, x1, x2);
		} else {
			/*
			 * This is the result from the secure client of an
			 * earlier request. The results are in x1-x3. Copy it
			 * into the non-secure context, save the secure state
			 * and return to the non-secure state.
			 */
			assert(handle == cm_get_context(SECURE));
			cm_el1_sysregs_context_save(SECURE);

			/* Get a reference to the non-secure context */
			ns_cpu_context = cm_get_context(NON_SECURE);
			assert(ns_cpu_context);

			/* Restore non-secure state */
			cm_el1_sysregs_context_restore(NON_SECURE);
			cm_set_next_eret_context(NON_SECURE);
			if (GET_SMC_TYPE(smc_fid) == SMC_TYPE_STD) {
				clr_std_smc_active_flag(tsp_ctx->state);
#if TSP_NS_INTR_ASYNC_PREEMPT
				/*
				 * Disable the routing of NS interrupts to EL3
				 * after STD SMC processing is finished on this
				 * core.
				 */
				disable_intr_rm_local(INTR_TYPE_NS, SECURE);
#endif
			}

			SMC_RET3(ns_cpu_context, x1, x2, x3);
		}

		break;

		/*
		 * Request from non secure world to resume the preempted
		 * Standard SMC call.
		 */
	case TSP_FID_RESUME:
		/* RESUME should be invoked only by normal world */
		if (!ns) {
			assert(0);
			break;
		}

		/*
		 * This is a resume request from the non-secure client.
		 * save the non-secure state and send the request to
		 * the secure payload.
		 */
		assert(handle == cm_get_context(NON_SECURE));

		/* Check if we are already preempted before resume */
		if (!get_std_smc_active_flag(tsp_ctx->state))
			SMC_RET1(handle, SMC_UNK);

		cm_el1_sysregs_context_save(NON_SECURE);

		/*
		 * We are done stashing the non-secure context. Ask the
		 * secure payload to do the work now.
		 */
#if TSP_NS_INTR_ASYNC_PREEMPT
		/*
		 * Enable the routing of NS interrupts to EL3 during resumption
		 * of STD SMC call on this core.
		 */
		enable_intr_rm_local(INTR_TYPE_NS, SECURE);
#endif



		/* We just need to return to the preempted point in
		 * TSP and the execution will resume as normal.
		 */
		cm_el1_sysregs_context_restore(SECURE);
		cm_set_next_eret_context(SECURE);
		SMC_RET0(&tsp_ctx->cpu_ctx);

		/*
		 * This is a request from the secure payload for more arguments
		 * for an ongoing arithmetic operation requested by the
		 * non-secure world. Simply return the arguments from the non-
		 * secure client in the original call.
		 */
	case TSP_GET_ARGS:
		if (ns)
			SMC_RET1(handle, SMC_UNK);

		get_tsp_args(tsp_ctx, x1, x2);
		SMC_RET2(handle, x1, x2);

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

/* Define a SPD runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	tspd_fast,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_FAST,
	tspd_setup,
	tspd_smc_handler
);

/* Define a SPD runtime service descriptor for standard SMC calls */
DECLARE_RT_SVC(
	tspd_std,

	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_STD,
	NULL,
	tspd_smc_handler
);
