/*
 * Copyright (c) 2014-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <drivers/console.h>
#if ENABLE_FEAT_RAS
#include <lib/extensions/ras.h>
#endif
#include <lib/xlat_tables/xlat_mmu_helpers.h>
#include <plat/common/platform.h>

/* Pointer and function to register platform function to load alernate images */
const struct plat_try_images_ops *plat_try_img_ops;

void plat_setup_try_img_ops(const struct plat_try_images_ops *plat_try_ops)
{
	plat_try_img_ops = plat_try_ops;
}

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

#if FFH_SUPPORT
#pragma weak plat_ea_handler = plat_default_ea_handler
#endif

void bl31_plat_runtime_setup(void)
{
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

const char *get_el_str(unsigned int el)
{
	const char *mode = NULL;

	switch (el) {
	case MODE_EL3:
		mode = "EL3";
		break;
	case MODE_EL2:
		mode = "EL2";
		break;
	case MODE_EL1:
		mode = "EL1";
		break;
	case MODE_EL0:
		mode = "EL0";
		break;
	default:
		assert(false);
		break;
	}

	return mode;
}

#if FFH_SUPPORT
/* Handler for External Aborts from lower EL including RAS errors */
void plat_default_ea_handler(unsigned int ea_reason, uint64_t syndrome, void *cookie,
		void *handle, uint64_t flags)
{
#if ENABLE_FEAT_RAS
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

	/* We reached here due to a panic from a lower EL and assuming this is the default
	 * platform registered handler that we could call on a lower EL panic.
	 */
	lower_el_panic();
}
#endif
