/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <lib/gpt_rme/gpc_fault.h>

	.globl	get_el3_gpf_resume_pc
	.globl	rme_gpf_safe_read
	.globl	rme_gpf_safe_write

RME_GPF_TRAP_ENTRY(read_fault_ldr, rme_gpf_safe_return)
RME_GPF_TRAP_ENTRY(write_fault_str, rme_gpf_safe_return)

/*
 * Copy data from a lower EL buffer into Root memory.
 * The function returns the number of bytes copied.
 * If the access to the lower EL generates a GPF, the exception handler
 * returns to rme_gpf_safe_return and the number of bytes copied before the GPF
 * is returned to the caller. In case of failure, partial data may have been
 * written to the destination buffer.
 *
 * x0 - The address of buffer in Root memory to write into
 * x1 - The address of buffer in lower EL memory to read from.
 * x2 - The number of bytes to read in bytes.
 * All arguments must be aligned to 8 bytes.
 */
func rme_gpf_safe_read
	cbz	x2, 2f
	mov	x3, #0

1:
read_fault_ldr:
	ldr	x4, [x1], #8
	str	x4, [x0], #8
	add	x3, x3, #8
	cmp	x3, x2
	bne	read_fault_ldr

2:
	mov	x0, x2
	ret
endfunc rme_gpf_safe_read

/*
 * Copy data from Root into lower EL memory.
 * The function returns the number of bytes copied.
 * If the access to the lower EL generates a GPF, the exception handler
 * returns to rme_gpf_safe_return and the number of bytes copied before the GPF
 * is returned to the caller. In case of failure, partial data may have been
 * written to the destination buffer.
 *
 * x0 - The address of buffer in lower EL to write into
 * x1 - The address of buffer in Root memory to read from.
 * x2 - The number of bytes to write.
 * All arguments must be aligned to 8 bytes.
 */
func rme_gpf_safe_write
	cbz	x2, 2f
	mov	x3, #0
1:
	ldr	x4, [x1], #8
write_fault_str:
	str	x4, [x0], #8
	add	x3, x3, #8
	cmp	x3, x2
	bne	1b
2:
	mov	x0, x2
	ret
endfunc rme_gpf_safe_write

func rme_gpf_safe_return
	mov	x0, x3
	ret
endfunc rme_gpf_safe_return


/*
 * Find the safe resume PC for the current EL3 exception if it is a
 * recoverable GPF raised by a GPF-tolerant RME helper.
 *
 * Returns:
 * x0 - Resume PC if the current exception is recoverable, otherwise 0.
 *
 * Clobbers: x1-x4
 */
func get_el3_gpf_resume_pc
	mrs	x1, esr_el3
	ubfx	x2, x1, #ESR_EC_SHIFT, #6
	cmp	x2, #EC_DABORT_CUR_EL
	b.ne	3f

	and	x2, x1, #ISS_DFSC_MASK
	cmp	x2, #DFSC_GPF_DABORT
	b.ne	3f

	mrs	x0, elr_el3

	adrp	x1, __GPC_GPF_TRAPS_START__
	add	x1, x1, :lo12:__GPC_GPF_TRAPS_START__
	adrp	x2, __GPC_GPF_TRAPS_END__
	add	x2, x2, :lo12:__GPC_GPF_TRAPS_END__

	cmp	x1, x2
	b.hs	3f

1:
	ldp	x3, x4, [x1]
	cmp	x0, x3
	b.ne	2f

	mov	x0, x4
	ret

2:
	add	x1, x1, #GPC_GPF_TRAP_SIZE
	cmp	x1, x2
	b.lo	1b

3:
	mov	x0, xzr
	ret
endfunc get_el3_gpf_resume_pc
