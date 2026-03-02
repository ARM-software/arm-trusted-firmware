/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPC_FAULT_H
#define GPC_FAULT_H

#define	GPC_GPF_TRAP_PC_START_OFFSET	U(0)
#define	GPC_GPF_TRAP_PC_RESUME_OFFSET	U(8)
#define	GPC_GPF_TRAP_SIZE		U(16)

#ifndef __ASSEMBLER__

struct gpc_gpf_trap_entry {
	/* fault_pc is expected to fault */
	uintptr_t fault_pc;
	/* Resume execution at this EL3 address after a GPF fault */
	uintptr_t resume_pc;
};

#endif /* __ASSEMBLER__ */
#endif /* GPC_FAULT_H */
