/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CONTEXT_DEBUG_H
#define CONTEXT_DEBUG_H

#if PLATFORM_REPORT_CTX_MEM_USE && defined(__aarch64__)
/********************************************************************************
 * Reports the allocated memory for every security state and then reports the
 * total system-wide allocated memory.
 *******************************************************************************/
void report_ctx_memory_usage(void);
#else
static inline void report_ctx_memory_usage(void) {}
#endif /* PLATFORM_REPORT_CTX_MEM_USE */

#endif /* CONTEXT_DEBUG_H */
