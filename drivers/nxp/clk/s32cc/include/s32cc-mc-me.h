// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021, 2023-2024 NXP
 */
#ifndef S32CC_MC_ME_H
#define S32CC_MC_ME_H

#include <stdbool.h>
#include <stdint.h>

int mc_me_enable_partition(uintptr_t mc_me, uintptr_t mc_rgm, uintptr_t rdc,
			   uint32_t part);
void mc_me_enable_part_cofb(uintptr_t mc_me, uint32_t partition_n, uint32_t block,
			    bool check_status);

#endif /* S32CC_MC_ME_H */
