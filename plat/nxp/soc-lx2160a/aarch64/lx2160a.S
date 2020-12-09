/*
 * Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
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

.global soc_init_start
.global soc_init_percpu
.global soc_init_finish
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

.equ TZPC_BASE,			  0x02200000
.equ TZPCDECPROT_0_SET_BASE, 0x02200804
.equ TZPCDECPROT_1_SET_BASE, 0x02200810
.equ TZPCDECPROT_2_SET_BASE, 0x0220081C

#define CLUSTER_3_CORES_MASK 0xC0
#define CLUSTER_3_IN_RESET  1
#define CLUSTER_3_NORMAL	0

/* cluster 3 handling no longer based on frequency, but rather on RCW[850],
 * which is bit 18 of RCWSR27
 */
#define CLUSTER_3_RCW_BIT  0x40000

/* retry count for clock-stop acks */
.equ CLOCK_RETRY_CNT,  800

/* disable prefetching in the A72 core */
#define  CPUACTLR_DIS_LS_HW_PRE	0x100000000000000
#define  CPUACTLR_DIS_L2_TLB_PRE   0x200000

/* Function starts the initialization tasks of the soc,
 * using secondary cores if they are available
 *
 * Called from C, saving the non-volatile regs
 * save these as pairs of registers to maintain the
 * required 16-byte alignment on the stack
 *
 * in:
 * out:
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11
 */
func soc_init_start
	stp  x4,  x5,  [sp, #-16]!
	stp  x6,  x7,  [sp, #-16]!
	stp  x8,  x9,  [sp, #-16]!
	stp  x10, x11, [sp, #-16]!
	stp  x12, x13, [sp, #-16]!
	stp  x18, x30, [sp, #-16]!

	/* make sure the personality has been
	 * established by releasing cores that
	 * are marked "to-be-disabled" from reset
	 */
	bl  release_disabled  		/* 0-9 */

	/* init the task flags */
	bl  _init_task_flags   		/* 0-1 */

	/* set SCRATCHRW7 to 0x0 */
	ldr  x0, =DCFG_SCRATCHRW7_OFFSET
	mov  x1, xzr
	bl   _write_reg_dcfg

1:
	/* restore the aarch32/64 non-volatile registers */
	ldp  x18, x30, [sp], #16
	ldp  x12, x13, [sp], #16
	ldp  x10, x11, [sp], #16
	ldp  x8,  x9,  [sp], #16
	ldp  x6,  x7,  [sp], #16
	ldp  x4,  x5,  [sp], #16
	ret
endfunc soc_init_start


/* Function performs any soc-specific initialization that is needed on
 * a per-core basis.
 * in:  none
 * out: none
 * uses x0, x1, x2, x3
 */
func soc_init_percpu
	stp  x4,  x30,  [sp, #-16]!

	bl   plat_my_core_mask
	mov  x2, x0				/* x2 = core mask */

	/* Check if this core is marked for prefetch disable
	 */
	mov   x0, #PREFETCH_DIS_OFFSET
	bl	_get_global_data		/* 0-1 */
	tst   x0, x2
	b.eq  1f
	bl	_disable_ldstr_pfetch_A72	/* 0 */
1:
	mov  x0, #NXP_PMU_ADDR
	bl enable_timer_base_to_cluster
	ldp  x4,  x30,  [sp], #16
	ret
endfunc soc_init_percpu


/* Function completes the initialization tasks of the soc
 * in:
 * out:
 * uses x0, x1, x2, x3, x4
 */
func soc_init_finish
	stp  x4,  x30,  [sp, #-16]!

	ldp   x4,  x30,  [sp], #16
	ret
endfunc soc_init_finish


/* Function sets the security mechanisms in the SoC to implement the
 * Platform Security Policy
 */
func _set_platform_security
	mov  x8, x30

#if (!SUPPRESS_TZC)
	/* initialize the tzpc */
	bl   init_tzpc
#endif

#if (!SUPPRESS_SEC)
	/* initialize secmon */
#ifdef NXP_SNVS_ENABLED
	mov x0, #NXP_SNVS_ADDR
	bl  init_sec_mon
#endif
#endif

	mov  x30, x8
	ret
endfunc _set_platform_security


/* Function writes a 64-bit address to bootlocptrh/l
 * in:  x0, 64-bit address to write to BOOTLOCPTRL/H
 * uses x0, x1, x2
 */
func _soc_set_start_addr
	/* Get the 64-bit base address of the dcfg block */
	ldr  x2, =NXP_DCFG_ADDR

	/* write the 32-bit BOOTLOCPTRL register */
	mov  x1, x0
	str  w1, [x2, #DCFG_BOOTLOCPTRL_OFFSET]

	/* write the 32-bit BOOTLOCPTRH register */
	lsr  x1, x0, #32
	str  w1, [x2, #DCFG_BOOTLOCPTRH_OFFSET]
	ret
endfunc _soc_set_start_addr

/* Function releases a secondary core from reset
 * in:   x0 = core_mask_lsb
 * out:  none
 * uses: x0, x1, x2, x3
 */
func _soc_core_release
	mov   x3, x30

	ldr  x1, =NXP_SEC_REGFILE_ADDR
	/* write to CORE_HOLD to tell
	 * the bootrom that this core is
	 * expected to run.
	 */
	str  w0, [x1, #CORE_HOLD_OFFSET]

	/* read-modify-write BRRL to release core */
	mov  x1, #NXP_RESET_ADDR
	ldr  w2, [x1, #BRR_OFFSET]

	/* x0 = core mask */
	orr  w2, w2, w0
	str  w2, [x1, #BRR_OFFSET]
	dsb  sy
	isb

	/* send event */
	sev
	isb

	mov   x30, x3
	ret
endfunc _soc_core_release


/* Function determines if a core is disabled via COREDISABLEDSR
 * in:  w0  = core_mask_lsb
 * out: w0  = 0, core not disabled
 *	  w0 != 0, core disabled
 * uses x0, x1
 */
func _soc_ck_disabled

	/* get base addr of dcfg block */
	ldr  x1, =NXP_DCFG_ADDR

	/* read COREDISABLEDSR */
	ldr  w1, [x1, #DCFG_COREDISABLEDSR_OFFSET]

	/* test core bit */
	and  w0, w1, w0

	ret
endfunc _soc_ck_disabled


/* Part of CPU_ON
 * Function restarts a core shutdown via _soc_core_entr_off
 * in:  x0 = core mask lsb (of the target cpu)
 * out: x0 == 0, on success
 *	  x0 != 0, on failure
 * uses x0, x1, x2, x3, x4, x5, x6
 */
func _soc_core_restart
	mov  x6, x30
	mov  x4, x0

	/* pgm GICD_CTLR - enable secure grp0  */
	mov  x5, #NXP_GICD_ADDR
	ldr  w2, [x5, #GICD_CTLR_OFFSET]
	orr  w2, w2, #GICD_CTLR_EN_GRP_0
	str  w2, [x5, #GICD_CTLR_OFFSET]
	dsb sy
	isb

	/* poll on RWP til write completes */
4:
	ldr  w2, [x5, #GICD_CTLR_OFFSET]
	tst  w2, #GICD_CTLR_RWP
	b.ne 4b

	/* x4 = core mask lsb
	* x5 = gicd base addr
	*/
	mov  x0, x4
	bl   get_mpidr_value

	/* x0 = mpidr of target core
	* x4 = core mask lsb of target core
	* x5 = gicd base addr
	*/

	/* generate target list bit */
	and  x1, x0, #MPIDR_AFFINITY0_MASK
	mov  x2, #1
	lsl  x2, x2, x1

	/* get the affinity1 field */
	and  x1, x0, #MPIDR_AFFINITY1_MASK
	lsl  x1, x1, #8
	orr  x2, x2, x1

	/* insert the INTID for SGI15 */
	orr  x2, x2, #ICC_SGI0R_EL1_INTID

	/* fire the SGI */
	msr  ICC_SGI0R_EL1, x2
	dsb  sy
	isb

	/* load '0' on success */
	mov  x0, xzr

	mov  x30, x6
	ret
endfunc _soc_core_restart


/* Part of CPU_OFF
 * Function programs SoC & GIC registers in preparation for shutting down
 * the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7
 */
func _soc_core_prep_off
	mov  x8, x30
	mov  x7, x0		/* x7 = core mask lsb */

	mrs  x1, CORTEX_A72_ECTLR_EL1

	/* set smp and disable L2 snoops in cpuectlr */
	orr  x1, x1, #CPUECTLR_SMPEN_EN
	orr  x1, x1, #CPUECTLR_DISABLE_TWALK_PREFETCH
	bic  x1, x1, #CPUECTLR_INS_PREFETCH_MASK
	bic  x1, x1, #CPUECTLR_DAT_PREFETCH_MASK

	/* set retention control in cpuectlr */
	bic  x1, x1, #CPUECTLR_TIMER_MASK
	orr  x1, x1, #CPUECTLR_TIMER_8TICKS
	msr  CORTEX_A72_ECTLR_EL1, x1

	/* get redistributor rd base addr for this core */
	mov  x0, x7
	bl   get_gic_rd_base
	mov  x6, x0

	/* get redistributor sgi base addr for this core */
	mov  x0, x7
	bl   get_gic_sgi_base
	mov  x5, x0

	/* x5 = gicr sgi base addr
 	 * x6 = gicr rd  base addr
	 * x7 = core mask lsb
	 */

	/* disable SGI 15 at redistributor - GICR_ICENABLER0 */
	mov  w3, #GICR_ICENABLER0_SGI15
	str  w3, [x5, #GICR_ICENABLER0_OFFSET]
2:
	/* poll on rwp bit in GICR_CTLR */
	ldr  w4, [x6, #GICR_CTLR_OFFSET]
	tst  w4, #GICR_CTLR_RWP
	b.ne 2b

	/* disable GRP1 interrupts at cpu interface */
	msr  ICC_IGRPEN1_EL3, xzr

	/* disable GRP0 ints at cpu interface */
	msr  ICC_IGRPEN0_EL1, xzr

	/* program the redistributor - poll on GICR_CTLR.RWP as needed */

	/* define SGI 15 as Grp0 - GICR_IGROUPR0 */
	ldr  w4, [x5, #GICR_IGROUPR0_OFFSET]
	bic  w4, w4, #GICR_IGROUPR0_SGI15
	str  w4, [x5, #GICR_IGROUPR0_OFFSET]

	/* define SGI 15 as Grp0 - GICR_IGRPMODR0 */
	ldr  w3, [x5, #GICR_IGRPMODR0_OFFSET]
	bic  w3, w3, #GICR_IGRPMODR0_SGI15
	str  w3, [x5, #GICR_IGRPMODR0_OFFSET]

	/* set priority of SGI 15 to highest (0x0) - GICR_IPRIORITYR3 */
	ldr  w4, [x5, #GICR_IPRIORITYR3_OFFSET]
	bic  w4, w4, #GICR_IPRIORITYR3_SGI15_MASK
	str  w4, [x5, #GICR_IPRIORITYR3_OFFSET]

	/* enable SGI 15 at redistributor - GICR_ISENABLER0 */
	mov  w3, #GICR_ISENABLER0_SGI15
	str  w3, [x5, #GICR_ISENABLER0_OFFSET]
	dsb  sy
	isb
3:
	/* poll on rwp bit in GICR_CTLR */
	ldr  w4, [x6, #GICR_CTLR_OFFSET]
	tst  w4, #GICR_CTLR_RWP
	b.ne 3b

	/* quiesce the debug interfaces */
	mrs  x3, osdlr_el1
	orr  x3, x3, #OSDLR_EL1_DLK_LOCK
	msr  osdlr_el1, x3
	isb

	/* enable grp0 ints */
	mov  x3, #ICC_IGRPEN0_EL1_EN
	msr  ICC_IGRPEN0_EL1, x3

	/* x5 = gicr sgi base addr
	 * x6 = gicr rd  base addr
	 * x7 = core mask lsb
	 */

	/* clear any pending interrupts */
	mvn  w1, wzr
	str  w1, [x5, #GICR_ICPENDR0_OFFSET]

	/* make sure system counter is enabled */
	ldr  x3, =NXP_TIMER_ADDR
	ldr  w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
	tst  w0, #SYS_COUNTER_CNTCR_EN
	b.ne 4f
	orr  w0, w0, #SYS_COUNTER_CNTCR_EN
	str  w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
4:
	/* enable the core timer and mask timer interrupt */
	mov  x1, #CNTP_CTL_EL0_EN
	orr  x1, x1, #CNTP_CTL_EL0_IMASK
	msr  cntp_ctl_el0, x1

	isb
	mov  x30, x8
	ret
endfunc _soc_core_prep_off


/* Part of CPU_OFF:
 * Function performs the final steps to shutdown the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5
 */
func _soc_core_entr_off
	mov  x5, x30
	mov  x4, x0

1:
	/* enter low-power state by executing wfi */
	wfi

	/* see if SGI15 woke us up */
	mrs  x2, ICC_IAR0_EL1
	mov  x3, #ICC_IAR0_EL1_SGI15
	cmp  x2, x3
	b.ne 2f

	/* deactivate the intrrupts. */
	msr ICC_EOIR0_EL1, x2

2:
	/* check if core is turned ON */
	mov  x0, x4
	/* Fetched the core state in x0 */
	bl   _getCoreState

	cmp  x0, #CORE_WAKEUP
	b.ne 1b

	/* Reached here, exited the wfi */

	mov  x30, x5
	ret
endfunc _soc_core_entr_off


/* Part of CPU_OFF:
 * Function starts the process of starting a core back up
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6
 */
func _soc_core_exit_off
	mov  x6, x30
	mov  x5, x0

	/* disable forwarding of GRP0 ints at cpu interface */
	msr  ICC_IGRPEN0_EL1, xzr

	/* get redistributor sgi base addr for this core */
	mov  x0, x5
	bl   get_gic_sgi_base
	mov  x4, x0

	/* x4 = gicr sgi base addr
	 * x5 = core mask
	 */

	/* disable SGI 15 at redistributor - GICR_ICENABLER0 */
	mov  w1, #GICR_ICENABLER0_SGI15
	str  w1, [x4, #GICR_ICENABLER0_OFFSET]

	/* get redistributor rd base addr for this core */
	mov  x0, x5
	bl   get_gic_rd_base
	mov  x4, x0

2:
	/* poll on rwp bit in GICR_CTLR */
	ldr  w2, [x4, #GICR_CTLR_OFFSET]
	tst  w2, #GICR_CTLR_RWP
	b.ne 2b

	/* unlock the debug interfaces */
	mrs  x3, osdlr_el1
	bic  x3, x3, #OSDLR_EL1_DLK_LOCK
	msr  osdlr_el1, x3
	isb

	dsb sy
	isb
	mov  x30, x6
	ret
endfunc _soc_core_exit_off


/* Function requests a reset of the entire SOC
 * in:  none
 * out: none
 * uses: x0, x1, x2, x3, x4, x5, x6
 */
func _soc_sys_reset
	mov  x6, x30

	ldr  x2, =NXP_RST_ADDR
	/* clear the RST_REQ_MSK and SW_RST_REQ */

	mov  w0, #0x00000000
	str  w0, [x2, #RSTCNTL_OFFSET]

	/* initiate the sw reset request */
	mov  w0, #SW_RST_REQ_INIT
	str  w0, [x2, #RSTCNTL_OFFSET]

	/* In case this address range is mapped as cacheable,
	 * flush the write out of the dcaches.
	 */
	add  x2, x2, #RSTCNTL_OFFSET
	dc   cvac, x2
	dsb  st
	isb

	/* Function does not return */
	b  .
endfunc _soc_sys_reset


/* Part of SYSTEM_OFF:
 * Function turns off the SoC clocks
 * Note: Function is not intended to return, and the only allowable
 *	   recovery is POR
 * in:  none
 * out: none
 * uses x0, x1, x2, x3
 */
func _soc_sys_off

	/* A-009810: LPM20 entry sequence might cause
	 * spurious timeout reset request
	 * workaround: MASK RESET REQ RPTOE
	 */
	ldr  x0, =NXP_RESET_ADDR
	ldr  w1, =RSTRQMR_RPTOE_MASK
	str  w1, [x0, #RST_RSTRQMR1_OFFSET]

	/* disable sec, QBman, spi and qspi */
	ldr  x2, =NXP_DCFG_ADDR
	ldr  x0, =DCFG_DEVDISR1_OFFSET
	ldr  w1, =DCFG_DEVDISR1_SEC
	str  w1, [x2, x0]
	ldr  x0, =DCFG_DEVDISR3_OFFSET
	ldr  w1, =DCFG_DEVDISR3_QBMAIN
	str  w1, [x2, x0]
	ldr  x0, =DCFG_DEVDISR4_OFFSET
	ldr  w1, =DCFG_DEVDISR4_SPI_QSPI
	str  w1, [x2, x0]

	/* set TPMWAKEMR0 */
	ldr  x0, =TPMWAKEMR0_ADDR
	mov  w1, #0x1
	str  w1, [x0]

	/* disable icache, dcache, mmu @ EL1 */
	mov  x1, #SCTLR_I_C_M_MASK
	mrs  x0, sctlr_el1
	bic  x0, x0, x1
	msr  sctlr_el1, x0

	/* disable L2 prefetches */
	mrs  x0, CORTEX_A72_ECTLR_EL1
	bic  x1, x1, #CPUECTLR_TIMER_MASK
	orr  x0, x0, #CPUECTLR_SMPEN_EN
	orr  x0, x0, #CPUECTLR_TIMER_8TICKS
	msr  CORTEX_A72_ECTLR_EL1, x0
	isb

	/* disable CCN snoop domain */
	mov  x1, #NXP_CCN_HN_F_0_ADDR
	ldr  x0, =CCN_HN_F_SNP_DMN_CTL_MASK
	str  x0, [x1, #CCN_HN_F_SNP_DMN_CTL_CLR_OFFSET]
3:
	ldr  w2, [x1, #CCN_HN_F_SNP_DMN_CTL_OFFSET]
	cmp  w2, #0x2
	b.ne 3b

	mov  x3, #NXP_PMU_ADDR

4:
	ldr  w1, [x3, #PMU_PCPW20SR_OFFSET]
	cmp  w1, #PMU_IDLE_CORE_MASK
	b.ne 4b

	mov  w1, #PMU_IDLE_CLUSTER_MASK
	str  w1, [x3, #PMU_CLAINACTSETR_OFFSET]

1:
	ldr  w1, [x3, #PMU_PCPW20SR_OFFSET]
	cmp  w1, #PMU_IDLE_CORE_MASK
	b.ne 1b

	mov  w1, #PMU_FLUSH_CLUSTER_MASK
	str  w1, [x3, #PMU_CLL2FLUSHSETR_OFFSET]

2:
	ldr  w1, [x3, #PMU_CLL2FLUSHSR_OFFSET]
	cmp  w1, #PMU_FLUSH_CLUSTER_MASK
	b.ne 2b

	mov  w1, #PMU_FLUSH_CLUSTER_MASK
	str  w1, [x3, #PMU_CLSL2FLUSHCLRR_OFFSET]

	mov  w1, #PMU_FLUSH_CLUSTER_MASK
	str  w1, [x3, #PMU_CLSINACTSETR_OFFSET]

	mov  x2, #DAIF_SET_MASK
	mrs  x1, spsr_el1
	orr  x1, x1, x2
	msr  spsr_el1, x1

	mrs  x1, spsr_el2
	orr  x1, x1, x2
	msr  spsr_el2, x1

	/* force the debug interface to be quiescent */
	mrs  x0, osdlr_el1
	orr  x0, x0, #0x1
	msr  osdlr_el1, x0

	/* invalidate all TLB entries at all 3 exception levels */
	tlbi alle1
	tlbi alle2
	tlbi alle3

	/* x3 = pmu base addr */

	/* request lpm20 */
	ldr  x0, =PMU_POWMGTCSR_OFFSET
	ldr  w1, =PMU_POWMGTCSR_VAL
	str  w1, [x3, x0]

5:
	wfe
	b.eq  5b
endfunc _soc_sys_off


/* Part of CPU_SUSPEND
 * Function puts the calling core into standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
func _soc_core_entr_stdby

	dsb  sy
	isb
	wfi

	ret
endfunc _soc_core_entr_stdby


/* Part of CPU_SUSPEND
 * Function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
func _soc_core_prep_stdby

	/* clear CORTEX_A72_ECTLR_EL1[2:0] */
	mrs  x1, CORTEX_A72_ECTLR_EL1
	bic  x1, x1, #CPUECTLR_TIMER_MASK
	msr  CORTEX_A72_ECTLR_EL1, x1

	ret
endfunc _soc_core_prep_stdby


/* Part of CPU_SUSPEND
 * Function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_core_exit_stdby

	ret
endfunc _soc_core_exit_stdby


/* Part of CPU_SUSPEND
 * Function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_core_prep_pwrdn

	/* make sure system counter is enabled */
	ldr  x2, =NXP_TIMER_ADDR
	ldr  w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]
	tst  w0, #SYS_COUNTER_CNTCR_EN
	b.ne 1f
	orr  w0, w0, #SYS_COUNTER_CNTCR_EN
	str  w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]
1:

	/* enable dynamic retention control (CPUECTLR[2:0])
	 * set the SMPEN bit (CPUECTLR[6])
	 */
	mrs  x1, CORTEX_A72_ECTLR_EL1
	bic  x1, x1, #CPUECTLR_RET_MASK
	orr  x1, x1, #CPUECTLR_TIMER_8TICKS
	orr  x1, x1, #CPUECTLR_SMPEN_EN
	msr  CORTEX_A72_ECTLR_EL1, x1

	isb
	ret
endfunc _soc_core_prep_pwrdn


/* Part of CPU_SUSPEND
 * Function puts the calling core into a power-down state
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
func _soc_core_entr_pwrdn

	/* X0 = core mask lsb */

	dsb  sy
	isb
	wfi

	ret
endfunc _soc_core_entr_pwrdn


/* Part of CPU_SUSPEND
 * Function performs any SoC-specific cleanup after power-down state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_core_exit_pwrdn

	ret
endfunc _soc_core_exit_pwrdn


/* Part of CPU_SUSPEND
 * Function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
func _soc_clstr_prep_stdby

	/* clear CORTEX_A72_ECTLR_EL1[2:0] */
	mrs  x1, CORTEX_A72_ECTLR_EL1
	bic  x1, x1, #CPUECTLR_TIMER_MASK
	msr  CORTEX_A72_ECTLR_EL1, x1

	ret
endfunc _soc_clstr_prep_stdby


/* Part of CPU_SUSPEND
 * Function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_clstr_exit_stdby

	ret
endfunc _soc_clstr_exit_stdby


/* Part of CPU_SUSPEND
 * Function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_clstr_prep_pwrdn

	/* make sure system counter is enabled */
	ldr  x2, =NXP_TIMER_ADDR
	ldr  w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]
	tst  w0, #SYS_COUNTER_CNTCR_EN
	b.ne 1f
	orr  w0, w0, #SYS_COUNTER_CNTCR_EN
	str  w0, [x2, #SYS_COUNTER_CNTCR_OFFSET]
1:

	/* enable dynamic retention control (CPUECTLR[2:0])
	 * set the SMPEN bit (CPUECTLR[6])
	 */
	mrs  x1, CORTEX_A72_ECTLR_EL1
	bic  x1, x1, #CPUECTLR_RET_MASK
	orr  x1, x1, #CPUECTLR_TIMER_8TICKS
	orr  x1, x1, #CPUECTLR_SMPEN_EN
	msr  CORTEX_A72_ECTLR_EL1, x1

	isb
	ret
endfunc _soc_clstr_prep_pwrdn


/* Part of CPU_SUSPEND
 * Function performs any SoC-specific cleanup after power-down state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_clstr_exit_pwrdn

	ret
endfunc _soc_clstr_exit_pwrdn


/* Part of CPU_SUSPEND
 * Function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
func _soc_sys_prep_stdby

	/* clear CORTEX_A72_ECTLR_EL1[2:0] */
	mrs  x1, CORTEX_A72_ECTLR_EL1
	bic  x1, x1, #CPUECTLR_TIMER_MASK
	msr  CORTEX_A72_ECTLR_EL1, x1
	ret
endfunc _soc_sys_prep_stdby


/* Part of CPU_SUSPEND
 * Function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
func _soc_sys_exit_stdby

	ret
endfunc _soc_sys_exit_stdby


/* Part of CPU_SUSPEND
 * Function performs SoC-specific programming prior to
 * suspend-to-power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
func _soc_sys_prep_pwrdn

	mrs   x1, CORTEX_A72_ECTLR_EL1
	/* make sure the smp bit is set */
	orr   x1, x1, #CPUECTLR_SMPEN_MASK
	/* set the retention control */
	orr   x1, x1, #CPUECTLR_RET_8CLK
	/* disable tablewalk prefetch */
	orr   x1, x1, #CPUECTLR_DISABLE_TWALK_PREFETCH
	msr   CORTEX_A72_ECTLR_EL1, x1
	isb

	ret
endfunc _soc_sys_prep_pwrdn


/* Part of CPU_SUSPEND
 * Function puts the calling core, and potentially the soc, into a
 * low-power state
 * in:  x0 = core mask lsb
 * out: x0 = 0, success
 *	  x0 < 0, failure
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14,
 *	  x15, x16, x17, x18, x19, x20, x21, x28
 */
func _soc_sys_pwrdn_wfi
	mov  x28, x30

	/* disable cluster snooping in the CCN-508 */
	ldr  x1, =NXP_CCN_HN_F_0_ADDR
	ldr  x7, [x1, #CCN_HN_F_SNP_DMN_CTL_OFFSET]
	mov  x6, #CCN_HNF_NODE_COUNT
1:
	str  x7, [x1, #CCN_HN_F_SNP_DMN_CTL_CLR_OFFSET]
	sub  x6, x6, #1
	add  x1, x1, #CCN_HNF_OFFSET
	cbnz x6, 1b

	/* x0  = core mask
	 * x7  = hnf sdcr
	 */

	ldr  x1, =NXP_PMU_CCSR_ADDR
	ldr  x2, =NXP_PMU_DCSR_ADDR

	/* enable the stop-request-override */
	mov  x3, #PMU_POWMGTDCR0_OFFSET
	mov  x4, #POWMGTDCR_STP_OV_EN
	str  w4, [x2, x3]

	/* x0  = core mask
	 * x1  = NXP_PMU_CCSR_ADDR
	 * x2  = NXP_PMU_DCSR_ADDR
	 * x7  = hnf sdcr
	 */

	/* disable prefetching in the A72 core */
	mrs  x8, CORTEX_A72_CPUACTLR_EL1
	tst  x8, #CPUACTLR_DIS_LS_HW_PRE
	b.ne 2f
	dsb  sy
	isb
	/* disable data prefetch */
	orr  x16, x8, #CPUACTLR_DIS_LS_HW_PRE
	/* disable tlb prefetch */
	orr  x16, x16, #CPUACTLR_DIS_L2_TLB_PRE
	msr  CORTEX_A72_CPUACTLR_EL1, x16
	isb

	/* x0  = core mask
	 * x1  = NXP_PMU_CCSR_ADDR
	 * x2  = NXP_PMU_DCSR_ADDR
	 * x7  = hnf sdcr
	 * x8  = cpuactlr
	 */

2:
	/* save hnf-sdcr and cpuactlr to stack */
	stp  x7,  x8,  [sp, #-16]!

	/* x0  = core mask
	 * x1  = NXP_PMU_CCSR_ADDR
	 * x2  = NXP_PMU_DCSR_ADDR
	 */

	/* save the IPSTPCRn registers to stack */
	mov  x15, #PMU_IPSTPCR0_OFFSET
	ldr  w9,  [x1, x15]
	mov  x16, #PMU_IPSTPCR1_OFFSET
	ldr  w10, [x1, x16]
	mov  x17, #PMU_IPSTPCR2_OFFSET
	ldr  w11, [x1, x17]
	mov  x18, #PMU_IPSTPCR3_OFFSET
	ldr  w12, [x1, x18]
	mov  x19, #PMU_IPSTPCR4_OFFSET
	ldr  w13, [x1, x19]
	mov  x20, #PMU_IPSTPCR5_OFFSET
	ldr  w14, [x1, x20]

	stp  x9,  x10,  [sp, #-16]!
	stp  x11, x12,  [sp, #-16]!
	stp  x13, x14,  [sp, #-16]!

	/* x0  = core mask
	 * x1  = NXP_PMU_CCSR_ADDR
	 * x2  = NXP_PMU_DCSR_ADDR
	 * x15 = PMU_IPSTPCR0_OFFSET
	 * x16 = PMU_IPSTPCR1_OFFSET
	 * x17 = PMU_IPSTPCR2_OFFSET
	 * x18 = PMU_IPSTPCR3_OFFSET
	 * x19 = PMU_IPSTPCR4_OFFSET
	 * x20 = PMU_IPSTPCR5_OFFSET
	 */

	/* load the full clock mask for IPSTPCR0 */
	ldr  x3, =DEVDISR1_MASK
	/* get the exclusions */
	mov  x21, #PMU_IPPDEXPCR0_OFFSET
	ldr  w4, [x1, x21]
	/* apply the exclusions to the mask */
	bic  w7, w3, w4
	/* stop the clocks in IPSTPCR0 */
	str  w7, [x1, x15]

	/* use same procedure for IPSTPCR1-IPSTPCR5 */

	/* stop the clocks in IPSTPCR1 */
	ldr  x5, =DEVDISR2_MASK
	mov  x21, #PMU_IPPDEXPCR1_OFFSET
	ldr  w6, [x1, x21]
	bic  w8, w5, w6
	str  w8, [x1, x16]

	/* stop the clocks in IPSTPCR2 */
	ldr  x3, =DEVDISR3_MASK
	mov  x21, #PMU_IPPDEXPCR2_OFFSET
	ldr  w4, [x1, x21]
	bic  w9, w3, w4
	str  w9, [x1, x17]

	/* stop the clocks in IPSTPCR3 */
	ldr  x5,  =DEVDISR4_MASK
	mov  x21, #PMU_IPPDEXPCR3_OFFSET
	ldr  w6,  [x1, x21]
	bic  w10, w5, w6
	str  w10, [x1, x18]

	/* stop the clocks in IPSTPCR4
	 *   - exclude the ddr clocks as we are currently executing
	 *	 out of *some* memory, might be ddr
	 *   - exclude the OCRAM clk so that we retain any code/data in
	 *	 OCRAM
	 *   - may need to exclude the debug clock if we are testing
	 */
	ldr  x3, =DEVDISR5_MASK
	mov  w6, #DEVDISR5_MASK_ALL_MEM
	bic  w3, w3, w6

	mov  w5, #POLICY_DEBUG_ENABLE
	cbz  w5, 3f
	mov  w6, #DEVDISR5_MASK_DBG
	bic  w3, w3, w6
3:
	mov  x21, #PMU_IPPDEXPCR4_OFFSET
	ldr  w4,  [x1, x21]
	bic  w11, w3, w4
	str  w11, [x1, x19]

	/* stop the clocks in IPSTPCR5 */
	ldr  x5,  =DEVDISR6_MASK
	mov  x21, #PMU_IPPDEXPCR5_OFFSET
	ldr  w6,  [x1, x21]
	bic  w12, w5, w6
	str  w12, [x1, x20]

	/* x0  = core mask
	 * x1  = NXP_PMU_CCSR_ADDR
	 * x2  = NXP_PMU_DCSR_ADDR
	 * x7  = IPSTPCR0
	 * x8  = IPSTPCR1
	 * x9  = IPSTPCR2
	 * x10 = IPSTPCR3
	 * x11 = IPSTPCR4
	 * x12 = IPSTPCR5
	 */

	/* poll until the clocks are stopped in IPSTPACKSR0 */
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x21, #PMU_IPSTPACKSR0_OFFSET
4:
	ldr  w5, [x1, x21]
	cmp  w5, w7
	b.eq 5f
	sub  w4, w4, #1
	cbnz w4, 4b

	/* poll until the clocks are stopped in IPSTPACKSR1 */
5:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x21, #PMU_IPSTPACKSR1_OFFSET
6:
	ldr  w5, [x1, x21]
	cmp  w5, w8
	b.eq 7f
	sub  w4, w4, #1
	cbnz w4, 6b

	/* poll until the clocks are stopped in IPSTPACKSR2 */
7:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x21, #PMU_IPSTPACKSR2_OFFSET
8:
	ldr  w5, [x1, x21]
	cmp  w5, w9
	b.eq 9f
	sub  w4, w4, #1
	cbnz w4, 8b

	/* poll until the clocks are stopped in IPSTPACKSR3 */
9:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x21, #PMU_IPSTPACKSR3_OFFSET
10:
	ldr  w5, [x1, x21]
	cmp  w5, w10
	b.eq 11f
	sub  w4, w4, #1
	cbnz w4, 10b

	/* poll until the clocks are stopped in IPSTPACKSR4 */
11:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x21, #PMU_IPSTPACKSR4_OFFSET
12:
	ldr  w5, [x1, x21]
	cmp  w5, w11
	b.eq 13f
	sub  w4, w4, #1
	cbnz w4, 12b

	/* poll until the clocks are stopped in IPSTPACKSR5 */
13:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x21, #PMU_IPSTPACKSR5_OFFSET
14:
	ldr  w5, [x1, x21]
	cmp  w5, w12
	b.eq 15f
	sub  w4, w4, #1
	cbnz w4, 14b

	/* x0  = core mask
	 * x1  = NXP_PMU_CCSR_ADDR
	 * x2  = NXP_PMU_DCSR_ADDR
	 * x7  = IPSTPCR0
	 * x8  = IPSTPCR1
	 * x9  = IPSTPCR2
	 * x10 = IPSTPCR3
	 * x11 = IPSTPCR4
	 * x12 = IPSTPCR5
	 */

15:
	mov  x3, #NXP_DCFG_ADDR

	/* save the devdisr registers to stack */
	ldr  w13, [x3, #DCFG_DEVDISR1_OFFSET]
	ldr  w14, [x3, #DCFG_DEVDISR2_OFFSET]
	ldr  w15, [x3, #DCFG_DEVDISR3_OFFSET]
	ldr  w16, [x3, #DCFG_DEVDISR4_OFFSET]
	ldr  w17, [x3, #DCFG_DEVDISR5_OFFSET]
	ldr  w18, [x3, #DCFG_DEVDISR6_OFFSET]

	stp  x13, x14,  [sp, #-16]!
	stp  x15, x16,  [sp, #-16]!
	stp  x17, x18,  [sp, #-16]!

	/* power down the IP in DEVDISR1 - corresponds to IPSTPCR0 */
	str  w7,  [x3, #DCFG_DEVDISR1_OFFSET]

	/* power down the IP in DEVDISR2 - corresponds to IPSTPCR1 */
	str  w8, [x3, #DCFG_DEVDISR2_OFFSET]

	/* power down the IP in DEVDISR3 - corresponds to IPSTPCR2 */
	str  w9,  [x3, #DCFG_DEVDISR3_OFFSET]

	/* power down the IP in DEVDISR4 - corresponds to IPSTPCR3 */
	str  w10, [x3, #DCFG_DEVDISR4_OFFSET]

	/* power down the IP in DEVDISR5 - corresponds to IPSTPCR4 */
	str  w11, [x3, #DCFG_DEVDISR5_OFFSET]

	/* power down the IP in DEVDISR6 - corresponds to IPSTPCR5 */
	str  w12, [x3, #DCFG_DEVDISR6_OFFSET]

	/* setup register values for the cache-only sequence */
	mov  x4, #NXP_DDR_ADDR
	mov  x5, #NXP_DDR2_ADDR
	mov  x6, x11
	mov  x7, x17
	ldr  x12, =PMU_CLAINACTSETR_OFFSET
	ldr  x13, =PMU_CLSINACTSETR_OFFSET
	ldr  x14, =PMU_CLAINACTCLRR_OFFSET
	ldr  x15, =PMU_CLSINACTCLRR_OFFSET

	/* x0  = core mask
	 * x1  = NXP_PMU_CCSR_ADDR
	 * x2  = NXP_PMU_DCSR_ADDR
	 * x3  = NXP_DCFG_ADDR
	 * x4  = NXP_DDR_ADDR
	 * x5  = NXP_DDR2_ADDR
	 * w6  = IPSTPCR4
	 * w7  = DEVDISR5
	 * x12 = PMU_CLAINACTSETR_OFFSET
	 * x13 = PMU_CLSINACTSETR_OFFSET
	 * x14 = PMU_CLAINACTCLRR_OFFSET
	 * x15 = PMU_CLSINACTCLRR_OFFSET
	 */

	mov  x8, #POLICY_DEBUG_ENABLE
	cbnz x8, 29f
	/* force the debug interface to be quiescent */
	mrs  x9, OSDLR_EL1
	orr  x9, x9, #0x1
	msr  OSDLR_EL1, x9

	/* enter the cache-only sequence */
29:
	bl   final_pwrdown

	/* when we are here, the core has come out of wfi and the
	 * ddr is back up
	 */

	mov  x8, #POLICY_DEBUG_ENABLE
	cbnz x8, 30f
	/* restart the debug interface */
	mrs  x9, OSDLR_EL1
	mov  x10, #1
	bic  x9, x9, x10
	msr  OSDLR_EL1, x9

	/* get saved DEVDISR regs off stack */
30:
	ldp  x17, x18, [sp], #16
	ldp  x15, x16, [sp], #16
	ldp  x13, x14, [sp], #16
	/* restore DEVDISR regs */
	str  w18, [x3, #DCFG_DEVDISR6_OFFSET]
	str  w17, [x3, #DCFG_DEVDISR5_OFFSET]
	str  w16, [x3, #DCFG_DEVDISR4_OFFSET]
	str  w15, [x3, #DCFG_DEVDISR3_OFFSET]
	str  w14, [x3, #DCFG_DEVDISR2_OFFSET]
	str  w13, [x3, #DCFG_DEVDISR1_OFFSET]
	isb

	/* get saved IPSTPCRn regs off stack */
	ldp  x13, x14, [sp], #16
	ldp  x11, x12, [sp], #16
	ldp  x9,  x10, [sp], #16

	/* restore IPSTPCRn regs */
	mov  x15, #PMU_IPSTPCR5_OFFSET
	str  w14, [x1, x15]
	mov  x16, #PMU_IPSTPCR4_OFFSET
	str  w13, [x1, x16]
	mov  x17, #PMU_IPSTPCR3_OFFSET
	str  w12, [x1, x17]
	mov  x18, #PMU_IPSTPCR2_OFFSET
	str  w11, [x1, x18]
	mov  x19, #PMU_IPSTPCR1_OFFSET
	str  w10, [x1, x19]
	mov  x20, #PMU_IPSTPCR0_OFFSET
	str  w9,  [x1, x20]
	isb

	/* poll on IPSTPACKCRn regs til IP clocks are restarted */
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x15, #PMU_IPSTPACKSR5_OFFSET
16:
	ldr  w5, [x1, x15]
	and  w5, w5, w14
	cbz  w5, 17f
	sub  w4, w4, #1
	cbnz w4, 16b

17:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x15, #PMU_IPSTPACKSR4_OFFSET
18:
	ldr  w5, [x1, x15]
	and  w5, w5, w13
	cbz  w5, 19f
	sub  w4, w4, #1
	cbnz w4, 18b

19:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x15, #PMU_IPSTPACKSR3_OFFSET
20:
	ldr  w5, [x1, x15]
	and  w5, w5, w12
	cbz  w5, 21f
	sub  w4, w4, #1
	cbnz w4, 20b

21:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x15, #PMU_IPSTPACKSR2_OFFSET
22:
	ldr  w5, [x1, x15]
	and  w5, w5, w11
	cbz  w5, 23f
	sub  w4, w4, #1
	cbnz w4, 22b

23:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x15, #PMU_IPSTPACKSR1_OFFSET
24:
	ldr  w5, [x1, x15]
	and  w5, w5, w10
	cbz  w5, 25f
	sub  w4, w4, #1
	cbnz w4, 24b

25:
	mov  w4,  #CLOCK_RETRY_CNT
	mov  x15, #PMU_IPSTPACKSR0_OFFSET
26:
	ldr  w5, [x1, x15]
	and  w5, w5, w9
	cbz  w5, 27f
	sub  w4, w4, #1
	cbnz w4, 26b

27:
	/* disable the stop-request-override */
	mov  x8, #PMU_POWMGTDCR0_OFFSET
	mov  w9, #POWMGTDCR_STP_OV_EN
	str  w9, [x2, x8]
	isb

	/* get hnf-sdcr and cpuactlr off stack */
	ldp  x7, x8, [sp], #16

	/* restore cpuactlr */
	msr  CORTEX_A72_CPUACTLR_EL1, x8
	isb

	/* restore snooping in the hnf nodes */
	ldr  x9, =NXP_CCN_HN_F_0_ADDR
	mov  x6, #CCN_HNF_NODE_COUNT
28:
	str  x7, [x9, #CCN_HN_F_SNP_DMN_CTL_SET_OFFSET]
	sub  x6, x6, #1
	add  x9, x9, #CCN_HNF_OFFSET
	cbnz x6, 28b
	isb

	mov  x30, x28
	ret
endfunc _soc_sys_pwrdn_wfi


/* Part of CPU_SUSPEND
 * Function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0,
 */
func _soc_sys_exit_pwrdn

	mrs   x1, CORTEX_A72_ECTLR_EL1
	/* make sure the smp bit is set */
	orr   x1, x1, #CPUECTLR_SMPEN_MASK
	/* clr the retention control */
	mov   x2, #CPUECTLR_RET_8CLK
	bic   x1, x1, x2
	/* enable tablewalk prefetch */
	mov   x2, #CPUECTLR_DISABLE_TWALK_PREFETCH
	bic   x1, x1, x2
	msr   CORTEX_A72_ECTLR_EL1, x1
	isb

	ret
endfunc _soc_sys_exit_pwrdn


/* Function will pwrdown ddr and the final core - it will do this
 * by loading itself into the icache and then executing from there
 * in:
 *   x0  = core mask
 *   x1  = NXP_PMU_CCSR_ADDR
 *   x2  = NXP_PMU_DCSR_ADDR
 *   x3  = NXP_DCFG_ADDR
 *   x4  = NXP_DDR_ADDR
 *   x5  = NXP_DDR2_ADDR
 *   w6  = IPSTPCR4
 *   w7  = DEVDISR5
 *   x12 = PMU_CLAINACTSETR_OFFSET
 *   x13 = PMU_CLSINACTSETR_OFFSET
 *   x14 = PMU_CLAINACTCLRR_OFFSET
 *   x15 = PMU_CLSINACTCLRR_OFFSET
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x13, x14, x15, x16,
 *	  x17, x18
 */

/* 4Kb aligned */
.align 12
func final_pwrdown

	mov  x0, xzr
	b	touch_line_0
start_line_0:
	mov  x0, #1
	/* put ddr controller 1 into self-refresh */
	ldr  w8, [x4, #DDR_CFG_2_OFFSET]
	orr  w8, w8, #CFG_2_FORCE_REFRESH
	str  w8, [x4, #DDR_CFG_2_OFFSET]

	/* put ddr controller 2 into self-refresh */
	ldr  w8, [x5, #DDR_CFG_2_OFFSET]
	orr  w8, w8, #CFG_2_FORCE_REFRESH
	str  w8, [x5, #DDR_CFG_2_OFFSET]

	/* stop the clocks in both ddr controllers */
	mov  w10, #DEVDISR5_MASK_DDR
	mov  x16, #PMU_IPSTPCR4_OFFSET
	orr  w9,  w6, w10
	str  w9,  [x1, x16]
	isb

	mov  x17, #PMU_IPSTPACKSR4_OFFSET
touch_line_0:
	cbz  x0, touch_line_1

start_line_1:
	/* poll IPSTPACKSR4 until
	 * ddr controller clocks are stopped.
	 */
1:
	ldr  w8, [x1, x17]
	and  w8, w8, w10
	cmp  w8, w10
	b.ne 1b

	/* shut down power to the ddr controllers */
	orr w9, w7, #DEVDISR5_MASK_DDR
	str w9, [x3, #DCFG_DEVDISR5_OFFSET]

	/* disable cluster acp ports */
	mov  w8, #CLAINACT_DISABLE_ACP
	str  w8, [x1, x12]

	/* disable skyros ports */
	mov  w9, #CLSINACT_DISABLE_SKY
	str  w9, [x1, x13]
	isb

touch_line_1:
	cbz  x0, touch_line_2

start_line_2:
	isb
3:
	wfi

	/* if we are here then we are awake
	 * - bring this device back up
	 */

	/* enable skyros ports */
	mov  w9, #CLSINACT_DISABLE_SKY
	str  w9, [x1, x15]

	/* enable acp ports */
	mov  w8, #CLAINACT_DISABLE_ACP
	str  w8, [x1, x14]
	isb

	/* bring up the ddr controllers */
	str w7, [x3, #DCFG_DEVDISR5_OFFSET]
	isb
	str w6,  [x1, x16]
	isb

	nop
touch_line_2:
	cbz  x0, touch_line_3

start_line_3:
	/* poll IPSTPACKSR4 until
	 * ddr controller clocks are running
	 */
	mov w10, #DEVDISR5_MASK_DDR
2:
	ldr  w8, [x1, x17]
	and  w8, w8, w10
	cbnz w8, 2b

	/* take ddr controller 2 out of self-refresh */
	mov w8, #CFG_2_FORCE_REFRESH
	ldr w9, [x5, #DDR_CFG_2_OFFSET]
	bic w9, w9, w8
	str w9, [x5, #DDR_CFG_2_OFFSET]

	/* take ddr controller 1 out of self-refresh */
	ldr w9, [x4, #DDR_CFG_2_OFFSET]
	bic w9, w9, w8
	str w9, [x4, #DDR_CFG_2_OFFSET]
	isb

	nop
	nop
	nop
touch_line_3:
	cbz  x0, start_line_0

	/* execute here after ddr is back up */

	ret
endfunc final_pwrdown

/* Function returns CLUSTER_3_NORMAL if the cores of cluster 3 are
 * to be handled normally, and it returns CLUSTER_3_IN_RESET if the cores
 * are to be held in reset
 * in:  none
 * out: x0 = #CLUSTER_3_NORMAL,   cluster 3 treated normal
 *	  x0 = #CLUSTER_3_IN_RESET, cluster 3 cores held in reset
 * uses x0, x1, x2
 */
func cluster3InReset

	/* default return is treat cores normal */
	mov  x0, #CLUSTER_3_NORMAL

	/* read RCW_SR27 register */
	mov  x1, #NXP_DCFG_ADDR
	ldr  w2, [x1, #RCW_SR27_OFFSET]

	/* test the cluster 3 bit */
	tst  w2, #CLUSTER_3_RCW_BIT
	b.eq 1f

	/* if we are here, then the bit was set */
	mov  x0, #CLUSTER_3_IN_RESET
1:
	ret
endfunc cluster3InReset


/* Function checks to see if cores which are to be disabled have been
 * released from reset - if not, it releases them
 * Note: there may be special handling of cluster 3 cores depending upon the
 *	   sys clk frequency
 * in:  none
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9
 */
func release_disabled
	mov  x9, x30

	/* check if we need to keep cluster 3 cores in reset */
	bl   cluster3InReset		/*  0-2  */
	mov  x8, x0

	/* x8 = cluster 3 handling */

	/* read COREDISABLESR */
	mov  x0, #NXP_DCFG_ADDR
	ldr  w4, [x0, #DCFG_COREDISABLEDSR_OFFSET]
	cmp  x8, #CLUSTER_3_IN_RESET
	b.ne 4f

	/* the cluster 3 cores are to be held in reset, so remove
	 * them from the disable mask
	 */
	bic  x4, x4, #CLUSTER_3_CORES_MASK
4:
	/* get the number of cpus on this device */
	mov   x6, #PLATFORM_CORE_COUNT

	mov  x0, #NXP_RESET_ADDR
	ldr  w5, [x0, #BRR_OFFSET]

	/* load the core mask for the first core */
	mov  x7, #1

	/* x4 = COREDISABLESR
	 * x5 = BRR
	 * x6 = loop count
	 * x7 = core mask bit
	 */
2:
	/* check if the core is to be disabled */
	tst  x4, x7
	b.eq 1f

	/* see if disabled cores have already been released from reset */
	tst  x5, x7
	b.ne 5f

	/* if core has not been released, then release it (0-3) */
	mov  x0, x7
	bl   _soc_core_release

	/* record the core state in the data area (0-3) */
	mov  x0, x7
	mov  x1, #CORE_STATE_DATA
	mov  x2, #CORE_DISABLED
	bl   _setCoreData

1:
	/* see if this is a cluster 3 core */
	mov   x3, #CLUSTER_3_CORES_MASK
	tst   x3, x7
	b.eq  5f

	/* this is a cluster 3 core - see if it needs to be held in reset */
	cmp  x8, #CLUSTER_3_IN_RESET
	b.ne 5f

	/* record the core state as disabled in the data area (0-3) */
	mov  x0, x7
	mov  x1, #CORE_STATE_DATA
	mov  x2, #CORE_DISABLED
	bl   _setCoreData

5:
	/* decrement the counter */
	subs  x6, x6, #1
	b.le  3f

	/* shift the core mask to the next core */
	lsl   x7, x7, #1
	/* continue */
	b	 2b
3:
	cmp  x8, #CLUSTER_3_IN_RESET
	b.ne 6f

	/* we need to hold the cluster 3 cores in reset,
	 * so mark them in the COREDISR and COREDISABLEDSR registers as
	 * "disabled", and the rest of the sw stack will leave them alone
	 * thinking that they have been disabled
	 */
	mov  x0, #NXP_DCFG_ADDR
	ldr  w1, [x0, #DCFG_COREDISR_OFFSET]
	orr  w1, w1, #CLUSTER_3_CORES_MASK
	str  w1, [x0, #DCFG_COREDISR_OFFSET]

	ldr  w2, [x0, #DCFG_COREDISABLEDSR_OFFSET]
	orr  w2, w2, #CLUSTER_3_CORES_MASK
	str  w2, [x0, #DCFG_COREDISABLEDSR_OFFSET]
	dsb  sy
	isb

#if (PSCI_TEST)
	/* x0 = NXP_DCFG_ADDR : read COREDISABLESR */
	ldr  w4, [x0, #DCFG_COREDISABLEDSR_OFFSET]
	/* read COREDISR */
	ldr  w3, [x0, #DCFG_COREDISR_OFFSET]
#endif

6:
	mov  x30, x9
	ret

endfunc release_disabled


/* Function setc up the TrustZone Address Space Controller (TZASC)
 * in:  none
 * out: none
 * uses x0, x1
 */
func init_tzpc

	/* set Non Secure access for all devices protected via TZPC */

	/* decode Protection-0 Set Reg */
	ldr	x1, =TZPCDECPROT_0_SET_BASE
	/* set decode region to NS, Bits[7:0] */
	mov	w0, #0xFF
	str	w0, [x1]

	/* decode Protection-1 Set Reg */
	ldr	x1, =TZPCDECPROT_1_SET_BASE
	/* set decode region to NS, Bits[7:0] */
	mov	w0, #0xFF
	str	w0, [x1]

	/* decode Protection-2 Set Reg */
	ldr	x1, =TZPCDECPROT_2_SET_BASE
	/* set decode region to NS, Bits[7:0] */
	mov	w0, #0xFF
	str	w0, [x1]

	/* entire SRAM as NS */
	/* secure RAM region size Reg */
	ldr	x1, =TZPC_BASE
	/* 0x00000000 = no secure region */
	mov	w0, #0x00000000
	str	w0, [x1]

	ret
endfunc init_tzpc

/* write a register in the DCFG block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2
 */
func _write_reg_dcfg
	ldr  x2, =NXP_DCFG_ADDR
	str  w1, [x2, x0]
	ret
endfunc _write_reg_dcfg


/* read a register in the DCFG block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
func _read_reg_dcfg
	ldr  x2, =NXP_DCFG_ADDR
	ldr  w1, [x2, x0]
	mov  w0, w1
	ret
endfunc _read_reg_dcfg


/* Function returns an mpidr value for a core, given a core_mask_lsb
 * in:  x0 = core mask lsb
 * out: x0 = affinity2:affinity1:affinity0, where affinity is 8-bits
 * uses x0, x1
 */
func get_mpidr_value

	/* convert a core mask to an SoC core number */
	clz  w0, w0
	mov  w1, #31
	sub  w0, w1, w0

	/* get the mpidr core number from the SoC core number */
	mov  w1, wzr
	tst  x0, #1
	b.eq 1f
	orr  w1, w1, #1

1:
	/* extract the cluster number */
	lsr  w0, w0, #1
	orr  w0, w1, w0, lsl #8

	ret
endfunc get_mpidr_value


/* Function returns the redistributor base address for the core specified
 * in x1
 * in:  x0 - core mask lsb of specified core
 * out: x0 = redistributor rd base address for specified core
 * uses x0, x1, x2
 */
func get_gic_rd_base
	clz  w1, w0
	mov  w2, #0x20
	sub  w2, w2, w1
	sub  w2, w2, #1

	ldr  x0, =NXP_GICR_ADDR
	mov  x1, #GIC_RD_OFFSET

	/* x2 = core number
	 * loop counter
	 */
2:
	cbz  x2, 1f
	add  x0, x0, x1
	sub  x2, x2, #1
	b	2b
1:
	ret
endfunc get_gic_rd_base


/* Function returns the redistributor base address for the core specified
 * in x1
 * in:  x0 - core mask lsb of specified core
 * out: x0 = redistributor sgi base address for specified core
 * uses x0, x1, x2
 */
func get_gic_sgi_base
	clz  w1, w0
	mov  w2, #0x20
	sub  w2, w2, w1
	sub  w2, w2, #1

	ldr  x0, =NXP_GICR_SGI_ADDR
	mov  x1, #GIC_SGI_OFFSET

	/* loop counter */
2:
	cbz  x2, 1f		/* x2 = core number */
	add  x0, x0, x1
	sub  x2, x2, #1
	b	2b
1:
	ret
endfunc get_gic_sgi_base

/* Function writes a register in the RESET block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2
 */
func _write_reg_reset
	ldr  x2, =NXP_RESET_ADDR
	str  w1, [x2, x0]
	ret
endfunc _write_reg_reset


/* Function reads a register in the RESET block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1
 */
func _read_reg_reset
	ldr  x1, =NXP_RESET_ADDR
	ldr  w0, [x1, x0]
	ret
endfunc _read_reg_reset
