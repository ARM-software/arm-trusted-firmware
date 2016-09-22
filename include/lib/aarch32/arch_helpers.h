/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __ARCH_HELPERS_H__
#define __ARCH_HELPERS_H__

#include <arch.h>	/* for additional register definitions */
#include <stdint.h>
#include <types.h>

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
 *  64 bit `mrrc` and `mcrr` instructions. It works only on Little Endian
 *  systems for GCC versions < 4.6. Above GCC 4.6, both Little Endian and
 *  Big Endian systems generate the right instruction encoding.
 */
#if !(__GNUC__ > (4) || __GNUC__ == (4) && __GNUC_MINOR__ >= (6))
#error "GCC 4.6 or above is required to build AArch32 Trusted Firmware"
#endif

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

/* Define read & write function for coproc register */
#define DEFINE_COPROCR_RW_FUNCS(_name, ...) 				\
	_DEFINE_COPROCR_READ_FUNC(_name, __VA_ARGS__)			\
	_DEFINE_COPROCR_WRITE_FUNC(_name, __VA_ARGS__)

/* Define 64 bit read function for coproc register */
#define DEFINE_COPROCR_READ_FUNC_64(_name, ...) 			\
	_DEFINE_COPROCR_READ_FUNC_64(_name, __VA_ARGS__)

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
	__asm__ (#_op " " #_type);					\
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

void disable_mmu_secure(void);
void disable_mmu_icache_secure(void);

DEFINE_SYSOP_FUNC(wfi)
DEFINE_SYSOP_FUNC(wfe)
DEFINE_SYSOP_FUNC(sev)
DEFINE_SYSOP_TYPE_FUNC(dsb, sy)
DEFINE_SYSOP_TYPE_FUNC(dmb, sy)
DEFINE_SYSOP_TYPE_FUNC(dsb, ish)
DEFINE_SYSOP_TYPE_FUNC(dmb, ish)
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
DEFINE_COPROCR_READ_FUNC(id_pfr1, ID_PFR1)
DEFINE_COPROCR_READ_FUNC(isr, ISR)
DEFINE_COPROCR_READ_FUNC(clidr, CLIDR)
DEFINE_COPROCR_READ_FUNC_64(cntpct, CNTPCT_64)

DEFINE_COPROCR_RW_FUNCS(scr, SCR)
DEFINE_COPROCR_RW_FUNCS(ctr, CTR)
DEFINE_COPROCR_RW_FUNCS(sctlr, SCTLR)
DEFINE_COPROCR_RW_FUNCS(hsctlr, HSCTLR)
DEFINE_COPROCR_RW_FUNCS(hcr, HCR)
DEFINE_COPROCR_RW_FUNCS(hcptr, HCPTR)
DEFINE_COPROCR_RW_FUNCS(cntfrq, CNTFRQ)
DEFINE_COPROCR_RW_FUNCS(cnthctl, CNTHCTL)
DEFINE_COPROCR_RW_FUNCS(mair0, MAIR0)
DEFINE_COPROCR_RW_FUNCS(mair1, MAIR1)
DEFINE_COPROCR_RW_FUNCS(ttbcr, TTBCR)
DEFINE_COPROCR_RW_FUNCS(ttbr0, TTBR0)
DEFINE_COPROCR_RW_FUNCS_64(ttbr0, TTBR0_64)
DEFINE_COPROCR_RW_FUNCS(ttbr1, TTBR1)
DEFINE_COPROCR_RW_FUNCS(vpidr, VPIDR)
DEFINE_COPROCR_RW_FUNCS(vmpidr, VMPIDR)
DEFINE_COPROCR_RW_FUNCS_64(vttbr, VTTBR_64)
DEFINE_COPROCR_RW_FUNCS_64(ttbr1, TTBR1_64)
DEFINE_COPROCR_RW_FUNCS_64(cntvoff, CNTVOFF_64)
DEFINE_COPROCR_RW_FUNCS(csselr, CSSELR)

DEFINE_COPROCR_RW_FUNCS(icc_sre_el1, ICC_SRE)
DEFINE_COPROCR_RW_FUNCS(icc_sre_el2, ICC_HSRE)
DEFINE_COPROCR_RW_FUNCS(icc_sre_el3, ICC_MSRE)
DEFINE_COPROCR_RW_FUNCS(icc_pmr_el1, ICC_PMR)
DEFINE_COPROCR_RW_FUNCS(icc_igrpen1_el3, ICC_MGRPEN1)
DEFINE_COPROCR_RW_FUNCS(icc_igrpen0_el1, ICC_IGRPEN0)
DEFINE_COPROCR_RW_FUNCS(icc_hppir0_el1, ICC_HPPIR0)
DEFINE_COPROCR_RW_FUNCS(icc_hppir1_el1, ICC_HPPIR1)
DEFINE_COPROCR_RW_FUNCS(icc_iar0_el1, ICC_IAR0)
DEFINE_COPROCR_RW_FUNCS(icc_iar1_el1, ICC_IAR1)
DEFINE_COPROCR_RW_FUNCS(icc_eoir0_el1, ICC_EOIR0)
DEFINE_COPROCR_RW_FUNCS(icc_eoir1_el1, ICC_EOIR1)

/*
 * TLBI operation prototypes
 */
DEFINE_TLBIOP_FUNC(all, TLBIALL)
DEFINE_TLBIOP_FUNC(allis, TLBIALLIS)
DEFINE_TLBIOP_PARAM_FUNC(mva, TLBIMVA)
DEFINE_TLBIOP_PARAM_FUNC(mvaa, TLBIMVAA)

/*
 * DC operation prototypes
 */
DEFINE_DCOP_PARAM_FUNC(civac, DCCIMVAC)
DEFINE_DCOP_PARAM_FUNC(ivac, DCIMVAC)
DEFINE_DCOP_PARAM_FUNC(cvac, DCCMVAC)

/* Previously defined accessor functions with incomplete register names  */
#define dsb()			dsbsy()

#define IS_IN_SECURE() \
	(GET_NS_BIT(read_scr()) == 0)

 /*
  * If EL3 is AArch32, then secure PL1 and monitor mode correspond to EL3
  */
#define IS_IN_EL3() \
	((GET_M32(read_cpsr()) == MODE32_mon) ||	\
		(IS_IN_SECURE() && (GET_M32(read_cpsr()) != MODE32_usr)))

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

#endif /* __ARCH_HELPERS_H__ */
