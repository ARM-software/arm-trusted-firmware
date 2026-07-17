/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * External-Abort / SError trap: verbose dump for kernel
 * bring-up forensics.
 *
 * BL31 declared plat_ea_handler() as a weak alias to plat_default_ea_handler(),
 * which itself just prints "Unhandled External Abort" + mpidr + syndrome
 * and panics. That is too weak for kernel-side debugging on a custom board:
 * we never see ESR_EL3/FAR_EL3/ELR_EL3, which are the registers that
 * actually identify the offending PC, the faulting address, and the
 * abort-class encoding.
 *
 * This file:
 *   - decodes ea_reason (ASYNC = SError, SYNC = synchronous EA, ESB = ack);
 *   - reads and prints ESR_EL3/FAR_EL3/ELR_EL3/SPSR_EL3/MPIDR;
 *   - decodes ESR_EL3.EC/IL/ISS for the common SError + EA cases (0x2f,
 *     0x25, 0x24);
 *   - lets the default code path continue after we have logged everything,
 *     so the panic sequence is unchanged.
 *
 * The benefit when kernel fault is: instead of "Unhandled External
 * Abort, mpidr=..., syndrome=..." with no further context, the log
 * shows ESR_EL3.EC decoded, FAR_EL3 (the faulting address), ELR_EL3 (the
 * PC at fault), and SPSR_EL3 (the EL state at fault).
 *
 * Gated by HANDLE_EA_EL3_FIRST_NS=1 in the nbxv3 platform.mk
 */

#include <stdint.h>

#include <arch_helpers.h>
#include <bl31/ea_handle.h>
#include <common/debug.h>
#include <plat/common/platform.h>

static const char *ea_reason_str(unsigned int ea_reason)
{
	switch (ea_reason) {
	case ERROR_EA_ASYNC:
		return "ASYNC (SError from lower EL)";
	case ERROR_EA_SYNC:
		return "SYNC (synchronous external abort from lower EL)";
	case ERROR_EA_ESB:
		return "ESB (Error Synchronization Barrier ack)";
	default:
		return "unknown";
	}
}

/*
 * ESR_EL3.EC is the exception class.
 *   0x24 = data abort from lower EL  (kernel/U-Boot read/write faulted)
 *   0x25 = data abort from current EL
 *   0x2f = SError interrupt from any EL
 * Everything else gets a generic label so we still print the raw hex.
 */
static const char *esr_ec_str(unsigned int ec)
{
	switch (ec) {
	case 0x24: return "Data abort from lower EL";
	case 0x25: return "Data abort from current EL";
	case 0x2c: return "FP/SIMD trap";
	case 0x2e: return "Trap from current EL";
	case 0x2f: return "SError interrupt";
	default:   return "see ARM ARM D13.2.39";
	}
}

void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome,
		     void *cookie, void *handle, uint64_t flags)
{
	uint64_t esr	= read_esr_el3();
	uint64_t far	= read_far_el3();
	uint64_t elr	= read_elr_el3();
	uint64_t spsr	= read_spsr_el3();
	uint64_t mpidr	= read_mpidr_el1();
	unsigned int ec	= (unsigned int)((esr >> 26) & 0x3f);
	unsigned int il	= (unsigned int)((esr >> 25) & 0x1);
	uint32_t iss	= (uint32_t)(esr & 0x01ffffffu);

	(void)cookie;
	(void)handle;
	(void)flags;

	ERROR_NL();
	ERROR("nbxv3-ea-trap: reason=%u (%s) syndrome=0x%lx mpidr=0x%lx\n",
	      ea_reason, ea_reason_str(ea_reason),
	      (unsigned long)syndrome, (unsigned long)mpidr);
	ERROR("nbxv3-ea-trap: ESR_EL3=0x%lx (EC=0x%x [%s] IL=%u ISS=0x%x)\n",
	      (unsigned long)esr, ec, esr_ec_str(ec), il, iss);
	ERROR("nbxv3-ea-trap: ELR_EL3=0x%lx FAR_EL3=0x%lx SPSR_EL3=0x%lx\n",
	      (unsigned long)elr, (unsigned long)far, (unsigned long)spsr);

	/*
	 * Hand off to the default. It will:
	 *   - call the RAS subsystem if FEAT_RAS is supported (no-op here);
	 *   - print the upstream-formatted error;
	 *   - panic().
	 *
	 * We don't suppress the panic: an unhandled SError at EL3 means the
	 * machine is already in a state we can't trust to keep running.
	 * Printing first, then panicking.
	 */
	plat_default_ea_handler(ea_reason, syndrome, cookie, handle, flags);
}
