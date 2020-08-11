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
        } >RAM

#ifdef BL31_PROGBITS_LIMIT
    ASSERT(__INIT_CODE_END__ <= BL31_PROGBITS_LIMIT,
            "BL31 init has exceeded progbits limit.")
#endif

    ASSERT(__INIT_CODE_END__ <= __STACKS_END__,
        "Init code ends past the end of the stacks")

}

#undef	MIN
#define	ABS		ABSOLUTE
#define	COUNT		PLATFORM_CORE_COUNT
#define	ALIGN_MASK	~(CACHE_WRITEBACK_GRANULE - 1)

#define PRIMARY_STACK							\
	__STACKS_START__ = .;						\
	*(tzfw_normal_stacks)						\
	OFFSET = ABS(SIZEOF(.init) - (. - __STACKS_START__));		\
	/* Offset sign */						\
	SIGN = ABS(OFFSET) & (1 << 63);					\
	/* Offset mask */						\
	MASK = ABS(SIGN >> 63) - 1;					\
	. +=  ABS(OFFSET) & ABS(MASK);					\
	.  = ALIGN(PAGE_SIZE);						\
	__STACKS_END__ = .;						\
	/* Total stack size */						\
	SIZE = ABS(. - __STACKS_START__);				\
	/* Maximum primary CPU stack */					\
	STACK = ABS(__STACKS_START__ + SIZE / COUNT) & ALIGN_MASK;	\
	/* Primary CPU stack */						\
	__PRIMARY_STACK__ = MIN(STACK, ABS(__INIT_CODE_START__));

#if (COUNT > 1)
#define	SECONDARY_STACK					\
	/* Size of the secondary CPUs' stack */		\
	REST = ABS(__STACKS_END__ - __PRIMARY_STACK__);	\
	/* Secondary per-CPU stack size */		\
	__STACK_SIZE__ = ABS(REST / (COUNT - 1));
#else
#define	SECONDARY_STACK
#endif

#define STACK_SECTION		\
	stacks (NOLOAD) : {	\
		PRIMARY_STACK	\
		SECONDARY_STACK	\
	}
#endif /* ARM_RECLAIM_INIT_LD_S */
