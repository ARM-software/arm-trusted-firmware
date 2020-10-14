/*
 * Copyright (c) 2017-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ARM_RECLAIM_INIT_LD_S
#define ARM_RECLAIM_INIT_LD_S

SECTIONS
{
        .init __STACKS_START__ : {
            . = . + PLATFORM_STACK_SIZE;
            . = ALIGN(PAGE_SIZE);
            __INIT_CODE_START__ = .;
	    *(*text.init*);
            __INIT_CODE_END__ = .;
            INIT_CODE_END_ALIGNED = ALIGN(PAGE_SIZE);
        } >RAM

#ifdef BL31_PROGBITS_LIMIT
    ASSERT(__INIT_CODE_END__ <= BL31_PROGBITS_LIMIT,
            "BL31 init has exceeded progbits limit.")
#endif
}

#define	ABS		ABSOLUTE

#define STACK_SECTION							\
	stacks (NOLOAD) : {						\
		__STACKS_START__ = .;					\
		*(tzfw_normal_stacks)					\
		__STACKS_END__ = .;					\
		/* Allow room for the init section where necessary. */	\
		OFFSET = ABS(SIZEOF(.init) - (. - __STACKS_START__));	\
		/* Offset sign */					\
		SIGN = ABS(OFFSET) & (1 << 63);				\
		/* Offset mask */					\
		MASK = ABS(SIGN >> 63) - 1;				\
		. +=  ABS(OFFSET) & ABS(MASK);				\
		.  = ALIGN(PAGE_SIZE);					\
	}

#endif /* ARM_RECLAIM_INIT_LD_S */
