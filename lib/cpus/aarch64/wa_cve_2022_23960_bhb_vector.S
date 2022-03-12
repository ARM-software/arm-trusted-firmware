/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <services/arm_arch_svc.h>
#include "wa_cve_2022_23960_bhb.S"

	/*
	 * This macro is used to isolate the vector table for relevant CPUs
	 * used in the mitigation for CVE_2022_23960.
	 */
	.macro wa_cve_2022_23960_bhb_vector_table _bhb_loop_count, _cpu

	.globl	wa_cve_vbar_\_cpu

vector_base wa_cve_vbar_\_cpu
	/* ---------------------------------------------------------------------
	 * Current EL with SP_EL0 : 0x0 - 0x200
	 * ---------------------------------------------------------------------
	 */
vector_entry bhb_sync_exception_sp_el0_\_cpu
	b	sync_exception_sp_el0
end_vector_entry bhb_sync_exception_sp_el0_\_cpu

vector_entry bhb_irq_sp_el0_\_cpu
	b	irq_sp_el0
end_vector_entry bhb_irq_sp_el0_\_cpu

vector_entry bhb_fiq_sp_el0_\_cpu
	b	fiq_sp_el0
end_vector_entry bhb_fiq_sp_el0_\_cpu

vector_entry bhb_serror_sp_el0_\_cpu
	b	serror_sp_el0
end_vector_entry bhb_serror_sp_el0_\_cpu

	/* ---------------------------------------------------------------------
	 * Current EL with SP_ELx: 0x200 - 0x400
	 * ---------------------------------------------------------------------
	 */
vector_entry bhb_sync_exception_sp_elx_\_cpu
	b	sync_exception_sp_elx
end_vector_entry bhb_sync_exception_sp_elx_\_cpu

vector_entry bhb_irq_sp_elx_\_cpu
	b	irq_sp_elx
end_vector_entry bhb_irq_sp_elx_\_cpu

vector_entry bhb_fiq_sp_elx_\_cpu
	b	fiq_sp_elx
end_vector_entry bhb_fiq_sp_elx_\_cpu

vector_entry bhb_serror_sp_elx_\_cpu
	b	serror_sp_elx
end_vector_entry bhb_serror_sp_elx_\_cpu

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch64 : 0x400 - 0x600
	 * ---------------------------------------------------------------------
	 */
vector_entry bhb_sync_exception_aarch64_\_cpu
	apply_cve_2022_23960_bhb_wa \_bhb_loop_count
	b	sync_exception_aarch64
end_vector_entry bhb_sync_exception_aarch64_\_cpu

vector_entry bhb_irq_aarch64_\_cpu
	apply_cve_2022_23960_bhb_wa \_bhb_loop_count
	b	irq_aarch64
end_vector_entry bhb_irq_aarch64_\_cpu

vector_entry bhb_fiq_aarch64_\_cpu
	apply_cve_2022_23960_bhb_wa \_bhb_loop_count
	b	fiq_aarch64
end_vector_entry bhb_fiq_aarch64_\_cpu

vector_entry bhb_serror_aarch64_\_cpu
	apply_cve_2022_23960_bhb_wa \_bhb_loop_count
	b	serror_aarch64
end_vector_entry bhb_serror_aarch64_\_cpu

	/* ---------------------------------------------------------------------
	 * Lower EL using AArch32 : 0x600 - 0x800
	 * ---------------------------------------------------------------------
	 */
vector_entry bhb_sync_exception_aarch32_\_cpu
	apply_cve_2022_23960_bhb_wa \_bhb_loop_count
	b	sync_exception_aarch32
end_vector_entry bhb_sync_exception_aarch32_\_cpu

vector_entry bhb_irq_aarch32_\_cpu
	apply_cve_2022_23960_bhb_wa \_bhb_loop_count
	b	irq_aarch32
end_vector_entry bhb_irq_aarch32_\_cpu

vector_entry bhb_fiq_aarch32_\_cpu
	apply_cve_2022_23960_bhb_wa \_bhb_loop_count
	b	fiq_aarch32
end_vector_entry bhb_fiq_aarch32_\_cpu

vector_entry bhb_serror_aarch32_\_cpu
	apply_cve_2022_23960_bhb_wa \_bhb_loop_count
	b	serror_aarch32
end_vector_entry bhb_serror_aarch32_\_cpu
	.endm
