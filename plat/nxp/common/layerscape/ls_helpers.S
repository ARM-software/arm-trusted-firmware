/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <asm_macros.S>
#include <drivers/console.h>
#include <platform_def.h>

.globl _ocram_init
.globl _get_ocram_2_init
.global	_set_smmu_pagesz_64

#define  OCRAM_REGION_LOWER  0
#define  OCRAM_REGION_UPPER  1
#define  OCRAM_REGION_ALL    2

/******************************************************************************
 * This function initializes ocram for ecc checking
 *****************************************************************************/

 /* in:  none
  * out: none
  * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10
  */
_ocram_init:
	mov  x10, x30

	 /* set the start flag */
	mov  x0, #1
	bl   _set_task1_start

	 /* get the start address and size of the OCRAM */
	mov  x0, #OCRAM_REGION_ALL
	bl   _get_ocram_2_init

	/* x0 = start address
	 * x1 = size in bytes
	 */

	 /* convert bytes to 64-byte chunks (using quad load/store pair ops) */
	lsr  x1, x1, #6

	 /* x0 = start address
	  * x1 = size in 64-byte chunks
	  */
1:
	 /* for each location, read and write-back */
	dc   ivac, x0
	isb
	ldp  x2, x3, [x0]
	ldp  x4, x5, [x0, #16]
	ldp  x6, x7, [x0, #32]
	ldp  x8, x9, [x0, #48]
	stp  x2, x3, [x0]
	stp  x4, x5, [x0, #16]
	stp  x6, x7, [x0, #32]
	stp  x8, x9, [x0, #48]
	dc   cvac, x0

	sub  x1, x1, #1
	cbz  x1, 2f
	add  x0, x0, #64
	b	1b

2:
	 /* set the done flag */
	mov  x0, #1
	bl   _set_task1_done

	 /* make sure the data accesses are complete */
	dsb  sy
	isb

	 /* restore link register */
	mov  x30, x10

	 /* clean the registers */
	mov  x0,  #0
	mov  x1,  #0
	mov  x2,  #0
	mov  x3,  #0
	mov  x4,  #0
	mov  x5,  #0
	mov  x6,  #0
	mov  x7,  #0
	mov  x8,  #0
	mov  x9,  #0
	mov  x10, #0

	ret

/*---------------------------------------------------------------------------*/

 /* this function returns the start address and size of two equal regions
  * of ocram for initialization purposes. If the stack area is in the top
  * of ocram, this is left out of the ocram regions to be initialized
  * in:  x0 = 0, return start addr and size of lower ocram region
  *		 = 1, return start addr and size of upper ocram region
  *		 = 2, return start addr and size of entire ocram region
  * out: x0 = start address of region
  *	  x1 = size of region in bytes
  * uses x0, x1, x2, x3, x4, x5
  */
_get_ocram_2_init:
	mov  x5, x30
	mov  x4, x0


	mov  x1, #NXP_OCRAM_SIZE
	mov  x2, #NXP_OCRAM_ADDR

1:
	 /* x1 = size of ocram to initialize
	  * x2 = NXP_OCRAM_ADDR
	  */

	cmp  x4, #2
	b.ne 4f

	 /* the request is for all of ocram */
	mov  x0, x2
	b	3f

4:
	 /* divide size in half */
	lsr  x1, x1, #1

	 /* determine if the upper or lower region of ocram is requested */
	cbz  x4, 2f

	 /* process the upper region of ocram */

	 /* x1 = size of ocram to initialize
	  * x2 = NXP_OCRAM_ADDR
	  */

	 /* add size to base addr to get start addr of upper half */
	add  x0, x2, x1
	b	3f

2:   /* process the lower region of ocram */

	 /* x1 = size of ocram to initialize
	  * x2 = NXP_OCRAM_ADDR
	  */

	 /* get start address of lower region */
	mov  x0, x2

3:
	mov  x30, x5
	ret

/*---------------------------------------------------------------------------*/

/*
 * this function sets the scXR pagesize to 64k
 */
_set_smmu_pagesz_64:

	ldr	x1, =NXP_SMMU_ADDR
	ldr	w0, [x1, #0x10]
	orr	w0, w0, #1 << 16  /* set sACR.pagesize to indicate 64K page */
	str	w0, [x1, #0x10]

	ret

 /*---------------------------------------------------------------------------*/
