/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*!
 * File containing client-side RPC functions for the TIMER service. These
 * functions are ported to clients that communicate to the SC.
 *
 * @addtogroup TIMER_SVC
 * @{
 */

/* Includes */

#include <sci/sci_types.h>
#include <sci/svc/rm/sci_rm_api.h>
#include <sci/svc/timer/sci_timer_api.h>
#include <sci/sci_rpc.h>
#include <stdlib.h>
#include "sci_timer_rpc.h"

/* Local Defines */

/* Local Types */

/* Local Functions */

sc_err_t sc_timer_set_wdog_timeout(sc_ipc_t ipc, sc_timer_wdog_time_t timeout)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_SET_WDOG_TIMEOUT;
	RPC_U32(&msg, 0U) = (uint32_t)timeout;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_wdog_pre_timeout(sc_ipc_t ipc,
				       sc_timer_wdog_time_t pre_timeout)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_SET_WDOG_PRE_TIMEOUT;
	RPC_U32(&msg, 0U) = (uint32_t)pre_timeout;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_start_wdog(sc_ipc_t ipc, sc_bool_t lock)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_START_WDOG;
	RPC_U8(&msg, 0U) = (uint8_t)lock;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_stop_wdog(sc_ipc_t ipc)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_STOP_WDOG;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_ping_wdog(sc_ipc_t ipc)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_PING_WDOG;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_get_wdog_status(sc_ipc_t ipc,
				  sc_timer_wdog_time_t *timeout,
				  sc_timer_wdog_time_t *max_timeout,
				  sc_timer_wdog_time_t *remaining_time)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_GET_WDOG_STATUS;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (timeout != NULL)
		*timeout = RPC_U32(&msg, 0U);

	if (max_timeout != NULL)
		*max_timeout = RPC_U32(&msg, 4U);

	if (remaining_time != NULL)
		*remaining_time = RPC_U32(&msg, 8U);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_pt_get_wdog_status(sc_ipc_t ipc, sc_rm_pt_t pt,
				     sc_bool_t *enb,
				     sc_timer_wdog_time_t *timeout,
				     sc_timer_wdog_time_t *remaining_time)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_PT_GET_WDOG_STATUS;
	RPC_U8(&msg, 0U) = (uint8_t)pt;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (timeout != NULL)
		*timeout = RPC_U32(&msg, 0U);

	if (remaining_time != NULL)
		*remaining_time = RPC_U32(&msg, 4U);

	result = RPC_R8(&msg);
	if (enb != NULL)
		*enb = RPC_U8(&msg, 8U);

	return (sc_err_t)result;
}

sc_err_t sc_timer_set_wdog_action(sc_ipc_t ipc,
				  sc_rm_pt_t pt, sc_timer_wdog_action_t action)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_SET_WDOG_ACTION;
	RPC_U8(&msg, 0U) = (uint8_t)pt;
	RPC_U8(&msg, 1U) = (uint8_t)action;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_rtc_time(sc_ipc_t ipc, uint16_t year, uint8_t mon,
			       uint8_t day, uint8_t hour, uint8_t min,
			       uint8_t sec)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_SET_RTC_TIME;
	RPC_U16(&msg, 0U) = (uint16_t)year;
	RPC_U8(&msg, 2U) = (uint8_t)mon;
	RPC_U8(&msg, 3U) = (uint8_t)day;
	RPC_U8(&msg, 4U) = (uint8_t)hour;
	RPC_U8(&msg, 5U) = (uint8_t)min;
	RPC_U8(&msg, 6U) = (uint8_t)sec;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_get_rtc_time(sc_ipc_t ipc, uint16_t *year, uint8_t *mon,
			       uint8_t *day, uint8_t *hour, uint8_t *min,
			       uint8_t *sec)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_GET_RTC_TIME;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (year != NULL)
		*year = RPC_U16(&msg, 0U);

	result = RPC_R8(&msg);
	if (mon != NULL)
		*mon = RPC_U8(&msg, 2U);

	if (day != NULL)
		*day = RPC_U8(&msg, 3U);

	if (hour != NULL)
		*hour = RPC_U8(&msg, 4U);

	if (min != NULL)
		*min = RPC_U8(&msg, 5U);

	if (sec != NULL)
		*sec = RPC_U8(&msg, 6U);

	return (sc_err_t)result;
}

sc_err_t sc_timer_get_rtc_sec1970(sc_ipc_t ipc, uint32_t *sec)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_GET_RTC_SEC1970;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	if (sec != NULL)
		*sec = RPC_U32(&msg, 0U);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_rtc_alarm(sc_ipc_t ipc, uint16_t year, uint8_t mon,
				uint8_t day, uint8_t hour, uint8_t min,
				uint8_t sec)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_SET_RTC_ALARM;
	RPC_U16(&msg, 0U) = (uint16_t)year;
	RPC_U8(&msg, 2U) = (uint8_t)mon;
	RPC_U8(&msg, 3U) = (uint8_t)day;
	RPC_U8(&msg, 4U) = (uint8_t)hour;
	RPC_U8(&msg, 5U) = (uint8_t)min;
	RPC_U8(&msg, 6U) = (uint8_t)sec;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_rtc_periodic_alarm(sc_ipc_t ipc, uint32_t sec)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_SET_RTC_PERIODIC_ALARM;
	RPC_U32(&msg, 0U) = (uint32_t)sec;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_cancel_rtc_alarm(sc_ipc_t ipc)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_CANCEL_RTC_ALARM;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_rtc_calb(sc_ipc_t ipc, int8_t count)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_SET_RTC_CALB;
	RPC_I8(&msg, 0U) = (int8_t) count;
	RPC_SIZE(&msg) = 2U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_sysctr_alarm(sc_ipc_t ipc, uint64_t ticks)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_SET_SYSCTR_ALARM;
	RPC_U32(&msg, 0U) = (uint32_t)(ticks >> 32U);
	RPC_U32(&msg, 4U) = (uint32_t)ticks;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_set_sysctr_periodic_alarm(sc_ipc_t ipc, uint64_t ticks)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_SET_SYSCTR_PERIODIC_ALARM;
	RPC_U32(&msg, 0U) = (uint32_t)(ticks >> 32U);
	RPC_U32(&msg, 4U) = (uint32_t)ticks;
	RPC_SIZE(&msg) = 3U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

sc_err_t sc_timer_cancel_sysctr_alarm(sc_ipc_t ipc)
{
	sc_rpc_msg_t msg;
	uint8_t result;

	RPC_VER(&msg) = SC_RPC_VERSION;
	RPC_SVC(&msg) = (uint8_t)SC_RPC_SVC_TIMER;
	RPC_FUNC(&msg) = (uint8_t)TIMER_FUNC_CANCEL_SYSCTR_ALARM;
	RPC_SIZE(&msg) = 1U;

	sc_call_rpc(ipc, &msg, SC_FALSE);

	result = RPC_R8(&msg);
	return (sc_err_t)result;
}

/**@}*/
