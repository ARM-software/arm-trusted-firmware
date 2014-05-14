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

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <bl31.h>
#include <context.h>
#include <context_mgmt.h>
#include <interrupt_mgmt.h>
#include <platform.h>
#include <platform_def.h>
#include <runtime_svc.h>

/*******************************************************************************
 * Data structure which holds the pointers to non-secure and secure security
 * state contexts for each cpu. It is aligned to the cache line boundary to
 * allow efficient concurrent manipulation of these pointers on different cpus
 ******************************************************************************/
typedef struct {
	void *ptr[2];
} __aligned (CACHE_WRITEBACK_GRANULE) context_info_t;

static context_info_t cm_context_info[PLATFORM_CORE_COUNT];

/* The per_cpu_ptr_cache_t space allocation */
static per_cpu_ptr_cache_t per_cpu_ptr_cache_space[PLATFORM_CORE_COUNT];

/*******************************************************************************
 * Context management library initialisation routine. This library is used by
 * runtime services to share pointers to 'cpu_context' structures for the secure
 * and non-secure states. Management of the structures and their associated
 * memory is not done by the context management library e.g. the PSCI service
 * manages the cpu context used for entry from and exit to the non-secure state.
 * The Secure payload dispatcher service manages the context(s) corresponding to
 * the secure state. It also uses this library to get access to the non-secure
 * state cpu context pointers.
 * Lastly, this library provides the api to make SP_EL3 point to the cpu context
 * which will used for programming an entry into a lower EL. The same context
 * will used to save state upon exception entry from that EL.
 ******************************************************************************/
void cm_init()
{
	/*
	 * The context management library has only global data to intialize, but
	 * that will be done when the BSS is zeroed out
	 */
}

/*******************************************************************************
 * This function returns a pointer to the most recent 'cpu_context' structure
 * that was set as the context for the specified security state. NULL is
 * returned if no such structure has been specified.
 ******************************************************************************/
void *cm_get_context(uint64_t mpidr, uint32_t security_state)
{
	uint32_t linear_id = platform_get_core_pos(mpidr);

	assert(security_state <= NON_SECURE);

	return cm_context_info[linear_id].ptr[security_state];
}

/*******************************************************************************
 * This function sets the pointer to the current 'cpu_context' structure for the
 * specified security state.
 ******************************************************************************/
void cm_set_context(uint64_t mpidr, void *context, uint32_t security_state)
{
	uint32_t linear_id = platform_get_core_pos(mpidr);

	assert(security_state <= NON_SECURE);

	cm_context_info[linear_id].ptr[security_state] = context;
}

/*******************************************************************************
 * The next four functions are used by runtime services to save and restore EL3
 * and EL1 contexts on the 'cpu_context' structure for the specified security
 * state.
 ******************************************************************************/
void cm_el3_sysregs_context_save(uint32_t security_state)
{
	cpu_context_t *ctx;

	ctx = cm_get_context(read_mpidr(), security_state);
	assert(ctx);

	el3_sysregs_context_save(get_el3state_ctx(ctx));
}

void cm_el3_sysregs_context_restore(uint32_t security_state)
{
	cpu_context_t *ctx;

	ctx = cm_get_context(read_mpidr(), security_state);
	assert(ctx);

	el3_sysregs_context_restore(get_el3state_ctx(ctx));
}

void cm_el1_sysregs_context_save(uint32_t security_state)
{
	cpu_context_t *ctx;

	ctx = cm_get_context(read_mpidr(), security_state);
	assert(ctx);

	el1_sysregs_context_save(get_sysregs_ctx(ctx));
}

void cm_el1_sysregs_context_restore(uint32_t security_state)
{
	cpu_context_t *ctx;

	ctx = cm_get_context(read_mpidr(), security_state);
	assert(ctx);

	el1_sysregs_context_restore(get_sysregs_ctx(ctx));
}

/*******************************************************************************
 * This function populates 'cpu_context' pertaining to the given security state
 * with the entrypoint, SPSR and SCR values so that an ERET from this security
 * state correctly restores corresponding values to drop the CPU to the next
 * exception level
 ******************************************************************************/
void cm_set_el3_eret_context(uint32_t security_state, uint64_t entrypoint,
		uint32_t spsr, uint32_t scr)
{
	cpu_context_t *ctx;
	el3_state_t *state;

	ctx = cm_get_context(read_mpidr(), security_state);
	assert(ctx);

	/* Program the interrupt routing model for this security state */
	scr &= ~SCR_FIQ_BIT;
	scr &= ~SCR_IRQ_BIT;
	scr |= get_scr_el3_from_routing_model(security_state);

	/* Populate EL3 state so that we've the right context before doing ERET */
	state = get_el3state_ctx(ctx);
	write_ctx_reg(state, CTX_SPSR_EL3, spsr);
	write_ctx_reg(state, CTX_ELR_EL3, entrypoint);
	write_ctx_reg(state, CTX_SCR_EL3, scr);
}

/*******************************************************************************
 * This function populates ELR_EL3 member of 'cpu_context' pertaining to the
 * given security state with the given entrypoint
 ******************************************************************************/
void cm_set_elr_el3(uint32_t security_state, uint64_t entrypoint)
{
	cpu_context_t *ctx;
	el3_state_t *state;

	ctx = cm_get_context(read_mpidr(), security_state);
	assert(ctx);

	/* Populate EL3 state so that ERET jumps to the correct entry */
	state = get_el3state_ctx(ctx);
	write_ctx_reg(state, CTX_ELR_EL3, entrypoint);
}

/*******************************************************************************
 * This function updates a single bit in the SCR_EL3 member of the 'cpu_context'
 * pertaining to the given security state using the value and bit position
 * specified in the parameters. It preserves all other bits.
 ******************************************************************************/
void cm_write_scr_el3_bit(uint32_t security_state,
			  uint32_t bit_pos,
			  uint32_t value)
{
	cpu_context_t *ctx;
	el3_state_t *state;
	uint32_t scr_el3;

	ctx = cm_get_context(read_mpidr(), security_state);
	assert(ctx);

	/* Ensure that the bit position is a valid one */
	assert((1 << bit_pos) & SCR_VALID_BIT_MASK);

	/* Ensure that the 'value' is only a bit wide */
	assert(value <= 1);

	/*
	 * Get the SCR_EL3 value from the cpu context, clear the desired bit
	 * and set it to its new value.
	 */
	state = get_el3state_ctx(ctx);
	scr_el3 = read_ctx_reg(state, CTX_SCR_EL3);
	scr_el3 &= ~(1 << bit_pos);
	scr_el3 |= value << bit_pos;
	write_ctx_reg(state, CTX_SCR_EL3, scr_el3);
}

/*******************************************************************************
 * This function retrieves SCR_EL3 member of 'cpu_context' pertaining to the
 * given security state.
 ******************************************************************************/
uint32_t cm_get_scr_el3(uint32_t security_state)
{
	cpu_context_t *ctx;
	el3_state_t *state;

	ctx = cm_get_context(read_mpidr(), security_state);
	assert(ctx);

	/* Populate EL3 state so that ERET jumps to the correct entry */
	state = get_el3state_ctx(ctx);
	return read_ctx_reg(state, CTX_SCR_EL3);
}

/*******************************************************************************
 * This function is used to program the context that's used for exception
 * return. This initializes the SP_EL3 to a pointer to a 'cpu_context' set for
 * the required security state
 ******************************************************************************/
void cm_set_next_eret_context(uint32_t security_state)
{
	cpu_context_t *ctx;
#if DEBUG
	uint64_t sp_mode;
#endif

	ctx = cm_get_context(read_mpidr(), security_state);
	assert(ctx);

#if DEBUG
	/*
	 * Check that this function is called with SP_EL0 as the stack
	 * pointer
	 */
	__asm__ volatile("mrs	%0, SPSel\n"
			 : "=r" (sp_mode));

	assert(sp_mode == MODE_SP_EL0);
#endif

	__asm__ volatile("msr	spsel, #1\n"
			 "mov	sp, %0\n"
			 "msr	spsel, #0\n"
			 : : "r" (ctx));
}

/************************************************************************
 * The following function is used to populate the per cpu pointer cache.
 * The pointer will be stored in the tpidr_el3 register.
 *************************************************************************/
void cm_init_pcpu_ptr_cache()
{
	unsigned long mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	per_cpu_ptr_cache_t *pcpu_ptr_cache;

	pcpu_ptr_cache = &per_cpu_ptr_cache_space[linear_id];
	assert(pcpu_ptr_cache);
	pcpu_ptr_cache->crash_stack = get_crash_stack(mpidr);

	cm_set_pcpu_ptr_cache(pcpu_ptr_cache);
}


void cm_set_pcpu_ptr_cache(const void *pcpu_ptr)
{
	write_tpidr_el3((unsigned long)pcpu_ptr);
}

void *cm_get_pcpu_ptr_cache(void)
{
	return (void *)read_tpidr_el3();
}

