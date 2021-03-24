/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <arch_helpers.h>
#include "caam.h"
#include <common/debug.h>
#include "jobdesc.h"
#include "sec_hw_specific.h"


/* Job rings used for communication with SEC HW */
extern struct sec_job_ring_t g_job_rings[MAX_SEC_JOB_RINGS];

/* The current state of SEC user space driver */
extern volatile sec_driver_state_t g_driver_state;

/* The number of job rings used by SEC user space driver */
extern int g_job_rings_no;

/* LOCAL FUNCTIONS */
static inline void hw_set_input_ring_start_addr(struct jobring_regs *regs,
						phys_addr_t *start_addr)
{
#if defined(CONFIG_PHYS_64BIT)
	sec_out32(&regs->irba_h, PHYS_ADDR_HI(start_addr));
#else
	sec_out32(&regs->irba_h, 0);
#endif
	sec_out32(&regs->irba_l, PHYS_ADDR_LO(start_addr));
}

static inline void hw_set_output_ring_start_addr(struct jobring_regs *regs,
						 phys_addr_t *start_addr)
{
#if defined(CONFIG_PHYS_64BIT)
	sec_out32(&regs->orba_h, PHYS_ADDR_HI(start_addr));
#else
	sec_out32(&regs->orba_h, 0);
#endif
	sec_out32(&regs->orba_l, PHYS_ADDR_LO(start_addr));
}

/* ORJR - Output Ring Jobs Removed Register shows how many jobs were
 * removed from the Output Ring for processing by software. This is done after
 * the software has processed the entries.
 */
static inline void hw_remove_entries(sec_job_ring_t *jr, int num)
{
	struct jobring_regs *regs =
	    (struct jobring_regs *)jr->register_base_addr;

	sec_out32(&regs->orjr, num);
}

/* IRSA - Input Ring Slots Available register holds the number of entries in
 * the Job Ring's input ring. Once a job is enqueued, the value returned is
 * decremented by the hardware by the number of jobs enqueued.
 */
static inline int hw_get_available_slots(sec_job_ring_t *jr)
{
	struct jobring_regs *regs =
	    (struct jobring_regs *)jr->register_base_addr;

	return sec_in32(&regs->irsa);
}

/* ORSFR - Output Ring Slots Full register holds the number of jobs which were
 * processed by the SEC and can be retrieved by the software. Once a job has
 * been processed by software, the user will call hw_remove_one_entry in order
 * to notify the SEC that the entry was processed
 */
static inline int hw_get_no_finished_jobs(sec_job_ring_t *jr)
{
	struct jobring_regs *regs =
	    (struct jobring_regs *)jr->register_base_addr;

	return sec_in32(&regs->orsf);
}

/* @brief Process Jump Halt Condition related errors
 * @param [in]  error_code The error code in the descriptor status word
 */
static inline void hw_handle_jmp_halt_cond_err(union hw_error_code error_code)
{
	ERROR("JMP %x\n", error_code.error_desc.jmp_halt_cond_src.jmp);
	ERROR("Descriptor Index: %d\n",
	      error_code.error_desc.jmp_halt_cond_src.desc_idx);
	ERROR(" Condition %x\n", error_code.error_desc.jmp_halt_cond_src.cond);
}

/* @brief Process DECO related errors
 * @param [in]  error_code      The error code in the descriptor status word
 */
static inline void hw_handle_deco_err(union hw_error_code error_code)
{
	ERROR("JMP %x\n", error_code.error_desc.deco_src.jmp);
	ERROR("Descriptor Index: 0x%x",
	      error_code.error_desc.deco_src.desc_idx);

	switch (error_code.error_desc.deco_src.desc_err) {
	case SEC_HW_ERR_DECO_HFN_THRESHOLD:
		WARN(" Descriptor completed but exceeds the Threshold");
		break;
	default:
		ERROR("Error 0x%04x not implemented",
		      error_code.error_desc.deco_src.desc_err);
		break;
	}
}

/* @brief Process  Jump Halt User Status related errors
 * @param [in]  error_code      The error code in the descriptor status word
 */
static inline void hw_handle_jmp_halt_user_err(union hw_error_code error_code)
{
	WARN(" Not implemented");
}

/* @brief Process CCB related errors
 * @param [in]  error_code      The error code in the descriptor status word
 */
static inline void hw_handle_ccb_err(union hw_error_code hw_error_code)
{
	WARN(" Not implemented");
}

/* @brief Process Job Ring related errors
 * @param [in]  error_code      The error code in the descriptor status word
 */
static inline void hw_handle_jr_err(union hw_error_code hw_error_code)
{
	WARN(" Not implemented");
}

/* GLOBAL FUNCTIONS */

int hw_reset_job_ring(sec_job_ring_t *job_ring)
{
	int ret = 0;
	struct jobring_regs *regs =
	    (struct jobring_regs *)job_ring->register_base_addr;

	/* First reset the job ring in hw */
	ret = hw_shutdown_job_ring(job_ring);
	if (ret != 0) {
		ERROR("Failed resetting job ring in hardware");
		return ret;
	}
	/* In order to have the HW JR in a workable state
	 *after a reset, I need to re-write the input
	 * queue size, input start address, output queue
	 * size and output start address
	 * Write the JR input queue size to the HW register
	 */
	sec_out32(&regs->irs, SEC_JOB_RING_SIZE);

	/* Write the JR output queue size to the HW register */
	sec_out32(&regs->ors, SEC_JOB_RING_SIZE);

	/* Write the JR input queue start address */
	hw_set_input_ring_start_addr(regs, vtop(job_ring->input_ring));

	/* Write the JR output queue start address */
	hw_set_output_ring_start_addr(regs, vtop(job_ring->output_ring));

	return 0;
}

int hw_shutdown_job_ring(sec_job_ring_t *job_ring)
{
	struct jobring_regs *regs =
	    (struct jobring_regs *)job_ring->register_base_addr;
	unsigned int timeout = SEC_TIMEOUT;
	uint32_t tmp = 0U;

	VERBOSE("Resetting Job ring\n");

	/*
	 * Mask interrupts since we are going to poll
	 * for reset completion status
	 * Also, at POR, interrupts are ENABLED on a JR, thus
	 * this is the point where I can disable them without
	 * changing the code logic too much
	 */

	jr_disable_irqs(job_ring);

	/* initiate flush (required prior to reset) */
	sec_out32(&regs->jrcr, JR_REG_JRCR_VAL_RESET);

	/* dummy read */
	tmp = sec_in32(&regs->jrcr);

	do {
		tmp = sec_in32(&regs->jrint);
	} while (((tmp & JRINT_ERR_HALT_MASK) ==
		  JRINT_ERR_HALT_INPROGRESS) && ((--timeout) != 0U));

	if ((tmp & JRINT_ERR_HALT_MASK) != JRINT_ERR_HALT_COMPLETE ||
	    timeout == 0U) {
		ERROR("Failed to flush hw job ring %x\n %u", tmp, timeout);
		/* unmask interrupts */
		if (job_ring->jr_mode != SEC_NOTIFICATION_TYPE_POLL) {
			jr_enable_irqs(job_ring);
		}
		return -1;
	}
	/* Initiate reset */
	timeout = SEC_TIMEOUT;
	sec_out32(&regs->jrcr, JR_REG_JRCR_VAL_RESET);

	do {
		tmp = sec_in32(&regs->jrcr);
	} while (((tmp & JR_REG_JRCR_VAL_RESET) != 0U) &&
		 ((--timeout) != 0U));

	if (timeout == 0U) {
		ERROR("Failed to reset hw job ring\n");
		/* unmask interrupts */
		if (job_ring->jr_mode != SEC_NOTIFICATION_TYPE_POLL) {
			jr_enable_irqs(job_ring);
		}
		return -1;
	}
	/* unmask interrupts */
	if (job_ring->jr_mode != SEC_NOTIFICATION_TYPE_POLL) {
		jr_enable_irqs(job_ring);
	}
	return 0;

}

void hw_handle_job_ring_error(sec_job_ring_t *job_ring, uint32_t error_code)
{
	union hw_error_code hw_err_code;

	hw_err_code.error = error_code;

	switch (hw_err_code.error_desc.value.ssrc) {
	case SEC_HW_ERR_SSRC_NO_SRC:
		INFO("No Status Source ");
		break;
	case SEC_HW_ERR_SSRC_CCB_ERR:
		INFO("CCB Status Source");
		hw_handle_ccb_err(hw_err_code);
		break;
	case SEC_HW_ERR_SSRC_JMP_HALT_U:
		INFO("Jump Halt User Status Source");
		hw_handle_jmp_halt_user_err(hw_err_code);
		break;
	case SEC_HW_ERR_SSRC_DECO:
		INFO("DECO Status Source");
		hw_handle_deco_err(hw_err_code);
		break;
	case SEC_HW_ERR_SSRC_JR:
		INFO("Job Ring Status Source");
		hw_handle_jr_err(hw_err_code);
		break;
	case SEC_HW_ERR_SSRC_JMP_HALT_COND:
		INFO("Jump Halt Condition Codes");
		hw_handle_jmp_halt_cond_err(hw_err_code);
		break;
	default:
		INFO("Unknown SSRC");
		break;
	}
}

int hw_job_ring_error(sec_job_ring_t *job_ring)
{
	uint32_t jrint_error_code;
	struct jobring_regs *regs =
	    (struct jobring_regs *)job_ring->register_base_addr;

	if (JR_REG_JRINT_JRE_EXTRACT(sec_in32(&regs->jrint)) == 0) {
		return 0;
	}

	jrint_error_code =
	    JR_REG_JRINT_ERR_TYPE_EXTRACT(sec_in32(&regs->jrint));
	switch (jrint_error_code) {
	case JRINT_ERR_WRITE_STATUS:
		ERROR("Error writing status to Output Ring ");
		break;
	case JRINT_ERR_BAD_INPUT_BASE:
		ERROR("Bad Input Ring Base (not on a 4-byte boundary)\n");
		break;
	case JRINT_ERR_BAD_OUTPUT_BASE:
		ERROR("Bad Output Ring Base (not on a 4-byte boundary)\n");
		break;
	case JRINT_ERR_WRITE_2_IRBA:
		ERROR("Invalid write to Input Ring Base Address Register\n");
		break;
	case JRINT_ERR_WRITE_2_ORBA:
		ERROR("Invalid write to Output Ring Base Address Register\n");
		break;
	case JRINT_ERR_RES_B4_HALT:
		ERROR("Job Ring released before Job Ring is halted\n");
		break;
	case JRINT_ERR_REM_TOO_MANY:
		ERROR("Removed too many jobs from job ring\n");
		break;
	case JRINT_ERR_ADD_TOO_MANY:
		ERROR("Added too many jobs on job ring\n");
		break;
	default:
		ERROR("Unknown SEC JR Error :%d\n", jrint_error_code);
		break;
	}
	return jrint_error_code;
}

int hw_job_ring_set_coalescing_param(sec_job_ring_t *job_ring,
				     uint16_t irq_coalescing_timer,
				     uint8_t irq_coalescing_count)
{
	uint32_t reg_val = 0U;
	struct jobring_regs *regs =
	    (struct jobring_regs *)job_ring->register_base_addr;

	/* Set descriptor count coalescing */
	reg_val |= (irq_coalescing_count << JR_REG_JRCFG_LO_ICDCT_SHIFT);

	/* Set coalescing timer value */
	reg_val |= (irq_coalescing_timer << JR_REG_JRCFG_LO_ICTT_SHIFT);

	/* Update parameters in HW */
	sec_out32(&regs->jrcfg1, reg_val);

	VERBOSE("Set coalescing params on jr\n");

	return 0;
}

int hw_job_ring_enable_coalescing(sec_job_ring_t *job_ring)
{
	uint32_t reg_val = 0U;
	struct jobring_regs *regs =
	    (struct jobring_regs *)job_ring->register_base_addr;

	/* Get the current value of the register */
	reg_val = sec_in32(&regs->jrcfg1);

	/* Enable coalescing */
	reg_val |= JR_REG_JRCFG_LO_ICEN_EN;

	/* Write in hw */
	sec_out32(&regs->jrcfg1, reg_val);

	VERBOSE("Enabled coalescing on jr\n");

	return 0;
}

int hw_job_ring_disable_coalescing(sec_job_ring_t *job_ring)
{
	uint32_t reg_val = 0U;
	struct jobring_regs *regs =
	    (struct jobring_regs *)job_ring->register_base_addr;

	/* Get the current value of the register */
	reg_val = sec_in32(&regs->jrcfg1);

	/* Disable coalescing */
	reg_val &= ~JR_REG_JRCFG_LO_ICEN_EN;

	/* Write in hw */
	sec_out32(&regs->jrcfg1, reg_val);

	VERBOSE("Disabled coalescing on jr");

	return 0;

}

void hw_flush_job_ring(struct sec_job_ring_t *job_ring,
		       uint32_t do_notify,
		       uint32_t error_code, uint32_t *notified_descs)
{
	int32_t jobs_no_to_discard = 0;
	int32_t discarded_descs_no = 0;
	int32_t number_of_jobs_available = 0;

	VERBOSE("JR pi[%d]i ci[%d]\n", job_ring->pidx, job_ring->cidx);
	VERBOSE("error code %x\n", error_code);
	VERBOSE("Notify_desc = %d\n", do_notify);

	number_of_jobs_available = hw_get_no_finished_jobs(job_ring);

	/* Discard all jobs */
	jobs_no_to_discard = number_of_jobs_available;

	VERBOSE("JR pi[%d]i ci[%d]\n", job_ring->pidx, job_ring->cidx);
	VERBOSE("Discarding desc = %d\n", jobs_no_to_discard);

	while (jobs_no_to_discard > discarded_descs_no) {
		discarded_descs_no++;
		/* Now increment the consumer index for the current job ring,
		 * AFTER saving job in temporary location!
		 * Increment the consumer index for the current job ring
		 */

		job_ring->cidx = SEC_CIRCULAR_COUNTER(job_ring->cidx,
						      SEC_JOB_RING_SIZE);

		hw_remove_entries(job_ring, 1);
	}

	if (do_notify == true) {
		if (notified_descs == NULL) {
			return;
		}
		*notified_descs = discarded_descs_no;
	}
}

/* return >0 in case of success
 *  -1 in case of error from SEC block
 *  0 in case job not yet processed by SEC
 *   or  Descriptor returned is NULL after dequeue
 */
int hw_poll_job_ring(struct sec_job_ring_t *job_ring, int32_t limit)
{
	int32_t jobs_no_to_notify = 0;
	int32_t number_of_jobs_available = 0;
	int32_t notified_descs_no = 0;
	uint32_t error_descs_no = 0U;
	uint32_t sec_error_code = 0U;
	uint32_t do_driver_shutdown = false;
	phys_addr_t *fnptr, *arg_addr;
	user_callback usercall = NULL;
	uint8_t *current_desc;
	void *arg;
	uintptr_t current_desc_addr;
	phys_addr_t current_desc_loc;

#if defined(SEC_MEM_NON_COHERENT) && defined(IMAGE_BL2)
	inv_dcache_range((uintptr_t)job_ring->register_base_addr, sizeof(struct jobring_regs));
	dmbsy();
#endif

	/* check here if any JR error that cannot be written
	 * in the output status word has occurred
	 */
	sec_error_code = hw_job_ring_error(job_ring);
	if (unlikely(sec_error_code) != 0) {
		ERROR("Error here itself %x\n", sec_error_code);
		return -1;
	}
	/* Compute the number of notifications that need to be raised to UA
	 * If limit < 0 -> notify all done jobs
	 * If limit > total number of done jobs -> notify all done jobs
	 * If limit = 0 -> error
	 * If limit > 0 && limit < total number of done jobs -> notify a number
	 * of done jobs equal with limit
	 */

	/*compute the number of jobs available in the job ring based on the
	 * producer and consumer index values.
	 */

	number_of_jobs_available = hw_get_no_finished_jobs(job_ring);
	jobs_no_to_notify = (limit < 0 || limit > number_of_jobs_available) ?
	    number_of_jobs_available : limit;
	VERBOSE("JR - pi %d, ci %d, ", job_ring->pidx, job_ring->cidx);
	VERBOSE("Jobs submitted %d", number_of_jobs_available);
	VERBOSE("Jobs to notify %d\n", jobs_no_to_notify);

	while (jobs_no_to_notify > notified_descs_no) {

#if defined(SEC_MEM_NON_COHERENT) && defined(IMAGE_BL2)
		inv_dcache_range(
			(uintptr_t)(&job_ring->output_ring[job_ring->cidx]),
			sizeof(struct sec_outring_entry));
		dmbsy();
#endif

		/* Get job status here */
		sec_error_code =
		    sec_in32(&(job_ring->output_ring[job_ring->cidx].status));

		/* Get completed descriptor
		 */
		current_desc_loc = (uintptr_t)
		    &job_ring->output_ring[job_ring->cidx].desc;
		current_desc_addr = sec_read_addr(current_desc_loc);

		current_desc = ptov((phys_addr_t *) current_desc_addr);
		if (current_desc == 0) {
			ERROR("No descriptor returned from SEC");
			assert(current_desc);
			return 0;
		}
		/* now increment the consumer index for the current job ring,
		 * AFTER saving job in temporary location!
		 */
		job_ring->cidx = SEC_CIRCULAR_COUNTER(job_ring->cidx,
						      SEC_JOB_RING_SIZE);

		if (sec_error_code != 0) {
			ERROR("desc at cidx %d\n ", job_ring->cidx);
			ERROR("generated error %x\n", sec_error_code);

			sec_handle_desc_error(job_ring,
					      sec_error_code,
					      &error_descs_no,
					      &do_driver_shutdown);
			hw_remove_entries(job_ring, 1);

			return -1;
		}
		/* Signal that the job has been processed & the slot is free */
		hw_remove_entries(job_ring, 1);
		notified_descs_no++;

		arg_addr = (phys_addr_t *) (current_desc +
				(MAX_DESC_SIZE_WORDS * sizeof(uint32_t)));

		fnptr = (phys_addr_t *) (current_desc +
					(MAX_DESC_SIZE_WORDS * sizeof(uint32_t)
					+  sizeof(void *)));

		arg = (void *)*(arg_addr);
		if (*fnptr != 0) {
			VERBOSE("Callback Function called\n");
			usercall = (user_callback) *(fnptr);
			(*usercall) ((uint32_t *) current_desc,
				     sec_error_code, arg, job_ring);
		}
	}

	return notified_descs_no;
}

void sec_handle_desc_error(sec_job_ring_t *job_ring,
			   uint32_t sec_error_code,
			   uint32_t *notified_descs,
			   uint32_t *do_driver_shutdown)
{
	/* Analyze the SEC error on this job ring */
	hw_handle_job_ring_error(job_ring, sec_error_code);
}

void flush_job_rings(void)
{
	struct sec_job_ring_t *job_ring = NULL;
	int i = 0;

	for (i = 0; i < g_job_rings_no; i++) {
		job_ring = &g_job_rings[i];
		/* Producer index is frozen. If consumer index is not equal
		 * with producer index, then we have descs to flush.
		 */
		while (job_ring->pidx != job_ring->cidx) {
			hw_flush_job_ring(job_ring, false, 0,	/* no error */
					  NULL);
		}
	}
}

int shutdown_job_ring(struct sec_job_ring_t *job_ring)
{
	int ret = 0;

	ret = hw_shutdown_job_ring(job_ring);
	if (ret != 0) {
		ERROR("Failed to shutdown hardware job ring\n");
		return ret;
	}

	if (job_ring->coalescing_en != 0) {
		hw_job_ring_disable_coalescing(job_ring);
	}

	if (job_ring->jr_mode != SEC_NOTIFICATION_TYPE_POLL) {
		ret = jr_disable_irqs(job_ring);
		if (ret != 0) {
			ERROR("Failed to disable irqs for job ring");
			return ret;
		}
	}

	return 0;
}

int jr_enable_irqs(struct sec_job_ring_t *job_ring)
{
	uint32_t reg_val = 0U;
	struct jobring_regs *regs =
	    (struct jobring_regs *)job_ring->register_base_addr;

	/* Get the current value of the register */
	reg_val = sec_in32(&regs->jrcfg1);

	/* Enable interrupts by disabling interrupt masking*/
	reg_val &= ~JR_REG_JRCFG_LO_IMSK_EN;

	/* Update parameters in HW */
	sec_out32(&regs->jrcfg1, reg_val);

	VERBOSE("Enable interrupts on JR\n");

	return 0;
}

int jr_disable_irqs(struct sec_job_ring_t *job_ring)
{
	uint32_t reg_val = 0U;
	struct jobring_regs *regs =
	    (struct jobring_regs *)job_ring->register_base_addr;

	/* Get the current value of the register */
	reg_val = sec_in32(&regs->jrcfg1);

	/* Disable interrupts by enabling interrupt masking*/
	reg_val |= JR_REG_JRCFG_LO_IMSK_EN;

	/* Update parameters in HW */
	sec_out32(&regs->jrcfg1, reg_val);

	VERBOSE("Disable interrupts on JR\n");

	return 0;
}
