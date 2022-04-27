/*
 * Copyright (c) 2014-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <drivers/console.h>
#if RAS_EXTENSION
#include <lib/extensions/ras.h>
#endif
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <plat/common/platform.h>

/*
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that may be re-used by multiple
 * platforms but may also be overridden by a platform if required.
 */
#pragma weak bl31_plat_runtime_setup

#if SDEI_SUPPORT
#pragma weak plat_sdei_handle_masked_trigger
#pragma weak plat_sdei_validate_entry_point
#endif

#pragma weak plat_ea_handler = plat_default_ea_handler

void bl31_plat_runtime_setup(void)
{
	console_switch_state(CONSOLE_FLAG_RUNTIME);
}

/*
 * Helper function for platform_get_pos() when platform compatibility is
 * disabled. This is to enable SPDs using the older platform API to continue
 * to work.
 */
unsigned int platform_core_pos_helper(unsigned long mpidr)
{
	int idx = plat_core_pos_by_mpidr(mpidr);
	assert(idx >= 0);
	return idx;
}

#if SDEI_SUPPORT
/*
 * Function that handles spurious SDEI interrupts while events are masked.
 */
void plat_sdei_handle_masked_trigger(uint64_t mpidr, unsigned int intr)
{
	WARN("Spurious SDEI interrupt %u on masked PE %" PRIx64 "\n", intr, mpidr);
}

/*
 * Default Function to validate SDEI entry point, which returns success.
 * Platforms may override this with their own validation mechanism.
 */
int plat_sdei_validate_entry_point(uintptr_t ep, unsigned int client_mode)
{
	return 0;
}
#endif

#if !ENABLE_BACKTRACE
static const char *get_el_str(unsigned int el)
{
	if (el == MODE_EL3) {
		return "EL3";
	} else if (el == MODE_EL2) {
		return "EL2";
	}
	return "S-EL1";
}
#endif /* !ENABLE_BACKTRACE */

/* RAS functions common to AArch64 ARM platforms */
void plat_default_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
#if RAS_EXTENSION
	/* Call RAS EA handler */
	int handled = ras_ea_handler(ea_reason, syndrome, cookie, handle, flags);
	if (handled != 0)
		return;
#endif
	unsigned int level = (unsigned int)GET_EL(read_spsr_el3());

	ERROR_NL();
	ERROR("Unhandled External Abort received on 0x%lx from %s\n",
		read_mpidr_el1(), get_el_str(level));
	ERROR("exception reason=%u syndrome=0x%" PRIx64 "\n", ea_reason, syndrome);
#if HANDLE_EA_EL3_FIRST
	/* Skip backtrace for lower EL */
	if (level != MODE_EL3) {
		console_flush();
		do_panic();
	}
#endif
	panic();
}
