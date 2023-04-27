/*
 * Copyright (c) 2022-2023, Advanced Micro Devices, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUSTOM_SVC_H
#define CUSTOM_SVC_H

#define ZYNQMP_SIP_SVC_CUSTOM   U(0x82002000)
#define ZYNQMP_SIP_SVC64_CUSTOM U(0xC2002000)

uint64_t custom_smc_handler(uint32_t smc_fid, uint64_t x1, uint64_t x2,
			    uint64_t x3, uint64_t x4, void *cookie,
			    void *handle, uint64_t flags);

void custom_early_setup(void);
void custom_mmap_add(void);
void custom_runtime_setup(void);

#endif /* CUSTOM_SVC_H */
