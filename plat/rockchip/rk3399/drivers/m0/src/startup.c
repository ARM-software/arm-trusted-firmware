/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rk3399_mcu.h"

/* Stack configuration */
#define STACK_SIZE	0x00000040
__attribute__ ((section(".co_stack")))
unsigned long pstack[STACK_SIZE];

/* Macro definition */
#define WEAK __attribute__ ((weak))

/* System exception vector handler */
__attribute__ ((used))
void WEAK reset_handler(void);
void WEAK nmi_handler(void);
void WEAK hardware_fault_handler(void);
void WEAK svc_handler(void);
void WEAK pend_sv_handler(void);
void WEAK systick_handler(void);

extern int m0_main(void);

/* Function prototypes */
static void default_reset_handler(void);
static void default_handler(void);

/*
 * The minimal vector table for a Cortex M3.  Note that the proper constructs
 * must be placed on this to ensure that it ends up at physical address
 * 0x00000000.
 */
__attribute__ ((used, section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
	/* core Exceptions */
	(void *)&pstack[STACK_SIZE], /* the initial stack pointer */
	reset_handler,
	nmi_handler,
	hardware_fault_handler,
	0, 0, 0, 0, 0, 0, 0,
	svc_handler,
	0, 0,
	pend_sv_handler,
	systick_handler,

	/* external exceptions */
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};

/**
 * This is the code that gets called when the processor first
 * starts execution following a reset event. Only the absolutely
 * necessary set is performed, after which the application
 * supplied m0_main() routine is called.
 */
static void default_reset_handler(void)
{
	/* call the application's entry point */
	m0_main();
}

/**
 * Provide weak aliases for each Exception handler to the Default_Handler.
 * As they are weak aliases, any function with the same name will override
 * this definition.
 */
#pragma weak reset_handler = default_reset_handler
#pragma weak nmi_handler = default_handler
#pragma weak hardware_fault_handler = default_handler
#pragma weak svc_handler = default_handler
#pragma weak pend_sv_handler = default_handler
#pragma weak systick_handler = default_handler

/**
 * This is the code that gets called when the processor receives
 * an unexpected interrupt.  This simply enters an infinite loop,
 * preserving the system state for examination by a debugger.
 */
static void default_handler(void)
{
    /* go into an infinite loop. */
	while (1)
		;
}
