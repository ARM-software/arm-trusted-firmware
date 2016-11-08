/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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

#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <debug.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <runtime_svc.h>
#include <string.h>

#include "smcall.h"
#include "sm_err.h"

struct trusty_stack {
	uint8_t space[PLATFORM_STACK_SIZE] __aligned(16);
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
};

struct trusty_cpu_ctx trusty_cpu_ctx[PLATFORM_CORE_COUNT];

struct args trusty_init_context_stack(void **sp, void *new_stack);
struct args trusty_context_switch_helper(void **sp, uint64_t r0, uint64_t r1,
					 uint64_t r2, uint64_t r3);

static struct trusty_cpu_ctx *get_trusty_ctx(void)
{
	return &trusty_cpu_ctx[plat_my_core_pos()];
}

static struct args trusty_context_switch(uint32_t security_state, uint64_t r0,
					 uint64_t r1, uint64_t r2, uint64_t r3)
{
	struct args ret;
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();

	assert(ctx->saved_security_state != security_state);

	cm_el1_sysregs_context_save(security_state);

	ctx->saved_security_state = security_state;
	ret = trusty_context_switch_helper(&ctx->saved_sp, r0, r1, r2, r3);

	assert(ctx->saved_security_state == !security_state);

	cm_el1_sysregs_context_restore(security_state);
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

	if (is_caller_secure(flags)) {
		if (smc_fid == SMC_SC_NS_RETURN) {
			ret = trusty_context_switch(SECURE, x1, 0, 0, 0);
			SMC_RET4(handle, ret.r0, ret.r1, ret.r2, ret.r3);
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
			ret = trusty_context_switch(NON_SECURE, smc_fid, x1,
				x2, x3);
			SMC_RET1(handle, ret.r0);
		}
	}
}

static int32_t trusty_init(void)
{
	void el3_exit();
	entry_point_info_t *ep_info;
	struct trusty_cpu_ctx *ctx = get_trusty_ctx();
	uint32_t cpu = plat_my_core_pos();
	int reg_width = GET_RW(read_ctx_reg(get_el3state_ctx(&ctx->cpu_ctx),
			       CTX_SPSR_EL3));

	ep_info = bl31_plat_get_next_image_ep_info(SECURE);

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
	trusty_init_context_stack(&ctx->saved_sp, &ctx->secure_stack);

	trusty_context_switch_helper(&ctx->saved_sp, 0, 0, 0, 0);

	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	return 0;
}

static void trusty_cpu_suspend(void)
{
	struct args ret;
	unsigned int linear_id = plat_my_core_pos();

	ret = trusty_context_switch(NON_SECURE, SMC_FC_CPU_SUSPEND, 0, 0, 0);
	if (ret.r0 != 0) {
		INFO("%s: cpu %d, SMC_FC_CPU_SUSPEND returned unexpected value, %ld\n",
		     __func__, linear_id, ret.r0);
	}
}

static void trusty_cpu_resume(void)
{
	struct args ret;
	unsigned int linear_id = plat_my_core_pos();

	ret = trusty_context_switch(NON_SECURE, SMC_FC_CPU_RESUME, 0, 0, 0);
	if (ret.r0 != 0) {
		INFO("%s: cpu %d, SMC_FC_CPU_RESUME returned unexpected value, %ld\n",
		     __func__, linear_id, ret.r0);
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
	uint32_t instr;
	uint32_t flags;
	int ret;
	int aarch32 = 0;

	ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!ep_info) {
		INFO("Trusty image missing.\n");
		return -1;
	}

	instr = *(uint32_t *)ep_info->pc;

	if (instr >> 24 == 0xea) {
		INFO("trusty: Found 32 bit image\n");
		aarch32 = 1;
	} else if (instr >> 8 == 0xd53810) {
		INFO("trusty: Found 64 bit image\n");
	} else {
		INFO("trusty: Found unknown image, 0x%x\n", instr);
	}

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

	bl31_register_bl32_init(trusty_init);

	psci_register_spd_pm_hook(&trusty_pm);

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

/* Define a SPD runtime service descriptor for standard SMC calls */
DECLARE_RT_SVC(
	trusty_std,

	OEN_TOS_START,
	SMC_ENTITY_SECURE_MONITOR,
	SMC_TYPE_STD,
	NULL,
	trusty_smc_handler
);
