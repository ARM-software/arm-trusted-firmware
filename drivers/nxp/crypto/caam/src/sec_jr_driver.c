/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arch_helpers.h>
#include "caam.h"
#include <common/debug.h>
#include "jobdesc.h"
#include "nxp_timer.h"
#include "sec_hw_specific.h"
#include "sec_jr_driver.h"


/* Job rings used for communication with SEC HW  */
struct sec_job_ring_t g_job_rings[MAX_SEC_JOB_RINGS];

/* The current state of SEC user space driver */
volatile sec_driver_state_t g_driver_state = SEC_DRIVER_STATE_IDLE;

int g_job_rings_no;

uint8_t ip_ring[SEC_DMA_MEM_INPUT_RING_SIZE] __aligned(CACHE_WRITEBACK_GRANULE);
uint8_t op_ring[SEC_DMA_MEM_OUTPUT_RING_SIZE] __aligned(CACHE_WRITEBACK_GRANULE);

void *init_job_ring(uint8_t jr_mode,
		    uint16_t irq_coalescing_timer,
		    uint8_t irq_coalescing_count,
		    void *reg_base_addr, uint32_t irq_id)
{
	struct sec_job_ring_t *job_ring = &g_job_rings[g_job_rings_no++];
	int ret = 0;

	job_ring->register_base_addr = reg_base_addr;
	job_ring->jr_mode = jr_mode;
	job_ring->irq_fd = irq_id;

	job_ring->input_ring = vtop(ip_ring);
	memset(job_ring->input_ring, 0, SEC_DMA_MEM_INPUT_RING_SIZE);

	job_ring->output_ring = (struct sec_outring_entry *)vtop(op_ring);
	memset(job_ring->output_ring, 0, SEC_DMA_MEM_OUTPUT_RING_SIZE);

	dsb();

#if defined(SEC_MEM_NON_COHERENT) && defined(IMAGE_BL2)
	flush_dcache_range((uintptr_t)(job_ring->input_ring),
				       SEC_DMA_MEM_INPUT_RING_SIZE),
	flush_dcache_range((uintptr_t)(job_ring->output_ring),
				       SEC_DMA_MEM_OUTPUT_RING_SIZE),

	dmbsy();
#endif
	/* Reset job ring in SEC hw and configure job ring registers */
	ret = hw_reset_job_ring(job_ring);
	if (ret != 0) {
		ERROR("Failed to reset hardware job ring\n");
		return NULL;
	}

	if (jr_mode == SEC_NOTIFICATION_TYPE_IRQ) {
		/* Enable IRQ if driver work sin interrupt mode */
		ERROR("Enabling DONE IRQ generation on job ring\n");
		ret = jr_enable_irqs(job_ring);
		if (ret != 0) {
			ERROR("Failed to enable irqs for job ring\n");
			return NULL;
		}
	}
	if ((irq_coalescing_timer != 0) || (irq_coalescing_count != 0)) {
		hw_job_ring_set_coalescing_param(job_ring,
						 irq_coalescing_timer,
						 irq_coalescing_count);

		hw_job_ring_enable_coalescing(job_ring);
		job_ring->coalescing_en = 1;
	}

	job_ring->jr_state = SEC_JOB_RING_STATE_STARTED;

	return job_ring;
}

int sec_release(void)
{
	int i;

	/* Validate driver state */
	if (g_driver_state == SEC_DRIVER_STATE_RELEASE) {
		ERROR("Driver release is already in progress");
		return SEC_DRIVER_RELEASE_IN_PROGRESS;
	}
	/* Update driver state */
	g_driver_state = SEC_DRIVER_STATE_RELEASE;

	/* If any descriptors in flight , poll and wait
	 * until all descriptors are received and silently discarded.
	 */

	flush_job_rings();

	for (i = 0; i < g_job_rings_no; i++) {
		shutdown_job_ring(&g_job_rings[i]);
	}
	g_job_rings_no = 0;
	g_driver_state = SEC_DRIVER_STATE_IDLE;

	return SEC_SUCCESS;
}

int sec_jr_lib_init(void)
{
	/* Validate driver state */
	if (g_driver_state != SEC_DRIVER_STATE_IDLE) {
		ERROR("Driver already initialized\n");
		return 0;
	}

	memset(g_job_rings, 0, sizeof(g_job_rings));
	g_job_rings_no = 0;

	/* Update driver state */
	g_driver_state = SEC_DRIVER_STATE_STARTED;
	return 0;
}

int dequeue_jr(void *job_ring_handle, int32_t limit)
{
	int ret = 0;
	int notified_descs_no = 0;
	struct sec_job_ring_t *job_ring = (sec_job_ring_t *) job_ring_handle;
	uint64_t start_time;

	/* Validate driver state */
	if (g_driver_state != SEC_DRIVER_STATE_STARTED) {
		ERROR("Driver release in progress or driver not initialized\n");
		return -1;
	}

	/* Validate input arguments */
	if (job_ring == NULL) {
		ERROR("job_ring_handle is NULL\n");
		return -1;
	}
	if (((limit == 0) || (limit > SEC_JOB_RING_SIZE))) {
		ERROR("Invalid limit parameter configuration\n");
		return -1;
	}

	VERBOSE("JR Polling limit[%d]\n", limit);

	/* Poll job ring
	 * If limit < 0 -> poll JR until no more notifications are available.
	 * If limit > 0 -> poll JR until limit is reached.
	 */

	start_time = get_timer_val(0);

	while (notified_descs_no == 0) {
		/* Run hw poll job ring */
		notified_descs_no = hw_poll_job_ring(job_ring, limit);
		if (notified_descs_no < 0) {
			ERROR("Error polling SEC engine job ring ");
			return notified_descs_no;
		}
		VERBOSE("Jobs notified[%d]. ", notified_descs_no);

		if (get_timer_val(start_time) >= CAAM_TIMEOUT) {
			break;
		}
	}

	if (job_ring->jr_mode == SEC_NOTIFICATION_TYPE_IRQ) {

		/* Always enable IRQ generation when in pure IRQ mode */
		ret = jr_enable_irqs(job_ring);
		if (ret != 0) {
			ERROR("Failed to enable irqs for job ring");
			return ret;
		}
	}
	return notified_descs_no;
}

int enq_jr_desc(void *job_ring_handle, struct job_descriptor *jobdescr)
{
	struct sec_job_ring_t *job_ring;

	job_ring = (struct sec_job_ring_t *)job_ring_handle;

	/* Validate driver state */
	if (g_driver_state != SEC_DRIVER_STATE_STARTED) {
		ERROR("Driver release in progress or driver not initialized\n");
		return -1;
	}

	/* Check job ring state */
	if (job_ring->jr_state != SEC_JOB_RING_STATE_STARTED) {
		ERROR("Job ring is currently resetting\n");
		return -1;
	}

	if (SEC_JOB_RING_IS_FULL(job_ring->pidx, job_ring->cidx,
				 SEC_JOB_RING_SIZE, SEC_JOB_RING_SIZE)) {
		ERROR("Job ring is full\n");
		return -1;
	}

	/* Set ptr in input ring to current descriptor  */
	sec_write_addr(&job_ring->input_ring[job_ring->pidx],
		       (phys_addr_t) vtop(jobdescr->desc));

	dsb();

#if defined(SEC_MEM_NON_COHERENT) && defined(IMAGE_BL2)
	flush_dcache_range((uintptr_t)(&job_ring->input_ring[job_ring->pidx]),
			   sizeof(phys_addr_t));

	inv_dcache_range((uintptr_t)(&job_ring->output_ring[job_ring->cidx]),
			   sizeof(struct sec_outring_entry));
	dmbsy();
#endif
	/* Notify HW that a new job is enqueued  */
	hw_enqueue_desc_on_job_ring(
			(struct jobring_regs *)job_ring->register_base_addr, 1);

	/* increment the producer index for the current job ring */
	job_ring->pidx = SEC_CIRCULAR_COUNTER(job_ring->pidx,
					      SEC_JOB_RING_SIZE);

	return 0;
}
