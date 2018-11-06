/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SETJMP_H
#define SETJMP_H

#define JMP_CTX_X19	0x0
#define JMP_CTX_X21	0x10
#define JMP_CTX_X23	0x20
#define JMP_CTX_X25	0x30
#define JMP_CTX_X27	0x40
#define JMP_CTX_X29	0x50
#define JMP_CTX_SP	0x60
#define JMP_CTX_END	0x70

#define JMP_SIZE	(JMP_CTX_END >> 3)

#ifndef __ASSEMBLY__

#include <stdint.h>

/* Jump buffer hosting x18 - x30 and sp_el0 registers */
struct jmpbuf {
	uint64_t buf[JMP_SIZE];
} __aligned(16);


/*
 * Set a jump point, and populate the jump buffer with context information so
 * that longjmp() can jump later. The caller must adhere to the following
 * conditions:
 *
 *  - After calling this function, the stack must not be shrunk. The contents of
 *    the stack must not be changed either.
 *
 *  - If the caller were to 'return', the buffer must be considered invalid, and
 *    must not be used with longjmp().
 *
 * The caller will observe this function returning at two distinct
 * circumstances, each with different return values:
 *
 *  - Zero, when the buffer is setup;
 *
 *  - Non-zero, when a call to longjmp() is made (presumably by one of the
 *    callee functions) with the same jump buffer.
 */
int setjmp(struct jmpbuf *buf);

/*
 * Reset execution to a jump point, and restore context information according to
 * the jump buffer populated by setjmp().
 */
void longjmp(struct jmpbuf *buf);

#endif /* __ASSEMBLY__ */
#endif /* SETJMP_H */
