/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMC_SVC_H
#define SPMC_SVC_H

#ifndef __ASSEMBLER__
#include <stdint.h>

#include <lib/utils_def.h>
#include <services/ffa_svc.h>
#include <services/spm_core_manifest.h>

int spmc_setup(void);
void spmc_populate_attrs(spmc_manifest_attribute_t *spmc_attrs);
void *spmc_get_config_addr(void);

void spmc_set_config_addr(uintptr_t soc_fw_config);

uint64_t spmc_smc_handler(uint32_t smc_fid,
			  bool secure_origin,
			  uint64_t x1,
			  uint64_t x2,
			  uint64_t x3,
			  uint64_t x4,
			  void *cookie,
			  void *handle,
			  uint64_t flags);

static inline bool is_spmc_at_el3(void)
{
	return SPMC_AT_EL3 == 1;
}

#endif /* __ASSEMBLER__ */

#endif /* SPMC_SVC_H */
