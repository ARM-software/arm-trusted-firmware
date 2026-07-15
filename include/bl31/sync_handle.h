/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 * Copyright (c) 2023, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRAP_HANDLE_H
#define TRAP_HANDLE_H

#include <stdbool.h>
#include <context.h>

#define TRAP_RET_UNHANDLED		-1
#define TRAP_RET_REPEAT			0
#define TRAP_RET_CONTINUE		1

#define XZR_REG_NUM   31

/**
 * handle_sysreg_trap() - Handle AArch64 system register traps from lower ELs
 * @esr_el3: The content of ESR_EL3, containing the trap syndrome information
 * @ctx: Pointer to the lower EL context, containing saved registers
 *
 * Called by the exception handler when a synchronous trap identifies as a
 * system register trap (EC=0x18). ESR contains the encoding of the op[x] and
 * CRm/CRn fields, to identify the system register, and the target/source
 * GPR plus the direction (MRS/MSR). The lower EL's context can be altered
 * by the function, to inject back the result of the emulation.
 *
 * Return: indication how to proceed with the trap:
 *   TRAP_RET_UNHANDLED(-1): trap is unhandled, trigger panic
 *   TRAP_RET_REPEAT(0): trap was handled, return to the trapping instruction
 *			 (repeating it)
 *   TRAP_RET_CONTINUE(1): trap was handled, return to the next instruction
 *		           (continuing after it)
 */
int handle_sysreg_trap(uint64_t esr_el3, cpu_context_t *ctx, u_register_t flags);

/* Handler for injecting UNDEF exception to lower EL */
void inject_undef64(cpu_context_t *ctx);

/* Handler for injecting SEA exception to lower EL */
#ifdef __aarch64__
void inject_sync_ea64(el3_state_t *state, u_register_t esr_el3);
#endif

u_register_t create_spsr(u_register_t old_spsr, unsigned int target_el, u_register_t scr_el3);

/* Prototypes for system register emulation handlers provided by platforms. */
int plat_handle_impdef_trap(uint64_t esr_el3, cpu_context_t *ctx);
#if ENABLE_FEAT_RNG_TRAP
int plat_handle_rng_trap(u_register_t *data, bool rndrrs);
#else
static inline int plat_handle_rng_trap(u_register_t *data, bool rndrrs)
{
	return TRAP_RET_UNHANDLED;
}
#endif

#endif
