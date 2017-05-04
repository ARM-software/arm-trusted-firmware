/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Contains definitions of commonly used macros and data types needed
 * for PU Power Management. This file should be common for all PU's.
 */

#ifndef _PM_COMMON_H_
#define _PM_COMMON_H_

#include <debug.h>
#include <stdint.h>
#include "pm_defs.h"

#define PAYLOAD_ARG_CNT		6U
#define PAYLOAD_ARG_SIZE	4U	/* size in bytes */

/**
 * pm_ipi - struct for capturing IPI-channel specific info
 * @mask	mask for enabling/disabling and triggering the IPI
 * @base	base address for IPI
 * @buffer_base	base address for payload buffer
 */
struct pm_ipi {
	const unsigned int mask;
	const uintptr_t base;
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
	const enum pm_node_id node_id;
	const unsigned int pwrdn_mask;
	const struct pm_ipi *ipi;
};

const struct pm_proc *pm_get_proc(unsigned int cpuid);
const struct pm_proc *pm_get_proc_by_node(enum pm_node_id nid);

#endif /* _PM_COMMON_H_ */
