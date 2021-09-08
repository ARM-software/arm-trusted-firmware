/*
 * Copyright (c) 2016-2021, ARM Limited and Contributors. All rights reserved.
 * Portions copyright (c) 2021-2022, ProvenRun S.A.S. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_HELPERS_H
#define ARCH_HELPERS_H

#include <cdefs.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <arch.h>

/**********************************************************************
 * Macros which create inline functions to read or write CPU system
 * registers
 *********************************************************************/

#define _DEFINE_COPROCR_WRITE_FUNC(_name, coproc, opc1, CRn, CRm, opc2)	\
static inline void write_## _name(u_register_t v)			\
{									\
	__asm__ volatile ("mcr "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : : "r" (v));\
}

#define _DEFINE_COPROCR_READ_FUNC(_name, coproc, opc1, CRn, CRm, opc2)	\
static inline u_register_t read_ ## _name(void)				\
{									\
	u_register_t v;							\
	__asm__ volatile ("mrc "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : "=r" (v));\
	return v;							\
}

/*
 *  The undocumented %Q and %R extended asm are used to implemented the below
 *  64 bit `mrrc` and `mcrr` instructions.
 */

#define _DEFINE_COPROCR_WRITE_FUNC_64(_name, coproc, opc1, CRm)		\
static inline void write64_## _name(uint64_t v)				\
{									\
	__asm__ volatile ("mcrr "#coproc","#opc1", %Q0, %R0,"#CRm : : "r" (v));\
}

#define _DEFINE_COPROCR_READ_FUNC_64(_name, coproc, opc1, CRm)		\
static inline uint64_t read64_## _name(void)				\
{	uint64_t v;							\
	__asm__ volatile ("mrrc "#coproc","#opc1", %Q0, %R0,"#CRm : "=r" (v));\
	return v;							\
}

#define _DEFINE_SYSREG_READ_FUNC(_name, _reg_name)			\
static inline u_register_t read_ ## _name(void)				\
{									\
	u_register_t v;							\
	__asm__ volatile ("mrs %0, " #_reg_name : "=r" (v));		\
	return v;							\
}

#define _DEFINE_SYSREG_WRITE_FUNC(_name, _reg_name)			\
static inline void write_ ## _name(u_register_t v)			\
{									\
	__asm__ volatile ("msr " #_reg_name ", %0" : : "r" (v));	\
}

#define _DEFINE_SYSREG_WRITE_CONST_FUNC(_name, _reg_name)		\
static inline void write_ ## _name(const u_register_t v)		\
{									\
	__asm__ volatile ("msr " #_reg_name ", %0" : : "i" (v));	\
}

/* Define read function for coproc register */
#define DEFINE_COPROCR_READ_FUNC(_name, ...) 				\
	_DEFINE_COPROCR_READ_FUNC(_name, __VA_ARGS__)

/* Define write function for coproc register */
#define DEFINE_COPROCR_WRITE_FUNC(_name, ...) 				\
	_DEFINE_COPROCR_WRITE_FUNC(_name, __VA_ARGS__)

/* Define read & write function for coproc register */
#define DEFINE_COPROCR_RW_FUNCS(_name, ...) 				\
	_DEFINE_COPROCR_READ_FUNC(_name, __VA_ARGS__)			\
	_DEFINE_COPROCR_WRITE_FUNC(_name, __VA_ARGS__)

/* Define 64 bit read function for coproc register */
#define DEFINE_COPROCR_READ_FUNC_64(_name, ...) 			\
	_DEFINE_COPROCR_READ_FUNC_64(_name, __VA_ARGS__)

/* Define 64 bit write function for coproc register */
#define DEFINE_COPROCR_WRITE_FUNC_64(_name, ...) 			\
	_DEFINE_COPROCR_WRITE_FUNC_64(_name, __VA_ARGS__)

/* Define 64 bit read & write function for coproc register */
#define DEFINE_COPROCR_RW_FUNCS_64(_name, ...) 				\
	_DEFINE_COPROCR_READ_FUNC_64(_name, __VA_ARGS__)		\
	_DEFINE_COPROCR_WRITE_FUNC_64(_name, __VA_ARGS__)

/* Define read & write function for system register */
#define DEFINE_SYSREG_RW_FUNCS(_name)					\
	_DEFINE_SYSREG_READ_FUNC(_name, _name)				\
	_DEFINE_SYSREG_WRITE_FUNC(_name, _name)

/**********************************************************************
 * Macros to create inline functions for tlbi operations
 *********************************************************************/

#define _DEFINE_TLBIOP_FUNC(_op, coproc, opc1, CRn, CRm, opc2)		\
static inline void tlbi##_op(void)					\
{									\
	u_register_t v = 0;						\
	__asm__ volatile ("mcr "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : : "r" (v));\
}

#define _DEFINE_BPIOP_FUNC(_op, coproc, opc1, CRn, CRm, opc2)		\
static inline void bpi##_op(void)					\
{									\
	u_register_t v = 0;						\
	__asm__ volatile ("mcr "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : : "r" (v));\
}

#define _DEFINE_TLBIOP_PARAM_FUNC(_op, coproc, opc1, CRn, CRm, opc2)	\
static inline void tlbi##_op(u_register_t v)				\
{									\
	__asm__ volatile ("mcr "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : : "r" (v));\
}

/* Define function for simple TLBI operation */
#define DEFINE_TLBIOP_FUNC(_op, ...)					\
	_DEFINE_TLBIOP_FUNC(_op, __VA_ARGS__)

/* Define function for TLBI operation with register parameter */
#define DEFINE_TLBIOP_PARAM_FUNC(_op, ...)				\
	_DEFINE_TLBIOP_PARAM_FUNC(_op, __VA_ARGS__)

/* Define function for simple BPI operation */
#define DEFINE_BPIOP_FUNC(_op, ...)					\
	_DEFINE_BPIOP_FUNC(_op, __VA_ARGS__)

/**********************************************************************
 * Macros to create inline functions for DC operations
 *********************************************************************/
#define _DEFINE_DCOP_PARAM_FUNC(_op, coproc, opc1, CRn, CRm, opc2)	\
static inline void dc##_op(u_register_t v)				\
{									\
	__asm__ volatile ("mcr "#coproc","#opc1",%0,"#CRn","#CRm","#opc2 : : "r" (v));\
}

/* Define function for DC operation with register parameter */
#define DEFINE_DCOP_PARAM_FUNC(_op, ...)				\
	_DEFINE_DCOP_PARAM_FUNC(_op, __VA_ARGS__)

/**********************************************************************
 * Macros to create inline functions for system instructions
 *********************************************************************/
 /* Define function for simple system instruction */
#define DEFINE_SYSOP_FUNC(_op)						\
static inline void _op(void)						\
{									\
	__asm__ (#_op);							\
}


/* Define function for system instruction with type specifier */
#define DEFINE_SYSOP_TYPE_FUNC(_op, _type)				\
static inline void _op ## _type(void)					\
{									\
	__asm__ (#_op " " #_type : : : "memory");			\
}

/* Define function for system instruction with register parameter */
#define DEFINE_SYSOP_TYPE_PARAM_FUNC(_op, _type)			\
static inline void _op ## _type(u_register_t v)				\
{									\
	 __asm__ (#_op " " #_type ", %0" : : "r" (v));			\
}

void flush_dcache_range(uintptr_t addr, size_t size);
void clean_dcache_range(uintptr_t addr, size_t size);
void inv_dcache_range(uintptr_t addr, size_t size);
bool is_dcache_enabled(void);

void dcsw_op_louis(u_register_t op_type);
void dcsw_op_all(u_register_t op_type);

void disable_mmu_secure(void);
void disable_mmu_icache_secure(void);

DEFINE_SYSOP_FUNC(wfi)
DEFINE_SYSOP_FUNC(wfe)
DEFINE_SYSOP_FUNC(sev)
DEFINE_SYSOP_TYPE_FUNC(dsb, sy)
DEFINE_SYSOP_TYPE_FUNC(dmb, sy)
DEFINE_SYSOP_TYPE_FUNC(dmb, st)

/* dmb ld is not valid for armv7/thumb machines */
#if ARM_ARCH_MAJOR != 7
DEFINE_SYSOP_TYPE_FUNC(dmb, ld)
#endif

DEFINE_SYSOP_TYPE_FUNC(dsb, ish)
DEFINE_SYSOP_TYPE_FUNC(dsb, ishst)
DEFINE_SYSOP_TYPE_FUNC(dmb, ish)
DEFINE_SYSOP_TYPE_FUNC(dmb, ishst)
DEFINE_SYSOP_FUNC(isb)

void __dead2 smc(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3,
		 uint32_t r4, uint32_t r5, uint32_t r6, uint32_t r7);

DEFINE_SYSREG_RW_FUNCS(spsr)
DEFINE_SYSREG_RW_FUNCS(cpsr)

/*******************************************************************************
 * System register accessor prototypes
 ******************************************************************************/
DEFINE_COPROCR_READ_FUNC(mpidr, MPIDR)
DEFINE_COPROCR_READ_FUNC(midr, MIDR)
DEFINE_COPROCR_READ_FUNC(id_mmfr4, ID_MMFR4)
DEFINE_COPROCR_READ_FUNC(id_dfr0, ID_DFR0)
DEFINE_COPROCR_READ_FUNC(id_pfr0, ID_PFR0)
DEFINE_COPROCR_READ_FUNC(id_pfr1, ID_PFR1)
DEFINE_COPROCR_READ_FUNC(isr, ISR)
DEFINE_COPROCR_READ_FUNC(clidr, CLIDR)
DEFINE_COPROCR_READ_FUNC_64(cntpct, CNTPCT_64)

DEFINE_COPROCR_RW_FUNCS(scr, SCR)
DEFINE_COPROCR_RW_FUNCS(ctr, CTR)
DEFINE_COPROCR_RW_FUNCS(sctlr, SCTLR)
DEFINE_COPROCR_RW_FUNCS(actlr, ACTLR)
DEFINE_COPROCR_RW_FUNCS(hsctlr, HSCTLR)
DEFINE_COPROCR_RW_FUNCS(hcr, HCR)
DEFINE_COPROCR_RW_FUNCS(hcptr, HCPTR)
DEFINE_COPROCR_RW_FUNCS(cntfrq, CNTFRQ)
DEFINE_COPROCR_RW_FUNCS(cnthctl, CNTHCTL)
DEFINE_COPROCR_RW_FUNCS(mair0, MAIR0)
DEFINE_COPROCR_RW_FUNCS(mair1, MAIR1)
DEFINE_COPROCR_RW_FUNCS(hmair0, HMAIR0)
DEFINE_COPROCR_RW_FUNCS(ttbcr, TTBCR)
DEFINE_COPROCR_RW_FUNCS(htcr, HTCR)
DEFINE_COPROCR_RW_FUNCS(ttbr0, TTBR0)
DEFINE_COPROCR_RW_FUNCS_64(ttbr0, TTBR0_64)
DEFINE_COPROCR_RW_FUNCS(ttbr1, TTBR1)
DEFINE_COPROCR_RW_FUNCS_64(httbr, HTTBR_64)
DEFINE_COPROCR_RW_FUNCS(vpidr, VPIDR)
DEFINE_COPROCR_RW_FUNCS(vmpidr, VMPIDR)
DEFINE_COPROCR_RW_FUNCS_64(vttbr, VTTBR_64)
DEFINE_COPROCR_RW_FUNCS_64(ttbr1, TTBR1_64)
DEFINE_COPROCR_RW_FUNCS_64(cntvoff, CNTVOFF_64)
DEFINE_COPROCR_RW_FUNCS(csselr, CSSELR)
DEFINE_COPROCR_RW_FUNCS(hstr, HSTR)
DEFINE_COPROCR_RW_FUNCS(cnthp_ctl_el2, CNTHP_CTL)
DEFINE_COPROCR_RW_FUNCS(cnthp_tval_el2, CNTHP_TVAL)
DEFINE_COPROCR_RW_FUNCS_64(cnthp_cval_el2, CNTHP_CVAL_64)

#define get_cntp_ctl_enable(x)  (((x) >> CNTP_CTL_ENABLE_SHIFT) & \
					 CNTP_CTL_ENABLE_MASK)
#define get_cntp_ctl_imask(x)   (((x) >> CNTP_CTL_IMASK_SHIFT) & \
					 CNTP_CTL_IMASK_MASK)
#define get_cntp_ctl_istatus(x) (((x) >> CNTP_CTL_ISTATUS_SHIFT) & \
					 CNTP_CTL_ISTATUS_MASK)

#define set_cntp_ctl_enable(x)  ((x) |= U(1) << CNTP_CTL_ENABLE_SHIFT)
#define set_cntp_ctl_imask(x)   ((x) |= U(1) << CNTP_CTL_IMASK_SHIFT)

#define clr_cntp_ctl_enable(x)  ((x) &= ~(U(1) << CNTP_CTL_ENABLE_SHIFT))
#define clr_cntp_ctl_imask(x)   ((x) &= ~(U(1) << CNTP_CTL_IMASK_SHIFT))

DEFINE_COPROCR_RW_FUNCS(icc_sre_el1, ICC_SRE)
DEFINE_COPROCR_RW_FUNCS(icc_sre_el2, ICC_HSRE)
DEFINE_COPROCR_RW_FUNCS(icc_sre_el3, ICC_MSRE)
DEFINE_COPROCR_RW_FUNCS(icc_pmr_el1, ICC_PMR)
DEFINE_COPROCR_RW_FUNCS(icc_rpr_el1, ICC_RPR)
DEFINE_COPROCR_RW_FUNCS(icc_igrpen1_el3, ICC_MGRPEN1)
DEFINE_COPROCR_RW_FUNCS(icc_igrpen1_el1, ICC_IGRPEN1)
DEFINE_COPROCR_RW_FUNCS(icc_igrpen0_el1, ICC_IGRPEN0)
DEFINE_COPROCR_RW_FUNCS(icc_hppir0_el1, ICC_HPPIR0)
DEFINE_COPROCR_RW_FUNCS(icc_hppir1_el1, ICC_HPPIR1)
DEFINE_COPROCR_RW_FUNCS(icc_iar0_el1, ICC_IAR0)
DEFINE_COPROCR_RW_FUNCS(icc_iar1_el1, ICC_IAR1)
DEFINE_COPROCR_RW_FUNCS(icc_eoir0_el1, ICC_EOIR0)
DEFINE_COPROCR_RW_FUNCS(icc_eoir1_el1, ICC_EOIR1)
DEFINE_COPROCR_RW_FUNCS_64(icc_sgi0r_el1, ICC_SGI0R_EL1_64)
DEFINE_COPROCR_WRITE_FUNC_64(icc_sgi1r, ICC_SGI1R_EL1_64)
DEFINE_COPROCR_WRITE_FUNC_64(icc_asgi1r, ICC_ASGI1R_EL1_64)

DEFINE_COPROCR_RW_FUNCS(sdcr, SDCR)
DEFINE_COPROCR_RW_FUNCS(hdcr, HDCR)
DEFINE_COPROCR_RW_FUNCS(cnthp_ctl, CNTHP_CTL)
DEFINE_COPROCR_READ_FUNC(pmcr, PMCR)

/*
 * Address translation
 */
DEFINE_COPROCR_WRITE_FUNC(ats1cpr, ATS1CPR)
DEFINE_COPROCR_WRITE_FUNC(ats1hr, ATS1HR)
DEFINE_COPROCR_RW_FUNCS_64(par, PAR_64)

DEFINE_COPROCR_RW_FUNCS(nsacr, NSACR)

/* AArch32 coproc registers for 32bit MMU descriptor support */
DEFINE_COPROCR_RW_FUNCS(prrr, PRRR)
DEFINE_COPROCR_RW_FUNCS(nmrr, NMRR)
DEFINE_COPROCR_RW_FUNCS(dacr, DACR)

/* Coproc registers for 32bit AMU support */
DEFINE_COPROCR_READ_FUNC(amcfgr, AMCFGR)
DEFINE_COPROCR_READ_FUNC(amcgcr, AMCGCR)
DEFINE_COPROCR_RW_FUNCS(amcr, AMCR)

DEFINE_COPROCR_RW_FUNCS(amcntenset0, AMCNTENSET0)
DEFINE_COPROCR_RW_FUNCS(amcntenset1, AMCNTENSET1)
DEFINE_COPROCR_RW_FUNCS(amcntenclr0, AMCNTENCLR0)
DEFINE_COPROCR_RW_FUNCS(amcntenclr1, AMCNTENCLR1)

/* Coproc registers for 64bit AMU support */
DEFINE_COPROCR_RW_FUNCS_64(amevcntr00, AMEVCNTR00)
DEFINE_COPROCR_RW_FUNCS_64(amevcntr01, AMEVCNTR01)
DEFINE_COPROCR_RW_FUNCS_64(amevcntr02, AMEVCNTR02)
DEFINE_COPROCR_RW_FUNCS_64(amevcntr03, AMEVCNTR03)

/*
 * TLBI operation prototypes
 */
DEFINE_TLBIOP_FUNC(all, TLBIALL)
DEFINE_TLBIOP_FUNC(allis, TLBIALLIS)
DEFINE_TLBIOP_PARAM_FUNC(mva, TLBIMVA)
DEFINE_TLBIOP_PARAM_FUNC(mvaa, TLBIMVAA)
DEFINE_TLBIOP_PARAM_FUNC(mvaais, TLBIMVAAIS)
DEFINE_TLBIOP_PARAM_FUNC(mvahis, TLBIMVAHIS)

/*
 * BPI operation prototypes.
 */
DEFINE_BPIOP_FUNC(allis, BPIALLIS)

/*
 * DC operation prototypes
 */
DEFINE_DCOP_PARAM_FUNC(civac, DCCIMVAC)
DEFINE_DCOP_PARAM_FUNC(ivac, DCIMVAC)
#if ERRATA_A53_819472 || ERRATA_A53_824069 || ERRATA_A53_827319
DEFINE_DCOP_PARAM_FUNC(cvac, DCCIMVAC)
#else
DEFINE_DCOP_PARAM_FUNC(cvac, DCCMVAC)
#endif

/*
 * DynamIQ Shared Unit power management
 */
DEFINE_COPROCR_RW_FUNCS(clusterpwrdn, CLUSTERPWRDN)

/* Previously defined accessor functions with incomplete register names  */
#define dsb()			dsbsy()
#define dmb()			dmbsy()

/* dmb ld is not valid for armv7/thumb machines, so alias it to dmb */
#if ARM_ARCH_MAJOR == 7
#define	dmbld()			dmb()
#endif

#define IS_IN_SECURE() \
	(GET_NS_BIT(read_scr()) == 0)

#define IS_IN_HYP()	(GET_M32(read_cpsr()) == MODE32_hyp)
#define IS_IN_SVC()	(GET_M32(read_cpsr()) == MODE32_svc)
#define IS_IN_MON()	(GET_M32(read_cpsr()) == MODE32_mon)
#define IS_IN_EL2()	IS_IN_HYP()
/* If EL3 is AArch32, then secure PL1 and monitor mode correspond to EL3 */
#define IS_IN_EL3() \
	((GET_M32(read_cpsr()) == MODE32_mon) ||	\
		(IS_IN_SECURE() && (GET_M32(read_cpsr()) != MODE32_usr)))

static inline unsigned int get_current_el(void)
{
	if (IS_IN_EL3()) {
		return 3U;
	} else if (IS_IN_EL2()) {
		return 2U;
	} else {
		return 1U;
	}
}

/* Macros for compatibility with AArch64 system registers */
#define read_mpidr_el1()	read_mpidr()

#define read_scr_el3()		read_scr()
#define write_scr_el3(_v)	write_scr(_v)

#define read_hcr_el2()		read_hcr()
#define write_hcr_el2(_v)	write_hcr(_v)

#define read_cpacr_el1()	read_cpacr()
#define write_cpacr_el1(_v)	write_cpacr(_v)

#define read_cntfrq_el0()	read_cntfrq()
#define write_cntfrq_el0(_v)	write_cntfrq(_v)
#define read_isr_el1()		read_isr()

#define read_cntpct_el0()	read64_cntpct()

#define read_ctr_el0()		read_ctr()

#define write_icc_sgi0r_el1(_v)	write64_icc_sgi0r_el1(_v)
#define write_icc_sgi1r(_v)	write64_icc_sgi1r(_v)
#define write_icc_asgi1r(_v)	write64_icc_asgi1r(_v)

#define read_daif()		read_cpsr()
#define write_daif(flags)	write_cpsr(flags)

#define read_cnthp_cval_el2()	read64_cnthp_cval_el2()
#define write_cnthp_cval_el2(v)	write64_cnthp_cval_el2(v)

#define read_amcntenset0_el0()	read_amcntenset0()
#define read_amcntenset1_el0()	read_amcntenset1()

/* Helper functions to manipulate CPSR */
static inline void enable_irq(void)
{
	/*
	 * The compiler memory barrier will prevent the compiler from
	 * scheduling non-volatile memory access after the write to the
	 * register.
	 *
	 * This could happen if some initialization code issues non-volatile
	 * accesses to an area used by an interrupt handler, in the assumption
	 * that it is safe as the interrupts are disabled at the time it does
	 * that (according to program order). However, non-volatile accesses
	 * are not necessarily in program order relatively with volatile inline
	 * assembly statements (and volatile accesses).
	 */
	COMPILER_BARRIER();
	__asm__ volatile ("cpsie	i");
	isb();
}

static inline void enable_serror(void)
{
	COMPILER_BARRIER();
	__asm__ volatile ("cpsie	a");
	isb();
}

static inline void enable_fiq(void)
{
	COMPILER_BARRIER();
	__asm__ volatile ("cpsie	f");
	isb();
}

static inline void disable_irq(void)
{
	COMPILER_BARRIER();
	__asm__ volatile ("cpsid	i");
	isb();
}

static inline void disable_serror(void)
{
	COMPILER_BARRIER();
	__asm__ volatile ("cpsid	a");
	isb();
}

static inline void disable_fiq(void)
{
	COMPILER_BARRIER();
	__asm__ volatile ("cpsid	f");
	isb();
}

#endif /* ARCH_HELPERS_H */
