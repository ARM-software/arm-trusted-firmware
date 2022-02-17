/*
 * Copyright 2020-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <asm_macros.S>
#include <dcfg_lsch2.h>
#include <nxp_timer.h>
#include <plat_gic.h>
#include <scfg.h>

#include <bl31_data.h>
#include <plat_psci.h>
#include <platform_def.h>

#define DAIF_DATA		AUX_01_DATA
#define TIMER_CNTRL_DATA	AUX_02_DATA

.global soc_init_lowlevel
.global soc_init_percpu
.global _soc_core_release
.global _soc_core_restart
.global _soc_ck_disabled
.global _soc_sys_reset
.global _soc_sys_off
.global _soc_set_start_addr
.global _getGICC_BaseAddr
.global _getGICD_BaseAddr
.global _soc_core_prep_off
.global _soc_core_entr_off
.global _soc_core_exit_off
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


/* This function initialize the soc
 * in: void
 * out: void
 */
func soc_init_lowlevel
    ret
endfunc soc_init_lowlevel


/* void soc_init_percpu(void)
 * this function performs any soc-specific initialization that is needed on
 * a per-core basis
 * in:  none
 * out: none
 * uses x0, x1, x2, x3
 */
func soc_init_percpu
	mov	x3, x30

	bl	plat_my_core_mask
	mov	x2, x0

	/* see if this core is marked for prefetch disable */
	mov	x0, #PREFETCH_DIS_OFFSET
	bl	_get_global_data  /* 0-1 */
	tst	x0, x2
	b.eq	1f
	bl	_disable_ldstr_pfetch_A72  /* 0 */
1:
	mov	x30, x3
	ret
endfunc soc_init_percpu

/* part of CPU_ON
 * this function releases a secondary core from reset
 * in:   x0 = core_mask_lsb
 * out:  none
 * uses: x0, x1, x2, x3
 */
func _soc_core_release

#if (TEST_BL31)
	rbit	w2, w0
	/* x2 = core mask msb */
#else
	mov	x2, x0
#endif
	/* write COREBCR */
	mov	x1, #NXP_SCFG_ADDR
	rev	w3, w2
	str	w3, [x1, #SCFG_COREBCR_OFFSET]
	isb

	/* read-modify-write BRR */
	mov	x1, #NXP_DCFG_ADDR
	ldr	w2, [x1, #DCFG_BRR_OFFSET]
	rev	w3, w2
	orr	w3, w3, w0
	rev	w2, w3
	str	w2, [x1, #DCFG_BRR_OFFSET]
	isb

	/* send event */
	sev
	isb
	ret
endfunc _soc_core_release


/* part of CPU_ON
 * this function restarts a core shutdown via _soc_core_entr_off
 * in:  x0 = core mask lsb (of the target cpu)
 * out: x0 == 0, on success
 *      x0 != 0, on failure
 * uses x0, x1, x2, x3, x4, x5
 */
func _soc_core_restart
	mov	x5, x30
	mov	x3, x0

	/*
	 * unset ph20 request in RCPM_PCPH20CLEARR
	 * this is an lsb-0 register
	 */
	ldr	x1, =NXP_RCPM_ADDR
	rev	w2, w3
	str	w2, [x1, #RCPM_PCPH20CLRR_OFFSET]
	dsb	sy
	isb

	bl	_getGICD_BaseAddr
	mov	x4, x0

	/* enable forwarding of group 0 interrupts by setting GICD_CTLR[0] = 1 */
	ldr	w1, [x4, #GICD_CTLR_OFFSET]
	orr	w1, w1, #GICD_CTLR_EN_GRP0
	str	w1, [x4, #GICD_CTLR_OFFSET]
	dsb	sy
	isb


	/*
	 * fire SGI by writing to GICD_SGIR the following values:
	 * [25:24] = 0x0 (forward interrupt to the CPU interfaces
	 *           specified in CPUTargetList field)
	 * [23:16] = core mask lsb[7:0] (forward interrupt to target cpu)
	 * [15]    = 0 (forward SGI only if it is configured as group 0 interrupt)
	 * [3:0]   = 0xF (interrupt ID = 15)
	 */
	lsl	w1, w3, #16
	orr	w1, w1, #0xF
	str	w1, [x4, #GICD_SGIR_OFFSET]
	dsb	sy
	isb

	/* load '0' on success */
	mov	x0, xzr

	mov	x30, x5
	ret
endfunc _soc_core_restart

/*
 * This function determines if a core is disabled via COREDISR
 * in:  w0  = core_mask_lsb
 * out: w0  = 0, core not disabled
 *      w0 != 0, core disabled
 * uses x0, x1, x2
 */
func _soc_ck_disabled
	/* get base addr of dcfg block */
	mov	x1, #NXP_DCFG_ADDR

	/* read COREDISR */
	ldr	w1, [x1, #DCFG_COREDISR_OFFSET]
	rev	w2, w1

	/* test core bit */
	and	w0, w2, w0
	ret
endfunc _soc_ck_disabled

/*
 *This function resets the system via SoC-specific methods
 * in:  none
 * out: none
 * uses x0, x1, x2, x3
 */
func _soc_sys_reset
	ldr	x2, =NXP_DCFG_ADDR

	/* make sure the mask is cleared in the reset request mask register */
	mov	w1, wzr
	str	w1, [x2, #DCFG_RSTRQMR1_OFFSET]

	/* set the reset request */
	ldr	w1, =RSTCR_RESET_REQ
	ldr	x3, =DCFG_RSTCR_OFFSET
	rev	w0, w1
	str	w0, [x2, x3]

	/*
	 * just in case this address range is mapped as cacheable,
	 * flush the write out of the dcaches
	 */
	add	x3, x2, x3
	dc	cvac, x3
	dsb	st
	isb

	/* Note: this function does not return */
1:
	wfi
	b	1b
endfunc _soc_sys_reset

/*
 * Part of SYSTEM_OFF
 * this function turns off the SoC clocks
 * Note: this function is not intended to return, and the only allowable
 *       recovery is POR
 * in:  none
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9
 */
func _soc_sys_off

	/* mask interrupts at the core */
	mrs	x1, DAIF
	mov	x0, #DAIF_SET_MASK
	orr	x0, x1, x0
	msr	DAIF, x0

	/* disable icache, dcache, mmu @ EL1 */
	mov	x1, #SCTLR_I_C_M_MASK
	mrs	x0, sctlr_el1
	bic	x0, x0, x1
	msr	sctlr_el1, x0

	/* disable dcache for EL3 */
	mrs	x1, SCTLR_EL3
	bic	x1, x1, #SCTLR_C_MASK
	/* make sure icache is enabled */
	orr	x1, x1, #SCTLR_I_MASK
	msr	SCTLR_EL3, x1
	isb

	/* Enable dynamic retention ctrl (CPUECTLR[2:0]) and SMP (CPUECTLR[6]) */
	mrs	x0, CORTEX_A72_ECTLR_EL1
	orr	x0, x0, #CPUECTLR_TIMER_8TICKS
	orr	x0, x0, #CPUECTLR_SMPEN_EN
	msr	CORTEX_A72_ECTLR_EL1, x0

	/* set WFIL2EN in SCFG_CLUSTERPMCR */
	ldr	x0, =SCFG_COREPMCR_OFFSET
	ldr	x1, =COREPMCR_WFIL2
	bl	write_reg_scfg

	/* request LPM20 */
	mov	x0, #RCPM_POWMGTCSR_OFFSET
	bl	read_reg_rcpm
	orr	x1, x0, #RCPM_POWMGTCSR_LPM20_REQ
	mov	x0, #RCPM_POWMGTCSR_OFFSET
	bl	write_reg_rcpm

	dsb  sy
	isb
1:
	wfi
	b	1b
endfunc _soc_sys_off

/*
 * Write a register in the RCPM block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
func write_reg_rcpm
	ldr	x2, =NXP_RCPM_ADDR
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
	ret
endfunc write_reg_rcpm

/*
 * Read a register in the RCPM block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
func read_reg_rcpm
	ldr	x2, =NXP_RCPM_ADDR
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret
endfunc read_reg_rcpm

/*
 * Write a register in the SCFG block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
func write_reg_scfg
	mov	x2, #NXP_SCFG_ADDR
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
	ret
endfunc write_reg_scfg

/*
 * Read a register in the SCFG block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
func read_reg_scfg
	mov	x2, #NXP_SCFG_ADDR
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret
endfunc read_reg_scfg

/*
 * Part of CPU_OFF
 * this function programs SoC & GIC registers in preparation for shutting down
 * the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7
 */
func _soc_core_prep_off
	mov	x7, x30
	mov	x6, x0

	/* Set retention control in CPUECTLR make sure smpen bit is set */
	mrs	x4, CORTEX_A72_ECTLR_EL1
	bic	x4, x4, #CPUECTLR_RET_MASK
	orr	x4, x4, #CPUECTLR_TIMER_8TICKS
	orr	x4, x4, #CPUECTLR_SMPEN_EN
	msr	CORTEX_A72_ECTLR_EL1, x4

	/* save timer control current value */
	mov	x5, #NXP_TIMER_ADDR
	ldr	w4, [x5, #SYS_COUNTER_CNTCR_OFFSET]
	mov	w2, w4
	mov	x0, x6
	mov	x1, #TIMER_CNTRL_DATA
	bl	_setCoreData

	/* enable the timer */
	orr	w4, w4, #CNTCR_EN_MASK
	str	w4, [x5, #SYS_COUNTER_CNTCR_OFFSET]

	bl	_getGICC_BaseAddr
	mov	x5, x0

	/* disable signaling of ints */
	ldr	w3, [x5, #GICC_CTLR_OFFSET]
	bic	w3, w3, #GICC_CTLR_EN_GRP0
	bic	w3, w3, #GICC_CTLR_EN_GRP1
	str	w3, [x5, #GICC_CTLR_OFFSET]
	dsb	sy
	isb


	/*
	 * set retention control in SCFG_RETREQCR
	 * Note: this register is msb 0
	 */
	ldr	x4, =SCFG_RETREQCR_OFFSET
	mov	x0, x4
	bl	read_reg_scfg
	rbit	w1, w6
	orr	w1, w0, w1
	mov	x0, x4
	bl	write_reg_scfg

	/* set the priority filter */
	ldr	w2, [x5, #GICC_PMR_OFFSET]
	orr	w2, w2, #GICC_PMR_FILTER
	str	w2, [x5, #GICC_PMR_OFFSET]

	/* setup GICC_CTLR */
	bic	w3, w3, #GICC_CTLR_ACKCTL_MASK
	orr	w3, w3, #GICC_CTLR_FIQ_EN_MASK
	orr	w3, w3, #GICC_CTLR_EOImodeS_MASK
	orr	w3, w3, #GICC_CTLR_CBPR_MASK
	str	w3, [x5, #GICC_CTLR_OFFSET]

	/* setup the banked-per-core GICD registers */
	bl	_getGICD_BaseAddr
	mov	x5, x0

	/* define SGI15 as Grp0 */
	ldr	w2, [x5, #GICD_IGROUPR0_OFFSET]
	bic	w2, w2, #GICD_IGROUP0_SGI15
	str	w2, [x5, #GICD_IGROUPR0_OFFSET]

	/* set priority of SGI 15 to highest... */
	ldr	w2, [x5, #GICD_IPRIORITYR3_OFFSET]
	bic	w2, w2, #GICD_IPRIORITY_SGI15_MASK
	str	w2, [x5, #GICD_IPRIORITYR3_OFFSET]

	/* enable SGI 15 */
	ldr	w2, [x5, #GICD_ISENABLER0_OFFSET]
	orr	w2, w2, #GICD_ISENABLE0_SGI15
	str	w2, [x5, #GICD_ISENABLER0_OFFSET]

	/* enable the cpu interface */
	bl	_getGICC_BaseAddr
	mov	x2, x0
	orr	w3, w3, #GICC_CTLR_EN_GRP0
	str	w3, [x2, #GICC_CTLR_OFFSET]


	/* clear any pending SGIs */
	ldr	x2, =GICD_CPENDSGIR_CLR_MASK
	add	x0, x5, #GICD_CPENDSGIR3_OFFSET
	str	w2, [x0]

	/*
	 * Set the PC_PH20_REQ bit in RCPM_PCPH20SETR
	 * this is an lsb-0 register
	 */
	mov	x1, x6
	mov	x0, #RCPM_PCPH20SETR_OFFSET
	bl	write_reg_rcpm

	dsb	sy
	isb
	mov	x30, x7
	ret
endfunc _soc_core_prep_off

/*
 * Part of CPU_OFF
 * this function performs the final steps to shutdown the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5
 */
func _soc_core_entr_off
	mov	x5, x30
	mov	x4, x0

	bl	_getGICD_BaseAddr
	mov	x3, x0

3:
	/* enter low-power state by executing wfi */
	wfi

	/* see if we got hit by SGI 15 */
	add	x0, x3, #GICD_SPENDSGIR3_OFFSET
	ldr	w2, [x0]
	and	w2, w2, #GICD_SPENDSGIR3_SGI15_MASK
	cbz	w2, 4f

	/* clear the pending SGI */
	ldr	x2, =GICD_CPENDSGIR_CLR_MASK
	add	x0, x3, #GICD_CPENDSGIR3_OFFSET
	str	w2, [x0]
4:
	/* check if core has been turned on */
	mov	x0, x4
	bl	_getCoreState

	cmp	x0, #CORE_WAKEUP
	b.ne	3b

	/* if we get here, then we have exited the wfi */
	dsb	sy
	isb
	mov	x30, x5
	ret
endfunc _soc_core_entr_off

/*
 * Part of CPU_OFF
 * this function starts the process of starting a core back up
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6
 */
func _soc_core_exit_off
	mov	x6, x30
	mov	x5, x0

	/*
	 * Clear ph20 request in RCPM_PCPH20CLRR - no need
	 * to do that here, it has been done in _soc_core_restart
	 */
	bl	_getGICC_BaseAddr
	mov	x1, x0

	/* read GICC_IAR */
	ldr	w0, [x1, #GICC_IAR_OFFSET]

	/* write GICC_EIOR - signal end-of-interrupt */
	str	w0, [x1, #GICC_EOIR_OFFSET]

	/* write GICC_DIR - disable interrupt */
	str	w0, [x1, #GICC_DIR_OFFSET]

	/* disable signaling of grp0 ints */
	ldr	w3, [x1, #GICC_CTLR_OFFSET]
	bic	w3, w3, #GICC_CTLR_EN_GRP0
	str	w3, [x1, #GICC_CTLR_OFFSET]

	/*
	 * Unset retention request in SCFG_RETREQCR
	 * Note: this register is msb-0
	 */
	ldr	x4, =SCFG_RETREQCR_OFFSET
	mov	x0, x4
	bl	read_reg_scfg
	rbit	w1, w5
	bic	w1, w0, w1
	mov	x0, x4
	bl	write_reg_scfg

	/* restore timer ctrl */
	mov	x0, x5
	mov	x1, #TIMER_CNTRL_DATA
	bl	_getCoreData
	/* w0 = timer ctrl saved value */
	mov	x2, #NXP_TIMER_ADDR
	str	w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]

	dsb	sy
	isb
	mov	x30, x6
	ret
endfunc _soc_core_exit_off

/*
 * Function loads a 64-bit execution address of the core in the soc registers
 * BOOTLOCPTRL/H
 * in:  x0, 64-bit address to write to BOOTLOCPTRL/H
 * uses x0, x1, x2, x3
 */
func _soc_set_start_addr
	/* get the 64-bit base address of the scfg block */
	ldr	x2, =NXP_SCFG_ADDR

	/* write the 32-bit BOOTLOCPTRL register */
	mov	x1, x0
	rev	w3, w1
	str	w3, [x2, #SCFG_BOOTLOCPTRL_OFFSET]

	/* write the 32-bit BOOTLOCPTRH register */
	lsr	x1, x0, #32
	rev	w3, w1
	str	w3, [x2, #SCFG_BOOTLOCPTRH_OFFSET]
	ret
endfunc _soc_set_start_addr

/*
 * This function returns the base address of the gic distributor
 * in:  none
 * out: x0 = base address of gic distributor
 * uses x0
 */
func _getGICD_BaseAddr
#if (TEST_BL31)
	/* defect in simulator - gic base addresses are on 4Kb boundary */
	ldr	x0, =NXP_GICD_4K_ADDR
#else
	ldr	x0, =NXP_GICD_64K_ADDR
#endif
	ret
endfunc _getGICD_BaseAddr

/*
 * This function returns the base address of the gic controller
 * in:  none
 * out: x0 = base address of gic controller
 * uses x0
 */
func _getGICC_BaseAddr
#if (TEST_BL31)
	/* defect in simulator - gic base addresses are on 4Kb boundary */
	ldr	x0, =NXP_GICC_4K_ADDR
#else
	ldr	x0, =NXP_GICC_64K_ADDR
#endif
	ret
endfunc _getGICC_BaseAddr

/*
 * Part of CPU_SUSPEND
 * this function puts the calling core into standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
func _soc_core_entr_stdby
	dsb	sy
	isb
	wfi

	ret
endfunc _soc_core_entr_stdby


/*
 * Part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
func _soc_core_prep_stdby
	/* clear CORTEX_A72_ECTLR_EL1[2:0] */
	mrs	x1, CORTEX_A72_ECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CORTEX_A72_ECTLR_EL1, x1

	ret
endfunc _soc_core_prep_stdby

/*
 * Part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_core_exit_stdby
	ret
endfunc _soc_core_exit_stdby

/*
 * Part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5
 */
func _soc_core_prep_pwrdn
	mov	x5, x30
	mov	x4, x0

	/* enable CPU retention + set smp */
	mrs	x1, CORTEX_A72_ECTLR_EL1
	orr	x1, x1, #0x1
	orr	x1, x1, #CPUECTLR_SMPEN_MASK
	msr	CORTEX_A72_ECTLR_EL1, x1

	/*
	 * set the retention request in SCFG_RETREQCR
	 * this is an msb-0 register
	 */
	ldr	x3, =SCFG_RETREQCR_OFFSET
	mov	x0, x3
	bl	read_reg_scfg
	rbit	w1, w4
	orr	w1, w0, w1
	mov	x0, x3
	bl	write_reg_scfg

	/*
	 * Set the PC_PH20_REQ bit in RCPM_PCPH20SETR
	 * this is an lsb-0 register
	 */
	mov	x1, x4
	mov	x0, #RCPM_PCPH20SETR_OFFSET
	bl	write_reg_rcpm

	mov	x30, x5
	ret
endfunc _soc_core_prep_pwrdn

/*
 * Part of CPU_SUSPEND
 * this function puts the calling core into a power-down state
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
func _soc_core_entr_pwrdn
	dsb	sy
	isb
	wfi

	ret
endfunc _soc_core_entr_pwrdn

/*
 * Part of CPU_SUSPEND
 * this function cleans up after a core exits power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5
 */
func _soc_core_exit_pwrdn
	mov	x5, x30
	mov	x4, x0

	/*
	 * Set the PC_PH20_REQ bit in RCPM_PCPH20CLRR
	 * this is an lsb-0 register
	 */
	mov	x1, x4
	mov	x0, #RCPM_PCPH20CLRR_OFFSET
	bl	write_reg_rcpm

	/*
	 * Unset the retention request in SCFG_RETREQCR
	 * this is an msb-0 register
	 */
	ldr	x3, =SCFG_RETREQCR_OFFSET
	mov	x0, x3
	bl	read_reg_scfg
	rbit	w1, w4
	bic	w1, w0, w1
	mov	x0, x3
	bl	write_reg_scfg

	mov	x30, x5
	ret
endfunc _soc_core_exit_pwrdn

/*
 * Part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_clstr_prep_stdby
	/* clear CORTEX_A72_ECTLR_EL1[2:0] */
	mrs	x1, CORTEX_A72_ECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CORTEX_A72_ECTLR_EL1, x1

	ret
endfunc _soc_clstr_prep_stdby

/*
 * Part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_clstr_exit_stdby
	ret
endfunc _soc_clstr_exit_stdby

/*
 * Part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5
 */
func _soc_clstr_prep_pwrdn
	mov	x5, x30
	mov	x4, x0

	/* enable CPU retention + set smp */
	mrs	x1, CORTEX_A72_ECTLR_EL1
	orr	x1, x1, #0x1
	orr	x1, x1, #CPUECTLR_SMPEN_MASK
	msr	CORTEX_A72_ECTLR_EL1, x1

	/*
	 * Set the retention request in SCFG_RETREQCR
	 * this is an msb-0 register.
	 */
	ldr	x3, =SCFG_RETREQCR_OFFSET
	mov	x0, x3
	bl	read_reg_scfg
	rbit	w1, w4
	orr	w1, w0, w1
	mov	x0, x3
	bl	write_reg_scfg

	/*
	 * Set the PC_PH20_REQ bit in RCPM_PCPH20SETR
	 * this is an lsb-0 register.
	 */
	mov	x1, x4
	mov	x0, #RCPM_PCPH20SETR_OFFSET
	bl	write_reg_rcpm

	mov	x30, x5
	ret
endfunc _soc_clstr_prep_pwrdn

/*
 * Part of CPU_SUSPEND
 * this function cleans up after a core exits power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5
 */
func _soc_clstr_exit_pwrdn
	mov	x5, x30
	mov	x4, x0

	/*
	 * Set the PC_PH20_REQ bit in RCPM_PCPH20CLRR
	 * this is an lsb-0 register.
	 */
	mov	x1, x4
	mov	x0, #RCPM_PCPH20CLRR_OFFSET
	bl	write_reg_rcpm

	/*
	 * Unset the retention request in SCFG_RETREQCR
	 * this is an msb-0 register.
	 */
	ldr	x3, =SCFG_RETREQCR_OFFSET
	mov	x0, x3
	bl	read_reg_scfg
	rbit	w1, w4
	bic	w1, w0, w1
	mov	x0, x3
	bl	write_reg_scfg

	mov	x30, x5
	ret
endfunc _soc_clstr_exit_pwrdn

/*
 * Part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_sys_prep_stdby
	/* clear CORTEX_A72_ECTLR_EL1[2:0] */
	mrs	x1, CORTEX_A72_ECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CORTEX_A72_ECTLR_EL1, x1

	ret
endfunc _soc_sys_prep_stdby

/* Part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_sys_exit_stdby
	ret
endfunc _soc_sys_exit_stdby

/*
 * Part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to
 * suspend-to-power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4
 */
func _soc_sys_prep_pwrdn
	mov	x4, x30

	/* Enable dynamic retention contrl (CPUECTLR[2:0]) and SMP (CPUECTLR[6]) */
	mrs	x0, CORTEX_A72_ECTLR_EL1
	bic	x0, x0, #CPUECTLR_TIMER_MASK
	orr	x0, x0, #CPUECTLR_TIMER_8TICKS
	orr	x0, x0, #CPUECTLR_SMPEN_EN
	msr	CORTEX_A72_ECTLR_EL1, x0

	/* Set WFIL2EN in SCFG_CLUSTERPMCR */
	ldr	x0, =SCFG_COREPMCR_OFFSET
	ldr	x1, =COREPMCR_WFIL2
	bl	write_reg_scfg

	isb
	mov	x30, x4
	ret
endfunc _soc_sys_prep_pwrdn

/*
 * Part of CPU_SUSPEND
 * this function puts the calling core, and potentially the soc, into a
 * low-power state
 * in:  x0 = core mask lsb
 * out: x0 = 0, success
 *      x0 < 0, failure
 * uses x0, x1, x2, x3, x4
 */
func _soc_sys_pwrdn_wfi
	mov	x4, x30

	/* request LPM20 */
	mov	x0, #RCPM_POWMGTCSR_OFFSET
	bl	read_reg_rcpm
	orr	x1, x0, #RCPM_POWMGTCSR_LPM20_REQ
	mov	x0, #RCPM_POWMGTCSR_OFFSET
	bl	write_reg_rcpm

	dsb	sy
	isb
	wfi

	mov	x30, x4
	ret
endfunc _soc_sys_pwrdn_wfi

/*
 * Part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
func _soc_sys_exit_pwrdn
	/* clear WFIL2_EN in SCFG_COREPMCR */
	mov	x1, #NXP_SCFG_ADDR
	str	wzr, [x1, #SCFG_COREPMCR_OFFSET]

	ret
endfunc _soc_sys_exit_pwrdn
