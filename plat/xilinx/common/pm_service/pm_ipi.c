/*
 * Copyright (c) 2013-2020, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2022, Xilinx, Inc. All rights reserved.
 * Copyright (c) 2022-2024, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <arch_helpers.h>
#include <lib/bakery_lock.h>
#include <lib/mmio.h>
#include <lib/spinlock.h>
#include <plat/common/platform.h>

#include <ipi.h>
#include <plat_ipi.h>
#include <plat_private.h>
#include "pm_defs.h"
#include "pm_ipi.h"

#define ERROR_CODE_MASK		(0xFFFFU)
#define PM_OFFSET		(0U)

/*
 * ARM v8.2, the cache will turn off automatically when cpu
 * power down. Therefore, there is no doubt to use the spin_lock here.
 */
#if !HW_ASSISTED_COHERENCY
DEFINE_BAKERY_LOCK(pm_secure_lock);
static inline void pm_ipi_lock_get(void)
{
	bakery_lock_get(&pm_secure_lock);
}

static inline void pm_ipi_lock_release(void)
{
	bakery_lock_release(&pm_secure_lock);
}
#else
spinlock_t pm_secure_lock;
static inline void pm_ipi_lock_get(void)
{
	spin_lock(&pm_secure_lock);
}

static inline void pm_ipi_lock_release(void)
{
	spin_unlock(&pm_secure_lock);
}
#endif

/**
 * pm_ipi_init() - Initialize IPI peripheral for communication with
 *                 remote processor.
 * @proc: Pointer to the processor who is initiating request.
 *
 * Return: On success, the initialization function must return 0.
 *         Any other return value will cause the framework to ignore
 *         the service.
 *
 * Called from pm_setup initialization function.
 */
void pm_ipi_init(const struct pm_proc *proc)
{
	ipi_mb_open(proc->ipi->local_ipi_id, proc->ipi->remote_ipi_id);
}

/**
 * pm_ipi_send_common() - Sends IPI request to the remote processor.
 * @proc: Pointer to the processor who is initiating request.
 * @payload: API id and call arguments to be written in IPI buffer.
 * @is_blocking: if to trigger the notification in blocking mode or not.
 *
 * Send an IPI request to the power controller. Caller needs to hold
 * the 'pm_secure_lock' lock.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ipi_send_common(const struct pm_proc *proc,
					     uint32_t payload[PAYLOAD_ARG_CNT],
					     uint32_t is_blocking)
{
	uint32_t offset = PM_OFFSET;
	uintptr_t buffer_base = proc->ipi->buffer_base +
					IPI_BUFFER_TARGET_REMOTE_OFFSET +
					IPI_BUFFER_REQ_OFFSET;
#if IPI_CRC_CHECK
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
 *                              without blocking notification.
 * @proc: Pointer to the processor who is initiating request.
 * @payload: API id and call arguments to be written in IPI buffer.
 *
 * Send an IPI request to the power controller.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_ipi_send_non_blocking(const struct pm_proc *proc,
					    uint32_t payload[PAYLOAD_ARG_CNT])
{
	enum pm_ret_status ret;

	pm_ipi_lock_get();

	ret = pm_ipi_send_common(proc, payload, IPI_NON_BLOCKING);

	pm_ipi_lock_release();

	return ret;
}

/**
 * pm_ipi_send() - Sends IPI request to the remote processor.
 * @proc: Pointer to the processor who is initiating request.
 * @payload: API id and call arguments to be written in IPI buffer.
 *
 * Send an IPI request to the power controller.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
enum pm_ret_status pm_ipi_send(const struct pm_proc *proc,
			       uint32_t payload[PAYLOAD_ARG_CNT])
{
	enum pm_ret_status ret;

	pm_ipi_lock_get();

	ret = pm_ipi_send_common(proc, payload, IPI_BLOCKING);

	pm_ipi_lock_release();

	return ret;
}


/**
 * pm_ipi_buff_read() - Reads IPI response after remote processor has handled
 *                      interrupt.
 * @proc: Pointer to the processor who is waiting and reading response.
 * @value: Used to return value from IPI buffer element (optional).
 * @count: Number of values to return in @value.
 *
 * Return: Returns status, either success or error+reason.
 *
 */
static enum pm_ret_status pm_ipi_buff_read(const struct pm_proc *proc,
					   uint32_t *value, size_t count)
{
	size_t i;
	enum pm_ret_status ret;
#if IPI_CRC_CHECK
	uint32_t crc;
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
	for (i = 0U; i < count; i++) {
		value[i] = mmio_read_32(buffer_base + ((i + 1U) * PAYLOAD_ARG_SIZE));
	}

	ret = mmio_read_32(buffer_base);
#if IPI_CRC_CHECK
	crc = mmio_read_32(buffer_base + (PAYLOAD_CRC_POS * PAYLOAD_ARG_SIZE));
	if (crc != calculate_crc((uint32_t *)buffer_base, IPI_W0_TO_W6_SIZE)) {
		NOTICE("ERROR in CRC response payload value:0x%x\n", crc);
		ret = PM_RET_ERROR_INVALID_CRC;
		/* Payload data is invalid as CRC validation failed
		 * Clear the payload to avoid leakage of data to upper layers
		 */
		memset(value, 0, count);
	}
#endif

	return ret;
}

/**
 * pm_ipi_buff_read_callb() - Callback function that reads value from
 *                            ipi response buffer.
 * @value: Used to return value from IPI buffer element.
 * @count: Number of values to return in @value.
 *
 * This callback function fills requested data in @value from ipi response
 * buffer.
 *
 * Return: Returns status, either success or error.
 *
 */
enum pm_ret_status pm_ipi_buff_read_callb(uint32_t *value, size_t count)
{
	size_t i;
	size_t local_count = count;
#if IPI_CRC_CHECK
	uint32_t crc;
#endif
	uintptr_t buffer_base = IPI_BUFFER_REMOTE_BASE +
				IPI_BUFFER_TARGET_LOCAL_OFFSET +
				IPI_BUFFER_REQ_OFFSET;
	enum pm_ret_status ret = PM_RET_SUCCESS;

	if (local_count > IPI_BUFFER_MAX_WORDS) {
		local_count = IPI_BUFFER_MAX_WORDS;
	}

	for (i = 0; i < count; i++) {
		value[i] = mmio_read_32(buffer_base + (i * PAYLOAD_ARG_SIZE));
	}
#if IPI_CRC_CHECK
	crc = mmio_read_32(buffer_base + (PAYLOAD_CRC_POS * PAYLOAD_ARG_SIZE));
	if (crc != calculate_crc((uint32_t *)buffer_base, IPI_W0_TO_W6_SIZE)) {
		NOTICE("ERROR in CRC response payload value:0x%x\n", crc);
		ret = PM_RET_ERROR_INVALID_CRC;
		/* Payload data is invalid as CRC validation failed
		 * Clear the payload to avoid leakage of data to upper layers
		 */
		memset(value, 0, local_count);
	}
#endif
	return ret;
}

/**
 * pm_ipi_send_sync() - Sends IPI request to the remote processor.
 * @proc: Pointer to the processor who is initiating request.
 * @payload: API id and call arguments to be written in IPI buffer.
 * @value: Used to return value from IPI buffer element (optional).
 * @count: Number of values to return in @value.
 *
 * Send an IPI request to the power controller and wait for it to be handled.
 *
 * Return: Returns status, either success or error+reason and, optionally,
 *         @value.
 *
 */
enum pm_ret_status pm_ipi_send_sync(const struct pm_proc *proc,
				    uint32_t payload[PAYLOAD_ARG_CNT],
				    uint32_t *value, size_t count)
{
	enum pm_ret_status ret;

	pm_ipi_lock_get();

	ret = pm_ipi_send_common(proc, payload, IPI_BLOCKING);
	if (ret != PM_RET_SUCCESS) {
		goto unlock;
	}

	ret = ERROR_CODE_MASK & (pm_ipi_buff_read(proc, value, count));

unlock:
	pm_ipi_lock_release();

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
	int32_t ret;

	ret = ipi_mb_enquire_status(proc->ipi->local_ipi_id,
				    proc->ipi->remote_ipi_id);
	if (ret & IPI_MB_STATUS_RECV_PENDING) {
		return 1;
	} else {
		return 0;
	}
}

#if IPI_CRC_CHECK
uint32_t calculate_crc(uint32_t payload[PAYLOAD_ARG_CNT], uint32_t bufsize)
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
