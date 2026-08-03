/*
 * Copyright (c) 2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <asm_macros.S>
#include <cpu_macros.S>
#include <cortex_a510.h>
#include <cortex_a520.h>
#include <c1_nano.h>


.global apply_faulting_side_channel_wa
func apply_faulting_side_channel_wa
	jump_if_not_cpu_midr	CORTEX_A510_MIDR, a520
	apply_erratum	cortex_a510, ERRATUM(4568615), ERRATA_A510_4568615
a520:
	jump_if_not_cpu_midr	CORTEX_A520_MIDR, c1_nano
	apply_erratum	cortex_a520, ERRATUM(4491980), ERRATA_A520_4491980, NO_GET_CPU_REV
c1_nano:
	jump_if_not_cpu_midr	C1_NANO_MIDR, end_wa
	apply_erratum	c1_nano, ERRATUM(4497400), ERRATA_C1NANO_4497400, NO_GET_CPU_REV
end_wa:
ret
endfunc apply_faulting_side_channel_wa
