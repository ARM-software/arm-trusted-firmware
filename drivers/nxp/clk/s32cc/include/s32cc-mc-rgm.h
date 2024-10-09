// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021, 2023-2024 NXP
 */
#ifndef S32CC_MC_RGM_H
#define S32CC_MC_RGM_H

#include <stdint.h>

void mc_rgm_periph_reset(uintptr_t rgm, uint32_t part, uint32_t value);
void mc_rgm_release_part(uintptr_t rgm, uint32_t part);
void mc_rgm_wait_part_deassert(uintptr_t rgm, uint32_t part);

#endif /* MC_RGM_H */
