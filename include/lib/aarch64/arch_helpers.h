/*
 * Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
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

#include <arch.h>	/* for CPUECTLR_EL1 */
#include <cdefs.h>	/* For __dead2 */
#include <stdint.h>

/*******************************************************************************
 * Aarch64 translation tables manipulation helper prototypes
******************************************************************************/
unsigned long create_table_desc(unsigned long *next_table_ptr);
unsigned long create_block_desc(unsigned long desc,
				unsigned long addr,
				unsigned int level);
unsigned long create_device_block(unsigned long output_addr,
				unsigned int level,
				unsigned int ns);
unsigned long create_romem_block(unsigned long output_addr,
				unsigned int level,
				unsigned int ns);
unsigned long create_rwmem_block(unsigned long output_addr,
				unsigned int level,
				unsigned int ns);

/*******************************************************************************
 * TLB maintenance accessor prototypes
 ******************************************************************************/
SYSTEM_OP2(tlbi, alle1)
SYSTEM_OP2(tlbi, alle1is)
SYSTEM_OP2(tlbi, alle2)
SYSTEM_OP2(tlbi, alle2is)
SYSTEM_OP2(tlbi, alle3)
SYSTEM_OP2(tlbi, alle3is)
SYSTEM_OP2(tlbi, vmalle1)

/*******************************************************************************
 * Cache maintenance accessor prototypes
 ******************************************************************************/
SYSTEM_OP2_P(dc, isw)
SYSTEM_OP2_P(dc, cisw)
SYSTEM_OP2_P(dc, csw)
SYSTEM_OP2_P(dc, cvac)
SYSTEM_OP2_P(dc, ivac)
SYSTEM_OP2_P(dc, civac)
SYSTEM_OP2_P(dc, cvau)
SYSTEM_OP2_P(dc, zva)

void flush_dcache_range(unsigned long, unsigned long);
void inv_dcache_range(unsigned long, unsigned long);
void dcsw_op_louis(unsigned int);
void dcsw_op_all(unsigned int);

void disable_mmu_el3(void);
void disable_mmu_icache_el3(void);

/*******************************************************************************
 * Misc. accessor prototypes
 ******************************************************************************/

SYSTEM_REG_WC(daifset)
SYSTEM_REG_WC(daifclr)

#define enable_irq()			write_daifclr(DAIF_IRQ_BIT)
#define enable_fiq()			write_daifclr(DAIF_FIQ_BIT)
#define enable_serror()			write_daifclr(DAIF_ABT_BIT)
#define enable_debug_exceptions()	write_daifclr(DAIF_DBG_BIT)
#define disable_irq()			write_daifset(DAIF_IRQ_BIT)
#define disable_fiq()			write_daifset(DAIF_FIQ_BIT)
#define disable_serror()		write_daifset(DAIF_ABT_BIT)
#define disable_debug_exceptions()	write_daifset(DAIF_DBG_BIT)

SYSTEM_REG_RO(id_pfr1_el1)
SYSTEM_REG_RO(id_aa64pfr0_el1)
SYSTEM_REG_RO(CurrentEl)
SYSTEM_REG(daif)
SYSTEM_REG(spsr_el1)
SYSTEM_REG(spsr_el2)
SYSTEM_REG(spsr_el3)
SYSTEM_REG(elr_el1)
SYSTEM_REG(elr_el2)
SYSTEM_REG(elr_el3)

SYSTEM_OP1(wfi)
SYSTEM_OP1(wfe)
SYSTEM_OP1(sev)
SYSTEM_OP2(dsb, sy)
SYSTEM_OP1(isb)

unsigned int get_afflvl_shift(unsigned int);
unsigned int mpidr_mask_lower_afflvls(unsigned long, unsigned int);


void __dead2 eret(uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3,
		  uint64_t x4, uint64_t x5, uint64_t x6, uint64_t x7);
void __dead2 smc(uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3,
		 uint64_t x4, uint64_t x5, uint64_t x6, uint64_t x7);

/*******************************************************************************
 * System register accessor prototypes
 ******************************************************************************/
SYSTEM_REG_RO(midr_el1)
SYSTEM_REG_RO(mpidr_el1)

SYSTEM_REG(scr_el3)
SYSTEM_REG(hcr_el2)

SYSTEM_REG(vbar_el1)
SYSTEM_REG(vbar_el2)
SYSTEM_REG(vbar_el3)

SYSTEM_REG(sctlr_el1)
SYSTEM_REG(sctlr_el2)
SYSTEM_REG(sctlr_el3)

SYSTEM_REG(actlr_el1)
SYSTEM_REG(actlr_el2)
SYSTEM_REG(actlr_el3)

SYSTEM_REG(esr_el1)
SYSTEM_REG(esr_el2)
SYSTEM_REG(esr_el3)

SYSTEM_REG(afsr0_el1)
SYSTEM_REG(afsr0_el2)
SYSTEM_REG(afsr0_el3)

SYSTEM_REG(afsr1_el1)
SYSTEM_REG(afsr1_el2)
SYSTEM_REG(afsr1_el3)

SYSTEM_REG(far_el1)
SYSTEM_REG(far_el2)
SYSTEM_REG(far_el3)

SYSTEM_REG(mair_el1)
SYSTEM_REG(mair_el2)
SYSTEM_REG(mair_el3)

SYSTEM_REG(amair_el1)
SYSTEM_REG(amair_el2)
SYSTEM_REG(amair_el3)

SYSTEM_REG_RO(rvbar_el1)
SYSTEM_REG_RO(rvbar_el2)
SYSTEM_REG_RO(rvbar_el3)

SYSTEM_REG(rmr_el1)
SYSTEM_REG(rmr_el2)
SYSTEM_REG(rmr_el3)

SYSTEM_REG(tcr_el1)
SYSTEM_REG(tcr_el2)
SYSTEM_REG(tcr_el3)

SYSTEM_REG(ttbr0_el1)
SYSTEM_REG(ttbr0_el2)
SYSTEM_REG(ttbr0_el3)

SYSTEM_REG(ttbr1_el1)

SYSTEM_REG(cptr_el2)
SYSTEM_REG(cptr_el3)

SYSTEM_REG(cpacr_el1)
SYSTEM_REG2(cpuectlr_el1, CPUECTLR_EL1)
SYSTEM_REG(cntfrq_el0)
SYSTEM_REG(cntps_ctl_el1)
SYSTEM_REG(cntps_tval_el1)
SYSTEM_REG(cntps_cval_el1)
SYSTEM_REG_RO(cntpct_el0)
SYSTEM_REG(cnthctl_el2)

SYSTEM_REG(tpidr_el3)

#define IS_IN_EL(x) \
	(GET_EL(read_CurrentEl()) == MODE_EL##x)

#define IS_IN_EL1() IS_IN_EL(1)
#define IS_IN_EL3() IS_IN_EL(3)

/* Some currently in use unsuffixed register accessors */

#define read_current_el()	read_CurrentEl()

#define dsb()			dsbsy()

#define read_midr()		read_midr_el1()

#define read_mpidr()		read_mpidr_el1()

#define read_scr()		read_scr_el3()
#define write_scr(v)		write_scr_el3(v)

#define read_hcr()		read_hcr_el2()
#define write_hcr(v)		write_hcr_el2(v)

#define read_cpuectlr()		read_cpuectlr_el1()
#define write_cpuectlr(v)	write_cpuectlr_el1(v)

#define read_cpacr()		read_cpacr_el1()
#define write_cpacr(v)		write_cpacr_el1(v)

#endif /* __ARCH_HELPERS_H__ */
