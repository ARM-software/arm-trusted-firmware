/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EP_INFO_H
#define EP_INFO_H

#include <common/param_header.h>

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <lib/cassert.h>
#endif /* __ASSEMBLER__ */

#include <export/common/ep_info_exp.h>

#define SECURE		EP_SECURE
#define NON_SECURE	EP_NON_SECURE
#define sec_state_is_valid(s) (((s) == SECURE) || ((s) == NON_SECURE))

#define PARAM_EP_SECURITY_MASK	EP_SECURITY_MASK

#define NON_EXECUTABLE	EP_NON_EXECUTABLE
#define EXECUTABLE	EP_EXECUTABLE

/* Secure or Non-secure image */
#define GET_SECURITY_STATE(x) ((x) & EP_SECURITY_MASK)
#define SET_SECURITY_STATE(x, security) \
			((x) = ((x) & ~EP_SECURITY_MASK) | (security))

#ifndef __ASSEMBLER__

/*
 * Compile time assertions related to the 'entry_point_info' structure to
 * ensure that the assembler and the compiler view of the offsets of
 * the structure members is the same.
 */
CASSERT(ENTRY_POINT_INFO_PC_OFFSET ==
		__builtin_offsetof(entry_point_info_t, pc), \
		assert_BL31_pc_offset_mismatch);

#ifndef __aarch64__
CASSERT(ENTRY_POINT_INFO_LR_SVC_OFFSET ==
		__builtin_offsetof(entry_point_info_t, lr_svc),
		assert_entrypoint_lr_offset_error);
#endif

CASSERT(ENTRY_POINT_INFO_ARGS_OFFSET == \
		__builtin_offsetof(entry_point_info_t, args), \
		assert_BL31_args_offset_mismatch);

CASSERT(sizeof(uintptr_t) ==
		__builtin_offsetof(entry_point_info_t, spsr) - \
		__builtin_offsetof(entry_point_info_t, pc), \
		assert_entrypoint_and_spsr_should_be_adjacent);

#endif /*__ASSEMBLER__*/

#endif /* EP_INFO_H */
