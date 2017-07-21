/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PROFILER_H__
#define __PROFILER_H__

/*******************************************************************************
 * Number of bytes of memory used by the profiler on Tegra
 ******************************************************************************/
#define PROFILER_SIZE_BYTES	U(0x1000)

void boot_profiler_init(uint64_t shmem_base, uint32_t tmr_base);
void boot_profiler_add_record(const char *str);
void boot_profiler_deinit(void);

#endif /* __PROFILER_H__ */
