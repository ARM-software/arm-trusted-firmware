/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FVP_R_ARCH_HELPERS_H
#define FVP_R_ARCH_HELPERS_H

#include <arch_helpers.h>

/*******************************************************************************
 * MPU register definitions
 ******************************************************************************/
#define MPUIR_EL2		S3_4_C0_C0_4
#define PRBAR_EL2		S3_4_C6_C8_0
#define PRLAR_EL2		S3_4_C6_C8_1
#define PRSELR_EL2		S3_4_C6_C2_1
#define PRENR_EL2		S3_4_C6_C1_1

/* v8-R64 MPU registers */
DEFINE_RENAME_SYSREG_RW_FUNCS(mpuir_el2, MPUIR_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(prenr_el2, PRENR_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(prselr_el2, PRSELR_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(prbar_el2, PRBAR_EL2)
DEFINE_RENAME_SYSREG_RW_FUNCS(prlar_el2, PRLAR_EL2)

#endif /* FVP_R_ARCH_HELPERS_H */
