/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cpu_macros.S>
#include <cortex_a510.h>


.global apply_faulting_side_channel_wa
func apply_faulting_side_channel_wa
	jump_if_not_cpu_midr	CORTEX_A510_MIDR, end_wa
	apply_erratum	cortex_a510, ERRATUM(4568615), ERRATA_A510_4568615
end_wa:
ret
endfunc apply_faulting_side_channel_wa
