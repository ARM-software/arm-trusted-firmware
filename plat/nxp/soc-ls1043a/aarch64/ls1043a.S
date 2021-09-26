/*
 * Copyright 2018-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cortex_a53.h>
#include <dcfg_lsch2.h>
#include <plat_gic.h>
#include <scfg.h>

#include <bl31_data.h>
#include <plat_psci.h>
#include <platform_def.h>

/* the BASE address for these offsets is AUX_01_DATA in the */
/* bootcore's psci data region */
#define DEVDISR2_MASK_OFFSET	0x0	/* references AUX_01_DATA */
#define DEVDISR5_MASK_OFFSET	0x8	/* references AUX_02_DATA */
#define CPUACTLR_DATA_OFFSET	0x10	/* references AUX_03_DATA */
/* the BASE address for these offsets is AUX_04_DATA in the */
/* bootcore's psci data region */
#define GICD_BASE_ADDR_OFFSET	0x0	/* references AUX_04_DATA */
#define GICC_BASE_ADDR_OFFSET	0x8	/* references AUX_05_DATA */

#define DAIF_DATA AUX_06_DATA	/* references AUX_06_DATA */

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

#define RESET_RETRY_CNT 800

.global soc_init_lowlevel
.global soc_init_percpu
.global _soc_core_release
.global _soc_core_restart
.global _soc_ck_disabled
.global _soc_sys_reset
.global _soc_sys_off
.global _getGICD_BaseAddr
.global _getGICC_BaseAddr
.global _soc_set_start_addr
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

/*
 * This function initialize the soc.
 * in: void
 * out: void
 */
func soc_init_lowlevel
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
	mov	x3, x30

	bl	plat_my_core_mask
	mov	x2, x0

	/* see if this core is marked for prefetch disable */
	mov	x0, #PREFETCH_DIS_OFFSET
	bl	_get_global_data  /* 0-1 */
	tst	x0, x2
	b.eq	1f
	bl	_disable_ldstr_pfetch_A53  /* 0 */
1:
	mov	x30, x3
	ret
endfunc soc_init_percpu

/*
 * part of CPU_ON
 * this function releases a secondary core from reset
 * in:   x0 = core_mask_lsb
 * out:  none
 * uses: x0, x1, x2, x3
 */
_soc_core_release:

#if (TEST_BL31)
	mov	w2, w0
	CoreMaskMsb	w2, w3
	/* x2 = core mask msb */
#else
	mov	x2, x0
#endif
	/* write COREBCR  */
	ldr	x1, =NXP_SCFG_ADDR
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


/*
 * part of CPU_ON
 * this function restarts a core shutdown via _soc_core_entr_off
 * in:  x0 = core mask lsb (of the target cpu)
 * out: x0 == 0, on success
 *      x0 != 0, on failure
 * uses x0 ~ x5
 */
_soc_core_restart:
	mov	x5, x30
	mov	x3, x0

	/* x3 = core mask lsb */
	bl	_getGICD_BaseAddr
	mov	x4, x0

	/* x4 = GICD_BASE_ADDR */
	/* enable forwarding of group 0 interrupts by setting GICD_CTLR[0] = 1 */
	ldr	w1, [x4, #GICD_CTLR_OFFSET]
	orr	w1, w1, #GICD_CTLR_EN_GRP0
	str	w1, [x4, #GICD_CTLR_OFFSET]
	dsb	sy
	isb

	/*
	 * fire SGI by writing to GICD_SGIR the following values:
	 * [25:24] = 0x0 (forward interrupt to the CPU interfaces specified in CPUTargetList field)
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

/*
 * this function determines if a core is disabled via COREDISR
 * in:  w0  = core_mask_lsb
 * out: w0  = 0, core not disabled
 *      w0 != 0, core disabled
 * uses x0, x1, x2
 */
_soc_ck_disabled:

	/* get base addr of dcfg block */
	ldr	x1, =NXP_DCFG_ADDR

	/* read COREDISR */
	ldr	w1, [x1, #DCFG_COREDISR_OFFSET]
	rev	w2, w1

	/* test core bit */
	and	w0, w2, w0
	ret

/*
 * this function resets the system via SoC-specific methods
 * in:  none
 * out: none
 * uses x0, x1, x2, x3
 */
_soc_sys_reset:

	ldr	x2, =NXP_DCFG_ADDR

	/* make sure the mask is cleared in the reset request mask register */
	mov	w1, wzr
	str	w1, [x2, #DCFG_RSTRQMR1_OFFSET]

	/* x2 = NXP_DCFG_ADDR */

	/* set the reset request */
	ldr	w1, =RSTCR_RESET_REQ
	ldr	x3, =DCFG_RSTCR_OFFSET
	rev	w0, w1
	str	w0, [x2, x3]

	/* x2 = NXP_DCFG_ADDR */
	/* x3 = DCFG_RSTCR_OFFSET */

	/* just in case this address range is mapped as cacheable,
	 * flush the write out of the dcaches */
	add	x3, x2, x3
	dc	cvac, x3
	dsb	st
	isb

	/* Note: this function does not return */
1:
	wfi
	b  1b


/*
 * part of SYSTEM_OFF
 * this function turns off the SoC clocks
 * Note: this function is not intended to return, and the only allowable
 *       recovery is POR
 * in:  none
 * out: none
 * uses x0 ~ x8
 */
_soc_sys_off:

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

	/* set WFIL2_EN in SCFG_COREPMCR */
	ldr	x0, =SCFG_COREPMCR_OFFSET
	ldr	x1, =COREPMCR_WFIL2
	bl	write_reg_scfg

	/* set OVRD_EN in RCPM2_POWMGTDCR */
	ldr	x0, =RCPM2_POWMGTDCR_OFFSET
	ldr	x1, =POWMGTDCR_OVRD_EN
	bl	write_reg_rcpm2

	/* read IPPDEXPCR0 @ RCPM_IPPDEXPCR0 */
	ldr	x0, =RCPM_IPPDEXPCR0_OFFSET
	bl	read_reg_rcpm
	mov	x7, x0

	/* build an override mask for IPSTPCR4/IPSTPACK4/DEVDISR5 */
	mov	x5, xzr
	ldr	x6, =IPPDEXPCR_MASK2
	and	x6, x6, x7
	cbz	x6, 1f

	/* x5 = override mask
	 * x6 = IPPDEXPCR bits for DEVDISR5
	 * x7 = IPPDEXPCR */

	/* get the overrides */
	orr	x4, x5, #DEVDISR5_I2C_1
	tst	x6, #IPPDEXPCR_I2C1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR5_LPUART1
	tst	x6, #IPPDEXPCR_LPUART1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR5_FLX_TMR
	tst	x6, #IPPDEXPCR_FLX_TMR1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR5_OCRAM1
	tst	x6, #IPPDEXPCR_OCRAM1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR5_GPIO
	tst	x6, #IPPDEXPCR_GPIO1
	csel	x5, x5, x4, EQ
1:
	/* store the DEVDISR5 override mask */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA
	str	w5, [x2, #DEVDISR5_MASK_OFFSET]

	/* build an override mask for IPSTPCR1/IPSTPACK1/DEVDISR2 */
	mov	x5, xzr
	ldr	x6, =IPPDEXPCR_MASK1
	and	x6, x6, x7
	cbz	x6, 2f

	/* x5 = override mask */
	/* x6 = IPPDEXPCR bits for DEVDISR2 */

	/* get the overrides */
	orr	x4, x5, #DEVDISR2_FMAN1_MAC1
	tst	x6, #IPPDEXPCR_MAC1_1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC2
	tst	x6, #IPPDEXPCR_MAC1_2
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC3
	tst	x6, #IPPDEXPCR_MAC1_3
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC4
	tst	x6, #IPPDEXPCR_MAC1_4
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC5
	tst	x6, #IPPDEXPCR_MAC1_5
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC6
	tst	x6, #IPPDEXPCR_MAC1_6
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC9
	tst	x6, #IPPDEXPCR_MAC1_9
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1
	tst	x6, #IPPDEXPCR_FM1
	csel	x5, x5, x4, EQ

2:
	/* store the DEVDISR2 override mask */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA
	str	w5, [x2, #DEVDISR2_MASK_OFFSET]

	/* x5 = DEVDISR2 override mask */

	/* write IPSTPCR0 - no overrides */
	ldr	x0, =RCPM2_IPSTPCR0_OFFSET
	ldr	x1, =IPSTPCR0_VALUE
	bl	write_reg_rcpm2

	/* x5 = DEVDISR2 override mask */

	/* write IPSTPCR1 - overrides possible */
	ldr	x0, =RCPM2_IPSTPCR1_OFFSET
	ldr	x1, =IPSTPCR1_VALUE
	bic	x1, x1, x5
	bl	write_reg_rcpm2

	/* write IPSTPCR2 - no overrides */
	ldr	x0, =RCPM2_IPSTPCR2_OFFSET
	ldr	x1, =IPSTPCR2_VALUE
	bl	write_reg_rcpm2

	/* write IPSTPCR3 - no overrides */
	ldr	x0, =RCPM2_IPSTPCR3_OFFSET
	ldr	x1, =IPSTPCR3_VALUE
	bl	write_reg_rcpm2

	/* write IPSTPCR4 - overrides possible */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA
	ldr	w6, [x2, #DEVDISR5_MASK_OFFSET]
	ldr	x0, =RCPM2_IPSTPCR4_OFFSET
	ldr	x1, =IPSTPCR4_VALUE
	bic	x1, x1, x6
	bl	write_reg_rcpm2

	/* x5 = DEVDISR2 override mask */
	/* x6 = DEVDISR5 override mask */

	/* poll on IPSTPACK0 */
	ldr	x3, =RCPM2_IPSTPACKR0_OFFSET
	ldr	x4, =IPSTPCR0_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
3:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	14f
	sub	x7, x7, #1
	cbnz	x7, 3b

14:
	/* poll on IPSTPACK1 */
	ldr	x3, =IPSTPCR1_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
	bic	x4, x3, x5
	ldr	x3, =RCPM2_IPSTPACKR1_OFFSET
4:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	15f
	sub	x7, x7, #1
	cbnz	x7, 4b

15:
	/* poll on IPSTPACK2 */
	ldr	x3, =RCPM2_IPSTPACKR2_OFFSET
	ldr	x4, =IPSTPCR2_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
5:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	16f
	sub	x7, x7, #1
	cbnz	x7, 5b

16:
	/* poll on IPSTPACK3 */
	ldr	x3, =RCPM2_IPSTPACKR3_OFFSET
	ldr	x4, =IPSTPCR3_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
6:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	17f
	sub	x7, x7, #1
	cbnz	x7, 6b

17:
	/* poll on IPSTPACK4 */
	ldr	x3, =IPSTPCR4_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
	bic	x4, x3, x6
	ldr	x3, =RCPM2_IPSTPACKR4_OFFSET
7:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	18f
	sub	x7, x7, #1
	cbnz	x7, 7b

18:
	ldr	x7, =BC_PSCI_BASE
	add	x7, x7, #AUX_01_DATA

	/* x5 = DEVDISR2 override mask
	 * x6 = DEVDISR5 override mask
	 * x7 = [soc_data_area] */

	/* DEVDISR1 - load new value */
	mov	x0, #DCFG_DEVDISR1_OFFSET
	bl	read_reg_dcfg
	mov	x0, #DCFG_DEVDISR1_OFFSET
	ldr	x1, =DEVDISR1_VALUE
	bl	write_reg_dcfg

	/* DEVDISR2 - load new value */
	mov	x0, #DCFG_DEVDISR2_OFFSET
	bl	read_reg_dcfg
	mov	x0, #DCFG_DEVDISR2_OFFSET
	ldr	x1, =DEVDISR2_VALUE
	bic	x1, x1, x5
	bl	write_reg_dcfg

	/* x6 = DEVDISR5 override mask */
	/* x7 = [soc_data_area] */

	/* DEVDISR3 - load new value */
	mov	x0, #DCFG_DEVDISR3_OFFSET
	bl	read_reg_dcfg
	mov	x0, #DCFG_DEVDISR3_OFFSET
	ldr	x1, =DEVDISR3_VALUE
	bl	write_reg_dcfg

	/* DEVDISR4 - load new value */
	mov	x0, #DCFG_DEVDISR4_OFFSET
	bl	read_reg_dcfg
	mov	x0, #DCFG_DEVDISR4_OFFSET
	ldr	x1, =DEVDISR4_VALUE
	bl	write_reg_dcfg

	/* DEVDISR5 - load new value */
	mov	x0, #DCFG_DEVDISR5_OFFSET
	bl	read_reg_dcfg
	mov	x0, #DCFG_DEVDISR5_OFFSET
	ldr	x1, =DEVDISR5_VALUE
	bic	x1, x1, x6
	bl	write_reg_dcfg

	/* x7 = [soc_data_area] */

	/* disable data prefetch */
	mrs	x0, CPUACTLR_EL1
	bic	x0, x0, #CPUACTLR_L1PCTL_MASK
	msr	CPUACTLR_EL1, x0

	/* x6 = DEVDISR5 override mask */

	/* setup registers for cache-only execution */
	ldr	x5, =IPSTPCR4_VALUE
	bic	x5, x5, x6
	mov	x6, #DDR_CNTRL_BASE_ADDR
	mov	x7, #DCSR_RCPM2_BASE
	mov	x8, #NXP_DCFG_ADDR
	dsb	sy
	isb

	/* set the DLL_LOCK cycle count */
	ldr	w1, [x6, #DDR_TIMING_CFG_4_OFFSET]
	rev	w2, w1
	bic	w2, w2, #DLL_LOCK_MASK
	orr	w2, w2, #DLL_LOCK_VALUE
	rev	w1, w2
	str	w1, [x6, #DDR_TIMING_CFG_4_OFFSET]

	/* x5  = ipstpcr4 (IPSTPCR4_VALUE bic DEVDISR5_MASK)
	 * x6  = DDR_CNTRL_BASE_ADDR
	 * x7  = DCSR_RCPM2_BASE
	 * x8  = NXP_DCFG_ADDR */

	/* enter the cache-only sequence - there is no return */
	b	final_shutdown


/*
 * part of CPU_OFF
 * this function programs SoC & GIC registers in preparation for shutting down
 * the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0 ~ x7
 */
_soc_core_prep_off:
	mov	x7, x30
	mov	x6, x0

	/* make sure the smpen bit is set */
	mrs	x2, CORTEX_A53_ECTLR_EL1
	orr	x2, x2, #CPUECTLR_SMPEN_MASK
	msr	CORTEX_A53_ECTLR_EL1, x2
	isb

	/* configure the cpu interface */

	/* disable signaling of ints */
	bl	_getGICC_BaseAddr  // 0-1
	mov	x4, x0

	ldr	w3, [x4, #GICC_CTLR_OFFSET]
	bic	w3, w3, #GICC_CTLR_EN_GRP0
	bic	w3, w3, #GICC_CTLR_EN_GRP1
	str	w3, [x4, #GICC_CTLR_OFFSET]
	dsb	sy
	isb

	/*
	 * x3 = GICC_CTRL
	 * x4 = GICC_BASE_ADDR
	 * x6 = core mask
	 */

	/* set the priority filter */
	ldr	w2, [x4, #GICC_PMR_OFFSET]
	orr	w2, w2, #GICC_PMR_FILTER
	str	w2, [x4, #GICC_PMR_OFFSET]

	/* setup GICC_CTLR */
	bic	w3, w3, #GICC_CTLR_ACKCTL_MASK
	orr	w3, w3, #GICC_CTLR_FIQ_EN_MASK
	orr	w3, w3, #GICC_CTLR_EOImodeS_MASK
	orr	w3, w3, #GICC_CTLR_CBPR_MASK
	str	w3, [x4, #GICC_CTLR_OFFSET]

	/* x3 = GICC_CTRL */
	/* x4 = GICC_BASE_ADDR */

	/* setup the banked-per-core GICD registers */
	bl	_getGICD_BaseAddr

	/*
	 * x0 = GICD_BASE_ADDR
	 * x3 = GICC_CTRL
	 * x4 = GICC_BASE_ADDR
	 * x6 = core mask
	 */

	/* define SGI15 as Grp0 */
	ldr	w2, [x0, #GICD_IGROUPR0_OFFSET]
	bic	w2, w2, #GICD_IGROUP0_SGI15
	str	w2, [x0, #GICD_IGROUPR0_OFFSET]

	/* set priority of SGI 15 to highest... */
	ldr	w2, [x0, #GICD_IPRIORITYR3_OFFSET]
	bic	w2, w2, #GICD_IPRIORITY_SGI15_MASK
	str	w2, [x0, #GICD_IPRIORITYR3_OFFSET]

	/* enable SGI 15 */
	ldr	w2, [x0, #GICD_ISENABLER0_OFFSET]
	orr	w2, w2, #GICD_ISENABLE0_SGI15
	str	w2, [x0, #GICD_ISENABLER0_OFFSET]

	/* enable the cpu interface */
	orr	w3, w3, #GICC_CTLR_EN_GRP0
	str	w3, [x4, #GICC_CTLR_OFFSET]

	/* x0 = GICD_BASE_ADDR
	 * x6 = core mask */

	/* clear any pending SGIs */
	add	x0, x0, #GICD_CPENDSGIR3_OFFSET
	ldr	x2, =GICD_CPENDSGIR_CLR_MASK
	str	w2, [x0]

	dsb	sy
	isb
	mov	x30, x7
	ret

/*
 * part of CPU_OFF
 * this function performs the final steps to shutdown the core
 * in:  x0 = core mask lsb
 * out: none
 * uses x0 ~ x5
 */
_soc_core_entr_off:
	mov	x5, x30
	mov	x4, x0

	bl	_getGICD_BaseAddr
	mov	x3, x0

	/* x3 = GICD_BASE_ADDR */
	/* x4 = core mask (lsb) */

3:
	/* enter low-power state by executing wfi */
	wfi

	/* x3 = GICD_BASE_ADDR */
	/* x4 = core mask (lsb) */

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

	/* x0 = core state */
	cmp	x0, #CORE_WAKEUP
	b.ne	3b

	/* if we get here, then we have exited the wfi */
	dsb	sy
	isb
	mov	x30, x5
	ret

/*
 * part of CPU_OFF
 * this function starts the process of starting a core back up
 * in:  x0 = core mask lsb
 * out: none
 * uses x0 ~ x5
 */
_soc_core_exit_off:
	mov	x5, x30
	mov	x4, x0

	/* x4 = core mask */

	bl	_getGICC_BaseAddr
	mov	x2, x0

	/* read GICC_IAR */
	ldr	w0, [x2, #GICC_IAR_OFFSET]

	/* write GICC_EIOR - signal end-of-interrupt */
	str	w0, [x2, #GICC_EOIR_OFFSET]

	/* write GICC_DIR - disable interrupt */
	str	w0, [x2, #GICC_DIR_OFFSET]

	/* x2 = GICC_BASE_ADDR */

	/* disable signaling of grp0 ints */
	ldr	w1, [x2, #GICC_CTLR_OFFSET]
	bic	w1, w1, #GICC_CTLR_EN_GRP0
	str	w1, [x2, #GICC_CTLR_OFFSET]

	dsb	sy
	isb
	mov	x30, x5
	ret

/*
 * this function loads a 64-bit execution address of the core in the soc registers
 * BOOTLOCPTRL/H
 * in:  x0, 64-bit address to write to BOOTLOCPTRL/H
 * uses x0, x1, x2, x3
 */
_soc_set_start_addr:
	/* get the 64-bit base address of the scfg block */
	ldr	x2, =NXP_SCFG_ADDR

	/* write the 32-bit BOOTLOCPTRL register (offset 0x604 in the scfg block) */
	mov	x1, x0
	rev	w3, w1
	str	w3, [x2, #SCFG_BOOTLOCPTRL_OFFSET]

	/* write the 32-bit BOOTLOCPTRH register (offset 0x600 in the scfg block) */
	lsr	x1, x0, #32
	rev	w3, w1
	str	w3, [x2, #SCFG_BOOTLOCPTRH_OFFSET]
	ret

/*
 * part of CPU_SUSPEND
 * this function puts the calling core into standby state
 * in:  x0 = core mask lsb
 * out: none
 * uses x0
 */
_soc_core_entr_stdby:
	dsb	sy
	isb
	wfi

	ret

/*
 * part of CPU_SUSPEND
 * this function performs SoC-specific programming prior to standby
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_core_prep_stdby:
	/* clear CORTEX_A53_ECTLR_EL1[2:0] */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	bic	x1, x1, #CPUECTLR_TIMER_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1

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
 * uses x0, x1
 */
_soc_core_prep_pwrdn:
	/* make sure the smp bit is set */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	orr	x1, x1, #CPUECTLR_SMPEN_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1
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
	dsb	sy
	isb
	wfi

	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses none
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
	/* clear CORTEX_A53_ECTLR_EL1[2:0] */
	mrs  x1, CORTEX_A53_ECTLR_EL1
	bic  x1, x1, #CPUECTLR_TIMER_MASK
	msr  CORTEX_A53_ECTLR_EL1, x1

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
 * uses x0, x1
 */
_soc_clstr_prep_pwrdn:
	/* make sure the smp bit is set */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	orr	x1, x1, #CPUECTLR_SMPEN_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1
	isb

	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses none
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
	/* clear CORTEX_A53_ECTLR_EL1[2:0] */
	mrs  x1, CORTEX_A53_ECTLR_EL1
	bic  x1, x1, #CPUECTLR_TIMER_MASK
	msr  CORTEX_A53_ECTLR_EL1, x1

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
 * uses x0, x1, x2, x3, x4
 */
_soc_sys_prep_pwrdn:
	mov	x4, x30
	/* make sure the smp bit is set */
	mrs	x1, CORTEX_A53_ECTLR_EL1
	orr	x1, x1, #CPUECTLR_SMPEN_MASK
	msr	CORTEX_A53_ECTLR_EL1, x1
	isb

	/* set WFIL2_EN in SCFG_COREPMCR */
	ldr	x0, =SCFG_COREPMCR_OFFSET
	ldr	x1, =COREPMCR_WFIL2
	bl	write_reg_scfg  // 0-3

	/* set OVRD_EN in RCPM2_POWMGTDCR */
	ldr	x0, =RCPM2_POWMGTDCR_OFFSET
	ldr	x1, =POWMGTDCR_OVRD_EN
	bl	write_reg_rcpm2  // 0-3

	mov	x30, x4
	ret
/*
 * part of CPU_SUSPEND
 * this function puts the calling core, and potentially the soc, into a
 * low-power state
 * in:  x0 = core mask lsb
 * out: x0 = 0, success
 *      x0 < 0, failure
 * uses x0 ~ x9
 */
_soc_sys_pwrdn_wfi:
	mov	x18, x30

	/* read IPPDEXPCR0 @ RCPM_IPPDEXPCR0 */
	ldr	x0, =RCPM_IPPDEXPCR0_OFFSET
	bl	read_reg_rcpm
	mov	x7, x0

	/* build an override mask for IPSTPCR4/IPSTPACK4/DEVDISR5 */
	mov	x5, xzr
	ldr	x6, =IPPDEXPCR_MASK2
	and	x6, x6, x7
	cbz	x6, 1f

	/* x5 = override mask
	 * x6 = IPPDEXPCR bits for DEVDISR5
	 * x7 = IPPDEXPCR */

	/* get the overrides */
	orr	x4, x5, #DEVDISR5_I2C_1
	tst	x6, #IPPDEXPCR_I2C1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR5_LPUART1
	tst	x6, #IPPDEXPCR_LPUART1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR5_FLX_TMR
	tst	x6, #IPPDEXPCR_FLX_TMR1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR5_OCRAM1
	tst	x6, #IPPDEXPCR_OCRAM1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR5_GPIO
	tst	x6, #IPPDEXPCR_GPIO1
	csel	x5, x5, x4, EQ
1:
	/* store the DEVDISR5 override mask */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA
	str	w5, [x2, #DEVDISR5_MASK_OFFSET]

	/* build an override mask for IPSTPCR1/IPSTPACK1/DEVDISR2 */
	mov	x5, xzr
	ldr	x6, =IPPDEXPCR_MASK1
	and	x6, x6, x7
	cbz	x6, 2f

	/* x5 = override mask */
	/* x6 = IPPDEXPCR bits for DEVDISR2 */

	/* get the overrides */
	orr	x4, x5, #DEVDISR2_FMAN1_MAC1
	tst	x6, #IPPDEXPCR_MAC1_1
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC2
	tst	x6, #IPPDEXPCR_MAC1_2
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC3
	tst	x6, #IPPDEXPCR_MAC1_3
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC4
	tst	x6, #IPPDEXPCR_MAC1_4
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC5
	tst	x6, #IPPDEXPCR_MAC1_5
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC6
	tst	x6, #IPPDEXPCR_MAC1_6
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1_MAC9
	tst	x6, #IPPDEXPCR_MAC1_9
	csel	x5, x5, x4, EQ

	orr	x4, x5, #DEVDISR2_FMAN1
	tst	x6, #IPPDEXPCR_FM1
	csel	x5, x5, x4, EQ

2:
	/* store the DEVDISR2 override mask */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA
	str	w5, [x2, #DEVDISR2_MASK_OFFSET]

	/* x5 = DEVDISR2 override mask */

	/* write IPSTPCR0 - no overrides */
	ldr	x0, =RCPM2_IPSTPCR0_OFFSET
	ldr	x1, =IPSTPCR0_VALUE
	bl	write_reg_rcpm2

	/* x5 = DEVDISR2 override mask */

	/* write IPSTPCR1 - overrides possible */
	ldr	x0, =RCPM2_IPSTPCR1_OFFSET
	ldr	x1, =IPSTPCR1_VALUE
	bic	x1, x1, x5
	bl	write_reg_rcpm2

	/* write IPSTPCR2 - no overrides */
	ldr	x0, =RCPM2_IPSTPCR2_OFFSET
	ldr	x1, =IPSTPCR2_VALUE
	bl	write_reg_rcpm2

	/* write IPSTPCR3 - no overrides */
	ldr	x0, =RCPM2_IPSTPCR3_OFFSET
	ldr	x1, =IPSTPCR3_VALUE
	bl	write_reg_rcpm2

	/* write IPSTPCR4 - overrides possible */
	ldr	x2, =BC_PSCI_BASE
	add	x2, x2, #AUX_01_DATA
	ldr	w6, [x2, #DEVDISR5_MASK_OFFSET]
	ldr	x0, =RCPM2_IPSTPCR4_OFFSET
	ldr	x1, =IPSTPCR4_VALUE
	bic	x1, x1, x6
	bl	write_reg_rcpm2

	/* x5 = DEVDISR2 override mask */
	/* x6 = DEVDISR5 override mask */

	/* poll on IPSTPACK0 */
	ldr	x3, =RCPM2_IPSTPACKR0_OFFSET
	ldr	x4, =IPSTPCR0_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
3:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	14f
	sub	x7, x7, #1
	cbnz	x7, 3b

14:
	/* poll on IPSTPACK1 */
	ldr	x3, =IPSTPCR1_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
	bic	x4, x3, x5
	ldr	x3, =RCPM2_IPSTPACKR1_OFFSET
4:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	15f
	sub	x7, x7, #1
	cbnz	x7, 4b

15:
	/* poll on IPSTPACK2 */
	ldr	x3, =RCPM2_IPSTPACKR2_OFFSET
	ldr	x4, =IPSTPCR2_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
5:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	16f
	sub	x7, x7, #1
	cbnz	x7, 5b

16:
	/* poll on IPSTPACK3 */
	ldr	x3, =RCPM2_IPSTPACKR3_OFFSET
	ldr	x4, =IPSTPCR3_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
6:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	17f
	sub	x7, x7, #1
	cbnz	x7, 6b

17:
	/* poll on IPSTPACK4 */
	ldr	x3, =IPSTPCR4_VALUE
	ldr	x7, =IPSTPACK_RETRY_CNT
	bic	x4, x3, x6
	ldr	x3, =RCPM2_IPSTPACKR4_OFFSET
7:
	mov	x0, x3
	bl	read_reg_rcpm2
	cmp	x0, x4
	b.eq	18f
	sub	x7, x7, #1
	cbnz	x7, 7b

18:
	ldr	x7, =BC_PSCI_BASE
	add	x7, x7, #AUX_01_DATA

	/* x5 = DEVDISR2 override mask
	 * x6 = DEVDISR5 override mask
	 * x7 = [soc_data_area] */

	/* save DEVDISR1 and load new value */
	mov	x0, #DCFG_DEVDISR1_OFFSET
	bl	read_reg_dcfg
	mov	w13, w0
	mov	x0, #DCFG_DEVDISR1_OFFSET
	ldr	x1, =DEVDISR1_VALUE
	bl	write_reg_dcfg

	/* save DEVDISR2 and load new value */
	mov	x0, #DCFG_DEVDISR2_OFFSET
	bl	read_reg_dcfg
	mov	w14, w0
	mov	x0, #DCFG_DEVDISR2_OFFSET
	ldr	x1, =DEVDISR2_VALUE
	bic	x1, x1, x5
	bl	write_reg_dcfg

	/* x6 = DEVDISR5 override mask */
	/* x7 = [soc_data_area] */

	/* save DEVDISR3 and load new value */
	mov	x0, #DCFG_DEVDISR3_OFFSET
	bl	read_reg_dcfg
	mov	w15, w0
	mov	x0, #DCFG_DEVDISR3_OFFSET
	ldr	x1, =DEVDISR3_VALUE
	bl	write_reg_dcfg

	/* save DEVDISR4 and load new value */
	mov	x0, #DCFG_DEVDISR4_OFFSET
	bl	read_reg_dcfg
	mov	w16, w0
	mov	x0, #DCFG_DEVDISR4_OFFSET
	ldr	x1, =DEVDISR4_VALUE
	bl	write_reg_dcfg

	/* save DEVDISR5 and load new value */
	mov	x0, #DCFG_DEVDISR5_OFFSET
	bl	read_reg_dcfg
	mov	w17, w0
	mov	x0, #DCFG_DEVDISR5_OFFSET
	ldr	x1, =DEVDISR5_VALUE
	bic	x1, x1, x6
	bl	write_reg_dcfg

	/* x7 = [soc_data_area] */

	/* save cpuactlr and disable data prefetch */
	mrs	x0, CPUACTLR_EL1
	str	w0, [x7, #CPUACTLR_DATA_OFFSET]
	bic	x0, x0, #CPUACTLR_L1PCTL_MASK
	msr	CPUACTLR_EL1, x0

	/* x6 = DEVDISR5 override mask */

	/* setup registers for cache-only execution */
	ldr	x5, =IPSTPCR4_VALUE
	bic	x5, x5, x6
	mov	x6, #DDR_CNTRL_BASE_ADDR
	mov	x7, #DCSR_RCPM2_BASE
	mov	x8, #NXP_DCFG_ADDR
	dsb sy
	isb

	/* set the DLL_LOCK cycle count */
	ldr	w1, [x6, #DDR_TIMING_CFG_4_OFFSET]
	rev	w2, w1
	bic	w2, w2, #DLL_LOCK_MASK
	orr	w2, w2, #DLL_LOCK_VALUE
	rev	w1, w2
	str	w1, [x6, #DDR_TIMING_CFG_4_OFFSET]

	/*
	 * x5  = ipstpcr4 (IPSTPCR4_VALUE bic DEVDISR5_MASK)
	 * x6  = DDR_CNTRL_BASE_ADDR
	 * x7  = DCSR_RCPM2_BASE
	 * x8  = NXP_DCFG_ADDR
	 * w13 = DEVDISR1 saved value
	 * w14 = DEVDISR2 saved value
	 * w15 = DEVDISR3 saved value
	 * w16 = DEVDISR4 saved value
	 * w17 = DEVDISR5 saved value
	 */

	/* enter the cache-only sequence */
	mov	x9, #CORE_RESTARTABLE
	bl	final_pwrdown

	/* when we are here, the core has come out of wfi and the SoC is back up */

	mov  x30, x18
	ret

/*
 * part of CPU_SUSPEND
 * this function performs any SoC-specific cleanup after power-down
 * in:  x0 = core mask lsb
 * out: none
 * uses x0, x1
 */
_soc_sys_exit_pwrdn:
	/* clear POWMGTDCR */
	mov	x1, #DCSR_RCPM2_BASE
	str	wzr, [x1, #RCPM2_POWMGTDCR_OFFSET]

	/* clear WFIL2_EN in SCFG_COREPMCR */
	mov	x1, #NXP_SCFG_ADDR
	str	wzr, [x1, #SCFG_COREPMCR_OFFSET]

	ret

/*
 * write a register in the SCFG block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
write_reg_scfg:
	ldr	x2, =NXP_SCFG_ADDR
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
	ret
/*
 * read a register in the SCFG block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
read_reg_scfg:
	ldr	x2, =NXP_SCFG_ADDR
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret

/*
 * write a register in the DCFG block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
write_reg_dcfg:
	ldr	x2, =NXP_DCFG_ADDR
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
	ret

/*
 * read a register in the DCFG block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
read_reg_dcfg:
	ldr	x2, =NXP_DCFG_ADDR
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret

/*
 * write a register in the RCPM block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
write_reg_rcpm:
	ldr	x2, =NXP_RCPM_ADDR
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
    ret

/*
 * read a register in the RCPM block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
read_reg_rcpm:
	ldr	x2, =NXP_RCPM_ADDR
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret

/*
 * write a register in the DCSR-RCPM2 block
 * in:  x0 = offset
 * in:  w1 = value to write
 * uses x0, x1, x2, x3
 */
write_reg_rcpm2:
	ldr	x2, =DCSR_RCPM2_BASE
	/* swap for BE */
	rev	w3, w1
	str	w3, [x2, x0]
	ret

/*
 * read a register in the DCSR-RCPM2 block
 * in:  x0 = offset
 * out: w0 = value read
 * uses x0, x1, x2
 */
read_reg_rcpm2:
	ldr	x2, =DCSR_RCPM2_BASE
	ldr	w1, [x2, x0]
	/* swap for BE */
	rev	w0, w1
	ret

/*
 * this function returns the base address of the gic distributor
 * in:  none
 * out: x0 = base address of gic distributor
 * uses x0, x1
 */
_getGICD_BaseAddr:
	/* read SVR and get the SoC version */
	mov	x0, #NXP_DCFG_ADDR
	ldr	w1, [x0, #DCFG_SVR_OFFSET]
	rev	w0, w1

	/* x0 =  svr */
	and	w0, w0, #SVR_MIN_VER_MASK
	cmp	w0, #SVR_MINOR_VER_0
	b.ne	8f

	/* load the gic base addresses for rev 1.0 parts */
	ldr	x0, =NXP_GICD_4K_ADDR
	b	10f
8:
	/* for rev 1.1 and later parts, the GIC base addresses */
	/* can be at 4k or 64k offsets */

	/* read the scfg reg GIC400_ADDR_ALIGN */
	mov	x0, #NXP_SCFG_ADDR
	ldr	w1, [x0, #SCFG_GIC400_ADDR_ALIGN_OFFSET]
	rev	w0, w1

	/* x0 = GIC400_ADDR_ALIGN value */
	and	x0, x0, #SCFG_GIC400_ADDR_ALIGN_4KMODE_MASK
	mov	x1, #SCFG_GIC400_ADDR_ALIGN_4KMODE_EN
	cmp	x0, x1
	b.ne	9f

	/* load the base addresses for 4k offsets */
	ldr	x0, =NXP_GICD_4K_ADDR
	b	10f
9:
	/* load the base address for 64k offsets */
	ldr	x0, =NXP_GICD_64K_ADDR
10:
	ret

/*
 * this function returns the base address of the gic distributor
 * in:  none
 * out: x0 = base address of gic controller
 * uses x0, x1
 */
_getGICC_BaseAddr:
	/* read SVR and get the SoC version */
	mov	x0, #NXP_DCFG_ADDR
	ldr	w1, [x0, #DCFG_SVR_OFFSET]
	rev	w0, w1

	/* x0 =  svr */
	and	w0, w0, #SVR_MIN_VER_MASK
	cmp	w0, #SVR_MINOR_VER_0
	b.ne	8f

	/* load the gic base addresses for rev 1.0 parts */
	ldr	x0, =NXP_GICC_4K_ADDR
	b	10f
8:
	/* for rev 1.1 and later parts, the GIC base addresses */
	/* can be at 4k or 64k offsets */

	/* read the scfg reg GIC400_ADDR_ALIGN */
	mov	x0, #NXP_SCFG_ADDR
	ldr	w1, [x0, #SCFG_GIC400_ADDR_ALIGN_OFFSET]
	rev	w0, w1

	/* x0 = GIC400_ADDR_ALIGN value */
	and	x0, x0, #SCFG_GIC400_ADDR_ALIGN_4KMODE_MASK
	mov	x1, #SCFG_GIC400_ADDR_ALIGN_4KMODE_EN
	cmp	x0, x1
	b.ne	9f

	/* load the base addresses for 4k offsets */
	ldr	x0, =NXP_GICC_4K_ADDR
	b	10f
9:
	/* load the base address for 64k offsets */
	ldr	x0, =NXP_GICC_64K_ADDR
10:
	ret

/*
 * this function will pwrdown ddr and the final core - it will do this
 * by loading itself into the icache and then executing from there
 * in:  x5  = ipstpcr4 (IPSTPCR4_VALUE bic DEVDISR5_MASK)
 *      x6  = DDR_CNTRL_BASE_ADDR
 *      x7  = DCSR_RCPM2_BASE
 *      x8  = NXP_DCFG_ADDR
 *      x9  = 0, restartable
 *          = 1, non-restartable
 *      w13 = DEVDISR1 saved value
 *      w14 = DEVDISR2 saved value
 *      w15 = DEVDISR3 saved value
 *      w16 = DEVDISR4 saved value
 *      w17 = DEVDISR5 saved value
 * out: none
 * uses x0 ~ x9
 */

/* 4Kb aligned */
.align 12
final_pwrdown:
	mov	x0, xzr
	b	touch_line_0
start_line_0:
	mov	x0, #1
	mov	x2, #DDR_SDRAM_CFG_2_FRCSR         /* put ddr in self refresh - start */
	ldr	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	rev	w4, w3
	orr	w4, w4, w2
	rev	w3, w4
	str	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]  /* put ddr in self refresh - end */
	orr	w3, w5, #DEVDISR5_MEM              /* quiesce ddr clocks - start */
	rev	w4, w3
	str	w4, [x7, #RCPM2_IPSTPCR4_OFFSET]   /* quiesce ddr clocks - end */

	mov	w3, #DEVDISR5_MEM
	rev	w3, w3                             /* polling mask */
	mov	x2, #DDR_SLEEP_RETRY_CNT           /* poll on ipstpack4 - start */
touch_line_0:
	cbz	x0,	touch_line_1

start_line_1:
	ldr	w1, [x7, #RCPM2_IPSTPACKR4_OFFSET]
	tst	w1, w3
	b.ne	1f
	subs	x2, x2, #1
	b.gt	start_line_1                       /* poll on ipstpack4 - end */

	/* if we get here, we have a timeout err */
	rev 	w4, w5
	str	w4, [x7, #RCPM2_IPSTPCR4_OFFSET]   /* re-enable ddr clks interface */
	mov	x0, #ERROR_DDR_SLEEP               /* load error code */
	b	2f
1:
	str	w4, [x8, #DCFG_DEVDISR5_OFFSET]    /* disable ddr cntrlr clk in devdisr5 */
5:
	wfi                                     /* stop the final core */

	cbnz	x9, 5b                             /* if non-restartable, keep in wfi */
	rev	w4, w5
	str	w4, [x8, #DCFG_DEVDISR5_OFFSET]    /* re-enable ddr in devdisr5 */
	str	w4, [x7, #RCPM2_IPSTPCR4_OFFSET]   /* re-enable ddr clk in ipstpcr4 */
touch_line_1:
	cbz	x0, touch_line_2

start_line_2:
	ldr	w1, [x7, #RCPM2_IPSTPACKR4_OFFSET] /* poll on ipstpack4 - start */
	tst	w1, w3
	b.eq	2f
	nop
	b	start_line_2                       /* poll on ipstpack4 - end */
2:
	mov	x2, #DDR_SDRAM_CFG_2_FRCSR         /* take ddr out-of self refresh - start */
	ldr	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	rev	w4, w3
	bic	w4, w4, w2
	rev	w3, w4
	mov	x1, #DDR_SLEEP_RETRY_CNT           /* wait for ddr cntrlr clock - start */
3:
	subs	x1, x1, #1
	b.gt	3b                                 /* wait for ddr cntrlr clock - end */
	str	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]  /* take ddr out-of self refresh - end */
	rev	w1, w17
touch_line_2:
	cbz	x0, touch_line_3

start_line_3:
	str	w1, [x8, #DCFG_DEVDISR5_OFFSET]    /* reset devdisr5 */
	rev	w1, w16
	str	w1, [x8, #DCFG_DEVDISR4_OFFSET]    /* reset devdisr4 */
	rev	w1, w15
	str	w1, [x8, #DCFG_DEVDISR3_OFFSET]    /* reset devdisr3 */
	rev	w1, w14
	str	w1, [x8, #DCFG_DEVDISR2_OFFSET]    /* reset devdisr2 */
	rev	w1, w13
	str	w1, [x8, #DCFG_DEVDISR1_OFFSET]    /* reset devdisr1 */
	str	wzr, [x7, #RCPM2_IPSTPCR4_OFFSET]  /* reset ipstpcr4 */
	str	wzr, [x7, #RCPM2_IPSTPCR3_OFFSET]  /* reset ipstpcr3 */
	str	wzr, [x7, #RCPM2_IPSTPCR2_OFFSET]  /* reset ipstpcr2 */
	str	wzr, [x7, #RCPM2_IPSTPCR1_OFFSET]  /* reset ipstpcr1 */
	str	wzr, [x7, #RCPM2_IPSTPCR0_OFFSET]  /* reset ipstpcr0 */
	b	continue_restart
touch_line_3:
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

/*
 * Note: there is no return from this function
 * this function will shutdown ddr and the final core - it will do this
 * by loading itself into the icache and then executing from there
 * in:  x5  = ipstpcr4 (IPSTPCR4_VALUE bic DEVDISR5_MASK)
 *      x6  = DDR_CNTRL_BASE_ADDR
 *      x7  = DCSR_RCPM2_BASE
 *      x8  = NXP_DCFG_ADDR
 * out: none
 * uses x0 ~ x8
 */

/* 4Kb aligned */
.align 12
final_shutdown:

	mov	x0, xzr
	b	touch_line0
start_line0:
	mov	x0, #1
	mov	x2, #DDR_SDRAM_CFG_2_FRCSR         /* put ddr in self refresh - start */
	ldr	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]
	rev	w4, w3
	orr	w4, w4, w2
	rev	w3, w4
	str	w3, [x6, #DDR_SDRAM_CFG_2_OFFSET]  /* put ddr in self refresh - end */
	orr	w3, w5, #DEVDISR5_MEM              /* quiesce ddr clocks - start */
	rev	w4, w3
	str	w4, [x7, #RCPM2_IPSTPCR4_OFFSET]   /* quiesce ddr clocks - end */

	mov	w3, #DEVDISR5_MEM
	rev	w3, w3                             /* polling mask */
	mov	x2, #DDR_SLEEP_RETRY_CNT           /* poll on ipstpack4 - start */
touch_line0:
	cbz  x0, touch_line1

start_line1:
	ldr	w1, [x7, #RCPM2_IPSTPACKR4_OFFSET]
	tst	w1, w3
	b.ne	1f
	subs	x2, x2, #1
	b.gt	start_line1                       /* poll on ipstpack4 - end */
	nop
	nop
	nop
	nop
1:
	str	w4, [x8, #DCFG_DEVDISR5_OFFSET]    /* disable ddr cntrlr clk in devdisr5 */
5:
	wfi                                     /* stop the final core */
	b	5b                                 /* stay here until POR */
	nop
	nop
	nop
touch_line1:
	cbz	x0, start_line0
