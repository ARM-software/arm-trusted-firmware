/*
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRACE_H
#define TRACE_H

#include <lib/trace_protocol.h>
#include <tisci/tisci_protocol.h>
#include <common/debug.h>
#define TRACE_PRINT_MAX_LENGTH				 255
#define TRACE_NUM_BUFFER_CONVERSION_SIZE		 ((sizeof(uint32_t) * 8U) + 1U)

#define TRACE_LEVEL_ERR					 0U
#define TRACE_LEVEL_WARN				 1U
#define TRACE_LEVEL_INFO				 2U
#define TRACE_LEVEL_DEBUG				 3U

#define TRACE_PM_VAL_PSC_MASK				0x300000U
#define TRACE_PM_VAL_PD_MASK				0xFE000U
#define TRACE_PM_VAL_DEVICE_ID_MASK			0x3FFU

void pm_trace_debug(uint32_t action, uint32_t val);

#define pm_trace(action, val) pm_trace_debug(action, val)

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define CONFIG_TRACE
#endif

#endif
