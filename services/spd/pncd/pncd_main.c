/*
 * Copyright (c) 2021-2022, ProvenRun S.A.S. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <bl31/interrupt_mgmt.h>
#include <bl_common.h>
#include <common/debug.h>
#include <common/ep_info.h>
#include <drivers/arm/gic_common.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>
#include <pnc.h>
#include "pncd_private.h"
#include <runtime_svc.h>
#include <tools_share/uuid.h>

/*******************************************************************************
 * Structure to keep track of ProvenCore state
 ******************************************************************************/
static pnc_context_t pncd_sp_context;

static bool ree_info;
static uint64_t ree_base_addr;
static uint64_t ree_length;
static uint64_t ree_tag;

static bool pnc_initialized;

static spinlock_t smc_handler_lock;

static int pncd_init(void);

static void context_save(unsigned long security_state)
{
	assert(sec_state_is_valid(security_state));

	cm_el1_sysregs_context_save((uint32_t) security_state);
#if CTX_INCLUDE_FPREGS
	fpregs_context_save(get_fpregs_ctx(cm_get_context(security_state)));
#endif
}

static void *context_restore(unsigned long security_state)
{
	void *handle;

	assert(sec_state_is_valid(security_state));

	/* Get a reference to the next context */
	handle = cm_get_context((uint32_t) security_state);
	assert(handle);

	/* Restore state */
	cm_el1_sysregs_context_restore((uint32_t) security_state);
#if CTX_INCLUDE_FPREGS
	fpregs_context_restore(get_fpregs_ctx(cm_get_context(security_state)));
#endif

	cm_set_next_eret_context((uint32_t) security_state);

	return handle;
}

static uint64_t pncd_sel1_interrupt_handler(uint32_t id,
		uint32_t flags, void *handle, void *cookie);

/*******************************************************************************
 * Switch context to the specified security state and return the targeted
 * handle. Note that the context may remain unchanged if the switch is not
 * allowed.
 ******************************************************************************/
void *pncd_context_switch_to(unsigned long security_state)
{
	unsigned long sec_state_from =
	    security_state == SECURE ? NON_SECURE : SECURE;

	assert(sec_state_is_valid(security_state));

	/* Check if this is the first world switch */
	if (!pnc_initialized) {
		int rc;
		uint32_t flags;

		assert(sec_state_from == SECURE);

		INFO("PnC initialization done\n");

		/*
		 * Register an interrupt handler for S-EL1 interrupts
		 * when generated during code executing in the
		 * non-secure state.
		 */
		flags = 0U;
		set_interrupt_rm_flag(flags, NON_SECURE);
		rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
				pncd_sel1_interrupt_handler,
				flags);
		if (rc != 0) {
			ERROR("Failed to register S-EL1 interrupt handler (%d)\n",
			      rc);
			panic();
		}

		context_save(SECURE);

		pnc_initialized = true;

		/*
		 * Release the lock before restoring the EL3 context to
		 * bl31_main.
		 */
		spin_unlock(&smc_handler_lock);

		/*
		 * SP reports completion. The SPD must have initiated
		 * the original request through a synchronous entry
		 * into the SP. Jump back to the original C runtime
		 * context.
		 */
		pncd_synchronous_sp_exit(&pncd_sp_context, (uint64_t) 0x0);

		/* Unreachable */
		ERROR("Returned from pncd_synchronous_sp_exit... Should not happen\n");
		panic();
	}

	/* Check that the world switch is allowed */
	if (read_mpidr() != pncd_sp_context.mpidr) {
		if (sec_state_from == SECURE) {
			/*
			 * Secure -> Non-Secure world switch initiated on a CPU where there
			 * should be no Trusted OS running
			 */
			WARN("Secure to Non-Secure switch requested on CPU where ProvenCore is not supposed to be running...\n");
		}

		/*
		 * Secure or Non-Secure world wants to switch world but there is no Secure
		 * software on this core
		 */
		return cm_get_context((uint32_t) sec_state_from);
	}

	context_save(sec_state_from);

	return context_restore(security_state);
}

/*******************************************************************************
 * This function is the handler registered for S-EL1 interrupts by the PNCD. It
 * validates the interrupt and upon success arranges entry into the PNC at
 * 'pnc_sel1_intr_entry()' for handling the interrupt.
 ******************************************************************************/
static uint64_t pncd_sel1_interrupt_handler(uint32_t id,
		uint32_t flags,
		void *handle,
		void *cookie)
{
	/* Check the security state when the exception was generated */
	assert(get_interrupt_src_ss(flags) == NON_SECURE);

	/* Sanity check the pointer to this cpu's context */
	assert(handle == cm_get_context(NON_SECURE));

	/* switch to PnC */
	handle = pncd_context_switch_to(SECURE);

	assert(handle != NULL);

	SMC_RET0(handle);
}

#pragma weak plat_pncd_setup
int plat_pncd_setup(void)
{
	return 0;
}

/*******************************************************************************
 * Secure Payload Dispatcher setup. The SPD finds out the SP entrypoint and type
 * (aarch32/aarch64) if not already known and initialises the context for entry
 * into the SP for its initialisation.
 ******************************************************************************/
static int pncd_setup(void)
{
	entry_point_info_t *pnc_ep_info;

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.
	 *
	 * TODO: Add support to conditionally include the SPD service
	 */
	pnc_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!pnc_ep_info) {
		WARN("No PNC provided by BL2 boot loader, Booting device without PNC initialization. SMC`s destined for PNC will return SMC_UNK\n");
		return 1;
	}

	/*
	 * If there's no valid entry point for SP, we return a non-zero value
	 * signalling failure initializing the service. We bail out without
	 * registering any handlers
	 */
	if (!pnc_ep_info->pc) {
		return 1;
	}

	pncd_init_pnc_ep_state(pnc_ep_info,
			pnc_ep_info->pc,
			&pncd_sp_context);

	/*
	 * All PNCD initialization done. Now register our init function with
	 * BL31 for deferred invocation
	 */
	bl31_register_bl32_init(&pncd_init);
	bl31_set_next_image_type(NON_SECURE);

	return plat_pncd_setup();
}

/*******************************************************************************
 * This function passes control to the Secure Payload image (BL32) for the first
 * time on the primary cpu after a cold boot. It assumes that a valid secure
 * context has already been created by pncd_setup() which can be directly used.
 * It also assumes that a valid non-secure context has been initialised by PSCI
 * so it does not need to save and restore any non-secure state. This function
 * performs a synchronous entry into the Secure payload. The SP passes control
 * back to this routine through a SMC.
 ******************************************************************************/
static int32_t pncd_init(void)
{
	entry_point_info_t *pnc_entry_point;
	uint64_t rc = 0;

	/*
	 * Get information about the Secure Payload (BL32) image. Its
	 * absence is a critical failure.
	 */
	pnc_entry_point = bl31_plat_get_next_image_ep_info(SECURE);
	assert(pnc_entry_point);

	cm_init_my_context(pnc_entry_point);

	/*
	 * Arrange for an entry into the test secure payload. It will be
	 * returned via PNC_ENTRY_DONE case
	 */
	rc = pncd_synchronous_sp_entry(&pncd_sp_context);

	/*
	 * If everything went well at this point, the return value should be 0.
	 */
	return rc == 0;
}

#pragma weak plat_pncd_smc_handler
/*******************************************************************************
 * This function is responsible for handling the platform-specific SMCs in the
 * Trusted OS/App range as defined in the SMC Calling Convention Document.
 ******************************************************************************/
uintptr_t plat_pncd_smc_handler(uint32_t smc_fid,
		u_register_t x1,
		u_register_t x2,
		u_register_t x3,
		u_register_t x4,
		void *cookie,
		void *handle,
		u_register_t flags)
{
	(void) smc_fid;
	(void) x1;
	(void) x2;
	(void) x3;
	(void) x4;
	(void) cookie;
	(void) flags;

	SMC_RET1(handle, SMC_UNK);
}

/*******************************************************************************
 * This function is responsible for handling all SMCs in the Trusted OS/App
 * range as defined in the SMC Calling Convention Document. It is also
 * responsible for communicating with the Secure payload to delegate work and
 * return results back to the non-secure state. Lastly it will also return any
 * information that the secure payload needs to do the work assigned to it.
 *
 * It should only be called with the smc_handler_lock held.
 ******************************************************************************/
static uintptr_t pncd_smc_handler_unsafe(uint32_t smc_fid,
		u_register_t x1,
		u_register_t x2,
		u_register_t x3,
		u_register_t x4,
		void *cookie,
		void *handle,
		u_register_t flags)
{
	uint32_t ns;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	assert(ns != 0 || read_mpidr() == pncd_sp_context.mpidr);

	switch (smc_fid) {
	case SMC_CONFIG_SHAREDMEM:
		if (ree_info) {
			/* Do not Yield */
			SMC_RET0(handle);
		}

		/*
		 * Fetch the physical base address (x1) and size (x2) of the
		 * shared memory allocated by the Non-Secure world. This memory
		 * will be used by PNC to communicate with the Non-Secure world.
		 * Verifying the validity of these values is up to the Trusted
		 * OS.
		 */
		ree_base_addr = x1 | (x2 << 32);
		ree_length = x3;
		ree_tag = x4;

		INFO("IN SMC_CONFIG_SHAREDMEM: addr=%lx, length=%lx, tag=%lx\n",
		     (unsigned long) ree_base_addr,
		     (unsigned long) ree_length,
		     (unsigned long) ree_tag);

		if ((ree_base_addr % 0x200000) != 0) {
			SMC_RET1(handle, SMC_UNK);
		}

		if ((ree_length % 0x200000) != 0) {
			SMC_RET1(handle, SMC_UNK);
		}

		ree_info = true;

		/* Do not Yield */
		SMC_RET4(handle, 0, 0, 0, 0);

		break;

	case SMC_GET_SHAREDMEM:
		if (ree_info) {
			x1 = (1U << 16) | ree_tag;
			x2 = ree_base_addr & 0xFFFFFFFF;
			x3 = (ree_base_addr >> 32) & 0xFFFFFFFF;
			x4 = ree_length & 0xFFFFFFFF;
			SMC_RET4(handle, x1, x2, x3, x4);
		} else {
			SMC_RET4(handle, 0, 0, 0, 0);
		}

		break;

	case SMC_ACTION_FROM_NS:
		if (ns == 0) {
			SMC_RET1(handle, SMC_UNK);
		}

		if (SPD_PNCD_S_IRQ < MIN_PPI_ID) {
			plat_ic_raise_s_el1_sgi(SPD_PNCD_S_IRQ,
						pncd_sp_context.mpidr);
		} else {
			plat_ic_set_interrupt_pending(SPD_PNCD_S_IRQ);
		}

		SMC_RET0(handle);

		break;

	case SMC_ACTION_FROM_S:
		if (ns != 0) {
			SMC_RET1(handle, SMC_UNK);
		}

		if (SPD_PNCD_NS_IRQ < MIN_PPI_ID) {
			/*
			 * NS SGI is sent to the same core as the one running
			 * PNC
			 */
			plat_ic_raise_ns_sgi(SPD_PNCD_NS_IRQ, read_mpidr());
		} else {
			plat_ic_set_interrupt_pending(SPD_PNCD_NS_IRQ);
		}

		SMC_RET0(handle);

		break;

	case SMC_YIELD:
		assert(handle == cm_get_context(ns != 0 ? NON_SECURE : SECURE));
		handle = pncd_context_switch_to(ns != 0 ? SECURE : NON_SECURE);

		assert(handle != NULL);

		SMC_RET0(handle);

		break;

	default:
		INFO("Unknown smc: %x\n", smc_fid);
		break;
	}

	return plat_pncd_smc_handler(smc_fid, x1, x2, x3, x4,
				     cookie, handle, flags);
}

static uintptr_t pncd_smc_handler(uint32_t smc_fid,
		u_register_t x1,
		u_register_t x2,
		u_register_t x3,
		u_register_t x4,
		void *cookie,
		void *handle,
		u_register_t flags)
{
	uintptr_t ret;

	/* SMC handling is serialized */
	spin_lock(&smc_handler_lock);
	ret = pncd_smc_handler_unsafe(smc_fid, x1, x2, x3, x4, cookie, handle,
								  flags);
	spin_unlock(&smc_handler_lock);

	return ret;
}

/* Define a SPD runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	pncd_fast,
	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_FAST,
	pncd_setup,
	pncd_smc_handler
);

/* Define a SPD runtime service descriptor for standard SMC calls */
DECLARE_RT_SVC(
	pncd_std,
	OEN_TOS_START,
	OEN_TOS_END,
	SMC_TYPE_YIELD,
	NULL,
	pncd_smc_handler
);
