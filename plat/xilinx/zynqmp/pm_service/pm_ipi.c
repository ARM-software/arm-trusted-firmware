/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <bakery_lock.h>
#include <mmio.h>
#include <platform.h>
#include "../zynqmp_ipi.h"
#include "../zynqmp_private.h"
#include "pm_ipi.h"

/* IPI message buffers */
#define IPI_BUFFER_BASEADDR	0xFF990000U

#define IPI_BUFFER_APU_BASE	(IPI_BUFFER_BASEADDR + 0x400U)
#define IPI_BUFFER_PMU_BASE	(IPI_BUFFER_BASEADDR + 0xE00U)

#define IPI_BUFFER_TARGET_APU_OFFSET	0x80U
#define IPI_BUFFER_TARGET_PMU_OFFSET	0x1C0U

#define IPI_BUFFER_MAX_WORDS	8

#define IPI_BUFFER_REQ_OFFSET	0x0U
#define IPI_BUFFER_RESP_OFFSET	0x20U

#define IPI_BLOCKING		1
#define IPI_NON_BLOCKING	0

DEFINE_BAKERY_LOCK(pm_secure_lock);

const struct pm_ipi apu_ipi = {
	.apu_ipi_id = IPI_ID_APU,
	.pmu_ipi_id = IPI_ID_PMU0,
	.buffer_base = IPI_BUFFER_APU_BASE,
};

/**
 * pm_ipi_init() - Initialize IPI peripheral for communication with PMU
 *
 * @proc	Pointer to the processor who is initiating request
 * @return	On success, the initialization function must return 0.
 *		Any other return value will cause the framework to ignore
 *		the service
 *
 * Called from pm_setup initialization function
 */
int pm_ipi_init(const struct pm_proc *proc)
{
	bakery_lock_init(&pm_secure_lock);
	ipi_mb_open(proc->ipi->apu_ipi_id, proc->ipi->pmu_ipi_id);

	return 0;
}

/**
 * pm_ipi_send_common() - Sends IPI request to the PMU
 * @proc	Pointer to the processor who is initiating request
 * @payload	API id and call arguments to be written in IPI buffer
 *
 * Send an IPI request to the power controller. Caller needs to hold
 * the 'pm_secure_lock' lock.
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_ipi_send_common(const struct pm_proc *proc,
					     uint32_t payload[PAYLOAD_ARG_CNT],
					     uint32_t is_blocking)
{
	unsigned int offset = 0;
	uintptr_t buffer_base = proc->ipi->buffer_base +
					IPI_BUFFER_TARGET_PMU_OFFSET +
					IPI_BUFFER_REQ_OFFSET;

	/* Write payload into IPI buffer */
	for (size_t i = 0; i < PAYLOAD_ARG_CNT; i++) {
		mmio_write_32(buffer_base + offset, payload[i]);
		offset += PAYLOAD_ARG_SIZE;
	}

	/* Generate IPI to PMU */
	ipi_mb_notify(proc->ipi->apu_ipi_id, proc->ipi->pmu_ipi_id,
		      is_blocking);

	return PM_RET_SUCCESS;
}

/**
 * pm_ipi_send_non_blocking() - Sends IPI request to the PMU without blocking
 *			        notification
 * @proc	Pointer to the processor who is initiating request
 * @payload	API id and call arguments to be written in IPI buffer
 *
 * Send an IPI request to the power controller.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_ipi_send_non_blocking(const struct pm_proc *proc,
					    uint32_t payload[PAYLOAD_ARG_CNT])
{
	enum pm_ret_status ret;

	bakery_lock_get(&pm_secure_lock);

	ret = pm_ipi_send_common(proc, payload, IPI_NON_BLOCKING);

	bakery_lock_release(&pm_secure_lock);

	return ret;
}

/**
 * pm_ipi_send() - Sends IPI request to the PMU
 * @proc	Pointer to the processor who is initiating request
 * @payload	API id and call arguments to be written in IPI buffer
 *
 * Send an IPI request to the power controller.
 *
 * @return	Returns status, either success or error+reason
 */
enum pm_ret_status pm_ipi_send(const struct pm_proc *proc,
			       uint32_t payload[PAYLOAD_ARG_CNT])
{
	enum pm_ret_status ret;

	bakery_lock_get(&pm_secure_lock);

	ret = pm_ipi_send_common(proc, payload, IPI_BLOCKING);

	bakery_lock_release(&pm_secure_lock);

	return ret;
}


/**
 * pm_ipi_buff_read() - Reads IPI response after PMU has handled interrupt
 * @proc	Pointer to the processor who is waiting and reading response
 * @value	Used to return value from IPI buffer element (optional)
 * @count	Number of values to return in @value
 *
 * @return	Returns status, either success or error+reason
 */
static enum pm_ret_status pm_ipi_buff_read(const struct pm_proc *proc,
					   unsigned int *value, size_t count)
{
	size_t i;
	uintptr_t buffer_base = proc->ipi->buffer_base +
				IPI_BUFFER_TARGET_PMU_OFFSET +
				IPI_BUFFER_RESP_OFFSET;

	/*
	 * Read response from IPI buffer
	 * buf-0: success or error+reason
	 * buf-1: value
	 * buf-2: unused
	 * buf-3: unused
	 */
	for (i = 1; i <= count; i++) {
		*value = mmio_read_32(buffer_base + (i * PAYLOAD_ARG_SIZE));
		value++;
	}

	return mmio_read_32(buffer_base);
}

/**
 * pm_ipi_buff_read_callb() - Reads IPI response after PMU has handled interrupt
 * @value	Used to return value from IPI buffer element (optional)
 * @count	Number of values to return in @value
 *
 * @return	Returns status, either success or error+reason
 */
void pm_ipi_buff_read_callb(unsigned int *value, size_t count)
{
	size_t i;
	uintptr_t buffer_base = IPI_BUFFER_PMU_BASE +
				IPI_BUFFER_TARGET_APU_OFFSET +
				IPI_BUFFER_REQ_OFFSET;

	if (count > IPI_BUFFER_MAX_WORDS)
		count = IPI_BUFFER_MAX_WORDS;

	for (i = 0; i <= count; i++) {
		*value = mmio_read_32(buffer_base + (i * PAYLOAD_ARG_SIZE));
		value++;
	}
}

/**
 * pm_ipi_send_sync() - Sends IPI request to the PMU
 * @proc	Pointer to the processor who is initiating request
 * @payload	API id and call arguments to be written in IPI buffer
 * @value	Used to return value from IPI buffer element (optional)
 * @count	Number of values to return in @value
 *
 * Send an IPI request to the power controller and wait for it to be handled.
 *
 * @return	Returns status, either success or error+reason and, optionally,
 *		@value
 */
enum pm_ret_status pm_ipi_send_sync(const struct pm_proc *proc,
				    uint32_t payload[PAYLOAD_ARG_CNT],
				    unsigned int *value, size_t count)
{
	enum pm_ret_status ret;

	bakery_lock_get(&pm_secure_lock);

	ret = pm_ipi_send_common(proc, payload, IPI_BLOCKING);
	if (ret != PM_RET_SUCCESS)
		goto unlock;

	ret = pm_ipi_buff_read(proc, value, count);

unlock:
	bakery_lock_release(&pm_secure_lock);

	return ret;
}

void pm_ipi_irq_enable(const struct pm_proc *proc)
{
	ipi_mb_enable_irq(proc->ipi->apu_ipi_id, proc->ipi->pmu_ipi_id);
}

void pm_ipi_irq_clear(const struct pm_proc *proc)
{
	ipi_mb_ack(proc->ipi->apu_ipi_id, proc->ipi->pmu_ipi_id);
}
