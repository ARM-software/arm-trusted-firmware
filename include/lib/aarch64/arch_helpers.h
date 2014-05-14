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

#include <cdefs.h> /* For __dead2 */


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
void tlbialle1(void);
void tlbialle1is(void);
void tlbialle2(void);
void tlbialle2is(void);
void tlbialle3(void);
void tlbialle3is(void);
void tlbivmalle1(void);

/*******************************************************************************
 * Cache maintenance accessor prototypes
 ******************************************************************************/
void dcisw(unsigned long);
void dccisw(unsigned long);
void dccsw(unsigned long);
void dccvac(unsigned long);
void dcivac(unsigned long);
void dccivac(unsigned long);
void dccvau(unsigned long);
void dczva(unsigned long);
void flush_dcache_range(unsigned long, unsigned long);
void inv_dcache_range(unsigned long, unsigned long);
void dcsw_op_louis(unsigned int);
void dcsw_op_all(unsigned int);

void disable_mmu_el3(void);
void disable_mmu_icache_el3(void);

/*******************************************************************************
 * Misc. accessor prototypes
 ******************************************************************************/
void enable_irq(void);
void enable_fiq(void);
void enable_serror(void);
void enable_debug_exceptions(void);

void disable_irq(void);
void disable_fiq(void);
void disable_serror(void);
void disable_debug_exceptions(void);

unsigned long read_id_pfr1_el1(void);
unsigned long read_id_aa64pfr0_el1(void);
unsigned long read_current_el(void);
unsigned long read_daif(void);
unsigned long read_spsr_el1(void);
unsigned long read_spsr_el2(void);
unsigned long read_spsr_el3(void);
unsigned long read_elr_el1(void);
unsigned long read_elr_el2(void);
unsigned long read_elr_el3(void);

void write_daif(unsigned long);
void write_spsr_el1(unsigned long);
void write_spsr_el2(unsigned long);
void write_spsr_el3(unsigned long);
void write_elr_el1(unsigned long);
void write_elr_el2(unsigned long);
void write_elr_el3(unsigned long);

void wfi(void);
void wfe(void);
void rfe(void);
void sev(void);
void dsb(void);
void isb(void);

unsigned int get_afflvl_shift(unsigned int);
unsigned int mpidr_mask_lower_afflvls(unsigned long, unsigned int);

void __dead2 eret(unsigned long, unsigned long,
		unsigned long, unsigned long,
		unsigned long, unsigned long,
		unsigned long, unsigned long);

void __dead2 smc(unsigned long, unsigned long,
		unsigned long, unsigned long,
		unsigned long, unsigned long,
		unsigned long, unsigned long);

/*******************************************************************************
 * System register accessor prototypes
 ******************************************************************************/
unsigned long read_midr(void);
unsigned long read_mpidr(void);

unsigned long read_scr(void);
unsigned long read_hcr(void);

unsigned long read_vbar_el1(void);
unsigned long read_vbar_el2(void);
unsigned long read_vbar_el3(void);

unsigned long read_sctlr_el1(void);
unsigned long read_sctlr_el2(void);
unsigned long read_sctlr_el3(void);

unsigned long read_actlr_el1(void);
unsigned long read_actlr_el2(void);
unsigned long read_actlr_el3(void);

unsigned long read_esr_el1(void);
unsigned long read_esr_el2(void);
unsigned long read_esr_el3(void);

unsigned long read_afsr0_el1(void);
unsigned long read_afsr0_el2(void);
unsigned long read_afsr0_el3(void);

unsigned long read_afsr1_el1(void);
unsigned long read_afsr1_el2(void);
unsigned long read_afsr1_el3(void);

unsigned long read_far_el1(void);
unsigned long read_far_el2(void);
unsigned long read_far_el3(void);

unsigned long read_mair_el1(void);
unsigned long read_mair_el2(void);
unsigned long read_mair_el3(void);

unsigned long read_amair_el1(void);
unsigned long read_amair_el2(void);
unsigned long read_amair_el3(void);

unsigned long read_rvbar_el1(void);
unsigned long read_rvbar_el2(void);
unsigned long read_rvbar_el3(void);

unsigned long read_rmr_el1(void);
unsigned long read_rmr_el2(void);
unsigned long read_rmr_el3(void);

unsigned long read_tcr_el1(void);
unsigned long read_tcr_el2(void);
unsigned long read_tcr_el3(void);

unsigned long read_ttbr0_el1(void);
unsigned long read_ttbr0_el2(void);
unsigned long read_ttbr0_el3(void);

unsigned long read_ttbr1_el1(void);

unsigned long read_cptr_el2(void);
unsigned long read_cptr_el3(void);

unsigned long read_cpacr(void);
unsigned long read_cpuectlr(void);
unsigned int read_cntfrq_el0(void);
unsigned long read_cnthctl_el2(void);

void write_scr(unsigned long);
void write_hcr(unsigned long);
void write_cpacr(unsigned long);
void write_cntfrq_el0(unsigned int);
void write_cnthctl_el2(unsigned long);

void write_vbar_el1(unsigned long);
void write_vbar_el2(unsigned long);
void write_vbar_el3(unsigned long);

void write_sctlr_el1(unsigned long);
void write_sctlr_el2(unsigned long);
void write_sctlr_el3(unsigned long);

void write_actlr_el1(unsigned long);
void write_actlr_el2(unsigned long);
void write_actlr_el3(unsigned long);

void write_esr_el1(unsigned long);
void write_esr_el2(unsigned long);
void write_esr_el3(unsigned long);

void write_afsr0_el1(unsigned long);
void write_afsr0_el2(unsigned long);
void write_afsr0_el3(unsigned long);

void write_afsr1_el1(unsigned long);
void write_afsr1_el2(unsigned long);
void write_afsr1_el3(unsigned long);

void write_far_el1(unsigned long);
void write_far_el2(unsigned long);
void write_far_el3(unsigned long);

void write_mair_el1(unsigned long);
void write_mair_el2(unsigned long);
void write_mair_el3(unsigned long);

void write_amair_el1(unsigned long);
void write_amair_el2(unsigned long);
void write_amair_el3(unsigned long);

void write_rmr_el1(unsigned long);
void write_rmr_el2(unsigned long);
void write_rmr_el3(unsigned long);

void write_tcr_el1(unsigned long);
void write_tcr_el2(unsigned long);
void write_tcr_el3(unsigned long);

void write_ttbr0_el1(unsigned long);
void write_ttbr0_el2(unsigned long);
void write_ttbr0_el3(unsigned long);

void write_ttbr1_el1(unsigned long);

void write_cpuectlr(unsigned long);
void write_cptr_el2(unsigned long);
void write_cptr_el3(unsigned long);

#define IS_IN_EL(x) \
	(GET_EL(read_current_el()) == MODE_EL##x)

#define IS_IN_EL1() IS_IN_EL(1)
#define IS_IN_EL3() IS_IN_EL(3)

#endif /* __ARCH_HELPERS_H__ */
