/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPM_TRACE_H__
#define __LPM_TRACE_H__

#include <plat/common/platform.h>

/**
 * \brief Outputs uint32_t trace debug value to configured trace destinations.
 * \param value Trace debug value to output.
 */
__wkupsramfunc void lpm_trace_debug(uint32_t value);

#ifdef TI_AM62L_LPM_TRACE

	#define TRACE_PM_ACTION_LPM_SEQ_SHIFT		8U
	#define TRACE_DEBUG_CHANNEL_LPM			5U
	#define TRACE_DEBUG_DOMAIN_SHIFT		29U
	#define TRACE_PM_ACTION_LPM_SEQUENCE		0x30U
	#define TRACE_DEBUG_ACTION_SHIFT		22U
	#define TRACE_PM_ACTION_FAIL			0x40U
	#define lpm_seq_trace(step) lpm_trace_debug((TRACE_DEBUG_CHANNEL_LPM << TRACE_DEBUG_DOMAIN_SHIFT) \
								| (((uint32_t)TRACE_PM_ACTION_LPM_SEQUENCE) << TRACE_DEBUG_ACTION_SHIFT) \
								| (((uint32_t)step) << TRACE_PM_ACTION_LPM_SEQ_SHIFT)			\
								| (0U))

	#define lpm_seq_trace_fail(step) lpm_trace_debug((TRACE_DEBUG_CHANNEL_LPM << TRACE_DEBUG_DOMAIN_SHIFT) \
								| (((uint32_t)TRACE_PM_ACTION_LPM_SEQUENCE | TRACE_PM_ACTION_FAIL) << TRACE_DEBUG_ACTION_SHIFT)  \
								| (((uint32_t)step) << TRACE_PM_ACTION_LPM_SEQ_SHIFT)		     \
								| (0U))

	#define lpm_seq_trace_val(step, val) lpm_trace_debug((TRACE_DEBUG_CHANNEL_LPM << TRACE_DEBUG_DOMAIN_SHIFT) \
								| (((uint32_t)TRACE_PM_ACTION_LPM_SEQUENCE) << TRACE_DEBUG_ACTION_SHIFT)  \
								| (((uint32_t)step) << TRACE_PM_ACTION_LPM_SEQ_SHIFT) \
								| (val))
#else
	#define lpm_seq_trace(step)
	#define lpm_seq_trace_fail(step)
	#define lpm_seq_trace_val(step, val)
#endif /* TI_AM62L_LPM_TRACE */

#endif /* __LPM_TRACE_H__ */
