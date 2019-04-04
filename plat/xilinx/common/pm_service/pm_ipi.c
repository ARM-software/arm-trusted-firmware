/*
 * Copyright (c) 2013-2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <arch_helpers.h>

#include <lib/bakery_lock.h>
#include <lib/mmio.h>

#include <ipi.h>
#include <plat_ipi.h>
#include <plat_private.h>
#include <plat/common/platform.h>

#include "pm_ipi.h"


DEFINE_BAKERY_LOCK(pm_secure_lock);

/**
 * pm_ipi_init() - Initialize IPI peripheral for communication with
 *		   remote processor
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
	ipi_mb_open(proc->ipi->local_ipi_id, proc->ipi->remote_ipi_id);

	return 0;
}

/**
 * pm_ipi_send_common() - Sends IPI request to the remote processor
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
					IPI_BUFFER_TARGET_REMOTE_OFFSET +
					IPI_BUFFER_REQ_OFFSET;
#if ZYNQMP_IPI_CRC_CHECK
	payload[PAYLOAD_CRC_POS] = calculate_crc(payload, IPI_W0_TO_W6_SIZE);
#endif

	/* Write payload into IPI buffer */
	for (size_t i = 0; i < PAYLOAD_ARG_CNT; i++) {
		mmio_write_32(buffer_base + offset, payload[i]);
		offset += PAYLOAD_ARG_SIZE;
	}

	/* Generate IPI to remote processor */
	ipi_mb_notify(proc->ipi->local_ipi_id, proc->ipi->remote_ipi_id,
		      is_blocking);

	return PM_RET_SUCCESS;
}

/**
 * pm_ipi_send_non_blocking() - Sends IPI request to the remote processor
 *			        without blocking notification
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
 * pm_ipi_send() - Sends IPI request to the remote processor
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
 * pm_ipi_buff_read() - Reads IPI response after remote processor has handled
 *			interrupt
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
#if ZYNQMP_IPI_CRC_CHECK
	size_t j;
	unsigned int response_payload[PAYLOAD_ARG_CNT];
#endif
	uintptr_t buffer_base = proc->ipi->buffer_base +
				IPI_BUFFER_TARGET_REMOTE_OFFSET +
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
#if ZYNQMP_IPI_CRC_CHECK
	for (j = 0; j < PAYLOAD_ARG_CNT; j++)
		response_payload[j] = mmio_read_32(buffer_base +
						(j * PAYLOAD_ARG_SIZE));

	if (response_payload[PAYLOAD_CRC_POS] !=
			calculate_crc(response_payload, IPI_W0_TO_W6_SIZE))
		NOTICE("ERROR in CRC response payload value:0x%x\n",
					response_payload[PAYLOAD_CRC_POS]);
#endif

	return mmio_read_32(buffer_base);
}

/**
 * pm_ipi_buff_read_callb() - Reads IPI response after remote processor has
 *			      handled interrupt
 * @value	Used to return value from IPI buffer element (optional)
 * @count	Number of values to return in @value
 *
 * @return	Returns status, either success or error+reason
 */
void pm_ipi_buff_read_callb(unsigned int *value, size_t count)
{
	size_t i;
#if ZYNQMP_IPI_CRC_CHECK
	size_t j;
	unsigned int response_payload[PAYLOAD_ARG_CNT];
#endif
	uintptr_t buffer_base = IPI_BUFFER_REMOTE_BASE +
				IPI_BUFFER_TARGET_LOCAL_OFFSET +
				IPI_BUFFER_REQ_OFFSET;

	if (count > IPI_BUFFER_MAX_WORDS)
		count = IPI_BUFFER_MAX_WORDS;

	for (i = 0; i <= count; i++) {
		*value = mmio_read_32(buffer_base + (i * PAYLOAD_ARG_SIZE));
		value++;
	}
#if ZYNQMP_IPI_CRC_CHECK
	for (j = 0; j < PAYLOAD_ARG_CNT; j++)
		response_payload[j] = mmio_read_32(buffer_base +
						(j * PAYLOAD_ARG_SIZE));

	if (response_payload[PAYLOAD_CRC_POS] !=
			calculate_crc(response_payload, IPI_W0_TO_W6_SIZE))
		NOTICE("ERROR in CRC response payload value:0x%x\n",
					response_payload[PAYLOAD_CRC_POS]);
#endif
}

/**
 * pm_ipi_send_sync() - Sends IPI request to the remote processor
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
	ipi_mb_enable_irq(proc->ipi->local_ipi_id, proc->ipi->remote_ipi_id);
}

void pm_ipi_irq_clear(const struct pm_proc *proc)
{
	ipi_mb_ack(proc->ipi->local_ipi_id, proc->ipi->remote_ipi_id);
}

uint32_t pm_ipi_irq_status(const struct pm_proc *proc)
{
	int ret;

	ret = ipi_mb_enquire_status(proc->ipi->local_ipi_id,
				    proc->ipi->remote_ipi_id);
	if (ret & IPI_MB_STATUS_RECV_PENDING)
		return 1;
	else
		return 0;
}

#if ZYNQMP_IPI_CRC_CHECK
uint32_t calculate_crc(uint32_t *payload, uint32_t bufsize)
{
	uint32_t crcinit = CRC_INIT_VALUE;
	uint32_t order   = CRC_ORDER;
	uint32_t polynom = CRC_POLYNOM;
	uint32_t i, j, c, bit, datain, crcmask, crchighbit;
	uint32_t crc = crcinit;

	crcmask = ((uint32_t)((1U << (order - 1U)) - 1U) << 1U) | 1U;
	crchighbit = (uint32_t)(1U << (order - 1U));

	for (i = 0U; i < bufsize; i++) {
		datain = mmio_read_8((unsigned long)payload + i);
		c = datain;
		j = 0x80U;
		while (j != 0U) {
			bit = crc & crchighbit;
			crc <<= 1U;
			if (0U != (c & j))
				bit ^= crchighbit;
			if (bit != 0U)
				crc ^= polynom;
			j >>= 1U;
		}
		crc &= crcmask;
	}
	return crc;
}
#endif
