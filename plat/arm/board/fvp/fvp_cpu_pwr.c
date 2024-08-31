/*
 * Copyright (c) 2024, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if __aarch64__

#include <aem_generic.h>
#include <arch_helpers.h>
#include <cortex_a35.h>
#include <cortex_a53.h>
#include <cortex_a57.h>
#include <cortex_a72.h>
#include <cortex_a73.h>
#include <cortex_a78_ae.h>
#include <drivers/arm/fvp/fvp_cpu_pwr.h>
#include <lib/utils_def.h>
#include <neoverse_e1.h>

bool check_cpupwrctrl_el1_is_available(void)
{
	/* Poupulate list of CPU midr that doesn't support CPUPWRCTL_EL1 */
	const unsigned int midr_no_cpupwrctl[] = {
		BASE_AEM_MIDR,
		CORTEX_A35_MIDR,
		CORTEX_A53_MIDR,
		CORTEX_A57_MIDR,
		CORTEX_A72_MIDR,
		CORTEX_A73_MIDR,
		CORTEX_A78_AE_MIDR,
		NEOVERSE_E1_MIDR
	};
	unsigned int midr = (unsigned int)read_midr();

	for (unsigned int i = 0U; i < ARRAY_SIZE(midr_no_cpupwrctl); i++) {
		if (midr_no_cpupwrctl[i] == midr) {
			return false;
		}
	}

	return true;
}

#endif /* __arch64__ */
