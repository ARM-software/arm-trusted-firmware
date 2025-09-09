/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IDTE3_H
#define IDTE3_H

#ifdef IMAGE_BL31
#include <bl31/sync_handle.h>
#include <context.h>
#include <lib/el3_runtime/cpu_data.h>
#define ESR_ELx_ISS(esr)          ((esr) & 0x01ffffff)

/* ISS layout for trapped AArch64 system-register access (ESR_EL3.ISS)
 *
 *  [21:20]  Op0
 *  [19:17]  Op2
 *  [16:14]  Op1
 *  [13:10]  CRn
 *  [9:5]    Rt
 *  [4:1]    CRm
 *  [0]      DIR
 */
#define ISS_SYS64_OP0_SHIFT       U(20)
#define ISS_SYS64_OP2_SHIFT       U(17)
#define ISS_SYS64_OP1_SHIFT       U(14)
#define ISS_SYS64_CRN_SHIFT       U(10)
#define ISS_SYS64_RT_SHIFT        U(5)
#define ISS_SYS64_CRM_SHIFT       U(1)
#define ISS_SYS64_DIR_SHIFT       U(0)

#define ISS_SYS64_OP0_MASK        U(0x3)
#define ISS_SYS64_OP1_MASK        U(0x7)
#define ISS_SYS64_OP2_MASK        U(0x7)
#define ISS_SYS64_CRN_MASK        U(0xf)
#define ISS_SYS64_RT_MASK         ULL(0x1f)
#define ISS_SYS64_CRM_MASK        U(0xf)
#define ISS_SYS64_DIR_MASK        U(0x1)

/* Field extractors */
#define ISS_SYS64_OP0(iss)        (((iss) >> ISS_SYS64_OP0_SHIFT) & \
		ISS_SYS64_OP0_MASK)

#define ISS_SYS64_OP1(iss)        (((iss) >> ISS_SYS64_OP1_SHIFT) & \
		ISS_SYS64_OP1_MASK)

#define ISS_SYS64_OP2(iss)        (((iss) >> ISS_SYS64_OP2_SHIFT) & \
		ISS_SYS64_OP2_MASK)

#define ISS_SYS64_CRN(iss)        (((iss) >> ISS_SYS64_CRN_SHIFT) & \
		ISS_SYS64_CRN_MASK)

#define ISS_SYS64_RT(iss)         (((iss) >> ISS_SYS64_RT_SHIFT)  & \
		ISS_SYS64_RT_MASK)

#define ISS_SYS64_CRM(iss)        (((iss) >> ISS_SYS64_CRM_SHIFT) & \
		ISS_SYS64_CRM_MASK)

#define ISS_SYS64_DIR(iss)        (((iss) >> ISS_SYS64_DIR_SHIFT) & \
		ISS_SYS64_DIR_MASK)

#define SYSREG_ESR(op0, op1, crn, crm, op2) \
		((UL(op0) << ISS_SYS64_OP0_SHIFT) | \
		 (UL(op1) << ISS_SYS64_OP1_SHIFT) | \
		 (UL(crn) << ISS_SYS64_CRN_SHIFT) | \
		 (UL(crm) << ISS_SYS64_CRM_SHIFT) | \
		 (UL(op2) << ISS_SYS64_OP2_SHIFT))

#define ESR_EL3_SYSREG_MASK SYSREG_ESR(3, 7, 15, 15, 7)

#define ESR_EL3_IDREG_ID_PFR0_EL1 SYSREG_ESR(3, 0, 0, 1, 0)
#define ESR_EL3_IDREG_ID_PFR1_EL1 SYSREG_ESR(3, 0, 0, 1, 1)
#define ESR_EL3_IDREG_ID_DFR0_EL1 SYSREG_ESR(3, 0, 0, 1, 2)
#define ESR_EL3_IDREG_ID_AFR0_EL1 SYSREG_ESR(3, 0, 0, 1, 3)
#define ESR_EL3_IDREG_ID_MMFR0_EL1 SYSREG_ESR(3, 0, 0, 1, 4)
#define ESR_EL3_IDREG_ID_MMFR1_EL1 SYSREG_ESR(3, 0, 0, 1, 5)
#define ESR_EL3_IDREG_ID_MMFR2_EL1 SYSREG_ESR(3, 0, 0, 1, 6)
#define ESR_EL3_IDREG_ID_MMFR3_EL1 SYSREG_ESR(3, 0, 0, 1, 7)

#define ESR_EL3_IDREG_ID_ISAR0_EL1 SYSREG_ESR(3, 0, 0, 2, 0)
#define ESR_EL3_IDREG_ID_ISAR1_EL1 SYSREG_ESR(3, 0, 0, 2, 1)
#define ESR_EL3_IDREG_ID_ISAR2_EL1 SYSREG_ESR(3, 0, 0, 2, 2)
#define ESR_EL3_IDREG_ID_ISAR3_EL1 SYSREG_ESR(3, 0, 0, 2, 3)
#define ESR_EL3_IDREG_ID_ISAR4_EL1 SYSREG_ESR(3, 0, 0, 2, 4)
#define ESR_EL3_IDREG_ID_ISAR5_EL1 SYSREG_ESR(3, 0, 0, 2, 5)
#define ESR_EL3_IDREG_ID_MMFR4_EL1 SYSREG_ESR(3, 0, 0, 2, 6)
#define ESR_EL3_IDREG_ID_ISAR6_EL1 SYSREG_ESR(3, 0, 0, 2, 7)

#define ESR_EL3_IDREG_MVFR0_EL1 SYSREG_ESR(3, 0, 0, 3, 0)
#define ESR_EL3_IDREG_MVFR1_EL1 SYSREG_ESR(3, 0, 0, 3, 1)
#define ESR_EL3_IDREG_MVFR2_EL1 SYSREG_ESR(3, 0, 0, 3, 2)
#define ESR_EL3_IDREG_ID_PFR2_EL1 SYSREG_ESR(3, 0, 0, 3, 4)
#define ESR_EL3_IDREG_ID_DFR1_EL1 SYSREG_ESR(3, 0, 0, 3, 5)
#define ESR_EL3_IDREG_ID_MMFR5_EL1 SYSREG_ESR(3, 0, 0, 3, 6)

#define ESR_EL3_IDREG_ID_AA64PFR0_EL1 SYSREG_ESR(3, 0, 0, 4, 0)
#define ESR_EL3_IDREG_ID_AA64PFR1_EL1 SYSREG_ESR(3, 0, 0, 4, 1)
#define ESR_EL3_IDREG_ID_AA64PFR2_EL1 SYSREG_ESR(3, 0, 0, 4, 2)
#define ESR_EL3_IDREG_ID_AA64ZFR0_EL1 SYSREG_ESR(3, 0, 0, 4, 4)
#define ESR_EL3_IDREG_ID_AA64SMFR0_EL1 SYSREG_ESR(3, 0, 0, 4, 5)
#define ESR_EL3_IDREG_ID_AA64FPFR0_EL1 SYSREG_ESR(3, 0, 0, 4, 7)

#define ESR_EL3_IDREG_ID_AA64DFR0_EL1 SYSREG_ESR(3, 0, 0, 5, 0)
#define ESR_EL3_IDREG_ID_AA64DFR1_EL1 SYSREG_ESR(3, 0, 0, 5, 1)
#define ESR_EL3_IDREG_ID_AA64DFR2_EL1 SYSREG_ESR(3, 0, 0, 5, 2)
#define ESR_EL3_IDREG_ID_AA64AFR0_EL1 SYSREG_ESR(3, 0, 0, 5, 4)
#define ESR_EL3_IDREG_ID_AA64AFR1_EL1 SYSREG_ESR(3, 0, 0, 5, 5)

#define ESR_EL3_IDREG_ID_AA64ISAR0_EL1 SYSREG_ESR(3, 0, 0, 6, 0)
#define ESR_EL3_IDREG_ID_AA64ISAR1_EL1 SYSREG_ESR(3, 0, 0, 6, 1)
#define ESR_EL3_IDREG_ID_AA64ISAR2_EL1 SYSREG_ESR(3, 0, 0, 6, 2)
#define ESR_EL3_IDREG_ID_AA64ISAR3_EL1 SYSREG_ESR(3, 0, 0, 6, 3)
#define ESR_EL3_IDREG_ID_AA64MMFR0_EL1 SYSREG_ESR(3, 0, 0, 7, 0)

#define ESR_EL3_IDREG_ID_AA64MMFR1_EL1 SYSREG_ESR(3, 0, 0, 7, 1)
#define ESR_EL3_IDREG_ID_AA64MMFR2_EL1 SYSREG_ESR(3, 0, 0, 7, 2)
#define ESR_EL3_IDREG_ID_AA64MMFR3_EL1 SYSREG_ESR(3, 0, 0, 7, 3)
#define ESR_EL3_IDREG_ID_AA64MMFR4_EL1 SYSREG_ESR(3, 0, 0, 7, 4)

/* Group 5 ID Registers trapped*/
#define ESR_EL3_IDREG_GMID_EL1         SYSREG_ESR(3, 1, 0, 0, 4)

#if ENABLE_FEAT_IDTE3
void idte3_enable(cpu_context_t *ctx);
int handle_idreg_trap(uint64_t esr_el3, cpu_context_t *ctx,
				u_register_t flags);
void idte3_init_cached_idregs_per_world(size_t security_state);
void idte3_init_percpu_once_regs(size_t security_state);
#else
static inline void idte3_enable(cpu_context_t *ctx)
{
}
static inline int handle_idreg_trap(uint64_t esr_el3, cpu_context_t *ctx,
							u_register_t flags)
{
	return TRAP_RET_UNHANDLED;
}
static inline void idte3_init_percpu_once_regs(size_t security_state)
{
}
static inline void idte3_init_cached_idregs_per_world(size_t security_state)
{
}
#endif /* ENABLE_FEAT_IDTE3 */
#endif /* IMAGE_BL31 */
#endif /* IDTE3_H */
