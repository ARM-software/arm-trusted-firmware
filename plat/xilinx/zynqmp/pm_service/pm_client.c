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
 * APU specific definition of processors in the subsystem as well as functions
 * for getting information about and changing state of the APU.
 */

#include <gicv2.h>
#include <bl_common.h>
#include <mmio.h>
#include "pm_api_sys.h"
#include "pm_client.h"
#include "pm_ipi.h"
#include "../zynqmp_def.h"

#define OCM_BANK_0	0xFFFC0000
#define OCM_BANK_1	(OCM_BANK_0 + 0x10000)
#define OCM_BANK_2	(OCM_BANK_1 + 0x10000)
#define OCM_BANK_3	(OCM_BANK_2 + 0x10000)

#define UNDEFINED_CPUID		(~0)

/* Declaration of linker defined symbol */
extern unsigned long __BL31_END__;
extern const struct pm_ipi apu_ipi;

/* Order in pm_procs_all array must match cpu ids */
static const struct pm_proc const pm_procs_all[] = {
	{
		.node_id = NODE_APU_0,
		.pwrdn_mask = APU_0_PWRCTL_CPUPWRDWNREQ_MASK,
		.ipi = &apu_ipi,
	},
	{
		.node_id = NODE_APU_1,
		.pwrdn_mask = APU_1_PWRCTL_CPUPWRDWNREQ_MASK,
		.ipi = &apu_ipi,
	},
	{
		.node_id = NODE_APU_2,
		.pwrdn_mask = APU_2_PWRCTL_CPUPWRDWNREQ_MASK,
		.ipi = &apu_ipi,
	},
	{
		.node_id = NODE_APU_3,
		.pwrdn_mask = APU_3_PWRCTL_CPUPWRDWNREQ_MASK,
		.ipi = &apu_ipi,
	},
};

/**
 * set_ocm_retention() - Configure OCM memory banks for retention
 *
 * APU specific requirements for suspend action:
 * OCM has to enter retention state in order to preserve saved
 * context after suspend request. OCM banks are determined by
 * __BL31_END__ linker symbol.
 *
 * Return:	Returns status, either success or error+reason
 */
enum pm_ret_status set_ocm_retention(void)
{
	enum pm_ret_status ret;

	/* OCM_BANK_0 will always be occupied */
	ret = pm_set_requirement(NODE_OCM_BANK_0, PM_CAP_CONTEXT, 0,
				 REQ_ACK_NO);

	/* Check for other OCM banks  */
	if ((unsigned long)&__BL31_END__ >= OCM_BANK_1)
		ret = pm_set_requirement(NODE_OCM_BANK_1, PM_CAP_CONTEXT, 0,
					 REQ_ACK_NO);
	if ((unsigned long)&__BL31_END__ >= OCM_BANK_2)
		ret = pm_set_requirement(NODE_OCM_BANK_2, PM_CAP_CONTEXT, 0,
					 REQ_ACK_NO);
	if ((unsigned long)&__BL31_END__ >= OCM_BANK_3)
		ret = pm_set_requirement(NODE_OCM_BANK_3, PM_CAP_CONTEXT, 0,
					 REQ_ACK_NO);

	return ret;
}

/**
 * pm_get_proc() - returns pointer to the proc structure
 * @cpuid:	id of the cpu whose proc struct pointer should be returned
 *
 * Return: pointer to a proc structure if proc is found, otherwise NULL
 */
const struct pm_proc *pm_get_proc(unsigned int cpuid)
{
	if (cpuid < ARRAY_SIZE(pm_procs_all))
		return &pm_procs_all[cpuid];

	return NULL;
}

/**
 * pm_get_proc_by_node() - returns pointer to the proc structure
 * @nid:	node id of the processor
 *
 * Return: pointer to a proc structure if proc is found, otherwise NULL
 */
const struct pm_proc *pm_get_proc_by_node(enum pm_node_id nid)
{
	for (size_t i = 0; i < ARRAY_SIZE(pm_procs_all); i++) {
		if (nid == pm_procs_all[i].node_id)
			return &pm_procs_all[i];
	}
	return NULL;
}

/**
 * pm_get_cpuid() - get the local cpu ID for a global node ID
 * @nid:	node id of the processor
 *
 * Return: the cpu ID (starting from 0) for the subsystem
 */
static unsigned int pm_get_cpuid(enum pm_node_id nid)
{
	for (size_t i = 0; i < ARRAY_SIZE(pm_procs_all); i++) {
		if (pm_procs_all[i].node_id == nid)
			return i;
	}
	return UNDEFINED_CPUID;
}

const struct pm_proc *primary_proc = &pm_procs_all[0];

/**
 * pm_client_suspend() - Client-specific suspend actions
 *
 * This function should contain any PU-specific actions
 * required prior to sending suspend request to PMU
 */
void pm_client_suspend(const struct pm_proc *proc)
{
	/* Set powerdown request */
	mmio_write_32(APU_PWRCTL, mmio_read_32(APU_PWRCTL) | proc->pwrdn_mask);
}


/**
 * pm_client_abort_suspend() - Client-specific abort-suspend actions
 *
 * This function should contain any PU-specific actions
 * required for aborting a prior suspend request
 */
void pm_client_abort_suspend(void)
{
	/* Enable interrupts at processor level (for current cpu) */
	gicv2_cpuif_enable();
	/* Clear powerdown request */
	mmio_write_32(APU_PWRCTL,
		 mmio_read_32(APU_PWRCTL) & ~primary_proc->pwrdn_mask);
}

/**
 * pm_client_wakeup() - Client-specific wakeup actions
 *
 * This function should contain any PU-specific actions
 * required for waking up another APU core
 */
void pm_client_wakeup(const struct pm_proc *proc)
{
	unsigned int cpuid = pm_get_cpuid(proc->node_id);

	if (cpuid == UNDEFINED_CPUID)
		return;

	/* clear powerdown bit for affected cpu */
	uint32_t val = mmio_read_32(APU_PWRCTL);
	val &= ~(proc->pwrdn_mask);
	mmio_write_32(APU_PWRCTL, val);
}
