/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_CPU_PWR_H
#define FVP_CPU_PWR_H

#ifndef __ASSEMBLER__
#include <stdbool.h>
#include <stdint.h>

#if __aarch64__
bool check_cpupwrctrl_el1_is_available(void);
#endif /* __aarch64__ */
#endif /* __ASSEMBLER__ */

/*******************************************************************************
 * CPU Power Control register specific definitions
 ******************************************************************************/
#define CPUPWRCTLR_EL1                  S3_0_C15_C2_7
#define CPUPWRCTLR_EL1_CORE_PWRDN_BIT   U(1)

#endif /* FVP_CPU_PWR_H */
