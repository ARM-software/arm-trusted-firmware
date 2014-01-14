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

#include <arch_helpers.h>
#include <platform.h>
#include <bl_common.h>
#include <bl31.h>
#include <psci.h>
#include <assert.h>
#include <runtime_svc.h>

/*******************************************************************************
 * This function checks whether this is a valid smc e.g.
 * the function id is correct, top word of args are zeroed
 * when aarch64 makes an aarch32 call etc.
 ******************************************************************************/
int validate_smc(gp_regs *regs)
{
	unsigned int rw = GET_RW(regs->spsr);
	unsigned int cc = GET_SMC_CC(regs->x0);

	/* Check if there is a difference in the caller RW and SMC CC */
	if (rw == cc) {

		/* Check whether the caller has chosen the right func. id */
		if (cc == SMC_64) {
			regs->x0 = SMC_UNK;
			return SMC_UNK;
		}

		/*
		 * Paranoid check to zero the top word of passed args
		 * irrespective of caller's register width.
		 *
		 * TODO: Check if this needed if the caller is aarch32
		 */
		regs->x0 &= (unsigned int) 0xFFFFFFFF;
		regs->x1 &= (unsigned int) 0xFFFFFFFF;
		regs->x2 &= (unsigned int) 0xFFFFFFFF;
		regs->x3 &= (unsigned int) 0xFFFFFFFF;
		regs->x4 &= (unsigned int) 0xFFFFFFFF;
		regs->x5 &= (unsigned int) 0xFFFFFFFF;
		regs->x6 &= (unsigned int) 0xFFFFFFFF;
	}

	return 0;
}

/* TODO: Break down the SMC handler into fast and standard SMC handlers. */
void smc_handler(unsigned type, unsigned long esr, gp_regs *regs)
{
	/* Check if the SMC has been correctly called */
	if (validate_smc(regs) != 0)
		return;

	switch (regs->x0) {
	case PSCI_VERSION:
		regs->x0 = psci_version();
		break;

	case PSCI_CPU_OFF:
		regs->x0 = __psci_cpu_off();
		break;

	case PSCI_CPU_SUSPEND_AARCH64:
	case PSCI_CPU_SUSPEND_AARCH32:
		regs->x0 = __psci_cpu_suspend(regs->x1, regs->x2, regs->x3);
		break;

	case PSCI_CPU_ON_AARCH64:
	case PSCI_CPU_ON_AARCH32:
		regs->x0 = psci_cpu_on(regs->x1, regs->x2, regs->x3);
		break;

	case PSCI_AFFINITY_INFO_AARCH32:
	case PSCI_AFFINITY_INFO_AARCH64:
		regs->x0 = psci_affinity_info(regs->x1, regs->x2);
		break;

	default:
		regs->x0 = SMC_UNK;
	}

	return;
}

void irq_handler(unsigned type, unsigned long esr, gp_regs *regs)
{
	plat_report_exception(type);
	assert(0);
}

void fiq_handler(unsigned type, unsigned long esr, gp_regs *regs)
{
	plat_report_exception(type);
	assert(0);
}

void serror_handler(unsigned type, unsigned long esr, gp_regs *regs)
{
	plat_report_exception(type);
	assert(0);
}

void sync_exception_handler(unsigned type, gp_regs *regs)
{
	unsigned long esr = read_esr();
	unsigned int ec = EC_BITS(esr);

	switch (ec) {

	case EC_AARCH32_SMC:
	case EC_AARCH64_SMC:
		smc_handler(type, esr, regs);
		break;

	default:
		plat_report_exception(type);
		assert(0);
	}
	return;
}

void async_exception_handler(unsigned type, gp_regs *regs)
{
	unsigned long esr = read_esr();

	switch (type) {

	case IRQ_SP_EL0:
	case IRQ_SP_ELX:
	case IRQ_AARCH64:
	case IRQ_AARCH32:
		irq_handler(type, esr, regs);
		break;

	case FIQ_SP_EL0:
	case FIQ_SP_ELX:
	case FIQ_AARCH64:
	case FIQ_AARCH32:
		fiq_handler(type, esr, regs);
		break;

	case SERROR_SP_EL0:
	case SERROR_SP_ELX:
	case SERROR_AARCH64:
	case SERROR_AARCH32:
		serror_handler(type, esr, regs);
		break;

	default:
		plat_report_exception(type);
		assert(0);
	}

	return;
}
