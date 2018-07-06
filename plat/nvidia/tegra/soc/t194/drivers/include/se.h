/*
 * Copyright (c) 2019-2020, NVIDIA CORPORATION. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SE_H
#define SE_H

int32_t tegra_se_calculate_save_sha256(uint64_t src_addr,
						uint32_t src_len_inbyte);
int32_t tegra_se_suspend(void);
void tegra_se_resume(void);

#endif /* SE_H */
