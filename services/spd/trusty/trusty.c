/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h> /* for context_mgmt.h */
#include <bl31.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <debug.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <runtime_svc.h>
#include <string.h>

#include "sm_err.h"
#include "smcall.h"

/* macro to check if Hypervisor is enabled in the HCR_EL2 register */
#define HYP_ENABLE_FLAG		0x286001

/* length of Trusty's input parameters (in bytes) */
#define TRUSTY_PARAMS_LEN_BYTES	(4096*2)

struct trusty_stack {
	uint8_t space[PLATFORM_STACK_SIZE] __aligned(16);
	uint32_t end;
};

struct trusty_cpu_ctx {
	cpu_context_t	cpu_ctx;
	void		*saved_sp;
	uint32_t	saved_security_state;
	int		fiq_handler_active;
	uint64_t	fiq_handler_pc;
	uint64_t	fiq_handler_cpsr;
	uint64_t	fiq_handler_sp;
	uint64_t	fiq_pc;
	uint64_t	fiq_cpsr;
	uint64_t	fiq_sp_el1;
	gp_regs_t	fiq_gpregs;
	struct trusty_stack	secure_stack;
};

struct args {
	uint64_t	r0;
	uint64_t	r1;
	uint64_t	r2;
	uint64_t	r3;
	uint64_t	r4;
	uint64_t	r5;
	uint64_t	r6;
	uint64_t	r7;
};

struct trusty_cpu_ctx trusty_cpu_ctx[PLATFORM_CORE_COUNT];

struct args trusty_init_context_stack(void **sp, void *new_stack);
struct args trusty_context_switch_helper(void **sp, void *smc_params);

static uint32_t current_vmid;

static struct trusty_cpu_ctx *get_trusty_ctx(void)
{
	return &trusty_cpu_ctx[plat_my_core_pos()];
}

static uint32_t is_hypervisor_mode(void)
{
	uint64_t hcr = read_hcr();

	return !!(hcr & HYP_ENABLE_FLAG);
}

static struct args trusty_context_switch(uint32_t security_state, uint64_t r0,
					 uint64_t r1, uint64_t r2, uint64_t r3)
{
	struct args ret;
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();
	struct trusty_cpu_ctx *ctx_smc;

	assert(ctx->saved_security_state != security_state);

	ret.r7 = 0;
	if (is_hypervisor_mode()) {
		/* According to the ARM DEN0028A spec, VMID is stored in x7 */
		ctx_smc = cm_get_context(NON_SECURE);
		assert(ctx_smc);
		ret.r7 = SMC_GET_GP(ctx_smc, CTX_GPREG_X7);
	}
	/* r4, r5, r6 reserved for future use. */
	ret.r6 = 0;
	ret.r5 = 0;
	ret.r4 = 0;
	ret.r3 = r3;
	ret.r2 = r2;
	ret.r1 = r1;
	ret.r0 = r0;

	/*
	 * To avoid the additional overhead in PSCI flow, skip FP context
	 * saving/restoring in case of CPU suspend and resume, asssuming that
	 * when it's needed the PSCI caller has preserved FP context before
	 * going here.
	 */
#if CTX_INCLUDE_FPREGS
	if (r0 != SMC_FC_CPU_SUSPEND && r0 != SMC_FC_CPU_RESUME)
		fpregs_context_save(get_fpregs_ctx(cm_get_context(security_state)));
#endif
	cm_el1_sysregs_context_save(security_state);

	ctx->saved_security_state = security_state;
	ret = trusty_context_switch_helper(&ctx->saved_sp, &ret);

	assert(ctx->saved_security_state == !security_state);

	cm_el1_sysregs_context_restore(security_state);
#if CTX_INCLUDE_FPREGS
	if (r0 != SMC_FC_CPU_SUSPEND && r0 != SMC_FC_CPU_RESUME)
		fpregs_context_restore(get_fpregs_ctx(cm_get_context(security_state)));
#endif

	cm_set_next_eret_context(security_state);

	return ret;
}

static uint64_t trusty_fiq_handler(uint32_t id,
				   uint32_t flags,
				   void *handle,
				   void *cookie)
{
	struct args ret;
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();

	assert(!is_caller_secure(flags));

	ret = trusty_context_switch(NON_SECURE, SMC_FC_FIQ_ENTER, 0, 0, 0);
	if (ret.r0) {
		SMC_RET0(handle);
	}

	if (ctx->fiq_handler_active) {
		INFO("%s: fiq handler already active\n", __func__);
		SMC_RET0(handle);
	}

	ctx->fiq_handler_active = 1;
	memcpy(&ctx->fiq_gpregs, get_gpregs_ctx(handle), sizeof(ctx->fiq_gpregs));
	ctx->fiq_pc = SMC_GET_EL3(handle, CTX_ELR_EL3);
	ctx->fiq_cpsr = SMC_GET_EL3(handle, CTX_SPSR_EL3);
	ctx->fiq_sp_el1 = read_ctx_reg(get_sysregs_ctx(handle), CTX_SP_EL1);

	write_ctx_reg(get_sysregs_ctx(handle), CTX_SP_EL1, ctx->fiq_handler_sp);
	cm_set_elr_spsr_el3(NON_SECURE, ctx->fiq_handler_pc, ctx->fiq_handler_cpsr);

	SMC_RET0(handle);
}

static uint64_t trusty_set_fiq_handler(void *handle, uint64_t cpu,
			uint64_t handler, uint64_t stack)
{
	struct trusty_cpu_ctx *ctx;

	if (cpu >= PLATFORM_CORE_COUNT) {
		ERROR("%s: cpu %ld >= %d\n", __func__, cpu, PLATFORM_CORE_COUNT);
		return SM_ERR_INVALID_PARAMETERS;
	}

	ctx = &trusty_cpu_ctx[cpu];
	ctx->fiq_handler_pc = handler;
	ctx->fiq_handler_cpsr = SMC_GET_EL3(handle, CTX_SPSR_EL3);
	ctx->fiq_handler_sp = stack;

	SMC_RET1(handle, 0);
}

static uint64_t trusty_get_fiq_regs(void *handle)
{
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();
	uint64_t sp_el0 = read_ctx_reg(&ctx->fiq_gpregs, CTX_GPREG_SP_EL0);

	SMC_RET4(handle, ctx->fiq_pc, ctx->fiq_cpsr, sp_el0, ctx->fiq_sp_el1);
}

static uint64_t trusty_fiq_exit(void *handle, uint64_t x1, uint64_t x2, uint64_t x3)
{
	struct args ret;
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();

	if (!ctx->fiq_handler_active) {
		NOTICE("%s: fiq handler not active\n", __func__);
		SMC_RET1(handle, SM_ERR_INVALID_PARAMETERS);
	}

	ret = trusty_context_switch(NON_SECURE, SMC_FC_FIQ_EXIT, 0, 0, 0);
	if (ret.r0 != 1) {
		INFO("%s(%p) SMC_FC_FIQ_EXIT returned unexpected value, %ld\n",
		       __func__, handle, ret.r0);
	}

	/*
	 * Restore register state to state recorded on fiq entry.
	 *
	 * x0, sp_el1, pc and cpsr need to be restored because el1 cannot
	 * restore them.
	 *
	 * x1-x4 and x8-x17 need to be restored here because smc_handler64
	 * corrupts them (el1 code also restored them).
	 */
	memcpy(get_gpregs_ctx(handle), &ctx->fiq_gpregs, sizeof(ctx->fiq_gpregs));
	ctx->fiq_handler_active = 0;
	write_ctx_reg(get_sysregs_ctx(handle), CTX_SP_EL1, ctx->fiq_sp_el1);
	cm_set_elr_spsr_el3(NON_SECURE, ctx->fiq_pc, ctx->fiq_cpsr);

	SMC_RET0(handle);
}

static uint64_t trusty_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	struct args ret;
	uint32_t vmid = 0;
	entry_point_info_t *ep_info = bl31_plat_get_next_image_ep_info(SECURE);

	/*
	 * Return success for SET_ROT_PARAMS if Trusty is not present, as
	 * Verified Boot is not even supported and returning success here
	 * would not compromise the boot process.
	 */
	if (!ep_info && (smc_fid == SMC_YC_SET_ROT_PARAMS)) {
		SMC_RET1(handle, 0);
	} else if (!ep_info) {
		SMC_RET1(handle, SMC_UNK);
	}

	if (is_caller_secure(flags)) {
		if (smc_fid == SMC_YC_NS_RETURN) {
			ret = trusty_context_switch(SECURE, x1, 0, 0, 0);
			SMC_RET8(handle, ret.r0, ret.r1, ret.r2, ret.r3,
				 ret.r4, ret.r5, ret.r6, ret.r7);
		}
		INFO("%s (0x%x, 0x%lx, 0x%lx, 0x%lx, 0x%lx, %p, %p, 0x%lx) \
		     cpu %d, unknown smc\n",
		     __func__, smc_fid, x1, x2, x3, x4, cookie, handle, flags,
		     plat_my_core_pos());
		SMC_RET1(handle, SMC_UNK);
	} else {
		switch (smc_fid) {
		case SMC_FC64_SET_FIQ_HANDLER:
			return trusty_set_fiq_handler(handle, x1, x2, x3);
		case SMC_FC64_GET_FIQ_REGS:
			return trusty_get_fiq_regs(handle);
		case SMC_FC_FIQ_EXIT:
			return trusty_fiq_exit(handle, x1, x2, x3);
		default:
			if (is_hypervisor_mode())
				vmid = SMC_GET_GP(handle, CTX_GPREG_X7);

			if ((current_vmid != 0) && (current_vmid != vmid)) {
				/* This message will cause SMC mechanism
				 * abnormal in multi-guest environment.
				 * Change it to WARN in case you need it.
				 */
				VERBOSE("Previous SMC not finished.\n");
				SMC_RET1(handle, SM_ERR_BUSY);
			}
			current_vmid = vmid;
			ret = trusty_context_switch(NON_SECURE, smc_fid, x1,
				x2, x3);
			current_vmid = 0;
			SMC_RET1(handle, ret.r0);
		}
	}
}

static int32_t trusty_init(void)
{
	void el3_exit(void);
	entry_point_info_t *ep_info;
	struct args zero_args = {0};
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();
	uint32_t cpu = plat_my_core_pos();
	int reg_width = GET_RW(read_ctx_reg(get_el3state_ctx(&ctx->cpu_ctx),
			       CTX_SPSR_EL3));

	/*
	 * Get information about the Trusty image. Its absence is a critical
	 * failure.
	 */
	ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	assert(ep_info);

	cm_el1_sysregs_context_save(NON_SECURE);

	cm_set_context(&ctx->cpu_ctx, SECURE);
	cm_init_my_context(ep_info);

	/*
	 * Adjust secondary cpu entry point for 32 bit images to the
	 * end of exeption vectors
	 */
	if ((cpu != 0) && (reg_width == MODE_RW_32)) {
		INFO("trusty: cpu %d, adjust entry point to 0x%lx\n",
		     cpu, ep_info->pc + (1U << 5));
		cm_set_elr_el3(SECURE, ep_info->pc + (1U << 5));
	}

	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	ctx->saved_security_state = ~0; /* initial saved state is invalid */
	trusty_init_context_stack(&ctx->saved_sp, &ctx->secure_stack.end);

	trusty_context_switch_helper(&ctx->saved_sp, &zero_args);

	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	return 0;
}

static void trusty_cpu_suspend(void)
{
	struct args ret;

	ret = trusty_context_switch(NON_SECURE, SMC_FC_CPU_SUSPEND, 0, 0, 0);
	if (ret.r0 != 0) {
		INFO("%s: cpu %d, SMC_FC_CPU_SUSPEND returned unexpected value, %ld\n",
		     __func__, plat_my_core_pos(), ret.r0);
	}
}

static void trusty_cpu_resume(void)
{
	struct args ret;

	ret = trusty_context_switch(NON_SECURE, SMC_FC_CPU_RESUME, 0, 0, 0);
	if (ret.r0 != 0) {
		INFO("%s: cpu %d, SMC_FC_CPU_RESUME returned unexpected value, %ld\n",
		     __func__, plat_my_core_pos(), ret.r0);
	}
}

static int32_t trusty_cpu_off_handler(uint64_t unused)
{
	trusty_cpu_suspend();

	return 0;
}

static void trusty_cpu_on_finish_handler(uint64_t unused)
{
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();

	if (!ctx->saved_sp) {
		trusty_init();
	} else {
		trusty_cpu_resume();
	}
}

static void trusty_cpu_suspend_handler(uint64_t unused)
{
	trusty_cpu_suspend();
}

static void trusty_cpu_suspend_finish_handler(uint64_t unused)
{
	trusty_cpu_resume();
}

static const spd_pm_ops_t trusty_pm = {
	.svc_off = trusty_cpu_off_handler,
	.svc_suspend = trusty_cpu_suspend_handler,
	.svc_on_finish = trusty_cpu_on_finish_handler,
	.svc_suspend_finish = trusty_cpu_suspend_finish_handler,
};

static int32_t trusty_setup(void)
{
	entry_point_info_t *ep_info;
	uint32_t flags;
	int ret;

	/* Get trusty's entry point info */
	ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!ep_info) {
		INFO("Trusty image missing.\n");
		return -1;
	}

	/* Trusty runs in AARCH64 mode */
	SET_PARAM_HEAD(ep_info, PARAM_EP, VERSION_1, SECURE | EP_ST_ENABLE);
	ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX, DISABLE_ALL_EXCEPTIONS);

	/*
	 * arg0 = TZDRAM aperture available for BL32
	 * arg1 = BL32 boot params
	 * arg2 = BL32 boot params length
	 */
	ep_info->args.arg1 = ep_info->args.arg2;
	ep_info->args.arg2 = TRUSTY_PARAMS_LEN_BYTES;

	/* register init handler */
	bl31_register_bl32_init(trusty_init);

	/* register power management hooks */
	psci_register_spd_pm_hook(&trusty_pm);

	/* register interrupt handler */
	flags = 0;
	set_interrupt_rm_flag(flags, NON_SECURE);
	ret = register_interrupt_type_handler(INTR_TYPE_S_EL1,
					      trusty_fiq_handler,
					      flags);
	if (ret)
		ERROR("trusty: failed to register fiq handler, ret = %d\n", ret);

	return 0;
}

/* Define a SPD runtime service descriptor for fast SMC calls */
DECLARE_RT_SVC(
	trusty_fast,

	OEN_TOS_START,
	SMC_ENTITY_SECURE_MONITOR,
	SMC_TYPE_FAST,
	trusty_setup,
	trusty_smc_handler
);

/* Define a SPD runtime service descriptor for yielding SMC calls */
DECLARE_RT_SVC(
	trusty_std,

	OEN_TAP_START,
	SMC_ENTITY_SECURE_MONITOR,
	SMC_TYPE_YIELD,
	NULL,
	trusty_smc_handler
);
