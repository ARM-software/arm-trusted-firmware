/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
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
            /*
             * Exclude PSCI initialization functions to ensure the init section
             * does not become larger than the overlaid stack region
             */
            *(EXCLUDE_FILE (*psci_setup.o).text.init*)
            __INIT_CODE_UNALIGNED__ = .;
            .  = ALIGN(PAGE_SIZE);
            __INIT_CODE_END__ = .;
        } >RAM

#ifdef BL31_PROGBITS_LIMIT
    ASSERT(__INIT_CODE_END__ <= BL31_PROGBITS_LIMIT,
            "BL31 init has exceeded progbits limit.")
#endif

#if RECLAIM_INIT_CODE
    ASSERT(__INIT_CODE_END__ <= __STACKS_END__,
        "Init code ends past the end of the stacks")
#endif
}

#endif /* ARM_RECLAIM_INIT_LD_S */
