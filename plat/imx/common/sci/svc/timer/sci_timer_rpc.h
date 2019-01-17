/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * Header file for the TIMER RPC implementation.
 *
 * @addtogroup TIMER_SVC
 * @{
 */

#ifndef SC_TIMER_RPC_H
#define SC_TIMER_RPC_H

/* Includes */

/* Defines */

/*!
 * @name Defines for RPC TIMER function calls
 */
/*@{*/
#define TIMER_FUNC_UNKNOWN 0	/* Unknown function */
#define TIMER_FUNC_SET_WDOG_TIMEOUT 1U	/* Index for timer_set_wdog_timeout() RPC call */
#define TIMER_FUNC_SET_WDOG_PRE_TIMEOUT 12U	/* Index for timer_set_wdog_pre_timeout() RPC call */
#define TIMER_FUNC_START_WDOG 2U	/* Index for timer_start_wdog() RPC call */
#define TIMER_FUNC_STOP_WDOG 3U	/* Index for timer_stop_wdog() RPC call */
#define TIMER_FUNC_PING_WDOG 4U	/* Index for timer_ping_wdog() RPC call */
#define TIMER_FUNC_GET_WDOG_STATUS 5U	/* Index for timer_get_wdog_status() RPC call */
#define TIMER_FUNC_PT_GET_WDOG_STATUS 13U	/* Index for timer_pt_get_wdog_status() RPC call */
#define TIMER_FUNC_SET_WDOG_ACTION 10U	/* Index for timer_set_wdog_action() RPC call */
#define TIMER_FUNC_SET_RTC_TIME 6U	/* Index for timer_set_rtc_time() RPC call */
#define TIMER_FUNC_GET_RTC_TIME 7U	/* Index for timer_get_rtc_time() RPC call */
#define TIMER_FUNC_GET_RTC_SEC1970 9U	/* Index for timer_get_rtc_sec1970() RPC call */
#define TIMER_FUNC_SET_RTC_ALARM 8U	/* Index for timer_set_rtc_alarm() RPC call */
#define TIMER_FUNC_SET_RTC_PERIODIC_ALARM 14U	/* Index for timer_set_rtc_periodic_alarm() RPC call */
#define TIMER_FUNC_CANCEL_RTC_ALARM 15U	/* Index for timer_cancel_rtc_alarm() RPC call */
#define TIMER_FUNC_SET_RTC_CALB 11U	/* Index for timer_set_rtc_calb() RPC call */
#define TIMER_FUNC_SET_SYSCTR_ALARM 16U	/* Index for timer_set_sysctr_alarm() RPC call */
#define TIMER_FUNC_SET_SYSCTR_PERIODIC_ALARM 17U	/* Index for timer_set_sysctr_periodic_alarm() RPC call */
#define TIMER_FUNC_CANCEL_SYSCTR_ALARM 18U	/* Index for timer_cancel_sysctr_alarm() RPC call */
/*@}*/

/* Types */

/* Functions */

/*!
 * This function dispatches an incoming TIMER RPC request.
 *
 * @param[in]     caller_pt   caller partition
 * @param[in]     msg         pointer to RPC message
 */
void timer_dispatch(sc_rm_pt_t caller_pt, sc_rpc_msg_t *msg);

/*!
 * This function translates and dispatches an TIMER RPC request.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     msg         pointer to RPC message
 */
void timer_xlate(sc_ipc_t ipc, sc_rpc_msg_t *msg);

#endif				/* SC_TIMER_RPC_H */

/**@}*/
