/*
 * Copyright (c) 2015-2017, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/bl_common.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <drivers/arm/gicv2.h>
#include <lib/mmio.h>

#include "rcar_def.h"

#define SWDT_ERROR_ID	(1024U)
#define SWDT_ERROR_TYPE	(16U)
#define SWDT_CHAR_MAX	(13U)

extern void rcar_swdt_release(void);

void bl2_interrupt_error_id(uint32_t int_id)
{
	ERROR("\n");
	if (int_id >= SWDT_ERROR_ID) {
		ERROR("Unhandled exception occurred.\n");
		ERROR("     Exception type = FIQ_SP_EL0\n");
		panic();
	}

	/* Clear the interrupt request */
	gicv2_end_of_interrupt((uint32_t) int_id);
	rcar_swdt_release();
	ERROR("Unhandled exception occurred.\n");
	ERROR("     Exception type = FIQ_SP_EL0\n");
	ERROR("     SPSR_EL3 = 0x%x\n", (uint32_t) read_spsr_el3());
	ERROR("     ELR_EL3  = 0x%x\n", (uint32_t) read_elr_el3());
	ERROR("     ESR_EL3  = 0x%x\n", (uint32_t) read_esr_el3());
	ERROR("     FAR_EL3  = 0x%x\n", (uint32_t) read_far_el3());
	ERROR("\n");
	panic();
}

void bl2_interrupt_error_type(uint32_t ex_type)
{
	const uint8_t interrupt_ex[SWDT_ERROR_TYPE][SWDT_CHAR_MAX] = {
		"SYNC SP EL0",
		"IRQ SP EL0",
		"FIQ SP EL0",
		"SERR SP EL0",
		"SYNC SP ELx",
		"IRQ SP ELx",
		"FIQ SP ELx",
		"SERR SP ELx",
		"SYNC AARCH64",
		"IRQ AARCH64",
		"FIQ AARCH64",
		"SERR AARCH64",
		"SYNC AARCH32",
		"IRQ AARCH32",
		"FIQ AARCH32",
		"SERR AARCH32"
	};
	char msg[128];

	/* Clear the interrupt request  */
	if (ex_type >= SWDT_ERROR_TYPE) {
		ERROR("\n");
		ERROR("Unhandled exception occurred.\n");
		ERROR("     Exception type = Unknown (%d)\n", ex_type);
		goto loop;
	}

	rcar_swdt_release();
	ERROR("\n");
	ERROR("Unhandled exception occurred.\n");
	snprintf(msg, sizeof(msg), "     Exception type = %s\n",
		 &interrupt_ex[ex_type][0]);
	ERROR("%s", msg);
	switch (ex_type) {
	case SYNC_EXCEPTION_SP_EL0:
		ERROR("     SPSR_EL3 = 0x%x\n", (uint32_t) read_spsr_el3());
		ERROR("     ELR_EL3  = 0x%x\n", (uint32_t) read_elr_el3());
		ERROR("     ESR_EL3  = 0x%x\n", (uint32_t) read_esr_el3());
		ERROR("     FAR_EL3  = 0x%x\n", (uint32_t) read_far_el3());
		break;
	case IRQ_SP_EL0:
		ERROR("     SPSR_EL3 = 0x%x\n", (uint32_t) read_spsr_el3());
		ERROR("     ELR_EL3  = 0x%x\n", (uint32_t) read_elr_el3());
		ERROR("     IAR_EL3  = 0x%x\n", gicv2_acknowledge_interrupt());
		break;
	case FIQ_SP_EL0:
		ERROR("     SPSR_EL3 = 0x%x\n", (uint32_t) read_spsr_el3());
		ERROR("     ELR_EL3  = 0x%x\n", (uint32_t) read_elr_el3());
		ERROR("     IAR_EL3  = 0x%x\n", gicv2_acknowledge_interrupt());
		break;
	case SERROR_SP_EL0:
		ERROR("     SPSR_EL3 = 0x%x\n", (uint32_t) read_spsr_el3());
		ERROR("     ELR_EL3  = 0x%x\n", (uint32_t) read_elr_el3());
		ERROR("     ESR_EL3  = 0x%x\n", (uint32_t) read_esr_el3());
		ERROR("     FAR_EL3  = 0x%x\n", (uint32_t) read_far_el3());
		break;
	default:
		break;
	}
loop:
	ERROR("\n");
	panic();
}
