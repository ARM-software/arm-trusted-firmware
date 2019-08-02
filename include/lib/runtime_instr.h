/*
 * Copyright (c) 2016-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RUNTIME_INSTR_H
#define RUNTIME_INSTR_H

#include <lib/utils_def.h>

#define RT_INSTR_ENTER_PSCI		U(0)
#define RT_INSTR_EXIT_PSCI		U(1)
#define RT_INSTR_ENTER_HW_LOW_PWR	U(2)
#define RT_INSTR_EXIT_HW_LOW_PWR	U(3)
#define RT_INSTR_ENTER_CFLUSH		U(4)
#define RT_INSTR_EXIT_CFLUSH		U(5)
#define RT_INSTR_TOTAL_IDS		U(6)

#ifndef __ASSEMBLER__
PMF_DECLARE_CAPTURE_TIMESTAMP(rt_instr_svc)
PMF_DECLARE_GET_TIMESTAMP(rt_instr_svc)
#endif /* __ASSEMBLER__ */

#endif /* RUNTIME_INSTR_H */
