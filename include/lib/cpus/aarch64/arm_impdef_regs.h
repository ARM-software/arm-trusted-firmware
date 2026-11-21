/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_IMPDEF_REGS_H
#define ARM_IMPDEF_REGS_H

/*
 * This header contains IMP DEF register mappings that are shared between Arm
 * designs. These tend to be stable across generations but must be checked
 * before a new core uses them
 *
 * NOTE: new additions have to be checked with care that they are indeed shared
 * among all cores that implement this register. Please consult each core's TRM.
 */
#define CPUACTLR_EL1			S3_0_C15_C1_0
#define CPUACTLR2_EL1			S3_0_C15_C1_1
#define CPUECTLR_EL1			S3_0_C15_C1_4

/* Instruction Patch Registers */
#define CPUPSELR_EL3			S3_6_C15_C8_0
#define CPUPCR_EL3			S3_6_C15_C8_1
#define CPUPOR_EL3			S3_6_C15_C8_2
#define CPUPMR_EL3			S3_6_C15_C8_3
#define CPUPOR2_EL3			S3_6_C15_C8_4
#define CPUPMR2_EL3			S3_6_C15_C8_5

#ifndef __ASSEMBLER__

DEFINE_RENAME_SYSREG_RW_FUNCS(cpuactlr_el1, CPUACTLR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(cpuactlr2_el1, CPUACTLR2_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(cpuectlr_el1, CPUECTLR_EL1)

DEFINE_RENAME_SYSREG_RW_FUNCS(cpupselr_el3, CPUPSELR_EL3)
DEFINE_RENAME_SYSREG_RW_FUNCS(cpupcr_el3, CPUPCR_EL3)
DEFINE_RENAME_SYSREG_RW_FUNCS(cpupor_el3, CPUPOR_EL3)
DEFINE_RENAME_SYSREG_RW_FUNCS(cpupmr_el3, CPUPMR_EL3)
DEFINE_RENAME_SYSREG_RW_FUNCS(cpupor2_el3, CPUPOR2_EL3)
DEFINE_RENAME_SYSREG_RW_FUNCS(cpupmr2_el3, CPUPMR2_EL3)

#endif


#endif /* ARM_IMPDEF_REGS_H */
