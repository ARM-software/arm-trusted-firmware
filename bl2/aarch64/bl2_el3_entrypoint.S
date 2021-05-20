/*
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <platform_def.h>

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <el3_common_macros.S>

	.globl	bl2_entrypoint
	.globl	bl2_el3_run_image
	.globl	bl2_run_next_image

#if BL2_IN_XIP_MEM
#define FIXUP_SIZE	0
#else
#define FIXUP_SIZE	((BL2_LIMIT) - (BL2_BASE))
#endif

/*
 * Notes on ENABLE_RME build option
 *
 * The purpose of the BL2_IN_EL3 build option is to allow BL2 to
 * essentially function as BL1 in systems that don't use a TFA boot ROM.
 * For the purposes of RME, we use a normal BL1 image but we also want
 * to run BL2 in EL3/root as normally as possible, so rather than use the
 * special bl2_el3_setup function here we use the standard bl2_setup and we
 * don't need reset or mailbox initialization code seen in the
 * el3_entrypoint_common macro. This functionality could be split off into its
 * own file if more separation between code paths is desired.
 *
 * See https://trustedfirmware-a.readthedocs.io/en/latest/design/
 *     firmware-design.html?highlight=BL2_AT_EL3
 *     #running-bl2-at-el3-execution-level
 */

func bl2_entrypoint
	/* Save arguments x0-x3 from previous Boot loader */
	mov	x20, x0
	mov	x21, x1
	mov	x22, x2
	mov	x23, x3

#if ENABLE_RME
	/* RME support is enabled. */
	el3_entrypoint_common                                   \
		_init_sctlr=0                                   \
		_warm_boot_mailbox=0                            \
		_secondary_cold_boot=0                          \
		_init_memory=0                                  \
		_init_c_runtime=1                               \
		_exception_vectors=bl2_el3_exceptions           \
		_pie_fixup_size=FIXUP_SIZE
#else /* ENABLE_RME */
	/* RME support is disabled. */
	el3_entrypoint_common                                   \
		_init_sctlr=1                                   \
		_warm_boot_mailbox=!PROGRAMMABLE_RESET_ADDRESS  \
		_secondary_cold_boot=!COLD_BOOT_SINGLE_CPU      \
		_init_memory=1                                  \
		_init_c_runtime=1                               \
		_exception_vectors=bl2_el3_exceptions		\
		_pie_fixup_size=FIXUP_SIZE
#endif /* ENABLE_RME */

	/* ---------------------------------------------
	 * Restore parameters of boot rom
	 * ---------------------------------------------
	 */
	mov	x0, x20
	mov	x1, x21
	mov	x2, x22
	mov	x3, x23

	/* ---------------------------------------------
	 * Perform BL2 setup
	 * ---------------------------------------------
	 */
#if ENABLE_RME
	/* RME is a special case, use normal bl2_setup. */
	bl	bl2_setup
#else
	/* Otherwise, use the normal bl2 in el3 setup function. */
	bl	bl2_el3_setup
#endif /* ENABLE_RME */

#if ENABLE_PAUTH
	/* ---------------------------------------------
	 * Program APIAKey_EL1 and enable pointer authentication.
	 * ---------------------------------------------
	 */
	bl	pauth_init_enable_el3
#endif /* ENABLE_PAUTH */

	/* ---------------------------------------------
	 * Jump to main function.
	 * ---------------------------------------------
	 */
	bl	bl2_main

	/* ---------------------------------------------
	 * Should never reach this point.
	 * ---------------------------------------------
	 */
	no_ret	plat_panic_handler
endfunc bl2_entrypoint

func bl2_run_next_image
	mov	x20,x0
	/* ---------------------------------------------
	 * MMU needs to be disabled because both BL2 and BL31 execute
	 * in EL3, and therefore share the same address space.
	 * BL31 will initialize the address space according to its
	 * own requirement.
	 * ---------------------------------------------
	 */
	bl	disable_mmu_icache_el3
	tlbi	alle3
	bl	bl2_el3_plat_prepare_exit

#if ENABLE_PAUTH
	/* ---------------------------------------------
	 * Disable pointer authentication before jumping
	 * to next boot image.
	 * ---------------------------------------------
	 */
	bl	pauth_disable_el3
#endif /* ENABLE_PAUTH */

	ldp	x0, x1, [x20, #ENTRY_POINT_INFO_PC_OFFSET]
	msr	elr_el3, x0
	msr	spsr_el3, x1

	ldp	x6, x7, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x30)]
	ldp	x4, x5, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x20)]
	ldp	x2, x3, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x10)]
	ldp	x0, x1, [x20, #(ENTRY_POINT_INFO_ARGS_OFFSET + 0x0)]
	exception_return
endfunc bl2_run_next_image
