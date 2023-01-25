/*
 * Copyright (c) 2017-2023, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ERRATA_REPORT_H
#define ERRATA_REPORT_H

#ifndef __ASSEMBLER__

void print_errata_status(void);
void errata_print_msg(unsigned int status, const char *cpu, const char *id);

#endif /* __ASSEMBLER__ */

/* Errata status */
#define ERRATA_NOT_APPLIES	0
#define ERRATA_APPLIES		1
#define ERRATA_MISSING		2

/* Macro to get CPU revision code for checking errata version compatibility. */
#define CPU_REV(r, p)		((r << 4) | p)

#endif /* ERRATA_REPORT_H */
