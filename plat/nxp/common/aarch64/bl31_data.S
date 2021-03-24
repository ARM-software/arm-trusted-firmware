/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <asm_macros.S>

#include "bl31_data.h"
#include "plat_psci.h"
#include "platform_def.h"

.global _getCoreData
.global _setCoreData
.global _getCoreState
.global _setCoreState
.global _init_global_data
.global _get_global_data
.global _set_global_data
.global _initialize_psci
.global _init_task_flags
.global _set_task1_start
.global _set_task1_done


/* Function returns the specified data field value from the specified cpu
 * core data area
 * in:  x0 = core mask lsb
 *	x1 = data field name/offset
 * out: x0 = data value
 * uses x0, x1, x2, [x13, x14, x15]
 */
func _getCoreData

	/* generate a 0-based core number from the input mask */
	clz   x2, x0
	mov   x0, #63
	sub   x0, x0, x2

	/* x0 = core number (0-based) */
	/* x1 = field offset */

	/* determine if this is bootcore or secondary core */
	cbnz  x0, 1f

	/* get base address for bootcore data */
	ldr  x2, =BC_PSCI_BASE
	add  x2, x2, x1
	b	2f

1:	/* get base address for secondary core data */

	/* x0 = core number (0-based) */
	/* x1 = field offset */

	/* generate number of regions to offset */
	mov   x2, #SEC_REGION_SIZE
	mul   x2, x2, x0

	/* x1 = field offset */
	/* x2 = region offset */

	/* generate the total offset to data element */
	sub   x1, x2, x1

	/* x1 = total offset to data element */

	/* get the base address */
	ldr   x2, =SECONDARY_TOP

	/* apply offset to base addr */
	sub   x2, x2, x1
2:
	/* x2 = data element address */

	dc   ivac, x2
	dsb  sy
	isb
	/* read data */
	ldr  x0, [x2]

	ret
endfunc _getCoreData


/* Function returns the SoC-specific state of the specified cpu
 * in:  x0 = core mask lsb
 * out: x0 = data value
 * uses x0, x1, x2, [x13, x14, x15]
 */
func _getCoreState

	mov   x1, #CORE_STATE_DATA

	/* generate a 0-based core number from the input mask */
	clz   x2, x0
	mov   x0, #63
	sub   x0, x0, x2

	/* x0 = core number (0-based) */
	/* x1 = field offset */

	/* determine if this is bootcore or secondary core */
	cbnz  x0, 1f

	/* get base address for bootcore data */
	ldr  x2, =BC_PSCI_BASE
	add  x2, x2, x1
	b	2f

1:	/* get base address for secondary core data */

	/* x0 = core number (0-based) */
	/* x1 = field offset */

	/* generate number of regions to offset */
	mov   x2, #SEC_REGION_SIZE
	mul   x2, x2, x0

	/* x1 = field offset */
	/* x2 = region offset */

	/* generate the total offset to data element */
	sub   x1, x2, x1

	/* x1 = total offset to data element */

	/* get the base address */
	ldr   x2, =SECONDARY_TOP

	/* apply offset to base addr */
	sub   x2, x2, x1
2:
	/* x2 = data element address */

	dc   ivac, x2
	dsb  sy
	isb

	/* read data */
	ldr  x0, [x2]

	ret
endfunc _getCoreState


/* Function writes the specified data value into the specified cpu
 * core data area
 * in:  x0 = core mask lsb
 *	  x1 = data field offset
 *	  x2 = data value to write/store
 * out: none
 * uses x0, x1, x2, x3, [x13, x14, x15]
 */
func _setCoreData
	/* x0 = core mask */
	/* x1 = field offset */
	/* x2 = data value */

	clz   x3, x0
	mov   x0, #63
	sub   x0, x0, x3

	/* x0 = core number (0-based) */
	/* x1 = field offset */
	/* x2 = data value */

	/* determine if this is bootcore or secondary core */
	cbnz  x0, 1f

	/* get base address for bootcore data */
	ldr  x3, =BC_PSCI_BASE
	add  x3, x3, x1
	b	2f

1:	/* get base address for secondary core data */

	/* x0 = core number (0-based) */
	/* x1 = field offset */
	/* x2 = data value */

	/* generate number of regions to offset */
	mov   x3, #SEC_REGION_SIZE
	mul   x3, x3, x0

	/* x1 = field offset */
	/* x2 = data value */
	/* x3 = region offset */

	/* generate the total offset to data element */
	sub   x1, x3, x1

	/* x1 = total offset to data element */
	/* x2 = data value */

	ldr   x3, =SECONDARY_TOP

	/* apply offset to base addr */
	sub   x3, x3, x1

2:
	/* x2 = data value */
	/* x3 = data element address */

	str   x2, [x3]

	dc	cvac, x3
	dsb   sy
	isb
	ret
endfunc _setCoreData


/* Function stores the specified core state
 * in:  x0 = core mask lsb
 *	x1 = data value to write/store
 * out: none
 * uses x0, x1, x2, x3, [x13, x14, x15]
 */
func _setCoreState
	mov  x2, #CORE_STATE_DATA

	clz   x3, x0
	mov   x0, #63
	sub   x0, x0, x3

	/* x0 = core number (0-based) */
	/* x1 = data value */
	/* x2 = field offset */

	/* determine if this is bootcore or secondary core */
	cbnz  x0, 1f

	/* get base address for bootcore data */
	ldr  x3, =BC_PSCI_BASE
	add  x3, x3, x2
	b	2f

1:	/* get base address for secondary core data */

	/* x0 = core number (0-based) */
	/* x1 = data value */
	/* x2 = field offset */

	/* generate number of regions to offset */
	mov   x3, #SEC_REGION_SIZE
	mul   x3, x3, x0

	/* x1 = data value */
	/* x2 = field offset */
	/* x3 = region offset */

	/* generate the total offset to data element */
	sub   x2, x3, x2

	/* x1 = data value */
	/* x2 = total offset to data element */

	ldr   x3, =SECONDARY_TOP

	/* apply offset to base addr */
	sub   x3, x3, x2

2:
	/* x1 = data value */
	/* x3 = data element address */

	str   x1, [x3]

	dc	civac, x3
	dsb   sy
	isb
	ret
endfunc _setCoreState


/* Function sets the task1 start
 * in:  w0 = value to set flag to
 * out: none
 * uses x0, x1
 */
func _set_task1_start

	ldr  x1, =SMC_TASK1_BASE

	add  x1, x1, #TSK_START_OFFSET
	str  w0, [x1]
	dc   cvac, x1
	dsb  sy
	isb
	ret
endfunc _set_task1_start


/* Function sets the state of the task 1 done flag
 * in:  w0 = value to set flag to
 * out: none
 * uses x0, x1
 */
func _set_task1_done

	ldr  x1, =SMC_TASK1_BASE

	add  x1, x1, #TSK_DONE_OFFSET
	str  w0, [x1]
	dc   cvac, x1
	dsb  sy
	isb
	ret
endfunc _set_task1_done


/* Function initializes the smc global data entries
 * Note: the constant LAST_SMC_GLBL_OFFSET must reference the last entry in the
 *	   smc global region
 * in:  none
 * out: none
 * uses x0, x1, x2
 */
func _init_global_data

	ldr  x1, =SMC_GLBL_BASE

	/* x1 = SMC_GLBL_BASE */

	mov x2, #LAST_SMC_GLBL_OFFSET
	add x2, x2, x1
1:
	str  xzr, [x1]
	dc   cvac, x1
	cmp  x2, x1
	add  x1, x1, #8
	b.hi 1b

	dsb  sy
	isb
	ret
endfunc _init_global_data


/* Function gets the value of the specified global data element
 * in:  x0 = offset of data element
 * out: x0 = requested data element
 * uses x0, x1
 */
func _get_global_data

	ldr  x1, =SMC_GLBL_BASE
	add  x1, x1, x0
	dc   ivac, x1
	isb

	ldr  x0, [x1]
	ret
endfunc _get_global_data


/* Function sets the value of the specified global data element
 * in:  x0 = offset of data element
 *	  x1 = value to write
 * out: none
 * uses x0, x1, x2
 */
func _set_global_data

	ldr  x2, =SMC_GLBL_BASE
	add  x0, x0, x2
	str  x1, [x0]
	dc   cvac, x0

	dsb  sy
	isb
	ret
endfunc _set_global_data


/* Function initializes the core data areas
 * only executed by the boot core
 * in:   none
 * out:  none
 * uses: x0, x1, x2, x3, x4, x5, x6, x7, [x13, x14, x15]
 */
func _initialize_psci
	mov   x7, x30

	/* initialize the bootcore psci data */
	ldr   x5, =BC_PSCI_BASE
	mov   x6, #CORE_RELEASED

	str   x6,  [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5], #8
	dc cvac, x5
	str   xzr, [x5]
	dc cvac, x5
	dsb sy
	isb

	/* see if we have any secondary cores */
	mov   x4, #PLATFORM_CORE_COUNT
	sub   x4, x4, #1
	cbz   x4, 3f

	/* initialize the secondary core's psci data */
	ldr  x5, =SECONDARY_TOP
	/* core mask lsb for core 1 */
	mov  x3, #2
	sub  x5, x5, #SEC_REGION_SIZE

	/* x3 = core1 mask lsb */
	/* x4 = number of secondary cores */
	/* x5 = core1 psci data base address */
2:
	/* set core state in x6 */
	mov  x0, x3
	mov  x6, #CORE_IN_RESET
	bl   _soc_ck_disabled
	cbz  x0, 1f
	mov  x6, #CORE_DISABLED
1:
	add   x2, x5, #CORE_STATE_DATA
	str   x6,  [x2]
	dc cvac, x2
	add   x2, x5, #SPSR_EL3_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #CNTXT_ID_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #START_ADDR_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #LINK_REG_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #GICC_CTLR_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #ABORT_FLAG_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #SCTLR_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #CPUECTLR_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #AUX_01_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #AUX_02_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #AUX_03_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #AUX_04_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #AUX_05_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #SCR_EL3_DATA
	str   xzr, [x2]
	dc cvac, x2
	add   x2, x5, #HCR_EL2_DATA
	str   xzr, [x2]
	dc cvac, x2
	dsb sy
	isb

	sub   x4, x4, #1
	cbz   x4, 3f

	/* generate next core mask */
	lsl  x3, x3, #1

	/* decrement base address to next data area */
	sub  x5, x5, #SEC_REGION_SIZE
	b	2b
3:
	mov   x30, x7
	ret
endfunc _initialize_psci


/* Function initializes the soc init task flags
 * in:  none
 * out: none
 * uses x0, x1, [x13, x14, x15]
 */
func _init_task_flags

	/* get the base address of the first task structure */
	ldr  x0, =SMC_TASK1_BASE

	/* x0 = task1 base address */

	str  wzr, [x0, #TSK_START_OFFSET]
	str  wzr, [x0, #TSK_DONE_OFFSET]
	str  wzr, [x0, #TSK_CORE_OFFSET]
	dc   cvac, x0

	/* move to task2 structure */
	add  x0, x0, #SMC_TASK_OFFSET

	str  wzr, [x0, #TSK_START_OFFSET]
	str  wzr, [x0, #TSK_DONE_OFFSET]
	str  wzr, [x0, #TSK_CORE_OFFSET]
	dc   cvac, x0

	/* move to task3 structure */
	add  x0, x0, #SMC_TASK_OFFSET

	str  wzr, [x0, #TSK_START_OFFSET]
	str  wzr, [x0, #TSK_DONE_OFFSET]
	str  wzr, [x0, #TSK_CORE_OFFSET]
	dc   cvac, x0

	/* move to task4 structure */
	add  x0, x0, #SMC_TASK_OFFSET

	str  wzr, [x0, #TSK_START_OFFSET]
	str  wzr, [x0, #TSK_DONE_OFFSET]
	str  wzr, [x0, #TSK_CORE_OFFSET]
	dc   cvac, x0

	dsb  sy
	isb
	ret
endfunc _init_task_flags
