/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * Header file containing the public API for the System Controller (SC)
 * Timer function.
 *
 * @addtogroup TIMER_SVC (SVC) Timer Service
 *
 * Module for the Timer service. This includes support for the watchdog, RTC,
 * and system counter. Note every resource partition has a watchdog it can
 * use.
 *
 * @{
 */

#ifndef SC_TIMER_API_H
#define SC_TIMER_API_H

/* Includes */

#include <sci/sci_types.h>

/* Defines */

/*!
 * @name Defines for type widths
 */
/*@{*/
#define SC_TIMER_ACTION_W   3U	/* Width of sc_timer_wdog_action_t */
/*@}*/

/*!
 * @name Defines for sc_timer_wdog_action_t
 */
/*@{*/
#define SC_TIMER_WDOG_ACTION_PARTITION      0U	/* Reset partition */
#define SC_TIMER_WDOG_ACTION_WARM           1U	/* Warm reset system */
#define SC_TIMER_WDOG_ACTION_COLD           2U	/* Cold reset system */
#define SC_TIMER_WDOG_ACTION_BOARD          3U	/* Reset board */
#define SC_TIMER_WDOG_ACTION_IRQ            4U	/* Only generate IRQs */
/*@}*/

/* Types */

/*!
 * This type is used to configure the watchdog action.
 */
typedef uint8_t sc_timer_wdog_action_t;

/*!
 * This type is used to declare a watchdog time value in milliseconds.
 */
typedef uint32_t sc_timer_wdog_time_t;

/* Functions */

/*!
 * @name Watchdog Functions
 * @{
 */

/*!
 * This function sets the watchdog timeout in milliseconds. If not
 * set then the timeout defaults to the max. Once locked this value
 * cannot be changed.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     timeout     timeout period for the watchdog
 *
 * @return Returns an error code (SC_ERR_NONE = success, SC_ERR_LOCKED
 *         = locked).
 */
sc_err_t sc_timer_set_wdog_timeout(sc_ipc_t ipc, sc_timer_wdog_time_t timeout);

/*!
 * This function sets the watchdog pre-timeout in milliseconds. If not
 * set then the pre-timeout defaults to the max. Once locked this value
 * cannot be changed.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pre_timeout pre-timeout period for the watchdog
 *
 * When the pre-timeout expires an IRQ will be generated. Note this timeout
 * clears when the IRQ is triggered. An IRQ is generated for the failing
 * partition and all of its child partitions.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 */
sc_err_t sc_timer_set_wdog_pre_timeout(sc_ipc_t ipc,
				       sc_timer_wdog_time_t pre_timeout);

/*!
 * This function starts the watchdog.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     lock        boolean indicating the lock status
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * If \a lock is set then the watchdog cannot be stopped or the timeout
 * period changed.
 */
sc_err_t sc_timer_start_wdog(sc_ipc_t ipc, sc_bool_t lock);

/*!
 * This function stops the watchdog if it is not locked.
 *
 * @param[in]     ipc         IPC handle
 *
 * @return Returns an error code (SC_ERR_NONE = success, SC_ERR_LOCKED
 *         = locked).
 */
sc_err_t sc_timer_stop_wdog(sc_ipc_t ipc);

/*!
 * This function pings (services, kicks) the watchdog resetting the time
 * before expiration back to the timeout.
 *
 * @param[in]     ipc         IPC handle
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 */
sc_err_t sc_timer_ping_wdog(sc_ipc_t ipc);

/*!
 * This function gets the status of the watchdog. All arguments are
 * in milliseconds.
 *
 * @param[in]     ipc             IPC handle
 * @param[out]    timeout         pointer to return the timeout
 * @param[out]    max_timeout     pointer to return the max timeout
 * @param[out]    remaining_time  pointer to return the time remaining
 *                                until trigger
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 */
sc_err_t sc_timer_get_wdog_status(sc_ipc_t ipc,
				  sc_timer_wdog_time_t *timeout,
				  sc_timer_wdog_time_t *max_timeout,
				  sc_timer_wdog_time_t *remaining_time);

/*!
 * This function gets the status of the watchdog of a partition. All
 * arguments are in milliseconds.
 *
 * @param[in]     ipc             IPC handle
 * @param[in]     pt              partition to query
 * @param[out]    enb             pointer to return enable status
 * @param[out]    timeout         pointer to return the timeout
 * @param[out]    remaining_time  pointer to return the time remaining
 *                                until trigger
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 */
sc_err_t sc_timer_pt_get_wdog_status(sc_ipc_t ipc, sc_rm_pt_t pt,
				     sc_bool_t *enb,
				     sc_timer_wdog_time_t *timeout,
				     sc_timer_wdog_time_t *remaining_time);

/*!
 * This function configures the action to be taken when a watchdog
 * expires.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          partition to affect
 * @param[in]     action      action to take
 *
 * Default action is inherited from the parent.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid parameters,
 * - SC_ERR_NOACCESS if caller's partition is not the SYSTEM owner,
 * - SC_ERR_LOCKED if the watchdog is locked
 */
sc_err_t sc_timer_set_wdog_action(sc_ipc_t ipc,
				  sc_rm_pt_t pt, sc_timer_wdog_action_t action);

/* @} */

/*!
 * @name Real-Time Clock (RTC) Functions
 * @{
 */

/*!
 * This function sets the RTC time. Only the owner of the SC_R_SYSTEM
 * resource can set the time.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     year        year (min 1970)
 * @param[in]     mon         month (1-12)
 * @param[in]     day         day of the month (1-31)
 * @param[in]     hour        hour (0-23)
 * @param[in]     min         minute (0-59)
 * @param[in]     sec         second (0-59)
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid time/date parameters,
 * - SC_ERR_NOACCESS if caller's partition is not the SYSTEM owner
 */
sc_err_t sc_timer_set_rtc_time(sc_ipc_t ipc, uint16_t year, uint8_t mon,
			       uint8_t day, uint8_t hour, uint8_t min,
			       uint8_t sec);

/*!
 * This function gets the RTC time.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    year        pointer to return year (min 1970)
 * @param[out]    mon         pointer to return month (1-12)
 * @param[out]    day         pointer to return day of the month (1-31)
 * @param[out]    hour        pointer to return hour (0-23)
 * @param[out]    min         pointer to return minute (0-59)
 * @param[out]    sec         pointer to return second (0-59)
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 */
sc_err_t sc_timer_get_rtc_time(sc_ipc_t ipc, uint16_t *year, uint8_t *mon,
			       uint8_t *day, uint8_t *hour, uint8_t *min,
			       uint8_t *sec);

/*!
 * This function gets the RTC time in seconds since 1/1/1970.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    sec         pointer to return second
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 */
sc_err_t sc_timer_get_rtc_sec1970(sc_ipc_t ipc, uint32_t *sec);

/*!
 * This function sets the RTC alarm.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     year        year (min 1970)
 * @param[in]     mon         month (1-12)
 * @param[in]     day         day of the month (1-31)
 * @param[in]     hour        hour (0-23)
 * @param[in]     min         minute (0-59)
 * @param[in]     sec         second (0-59)
 *
 * Note this alarm setting clears when the alarm is triggered.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid time/date parameters
 */
sc_err_t sc_timer_set_rtc_alarm(sc_ipc_t ipc, uint16_t year, uint8_t mon,
				uint8_t day, uint8_t hour, uint8_t min,
				uint8_t sec);

/*!
 * This function sets the RTC alarm (periodic mode).
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     sec         period in seconds
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid time/date parameters
 */
sc_err_t sc_timer_set_rtc_periodic_alarm(sc_ipc_t ipc, uint32_t sec);

/*!
 * This function cancels the RTC alarm.
 *
 * @param[in]     ipc         IPC handle
 *
 * Note this alarm setting clears when the alarm is triggered.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid time/date parameters
 */
sc_err_t sc_timer_cancel_rtc_alarm(sc_ipc_t ipc);

/*!
 * This function sets the RTC calibration value. Only the owner of the SC_R_SYSTEM
 * resource can set the calibration.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     count       calbration count (-16 to 15)
 *
 * The calibration value is a 5-bit value including the sign bit, which is
 * implemented in 2's complement. It is added or subtracted from the RTC on
 * a perdiodic basis, once per 32768 cycles of the RTC clock.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 */
sc_err_t sc_timer_set_rtc_calb(sc_ipc_t ipc, int8_t count);

/* @} */

/*!
 * @name System Counter (SYSCTR) Functions
 * @{
 */

/*!
 * This function sets the SYSCTR alarm.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     ticks       number of 8MHz cycles
 *
 * Note this alarm setting clears when the alarm is triggered.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid time/date parameters
 */
sc_err_t sc_timer_set_sysctr_alarm(sc_ipc_t ipc, uint64_t ticks);

/*!
 * This function sets the SYSCTR alarm (periodic mode).
 *
 * @param[in]     ipc          IPC handle
 * @param[in]     ticks        number of 8MHz cycles
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid time/date parameters
 */
sc_err_t sc_timer_set_sysctr_periodic_alarm(sc_ipc_t ipc, uint64_t ticks);

/*!
 * This function cancels the SYSCTR alarm.
 *
 * @param[in]     ipc         IPC handle
 *
 * Note this alarm setting clears when the alarm is triggered.
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if invalid time/date parameters
 */
sc_err_t sc_timer_cancel_sysctr_alarm(sc_ipc_t ipc);

/* @} */

#endif				/* SC_TIMER_API_H */

/**@}*/
