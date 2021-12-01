/*
 * Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <common/bl_common.h>
#include <lib/xlat_tables/xlat_tables_defs.h>

	.globl	smc
	.globl	zeromem
	.globl	zero_normalmem
	.globl	memcpy4
	.globl	disable_mmu_icache_secure
	.globl	disable_mmu_secure
	.globl	fixup_gdt_reloc

#define PAGE_START_MASK		~(PAGE_SIZE_MASK)

func smc
	/*
	 * For AArch32 only r0-r3 will be in the registers;
	 * rest r4-r6 will be pushed on to the stack. So here, we'll
	 * have to load them from the stack to registers r4-r6 explicitly.
	 * Clobbers: r4-r6
	 */
	ldm	sp, {r4, r5, r6}
	smc	#0
endfunc smc

/* -----------------------------------------------------------------------
 * void zeromem(void *mem, unsigned int length)
 *
 * Initialise a region in normal memory to 0. This functions complies with the
 * AAPCS and can be called from C code.
 *
 * -----------------------------------------------------------------------
 */
func zeromem
	/*
	 * Readable names for registers
	 *
	 * Registers r0, r1 and r2 are also set by zeromem which
	 * branches into the fallback path directly, so cursor, length and
	 * stop_address should not be retargeted to other registers.
	 */
	cursor       .req r0 /* Start address and then current address */
	length       .req r1 /* Length in bytes of the region to zero out */
	/*
	 * Reusing the r1 register as length is only used at the beginning of
	 * the function.
	 */
	stop_address .req r1  /* Address past the last zeroed byte */
	zeroreg1     .req r2  /* Source register filled with 0 */
	zeroreg2     .req r3  /* Source register filled with 0 */
	tmp	     .req r12 /* Temporary scratch register */

	mov	zeroreg1, #0

	/* stop_address is the address past the last to zero */
	add	stop_address, cursor, length

	/*
	 * Length cannot be used anymore as it shares the same register with
	 * stop_address.
	 */
	.unreq	length

	/*
	 * If the start address is already aligned to 8 bytes, skip this loop.
	 */
	tst	cursor, #(8-1)
	beq	.Lzeromem_8bytes_aligned

	/* Calculate the next address aligned to 8 bytes */
	orr	tmp, cursor, #(8-1)
	adds	tmp, tmp, #1
	/* If it overflows, fallback to byte per byte zeroing */
	beq	.Lzeromem_1byte_aligned
	/* If the next aligned address is after the stop address, fall back */
	cmp	tmp, stop_address
	bhs	.Lzeromem_1byte_aligned

	/* zero byte per byte */
1:
	strb	zeroreg1, [cursor], #1
	cmp	cursor, tmp
	bne	1b

	/* zero 8 bytes at a time */
.Lzeromem_8bytes_aligned:

	/* Calculate the last 8 bytes aligned address. */
	bic	tmp, stop_address, #(8-1)

	cmp	cursor, tmp
	bhs	2f

	mov	zeroreg2, #0
1:
	stmia	cursor!, {zeroreg1, zeroreg2}
	cmp	cursor, tmp
	blo	1b
2:

	/* zero byte per byte */
.Lzeromem_1byte_aligned:
	cmp	cursor, stop_address
	beq	2f
1:
	strb	zeroreg1, [cursor], #1
	cmp	cursor, stop_address
	bne	1b
2:
	bx	lr

	.unreq	cursor
	/*
	 * length is already unreq'ed to reuse the register for another
	 * variable.
	 */
	.unreq	stop_address
	.unreq	zeroreg1
	.unreq	zeroreg2
	.unreq	tmp
endfunc zeromem

/*
 * AArch32 does not have special ways of zeroing normal memory as AArch64 does
 * using the DC ZVA instruction, so we just alias zero_normalmem to zeromem.
 */
.equ	zero_normalmem, zeromem

/* --------------------------------------------------------------------------
 * void memcpy4(void *dest, const void *src, unsigned int length)
 *
 * Copy length bytes from memory area src to memory area dest.
 * The memory areas should not overlap.
 * Destination and source addresses must be 4-byte aligned.
 * --------------------------------------------------------------------------
 */
func memcpy4
#if ENABLE_ASSERTIONS
	orr	r3, r0, r1
	tst	r3, #0x3
	ASM_ASSERT(eq)
#endif
/* copy 4 bytes at a time */
m_loop4:
	cmp	r2, #4
	blo	m_loop1
	ldr	r3, [r1], #4
	str	r3, [r0], #4
	subs	r2, r2, #4
	bne	m_loop4
	bx	lr

/* copy byte per byte */
m_loop1:
	ldrb	r3, [r1], #1
	strb	r3, [r0], #1
	subs	r2, r2, #1
	bne	m_loop1
	bx	lr
endfunc memcpy4

/* ---------------------------------------------------------------------------
 * Disable the MMU in Secure State
 * ---------------------------------------------------------------------------
 */

func disable_mmu_secure
	mov	r1, #(SCTLR_M_BIT | SCTLR_C_BIT)
do_disable_mmu:
#if ERRATA_A9_794073
	stcopr	r0, BPIALL
	dsb
#endif
	ldcopr	r0, SCTLR
	bic	r0, r0, r1
	stcopr	r0, SCTLR
	isb				// ensure MMU is off
	dsb	sy
	bx	lr
endfunc disable_mmu_secure


func disable_mmu_icache_secure
	ldr	r1, =(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	b	do_disable_mmu
endfunc disable_mmu_icache_secure

/* ---------------------------------------------------------------------------
 * Helper to fixup Global Descriptor table (GDT) and dynamic relocations
 * (.rel.dyn) at runtime.
 *
 * This function is meant to be used when the firmware is compiled with -fpie
 * and linked with -pie options. We rely on the linker script exporting
 * appropriate markers for start and end of the section. For GOT, we
 * expect __GOT_START__ and __GOT_END__. Similarly for .rela.dyn, we expect
 * __RELA_START__ and __RELA_END__.
 *
 * The function takes the limits of the memory to apply fixups to as
 * arguments (which is usually the limits of the relocable BL image).
 *   r0 -  the start of the fixup region
 *   r1 -  the limit of the fixup region
 * These addresses have to be 4KB page aligned.
 * ---------------------------------------------------------------------------
 */

/* Relocation codes */
#define R_ARM_RELATIVE 	23

func fixup_gdt_reloc
	mov	r6, r0
	mov	r7, r1

#if ENABLE_ASSERTIONS
	/* Test if the limits are 4K aligned */
	orr	r0, r0, r1
	mov	r1, #(PAGE_SIZE_MASK)
	tst	r0, r1
	ASM_ASSERT(eq)
#endif
	/*
	 * Calculate the offset based on return address in lr.
	 * Assume that this function is called within a page at the start of
	 * fixup region.
	 */
	ldr	r1, =PAGE_START_MASK
	and	r2, lr, r1
	subs	r0, r2, r6	/* Diff(S) = Current Address - Compiled Address */
	beq	3f		/* Diff(S) = 0. No relocation needed */

	ldr	r1, =__GOT_START__
	add	r1, r1, r0
	ldr	r2, =__GOT_END__
	add	r2, r2, r0

	/*
	 * GOT is an array of 32_bit addresses which must be fixed up as
	 * new_addr = old_addr + Diff(S).
	 * The new_addr is the address currently the binary is executing from
	 * and old_addr is the address at compile time.
	 */
1:	ldr	r3, [r1]

	/* Skip adding offset if address is < lower limit */
	cmp	r3, r6
	blo	2f

	/* Skip adding offset if address is > upper limit */
	cmp	r3, r7
	bhi	2f
	add	r3, r3, r0
	str	r3, [r1]

2:	add	r1, r1, #4
	cmp	r1, r2
	blo	1b

	/* Starting dynamic relocations. Use ldr to get RELA_START and END */
3:	ldr	r1, =__RELA_START__
	add	r1, r1, r0
	ldr	r2, =__RELA_END__
	add	r2, r2, r0

	/*
	 * According to ELF-32 specification, the RELA data structure is as
	 * follows:
	 *	typedef struct {
	 *		Elf32_Addr r_offset;
	 *		Elf32_Xword r_info;
	 *	} Elf32_Rela;
	 *
	 * r_offset is address of reference
	 * r_info is symbol index and type of relocation (in this case
	 * code 23  which corresponds to R_ARM_RELATIVE).
	 *
	 * Size of Elf32_Rela structure is 8 bytes.
	 */

	/* Skip R_ARM_NONE entry with code 0 */
1:	ldr	r3, [r1, #4]
	ands	r3, r3, #0xff
	beq	2f

#if ENABLE_ASSERTIONS
	/* Assert that the relocation type is R_ARM_RELATIVE */
	cmp	r3, #R_ARM_RELATIVE
	ASM_ASSERT(eq)
#endif
	ldr	r3, [r1]	/* r_offset */
	add	r3, r0, r3	/* Diff(S) + r_offset */
	ldr 	r4, [r3]

	/* Skip adding offset if address is < lower limit */
	cmp	r4, r6
	blo	2f

	/* Skip adding offset if address is > upper limit */
	cmp	r4, r7
	bhi	2f

	add 	r4, r0, r4
	str	r4, [r3]

2:	add	r1, r1, #8
	cmp	r1, r2
	blo	1b
	bx	lr
endfunc fixup_gdt_reloc
