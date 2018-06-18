/*
 * Copyright (c) 2014-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <console.h>
#include <platform.h>
#if RAS_EXTENSION
#include <ras.h>
#endif
#include <xlat_mmu_helpers.h>

/*
 * The following platform setup functions are weakly defined. They
 * provide typical implementations that may be re-used by multiple
 * platforms but may also be overridden by a platform if required.
 */
#pragma weak bl31_plat_runtime_setup
#if !ERROR_DEPRECATED
#pragma weak plat_get_syscnt_freq2
#pragma weak bl31_early_platform_setup2
#endif /* ERROR_DEPRECATED */

#if SDEI_SUPPORT
#pragma weak plat_sdei_handle_masked_trigger
#pragma weak plat_sdei_validate_entry_point
#endif

#pragma weak plat_ea_handler

void bl31_plat_runtime_setup(void)
{
#if MULTI_CONSOLE_API
	console_switch_state(CONSOLE_FLAG_RUNTIME);
#else
	console_uninit();
#endif
}

#if !ENABLE_PLAT_COMPAT
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
#endif


#if !ERROR_DEPRECATED
unsigned int plat_get_syscnt_freq2(void)
{
	WARN("plat_get_syscnt_freq() is deprecated\n");
	WARN("Please define plat_get_syscnt_freq2()\n");
	/*
	 * Suppress deprecated declaration warning in compatibility function
	 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	unsigned long long freq = plat_get_syscnt_freq();
#pragma GCC diagnostic pop

	assert(freq >> 32 == 0);

	return (unsigned int)freq;
}

void bl31_early_platform_setup2(u_register_t arg0, u_register_t arg1,
			u_register_t arg2, u_register_t arg3)
{
	bl31_early_platform_setup((void *) arg0, (void *)arg1);
}
#endif /* ERROR_DEPRECATED */

#if SDEI_SUPPORT
/*
 * Function that handles spurious SDEI interrupts while events are masked.
 */
void plat_sdei_handle_masked_trigger(uint64_t mpidr, unsigned int intr)
{
	WARN("Spurious SDEI interrupt %u on masked PE %llx\n", intr, mpidr);
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

/* RAS functions common to AArch64 ARM platforms */
void plat_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
#if RAS_EXTENSION
	/* Call RAS EA handler */
	int handled = ras_ea_handler(ea_reason, syndrome, cookie, handle, flags);
	if (handled != 0)
		return;
#endif

	ERROR("Unhandled External Abort received on 0x%lx at EL3!\n",
			read_mpidr_el1());
	ERROR(" exception reason=%u syndrome=0x%llx\n", ea_reason, syndrome);
	panic();
}
