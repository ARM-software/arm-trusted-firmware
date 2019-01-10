/*
 * Copyright (c) 2013-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Contains definitions of commonly used macros and data types needed
 * for PU Power Management. This file should be common for all PU's.
 */

#ifndef PM_COMMON_H
#define PM_COMMON_H

#include <stdint.h>
#include <plat_pm_common.h>

/**
 * pm_ipi - struct for capturing IPI-channel specific info
 * @local_ipi_id	Local IPI agent ID
 * @remote_ipi_id	Remote IPI Agent ID
 * @buffer_base	base address for payload buffer
 */
struct pm_ipi {
	const uint32_t local_ipi_id;
	const uint32_t remote_ipi_id;
	const uintptr_t buffer_base;
};

/**
 * pm_proc - struct for capturing processor related info
 * @node_id	node-ID of the processor
 * @pwrdn_mask	cpu-specific mask to be used for power control register
 * @ipi		pointer to IPI channel structure
 *		(in APU all processors share one IPI channel)
 */
struct pm_proc {
	const uint32_t node_id;
	const unsigned int pwrdn_mask;
	const struct pm_ipi *ipi;
};

const struct pm_proc *pm_get_proc(unsigned int cpuid);

#endif /* PM_COMMON_H */
