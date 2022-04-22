/*
 * Copyright (c) 2020-2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMD_SVC_H
#define SPMD_SVC_H

#ifndef __ASSEMBLER__
#include <services/ffa_svc.h>
#include <stdint.h>

int spmd_setup(void);
uint64_t spmd_ffa_smc_handler(uint32_t smc_fid,
			      uint64_t x1,
			      uint64_t x2,
			      uint64_t x3,
			      uint64_t x4,
			      void *cookie,
			      void *handle,
			      uint64_t flags);
uint64_t spmd_smc_handler(uint32_t smc_fid,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags);
uint64_t spmd_smc_switch_state(uint32_t smc_fid,
			       bool secure_origin,
			       uint64_t x1,
			       uint64_t x2,
			       uint64_t x3,
			       uint64_t x4,
			       void *handle);
#endif /* __ASSEMBLER__ */

#endif /* SPMD_SVC_H */
