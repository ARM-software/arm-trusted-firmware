/*
 * Copyright (c) 2022, ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRAP_HANDLE_H
#define TRAP_HANDLE_H

#include <stdbool.h>
#include <context.h>

#define ISS_SYSREG_OPCODE_MASK		0x3ffc1eUL
#define ISS_SYSREG_REG_MASK		0x0003e0UL
#define ISS_SYSREG_REG_SHIFT		5U
#define ISS_SYSREG_DIRECTION_MASK	0x000001UL

#define ISS_SYSREG_OPCODE_RNDR		0x30c808U
#define ISS_SYSREG_OPCODE_RNDRRS	0x32c808U

#define TRAP_RET_UNHANDLED		-1
#define TRAP_RET_REPEAT			0
#define TRAP_RET_CONTINUE		1

#ifndef __ASSEMBLER__
static inline unsigned int get_sysreg_iss_rt(uint64_t esr)
{
	return (esr & ISS_SYSREG_REG_MASK) >> ISS_SYSREG_REG_SHIFT;
}

static inline bool is_sysreg_iss_write(uint64_t esr)
{
	return !(esr & ISS_SYSREG_DIRECTION_MASK);
}

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
int handle_sysreg_trap(uint64_t esr_el3, cpu_context_t *ctx);

/* Prototypes for system register emulation handlers provided by platforms. */
int plat_handle_rng_trap(uint64_t esr_el3, cpu_context_t *ctx);

#endif /* __ASSEMBLER__ */

#endif
