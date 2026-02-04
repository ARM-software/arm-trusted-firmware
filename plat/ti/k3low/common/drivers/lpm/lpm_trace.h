/*
 * Copyright (c) 2024-2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LPM_TRACE_H__
#define __LPM_TRACE_H__

#include <plat/common/platform.h>
#include <common/debug.h>

/**
 * @brief Outputs uint32_t trace debug value to configured trace destinations
 *
 * @param value Trace debug value to output
 */
__wkupsramfunc void k3low_lpm_trace_debug(uint32_t value);

#define TRACE_PM_ACTION_LPM_SEQ_SHIFT		8U
#define TRACE_DEBUG_CHANNEL_LPM			5U
#define TRACE_DEBUG_DOMAIN_SHIFT		29U
#define TRACE_PM_ACTION_LPM_SEQUENCE		0x30U
#define TRACE_DEBUG_ACTION_SHIFT		22U
#define TRACE_PM_ACTION_FAIL			0x40U

/* LPM sequence trace step values */
#define LPM_SEQ_SECONDARY_CORE_DOWN		0x1U
#define LPM_SEQ_TIFS_WFI_WAIT			0x2U
#define LPM_SEQ_DDR_SELF_REFRESH		0x3U
#define LPM_SEQ_DDR_LPSC_DISABLE		0x4U
#define LPM_SEQ_SAVE_MAIN_PLL			0x5U
#define LPM_SEQ_DISABLE_MAIN_PLL		0x6U
#define LPM_SEQ_PMIC_CONFIG			0x7U
#define LPM_SEQ_SAVE_DDR_REGS			0x9U
#define LPM_SEQ_BEFORE_WFI			0xAU
#define LPM_SEQ_RESTORE_MAIN_PLL		0xBU
#define LPM_SEQ_ENABLE_DDR_LPSC			0xCU
#define LPM_SEQ_RESTORE_DDR_REGS		0xDU
#define LPM_SEQ_MAILBOX_SEND			0xFU
#define LPM_SEQ_AFTER_WFI_RESUME		0x10U
#define LPM_SEQ_UNEXPECTED_WFI_RETURN		0xF0U
#define LPM_SEQ_INVALID_MODE			0xF1U
#define LPM_SEQ_ABORT				0xF5U

#define LPM_TRACE_BASE \
	(TRACE_DEBUG_CHANNEL_LPM << TRACE_DEBUG_DOMAIN_SHIFT)

#define LPM_TRACE_SEQ_ACTION \
	(((uint32_t)TRACE_PM_ACTION_LPM_SEQUENCE) << TRACE_DEBUG_ACTION_SHIFT)

#define LPM_TRACE_SEQ_FAIL_ACTION \
	((((uint32_t)TRACE_PM_ACTION_LPM_SEQUENCE | TRACE_PM_ACTION_FAIL)) << \
	 TRACE_DEBUG_ACTION_SHIFT)

#define LPM_TRACE_STEP(step) \
	(((uint32_t)step) << TRACE_PM_ACTION_LPM_SEQ_SHIFT)

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define lpm_seq_trace(step) \
	k3low_lpm_trace_debug(LPM_TRACE_BASE | LPM_TRACE_SEQ_ACTION | \
			      LPM_TRACE_STEP(step) | (0U))

#define lpm_seq_trace_val(step, val) \
	k3low_lpm_trace_debug(LPM_TRACE_BASE | LPM_TRACE_SEQ_ACTION | \
			      LPM_TRACE_STEP(step) | (val))
#else
#define lpm_seq_trace(step)
#define lpm_seq_trace_val(step, val)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define lpm_seq_trace_fail(step) \
	k3low_lpm_trace_debug(LPM_TRACE_BASE | LPM_TRACE_SEQ_FAIL_ACTION | \
			      LPM_TRACE_STEP(step) | (0U))
#else
#define lpm_seq_trace_fail(step)
#endif

#endif /* __LPM_TRACE_H__ */
