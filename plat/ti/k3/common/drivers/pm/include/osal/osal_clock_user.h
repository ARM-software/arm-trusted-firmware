/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef OSAL_CLK_H
#define OSAL_CLK_H

#include <types/short_types.h>

/**
 * \brief Cause the task to enter blocked state and not be scheduled until
 *	  `usecs` elapses. This is provided for convenience but will still
 *	  actually use clock_ticks for the backend implementation, meaning
 *	  the actual valid values will be limited to the system tick period
 *	  and any aligned to the tick boundary will be rounded up to the next
 *	  nearest tick.
 *
 * \param msecs Amount of time to wait until task is scheduled again.
 */
void osal_sleep_us(uint32_t msecs);

/**
 * \brief Cause the task to enter blocked state and not be scheduled until
 *	  `clock_ticks` elapses. The default time for a clock tick is 1000uS.
 *
 * \param clock_ticks Number of ticks of system clock to wait until task
 *	  is scheduled again.
 */
void osal_sleep_ticks(uint32_t clock_ticks);

/**
 * \brief Block inside the current task for the specified amount of time.
 *
 * \param usecs Amount of time to block before continuing execution. Max delay
 *		 is ((2^64 - 1)/1000) microseconds.
 */
void osal_delay(uint64_t usecs);

/**
 * \brief Block inside the current task for the specified amount of time.
 *
 * \param usecs Amount of time to block before continuing execution. Max delay
 *		 is ((2^64 - 1)/1000) microseconds.
 *
 * NOTE: Able to be used in no interrupt context.
 */
void osal_delay_core(uint64_t usecs);

/**
 * \brief Return time in ns since system clock was started as 32-bit number.
 *
 * This returns a 32-bit number representing ns since syystem clock start
 * and wraps around once overflow occurs.
 *
 * \return 0 if timer is not yet initialized or ns since clock start.
 */
uint32_t osal_timestamp32(void);

/**
 * \brief Return time in ns since system clock was started as 32-bit number.
 *
 * This returns a 32-bit number representing ns since syystem clock start
 * and wraps around once overflow occurs.
 *
 * \return 0 if timer is not yet initialized or ns since clock start.
 *
 * NOTE: Able to be used in no interrupt context.
 */
uint32_t osal_timestamp32_core(void);

/**
 * \brief Return time in ns since system clock was started as 64-bit number.
 *
 * This returns a 64-bit number representing ns since syystem clock start
 * and wraps around once overflow occurs.
 *
 * \return 0 if timer is not yet initialized or ns since clock start.
 */
uint64_t osal_timestamp64(void);

/**
 * \brief Return time in ns since system clock was started as 64-bit number.
 *
 * This returns a 64-bit number representing ns since syystem clock start
 * and wraps around once overflow occurs.
 *
 * \return 0 if timer is not yet initialized or ns since clock start.
 *
 * NOTE: Able to be used in no interrupt context.
 */
uint64_t osal_timestamp64_core(void);

/**
 * \brief Return true if OSAL time functions are initialized.
 *
 * \return true if OSAL time functions are initialized, false otherwise.
 */
bool osal_time_is_initialized(void);

#endif /* OSAL_CLK_H */
