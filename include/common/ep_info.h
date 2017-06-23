/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __EP_INFO_H__
#define __EP_INFO_H__

#include <param_header.h>
#include <utils_def.h>

#define SECURE		U(0x0)
#define NON_SECURE	U(0x1)
#define sec_state_is_valid(s) (((s) == SECURE) || ((s) == NON_SECURE))

/*******************************************************************************
 * Constants that allow assembler code to access members of and the
 * 'entry_point_info' structure at their correct offsets.
 ******************************************************************************/
#define ENTRY_POINT_INFO_PC_OFFSET	U(0x08)
#ifdef AARCH32
#define ENTRY_POINT_INFO_ARGS_OFFSET	U(0x10)
#else
#define ENTRY_POINT_INFO_ARGS_OFFSET	U(0x18)
#endif

/* The following are used to set/get image attributes. */
#define PARAM_EP_SECURITY_MASK		U(0x1)

#define GET_SECURITY_STATE(x) (x & PARAM_EP_SECURITY_MASK)
#define SET_SECURITY_STATE(x, security) \
			((x) = ((x) & ~PARAM_EP_SECURITY_MASK) | (security))

#define EP_EE_MASK	U(0x2)
#define EP_EE_SHIFT	1
#define EP_EE_LITTLE	U(0x0)
#define EP_EE_BIG	U(0x2)
#define EP_GET_EE(x) (x & EP_EE_MASK)
#define EP_SET_EE(x, ee) ((x) = ((x) & ~EP_EE_MASK) | (ee))

#define EP_ST_MASK	U(0x4)
#define EP_ST_DISABLE	U(0x0)
#define EP_ST_ENABLE	U(0x4)
#define EP_GET_ST(x) (x & EP_ST_MASK)
#define EP_SET_ST(x, ee) ((x) = ((x) & ~EP_ST_MASK) | (ee))

#define EP_EXE_MASK	U(0x8)
#define NON_EXECUTABLE	U(0x0)
#define EXECUTABLE	U(0x8)
#define EP_GET_EXE(x) (x & EP_EXE_MASK)
#define EP_SET_EXE(x, ee) ((x) = ((x) & ~EP_EXE_MASK) | (ee))

#define EP_FIRST_EXE_MASK	U(0x10)
#define EP_FIRST_EXE		U(0x10)
#define EP_GET_FIRST_EXE(x) ((x) & EP_FIRST_EXE_MASK)
#define EP_SET_FIRST_EXE(x, ee) ((x) = ((x) & ~EP_FIRST_EXE_MASK) | (ee))

#ifndef __ASSEMBLY__

#include <cassert.h>
#include <types.h>

typedef struct aapcs64_params {
	u_register_t arg0;
	u_register_t arg1;
	u_register_t arg2;
	u_register_t arg3;
	u_register_t arg4;
	u_register_t arg5;
	u_register_t arg6;
	u_register_t arg7;
} aapcs64_params_t;

typedef struct aapcs32_params {
	u_register_t arg0;
	u_register_t arg1;
	u_register_t arg2;
	u_register_t arg3;
} aapcs32_params_t;

/*****************************************************************************
 * This structure represents the superset of information needed while
 * switching exception levels. The only two mechanisms to do so are
 * ERET & SMC. Security state is indicated using bit zero of header
 * attribute
 * NOTE: BL1 expects entrypoint followed by spsr at an offset from the start
 * of this structure defined by the macro `ENTRY_POINT_INFO_PC_OFFSET` while
 * processing SMC to jump to BL31.
 *****************************************************************************/
typedef struct entry_point_info {
	param_header_t h;
	uintptr_t pc;
	uint32_t spsr;
#ifdef AARCH32
	aapcs32_params_t args;
#else
	aapcs64_params_t args;
#endif
} entry_point_info_t;

/*
 * Compile time assertions related to the 'entry_point_info' structure to
 * ensure that the assembler and the compiler view of the offsets of
 * the structure members is the same.
 */
CASSERT(ENTRY_POINT_INFO_PC_OFFSET ==
		__builtin_offsetof(entry_point_info_t, pc), \
		assert_BL31_pc_offset_mismatch);

CASSERT(ENTRY_POINT_INFO_ARGS_OFFSET == \
		__builtin_offsetof(entry_point_info_t, args), \
		assert_BL31_args_offset_mismatch);

CASSERT(sizeof(uintptr_t) ==
		__builtin_offsetof(entry_point_info_t, spsr) - \
		__builtin_offsetof(entry_point_info_t, pc), \
		assert_entrypoint_and_spsr_should_be_adjacent);

#endif /*__ASSEMBLY__*/

#endif /* __EP_INFO_H__ */

