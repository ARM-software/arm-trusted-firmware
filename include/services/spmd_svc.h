/*
 * Copyright (c) 2020, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMD_SVC_H
#define SPMD_SVC_H

#ifndef __ASSEMBLER__
#include <services/ffa_svc.h>
#include <stdint.h>

int spmd_setup(void);
uint64_t spmd_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags);
#endif /* __ASSEMBLER__ */

#endif /* SPMD_SVC_H */
