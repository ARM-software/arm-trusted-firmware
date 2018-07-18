/*
 * Copyright (c) 2016-2017, Cavium Inc. All rights reserved.<BR>
 * Copyright (c) 2018     , Facebook, Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <bakery_lock.h>
#include <mmio.h>
#include <thunder_pwrc.h>
#include <thunder_private.h>
#include <arch_helpers.h>
#include <debug.h>

unsigned int thunder_pwrc_get_cpu_wkr(unsigned long mpidr)
{
	unsigned int rc = 0;
	return rc;
}

unsigned int thunder_pwrc_read_psysr(unsigned long mpidr)
{
	unsigned int rc = 0;
	return rc;
}

int thunder_wait_for_core(unsigned node)
{
	int loop=10;
	volatile union cavm_rst_pp_pending pp_pending;

	/* TODO: This might be a good chance to implement
	 * Simple timer library.
	 **/

	/* give core chance to come up */
	while (loop) {
		__asm__ __volatile__ ("udiv xzr, xzr,xzr");
		__asm__ __volatile__ ("udiv xzr, xzr,xzr");
		__asm__ __volatile__ ("udiv xzr, xzr,xzr");
		__asm__ __volatile__ ("udiv xzr, xzr,xzr");
		loop --;
	}

	loop =1000000;
	while (loop) {
		pp_pending.u = CSR_READ_PA(node, CAVM_RST_PP_PENDING);

		if(!pp_pending.s.pend)
			break;
		__asm__ __volatile__ ("udiv xzr, xzr,xzr");
		__asm__ __volatile__ ("udiv xzr, xzr,xzr");
		loop--;
	}
	if (loop == 0)
		return 1;
	return 0;

}

void thunder_pwrc_write_pponr(unsigned long mpidr)
{
	union cavm_rst_pp_reset pp_reset;
	unsigned long node, aff1_id, aff0_id, cavm_core_id;

	node = ((mpidr >> MPIDR_AFF2_SHIFT) & MPIDR_AFFLVL_MASK);
	aff1_id = ((mpidr >> MPIDR_AFF1_SHIFT) & MPIDR_AFFLVL_MASK);
	aff0_id = ((mpidr >> MPIDR_AFF0_SHIFT) & MPIDR_AFFLVL_MASK);
	cavm_core_id = (aff1_id * 16) + aff0_id;

	pp_reset.u = CSR_READ_PA(node, CAVM_RST_PP_RESET);

	if (!(pp_reset.u & (1ul << cavm_core_id))) {
		/* core is WFI suspended state
		 * Need to reset it by writing 1 to RST_PP_RESET and then
		 * clearing it.
		 **/
		pp_reset.u |= (1ul << cavm_core_id);
		CSR_WRITE_PA(node, CAVM_RST_PP_RESET, pp_reset.u);
		__asm("dsb ishst");
		__asm("sev");
		if (thunder_wait_for_core(node)) {
			WARN("Failed to release core:%lu on node:%lu\n ",
					cavm_core_id,node);
			while(1);
			return;
		}
		pp_reset.u = CSR_READ_PA(node, CAVM_RST_PP_RESET);
	}
	pp_reset.u &= ~(1ul << cavm_core_id);
	CSR_WRITE_PA(node, CAVM_RST_PP_RESET, pp_reset.u);
	__asm("dsb ishst");
	__asm("sev");
	if (thunder_wait_for_core(node)){
		WARN("Failed to release core:%lu on node:%lu\n ",
				cavm_core_id,node);
	}
}

void thunder_pwrc_write_ppoffr(unsigned long mpidr)
{

}

void thunder_pwrc_set_wen(unsigned long mpidr)
{

}

void thunder_pwrc_clr_wen(unsigned long mpidr)
{

}

void thunder_pwrc_write_pcoffr(unsigned long mpidr)
{

}

extern void cavium_secure_entrypoint(void);

int thunder_pwrc_setup(void)
{
	set_secondary_cpu_jump_addr((unsigned long)&cavium_secure_entrypoint);
	return 0;
}
