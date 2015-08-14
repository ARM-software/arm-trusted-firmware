/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
 *
 * Virtual Open Systems SAS, 32-bit Secure Payload Dispatcher power management
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

#include <arch_helpers.h>
#include <assert.h>
#include <bl_common.h>
#include <context_mgmt.h>
#include <debug.h>
#include "tspd_aarch32_private.h"

#define MPIDR_CPU0	0x80000000

/*******************************************************************************
 * Return the type of TSP the TSPD is dealing with. Report the current resident
 * cpu (mpidr format) if it is a UP/UP migratable TSP.
 ******************************************************************************/
static int32_t tspd_cpu_migrate_info(uint64_t *resident_cpu)
{
	/* The payload runs only on CPU0 */
	*resident_cpu = MPIDR_CPU0;

	/* TSP bookkeeping not yet implemented so no migrate capable payload */
	return TSP_MIGRATE_INFO;
}

/*******************************************************************************
 * Structure populated by the TSP Dispatcher to be given a chance to perform any
 * TSP bookkeeping before PSCI executes a power mgmt.  operation.
 ******************************************************************************/
const spd_pm_ops_t tspd_pm = {
	.svc_on = NULL,
	.svc_off = NULL,
	.svc_suspend = NULL,
	.svc_on_finish = NULL,
	.svc_suspend_finish = NULL,
	.svc_migrate = NULL,
	.svc_migrate_info = tspd_cpu_migrate_info,
	.svc_system_off = NULL,
	.svc_system_reset = NULL
};
