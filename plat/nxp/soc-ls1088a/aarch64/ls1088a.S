/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

.section .text, "ax"

#include <asm_macros.S>
#include <lib/psci/psci.h>
#include <nxp_timer.h>
#include <plat_gic.h>

#include "bl31_data.h"
#include "plat_psci.h"
#include "platform_def.h"

/*
 * the BASE address for these offsets is AUX_01_DATA in the
 * bootcore's psci data region
 */
#define DEVDISR2_MASK_OFFSET	0x0    /* references AUX_01_DATA */
#define DEVDISR5_MASK_OFFSET	0x8    /* references AUX_02_DATA */

/*
 * the BASE address for these offsets is AUX_04_DATA in the
 * bootcore's psci data region
 */
#define GICD_BASE_ADDR_OFFSET	0x0    /* references AUX_04_DATA */
#define GICC_BASE_ADDR_OFFSET	0x8    /* references AUX_05_DATA */

#define IPSTPACK_RETRY_CNT	0x10000
#define DDR_SLEEP_RETRY_CNT	0x10000
#define CPUACTLR_EL1		S3_1_C15_C2_0
#define DDR_SDRAM_CFG_2_FRCSR	0x80000000
#define DDR_SDRAM_CFG_2_OFFSET	0x114
#define DDR_TIMING_CFG_4_OFFSET	0x160
#define DDR_CNTRL_BASE_ADDR	0x01080000

#define DLL_LOCK_MASK		0x3
#define DLL_LOCK_VALUE		0x2

#define ERROR_DDR_SLEEP		-1
#define ERROR_DDR_WAKE		-2
#define ERROR_NO_QUIESCE	-3

#define CORE_RESTARTABLE	0
#define CORE_NOT_RESTARTABLE	1

.global soc_init_lowlevel
.global soc_init_percpu

.global _soc_core_release
.global _soc_core_restart
.global _soc_ck_disabled
.global _soc_sys_reset
.global _soc_sys_off

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

.global _set_platform_security
.global _soc_set_start_addr

.equ TZPCDECPROT_0_SET_BASE, 0x02200804
.equ TZPCDECPROT_1_SET_BASE, 0x02200810
.equ TZPCDECPROT_2_SET_BASE, 0x0220081C

.equ TZASC_REGION_ATTRIBUTES_0_0, 0x01100110

.equ MPIDR_AFFINITY0_MASK, 0x00FF
.equ MPIDR_AFFINITY1_MASK, 0xFF00
.equ CPUECTLR_DISABLE_TWALK_PREFETCH, 0x4000000000
.equ CPUECTLR_INS_PREFETCH_MASK, 0x1800000000
.equ CPUECTLR_DAT_PREFETCH_MASK, 0x0300000000
.equ OSDLR_EL1_DLK_LOCK, 0x1
.equ CNTP_CTL_EL0_EN, 0x1
.equ CNTP_CTL_EL0_IMASK, 0x2
/* shifted value for incrementing cluster count in mpidr */
.equ MPIDR_CLUSTER, 0x100

/*
 * This function initialize the soc,
 * in: none
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11
 */
func soc_init_lowlevel
	/*
	 * called from C, so save the non-volatile regs
	 * save these as pairs of registers to maintain the
	 * required 16-byte alignment on the stack
	 */
	stp	x4, x5, [sp, #-16]!
	stp	x6, x7, [sp, #-16]!
	stp	x8, x9, [sp, #-16]!
	stp	x10, x11, [sp, #-16]!
	stp	x12, x13, [sp, #-16]!
	stp	x18, x30, [sp, #-16]!

	/*
	 * make sure the personality has been established by releasing cores
	 * that are marked "to-be-disabled" from reset
	 */
	bl	release_disabled

	/* set SCRATCHRW7 to 0x0 */
	ldr	x0, =DCFG_SCRATCHRW7_OFFSET
	mov	x1, xzr
	bl	_write_reg_dcfg

	/* restore the aarch32/64 non-volatile registers */
	ldp	x18, x30, [sp], #16
	ldp	x12, x13, [sp], #16
	ldp	x10, x11, [sp], #16
	ldp	x8, x9, [sp], #16
	ldp	x6, x7, [sp], #16
	ldp	x4, x5, [sp], #16
	ret
endfunc soc_init_lowlevel

/*
 * void soc_init_percpu(void)
 * this function performs any soc-specific initialization that is needed on
 * a per-core basis
 * in:  none
 * out: none
 * uses x0, x1, x2, x3
 */
func soc_init_percpu
	stp	x4, x30, [sp, #-16]!

	bl	plat_my_core_mask
	mov	x2, x0

	/* x2 = core mask */

	/* see if this core is marked for prefetch disable */
	mov	x0, #PREFETCH_DIS_OFFSET
	bl	_get_global_data
	tst	x0, x2
	b.eq	1f
	bl	_disable_ldstr_pfetch_A53
1:
	mov	x0, #NXP_PMU_ADDR
	bl	enable_timer_base_to_cluster
	ldp	x4, x30, [sp], #16
	ret
endfunc soc_init_percpu

/*
 * this function sets the security mechanisms in the SoC to implement the
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
 * this function writes a 64-bit address to bootlocptrh/l
 * in:  x0, 64-bit address to write to BOOTLOCPTRL/H
 * out: none
 * uses x0, x1, x2
 */
func _soc_set_start_addr
	/* get the 64-bit base address of the dcfg block */
	ldr	x2, =NXP_DCFG_ADDR

	/* write the 32-bit BOOTLOCPTRL register */
	mov	x1, x0
	str	w1, [x2, #DCFG_BOOTLOCPTRL_OFFSET]

	/* write the 32-bit BOOTLOCPTRH register */
	lsr	x1, x0, #32
	str	w1, [x2, #DCFG_BOOTLOCPTRH_OFFSET]
	ret
endfunc _soc_set_start_addr

/*
 * part of CPU_ON
 * this function releases a secondary core from reset
 * in: x0 = core_mask_lsb
 * out: none
 * uses: x0, x1, x2, x3
 */
_soc_core_release:
	mov	x3, x30

	/* x0 = core mask */

	ldr	x1, =NXP_SEC_REGFILE_ADDR
	/*
	 * write to CORE_HOLD to tell the bootrom that we want this core
	 * to run
	 */
	str	w0, [x1, #CORE_HOLD_OFFSET]

	/* x0 = core mask */

	/* read-modify-write BRRL to release core */
	mov	x1, #NXP_RESET_ADDR
	ldr	w2, [x1, #BRR_OFFSET]
	orr	w2, w2, w0
	str	w2, [x1, #BRR_OFFSET]
	dsb	sy
	isb

	/* send event */
	sev
	isb

	mov	x30, x3
	ret

/*
 * this function determines if a core is disabled via COREDISABLEDSR
 * in:  w0  = core_mask_lsb
 * out: w0  = 0, core not disabled
 *      w0 != 0, core disabled
 * uses x0, x1
 */
_soc_ck_disabled:
	/* get base addr of dcfg block */
	ldr	x1, =NXP_DCFG_ADDR

	/* read COREDISABLEDSR */
	ldr	w1, [x1, #DCFG_COREDISABLEDSR_OFFSET]

	/* test core bit */
	and	w0, w1, w0

	ret

/*
 * part of CPU_ON
 * this function restarts a core shutdown via _soc_core_entr_off
 * in:  x0 = core mask lsb (of the target cpu)
 * out: x0 == 0, on success
 *      x0 != 0, on failure
 * uses x0, x1, x2, x3, x4, x5, x6
 */
_soc_core_restart:
	mov	x6, x30
	mov	x4, x0

	/* x4 = core mask lsb */

	/* pgm GICD_CTLR - enable secure grp0  */
	mov	x5, #NXP_GICD_ADDR
	ldr	w2, [x5, #GICD_CTLR_OFFSET]
	orr	w2, w2, #GICD_CTLR_EN_GRP_0
	str	w2, [x5, #GICD_CTLR_OFFSET]
	dsb	sy
	isb
	/* poll on RWP til write completes */
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

	/*
	 * x0 = mpidr of target core
	 * x4 = core mask lsb of target core
	 * x5 = gicd base addr
	 */

	/* generate target list bit */
	and	x1, x0, #MPIDR_AFFINITY0_MASK
	mov	x2, #1
	lsl	x2, x2, x1
	/* get the affinity1 field */
	and	x1, x0, #MPIDR_AFFINITY1_MASK
	lsl	x1, x1, #8
	orr	x2, x2, x1
	/* insert the INTID for SGI15 */
	orr	x2, x2, #ICC_SGI0R_EL1_INTID
	/* fire the SGI */
	msr	ICC_SGI0R_EL1, x2
	dsb	sy
	isb

	/* load '0' on success */
	mov	x0, xzr

	mov	x30, x6
	ret

/*
 * part of CPU_OFF
 * this function programs SoC & GIC registers in preparation for shutting down
 * the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7
 */
_soc_core_prep_off:
	mov	x8, x30
	mov	x7, x0

	/* x7 = core mask lsb */

	mrs	x1, CPUECTLR_EL1
	/* set smp and disable L2 snoops in cpuectlr */
	orr	x1, x1, #CPUECTLR_SMPEN_EN
	orr	x1, x1, #CPUECTLR_DISABLE_TWALK_PREFETCH
	bic	x1, x1, #CPUECTLR_INS_PREFETCH_MASK
	bic	x1, x1, #CPUECTLR_DAT_PREFETCH_MASK
	/* set retention control in cpuectlr */
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	orr	x1, x1, #CPUECTLR_TIMER_8TICKS
	msr	CPUECTLR_EL1, x1

	/* get redistributor rd base addr for this core */
	mov	x0, x7
	bl	get_gic_rd_base
	mov	x6, x0

	/* get redistributor sgi base addr for this core */
	mov	x0, x7
	bl	get_gic_sgi_base
	mov	x5, x0

	/* x5 = gicr sgi base addr
	 * x6 = gicr rd  base addr
	 * x7 = core mask lsb
	 */

	/* disable SGI 15 at redistributor - GICR_ICENABLER0 */
	mov	w3, #GICR_ICENABLER0_SGI15
	str	w3, [x5, #GICR_ICENABLER0_OFFSET]
2:
	/* poll on rwp bit in GICR_CTLR */
	ldr	w4, [x6, #GICR_CTLR_OFFSET]
	tst	w4, #GICR_CTLR_RWP
	b.ne	2b

	/* disable GRP1 interrupts at cpu interface */
	msr	ICC_IGRPEN1_EL3, xzr

	/* disable GRP0 ints at cpu interface */
	msr	ICC_IGRPEN0_EL1, xzr

	/* program the redistributor - poll on GICR_CTLR.RWP as needed */

	/* define SGI 15 as Grp0 - GICR_IGROUPR0 */
	ldr	w4, [x5, #GICR_IGROUPR0_OFFSET]
	bic	w4, w4, #GICR_IGROUPR0_SGI15
	str	w4, [x5, #GICR_IGROUPR0_OFFSET]

	/* define SGI 15 as Grp0 - GICR_IGRPMODR0 */
	ldr	w3, [x5, #GICR_IGRPMODR0_OFFSET]
	bic	w3, w3, #GICR_IGRPMODR0_SGI15
	str	w3, [x5, #GICR_IGRPMODR0_OFFSET]

	/* set priority of SGI 15 to highest (0x0) - GICR_IPRIORITYR3 */
	ldr	w4, [x5, #GICR_IPRIORITYR3_OFFSET]
	bic	w4, w4, #GICR_IPRIORITYR3_SGI15_MASK
	str	w4, [x5, #GICR_IPRIORITYR3_OFFSET]

	/* enable SGI 15 at redistributor - GICR_ISENABLER0 */
	mov	w3, #GICR_ISENABLER0_SGI15
	str	w3, [x5, #GICR_ISENABLER0_OFFSET]
	dsb	sy
	isb
3:
	/* poll on rwp bit in GICR_CTLR */
	ldr	w4, [x6, #GICR_CTLR_OFFSET]
	tst	w4, #GICR_CTLR_RWP
	b.ne	3b

	/* quiesce the debug interfaces */
	mrs	x3, osdlr_el1
	orr	x3, x3, #OSDLR_EL1_DLK_LOCK
	msr	osdlr_el1, x3
	isb

	/* enable grp0 ints */
	mov	x3, #ICC_IGRPEN0_EL1_EN
	msr	ICC_IGRPEN0_EL1, x3

	/*
	 * x5 = gicr sgi base addr
	 * x6 = gicr rd  base addr
	 * x7 = core mask lsb
	 */

	/* clear any pending interrupts */
	mvn	w1, wzr
	str	w1, [x5, #GICR_ICPENDR0_OFFSET]

	/* make sure system counter is enabled */
	ldr	x3, =NXP_TIMER_ADDR
	ldr	w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
	tst	w0, #SYS_COUNTER_CNTCR_EN
	b.ne	4f
	orr	w0, w0, #SYS_COUNTER_CNTCR_EN
	str	w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
4:
	/* enable the core timer and mask timer interrupt */
	mov	x1, #CNTP_CTL_EL0_EN
	orr	x1, x1, #CNTP_CTL_EL0_IMASK
	msr	cntp_ctl_el0, x1

	mov	x30, x8
	ret

/*
 * part of CPU_OFF
 * this function performs the final steps to shutdown the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5
 */
_soc_core_entr_off:
	mov	x5, x30
	mov	x4, x0

	/* x4 = core mask */
1:
	/* enter low-power state by executing wfi */
	wfi

	/* see if SGI15 woke us up */
	mrs	x2, ICC_IAR0_EL1
	mov	x3, #ICC_IAR0_EL1_SGI15
	cmp	x2, x3
	b.ne	2f

	/* deactivate the int */
	msr	ICC_EOIR0_EL1, x2

	/* x4 = core mask */
2:
	/* check if core has been turned on */
	mov	x0, x4
	bl	_getCoreState

	/* x0 = core state */

	cmp	x0, #CORE_WAKEUP
	b.ne	1b

	/* if we get here, then we have exited the wfi */

	mov	x30, x5
	ret

/*
 * part of CPU_OFF
 * this function starts the process of starting a core back up
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6
 */
_soc_core_exit_off:
	mov	x6, x30
	mov	x5, x0

	/* disable forwarding of GRP0 ints at cpu interface */
	msr	ICC_IGRPEN0_EL1, xzr

	/* get redistributor sgi base addr for this core */
	mov	x0, x5
	bl	get_gic_sgi_base
	mov	x4, x0

	/*
	 * x4 = gicr sgi base addr
	 * x5 = core mask
	 */

	/* disable SGI 15 at redistributor - GICR_ICENABLER0 */
	mov	w1, #GICR_ICENABLER0_SGI15
	str	w1, [x4, #GICR_ICENABLER0_OFFSET]

	/* get redistributor rd base addr for this core */
	mov	x0, x5
	bl	get_gic_rd_base
	mov	x4, x0

	/* x4 = gicr rd  base addr */
2:
	/* poll on rwp bit in GICR_CTLR */
	ldr	w2, [x4, #GICR_CTLR_OFFSET]
	tst	w2, #GICR_CTLR_RWP
	b.ne	2b

	/* x4 = gicr rd  base addr */

	/* unlock the debug interfaces */
	mrs	x3, osdlr_el1
	bic	x3, x3, #OSDLR_EL1_DLK_LOCK
	msr	osdlr_el1, x3
	isb

	dsb	sy
	isb
	mov	x30, x6
	ret

/*
 * this function requests a reset of the entire SOC
 * in:  none
 * out: none
 * uses: x0, x1, x2, x3, x4, x5, x6
 */
_soc_sys_reset:
	mov	x3, x30

	/* make sure the mask is cleared in the reset request mask register */
	mov	x0, #RST_RSTRQMR1_OFFSET
	mov	w1, wzr
	bl	_write_reg_reset

	/* set the reset request */
	mov	x4, #RST_RSTCR_OFFSET
	mov	x0, x4
	mov	w1, #RSTCR_RESET_REQ
	bl	_write_reg_reset

	/* x4 = RST_RSTCR_OFFSET */

	/*
	 * just in case this address range is mapped as cacheable,
	 * flush the write out of the dcaches
	 */
	mov	x2, #NXP_RESET_ADDR
	add	x2, x2, x4
	dc	cvac, x2
	dsb	st
	isb

	/* this function does not return */
	b	.

/*
 * this function turns off the SoC
 * Note: this function is not intended to return, and the only allowable
 *       recovery is POR
 * in:  none
 * out: none
 * uses x0, x1, x2, x3
 */
_soc_sys_off:
	/*
	 * A-009810: LPM20 entry sequence might cause
	 * spurious timeout reset request
	 * workaround: MASK RESET REQ RPTOE
	 */
	ldr	x0, =NXP_RESET_ADDR
	ldr	w1, [x0, #RST_RSTRQMR1_OFFSET]
	orr	w1, w1, #RSTRQMR_RPTOE_MASK
	str	w1, [x0, #RST_RSTRQMR1_OFFSET]

	/* disable SEC, QBman spi and qspi */
	ldr	x2, =NXP_DCFG_ADDR
	ldr	x0, =DCFG_DEVDISR1_OFFSET
	ldr	w1, =DCFG_DEVDISR1_SEC
	str	w1, [x2, x0]
	ldr	x0, =DCFG_DEVDISR3_OFFSET
	ldr	w1, =DCFG_DEVDISR3_QBMAIN
	str	w1, [x2, x0]
	ldr	x0, =DCFG_DEVDISR4_OFFSET
	ldr	w1, =DCFG_DEVDISR4_SPI_QSPI
	str	w1, [x2, x0]

	/* set TPMWAKEMR0 */
	ldr	x0, =TPMWAKEMR0_ADDR
	mov	w1, #0x1
	str	w1, [x0]

	/* disable icache, dcache, mmu @ EL1 */
	mov	x1, #SCTLR_I_C_M_MASK
	mrs	x0, sctlr_el1
	bic	x0, x0, x1
	msr	sctlr_el1, x0

	/* disable L2 prefetches */
	mrs	x0, CPUECTLR_EL1
	orr	x0, x0, #CPUECTLR_SMPEN_EN
	orr	x0, x0, #CPUECTLR_TIMER_8TICKS
	msr	CPUECTLR_EL1, x0
	dsb	sy
	isb

	/* disable CCN snoop domain */
	ldr	x0, =NXP_CCI_ADDR
	mov	w1, #0x1
	str	w1, [x0]

	mov	x2, #DAIF_SET_MASK

	mrs	x1, spsr_el1
	orr	x1, x1, x2
	msr	spsr_el1, x1

	mrs	x1, spsr_el2
	orr	x1, x1, x2
	msr	spsr_el2, x1

	bl	get_pmu_idle_cluster_mask
	mov	x3, #NXP_PMU_ADDR

	/* x3 = pmu base addr */

	/* idle the ACP interfaces */
	str	w0, [x3, #PMU_CLAINACTSETR_OFFSET]

	/* force the debug interface to be quiescent */
	mrs	x0, osdlr_el1
	orr	x0, x0, #0x1
	msr	osdlr_el1, x0

	bl	get_pmu_flush_cluster_mask
	/* x3 = pmu base addr */
	mov	x3, #NXP_PMU_ADDR

	/* clear flush request and status */
	ldr	x2, =PMU_CLSL2FLUSHCLRR_OFFSET
	str	w0, [x3, x2]

	/* close the Skyros master port */
	ldr	x2, =PMU_CLSINACTSETR_OFFSET
	str	w0, [x3, x2]

	/* request lpm20 */
	ldr	x0, =PMU_POWMGTCSR_OFFSET
	ldr	w1, =PMU_POWMGTCSR_VAL
	str	w1, [x3, x0]

	/* this function does not return */
1:
	wfi
	b	1b

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_core_prep_stdby:
	/* clear CPUECTLR_EL1[2:0] */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CPUECTLR_EL1, x1

	ret

/*
 * part of CPU_SUSPEND
 * this function puts the calling core into standby state
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
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_core_exit_stdby:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3
 */
_soc_core_prep_pwrdn:
	/* make sure system counter is enabled */
	ldr	x3, =NXP_TIMER_ADDR
	ldr	w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
	tst	w0, #SYS_COUNTER_CNTCR_EN
	b.ne	1f
	orr	w0, w0, #SYS_COUNTER_CNTCR_EN
	str	w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
1:
	/*
	 * enable dynamic retention control (CPUECTLR[2:0])
	 * set the SMPEN bit (CPUECTLR[6])
	 */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_RET_MASK
	orr	x1, x1, #CPUECTLR_TIMER_8TICKS
	orr	x1, x1, #CPUECTLR_SMPEN_EN
	msr	CPUECTLR_EL1, x1

	isb
	ret

/*
 * part of CPU_SUSPEND
 * this function puts the calling core into a power-down state
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
 * part of CPU_SUSPEND
 * this function cleans up after a core exits power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses
 */
_soc_core_exit_pwrdn:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_clstr_prep_stdby:
	/* clear CPUECTLR_EL1[2:0] */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CPUECTLR_EL1, x1

	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_clstr_exit_stdby:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1, x2, x3
 */
_soc_clstr_prep_pwrdn:
	/* make sure system counter is enabled */
	ldr	x3, =NXP_TIMER_ADDR
	ldr	w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
	tst	w0, #SYS_COUNTER_CNTCR_EN
	b.ne	1f
	orr	w0, w0, #SYS_COUNTER_CNTCR_EN
	str	w0, [x3, #SYS_COUNTER_CNTCR_OFFSET]
1:
	/*
	 * enable dynamic retention control (CPUECTLR[2:0])
	 * set the SMPEN bit (CPUECTLR[6])
	 */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_RET_MASK
	orr	x1, x1, #CPUECTLR_TIMER_8TICKS
	orr	x1, x1, #CPUECTLR_SMPEN_EN
	msr	CPUECTLR_EL1, x1

	isb
	ret

/*
 * part of CPU_SUSPEND
 * this function cleans up after a core exits power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses
 */
_soc_clstr_exit_pwrdn:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_sys_prep_stdby:
	/* clear CPUECTLR_EL1[2:0] */
	mrs	x1, CPUECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CPUECTLR_EL1, x1

	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses none
 */
_soc_sys_exit_stdby:
	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to
 * suspend-to-power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
_soc_sys_prep_pwrdn:
	/* set retention control */
	mrs	x0, CPUECTLR_EL1
	bic	x0, x0, #CPUECTLR_TIMER_MASK
	orr	x0, x0, #CPUECTLR_TIMER_8TICKS
	orr	x0, x0, #CPUECTLR_SMPEN_EN
	msr	CPUECTLR_EL1, x0
	dsb	sy
	isb

	ret

/*
 * part of CPU_SUSPEND
 * this function puts the calling core, and potentially the soc, into a
 * low-power state
 * in:  x0 = core mask lsb
 * out: x0 = 0, success
 *      x0 < 0, failure
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8
 */
_soc_sys_pwrdn_wfi:
	/* Save LR to stack */
	stp	x18, x30, [sp, #-16]!

	/* Poll PCPW20SR for all secondary cores to be placed in PW20 */
	bl	get_tot_num_cores
	mov	x3, #0x1
	lsl	x3, x3, x0
	sub	x3, x3, #2
1:
	mov	x0, #NXP_PMU_ADDR
	ldr	w1, [x0, #PMU_PCPW20SR_OFFSET]
	cmp	w1, w3
	b.ne	1b

	/* backup EPU registers to stack */
	mov	x3, #NXP_PMU_ADDR
	ldr	x2, =NXP_EPU_ADDR
	ldr	w4, [x2, #EPU_EPIMCR10_OFFSET]
	ldr	w5, [x2, #EPU_EPCCR10_OFFSET]
	ldr	w6, [x2, #EPU_EPCTR10_OFFSET]
	ldr	w7, [x2, #EPU_EPGCR_OFFSET]
	stp	x4, x5, [sp, #-16]!
	stp	x6, x7, [sp, #-16]!

	/*
	 * x2 = epu base addr
	 * x3 = pmu base addr
	 */

	/* set up EPU event to receive the wake signal from PMU */
	mov	w4, #EPU_EPIMCR10_VAL
	mov	w5, #EPU_EPCCR10_VAL
	mov	w6, #EPU_EPCTR10_VAL
	mov	w7, #EPU_EPGCR_VAL
	str	w4, [x2, #EPU_EPIMCR10_OFFSET]
	str	w5, [x2, #EPU_EPCCR10_OFFSET]
	str	w6, [x2, #EPU_EPCTR10_OFFSET]
	str	w7, [x2, #EPU_EPGCR_OFFSET]

	/*
	 * A-010194: There is logic problem
	 * in the path of GIC-to-PMU to issue
	 * wake request to core0
	 * Workaround: Re-target the wakeup interrupts
	 * to a core other than the last active core0
	 */
	ldr	x2, =NXP_GICD_ADDR

	/* backup flextimer/mmc/usb interrupt router */
	ldr	x0, =GICD_IROUTER60_OFFSET
	ldr	x1, =GICD_IROUTER76_OFFSET
	ldr	w4, [x2, x0]
	ldr	w5, [x2, x1]
	ldr	x0, =GICD_IROUTER112_OFFSET
	ldr	x1, =GICD_IROUTER113_OFFSET
	ldr	w6, [x2, x0]
	ldr	w7, [x2, x1]
	stp	x4, x5, [sp, #-16]!
	stp	x6,  x7,  [sp, #-16]!

	/*
	 * x2 = gicd base addr
	 * x0 = GICD_IROUTER112_OFFSET
	 * x1 = GICD_IROUTER113_OFFSET
	 */

	/* re-route interrupt to cluster 1 */
	ldr	w4, =GICD_IROUTER_VALUE
	str	w4, [x2, x0]
	str	w4, [x2, x1]
	ldr	x0, =GICD_IROUTER60_OFFSET
	ldr	x1, =GICD_IROUTER76_OFFSET
	str	w4, [x2, x0]
	str	w4, [x2, x1]
	dsb	sy
	isb

	/* backup flextimer/mmc/usb interrupt enabler */
	ldr	x0, =GICD_ISENABLER_1
	ldr	w4, [x2, x0]
	ldr	x1, =GICD_ISENABLER_2
	ldr	w5, [x2, x1]
	stp	x4, x5, [sp, #-16]!

	ldr	x0, =GICD_ISENABLER_3
	ldr	w4, [x2, x0]
	ldr	x1, =GICD_ICENABLER_1
	ldr	w5, [x2, x1]
	stp	x4, x5, [sp, #-16]!

	ldr	x0, =GICD_ICENABLER_2
	ldr	w4, [x2, x0]
	ldr	x1, =GICD_ICENABLER_3
	ldr	w5, [x2, x1]
	stp	x4, x5, [sp, #-16]!

	/* enable related interrupt routing */
	ldr	w4, =GICD_ISENABLER_1_VALUE
	ldr	x0, =GICD_ISENABLER_1
	str	w4, [x2, x0]
	dsb	sy
	isb

	ldr	w4, =GICD_ISENABLER_2_VALUE
	ldr	x0, =GICD_ISENABLER_2
	str	w4, [x2, x0]
	dsb	sy
	isb

	ldr	w4, =GICD_ISENABLER_3_VALUE
	ldr	x0, =GICD_ISENABLER_3
	str	w4, [x2, x0]
	dsb	sy
	isb

	/* set POWMGTDCR [STP_PV_EN] = 1 */
	ldr	x2, =NXP_POWMGTDCR
	ldr	w4, =0x01
	str	w4, [x2]

	/* program IPSTPCR for override stop request (except DDR) */
	mov	x3, #NXP_PMU_ADDR

	/* build an override mask for IPSTPCR4/IPSTPACK4/DEVDISR5 */
	ldr	x2, =PMU_IPPDEXPCR4_OFFSET
	ldr	w7, [x3, x2]

	mov	x5, xzr
	ldr	x6, =IPPDEXPCR4_MASK
	and	x6, x6, x7
	cbz	x6, 1f

	/*
	 * x5 = override mask
	 * x6 = IPPDEXPCR bits for DEVDISR5
	 * x7 = IPPDEXPCR
	 */

	/* get the overrides */
	orr	x4, x5, #DEVDISR5_FLX_TMR
	tst	x6, #IPPDEXPCR_FLX_TMR
	csel	x5, x5, x4, EQ
1:
	/* store the DEVDISR5 override mask */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA
	str	w5, [x2, #DEVDISR5_MASK_OFFSET]

	mov	x3, #NXP_PMU_ADDR

	/* write IPSTPCR0 - no overrides */
	ldr	x2, =PMU_IPSTPCR0_OFFSET
	ldr	w5, =IPSTPCR0_VALUE
	str	w5, [x3, x2]

	/* write IPSTPCR1 - no overrides */
	ldr	x2, =PMU_IPSTPCR1_OFFSET
	ldr	w5, =IPSTPCR1_VALUE
	str	w5, [x3, x2]

	/* write IPSTPCR2 - no overrides */
	ldr	x2, =PMU_IPSTPCR2_OFFSET
	ldr	w5, =IPSTPCR2_VALUE
	str	w5, [x3, x2]

	/* write IPSTPCR3 - no overrides */
	ldr	x2, =PMU_IPSTPCR3_OFFSET
	ldr	w5, =IPSTPCR3_VALUE
	str	w5, [x3, x2]

	/* write IPSTPCR4 - overrides possible */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA
	ldr	w6, [x2, #DEVDISR5_MASK_OFFSET]
	ldr	x2, =PMU_IPSTPCR4_OFFSET
	ldr	w5, =IPSTPCR4_VALUE
	bic	x5, x5, x6
	str	w5, [x3, x2]

	/* write IPSTPCR5 - no overrides */
	ldr	x2, =PMU_IPSTPCR5_OFFSET
	ldr	w5, =IPSTPCR5_VALUE
	str	w5, [x3, x2]

	/* write IPSTPCR6 - no overrides */
	ldr	x2, =PMU_IPSTPCR6_OFFSET
	ldr	w5, =IPSTPCR6_VALUE
	str	w5, [x3, x2]

	/* poll IPSTPACK for IP stop acknowledgment (except DDR) */
	mov	x3, #NXP_PMU_ADDR

	/* poll on IPSTPACK0 */
	ldr	x2, =PMU_IPSTPACK0_OFFSET
	ldr	x4, =IPSTPCR0_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
3:
	ldr	w0, [x3, x2]
	cmp	x0, x4
	b.eq	14f
	sub	x7, x7, #1
	cbnz	x7, 3b

14:
	/* poll on IPSTPACK1 */
	ldr	x2, =PMU_IPSTPACK1_OFFSET
	ldr	x4, =IPSTPCR1_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
4:
	ldr	w0, [x3, x2]
	cmp	x0, x4
	b.eq	15f
	sub	x7, x7, #1
	cbnz	x7, 4b

15:
	/* poll on IPSTPACK2 */
	ldr	x2, =PMU_IPSTPACK2_OFFSET
	ldr	x4, =IPSTPCR2_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
5:
	ldr	w0, [x3, x2]
	cmp	x0, x4
	b.eq	16f
	sub	x7, x7, #1
	cbnz	x7, 5b

16:
	/* poll on IPSTPACK3 */
	ldr	x2, =PMU_IPSTPACK3_OFFSET
	ldr	x4, =IPSTPCR3_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
6:
	ldr	w0, [x3, x2]
	cmp	x0, x4
	b.eq	17f
	sub	x7, x7, #1
	cbnz	x7, 6b

17:
	/* poll on IPSTPACK4 */
	ldr	x2, =PMU_IPSTPACK4_OFFSET
	ldr	x4, =IPSTPCR4_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
7:
	ldr	w0, [x3, x2]
	cmp	x0, x4
	b.eq	18f
	sub	x7, x7, #1
	cbnz	x7, 7b

18:
	/* poll on IPSTPACK5 */
	ldr	x2, =PMU_IPSTPACK5_OFFSET
	ldr	x4, =IPSTPCR5_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
8:
	ldr	w0, [x3, x2]
	cmp	x0, x4
	b.eq	19f
	sub	x7, x7, #1
	cbnz	x7, 8b

19:
	/* poll on IPSTPACK6 */
	ldr	x2, =PMU_IPSTPACK6_OFFSET
	ldr	x4, =IPSTPCR6_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
9:
	ldr	w0, [x3, x2]
	cmp	x0, x4
	b.eq	20f
	sub	x7, x7, #1
	cbnz	x7, 9b

20:
	/* save current DEVDISR states to DDR. */
	ldr	x2, =NXP_DCFG_ADDR

	/* save DEVDISR1 and load new value */
	ldr	x0, =DCFG_DEVDISR1_OFFSET
	ldr	w1, [x2, x0]
	mov	w13, w1
	ldr	x1, =DEVDISR1_VALUE
	str	w1, [x2, x0]
	/* save DEVDISR2 and load new value */
	ldr	x0, =DCFG_DEVDISR2_OFFSET
	ldr	w1, [x2, x0]
	mov	w14, w1
	ldr	x1, =DEVDISR2_VALUE
	str	w1, [x2, x0]

	/* x6 = DEVDISR5 override mask */

	/* save DEVDISR3 and load new value */
	ldr	x0, =DCFG_DEVDISR3_OFFSET
	ldr	w1, [x2, x0]
	mov	w15, w1
	ldr	x1, =DEVDISR3_VALUE
	str	w1, [x2, x0]

	/* save DEVDISR4 and load new value */
	ldr	x0, =DCFG_DEVDISR4_OFFSET
	ldr	w1, [x2, x0]
	mov	w16, w1
	/* not stop uart print */
	ldr	x1, =0x0000332
	str	w1, [x2, x0]

	/* save DEVDISR5 and load new value */
	ldr	x0, =DCFG_DEVDISR5_OFFSET
	ldr	w1, [x2, x0]
	mov	w17, w1
	/* Enable this wakeup will fail, should enable OCRAM */
	ldr	x1, =0x00102300
	str	w1, [x2, x0]

	/* save DEVDISR6 and load new value */
	ldr	x0, =DCFG_DEVDISR6_OFFSET
	ldr	w1, [x2, x0]
	mov	w18, w1
	ldr	x1, =DEVDISR6_VALUE
	str	w1, [x2, x0]

	/*
	 * w13 = DEVDISR1 saved value
	 * w14 = DEVDISR2 saved value
	 * w15 = DEVDISR3 saved value
	 * w16 = DEVDISR4 saved value
	 * w17 = DEVDISR5 saved value
	 * w18 = DEVDISR6 saved value
	 */
	/*
	 * A-009810: LPM20 entry sequence might cause
	 * spurious timeout reset request
	 * workaround: MASK RESET REQ RPTOE
	 */
	ldr	x0, =NXP_RESET_ADDR
	ldr	w1, =RSTRQMR_RPTOE_MASK
	str	w1, [x0, #RST_RSTRQMR1_OFFSET]

	/* disable SEC, QBman spi and qspi */
	ldr	x2, =NXP_DCFG_ADDR
	ldr	x0, =DCFG_DEVDISR1_OFFSET
	ldr	w1, =DCFG_DEVDISR1_SEC
	str	w1, [x2, x0]
	ldr	x0, =DCFG_DEVDISR3_OFFSET
	ldr	w1, =DCFG_DEVDISR3_QBMAIN
	str	w1, [x2, x0]
	ldr	x0, =DCFG_DEVDISR4_OFFSET
	ldr	w1, =DCFG_DEVDISR4_SPI_QSPI
	str	w1, [x2, x0]

	/*
	 * write the GICR_WAKER.ProcessorSleep bits to 1
	 * enable the WakeRequest signal
	 * x3 is cpu mask starting from cpu7
	 */
	bl	get_tot_num_cores
	sub	x0, x0, #1
	mov	x3, #0x1
	lsl	x3, x3, x0
2:
	mov	x0, x3
	bl	get_gic_rd_base
	ldr	w1, [x0, #GICR_WAKER_OFFSET]
	orr	w1, w1, #GICR_WAKER_SLEEP_BIT
	str	w1, [x0, #GICR_WAKER_OFFSET]
1:
	ldr	w1, [x0, #GICR_WAKER_OFFSET]
	cmp	w1, #GICR_WAKER_ASLEEP
	b.ne	1b

	lsr	x3, x3, #1
	cbnz	x3, 2b

	/* x3 = pmu base addr */

	/* perform Icache Warming Sequence */
	ldr	x5, =IPSTPCR4_VALUE
	mov	x6, DDR_CNTRL_BASE_ADDR
	mov	x7, #NXP_PMU_ADDR
	mov	x8, #NXP_DCFG_ADDR
	mov	x10, #PMU_IPSTPCR4_OFFSET
	mov	x11, #PMU_IPSTPACK4_OFFSET
	mov	x12, #PMU_IPSTPCR3_OFFSET
	mov	x18, #PMU_IPSTPCR2_OFFSET
	mov	x19, #PMU_IPSTPCR1_OFFSET
	mov	x21, #PMU_IPSTPCR0_OFFSET
	ldr	x22, =DCFG_DEVDISR5_OFFSET
	ldr	x23, =NXP_EPU_ADDR
	mov	x9, #CORE_RESTARTABLE
	bl	final_pwrdown

	/*
	 * disable the WakeRequest signal on cpu 0-7
	 * x3 is cpu mask starting from cpu7
	 */
	bl	get_tot_num_cores
	sub	x0, x0, #1
	mov	x3, #0x1
	lsl	x3, x3, x0
2:
	mov	x0, x3
	bl	get_gic_rd_base
	ldr	w1, [x0, #GICR_WAKER_OFFSET]
	bic	w1, w1, #GICR_WAKER_SLEEP_BIT
	str	w1, [x0, #GICR_WAKER_OFFSET]
1:
	ldr	w1, [x0, #GICR_WAKER_OFFSET]
	cbnz	w1, 1b

	lsr	x3, x3, #1
	cbnz	x3, 2b

	/* set SGI for secondary core wakeup */
	ldr	x0, =0x1000002
	msr	S3_0_C12_C11_7, x0
	isb
	ldr	x0, =0x2000004
	msr	S3_0_C12_C11_7, x0
	isb
	ldr	x0, =0x3000008
	msr	S3_0_C12_C11_7, x0
	isb
	ldr	x0, =0x4010001
	msr	S3_0_C12_C11_7, x0
	isb
	ldr	x0, =0x5010002
	msr	S3_0_C12_C11_7, x0
	isb
	ldr	x0, =0x6010004
	msr	S3_0_C12_C11_7, x0
	isb
	ldr	x0, =0x7010008
	msr	S3_0_C12_C11_7, x0

	/* enable SEC, QBman spi and qspi */
	ldr	x2, =NXP_DCFG_ADDR
	str	wzr, [x2, #DCFG_DEVDISR1_OFFSET]
	str	wzr, [x2, #DCFG_DEVDISR3_OFFSET]
	str	wzr, [x2, #DCFG_DEVDISR4_OFFSET]

	/* clear POWMGTDCR [STP_PV_EN] */
	ldr	x2, =NXP_POWMGTDCR
	ldr	w4, [x2]
	bic	w4, w4, #0x01
	str	w4, [x2]

	/* restore flextimer/mmc/usb interrupt enabler */
	ldr	x3, =NXP_GICD_ADDR
	ldp	x0, x2, [sp], #16
	ldr	x1, =GICD_ICENABLER_2
	mvn	w0, w0
	str	w0, [x3, x1]
	ldr	x1, =GICD_ICENABLER_3
	mvn	w2, w2
	str	w2, [x3, x1]

	ldp	x0, x2, [sp], #16
	ldr	x1, =GICD_ISENABLER_3
	str	w0, [x3, x1]
	ldr	x1, =GICD_ICENABLER_1
	mvn	w2, w2
	str	w0, [x3, x1]

	ldp	x0, x2, [sp], #16
	ldr	x1, =GICD_ISENABLER_1
	str	w0, [x3, x1]
	ldr	x1, =GICD_ISENABLER_2
	str	w0, [x3, x1]

	/* restore flextimer/mmc/usb interrupt router */
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

	/* restore EPU registers */
	ldr	x3, =NXP_EPU_ADDR
	ldp	x0, x2, [sp], #16
	str	w2, [x3, #EPU_EPGCR_OFFSET]
	str	w0, [x3, #EPU_EPCTR10_OFFSET]
	ldp	x2, x1, [sp], #16
	str	w1, [x3, #EPU_EPCCR10_OFFSET]
	str	w2, [x3, #EPU_EPIMCR10_OFFSET]

	isb
	/* Restor LR */
	ldp	x18, x30, [sp], #16
	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_sys_exit_pwrdn:
	mrs	x1, SCTLR_EL1
	orr	x1, x1, #SCTLR_I_MASK
	msr	SCTLR_EL1, x1
	isb
	ret

/*
 * this function checks to see if cores which are to be disabled have been
 * released from reset - if not, it releases them
 * in:  none
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8
 */
release_disabled:
	mov	x8, x30

	/* read COREDISABLESR */
	mov	x0, #NXP_DCFG_ADDR
	ldr	w4, [x0, #DCFG_COREDISABLEDSR_OFFSET]

	/* get the number of cpus on this device */
	mov	x6, #PLATFORM_CORE_COUNT

	mov	x0, #NXP_RESET_ADDR
	ldr	w5, [x0, #BRR_OFFSET]

	/* load the core mask for the first core */
	mov	x7, #1

	/*
	 * x4 = COREDISABLESR
	 * x5 = BRR
	 * x6 = loop count
	 * x7 = core mask bit
	 */
2:
	/* check if the core is to be disabled */
	tst	x4, x7
	b.eq	1f

	/* see if disabled cores have already been released from reset */
	tst	x5, x7
	b.ne	1f

	/* if core has not been released, then release it (0-3) */
	mov	x0, x7
	bl	_soc_core_release

	/* record the core state in the data area (0-3) */
	mov	x0, x7
	mov	x1, #CORE_DISABLED
	bl	_setCoreState

1:
	/* decrement the counter */
	subs	x6, x6, #1
	b.le	3f

	/* shift the core mask to the next core */
	lsl	x7, x7, #1
	/* continue */
	b	2b
3:
	mov	x30, x8
	ret

/*
 * write a register in the DCFG block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2
 */
_write_reg_dcfg:
	ldr	x2, =NXP_DCFG_ADDR
	str	w1, [x2, x0]
	ret

/*
 * read a register in the DCFG block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1
 */
_read_reg_dcfg:
	ldr	x1, =NXP_DCFG_ADDR
	ldr	w0, [x1, x0]
	ret

/*
 * this function sets up the TrustZone Address Space Controller (TZASC)
 * in:  none
 * out: none
 * uses x0, x1
 */
init_tzpc:
	/*
	 * set Non Secure access for all devices protected via TZPC
	 * decode Protection-0 Set Reg
	 */
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

	/*
	 * entire SRAM as NS
	 * secure RAM region size Reg
	 */
	ldr	x1, =NXP_OCRAM_TZPC_ADDR
	/* 0x00000000 = no secure region */
	mov	w0, #0x00000000
	str	w0, [x1]

	ret

/* this function performs initialization on SecMon for boot services */
initSecMon:
	/* read the register hpcomr */
	ldr	x1, =NXP_SNVS_ADDR
	ldr	w0, [x1, #SECMON_HPCOMR_OFFSET]
	/* turn off secure access for the privileged registers */
	orr	w0, w0, #SECMON_HPCOMR_NPSWAEN
	/* write back */
	str	w0, [x1, #SECMON_HPCOMR_OFFSET]

	ret

/*
 * this function returns the redistributor base address for the core specified
 * in x1
 * in:  x0 - core mask lsb of specified core
 * out: x0 = redistributor rd base address for specified core
 * uses x0, x1, x2
 */
get_gic_rd_base:
	/* get the 0-based core number */
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
 * this function returns the redistributor base address for the core specified
 * in x1
 * in:  x0 - core mask lsb of specified core
 * out: x0 = redistributor sgi base address for specified core
 * uses x0, x1, x2
 */
get_gic_sgi_base:
	/* get the 0-based core number */
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
 * this function returns an mpidr value for a core, given a core_mask_lsb
 * in:  x0 = core mask lsb
 * out: x0 = affinity2:affinity1:affinity0, where affinity is 8-bits
 * uses x0, x1
 */
get_mpidr_value:
	/* convert a core mask to an SoC core number */
	clz	w0, w0
	mov	w1, #31
	sub	w0, w1, w0

	/* w0 = SoC core number */

	mov	w1, wzr
2:
	cmp	w0, #CORES_PER_CLUSTER
	b.lt	1f
	sub	w0, w0, #CORES_PER_CLUSTER
	add	w1, w1, #MPIDR_CLUSTER
	b	2b

	/* insert the mpidr core number */
1:
	orr	w0, w1, w0
	ret

/*
 * write a register in the RESET block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2
 */
_write_reg_reset:
	ldr	x2, =NXP_RESET_ADDR
	str	w1, [x2, x0]
	ret

/*
 * read a register in the RESET block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1
 */
_read_reg_reset:
	ldr	x1, =NXP_RESET_ADDR
	ldr	w0, [x1, x0]
	ret

/*
 * this function will pwrdown ddr and the final core - it will do this
 * by loading itself into the icache and then executing from there
 * in:  x5  = ipstpcr4 (IPSTPCR4_VALUE bic DEVDISR5_MASK)
 *      x6  = DDR_CNTRL_BASE_ADDR
 *      x7  = NXP_PMU_ADDR
 *      x8  = NXP_DCFG_ADDR
 *      x9  = 0, restartable
 *          = 1, non-restartable
 *      x10 = PMU_IPSTPCR4_OFFSET
 *      x11 = PMU_IPSTPACK4_OFFSET
 *      x12 = PMU_IPSTPCR3_OFFSET
 *      x18 = PMU_IPSTPCR2_OFFSET
 *      x19 = PMU_IPSTPCR1_OFFSET
 *      x21 = PMU_IPSTPCR0_OFFSET
 *      w13 = DEVDISR1 saved value
 *      w14 = DEVDISR2 saved value
 *      w15 = DEVDISR3 saved value
 *      w16 = DEVDISR4 saved value
 *      w17 = DEVDISR5 saved value
 *      x22 = DCFG_DEVDISR5_OFFSET
 *      x23 = NXP_EPU_ADDR
 * out: none
 * uses x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x13, x14, x15, x16, x17
 * x10, x11, x12, x18, x19, x21, x22, x23
 */

final_pwrdown:
	/* delay */
	mov	w4, #0xffffff
554:
	sub	w4, w4, #1
	cmp	w4, #0
	b.ge	554b

	mov	x0, xzr
	b	touch_line_0

/* 4Kb aligned */
.align 12
start_line_0:
	mov	x0, #1
	/* put ddr in self refresh - start */
	mov	x2, #DDR_SDRAM_CFG_2_FRCSR
	ldr	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	orr	w3, w3, w2
	/* put ddr in self refresh - end */
	str	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	nop
	nop
touch_line_0:
	cbz	x0, touch_line_1

start_line_1:
	/* quiesce ddr clocks - start */
	orr	w3, w5, #DCFG_DEVDISR5_MEM
	mov	w4, w3
	/* quiesce ddr clocks - end */
	str	w4, [x7, x10]
	mov	w3, #DCFG_DEVDISR5_MEM
	/* poll on ipstpack4 - start */
	mov	x2, #DDR_SLEEP_RETRY_CNT
	nop
	nop
touch_line_1:
	cbz	x0, touch_line_2

start_line_2:
	/* x11 = PMU_IPSTPACK4_OFFSET */
	ldr	w1, [x7, x11]
	tst	w1, w3
	b.ne	5f
	subs	x2, x2, #1
	/* poll on ipstpack4 - end */
	b.gt	start_line_2

	/* if we get here, we have a timeout err */
	mov	w4, w5
	/* x10 = PMU_IPSTPCR4_OFFSET re-enable ddr clks interface */
	str	w4, [x7, x10]
touch_line_2:
	cbz	x0, touch_line_3

start_line_3:
	/* load error code */
	mov	x0, #ERROR_DDR_SLEEP
	b	2f
5:
	wfe
	ldr	w1, [x23, #EPU_EPCTR10_OFFSET]
	cbz	w1, 5b

	mov	w4, w5
touch_line_3:
	cbz	x0, touch_line_4

start_line_4:
	/* re-enable ddr in devdisr5 */
	str	w4, [x8, x22]
	/* re-enable ddr clk in ipstpcr4 */
	str	w4, [x7, x10]
13:
	/* poll on ipstpack4 - start */
	ldr	w1, [x7, x11]
	tst	w1, w3
	b.eq	2f
	nop
	b	13b
	/* poll on ipstpack4 - end */
2:
touch_line_4:
	cbz	x0, touch_line_5

start_line_5:
	/* take ddr out-of self refresh - start */
	mov	x2, #DDR_SDRAM_CFG_2_FRCSR
	ldr	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	mov	w4, w3
	bic	w4, w4, w2
	mov	w3, w4
	/* wait for ddr cntrlr clock- start */
	mov	x1, #DDR_SLEEP_RETRY_CNT
3:
	subs	x1, x1, #1
touch_line_5:
	cbz	x0, touch_line_6

start_line_6:
	/* wait for ddr cntrlr clock - end */
	b.gt	3b
	/* take ddr out-of self refresh - end */
	str	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	mov	w1, w17
	/* reset devdisr5 */
	str	w1, [x8, #DCFG_DEVDISR5_OFFSET]
	mov	w1, w16
	/* reset devdisr4 */
	str	w1, [x8, #DCFG_DEVDISR4_OFFSET]
	mov	w1, w15
touch_line_6:
	cbz	x0, touch_line_7

start_line_7:
	/* reset devdisr3 */
	str	w1, [x8, #DCFG_DEVDISR3_OFFSET]
	mov	w1, w14
	/* reset devdisr2 */
	str	w1, [x8, #DCFG_DEVDISR2_OFFSET]
	mov	w1, w13
	/* reset devdisr1 */
	str	w1, [x8, #DCFG_DEVDISR1_OFFSET]
	/* reset ipstpcr4 */
	str	wzr, [x7, x10]
	/* reset ipstpcr3 */
	str	wzr, [x7, x12]
touch_line_7:
	cbz	x0, touch_line_8

start_line_8:
	/* reset ipstpcr2 */
	str	wzr, [x7, x18]
	/* reset ipstpcr1 */
	str	wzr, [x7, x19]
	/* reset ipstpcr0 */
	str	wzr, [x7, x21]

touch_line_8:
	cbz	x0, touch_line_9

start_line_9:
	b	continue_restart
touch_line_9:
	cbz	x0, start_line_0

/* execute here after ddr is back up */
continue_restart:
	/*
	 * if x0 = 1, all is well
	 * if x0 < 1, we had an error
	 */
	cmp	x0, #1
	b.ne	4f
	mov	x0, #0
4:
	ret
