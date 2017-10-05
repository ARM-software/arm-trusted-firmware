/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>

	.globl	get_afflvl_shift
	.globl	mpidr_mask_lower_afflvls
	.globl	eret
	.globl	smc

	.globl	zero_normalmem
	.globl	zeromem
	.globl	zeromem16
	.globl	memcpy16

	.globl	disable_mmu_el1
	.globl	disable_mmu_el3
	.globl	disable_mmu_icache_el1
	.globl	disable_mmu_icache_el3

#if SUPPORT_VFP
	.globl	enable_vfp
#endif

func get_afflvl_shift
	cmp	x0, #3
	cinc	x0, x0, eq
	mov	x1, #MPIDR_AFFLVL_SHIFT
	lsl	x0, x0, x1
	ret
endfunc get_afflvl_shift

func mpidr_mask_lower_afflvls
	cmp	x1, #3
	cinc	x1, x1, eq
	mov	x2, #MPIDR_AFFLVL_SHIFT
	lsl	x2, x1, x2
	lsr	x0, x0, x2
	lsl	x0, x0, x2
	ret
endfunc mpidr_mask_lower_afflvls


func eret
	eret
endfunc eret


func smc
	smc	#0
endfunc smc

/* -----------------------------------------------------------------------
 * void zeromem16(void *mem, unsigned int length);
 *
 * Initialise a memory region to 0.
 * The memory address must be 16-byte aligned.
 * NOTE: This function is deprecated and zeromem should be used instead.
 * -----------------------------------------------------------------------
 */
.equ	zeromem16, zeromem

/* -----------------------------------------------------------------------
 * void zero_normalmem(void *mem, unsigned int length);
 *
 * Initialise a region in normal memory to 0. This functions complies with the
 * AAPCS and can be called from C code.
 *
 * NOTE: MMU must be enabled when using this function as it can only operate on
 *       normal memory. It is intended to be mainly used from C code when MMU
 *       is usually enabled.
 * -----------------------------------------------------------------------
 */
.equ	zero_normalmem, zeromem_dczva

/* -----------------------------------------------------------------------
 * void zeromem(void *mem, unsigned int length);
 *
 * Initialise a region of device memory to 0. This functions complies with the
 * AAPCS and can be called from C code.
 *
 * NOTE: When data caches and MMU are enabled, zero_normalmem can usually be
 *       used instead for faster zeroing.
 *
 * -----------------------------------------------------------------------
 */
func zeromem
	/* x2 is the address past the last zeroed address */
	add	x2, x0, x1
	/*
	 * Uses the fallback path that does not use DC ZVA instruction and
	 * therefore does not need enabled MMU
	 */
	b	.Lzeromem_dczva_fallback_entry
endfunc zeromem

/* -----------------------------------------------------------------------
 * void zeromem_dczva(void *mem, unsigned int length);
 *
 * Fill a region of normal memory of size "length" in bytes with null bytes.
 * MMU must be enabled and the memory be of
 * normal type. This is because this function internally uses the DC ZVA
 * instruction, which generates an Alignment fault if used on any type of
 * Device memory (see section D3.4.9 of the ARMv8 ARM, issue k). When the MMU
 * is disabled, all memory behaves like Device-nGnRnE memory (see section
 * D4.2.8), hence the requirement on the MMU being enabled.
 * NOTE: The code assumes that the block size as defined in DCZID_EL0
 *       register is at least 16 bytes.
 *
 * -----------------------------------------------------------------------
 */
func zeromem_dczva

	/*
	 * The function consists of a series of loops that zero memory one byte
	 * at a time, 16 bytes at a time or using the DC ZVA instruction to
	 * zero aligned block of bytes, which is assumed to be more than 16.
	 * In the case where the DC ZVA instruction cannot be used or if the
	 * first 16 bytes loop would overflow, there is fallback path that does
	 * not use DC ZVA.
	 * Note: The fallback path is also used by the zeromem function that
	 *       branches to it directly.
	 *
	 *              +---------+   zeromem_dczva
	 *              |  entry  |
	 *              +----+----+
	 *                   |
	 *                   v
	 *              +---------+
	 *              | checks  |>o-------+ (If any check fails, fallback)
	 *              +----+----+         |
	 *                   |              |---------------+
	 *                   v              | Fallback path |
	 *            +------+------+       |---------------+
	 *            | 1 byte loop |       |
	 *            +------+------+ .Lzeromem_dczva_initial_1byte_aligned_end
	 *                   |              |
	 *                   v              |
	 *           +-------+-------+      |
	 *           | 16 bytes loop |      |
	 *           +-------+-------+      |
	 *                   |              |
	 *                   v              |
	 *            +------+------+ .Lzeromem_dczva_blocksize_aligned
	 *            | DC ZVA loop |       |
	 *            +------+------+       |
	 *       +--------+  |              |
	 *       |        |  |              |
	 *       |        v  v              |
	 *       |   +-------+-------+ .Lzeromem_dczva_final_16bytes_aligned
	 *       |   | 16 bytes loop |      |
	 *       |   +-------+-------+      |
	 *       |           |              |
	 *       |           v              |
	 *       |    +------+------+ .Lzeromem_dczva_final_1byte_aligned
	 *       |    | 1 byte loop |       |
	 *       |    +-------------+       |
	 *       |           |              |
	 *       |           v              |
	 *       |       +---+--+           |
	 *       |       | exit |           |
	 *       |       +------+           |
	 *       |			    |
	 *       |           +--------------+    +------------------+ zeromem
	 *       |           |  +----------------| zeromem function |
	 *       |           |  |                +------------------+
	 *       |           v  v
	 *       |    +-------------+ .Lzeromem_dczva_fallback_entry
	 *       |    | 1 byte loop |
	 *       |    +------+------+
	 *       |           |
	 *       +-----------+
	 */

	/*
	 * Readable names for registers
	 *
	 * Registers x0, x1 and x2 are also set by zeromem which
	 * branches into the fallback path directly, so cursor, length and
	 * stop_address should not be retargeted to other registers.
	 */
	cursor       .req x0 /* Start address and then current address */
	length       .req x1 /* Length in bytes of the region to zero out */
	/* Reusing x1 as length is never used after block_mask is set */
	block_mask   .req x1 /* Bitmask of the block size read in DCZID_EL0 */
	stop_address .req x2 /* Address past the last zeroed byte */
	block_size   .req x3 /* Size of a block in bytes as read in DCZID_EL0 */
	tmp1         .req x4
	tmp2         .req x5

#if ENABLE_ASSERTIONS
	/*
	 * Check for M bit (MMU enabled) of the current SCTLR_EL(1|3)
	 * register value and panic if the MMU is disabled.
	 */
#if defined(IMAGE_BL1) || defined(IMAGE_BL31)
	mrs	tmp1, sctlr_el3
#else
	mrs	tmp1, sctlr_el1
#endif

	tst	tmp1, #SCTLR_M_BIT
	ASM_ASSERT(ne)
#endif /* ENABLE_ASSERTIONS */

	/* stop_address is the address past the last to zero */
	add	stop_address, cursor, length

	/*
	 * Get block_size = (log2(<block size>) >> 2) (see encoding of
	 * dczid_el0 reg)
	 */
	mrs	block_size, dczid_el0

	/*
	 * Select the 4 lowest bits and convert the extracted log2(<block size
	 * in words>) to <block size in bytes>
	 */
	ubfx	block_size, block_size, #0, #4
	mov	tmp2, #(1 << 2)
	lsl	block_size, tmp2, block_size

#if ENABLE_ASSERTIONS
	/*
	 * Assumes block size is at least 16 bytes to avoid manual realignment
	 * of the cursor at the end of the DCZVA loop.
	 */
	cmp	block_size, #16
	ASM_ASSERT(hs)
#endif
	/*
	 * Not worth doing all the setup for a region less than a block and
	 * protects against zeroing a whole block when the area to zero is
	 * smaller than that. Also, as it is assumed that the block size is at
	 * least 16 bytes, this also protects the initial aligning loops from
	 * trying to zero 16 bytes when length is less than 16.
	 */
	cmp	length, block_size
	b.lo	.Lzeromem_dczva_fallback_entry

	/*
	 * Calculate the bitmask of the block alignment. It will never
	 * underflow as the block size is between 4 bytes and 2kB.
	 * block_mask = block_size - 1
	 */
	sub	block_mask, block_size, #1

	/*
	 * length alias should not be used after this point unless it is
	 * defined as a register other than block_mask's.
	 */
	 .unreq length

	/*
	 * If the start address is already aligned to zero block size, go
	 * straight to the cache zeroing loop. This is safe because at this
	 * point, the length cannot be smaller than a block size.
	 */
	tst	cursor, block_mask
	b.eq	.Lzeromem_dczva_blocksize_aligned

	/*
	 * Calculate the first block-size-aligned address. It is assumed that
	 * the zero block size is at least 16 bytes. This address is the last
	 * address of this initial loop.
	 */
	orr	tmp1, cursor, block_mask
	add	tmp1, tmp1, #1

	/*
	 * If the addition overflows, skip the cache zeroing loops. This is
	 * quite unlikely however.
	 */
	cbz	tmp1, .Lzeromem_dczva_fallback_entry

	/*
	 * If the first block-size-aligned address is past the last address,
	 * fallback to the simpler code.
	 */
	cmp	tmp1, stop_address
	b.hi	.Lzeromem_dczva_fallback_entry

	/*
	 * If the start address is already aligned to 16 bytes, skip this loop.
	 * It is safe to do this because tmp1 (the stop address of the initial
	 * 16 bytes loop) will never be greater than the final stop address.
	 */
	tst	cursor, #0xf
	b.eq	.Lzeromem_dczva_initial_1byte_aligned_end

	/* Calculate the next address aligned to 16 bytes */
	orr	tmp2, cursor, #0xf
	add	tmp2, tmp2, #1
	/* If it overflows, fallback to the simple path (unlikely) */
	cbz	tmp2, .Lzeromem_dczva_fallback_entry
	/*
	 * Next aligned address cannot be after the stop address because the
	 * length cannot be smaller than 16 at this point.
	 */

	/* First loop: zero byte per byte */
1:
	strb	wzr, [cursor], #1
	cmp	cursor, tmp2
	b.ne	1b
.Lzeromem_dczva_initial_1byte_aligned_end:

	/*
	 * Second loop: we need to zero 16 bytes at a time from cursor to tmp1
	 * before being able to use the code that deals with block-size-aligned
	 * addresses.
	 */
	cmp	cursor, tmp1
	b.hs	2f
1:
	stp	xzr, xzr, [cursor], #16
	cmp	cursor, tmp1
	b.lo	1b
2:

	/*
	 * Third loop: zero a block at a time using DC ZVA cache block zeroing
	 * instruction.
	 */
.Lzeromem_dczva_blocksize_aligned:
	/*
	 * Calculate the last block-size-aligned address. If the result equals
	 * to the start address, the loop will exit immediately.
	 */
	bic	tmp1, stop_address, block_mask

	cmp	cursor, tmp1
	b.hs	2f
1:
	/* Zero the block containing the cursor */
	dc	zva, cursor
	/* Increment the cursor by the size of a block */
	add	cursor, cursor, block_size
	cmp	cursor, tmp1
	b.lo	1b
2:

	/*
	 * Fourth loop: zero 16 bytes at a time and then byte per byte the
	 * remaining area
	 */
.Lzeromem_dczva_final_16bytes_aligned:
	/*
	 * Calculate the last 16 bytes aligned address. It is assumed that the
	 * block size will never be smaller than 16 bytes so that the current
	 * cursor is aligned to at least 16 bytes boundary.
	 */
	bic	tmp1, stop_address, #15

	cmp	cursor, tmp1
	b.hs	2f
1:
	stp	xzr, xzr, [cursor], #16
	cmp	cursor, tmp1
	b.lo	1b
2:

	/* Fifth and final loop: zero byte per byte */
.Lzeromem_dczva_final_1byte_aligned:
	cmp	cursor, stop_address
	b.eq	2f
1:
	strb	wzr, [cursor], #1
	cmp	cursor, stop_address
	b.ne	1b
2:
	ret

	/* Fallback for unaligned start addresses */
.Lzeromem_dczva_fallback_entry:
	/*
	 * If the start address is already aligned to 16 bytes, skip this loop.
	 */
	tst	cursor, #0xf
	b.eq	.Lzeromem_dczva_final_16bytes_aligned

	/* Calculate the next address aligned to 16 bytes */
	orr	tmp1, cursor, #15
	add	tmp1, tmp1, #1
	/* If it overflows, fallback to byte per byte zeroing */
	cbz	tmp1, .Lzeromem_dczva_final_1byte_aligned
	/* If the next aligned address is after the stop address, fall back */
	cmp	tmp1, stop_address
	b.hs	.Lzeromem_dczva_final_1byte_aligned

	/* Fallback entry loop: zero byte per byte */
1:
	strb	wzr, [cursor], #1
	cmp	cursor, tmp1
	b.ne	1b

	b	.Lzeromem_dczva_final_16bytes_aligned

	.unreq	cursor
	/*
	 * length is already unreq'ed to reuse the register for another
	 * variable.
	 */
	.unreq	stop_address
	.unreq	block_size
	.unreq	block_mask
	.unreq	tmp1
	.unreq	tmp2
endfunc zeromem_dczva

/* --------------------------------------------------------------------------
 * void memcpy16(void *dest, const void *src, unsigned int length)
 *
 * Copy length bytes from memory area src to memory area dest.
 * The memory areas should not overlap.
 * Destination and source addresses must be 16-byte aligned.
 * --------------------------------------------------------------------------
 */
func memcpy16
#if ENABLE_ASSERTIONS
	orr	x3, x0, x1
	tst	x3, #0xf
	ASM_ASSERT(eq)
#endif
/* copy 16 bytes at a time */
m_loop16:
	cmp	x2, #16
	b.lo	m_loop1
	ldp	x3, x4, [x1], #16
	stp	x3, x4, [x0], #16
	sub	x2, x2, #16
	b	m_loop16
/* copy byte per byte */
m_loop1:
	cbz	x2, m_end
	ldrb	w3, [x1], #1
	strb	w3, [x0], #1
	subs	x2, x2, #1
	b.ne	m_loop1
m_end:
	ret
endfunc memcpy16

/* ---------------------------------------------------------------------------
 * Disable the MMU at EL3
 * ---------------------------------------------------------------------------
 */

func disable_mmu_el3
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT)
do_disable_mmu_el3:
	mrs	x0, sctlr_el3
	bic	x0, x0, x1
	msr	sctlr_el3, x0
	isb	/* ensure MMU is off */
	dsb	sy
	ret
endfunc disable_mmu_el3


func disable_mmu_icache_el3
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	b	do_disable_mmu_el3
endfunc disable_mmu_icache_el3

/* ---------------------------------------------------------------------------
 * Disable the MMU at EL1
 * ---------------------------------------------------------------------------
 */

func disable_mmu_el1
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT)
do_disable_mmu_el1:
	mrs	x0, sctlr_el1
	bic	x0, x0, x1
	msr	sctlr_el1, x0
	isb	/* ensure MMU is off */
	dsb	sy
	ret
endfunc disable_mmu_el1


func disable_mmu_icache_el1
	mov	x1, #(SCTLR_M_BIT | SCTLR_C_BIT | SCTLR_I_BIT)
	b	do_disable_mmu_el1
endfunc disable_mmu_icache_el1

/* ---------------------------------------------------------------------------
 * Enable the use of VFP at EL3
 * ---------------------------------------------------------------------------
 */
#if SUPPORT_VFP
func enable_vfp
	mrs	x0, cpacr_el1
	orr	x0, x0, #CPACR_VFP_BITS
	msr	cpacr_el1, x0
	mrs	x0, cptr_el3
	mov	x1, #AARCH64_CPTR_TFP
	bic	x0, x0, x1
	msr	cptr_el3, x0
	isb
	ret
endfunc enable_vfp
#endif
