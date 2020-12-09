/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <asm_macros.S>
#include <bl31_data.h>

.global el2_2_aarch32
.global prefetch_disable

#define  SPSR_EL3_M4     0x10
#define  SPSR_EL_MASK    0xC
#define  SPSR_EL2        0x8
#define  SCR_EL3_4_EL2_AARCH32  0x131
#define  SPSR32_EL2_LE          0x1DA

#define  MIDR_PARTNUM_START      4
#define  MIDR_PARTNUM_WIDTH      12
#define  MIDR_PARTNUM_A53        0xD03
#define  MIDR_PARTNUM_A57        0xD07
#define  MIDR_PARTNUM_A72        0xD08

/*
 * uint64_t el2_2_aarch32(u_register_t smc_id,
 *                   u_register_t start_addr,
 *                   u_register_t parm1,
 *                   u_register_t parm2)
 * this function allows changing the execution width of EL2 from Aarch64
 * to Aarch32
 * Note: MUST be called from EL2 @ Aarch64
 * in:  x0 = smc function id
 *      x1 = start address for EL2 @ Aarch32
 *      x2 = first parameter to pass to EL2 @ Aarch32
 *      x3 = second parameter to pass to EL2 @ Aarch32
 * out: x0 = 0,  on success
 *      x0 = -1, on failure
 * uses x0, x1, x2, x3
 */
func el2_2_aarch32

	/* check that caller is EL2 @ Aarch64 - err return if not */
	mrs  x0, spsr_el3
	/* see if we were called from Aarch32 */
	tst  x0, #SPSR_EL3_M4
	b.ne 2f

	/* see if we were called from EL2 */
	and   x0, x0, SPSR_EL_MASK
	cmp   x0, SPSR_EL2
	b.ne  2f

	/* set ELR_EL3 */
	msr  elr_el3, x1

	/* set scr_el3 */
	mov  x0, #SCR_EL3_4_EL2_AARCH32
	msr  scr_el3, x0

	/* set sctlr_el2 */
	ldr   x1, =SCTLR_EL2_RES1
	msr  sctlr_el2, x1

	/* set spsr_el3 */
	ldr  x0, =SPSR32_EL2_LE
	msr  spsr_el3, x0

	/* x2 = parm 1
	 * x3 = parm2
	 */

	/* set the parameters to be passed-thru to EL2 @ Aarch32 */
	mov  x1, x2
	mov  x2, x3

	/* x1 = parm 1
	 * x2 = parm2
	 */

	mov  x0, xzr
	/* invalidate the icache */
	ic iallu
	dsb sy
	isb
	b  1f
2:
	/* error return */
	mvn  x0, xzr
	ret
1:
	eret
endfunc el2_2_aarch32

/*
 * int prefetch_disable(u_register_t smc_id, u_register_t mask)
 * this function marks cores which need to have the prefetch disabled -
 * secondary cores have prefetch disabled when they are released from reset -
 * the bootcore has prefetch disabled when this call is made
 * in:  x0 = function id
 *      x1 = core mask, where bit[0]=core0, bit[1]=core1, etc
 *           if a bit in the mask is set, then prefetch is disabled for that
 *           core
 * out: x0 = SMC_SUCCESS
 */
func prefetch_disable
	stp  x4, x30, [sp, #-16]!

	mov   x3, x1

	/* x1 = core prefetch disable mask */
	/* x3 = core prefetch disable mask */

	/* store the mask */
	mov   x0, #PREFETCH_DIS_OFFSET
	bl   _set_global_data

	/* x3 = core prefetch disable mask */

	/* see if we need to disable prefetch on THIS core */
	bl   plat_my_core_mask

	/* x0 = core mask lsb */
	/* x3 = core prefetch disable mask */

	tst   x3, x0
	b.eq  1f

	/* read midr_el1 */
	mrs   x1, midr_el1

	/* x1 = midr_el1 */

	mov   x0, xzr
	bfxil x0, x1, #MIDR_PARTNUM_START, #MIDR_PARTNUM_WIDTH

	/* x0 = part number (a53, a57, a72, etc) */

	/* branch on cpu-specific */
	cmp   x0, #MIDR_PARTNUM_A57
	b.eq  1f
	cmp   x0, #MIDR_PARTNUM_A72
	b.ne  1f

	bl    _disable_ldstr_pfetch_A72
	b     1f
1:
	ldp   x4, x30, [sp], #16
	mov   x0, xzr
	ret
endfunc prefetch_disable
