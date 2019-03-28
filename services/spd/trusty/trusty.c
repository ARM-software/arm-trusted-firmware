/*
 * Copyright (c) 2016-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <stdbool.h>
#include <string.h>

#include <arch_helpers.h>
#include <bl31/bl31.h>
#include <bl31/interrupt_mgmt.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <plat/common/platform.h>

#include "sm_err.h"
#include "smcall.h"

/* macro to check if Hypervisor is enabled in the HCR_EL2 register */
#define HYP_ENABLE_FLAG		0x286001U

/* length of Trusty's input parameters (in bytes) */
#define TRUSTY_PARAMS_LEN_BYTES	(4096U * 2)

struct trusty_stack {
	uint8_t space[PLATFORM_STACK_SIZE] __aligned(16);
	uint32_t end;
};

struct trusty_cpu_ctx {
	cpu_context_t	cpu_ctx;
	void		*saved_sp;
	uint32_t	saved_security_state;
	int32_t		fiq_handler_active;
	uint64_t	fiq_handler_pc;
	uint64_t	fiq_handler_cpsr;
	uint64_t	fiq_handler_sp;
	uint64_t	fiq_pc;
	uint64_t	fiq_cpsr;
	uint64_t	fiq_sp_el1;
	gp_regs_t	fiq_gpregs;
	struct trusty_stack	secure_stack;
};

struct smc_args {
	uint64_t	r0;
	uint64_t	r1;
	uint64_t	r2;
	uint64_t	r3;
	uint64_t	r4;
	uint64_t	r5;
	uint64_t	r6;
	uint64_t	r7;
};

static struct trusty_cpu_ctx trusty_cpu_ctx[PLATFORM_CORE_COUNT];

struct smc_args trusty_init_context_stack(void **sp, void *new_stack);
struct smc_args trusty_context_switch_helper(void **sp, void *smc_params);

static uint32_t current_vmid;

static struct trusty_cpu_ctx *get_trusty_ctx(void)
{
	return &trusty_cpu_ctx[plat_my_core_pos()];
}

static bool is_hypervisor_mode(void)
{
	uint64_t hcr = read_hcr();

	return ((hcr & HYP_ENABLE_FLAG) != 0U) ? true : false;
}

static struct smc_args trusty_context_switch(uint32_t security_state, uint64_t r0,
					 uint64_t r1, uint64_t r2, uint64_t r3)
{
	struct smc_args args, ret_args;
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();
	struct trusty_cpu_ctx *ctx_smc;

	assert(ctx->saved_security_state != security_state);

	args.r7 = 0;
	if (is_hypervisor_mode()) {
		/* According to the ARM DEN0028A spec, VMID is stored in x7 */
		ctx_smc = cm_get_context(NON_SECURE);
		assert(ctx_smc != NULL);
		args.r7 = SMC_GET_GP(ctx_smc, CTX_GPREG_X7);
	}
	/* r4, r5, r6 reserved for future use. */
	args.r6 = 0;
	args.r5 = 0;
	args.r4 = 0;
	args.r3 = r3;
	args.r2 = r2;
	args.r1 = r1;
	args.r0 = r0;

	/*
	 * To avoid the additional overhead in PSCI flow, skip FP context
	 * saving/restoring in case of CPU suspend and resume, assuming that
	 * when it's needed the PSCI caller has preserved FP context before
	 * going here.
	 */
	if (r0 != SMC_FC_CPU_SUSPEND && r0 != SMC_FC_CPU_RESUME)
		fpregs_context_save(get_fpregs_ctx(cm_get_context(security_state)));
	cm_el1_sysregs_context_save(security_state);

	ctx->saved_security_state = security_state;
	ret_args = trusty_context_switch_helper(&ctx->saved_sp, &args);

	assert(ctx->saved_security_state == ((security_state == 0U) ? 1U : 0U));

	cm_el1_sysregs_context_restore(security_state);
	if (r0 != SMC_FC_CPU_SUSPEND && r0 != SMC_FC_CPU_RESUME)
		fpregs_context_restore(get_fpregs_ctx(cm_get_context(security_state)));

	cm_set_next_eret_context(security_state);

	return ret_args;
}

static uint64_t trusty_fiq_handler(uint32_t id,
				   uint32_t flags,
				   void *handle,
				   void *cookie)
{
	struct smc_args ret;
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();

	assert(!is_caller_secure(flags));

	ret = trusty_context_switch(NON_SECURE, SMC_FC_FIQ_ENTER, 0, 0, 0);
	if (ret.r0 != 0U) {
		SMC_RET0(handle);
	}

	if (ctx->fiq_handler_active != 0) {
		INFO("%s: fiq handler already active\n", __func__);
		SMC_RET0(handle);
	}

	ctx->fiq_handler_active = 1;
	(void)memcpy(&ctx->fiq_gpregs, get_gpregs_ctx(handle), sizeof(ctx->fiq_gpregs));
	ctx->fiq_pc = SMC_GET_EL3(handle, CTX_ELR_EL3);
	ctx->fiq_cpsr = SMC_GET_EL3(handle, CTX_SPSR_EL3);
	ctx->fiq_sp_el1 = read_ctx_reg(get_sysregs_ctx(handle), CTX_SP_EL1);

	write_ctx_reg(get_sysregs_ctx(handle), CTX_SP_EL1, ctx->fiq_handler_sp);
	cm_set_elr_spsr_el3(NON_SECURE, ctx->fiq_handler_pc, (uint32_t)ctx->fiq_handler_cpsr);

	SMC_RET0(handle);
}

static uint64_t trusty_set_fiq_handler(void *handle, uint64_t cpu,
			uint64_t handler, uint64_t stack)
{
	struct trusty_cpu_ctx *ctx;

	if (cpu >= (uint64_t)PLATFORM_CORE_COUNT) {
		ERROR("%s: cpu %lld >= %d\n", __func__, cpu, PLATFORM_CORE_COUNT);
		return (uint64_t)SM_ERR_INVALID_PARAMETERS;
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
	struct smc_args ret;
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();

	if (ctx->fiq_handler_active == 0) {
		NOTICE("%s: fiq handler not active\n", __func__);
		SMC_RET1(handle, (uint64_t)SM_ERR_INVALID_PARAMETERS);
	}

	ret = trusty_context_switch(NON_SECURE, SMC_FC_FIQ_EXIT, 0, 0, 0);
	if (ret.r0 != 1U) {
		INFO("%s(%p) SMC_FC_FIQ_EXIT returned unexpected value, %lld\n",
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
	(void)memcpy(get_gpregs_ctx(handle), &ctx->fiq_gpregs, sizeof(ctx->fiq_gpregs));
	ctx->fiq_handler_active = 0;
	write_ctx_reg(get_sysregs_ctx(handle), CTX_SP_EL1, ctx->fiq_sp_el1);
	cm_set_elr_spsr_el3(NON_SECURE, ctx->fiq_pc, (uint32_t)ctx->fiq_cpsr);

	SMC_RET0(handle);
}

static uintptr_t trusty_smc_handler(uint32_t smc_fid,
			 u_register_t x1,
			 u_register_t x2,
			 u_register_t x3,
			 u_register_t x4,
			 void *cookie,
			 void *handle,
			 u_register_t flags)
{
	struct smc_args ret;
	uint32_t vmid = 0U;
	entry_point_info_t *ep_info = bl31_plat_get_next_image_ep_info(SECURE);

	/*
	 * Return success for SET_ROT_PARAMS if Trusty is not present, as
	 * Verified Boot is not even supported and returning success here
	 * would not compromise the boot process.
	 */
	if ((ep_info == NULL) && (smc_fid == SMC_YC_SET_ROT_PARAMS)) {
		SMC_RET1(handle, 0);
	} else if (ep_info == NULL) {
		SMC_RET1(handle, SMC_UNK);
	} else {
		; /* do nothing */
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
	entry_point_info_t *ep_info;
	struct smc_args zero_args = {0};
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();
	uint32_t cpu = plat_my_core_pos();
	uint64_t reg_width = GET_RW(read_ctx_reg(get_el3state_ctx(&ctx->cpu_ctx),
			       CTX_SPSR_EL3));

	/*
	 * Get information about the Trusty image. Its absence is a critical
	 * failure.
	 */
	ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	assert(ep_info != NULL);

	fpregs_context_save(get_fpregs_ctx(cm_get_context(NON_SECURE)));
	cm_el1_sysregs_context_save(NON_SECURE);

	cm_set_context(&ctx->cpu_ctx, SECURE);
	cm_init_my_context(ep_info);

	/*
	 * Adjust secondary cpu entry point for 32 bit images to the
	 * end of exception vectors
	 */
	if ((cpu != 0U) && (reg_width == MODE_RW_32)) {
		INFO("trusty: cpu %d, adjust entry point to 0x%lx\n",
		     cpu, ep_info->pc + (1U << 5));
		cm_set_elr_el3(SECURE, ep_info->pc + (1U << 5));
	}

	cm_el1_sysregs_context_restore(SECURE);
	fpregs_context_restore(get_fpregs_ctx(cm_get_context(SECURE)));
	cm_set_next_eret_context(SECURE);

	ctx->saved_security_state = ~0U; /* initial saved state is invalid */
	(void)trusty_init_context_stack(&ctx->saved_sp, &ctx->secure_stack.end);

	(void)trusty_context_switch_helper(&ctx->saved_sp, &zero_args);

	cm_el1_sysregs_context_restore(NON_SECURE);
	fpregs_context_restore(get_fpregs_ctx(cm_get_context(NON_SECURE)));
	cm_set_next_eret_context(NON_SECURE);

	return 1;
}

static void trusty_cpu_suspend(uint32_t off)
{
	struct smc_args ret;

	ret = trusty_context_switch(NON_SECURE, SMC_FC_CPU_SUSPEND, off, 0, 0);
	if (ret.r0 != 0U) {
		INFO("%s: cpu %d, SMC_FC_CPU_SUSPEND returned unexpected value, %lld\n",
		     __func__, plat_my_core_pos(), ret.r0);
	}
}

static void trusty_cpu_resume(uint32_t on)
{
	struct smc_args ret;

	ret = trusty_context_switch(NON_SECURE, SMC_FC_CPU_RESUME, on, 0, 0);
	if (ret.r0 != 0U) {
		INFO("%s: cpu %d, SMC_FC_CPU_RESUME returned unexpected value, %lld\n",
		     __func__, plat_my_core_pos(), ret.r0);
	}
}

static int32_t trusty_cpu_off_handler(u_register_t max_off_lvl)
{
	trusty_cpu_suspend(max_off_lvl);

	return 0;
}

static void trusty_cpu_on_finish_handler(u_register_t max_off_lvl)
{
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();

	if (ctx->saved_sp == NULL) {
		(void)trusty_init();
	} else {
		trusty_cpu_resume(max_off_lvl);
	}
}

static void trusty_cpu_suspend_handler(u_register_t max_off_lvl)
{
	trusty_cpu_suspend(max_off_lvl);
}

static void trusty_cpu_suspend_finish_handler(u_register_t max_off_lvl)
{
	trusty_cpu_resume(max_off_lvl);
}

static const spd_pm_ops_t trusty_pm = {
	.svc_off = trusty_cpu_off_handler,
	.svc_suspend = trusty_cpu_suspend_handler,
	.svc_on_finish = trusty_cpu_on_finish_handler,
	.svc_suspend_finish = trusty_cpu_suspend_finish_handler,
};

void plat_trusty_set_boot_args(aapcs64_params_t *args);

#ifdef TSP_SEC_MEM_SIZE
#pragma weak plat_trusty_set_boot_args
void plat_trusty_set_boot_args(aapcs64_params_t *args)
{
	args->arg0 = TSP_SEC_MEM_SIZE;
}
#endif

static int32_t trusty_setup(void)
{
	entry_point_info_t *ep_info;
	uint32_t instr;
	uint32_t flags;
	int32_t ret;
	bool aarch32 = false;

	/* Get trusty's entry point info */
	ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (ep_info == NULL) {
		INFO("Trusty image missing.\n");
		return -1;
	}

	/* memmap first page of trusty's code memory before peeking */
	ret = mmap_add_dynamic_region(ep_info->pc, /* PA */
			ep_info->pc, /* VA */
			PAGE_SIZE, /* size */
			MT_SECURE | MT_RW_DATA); /* attrs */
	assert(ret == 0);

	/* peek into trusty's code to see if we have a 32-bit or 64-bit image */
	instr = *(uint32_t *)ep_info->pc;

	if (instr >> 24 == 0xeaU) {
		INFO("trusty: Found 32 bit image\n");
		aarch32 = true;
	} else if (instr >> 8 == 0xd53810U || instr >> 16 == 0x9400U) {
		INFO("trusty: Found 64 bit image\n");
	} else {
		ERROR("trusty: Found unknown image, 0x%x\n", instr);
		return -1;
	}

	/* unmap trusty's memory page */
	(void)mmap_remove_dynamic_region(ep_info->pc, PAGE_SIZE);

	SET_PARAM_HEAD(ep_info, PARAM_EP, VERSION_1, SECURE | EP_ST_ENABLE);
	if (!aarch32)
		ep_info->spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS);
	else
		ep_info->spsr = SPSR_MODE32(MODE32_svc, SPSR_T_ARM,
					    SPSR_E_LITTLE,
					    DAIF_FIQ_BIT |
					    DAIF_IRQ_BIT |
					    DAIF_ABT_BIT);
	(void)memset(&ep_info->args, 0, sizeof(ep_info->args));
	plat_trusty_set_boot_args(&ep_info->args);

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
	if (ret != 0) {
		ERROR("trusty: failed to register fiq handler, ret = %d\n", ret);
	}

	if (aarch32) {
		entry_point_info_t *ns_ep_info;
		uint32_t spsr;

		ns_ep_info = bl31_plat_get_next_image_ep_info(NON_SECURE);
		if (ns_ep_info == NULL) {
			NOTICE("Trusty: non-secure image missing.\n");
			return -1;
		}
		spsr = ns_ep_info->spsr;
		if (GET_RW(spsr) == MODE_RW_64 && GET_EL(spsr) == MODE_EL2) {
			spsr &= ~(MODE_EL_MASK << MODE_EL_SHIFT);
			spsr |= MODE_EL1 << MODE_EL_SHIFT;
		}
		if (GET_RW(spsr) == MODE_RW_32 && GET_M32(spsr) == MODE32_hyp) {
			spsr &= ~(MODE32_MASK << MODE32_SHIFT);
			spsr |= MODE32_svc << MODE32_SHIFT;
		}
		if (spsr != ns_ep_info->spsr) {
			NOTICE("Trusty: Switch bl33 from EL2 to EL1 (spsr 0x%x -> 0x%x)\n",
			       ns_ep_info->spsr, spsr);
			ns_ep_info->spsr = spsr;
		}
	}

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
