
/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <asm_macros.S>
#include <assert_macros.S>

#include <lib/psci/psci.h>

#include <bl31_data.h>
#include <plat_psci.h>


#define RESET_RETRY_CNT   800
#define PSCI_ABORT_CNT	100

#if (SOC_CORE_RELEASE)

.global _psci_cpu_on

/*
 * int _psci_cpu_on(u_register_t core_mask)
 * x0   = target cpu core mask
 *
 * Called from C, so save the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 *
 */

func _psci_cpu_on
	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	mov  x6, x0

	/* x0   = core mask (lsb)
	 * x6   = core mask (lsb)
	 */

	/* check if core disabled */
	bl   _soc_ck_disabled		/* 0-2 */
	cbnz w0, psci_disabled

	/* check core data area to see if core cannot be turned on
	 * read the core state
	 */
	mov  x0, x6
	bl   _getCoreState		/* 0-5 */
	mov  x9, x0

	/* x6   = core mask (lsb)
	 * x9   = core state (from data area)
	 */

	cmp  x9, #CORE_DISABLED
	mov  x0, #PSCI_E_DISABLED
	b.eq cpu_on_done

	cmp  x9, #CORE_PENDING
	mov  x0, #PSCI_E_ON_PENDING
	b.eq cpu_on_done

	cmp  x9, #CORE_RELEASED
	mov  x0, #PSCI_E_ALREADY_ON
	b.eq cpu_on_done

8:
	/* x6   = core mask (lsb)
	 * x9   = core state (from data area)
	 */

	cmp  x9, #CORE_WFE
	b.eq core_in_wfe
	cmp  x9, #CORE_IN_RESET
	b.eq core_in_reset
	cmp  x9, #CORE_OFF
	b.eq core_is_off
	cmp  x9, #CORE_OFF_PENDING

	/* if state == CORE_OFF_PENDING, set abort */
	mov  x0, x6
	mov  x1, #ABORT_FLAG_DATA
	mov  x2, #CORE_ABORT_OP
	bl   _setCoreData		/* 0-3, [13-15] */

	ldr  x3, =PSCI_ABORT_CNT
7:
	/* watch for abort to take effect */
	mov  x0, x6
	bl   _getCoreState		/* 0-5 */
	cmp  x0, #CORE_OFF
	b.eq core_is_off
	cmp  x0, #CORE_PENDING
	mov  x0, #PSCI_E_SUCCESS
	b.eq cpu_on_done

	/* loop til finished */
	sub  x3, x3, #1
	cbnz x3, 7b

	/* if we didn't see either CORE_OFF or CORE_PENDING, then this
	 * core is in CORE_OFF_PENDING - exit with success, as the core will
	 * respond to the abort request
	 */
	mov  x0, #PSCI_E_SUCCESS
	b    cpu_on_done

/* this is where we start up a core out of reset */
core_in_reset:
	/* see if the soc-specific module supports this op */
	ldr  x7, =SOC_CORE_RELEASE
	cbnz  x7, 3f

	mov  x0, #PSCI_E_NOT_SUPPORTED
	b    cpu_on_done

	/* x6   = core mask (lsb) */
3:
	/* set core state in data area */
	mov  x0, x6
	mov  x1, #CORE_PENDING
	bl   _setCoreState   			/* 0-3, [13-15] */

	/* release the core from reset */
	mov   x0, x6
	bl    _soc_core_release 		/* 0-3 */
	mov   x0, #PSCI_E_SUCCESS
	b     cpu_on_done

	/* Start up the core that has been powered-down via CPU_OFF
	 */
core_is_off:
	/* see if the soc-specific module supports this op
	 */
	ldr  x7, =SOC_CORE_RESTART
	cbnz x7, 2f

	mov  x0, #PSCI_E_NOT_SUPPORTED
	b    cpu_on_done

	/* x6   = core mask (lsb) */
2:
	/* set core state in data area */
	mov  x0, x6
	mov  x1, #CORE_WAKEUP
	bl   _setCoreState			/* 0-3, [13-15] */

	/* put the core back into service */
	mov  x0, x6
#if (SOC_CORE_RESTART)
	bl   _soc_core_restart			/* 0-5 */
#endif
	mov  x0, #PSCI_E_SUCCESS
	b    cpu_on_done

/* this is where we release a core that is being held in wfe */
core_in_wfe:
	/* x6   = core mask (lsb) */

	/* set core state in data area */
	mov  x0, x6
	mov  x1, #CORE_PENDING
	bl   _setCoreState			/* 0-3, [13-15] */
	dsb  sy
	isb

	/* put the core back into service */
	sev
	sev
	isb
	mov  x0, #PSCI_E_SUCCESS

cpu_on_done:
	/* restore the aarch32/64 non-volatile registers */
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	b    psci_completed
endfunc _psci_cpu_on

#endif


#if (SOC_CORE_OFF)

.global _psci_cpu_prep_off
.global _psci_cpu_off_wfi

/*
 * void _psci_cpu_prep_off(u_register_t core_mask)
 * this function performs the SoC-specific programming prior
 * to shutting the core down
 * x0 = core_mask
 *
 * called from C, so save the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 */

func _psci_cpu_prep_off

	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	mov  x10, x0			/* x10 = core_mask */

	/* the core does not return from cpu_off, so no need
	 * to save/restore non-volatile registers
	 */

	/* mask interrupts by setting DAIF[7:4] to 'b1111 */
	msr DAIFSet, #0xF

	/* read cpuectlr and save current value */
	mrs   x4, CPUECTLR_EL1
	mov   x1, #CPUECTLR_DATA
	mov   x2, x4
	mov   x0, x10
	bl    _setCoreData

	/* remove the core from coherency */
	bic   x4, x4, #CPUECTLR_SMPEN_MASK
	msr   CPUECTLR_EL1, x4

	/* save scr_el3 */
	mov  x0, x10
	mrs  x4, SCR_EL3
	mov  x2, x4
	mov  x1, #SCR_EL3_DATA
	bl    _setCoreData

	/* x4 = scr_el3 */

	/* secure SGI (FIQ) taken to EL3, set SCR_EL3[FIQ] */
	orr   x4, x4, #SCR_FIQ_MASK
	msr   scr_el3, x4

	/* x10 = core_mask */

	/* prep the core for shutdown */
	mov  x0, x10
	bl   _soc_core_prep_off

	/* restore the aarch32/64 non-volatile registers */
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	b    psci_completed
endfunc _psci_cpu_prep_off

/*
 * void _psci_cpu_off_wfi(u_register_t core_mask, u_register_t resume_addr)
 *   - this function shuts down the core
 *   - this function does not return!!
 */

func _psci_cpu_off_wfi
	/* save the wakeup address */
	mov  x29, x1

	/* x0 = core_mask */

	/* shutdown the core */
	bl   _soc_core_entr_off

	/* branch to resume execution */
	br   x29
endfunc _psci_cpu_off_wfi

#endif


#if (SOC_CORE_RESTART)

.global _psci_wakeup

/*
 * void _psci_wakeup(u_register_t core_mask)
 * this function performs the SoC-specific programming
 * after a core wakes up from OFF
 * x0 = core mask
 *
 * called from C, so save the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 */

func _psci_wakeup

	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	mov  x4, x0			/* x4 = core mask */

	/* restore scr_el3 */
	mov  x0, x4
	mov  x1, #SCR_EL3_DATA
	bl   _getCoreData
	/* x0 = saved scr_el3 */
	msr  SCR_EL3, x0

	/* x4 = core mask */

	/* restore CPUECTLR */
	mov   x0, x4
	mov   x1, #CPUECTLR_DATA
	bl    _getCoreData
	orr   x0, x0, #CPUECTLR_SMPEN_MASK
	msr   CPUECTLR_EL1, x0

	/* x4 = core mask */

	/* start the core back up */
	mov   x0, x4
	bl   _soc_core_exit_off

	/* restore the aarch32/64 non-volatile registers
	 */
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	b    psci_completed
endfunc _psci_wakeup

#endif


#if (SOC_SYSTEM_RESET)

.global _psci_system_reset

func _psci_system_reset

	/* system reset is mandatory
	 * system reset is soc-specific
	 * Note: under no circumstances do we return from this call
	 */
	bl   _soc_sys_reset
endfunc _psci_system_reset

#endif


#if (SOC_SYSTEM_OFF)

.global _psci_system_off

func _psci_system_off

	/* system off is mandatory
	 * system off is soc-specific
	 * Note: under no circumstances do we return from this call */
	b    _soc_sys_off
endfunc _psci_system_off

#endif


#if (SOC_CORE_STANDBY)

.global _psci_core_entr_stdby
.global _psci_core_prep_stdby
.global _psci_core_exit_stdby

/*
 * void _psci_core_entr_stdby(u_register_t core_mask) - this
 * is the fast-path for simple core standby
 */

func _psci_core_entr_stdby
	stp  x4,  x5, [sp, #-16]!
	stp  x6, x30, [sp, #-16]!

	mov  x5, x0		/* x5 = core mask */

	/* save scr_el3 */
	mov  x0, x5
	mrs  x4, SCR_EL3
	mov  x2, x4
	mov  x1, #SCR_EL3_DATA
	bl    _setCoreData

	/* x4 = SCR_EL3
	 * x5 = core mask
	 */

	/* allow interrupts @ EL3 */
	orr  x4, x4, #(SCR_IRQ_MASK | SCR_FIQ_MASK)
	msr  SCR_EL3, x4

	/* x5 = core mask */

	/* put the core into standby */
	mov  x0, x5
	bl   _soc_core_entr_stdby

	/* restore scr_el3 */
	mov  x0, x5
	mov  x1, #SCR_EL3_DATA
	bl   _getCoreData
	/* x0 = saved scr_el3 */
	msr  SCR_EL3, x0

	ldp  x6,  x30, [sp], #16
	ldp  x4,  x5,  [sp], #16
	isb
	ret
endfunc _psci_core_entr_stdby

/*
 * void _psci_core_prep_stdby(u_register_t core_mask) - this
 * sets up the core to enter standby state thru the normal path
 */

func _psci_core_prep_stdby
	stp  x4,  x5, [sp, #-16]!
	stp  x6, x30, [sp, #-16]!

	mov  x5, x0

	/* x5 = core mask */

	/* save scr_el3 */
	mov  x0, x5
	mrs  x4, SCR_EL3
	mov  x2, x4
	mov  x1, #SCR_EL3_DATA
	bl    _setCoreData

	/* allow interrupts @ EL3 */
	orr  x4, x4, #(SCR_IRQ_MASK | SCR_FIQ_MASK)
	msr  SCR_EL3, x4

	/* x5 = core mask */

	/* call for any SoC-specific programming */
	mov  x0, x5
	bl   _soc_core_prep_stdby

	ldp  x6,  x30, [sp], #16
	ldp  x4,  x5,  [sp], #16
	isb
	ret
endfunc _psci_core_prep_stdby

/*
 * void _psci_core_exit_stdby(u_register_t core_mask) - this
 * exits the core from standby state thru the normal path
 */

func _psci_core_exit_stdby
	stp  x4,  x5, [sp, #-16]!
	stp  x6, x30, [sp, #-16]!

	mov  x5, x0

	/* x5 = core mask */

	/* restore scr_el3 */
	mov  x0, x5
	mov  x1, #SCR_EL3_DATA
	bl   _getCoreData
	/* x0 = saved scr_el3 */
	msr  SCR_EL3, x0

	/* x5 = core mask */

	/* perform any SoC-specific programming after standby state */
	mov  x0, x5
	bl   _soc_core_exit_stdby

	ldp  x6,  x30, [sp], #16
	ldp  x4,  x5,  [sp], #16
	isb
	ret
endfunc _psci_core_exit_stdby

#endif


#if (SOC_CORE_PWR_DWN)

.global _psci_core_prep_pwrdn
.global _psci_cpu_pwrdn_wfi
.global _psci_core_exit_pwrdn

/*
 * void _psci_core_prep_pwrdn_(u_register_t core_mask)
 * this function prepares the core for power-down
 * x0 = core mask
 *
 * called from C, so save the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 */

func _psci_core_prep_pwrdn
	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	mov  x6, x0

	/* x6 = core mask */

	/* mask interrupts by setting DAIF[7:4] to 'b1111 */
	msr DAIFSet, #0xF

	/* save scr_el3 */
	mov  x0, x6
	mrs  x4, SCR_EL3
	mov  x2, x4
	mov  x1, #SCR_EL3_DATA
	bl    _setCoreData

	/* allow interrupts @ EL3 */
	orr  x4, x4, #(SCR_IRQ_MASK | SCR_FIQ_MASK)
	msr  SCR_EL3, x4

	/* save cpuectlr */
	mov  x0, x6
	mov  x1, #CPUECTLR_DATA
	mrs  x2, CPUECTLR_EL1
	bl   _setCoreData

	/* x6 = core mask */

	/* SoC-specific programming for power-down */
	mov  x0, x6
	bl  _soc_core_prep_pwrdn

	/* restore the aarch32/64 non-volatile registers
	 */
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	b    psci_completed
endfunc _psci_core_prep_pwrdn

/*
 * void _psci_cpu_pwrdn_wfi(u_register_t core_mask, u_register_t resume_addr)
 * this function powers down the core
 */

func _psci_cpu_pwrdn_wfi
	/* save the wakeup address */
	mov  x29, x1

	/* x0 = core mask */

	/* shutdown the core */
	bl   _soc_core_entr_pwrdn

	/* branch to resume execution */
	br   x29
endfunc _psci_cpu_pwrdn_wfi

/*
 * void _psci_core_exit_pwrdn_(u_register_t core_mask)
 * this function cleans up after a core power-down
 * x0 = core mask
 *
 * called from C, so save the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 */

func _psci_core_exit_pwrdn
	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	mov  x5, x0			/* x5 = core mask */

	/* restore scr_el3 */
	mov  x0, x5
	mov  x1, #SCR_EL3_DATA
	bl   _getCoreData
	/* x0 = saved scr_el3 */
	msr  SCR_EL3, x0

	/* x5 = core mask */

	/* restore cpuectlr */
	mov  x0, x5
	mov  x1, #CPUECTLR_DATA
	bl   _getCoreData
	/* make sure smp is set */
	orr  x0, x0, #CPUECTLR_SMPEN_MASK
	msr  CPUECTLR_EL1, x0

	/* x5 = core mask */

	/* SoC-specific cleanup */
	mov  x0, x5
	bl   _soc_core_exit_pwrdn

	/* restore the aarch32/64 non-volatile registers
	 */
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	b    psci_completed
endfunc _psci_core_exit_pwrdn

#endif

#if (SOC_CLUSTER_STANDBY)

.global _psci_clstr_prep_stdby
.global _psci_clstr_exit_stdby

/*
 * void _psci_clstr_prep_stdby(u_register_t core_mask) - this
 * sets up the clstr to enter standby state thru the normal path
 */

func _psci_clstr_prep_stdby
	stp  x4,  x5, [sp, #-16]!
	stp  x6, x30, [sp, #-16]!

	mov  x5, x0

	/* x5 = core mask */

	/* save scr_el3 */
	mov  x0, x5
	mrs  x4, SCR_EL3
	mov  x2, x4
	mov  x1, #SCR_EL3_DATA
	bl    _setCoreData

	/* allow interrupts @ EL3 */
	orr  x4, x4, #(SCR_IRQ_MASK | SCR_FIQ_MASK)
	msr  SCR_EL3, x4

	/* x5 = core mask */

	/* call for any SoC-specific programming */
	mov  x0, x5
	bl   _soc_clstr_prep_stdby

	ldp  x6,  x30, [sp], #16
	ldp  x4,  x5,  [sp], #16
	isb
	ret
endfunc _psci_clstr_prep_stdby

/*
 * void _psci_clstr_exit_stdby(u_register_t core_mask) - this
 * exits the clstr from standby state thru the normal path
 */

func _psci_clstr_exit_stdby
	stp  x4,  x5, [sp, #-16]!
	stp  x6, x30, [sp, #-16]!

	mov  x5, x0			/* x5 = core mask */

	/* restore scr_el3 */
	mov  x0, x5
	mov  x1, #SCR_EL3_DATA
	bl   _getCoreData
	/* x0 = saved scr_el3 */
	msr  SCR_EL3, x0

	/* x5 = core mask */

	/* perform any SoC-specific programming after standby state */
	mov  x0, x5
	bl   _soc_clstr_exit_stdby

	ldp  x6,  x30, [sp], #16
	ldp  x4,  x5,  [sp], #16
	isb
	ret
endfunc _psci_clstr_exit_stdby

#endif

#if (SOC_CLUSTER_PWR_DWN)

.global _psci_clstr_prep_pwrdn
.global _psci_clstr_exit_pwrdn

/*
 * void _psci_clstr_prep_pwrdn_(u_register_t core_mask)
 * this function prepares the cluster+core for power-down
 * x0 = core mask
 *
 * called from C, so save the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 */

func _psci_clstr_prep_pwrdn
	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	mov  x6, x0			/* x6 = core mask */

	/* mask interrupts by setting DAIF[7:4] to 'b1111 */
	msr DAIFSet, #0xF

	/* save scr_el3 */
	mov  x0, x6
	mrs  x4, SCR_EL3
	mov  x2, x4
	mov  x1, #SCR_EL3_DATA
	bl    _setCoreData

	/* allow interrupts @ EL3 */
	orr  x4, x4, #(SCR_IRQ_MASK | SCR_FIQ_MASK)
	msr  SCR_EL3, x4

	/* save cpuectlr */
	mov  x0, x6
	mov  x1, #CPUECTLR_DATA
	mrs  x2, CPUECTLR_EL1
	mov  x4, x2
	bl   _setCoreData

	/* remove core from coherency */
	bic   x4, x4, #CPUECTLR_SMPEN_MASK
	msr   CPUECTLR_EL1, x4

	/* x6 = core mask */

	/* SoC-specific programming for power-down */
	mov  x0, x6
	bl  _soc_clstr_prep_pwrdn

	/* restore the aarch32/64 non-volatile registers
	 */
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	b    psci_completed
endfunc _psci_clstr_prep_pwrdn

/*
 * void _psci_clstr_exit_pwrdn_(u_register_t core_mask)
 * this function cleans up after a cluster power-down
 * x0 = core mask
 *
 * called from C, so save the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 */

func _psci_clstr_exit_pwrdn
	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	mov  x4, x0			/* x4 = core mask */

	/* restore scr_el3 */
	mov  x0, x4
	mov  x1, #SCR_EL3_DATA
	bl   _getCoreData
	/* x0 = saved scr_el3 */
	msr  SCR_EL3, x0

	/* x4 = core mask */

	/* restore cpuectlr */
	mov  x0, x4
	mov  x1, #CPUECTLR_DATA
	bl   _getCoreData
	/* make sure smp is set */
	orr  x0, x0, #CPUECTLR_SMPEN_MASK
	msr  CPUECTLR_EL1, x0

	/* x4 = core mask */

	/* SoC-specific cleanup */
	mov  x0, x4
	bl   _soc_clstr_exit_pwrdn

	/* restore the aarch32/64 non-volatile registers
	 */
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	b    psci_completed
endfunc _psci_clstr_exit_pwrdn

#endif

#if (SOC_SYSTEM_STANDBY)

.global _psci_sys_prep_stdby
.global _psci_sys_exit_stdby

/*
 * void _psci_sys_prep_stdby(u_register_t core_mask) - this
 * sets up the system to enter standby state thru the normal path
 */

func _psci_sys_prep_stdby
	stp  x4,  x5, [sp, #-16]!
	stp  x6, x30, [sp, #-16]!

	mov  x5, x0			/* x5 = core mask */

	/* save scr_el3 */
	mov  x0, x5
	mrs  x4, SCR_EL3
	mov  x2, x4
	mov  x1, #SCR_EL3_DATA
	bl    _setCoreData

	/* allow interrupts @ EL3 */
	orr  x4, x4, #(SCR_IRQ_MASK | SCR_FIQ_MASK)
	msr  SCR_EL3, x4

	/* x5 = core mask */

	/* call for any SoC-specific programming */
	mov  x0, x5
	bl   _soc_sys_prep_stdby

	ldp  x6,  x30, [sp], #16
	ldp  x4,  x5,  [sp], #16
	isb
	ret
endfunc _psci_sys_prep_stdby

/*
 * void _psci_sys_exit_stdby(u_register_t core_mask) - this
 * exits the system from standby state thru the normal path
 */

func _psci_sys_exit_stdby
	stp  x4,  x5, [sp, #-16]!
	stp  x6, x30, [sp, #-16]!

	mov  x5, x0

	/* x5 = core mask */

	/* restore scr_el3 */
	mov  x0, x5
	mov  x1, #SCR_EL3_DATA
	bl   _getCoreData
	/* x0 = saved scr_el3 */
	msr  SCR_EL3, x0

	/* x5 = core mask */

	/* perform any SoC-specific programming after standby state */
	mov  x0, x5
	bl   _soc_sys_exit_stdby

	ldp  x6,  x30, [sp], #16
	ldp  x4,  x5,  [sp], #16
	isb
	ret
endfunc _psci_sys_exit_stdby

#endif

#if (SOC_SYSTEM_PWR_DWN)

.global _psci_sys_prep_pwrdn
.global _psci_sys_pwrdn_wfi
.global _psci_sys_exit_pwrdn

/*
 * void _psci_sys_prep_pwrdn_(u_register_t core_mask)
 * this function prepares the system+core for power-down
 * x0 = core mask
 *
 * called from C, so save the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 */

func _psci_sys_prep_pwrdn
	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	mov  x6, x0			/* x6 = core mask */

	/* mask interrupts by setting DAIF[7:4] to 'b1111 */
	msr DAIFSet, #0xF

	/* save scr_el3 */
	mov  x0, x6
	mrs  x4, SCR_EL3
	mov  x2, x4
	mov  x1, #SCR_EL3_DATA
	bl    _setCoreData

	/* allow interrupts @ EL3 */
	orr  x4, x4, #(SCR_IRQ_MASK | SCR_FIQ_MASK)
	msr  SCR_EL3, x4

	/* save cpuectlr */
	mov  x0, x6
	mov  x1, #CPUECTLR_DATA
	mrs  x2, CPUECTLR_EL1
	mov  x4, x2
	bl   _setCoreData

	/* remove core from coherency */
	bic   x4, x4, #CPUECTLR_SMPEN_MASK
	msr   CPUECTLR_EL1, x4

	/* x6 = core mask */

	/* SoC-specific programming for power-down */
	mov  x0, x6
	bl  _soc_sys_prep_pwrdn

	/* restore the aarch32/64 non-volatile registers
	 */
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	b    psci_completed
endfunc _psci_sys_prep_pwrdn


/*
 * void _psci_sys_pwrdn_wfi(u_register_t core_mask, u_register_t resume_addr)
 * this function powers down the system
 */

func _psci_sys_pwrdn_wfi
	/* save the wakeup address */
	mov  x29, x1

	/* x0 = core mask */

	/* shutdown the system */
	bl   _soc_sys_pwrdn_wfi

	/* branch to resume execution */
	br   x29
endfunc _psci_sys_pwrdn_wfi

/*
 * void _psci_sys_exit_pwrdn_(u_register_t core_mask)
 * this function cleans up after a system power-down
 * x0 = core mask
 *
 * Called from C, so save the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 */

func _psci_sys_exit_pwrdn

	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x14, x15, [sp, #-16]!
	stp  x16, x17, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	mov  x4, x0			/* x4 = core mask */

	/* restore scr_el3 */
	mov  x0, x4
	mov  x1, #SCR_EL3_DATA
	bl   _getCoreData

	/* x0 = saved scr_el3 */
	msr  SCR_EL3, x0

	/* x4 = core mask */

	/* restore cpuectlr */
	mov  x0, x4
	mov  x1, #CPUECTLR_DATA
	bl   _getCoreData

	/* make sure smp is set */
	orr  x0, x0, #CPUECTLR_SMPEN_MASK
	msr  CPUECTLR_EL1, x0

	/* x4 = core mask */

	/* SoC-specific cleanup */
	mov  x0, x4
	bl   _soc_sys_exit_pwrdn

	/* restore the aarch32/64 non-volatile registers
	 */
	ldp  x18, x30, [sp], #16
	ldp  x16, x17, [sp], #16
	ldp  x14, x15, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	b    psci_completed
endfunc _psci_sys_exit_pwrdn

#endif


/* psci std returns */
func psci_disabled
	ldr  w0, =PSCI_E_DISABLED
	b    psci_completed
endfunc psci_disabled


func psci_not_present
	ldr  w0, =PSCI_E_NOT_PRESENT
	b    psci_completed
endfunc psci_not_present


func psci_on_pending
	ldr  w0, =PSCI_E_ON_PENDING
	b    psci_completed
endfunc psci_on_pending


func psci_already_on
	ldr  w0, =PSCI_E_ALREADY_ON
	b    psci_completed
endfunc psci_already_on


func psci_failure
	ldr  w0, =PSCI_E_INTERN_FAIL
	b    psci_completed
endfunc psci_failure


func psci_unimplemented
	ldr  w0, =PSCI_E_NOT_SUPPORTED
	b    psci_completed
endfunc psci_unimplemented


func psci_denied
	ldr  w0, =PSCI_E_DENIED
	b    psci_completed
endfunc psci_denied


func psci_invalid
	ldr  w0, =PSCI_E_INVALID_PARAMS
	b    psci_completed
endfunc psci_invalid


func psci_success
	mov  x0, #PSCI_E_SUCCESS
endfunc psci_success


func psci_completed
	/* x0 = status code */
	ret
endfunc psci_completed
