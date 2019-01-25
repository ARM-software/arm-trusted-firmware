/*
 * Copyright (c) 2017, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEGRA186_PRIVATE_H
#define TEGRA186_PRIVATE_H

void tegra186_cpu_reset_handler(void);
uint64_t tegra186_get_cpu_reset_handler_base(void);
uint64_t tegra186_get_cpu_reset_handler_size(void);
uint64_t tegra186_get_smmu_ctx_offset(void);
void tegra186_set_system_suspend_entry(void);

#endif /* TEGRA186_PRIVATE_H */
