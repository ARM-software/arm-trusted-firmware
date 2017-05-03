/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RUNTIME_INSTR_H__
#define __RUNTIME_INSTR_H__

#define RT_INSTR_ENTER_PSCI		0
#define RT_INSTR_EXIT_PSCI		1
#define RT_INSTR_ENTER_HW_LOW_PWR	2
#define RT_INSTR_EXIT_HW_LOW_PWR	3
#define RT_INSTR_ENTER_CFLUSH		4
#define RT_INSTR_EXIT_CFLUSH		5
#define RT_INSTR_TOTAL_IDS		6

#ifndef __ASSEMBLY__
PMF_DECLARE_CAPTURE_TIMESTAMP(rt_instr_svc)
PMF_DECLARE_GET_TIMESTAMP(rt_instr_svc)
#endif /* __ASSEMBLY__ */

#endif /* __RUNTIME_INSTR_H__ */
