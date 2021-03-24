/*
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _JR_DRIVER_H_
#define _JR_DRIVER_H_

#include "jr_driver_config.h"

/* The maximum size of a SEC descriptor, in WORDs (32 bits). */
#define MAX_DESC_SIZE_WORDS		64

#define CAAM_TIMEOUT   200000	/* ms */

/* Return codes for JR user space driver APIs */
typedef enum sec_return_code_e {
	SEC_SUCCESS = 0,
	SEC_INVALID_INPUT_PARAM,
	SEC_OUT_OF_MEMORY,
	SEC_DESCRIPTOR_IN_FLIGHT,
	SEC_LAST_DESCRIPTOR_IN_FLIGHT,
	SEC_PROCESSING_ERROR,
	SEC_DESC_PROCESSING_ERROR,
	SEC_JR_IS_FULL,
	SEC_DRIVER_RELEASE_IN_PROGRESS,
	SEC_DRIVER_ALREADY_INITIALIZED,
	SEC_DRIVER_NOT_INITIALIZED,
	SEC_JOB_RING_RESET_IN_PROGRESS,
	SEC_RESET_ENGINE_FAILED,
	SEC_ENABLE_IRQS_FAILED,
	SEC_DISABLE_IRQS_FAILED,
	SEC_RETURN_CODE_MAX_VALUE,
} sec_return_code_t;

/* STRUCTURES AND OTHER TYPEDEFS */

/*
 * @brief Function called by JR User Space driver to notify every processed
 *         descriptor.
 *
 * Callback provided by the User Application.
 * Callback is invoked by JR User Space driver for each descriptor processed by
 * SEC
 * @param [in] status          Status word indicating processing result for
 *                                this descriptor.
 * @param [in] arg               Opaque data passed by User Application
 *                                It is opaque from JR driver's point of view.
 * @param [in] job_ring           The job ring handle on which the processed
 *                               descriptor word was enqueued
 */
typedef void (*user_callback) (uint32_t *desc, uint32_t status,
			       void *arg, void *job_ring);

/*
 * Structure encompassing a job descriptor which is to be processed
 * by SEC. User should also initialise this structure with the callback
 * function pointer which will be called by driver after recieving proccessed
 * descriptor from SEC. User data is also passed in this data structure which
 * will be sent as an argument to the user callback function.
 */
struct job_descriptor {
	uint32_t desc[MAX_DESC_SIZE_WORDS];
	void *arg;
	user_callback callback;
};

/*
 * @brief Initialize the JR User Space driver.
 * This function will handle initialization of sec library
 * along with registering platform specific callbacks,
 * as well as local data initialization.
 * Call once during application startup.
 * @note Global SEC initialization is done in SEC kernel driver.
 * @note The hardware IDs of the initialized Job Rings are opaque to the UA.
 * The exact Job Rings used by this library are decided between SEC user
 * space driver and SEC kernel driver. A static partitioning of Job Rings is
 * assumed, configured in DTS(device tree specification) file.
 * @param [in] platform_cb     Registering the platform specific
 *                             callbacks with driver
 * @retval ::0                 for successful execution
 * @retval ::-1                failure
 */
int sec_jr_lib_init(void);

/*
 * @brief Initialize the software and hardware resources tied to a job ring.
 * @param [in] jr_mode;        Model to be used by SEC Driver to receive
 *                             notifications from SEC.  Can be either
 *                             SEC_NOTIFICATION_TYPE_IRQ or
 *                             SEC_NOTIFICATION_TYPE_POLL
 * @param [in] irq_coalescing_timer This value determines the maximum
 *                                     amount of time after processing a
 *                                     descriptor before raising an interrupt.
 * @param [in] irq_coalescing_count This value determines how many
 *                                     descriptors are completed before
 *                                     raising an interrupt.
 * @param [in] reg_base_addr   The job ring base address register
 * @param [in] irq_id          The job ring interrupt identification number.
 * @retval  job_ring_handle for successful job ring configuration
 * @retval  NULL on error
 */
void *init_job_ring(uint8_t jr_mode,
		    uint16_t irq_coalescing_timer,
		    uint8_t irq_coalescing_count,
		    void *reg_base_addr, uint32_t irq_id);

/*
 * @brief Release the resources used by the JR User Space driver.
 * Reset and release SEC's job rings indicated by the User Application at
 * init_job_ring() and free any memory allocated internally.
 * Call once during application tear down.
 * @note In case there are any descriptors in-flight (descriptors received by
 * JR driver for processing and for which no response was yet provided to UA),
 * the descriptors are discarded without any notifications to User Application.
 * @retval ::0                 is returned for a successful execution
 * @retval ::-1                is returned if JR driver release is in progress
 */
int sec_release(void);

/*
 * @brief Submit a descriptor for SEC processing.
 * This function creates a "job" which is meant to instruct SEC HW
 * to perform the processing on the input buffer. The "job" is enqueued
 * in the Job Ring associated. The function will return after the "job"
 * enqueue is finished. The function will not wait for SEC to
 * start or/and finish the "job" processing.
 * After the processing is finished the SEC HW writes the processing result
 * to the provided output buffer.
 * The Caller must poll JR driver using jr_dequeue()
 * to receive notifications of the processing completion
 * status. The notifications are received by caller by means of callback
 * (see ::user_callback).
 * @param [in]  job_ring_handle   The handle of the job ring on which
 *                                descriptor is to be enqueued
 * @param [in]  job_descriptor    The job descriptor structure of type
 *                                struct job_descriptor. This structure
 *                                should be filled with job descriptor along
 *                                with callback function to be called after
 *                                processing of descriptor and some
 *                                opaque data passed to be passed to the
 *                                callback function
 *
 * @retval ::0                 is returned for successful execution
 * @retval ::-1                is returned if there is some enqueue failure
 */
int enq_jr_desc(void *job_ring_handle, struct job_descriptor *jobdescr);

/*
 * @brief Polls for available descriptors processed by SEC on a specific
 * Job Ring
 * This function polls the SEC Job Rings and delivers processed descriptors
 * Each processed descriptor has a user_callback registered.
 * This user_callback is invoked for each processed descriptor.
 * The polling is stopped when "limit" descriptors are notified or when
 * there are no more descriptors to notify.
 * @note The dequeue_jr() API cannot be called from within a user_callback
 * function
 * @param [in]  job_ring_handle    The Job Ring handle.
 * @param [in]  limit              This value represents the maximum number
 *                                 of processed descriptors that can be
 *                                 notified API call on this Job Ring.
 *                                 Note that fewer descriptors may be notified
 *                                 if enough processed descriptors are not
 *                                 available.
 *                                 If limit has a negative value, then all
 *                                 ready descriptors will be notified.
 *
 * @retval :: >=0                  is returned where retval is the total
 *                                 Number of descriptors notified
 *                                 during this function call.
 * @retval :: -1                   is returned in case of some error
 */
int dequeue_jr(void *job_ring_handle, int32_t limit);

#endif /* _JR_DRIVER_H_  */
