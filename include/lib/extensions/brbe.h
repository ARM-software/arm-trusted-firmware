/*
 * Copyright (c) 2022-2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BRBE_H
#define BRBE_H

#include <context.h>

#if ENABLE_BRBE_FOR_NS
void brbe_enable(cpu_context_t *ctx);
void brbe_enable_el3(void);
void brbe_start_recording(void);
void brbe_stop_recording(void);
void _brbe_dump_branch_records(void);
static inline void brbe_dump_branch_records(void)
{
	u_register_t brbfcr = read_brbfcr_el1();

	/* Pause branch recording so that we can dump without polluting the
	 * record. Do this in a static inline for zero extra branches. */
	write_brbfcr_el1(brbfcr | BRBFCR_EL1_PAUSED_BIT);
	isb();

	_brbe_dump_branch_records();

	write_brbfcr_el1(brbfcr);
	isb();
}
#else
static inline void brbe_enable(cpu_context_t *ctx)
{
}
static inline void brbe_enable_el3(void)
{
}
static inline void brbe_start_recording(void)
{
}
static inline void brbe_stop_recording(void)
{
}
static inline void brbe_dump_branch_records(void)
{
}
#endif /* ENABLE_BRBE_FOR_NS */

/* Maximum number of branch records per bank (32 per bank, 2 banks max) */
#define BRBE_MAX_RECORDS_PER_BANK	U(32)
#define BRBE_MAX_BANKS			U(2)
#define BRBE_MAX_RECORDS		(BRBE_MAX_RECORDS_PER_BANK * BRBE_MAX_BANKS)

/* BRBE context structure for save/restore */
typedef struct brbe_regs {
	u_register_t brbfcr_el1;
	u_register_t records;
	u_register_t brbinf[BRBE_MAX_RECORDS];
	u_register_t brbsrc[BRBE_MAX_RECORDS];
	u_register_t brbtgt[BRBE_MAX_RECORDS];
} brbe_regs_t;
#endif /* BRBE_H */
