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

#include <arch.h>

#ifndef __ASSEMBLY__
#include <stdio.h>

/*******************************************************************************
 * Aarch64 translation tables manipulation helper prototypes
 ******************************************************************************/
extern unsigned long create_table_desc(unsigned long *next_table_ptr);
extern unsigned long create_block_desc(unsigned long desc,
				       unsigned long addr,
				       unsigned int level);
extern unsigned long create_device_block(unsigned long output_addr,
					 unsigned int level,
					 unsigned int ns);
extern unsigned long create_romem_block(unsigned long output_addr,
					unsigned int level,
					unsigned int ns);
extern unsigned long create_rwmem_block(unsigned long output_addr,
					unsigned int level,
					unsigned int ns);

/*******************************************************************************
 * TLB maintenance accessor prototypes
 ******************************************************************************/
extern void tlbialle1(void);
extern void tlbialle1is(void);
extern void tlbialle2(void);
extern void tlbialle2is(void);
extern void tlbialle3(void);
extern void tlbialle3is(void);
extern void tlbivmalle1(void);

/*******************************************************************************
 * Cache maintenance accessor prototypes
 ******************************************************************************/
extern void dcisw(unsigned long);
extern void dccisw(unsigned long);
extern void dccsw(unsigned long);
extern void dccvac(unsigned long);
extern void dcivac(unsigned long);
extern void dccivac(unsigned long);
extern void dccvau(unsigned long);
extern void dczva(unsigned long);
extern void flush_dcache_range(unsigned long, unsigned long);
extern void inv_dcache_range(unsigned long, unsigned long);
extern void dcsw_op_louis(unsigned int);
extern void dcsw_op_all(unsigned int);

/*******************************************************************************
 * Misc. accessor prototypes
 ******************************************************************************/
extern void enable_irq(void);
extern void enable_fiq(void);
extern void enable_serror(void);
extern void enable_debug_exceptions(void);

extern void disable_irq(void);
extern void disable_fiq(void);
extern void disable_serror(void);
extern void disable_debug_exceptions(void);

extern unsigned long read_id_pfr1_el1(void);
extern unsigned long read_id_aa64pfr0_el1(void);
extern unsigned long read_current_el(void);
extern unsigned long read_daif(void);
extern unsigned long read_spsr_el1(void);
extern unsigned long read_spsr_el2(void);
extern unsigned long read_spsr_el3(void);
extern unsigned long read_elr_el1(void);
extern unsigned long read_elr_el2(void);
extern unsigned long read_elr_el3(void);

extern void write_daif(unsigned long);
extern void write_spsr_el1(unsigned long);
extern void write_spsr_el2(unsigned long);
extern void write_spsr_el3(unsigned long);
extern void write_elr_el1(unsigned long);
extern void write_elr_el2(unsigned long);
extern void write_elr_el3(unsigned long);

extern void wfi(void);
extern void wfe(void);
extern void rfe(void);
extern void sev(void);
extern void dsb(void);
extern void isb(void);

extern unsigned int get_afflvl_shift(unsigned int);
extern unsigned int mpidr_mask_lower_afflvls(unsigned long, unsigned int);

extern void __dead2 eret(unsigned long, unsigned long,
			 unsigned long, unsigned long,
			 unsigned long, unsigned long,
			 unsigned long, unsigned long);

extern void __dead2 smc(unsigned long, unsigned long,
			 unsigned long, unsigned long,
			 unsigned long, unsigned long,
			  unsigned long, unsigned long);

/*******************************************************************************
 * System register accessor prototypes
 ******************************************************************************/
extern unsigned long read_midr(void);
extern unsigned long read_mpidr(void);

extern unsigned long read_scr(void);
extern unsigned long read_hcr(void);

extern unsigned long read_vbar_el1(void);
extern unsigned long read_vbar_el2(void);
extern unsigned long read_vbar_el3(void);

extern unsigned long read_sctlr_el1(void);
extern unsigned long read_sctlr_el2(void);
extern unsigned long read_sctlr_el3(void);

extern unsigned long read_actlr_el1(void);
extern unsigned long read_actlr_el2(void);
extern unsigned long read_actlr_el3(void);

extern unsigned long read_esr_el1(void);
extern unsigned long read_esr_el2(void);
extern unsigned long read_esr_el3(void);

extern unsigned long read_afsr0_el1(void);
extern unsigned long read_afsr0_el2(void);
extern unsigned long read_afsr0_el3(void);

extern unsigned long read_afsr1_el1(void);
extern unsigned long read_afsr1_el2(void);
extern unsigned long read_afsr1_el3(void);

extern unsigned long read_far_el1(void);
extern unsigned long read_far_el2(void);
extern unsigned long read_far_el3(void);

extern unsigned long read_mair_el1(void);
extern unsigned long read_mair_el2(void);
extern unsigned long read_mair_el3(void);

extern unsigned long read_amair_el1(void);
extern unsigned long read_amair_el2(void);
extern unsigned long read_amair_el3(void);

extern unsigned long read_rvbar_el1(void);
extern unsigned long read_rvbar_el2(void);
extern unsigned long read_rvbar_el3(void);

extern unsigned long read_rmr_el1(void);
extern unsigned long read_rmr_el2(void);
extern unsigned long read_rmr_el3(void);

extern unsigned long read_tcr_el1(void);
extern unsigned long read_tcr_el2(void);
extern unsigned long read_tcr_el3(void);

extern unsigned long read_ttbr0_el1(void);
extern unsigned long read_ttbr0_el2(void);
extern unsigned long read_ttbr0_el3(void);

extern unsigned long read_ttbr1(void);
extern unsigned long read_ttbr1_el1(void);
extern unsigned long read_ttbr1_el2(void);

extern unsigned long read_cptr_el2(void);
extern unsigned long read_cptr_el3(void);

extern unsigned long read_cpacr(void);
extern unsigned long read_cpuectlr(void);
extern unsigned int read_cntfrq_el0(void);
extern unsigned long read_cnthctl_el2(void);

extern void write_scr(unsigned long);
extern void write_hcr(unsigned long);
extern void write_cpacr(unsigned long);
extern void write_cntfrq_el0(unsigned int);
extern void write_cnthctl_el2(unsigned long);

extern void write_vbar_el1(unsigned long);
extern void write_vbar_el2(unsigned long);
extern void write_vbar_el3(unsigned long);

extern void write_sctlr_el1(unsigned long);
extern void write_sctlr_el2(unsigned long);
extern void write_sctlr_el3(unsigned long);

extern void write_actlr_el1(unsigned long);
extern void write_actlr_el2(unsigned long);
extern void write_actlr_el3(unsigned long);

extern void write_esr_el1(unsigned long);
extern void write_esr_el2(unsigned long);
extern void write_esr_el3(unsigned long);

extern void write_afsr0(unsigned long);
extern void write_afsr0_el1(unsigned long);
extern void write_afsr0_el2(unsigned long);
extern void write_afsr0_el3(unsigned long);

extern void write_afsr1(unsigned long);
extern void write_afsr1_el1(unsigned long);
extern void write_afsr1_el2(unsigned long);
extern void write_afsr1_el3(unsigned long);

extern void write_far_el1(unsigned long);
extern void write_far_el2(unsigned long);
extern void write_far_el3(unsigned long);

extern void write_mair_el1(unsigned long);
extern void write_mair_el2(unsigned long);
extern void write_mair_el3(unsigned long);

extern void write_amair_el1(unsigned long);
extern void write_amair_el2(unsigned long);
extern void write_amair_el3(unsigned long);

extern void write_rmr_el1(unsigned long);
extern void write_rmr_el2(unsigned long);
extern void write_rmr_el3(unsigned long);

extern void write_tcr_el1(unsigned long);
extern void write_tcr_el2(unsigned long);
extern void write_tcr_el3(unsigned long);

extern void write_ttbr0_el1(unsigned long);
extern void write_ttbr0_el2(unsigned long);
extern void write_ttbr0_el3(unsigned long);

extern void write_ttbr1_el1(unsigned long);
extern void write_ttbr1_el2(unsigned long);

extern void write_cpuectlr(unsigned long);
extern void write_cptr_el2(unsigned long);
extern void write_cptr_el3(unsigned long);

#endif /*__ASSEMBLY__*/

#endif /* __ARCH_HELPERS_H__ */
