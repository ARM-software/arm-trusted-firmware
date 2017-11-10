/*
 * Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TEGRA194_PRIVATE_H__
#define __TEGRA194_PRIVATE_H__

void tegra194_cpu_reset_handler(void);
uint64_t tegra194_get_cpu_reset_handler_base(void);
uint64_t tegra194_get_cpu_reset_handler_size(void);
uint64_t tegra194_get_smmu_ctx_offset(void);
void tegra194_set_system_suspend_entry(void);

#endif /* __TEGRA194_PRIVATE_H__ */
