/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARM_TRUSTED_FIRMWARE_EXPORT_COMMON_EP_INFO_EXP_H
#define ARM_TRUSTED_FIRMWARE_EXPORT_COMMON_EP_INFO_EXP_H

/* EXPORT HEADER -- See include/export/README for details! -- EXPORT HEADER */

#include "../lib/utils_def_exp.h"
#include "param_header_exp.h"

/*******************************************************************************
 * Constants that allow assembler code to access members of and the
 * 'entry_point_info' structure at their correct offsets.
 ******************************************************************************/
#define ENTRY_POINT_INFO_PC_OFFSET	U(0x08)
#ifdef __aarch64__
#define ENTRY_POINT_INFO_ARGS_OFFSET	U(0x18)
#else
#define ENTRY_POINT_INFO_LR_SVC_OFFSET	U(0x10)
#define ENTRY_POINT_INFO_ARGS_OFFSET	U(0x14)
#endif

/* Security state of the image. */
#define EP_SECURITY_MASK	UL(0x1)
#define EP_SECURITY_SHIFT	UL(0)
#define EP_SECURE		UL(0x0)
#define EP_NON_SECURE		UL(0x1)

/* Endianness of the image. */
#define EP_EE_MASK		U(0x2)
#define EP_EE_SHIFT		U(1)
#define EP_EE_LITTLE		U(0x0)
#define EP_EE_BIG		U(0x2)
#define EP_GET_EE(x)		((x) & EP_EE_MASK)
#define EP_SET_EE(x, ee)	((x) = ((x) & ~EP_EE_MASK) | (ee))

/* Enable or disable access to the secure timer from secure images. */
#define EP_ST_MASK		U(0x4)
#define EP_ST_SHIFT		U(2)
#define EP_ST_DISABLE		U(0x0)
#define EP_ST_ENABLE		U(0x4)
#define EP_GET_ST(x)		((x) & EP_ST_MASK)
#define EP_SET_ST(x, ee)	((x) = ((x) & ~EP_ST_MASK) | (ee))

/* Determine if an image is executable or not. */
#define EP_EXE_MASK		U(0x8)
#define EP_EXE_SHIFT		U(3)
#define EP_NON_EXECUTABLE	U(0x0)
#define EP_EXECUTABLE		U(0x8)
#define EP_GET_EXE(x)		((x) & EP_EXE_MASK)
#define EP_SET_EXE(x, ee)	((x) = ((x) & ~EP_EXE_MASK) | (ee))

/* Flag to indicate the first image that is executed. */
#define EP_FIRST_EXE_MASK	U(0x10)
#define EP_FIRST_EXE_SHIFT	U(4)
#define EP_FIRST_EXE		U(0x10)
#define EP_GET_FIRST_EXE(x)	((x) & EP_FIRST_EXE_MASK)
#define EP_SET_FIRST_EXE(x, ee)	((x) = ((x) & ~EP_FIRST_EXE_MASK) | (ee))

#ifndef __ASSEMBLER__

typedef struct aapcs64_params {
	uint64_t arg0;
	uint64_t arg1;
	uint64_t arg2;
	uint64_t arg3;
	uint64_t arg4;
	uint64_t arg5;
	uint64_t arg6;
	uint64_t arg7;
} aapcs64_params_t;

typedef struct aapcs32_params {
	uint32_t arg0;
	uint32_t arg1;
	uint32_t arg2;
	uint32_t arg3;
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
#ifdef __aarch64__
	aapcs64_params_t args;
#else
	uintptr_t lr_svc;
	aapcs32_params_t args;
#endif
} entry_point_info_t;

#endif /*__ASSEMBLER__*/

#endif /* ARM_TRUSTED_FIRMWARE_EXPORT_COMMON_EP_INFO_EXP_H */
