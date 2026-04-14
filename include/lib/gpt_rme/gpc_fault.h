/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPC_FAULT_H
#define GPC_FAULT_H

#define	GPC_GPF_TRAP_PC_START_OFFSET	U(0)
#define	GPC_GPF_TRAP_PC_RESUME_OFFSET	U(8)
#define	GPC_GPF_TRAP_SIZE		U(16)

#ifdef __ASSEMBLER__
#define RME_GPF_TRAP_ENTRY(_fault_pc, _resume_pc)	\
	.pushsection .el3_gpf_traps, "a", %progbits;	\
	.balign 8;					\
	.quad _fault_pc;				\
	.quad _resume_pc;				\
	.popsection
#else
#include <stddef.h>
#include <stdint.h>

struct gpc_gpf_trap_entry {
	/* fault_pc is expected to fault */
	uintptr_t fault_pc;
	/* Resume execution at this EL3 address after a GPF fault */
	uintptr_t resume_pc;
};

/******************************************************************************/
/* GPF Tolerant APIs                                                          */
/******************************************************************************/

/**
 * rme_gpf_safe_read() - Copy bytes from a lower-EL buffer into an EL3 buffer
 * @dest: EL3 destination buffer
 * @lower_el_src: Source buffer owned by a lower exception level
 * @size: Number of bytes to copy
 *
 * Attempts to copy @size bytes from @lower_el_src into @dest. If a GPF is
 * raised while accessing the lower-EL source buffer, the function returns the
 * number of bytes copied before the fault.
 *
 * @dest, @lower_el_src and @size must be 8-byte aligned.
 *
 * Return: @size on success, less than @size if the lower-EL access faults
 */
size_t rme_gpf_safe_read(void *dest, const void *lower_el_src, size_t size);

/**
 * rme_gpf_safe_write() - Copy bytes from an EL3 buffer into a lower-EL buffer
 * @lower_el_dest: Destination buffer owned by a lower exception level
 * @src: EL3 source buffer
 * @size: Number of bytes to copy
 *
 * Attempts to copy @size bytes from @src into @lower_el_dest. If a GPF is
 * raised while accessing the lower-EL destination buffer, the function returns
 * the number of bytes copied before the fault.
 *
 * @lower_el_dest, @src and @size must be 8-byte aligned.
 *
 * Return: @size on success, less than @size if the lower-EL access faults
 */
size_t rme_gpf_safe_write(void *lower_el_dest, const void *src, size_t size);
#endif /* __ASSEMBLER__ */
#endif /* GPC_FAULT_H */
