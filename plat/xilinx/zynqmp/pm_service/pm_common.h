/*
 * Copyright (c) 2013-2015, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
