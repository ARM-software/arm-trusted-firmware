/*
 * Copyright (c) 2013-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <arch_features.h>
#include <bl31/interrupt_mgmt.h>
#include <bl31/sync_handle.h>
#include <bl31/ea_handle.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <context.h>
#include <lib/cpus/cpu_ops.h>
#include <plat/common/platform.h>

/*******************************************************************************
 * The 'rt_svc_descs' array holds the runtime service descriptors exported by
 * services by placing them in the 'rt_svc_descs' linker section.
 * The 'rt_svc_descs_indices' array holds the index of a descriptor in the
 * 'rt_svc_descs' array. When an SMC arrives, the OEN[29:24] bits and the call
 * type[31] bit in the function id are combined to get an index into the
 * 'rt_svc_descs_indices' array. This gives the index of the descriptor in the
 * 'rt_svc_descs' array which contains the SMC handler.
 ******************************************************************************/
uint8_t rt_svc_descs_indices[MAX_RT_SVCS];

void __dead2 report_unhandled_exception(void);

#define RT_SVC_DECS_NUM		((RT_SVC_DESCS_END - RT_SVC_DESCS_START)\
					/ sizeof(rt_svc_desc_t))

static bool get_handler_for_smc_fid(uint32_t smc_fid, rt_svc_handle_t *handler)
{
	unsigned int index;
	unsigned int idx;
	const rt_svc_desc_t *rt_svc_descs;

	idx = get_unique_oen_from_smc_fid(smc_fid);
	assert(idx < MAX_RT_SVCS);
	index = rt_svc_descs_indices[idx];

	if (index >= RT_SVC_DECS_NUM)
		return false;

	rt_svc_descs = (rt_svc_desc_t *) RT_SVC_DESCS_START;
	assert(handler != NULL);
	*handler = rt_svc_descs[index].handle;
	assert(*handler != NULL);

	return true;
}

#if __aarch64__
#include <lib/extensions/ras_arch.h>

#if FFH_SUPPORT
static void ea_proceed(uint32_t ea_reason, u_register_t esr_el3, cpu_context_t *ctx)
{
	/*
	 * If it is a double fault invoke platform handler.  Double fault
	 * scenario would arise when platform is handling a fault in lower EL
	 * using plat_ea_handler() and another fault happens which would trap
	 * into EL3 as FFH_SUPPORT is enabled for the platform.
	 */
	el3_state_t *state = get_el3state_ctx(ctx);
	if (read_ctx_reg(state, CTX_DOUBLE_FAULT_ESR) != 0) {
		return plat_handle_double_fault(ea_reason, esr_el3);
	}

	/*
	 * Save CTX_DOUBLE_FAULT_ESR, so that if another fault happens in lower
	 * EL, we catch it as DoubleFault in next invocation of ea_proceed()
	 * along with preserving original ESR_EL3.
	 */
	write_ctx_reg(state, CTX_DOUBLE_FAULT_ESR, esr_el3);

	/* Call platform External Abort handler. */
	plat_ea_handler(ea_reason, esr_el3, NULL, ctx, read_scr_el3() & SCR_NS_BIT);

	/* Clear Double Fault storage */
	write_ctx_reg(state, CTX_DOUBLE_FAULT_ESR, 0);
}

/*
 * This function handles SErrors from lower ELs.
 *
 * It delegates the handling of the EA to platform handler, and upon
 * successfully handling the EA, exits EL3
 */
void handler_lower_el_async_ea(cpu_context_t *ctx)
{
	u_register_t esr_el3 = read_esr_el3();

#if ENABLE_FEAT_RAS
	/*  should only be invoked for SError */
	assert(EXTRACT(ESR_EC, esr_el3) == EC_SERROR);

	/*
	 * Check for Implementation Defined Syndrome. If so, skip checking
	 * Uncontainable error type from the syndrome as the format is unknown.
	 */
	if ((esr_el3 & SERROR_IDS_BIT) != 0) {
		/* AET only valid when DFSC is 0x11. Route to platform fatal
		 * error handler if it is an uncontainable error type */
		if (EXTRACT(EABORT_DFSC, esr_el3) == DFSC_SERROR &&
		    EXTRACT(EABORT_AET, esr_el3) == ERROR_STATUS_UET_UC) {
			return plat_handle_uncontainable_ea();
		}
	}
#endif
	return ea_proceed(ERROR_EA_ASYNC, esr_el3, ctx);
}

#endif /* FFH_SUPPORT */

/*
 * This function handles FIQ or IRQ interrupts i.e. EL3, S-EL1 and NS
 * interrupts.
 */
void handler_interrupt_exception(cpu_context_t *ctx)
{
	/*
	 * Find out whether this is a valid interrupt type.
	 * If the interrupt controller reports a spurious interrupt then return
	 * to where we came from.
	 */
	uint32_t type = plat_ic_get_pending_interrupt_type();
	if (type == INTR_TYPE_INVAL) {
		return;
	}

	/*
	 * Get the registered handler for this interrupt type.
	 * A NULL return value could be 'cause of the following conditions:
	 *
	 * a. An interrupt of a type was routed correctly but a handler for its
	 *    type was not registered.
	 *
	 * b. An interrupt of a type was not routed correctly so a handler for
	 *    its type was not registered.
	 *
	 * c. An interrupt of a type was routed correctly to EL3, but was
	 *    deasserted before its pending state could be read. Another
	 *    interrupt of a different type pended at the same time and its
	 *    type was reported as pending instead. However, a handler for this
	 *    type was not registered.
	 *
	 * a. and b. can only happen due to a programming error. The
	 * occurrence of c. could be beyond the control of Trusted Firmware.
	 * It makes sense to return from this exception instead of reporting an
	 * error.
	 */
	interrupt_type_handler_t handler = get_interrupt_type_handler(type);
	if (handler == NULL) {
		return;
	}

	handler(INTR_ID_UNAVAILABLE, read_scr_el3() & SCR_NS_BIT, ctx, NULL);
}

static void smc_unknown(cpu_context_t *ctx)
{
	/*
	 * Unknown SMC call. Populate return value with SMC_UNK and call
	 * el3_exit() which will restore the remaining architectural state
	 * i.e., SYS, GP and PAuth registers(if any) prior to issuing the ERET
	 * to the desired lower EL.
	 */
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X0, SMC_UNK);
}

static u_register_t get_flags(uint32_t smc_fid, u_register_t scr_el3)
{
	u_register_t flags = 0;

	/* Copy SCR_EL3.NS bit to the flag to indicate caller's security */
	flags |= scr_el3 & SCR_NS_BIT;
#if ENABLE_RME
	/* Copy SCR_EL3.NSE bit to the flag to indicate caller's security Shift
	 * copied SCR_EL3.NSE bit by 5 to create space for SCR_EL3.NS bit. Bit 5
	 * of the flag corresponds to the SCR_EL3.NSE bit.
	 */
	flags |= ((scr_el3 & SCR_NSE_BIT) >> SCR_NSE_SHIFT) << 5;
#endif /* ENABLE_RME */

	/*
	 * Per SMCCCv1.3 a caller can set the SVE hint bit in the SMC FID passed
	 * through x0. Copy the SVE hint bit to flags and mask the bit in
	 * smc_fid passed to the standard service dispatcher.  A
	 * service/dispatcher can retrieve the SVE hint bit state from flags
	 * using the appropriate helper.
	 */
	flags |= smc_fid & MASK(FUNCID_SVE_HINT);

	return flags;
}

static void sync_handler(cpu_context_t *ctx, uint32_t smc_fid)
{
	u_register_t scr_el3 = read_scr_el3();
	rt_svc_handle_t handler;

	/*
	 * Per SMCCC documentation, bits [23:17] must be zero for Fast SMCs.
	 * Other values are reserved for future use. Ensure that these bits are
	 * zeroes, if not report as unknown SMC.
	 */
	if (EXTRACT(FUNCID_TYPE, smc_fid) == SMC_TYPE_FAST &&
	    EXTRACT(FUNCID_FC_RESERVED, smc_fid) != 0) {
		return smc_unknown(ctx);
	}

	smc_fid &= ~MASK(FUNCID_SVE_HINT);

	/* Get the descriptor using the index */
	if (!get_handler_for_smc_fid(smc_fid, &handler)) {
		return smc_unknown(ctx);
	}

	u_register_t x1, x2, x3, x4;
	get_smc_params_from_ctx(ctx, x1, x2, x3, x4);
	handler(smc_fid, x1, x2, x3, x4, NULL, ctx, get_flags(smc_fid, scr_el3));
}

void handler_sync_exception(cpu_context_t *ctx)
{
	uint32_t smc_fid = read_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X0);
	u_register_t esr_el3 = read_esr_el3();
	u_register_t exc_class = EXTRACT(ESR_EC, esr_el3);
	el3_state_t *state = get_el3state_ctx(ctx);

	if (exc_class == EC_AARCH32_SMC || exc_class == EC_AARCH64_SMC) {
		if (exc_class == EC_AARCH32_SMC && EXTRACT(FUNCID_CC, smc_fid) != 0) {
			return smc_unknown(ctx);
		}
		return sync_handler(ctx, smc_fid);
	} else if (exc_class == EC_AARCH64_SYS) {
		int ret = handle_sysreg_trap(esr_el3, ctx, get_flags(smc_fid, read_scr_el3()));

		/* unhandled trap, UNDEF injection into lower. The support is
		 * only provided for lower EL in AArch64 mode. */
		if (ret == TRAP_RET_UNHANDLED) {
			if (read_spsr_el3() & MASK(SPSR_M)) {
				lower_el_panic();
			}
			inject_undef64(ctx);
		} else if (ret == TRAP_RET_CONTINUE) {
			/* advance the PC to continue after the instruction */
			write_ctx_reg(state, CTX_ELR_EL3, read_ctx_reg(state, CTX_ELR_EL3) + 4);
		} /* otherwise return to the trapping instruction (repeating it) */
		return;
	/* If FFH Support then try to handle lower EL EA exceptions. */
	} else if ((exc_class == EC_IABORT_LOWER_EL || exc_class == EC_DABORT_LOWER_EL)
		    && ((read_ctx_reg(state, CTX_SCR_EL3) & SCR_EA_BIT) != 0UL)) {
#if FFH_SUPPORT
		/*
		 * Check for Uncontainable error type. If so, route to the
		 * platform fatal error handler rather than the generic EA one.
		 */
		if (is_feat_ras_supported() &&
		    (EXTRACT(EABORT_SET, esr_el3) == ERROR_STATUS_SET_UC ||
		     EXTRACT(EABORT_DFSC, esr_el3) == SYNC_EA_FSC)) {
			return plat_handle_uncontainable_ea();
		}
		/* Setup exception class and syndrome arguments for platform handler */
		return ea_proceed(ERROR_EA_SYNC, esr_el3, ctx);
#endif /* FFH_SUPPORT */
	}

	/* Synchronous exceptions other than the above are unhandled */
	report_unhandled_exception();
}
#endif /* __aarch64__ */

/*******************************************************************************
 * Function to invoke the registered `handle` corresponding to the smc_fid in
 * AArch32 mode.
 ******************************************************************************/
uintptr_t handle_runtime_svc(uint32_t smc_fid,
			     void *cookie,
			     void *handle,
			     unsigned int flags)
{
	u_register_t x1, x2, x3, x4;
	rt_svc_handle_t handler;

	assert(handle != NULL);

	if (!get_handler_for_smc_fid(smc_fid, &handler)) {
		SMC_RET1(handle, SMC_UNK);
	}

	get_smc_params_from_ctx(handle, x1, x2, x3, x4);

	return handler(smc_fid, x1, x2, x3, x4, cookie, handle, flags);
}

/*******************************************************************************
 * Simple routine to sanity check a runtime service descriptor before using it
 ******************************************************************************/
static int32_t validate_rt_svc_desc(const rt_svc_desc_t *desc)
{
	if (desc == NULL) {
		return -EINVAL;
	}
	if (desc->start_oen > desc->end_oen) {
		return -EINVAL;
	}
	if (desc->end_oen >= OEN_LIMIT) {
		return -EINVAL;
	}
	if ((desc->call_type != SMC_TYPE_FAST) &&
	    (desc->call_type != SMC_TYPE_YIELD)) {
		return -EINVAL;
	}
	/* A runtime service having no init or handle function doesn't make sense */
	if ((desc->init == NULL) && (desc->handle == NULL)) {
		return -EINVAL;
	}
	return 0;
}

/*******************************************************************************
 * This function calls the initialisation routine in the descriptor exported by
 * a runtime service. Once a descriptor has been validated, its start & end
 * owning entity numbers and the call type are combined to form a unique oen.
 * The unique oen is used as an index into the 'rt_svc_descs_indices' array.
 * The index of the runtime service descriptor is stored at this index.
 ******************************************************************************/
void __init runtime_svc_init(void)
{
	int rc = 0;
	uint8_t index, start_idx, end_idx;
	rt_svc_desc_t *rt_svc_descs;

	/* Assert the number of descriptors detected are less than maximum indices */
	assert((RT_SVC_DESCS_END >= RT_SVC_DESCS_START) &&
			(RT_SVC_DECS_NUM < MAX_RT_SVCS));

	/* If no runtime services are implemented then simply bail out */
	if (RT_SVC_DECS_NUM == 0U) {
		return;
	}
	/* Initialise internal variables to invalid state */
	(void)memset(rt_svc_descs_indices, -1, sizeof(rt_svc_descs_indices));

	rt_svc_descs = (rt_svc_desc_t *) RT_SVC_DESCS_START;
	for (index = 0U; index < RT_SVC_DECS_NUM; index++) {
		rt_svc_desc_t *service = &rt_svc_descs[index];

		/*
		 * An invalid descriptor is an error condition since it is
		 * difficult to predict the system behaviour in the absence
		 * of this service.
		 */
		rc = validate_rt_svc_desc(service);
		if (rc != 0) {
			ERROR("Invalid runtime service descriptor %p\n",
				(void *) service);
			panic();
		}

		/*
		 * The runtime service may have separate rt_svc_desc_t
		 * for its fast smc and yielding smc. Since the service itself
		 * need to be initialized only once, only one of them will have
		 * an initialisation routine defined. Call the initialisation
		 * routine for this runtime service, if it is defined.
		 */
		if (service->init != NULL) {
			rc = service->init();
			if (rc != 0) {
				ERROR("Error initializing runtime service %s\n",
						service->name);
				continue;
			}
		}

		/*
		 * Fill the indices corresponding to the start and end
		 * owning entity numbers with the index of the
		 * descriptor which will handle the SMCs for this owning
		 * entity range.
		 */
		start_idx = (uint8_t)get_unique_oen(service->start_oen,
						    service->call_type);
		end_idx = (uint8_t)get_unique_oen(service->end_oen,
						  service->call_type);
		assert(start_idx <= end_idx);
		assert(end_idx < MAX_RT_SVCS);
		for (; start_idx <= end_idx; start_idx++) {
			rt_svc_descs_indices[start_idx] = index;
		}
	}
}
