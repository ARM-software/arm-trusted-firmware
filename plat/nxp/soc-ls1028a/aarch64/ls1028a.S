/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

	.section .text, "ax"

#include <asm_macros.S>

#include <lib/psci/psci.h>
#include <nxp_timer.h>
#include <plat_gic.h>
#include <pmu.h>

#include <bl31_data.h>
#include <plat_psci.h>
#include <platform_def.h>

	.global soc_init_lowlevel
	.global soc_init_percpu
	.global _set_platform_security
	.global _soc_set_start_addr

	.global _soc_core_release
	.global _soc_ck_disabled
	.global _soc_core_restart
	.global _soc_core_prep_off
	.global _soc_core_entr_off
	.global _soc_core_exit_off
	.global _soc_sys_reset
	.global _soc_sys_off
	.global _soc_core_prep_stdby
	.global _soc_core_entr_stdby
	.global _soc_core_exit_stdby
	.global _soc_core_prep_pwrdn
	.global _soc_core_entr_pwrdn
	.global _soc_core_exit_pwrdn
	.global _soc_clstr_prep_stdby
	.global _soc_clstr_exit_stdby
	.global _soc_clstr_prep_pwrdn
	.global _soc_clstr_exit_pwrdn
	.global _soc_sys_prep_stdby
	.global _soc_sys_exit_stdby
	.global _soc_sys_prep_pwrdn
	.global _soc_sys_pwrdn_wfi
	.global _soc_sys_exit_pwrdn

	.equ TZPCDECPROT_0_SET_BASE, 0x02200804
	.equ TZPCDECPROT_1_SET_BASE, 0x02200810
	.equ TZPCDECPROT_2_SET_BASE, 0x0220081C

	.equ TZASC_REGION_ATTRIBUTES_0_0, 0x01100110

/*
 * This function initialize the soc.
 * in: void
 * out: void
 * uses x0 - x11
 */
func soc_init_lowlevel
	/*
	 * Called from C, so save the non-volatile regs
	 * save these as pairs of registers to maintain the
	 * required 16-byte alignment on the stack
	 */
	stp	x4,  x5,  [sp, #-16]!
	stp	x6,  x7,  [sp, #-16]!
	stp	x8,  x9,  [sp, #-16]!
	stp	x10, x11, [sp, #-16]!
	stp	x12, x13, [sp, #-16]!
	stp	x18, x30, [sp, #-16]!

	/*
	 * Make sure the personality has been established by releasing cores
	 * that are marked "to-be-disabled" from reset
	 */
	bl	release_disabled  /* 0-8 */

	/* Set SCRATCHRW7 to 0x0 */
	ldr	x0, =DCFG_SCRATCHRW7_OFFSET
	mov	x1, xzr
	bl	_write_reg_dcfg

	/* Restore the aarch32/64 non-volatile registers */
	ldp	x18, x30, [sp], #16
	ldp	x12, x13, [sp], #16
	ldp	x10, x11, [sp], #16
	ldp	x8,  x9,  [sp], #16
	ldp	x6,  x7,  [sp], #16
	ldp	x4,  x5,  [sp], #16
	ret
endfunc soc_init_lowlevel

/*
 * void soc_init_percpu(void)
 *
 * This function performs any soc-specific initialization that is needed on
 * a per-core basis
 * in:  none
 * out: none
 * uses x0 - x3
 */
func soc_init_percpu
	stp	x4,  x30,  [sp, #-16]!

	bl	plat_my_core_mask
	mov	x2, x0

	/* x2 = core mask */

	/* see if this core is marked for prefetch disable */
	mov	x0, #PREFETCH_DIS_OFFSET
	bl	_get_global_data  /* 0-1 */
	tst	x0, x2
	b.eq	1f
	bl	_disable_ldstr_pfetch_A72  /* 0 */
1:
	mov	x0, #NXP_PMU_ADDR
	bl	enable_timer_base_to_cluster

	ldp	x4,  x30,  [sp], #16
	ret
endfunc soc_init_percpu

/*
 * This function determines if a core is disabled via COREDISABLEDSR
 * in:  w0  = core_mask_lsb
 * out: w0  = 0, core not disabled
 *      w0 != 0, core disabled
 * uses x0, x1
 */
func _soc_ck_disabled
	/* get base addr of dcfg block */
	ldr	x1, =NXP_DCFG_ADDR

	/* read COREDISABLEDSR */
	ldr	w1, [x1, #DCFG_COREDISABLEDSR_OFFSET]

	/* test core bit */
	and	w0, w1, w0

	ret
endfunc _soc_ck_disabled

/*
 * This function sets the security mechanisms in the SoC to implement the
 * Platform Security Policy
 */
func _set_platform_security
	mov	x3, x30

#if (!SUPPRESS_TZC)
	/* initialize the tzpc */
	bl	init_tzpc
#endif

#if (!SUPPRESS_SEC)
	/* initialize secmon */
	bl	initSecMon
#endif

	mov	x30, x3
	ret
endfunc _set_platform_security

/*
 * Part of CPU_ON
 *
 * This function releases a secondary core from reset
 * in:   x0 = core_mask_lsb
 * out:  none
 * uses: x0 - x3
 */
_soc_core_release:
	mov	x3, x30

	/*
	 * Write to CORE_HOLD to tell the bootrom that we want this core
	 * to run
	 */
	ldr	x1, =NXP_SEC_REGFILE_ADDR
	str	w0, [x1, #CORE_HOLD_OFFSET]

	/* Read-modify-write BRRL to release core */
	mov	x1, #NXP_RESET_ADDR
	ldr	w2, [x1, #BRR_OFFSET]
	orr	w2, w2, w0
	str	w2, [x1, #BRR_OFFSET]
	dsb	sy
	isb

	/* Send event */
	sev
	isb

	mov	x30, x3
	ret

/*
 * This function writes a 64-bit address to bootlocptrh/l
 * in:  x0, 64-bit address to write to BOOTLOCPTRL/H
 * uses x0, x1, x2
 */
func _soc_set_start_addr
	/* Get the 64-bit base address of the dcfg block */
	ldr	x2, =NXP_DCFG_ADDR

	/* Write the 32-bit BOOTLOCPTRL register */
	mov	x1, x0
	str	w1, [x2, #DCFG_BOOTLOCPTRL_OFFSET]

	/* Write the 32-bit BOOTLOCPTRH register */
	lsr	x1, x0, #32
	str	w1, [x2, #DCFG_BOOTLOCPTRH_OFFSET]
	ret
endfunc _soc_set_start_addr

/*
 * Part of CPU_ON
 *
 * This function restarts a core shutdown via _soc_core_entr_off
 * in:  x0 = core mask lsb (of the target cpu)
 * out: x0 == 0, on success
 *      x0 != 0, on failure
 * uses x0 - x6
 */
_soc_core_restart:
	mov	x6, x30
	mov	x4, x0

	/* pgm GICD_CTLR - enable secure grp0  */
	mov	x5, #NXP_GICD_ADDR
	ldr	w2, [x5, #GICD_CTLR_OFFSET]
	orr	w2, w2, #GICD_CTLR_EN_GRP_0
	str	w2, [x5, #GICD_CTLR_OFFSET]
	dsb	sy
	isb

	/* Poll on RWP til write completes */
4:
	ldr	w2, [x5, #GICD_CTLR_OFFSET]
	tst	w2, #GICD_CTLR_RWP
	b.ne	4b

	/*
	 * x4 = core mask lsb
	 * x5 = gicd base addr
	 */

	mov	x0, x4
	bl	get_mpidr_value

	/* Generate target list bit */
	and	x1, x0, #MPIDR_AFFINITY0_MASK
	mov	x2, #1
	lsl	x2, x2, x1

	/* Get the affinity1 field */
	and	x1, x0, #MPIDR_AFFINITY1_MASK
	lsl	x1, x1, #8
	orr	x2, x2, x1

	/* Insert the INTID for SGI15 */
	orr	x2, x2, #ICC_SGI0R_EL1_INTID

	/* Fire the SGI */
	msr	ICC_SGI0R_EL1, x2
	dsb	sy
	isb

	/* Load '0' on success */
	mov	x0, xzr

	mov	x30, x6
	ret

/*
 * Part of CPU_OFF
 *
 * This function programs SoC & GIC registers in preparation for shutting down
 * the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0 - x7
 */
_soc_core_prep_off:
	mov	x8, x30
	mov	x7, x0

	/* x7 = core mask lsb */

	mrs	x1, CPUECTLR_EL1

	/* Set smp and disable L2 snoops in cpuectlr */
	orr	x1, x1, #CPUECTLR_SMPEN_EN
	orr	x1, x1, #CPUECTLR_DISABLE_TWALK_PREFETCH
	bic	x1, x1, #CPUECTLR_INS_PREFETCH_MASK
	bic	x1, x1, #CPUECTLR_DAT_PREFETCH_MASK

	/* Set retention control in cpuectlr */
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	orr	x1, x1, #CPUECTLR_TIMER_2TICKS
	msr	CPUECTLR_EL1, x1

	/* Get redistributor rd base addr for this core */
	mov	x0, x7
	bl	get_gic_rd_base
	mov	x6, x0

	/* Get redistributor sgi base addr for this core */
	mov	x0, x7
	bl	get_gic_sgi_base
	mov	x5, x0

	/*
	 * x5 = gicr sgi base addr
	 * x6 = gicr rd  base addr
	 * x7 = core mask lsb
	 */

	/* Disable SGI 15 at redistributor - GICR_ICENABLER0 */
	mov	w3, #GICR_ICENABLER0_SGI15
	str	w3, [x5, #GICR_ICENABLER0_OFFSET]
2:
	/* Poll on rwp bit in GICR_CTLR */
	ldr	w4, [x6, #GICR_CTLR_OFFSET]
	tst	w4, #GICR_CTLR_RWP
	b.ne	2b

	/* Disable GRP1 interrupts at cpu interface */
	msr	ICC_IGRPEN1_EL3, xzr

	/* Disable GRP0 ints at cpu interface */
	msr	ICC_IGRPEN0_EL1, xzr

	/* Program the redistributor - poll on GICR_CTLR.RWP as needed */

	/* Define SGI 15 as Grp0 - GICR_IGROUPR0 */
	ldr	w4, [x5, #GICR_IGROUPR0_OFFSET]
	bic	w4, w4, #GICR_IGROUPR0_SGI15
	str	w4, [x5, #GICR_IGROUPR0_OFFSET]

	/* Define SGI 15 as Grp0 - GICR_IGRPMODR0 */
	ldr	w3, [x5, #GICR_IGRPMODR0_OFFSET]
	bic	w3, w3, #GICR_IGRPMODR0_SGI15
	str	w3, [x5, #GICR_IGRPMODR0_OFFSET]

	/* Set priority of SGI 15 to highest (0x0) - GICR_IPRIORITYR3 */
	ldr	w4, [x5, #GICR_IPRIORITYR3_OFFSET]
	bic	w4, w4, #GICR_IPRIORITYR3_SGI15_MASK
	str	w4, [x5, #GICR_IPRIORITYR3_OFFSET]

	/* Enable SGI 15 at redistributor - GICR_ISENABLER0 */
	mov	w3, #GICR_ISENABLER0_SGI15
	str	w3, [x5, #GICR_ISENABLER0_OFFSET]
	dsb	sy
	isb
3:
	/* Poll on rwp bit in GICR_CTLR */
	ldr	w4, [x6, #GICR_CTLR_OFFSET]
	tst	w4, #GICR_CTLR_RWP
	b.ne	3b

	/* Quiesce the debug interfaces */
	mrs	x3, osdlr_el1
	orr	x3, x3, #OSDLR_EL1_DLK_LOCK
	msr	osdlr_el1, x3
	isb

	/* Enable grp0 ints */
	mov	x3, #ICC_IGRPEN0_EL1_EN
	msr	ICC_IGRPEN0_EL1, x3

	/*
	 * x5 = gicr sgi base addr
	 * x6 = gicr rd  base addr
	 * x7 = core mask lsb
	 */

	/* Clear any pending interrupts */
	mvn	w1, wzr
	str	w1, [x5, #GICR_ICPENDR0_OFFSET]

	/* Make sure system counter is enabled */
	ldr	x3, =NXP_TIMER_ADDR
	ldr	w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
	tst	w0, #SYS_COUNTER_CNTCR_EN
	b.ne	4f
	orr	w0, w0, #SYS_COUNTER_CNTCR_EN
	str	w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
4:
	/* Enable the core timer and mask timer interrupt */
	mov	x1, #CNTP_CTL_EL0_EN
	orr	x1, x1, #CNTP_CTL_EL0_IMASK
	msr	cntp_ctl_el0, x1

	isb
	mov	x30, x8
	ret

/*
 * Part of CPU_OFF
 *
 * This function performs the final steps to shutdown the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0 - x5
 */
_soc_core_entr_off:
	mov	x5, x30
	mov	x4, x0

	/* x4 = core mask */
1:
	/* Enter low-power state by executing wfi */
	wfi

	/* See if SGI15 woke us up */
	mrs	x2, ICC_IAR0_EL1
	mov	x3, #ICC_IAR0_EL1_SGI15
	cmp	x2, x3
	b.ne	1b

	/* Deactivate the int */
	msr	ICC_EOIR0_EL1, x2

	/* x4 = core mask */
2:
	/* Check if core has been turned on */
	mov	x0, x4
	bl	_getCoreState

	/* x0 = core state */

	cmp	x0, #CORE_WAKEUP
	b.ne	1b

	/* If we get here, then we have exited the wfi */
	mov	x30, x5
	ret

/*
 * Part of CPU_OFF
 *
 * This function starts the process of starting a core back up
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6
 */
_soc_core_exit_off:
	mov	x6, x30
	mov	x5, x0

	/* Disable forwarding of GRP0 ints at cpu interface */
	msr	ICC_IGRPEN0_EL1, xzr

	/* Get redistributor sgi base addr for this core */
	mov	x0, x5
	bl	get_gic_sgi_base
	mov	x4, x0

	/* x4 = gicr sgi base addr */
	/* x5 = core mask */

	/* Disable SGI 15 at redistributor - GICR_ICENABLER0 */
	mov	w1, #GICR_ICENABLER0_SGI15
	str	w1, [x4, #GICR_ICENABLER0_OFFSET]

	/* Get redistributor rd base addr for this core */
	mov	x0, x5
	bl	get_gic_rd_base
	mov	x4, x0

	/* x4 = gicr rd  base addr */
2:
	/* Poll on rwp bit in GICR_CTLR */
	ldr	w2, [x4, #GICR_CTLR_OFFSET]
	tst	w2, #GICR_CTLR_RWP
	b.ne	2b

	/* x4 = gicr rd  base addr */

	/* Unlock the debug interfaces */
	mrs	x3, osdlr_el1
	bic	x3, x3, #OSDLR_EL1_DLK_LOCK
	msr	osdlr_el1, x3
	isb

	dsb	sy
	isb
	mov	x30, x6
	ret

/*
 * This function requests a reset of the entire SOC
 * in:  none
 * out: none
 * uses: x0, x1, x2, x3, x4, x5, x6
 */
_soc_sys_reset:
	mov	x3, x30

	/* Make sure the mask is cleared in the reset request mask register */
	mov	x0, #RST_RSTRQMR1_OFFSET
	mov	w1, wzr
	bl	_write_reg_reset

	/* Set the reset request */
	mov	x4, #RST_RSTCR_OFFSET
	mov	x0, x4
	mov	w1, #RSTCR_RESET_REQ
	bl	_write_reg_reset

	/* x4 = RST_RSTCR_OFFSET */

	/*
	 * Just in case this address range is mapped as cacheable,
	 * flush the write out of the dcaches
	 */
	mov	x2, #NXP_RESET_ADDR
	add	x2, x2, x4
	dc	cvac, x2
	dsb	st
	isb

	/* This function does not return */
1:
	wfi
	b	1b

/*
 * Part of SYSTEM_OFF
 *
 * This function turns off the SoC clocks
 * Note: this function is not intended to return, and the only allowable
 *       recovery is POR
 * in:  none
 * out: none
 * uses x0, x1, x2, x3
 */
_soc_sys_off:
	/*
	 * Disable sec, spi and flexspi
	 * TBD - Check if eNETC needs to be disabled
	 */
	ldr	x2, =NXP_DCFG_ADDR
	ldr	x0, =DCFG_DEVDISR1_OFFSET
	ldr	w1, =DCFG_DEVDISR1_SEC
	str	w1, [x2, x0]
	ldr	x0, =DCFG_DEVDISR4_OFFSET
	ldr	w1, =DCFG_DEVDISR4_SPI_QSPI
	str	w1, [x2, x0]

	/* Set TPMWAKEMR0 */
	ldr	x0, =TPMWAKEMR0_ADDR
	mov	w1, #0x1
	str	w1, [x0]

	/* Disable icache, dcache, mmu @ EL1 */
	mov	x1, #SCTLR_I_C_M_MASK
	mrs	x0, sctlr_el1
	bic	x0, x0, x1
	msr	sctlr_el1, x0

	/* Disable L2 prefetches */
	mrs	x0, CPUECTLR_EL1
	orr	x0, x0, #CPUECTLR_SMPEN_EN
	bic	x0, x0, #CPUECTLR_TIMER_MASK
	orr	x0, x0, #CPUECTLR_TIMER_2TICKS
	msr	CPUECTLR_EL1, x0
	dsb	sy
	isb

	/* Disable CCI snoop domain */
	ldr	x0, =NXP_CCI_ADDR
	mov	w1, #0x1
	str	w1, [x0]

	bl	get_pmu_idle_core_mask

	/* x3 = pmu base addr */
	mov	x3, #NXP_PMU_ADDR
4:
	ldr	w1, [x3, #PMU_PCPW20SR_OFFSET]
	cmp	w1, w0
	b.ne	4b

	bl	get_pmu_idle_cluster_mask
	mov	x3, #NXP_PMU_ADDR
	str	w0, [x3, #PMU_CLAINACTSETR_OFFSET]

	bl	get_pmu_idle_core_mask
	mov	x3, #NXP_PMU_ADDR
1:
	ldr	w1, [x3, #PMU_PCPW20SR_OFFSET]
	cmp	w1, w0
	b.ne	1b

	bl	get_pmu_flush_cluster_mask
	mov	x3, #NXP_PMU_ADDR
	str	w0, [x3, #PMU_CLL2FLUSHSETR_OFFSET]
2:
	ldr	w1, [x3, #PMU_CLL2FLUSHSR_OFFSET]
	cmp	w1, w0
	b.ne	2b

	str	w0, [x3, #PMU_CLSL2FLUSHCLRR_OFFSET]

	str	w0, [x3, #PMU_CLSINACTSETR_OFFSET]

	mov	x2, #DAIF_SET_MASK
	mrs	x1, spsr_el1
	orr	x1, x1, x2
	msr	spsr_el1, x1

	mrs	x1, spsr_el2
	orr	x1, x1, x2
	msr	spsr_el2, x1

	/* Force the debug interface to be quiescent */
	mrs	x0, osdlr_el1
	orr	x0, x0, #0x1
	msr	osdlr_el1, x0

	/* Invalidate all TLB entries at all 3 exception levels */
	tlbi	alle1
	tlbi	alle2
	tlbi	alle3

	/* x3 = pmu base addr */

	/* Request lpm20 */
	ldr	x0, =PMU_POWMGTCSR_OFFSET
	ldr	w1, =PMU_POWMGTCSR_VAL
	str	w1, [x3, x0]
	isb
	dsb	sy
5:
	wfe
	b.eq	5b

/*
 * Part of CPU_SUSPEND
 *
 * This function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_core_prep_stdby:
	/* Clear CPUECTLR_EL1[2:0] */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CPUECTLR_EL1, x1

	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function puts the calling core into standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
_soc_core_entr_stdby:
	/* X0 = core mask lsb */
	dsb	sy
	isb
	wfi

	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_core_exit_stdby:
	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2
 */
_soc_core_prep_pwrdn:
	/* Make sure system counter is enabled */
	ldr	x2, =NXP_TIMER_ADDR
	ldr	w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]
	tst	w0, #SYS_COUNTER_CNTCR_EN
	b.ne	1f
	orr	w0, w0, #SYS_COUNTER_CNTCR_EN
	str	w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]
1:
	/*
	 * Enable dynamic retention control (CPUECTLR[2:0])
	 * Set the SMPEN bit (CPUECTLR[6])
	 */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_RET_MASK
	orr	x1, x1, #CPUECTLR_TIMER_2TICKS
	orr	x1, x1, #CPUECTLR_SMPEN_EN
	msr	CPUECTLR_EL1, x1

	isb
	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function puts the calling core into a power-down state
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
_soc_core_entr_pwrdn:
	/* X0 = core mask lsb */
	dsb	sy
	isb
	wfi

	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs any SoC-specific cleanup after power-down state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_core_exit_pwrdn:
	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_clstr_prep_stdby:
	/* Clear CPUECTLR_EL1[2:0] */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CPUECTLR_EL1, x1

	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_clstr_exit_stdby:
	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2
 */
_soc_clstr_prep_pwrdn:
	/* Make sure system counter is enabled */
	ldr	x2, =NXP_TIMER_ADDR
	ldr	w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]
	tst	w0, #SYS_COUNTER_CNTCR_EN
	b.ne	1f
	orr	w0, w0, #SYS_COUNTER_CNTCR_EN
	str	w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]
1:
	/*
	 * Enable dynamic retention control (CPUECTLR[2:0])
	 * Set the SMPEN bit (CPUECTLR[6])
	 */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_RET_MASK
	orr	x1, x1, #CPUECTLR_TIMER_2TICKS
	orr	x1, x1, #CPUECTLR_SMPEN_EN
	msr	CPUECTLR_EL1, x1

	isb
	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs any SoC-specific cleanup after power-down state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_clstr_exit_pwrdn:
	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_sys_prep_stdby:
	/* Clear CPUECTLR_EL1[2:0] */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CPUECTLR_EL1, x1

	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_sys_exit_stdby:
	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs SoC-specific programming prior to
 * suspend-to-power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4
 */
_soc_sys_prep_pwrdn:
	/* Set retention control */
	mrs	x0, CPUECTLR_EL1
	bic	x0, x0, #CPUECTLR_TIMER_MASK
	orr	x0, x0, #CPUECTLR_TIMER_2TICKS
	orr	x0, x0, #CPUECTLR_SMPEN_EN
	msr	CPUECTLR_EL1, x0
	dsb	sy
	isb
	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function puts the calling core, and potentially the soc, into a
 * low-power state
 * in:  x0 = core mask lsb
 * out: x0 = 0, success
 *      x0 < 0, failure
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x13, x14, x15,
 *      x16, x17, x18
 */
_soc_sys_pwrdn_wfi:
	mov	x18, x30

	mov	x3, #NXP_PMU_ADDR

	/* x3 = pmu base addr */

	/* Backup epu registers to stack */
	ldr	x2, =NXP_EPU_ADDR
	ldr	w4, [x2, #EPU_EPIMCR10_OFFSET]
	ldr	w5, [x2, #EPU_EPCCR10_OFFSET]
	ldr	w6, [x2, #EPU_EPCTR10_OFFSET]
	ldr	w7, [x2, #EPU_EPGCR_OFFSET]
	stp	x4,  x5,  [sp, #-16]!
	stp	x6,  x7,  [sp, #-16]!

	/*
	 * x2 = epu base addr
	 * x3 = pmu base addr
	 */

	/* Set up EPU event to receive the wake signal from PMU */
	mov	w4, #EPU_EPIMCR10_VAL
	mov	w5, #EPU_EPCCR10_VAL
	mov	w6, #EPU_EPCTR10_VAL
	mov	w7, #EPU_EPGCR_VAL
	str	w4, [x2, #EPU_EPIMCR10_OFFSET]
	str	w5, [x2, #EPU_EPCCR10_OFFSET]
	str	w6, [x2, #EPU_EPCTR10_OFFSET]
	str	w7, [x2, #EPU_EPGCR_OFFSET]

	ldr	x2, =NXP_GICD_ADDR

	/*
	 * x2 = gicd base addr
	 * x3 = pmu base addr
	 */

	/* Backup flextimer/mmc/usb interrupt router */
	ldr	x0, =GICD_IROUTER60_OFFSET
	ldr	x1, =GICD_IROUTER76_OFFSET
	ldr	w4, [x2, x0]
	ldr	w5, [x2, x1]
	ldr	x0, =GICD_IROUTER112_OFFSET
	ldr	x1, =GICD_IROUTER113_OFFSET
	ldr	w6, [x2, x0]
	ldr	w7, [x2, x1]
	stp	x4,  x5,  [sp, #-16]!
	stp	x6,  x7,  [sp, #-16]!

	/*
	 * x2 = gicd base addr
	 * x3 = pmu base addr
	 * x0 = GICD_IROUTER112_OFFSET
	 * x1 = GICD_IROUTER113_OFFSET
	 */

	/* Re-route interrupt to cluster 1 */
	ldr	w4, =GICD_IROUTER_VALUE
	str	w4, [x2, x0]
	str	w4, [x2, x1]
	ldr	x0, =GICD_IROUTER60_OFFSET
	ldr	x1, =GICD_IROUTER76_OFFSET
	str	w4, [x2, x0]
	str	w4, [x2, x1]
	dsb	sy
	isb

	/* x3 = pmu base addr */

	/* Disable sec, Check for eNETC, spi and qspi */
	ldr	x2, =NXP_DCFG_ADDR
	ldr	x0, =DCFG_DEVDISR1_OFFSET
	ldr	w1, =DCFG_DEVDISR1_SEC
	str	w1, [x2, x0]

	ldr	x0, =DCFG_DEVDISR4_OFFSET
	ldr	w1, =DCFG_DEVDISR4_SPI_QSPI
	str	w1, [x2, x0]

	/* x3 = pmu base addr */

	/* Set TPMWAKEMR0 */
	ldr	x0, =TPMWAKEMR0_ADDR
	mov	w1, #0x1
	str	w1, [x0]

	/* Disable CCI snoop domain */
	ldr	x0, =NXP_CCI_ADDR
	mov	w1, #0x1
	str	w1, [x0]

	/* Setup retention control */
	mrs	x0, CPUECTLR_EL1
	orr	x0, x0, #CPUECTLR_SMPEN_EN
	orr	x0, x0, #CPUECTLR_TIMER_2TICKS
	msr	CPUECTLR_EL1, x0
	dsb	sy
	isb

	bl	get_pmu_idle_core_mask
	mov	x3, #NXP_PMU_ADDR
8:
	ldr	w1, [x3, #PMU_PCPW20SR_OFFSET]
	cmp	w1, w0
	b.ne	8b

	/* x3 = NXP_PMU_ADDR */
	/* 1 cluster SoC */

	bl	get_pmu_idle_cluster_mask
	mov	x3, #NXP_PMU_ADDR

	str	w0, [x3, #PMU_CLAINACTSETR_OFFSET]

	bl	get_pmu_idle_core_mask
	/* x3 = NXP_PMU_ADDR */
	mov	x3, #NXP_PMU_ADDR
1:
	ldr	w1, [x3, #PMU_PCPW20SR_OFFSET]
	cmp	w1, w0
	b.ne	1b

	/* x3 = NXP_PMU_ADDR */
	bl	get_pmu_flush_cluster_mask
	mov	x3, #NXP_PMU_ADDR

	str	w0, [x3, #PMU_CLL2FLUSHSETR_OFFSET]

	/* x3 = NXP_PMU_ADDR */
2:
	ldr	w1, [x3, #PMU_CLL2FLUSHSR_OFFSET]
	cmp	w1, w0
	b.ne	2b

	/* x3 = NXP_PMU_ADDR */

	str	w0, [x3, #PMU_CLSL2FLUSHCLRR_OFFSET]

	str	w0, [x3, #PMU_CLSINACTSETR_OFFSET]

	/* Force the debug interface to be quiescent */
	mrs	x0, osdlr_el1
	orr	x0, x0, #0x1
	msr	osdlr_el1, x0

	/*
	 * Enable the WakeRequest signal
	 * x3 is cpu mask starting from cpu1 to cpu0
	 */
	bl	get_tot_num_cores
	sub	x0, x0, #1
	mov	x3, #0x1
	lsl	x3, x3, x0
2:
	mov	x0, x3
	bl	get_gic_rd_base  // 0-2
	ldr	w1, [x0, #GICR_WAKER_OFFSET]
	orr	w1, w1, #GICR_WAKER_SLEEP_BIT
	str	w1, [x0, #GICR_WAKER_OFFSET]
1:
	ldr	w1, [x0, #GICR_WAKER_OFFSET]
	cmp	w1, #GICR_WAKER_ASLEEP
	b.ne	1b

	lsr	x3, x3, #1
	cbnz	x3, 2b

	/* Invalidate all TLB entries at all 3 exception levels */
	tlbi	alle1
	tlbi	alle2
	tlbi	alle3

	/* Request lpm20 */
	mov	x3, #NXP_PMU_ADDR
	ldr	x0, =PMU_POWMGTCSR_OFFSET
	ldr	w1, =PMU_POWMGTCSR_VAL
	str	w1, [x3, x0]

	ldr	x5, =NXP_EPU_ADDR
4:
	wfe
	ldr	w1, [x5, #EPU_EPCTR10_OFFSET]
	cmp	w1, #0
	b.eq	4b

	/* x3 = NXP_PMU_ADDR */

	bl	get_pmu_idle_cluster_mask
	mov	x3, NXP_PMU_ADDR

	/* Re-enable the GPP ACP */
	str	w0, [x3, #PMU_CLAINACTCLRR_OFFSET]
	str	w0, [x3, #PMU_CLSINACTCLRR_OFFSET]

	/* x3 = NXP_PMU_ADDR */
3:
	ldr	w1, [x3, #PMU_CLAINACTSETR_OFFSET]
	cbnz	w1, 3b
4:
	ldr	w1, [x3, #PMU_CLSINACTSETR_OFFSET]
	cbnz	w1, 4b

	/*
	 * Enable the WakeRequest signal on cpu 0-1
	 * x3 is cpu mask starting from cpu1
	 */
	bl	get_tot_num_cores
	sub	x0, x0, #1
	mov	x3, #0x1
	lsl	x3, x3, x0
2:
	mov	x0, x3
	bl	get_gic_rd_base  // 0-2
	ldr	w1, [x0, #GICR_WAKER_OFFSET]
	bic	w1, w1, #GICR_WAKER_SLEEP_BIT
	str	w1, [x0, #GICR_WAKER_OFFSET]
1:
	ldr	w1, [x0, #GICR_WAKER_OFFSET]
	cbnz	w1, 1b

	lsr	x3, x3, #1
	cbnz	x3, 2b

	/* Enable CCI snoop domain */
	ldr	x0, =NXP_CCI_ADDR
	str	wzr, [x0]
	dsb	sy
	isb

	ldr	x3, =NXP_EPU_ADDR

	/* x3 = epu base addr */

	/* Enable sec, enetc, spi and qspi */
	ldr	x2, =NXP_DCFG_ADDR
	str	wzr, [x2, #DCFG_DEVDISR1_OFFSET]
	str	wzr, [x2, #DCFG_DEVDISR2_OFFSET]
	str	wzr, [x2, #DCFG_DEVDISR4_OFFSET]

	/* Restore flextimer/mmc/usb interrupt router */
	ldr	x3, =NXP_GICD_ADDR
	ldp	x0, x2, [sp], #16
	ldr	x1, =GICD_IROUTER113_OFFSET
	str	w2, [x3, x1]
	ldr	x1, =GICD_IROUTER112_OFFSET
	str	w0, [x3, x1]
	ldp	x0, x2, [sp], #16
	ldr	x1, =GICD_IROUTER76_OFFSET
	str	w2, [x3, x1]
	ldr	x1, =GICD_IROUTER60_OFFSET
	str	w0, [x3, x1]

	/* Restore EPU registers */
	ldr	x3, =NXP_EPU_ADDR
	ldp	x0, x2, [sp], #16
	str	w2, [x3, #EPU_EPGCR_OFFSET]
	str	w0, [x3, #EPU_EPCTR10_OFFSET]
	ldp	x2, x1, [sp], #16
	str	w1, [x3, #EPU_EPCCR10_OFFSET]
	str	w2, [x3, #EPU_EPIMCR10_OFFSET]

	dsb	sy
	isb
	mov	x30, x18
	ret

/*
 * Part of CPU_SUSPEND
 *
 * This function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_sys_exit_pwrdn:
	/* Enable stack alignment checking */
	mrs	x1, SCTLR_EL1
	orr	x1, x1, #0x4
	msr	SCTLR_EL1, x1

	/* Enable debug interface */
	mrs	x1, osdlr_el1
	bic	x1, x1, #OSDLR_EL1_DLK_LOCK
	msr	osdlr_el1, x1

	/* Enable i-cache */
	mrs	x1, SCTLR_EL3
	orr	x1, x1, #SCTLR_I_MASK
	msr	SCTLR_EL3, x1

	isb
	ret

/*
 * This function setc up the TrustZone Address Space Controller (TZASC)
 * in:  none
 * out: none
 * uses x0, x1
 */
init_tzpc:
	/* Set Non Secure access for all devices protected via TZPC */
	ldr	x1, =TZPCDECPROT_0_SET_BASE   /* decode Protection-0 Set Reg */
	mov	w0, #0xFF		      /* set decode region to NS, Bits[7:0] */
	str	w0, [x1]

	ldr	x1, =TZPCDECPROT_1_SET_BASE   /* decode Protection-1 Set Reg */
	mov	w0, #0xFF		      /* set decode region to NS, Bits[7:0] */
	str	w0, [x1]

	ldr	x1, =TZPCDECPROT_2_SET_BASE   /* decode Protection-2 Set Reg */
	mov	w0, #0xFF		      /* set decode region to NS, Bits[7:0] */
	str	w0, [x1]

	 /* entire SRAM as NS */
	ldr	x1, =NXP_OCRAM_TZPC_ADDR      /* secure RAM region size Reg */
	mov	w0, #0x00000000		      /* 0x00000000 = no secure region */
	str	w0, [x1]

	ret

/*
 * This function performs any needed initialization on SecMon for
 * boot services
 */
initSecMon:
	/* Read the register hpcomr */
	ldr	x1, =NXP_SNVS_ADDR
	ldr	w0, [x1, #SECMON_HPCOMR_OFFSET]
	/* Turn off secure access for the privileged registers */
	orr	w0, w0, #SECMON_HPCOMR_NPSWAEN
	/* Write back */
	str	w0, [x1, #SECMON_HPCOMR_OFFSET]

	ret

/*
 * This function checks to see if cores which are to be disabled have been
 * released from reset - if not, it releases them
 * in:  none
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8
 */
release_disabled:
	stp	x18, x30, [sp, #-16]!

	/*
	 * Get the number of cpus on this device
	 * Calling the below c function.
	 * No need to Callee saved registers x9-x15,
	 * as these registers are not used by the callee
	 * prior to calling the below C-routine.
	 */
	bl	get_tot_num_cores
	mov	x6, x0

	/* Read COREDISABLESR */
	mov	x0, #NXP_DCFG_ADDR
	ldr	w4, [x0, #DCFG_COREDISABLEDSR_OFFSET]

	mov	x0, #NXP_RESET_ADDR
	ldr	w5, [x0, #BRR_OFFSET]

	/* Load the core mask for the first core */
	mov	x7, #1

	/*
	 * x4 = COREDISABLESR
	 * x5 = BRR
	 * x6 = loop count
	 * x7 = core mask bit
	 */
2:
	/* Check if the core is to be disabled */
	tst	x4, x7
	b.eq	1f

	/* See if disabled cores have already been released from reset */
	tst	x5, x7
	b.ne	1f

	/* If core has not been released, then release it (0-3) */
	mov	x0, x7
	bl	_soc_core_release

	/* Record the core state in the data area (0-3) */
	mov	x0, x7
	mov	x1, #CORE_DISABLED
	bl	_setCoreState
1:
	/* Decrement the counter */
	subs	x6, x6, #1
	b.le	3f
	/* Shift the core mask to the next core */
	lsl	x7, x7, #1
	/* Continue */
	b	2b
3:
	ldp	x18, x30, [sp], #16
	ret

/*
 * Write a register in the DCFG block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2
 */
_write_reg_dcfg:
	ldr	x2, =NXP_DCFG_ADDR
	str	w1, [x2, x0]
	ret

/*
 * Read a register in the DCFG block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
_read_reg_dcfg:
	ldr	x2, =NXP_DCFG_ADDR
	ldr	w1, [x2, x0]
	mov	w0, w1
	ret

/*
 * This function returns an mpidr value for a core, given a core_mask_lsb
 * in:  x0 = core mask lsb
 * out: x0 = affinity2:affinity1:affinity0, where affinity is 8-bits
 * uses x0, x1
 */
get_mpidr_value:
	/* Convert a core mask to an SoC core number */
	clz	w0, w0
	mov	w1, #31
	sub	w0, w1, w0

	/* Get the mpidr core number from the SoC core number */
	mov	w1, wzr
	tst	x0, #1
	b.eq	1f
	orr	w1, w1, #1
1:
	/* Extract the cluster number */
	lsr	w0, w0, #1
	orr	w0, w1, w0, lsl #8

	ret

/*
 * This function returns the redistributor base address for the core specified
 * in x1
 * in:  x0 - core mask lsb of specified core
 * out: x0 = redistributor rd base address for specified core
 * uses x0, x1, x2
 */
get_gic_rd_base:
	/* Get the 0-based core number */
	clz	w1, w0
	mov	w2, #0x20
	sub	w2, w2, w1
	sub	w2, w2, #1

	/* x2 = core number / loop counter */
	ldr	x0, =NXP_GICR_ADDR
	mov	x1, #GIC_RD_OFFSET
2:
	cbz	x2, 1f
	add	x0, x0, x1
	sub	x2, x2, #1
	b	2b
1:
	ret

/*
 * This function returns the redistributor base address for the core specified
 * in x1
 * in:  x0 - core mask lsb of specified core
 * out: x0 = redistributor sgi base address for specified core
 * uses x0, x1, x2
 */
get_gic_sgi_base:
	/* Get the 0-based core number */
	clz	w1, w0
	mov	w2, #0x20
	sub	w2, w2, w1
	sub	w2, w2, #1

	/* x2 = core number / loop counter */
	ldr	x0, =NXP_GICR_SGI_ADDR
	mov	x1, #GIC_SGI_OFFSET
2:
	cbz	x2, 1f
	add	x0, x0, x1
	sub	x2, x2, #1
	b	2b
1:
	ret

/*
 * Write a register in the RESET block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2
 */
_write_reg_reset:
	ldr	x2, =NXP_RESET_ADDR
	str	w1, [x2, x0]
	ret

/*
 * Read a register in the RESET block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1
 */
_read_reg_reset:
	ldr	x1, =NXP_RESET_ADDR
	ldr	w0, [x1, x0]
	ret
