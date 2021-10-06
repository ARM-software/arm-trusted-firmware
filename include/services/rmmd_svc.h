/*
 * Copyright (c) 2021, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RMMD_SVC_H
#define RMMD_SVC_H

#ifndef __ASSEMBLER__
#include <stdint.h>

int rmmd_setup(void);
uint64_t rmmd_rmi_handler(uint32_t smc_fid,
		uint64_t x1,
		uint64_t x2,
		uint64_t x3,
		uint64_t x4,
		void *cookie,
		void *handle,
		uint64_t flags);

uint64_t rmmd_gtsi_handler(uint32_t smc_fid,
		uint64_t x1,
		uint64_t x2,
		uint64_t x3,
		uint64_t x4,
		void *cookie,
		void *handle,
		uint64_t flags);

#endif /* __ASSEMBLER__ */

#endif /* RMMD_SVC_H */
