/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SETJMP__H
#define SETJMP__H

#define JMP_CTX_X19	0x0
#define JMP_CTX_X21	0x10
#define JMP_CTX_X23	0x20
#define JMP_CTX_X25	0x30
#define JMP_CTX_X27	0x40
#define JMP_CTX_X29	0x50
#define JMP_CTX_SP	0x60
#define JMP_CTX_END	0x70 /* Aligned to 16 bytes */

#define JMP_SIZE	(JMP_CTX_END >> 3)

#ifndef __ASSEMBLER__

#include <cdefs.h>

/* Jump buffer hosting x18 - x30 and sp_el0 registers */
typedef uint64_t jmp_buf[JMP_SIZE] __aligned(16);

#endif /* __ASSEMBLER__ */

#endif /* SETJMP__H */
