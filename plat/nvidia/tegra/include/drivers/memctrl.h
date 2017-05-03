/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MEMCTRL_H__
#define __MEMCTRL_H__

void tegra_memctrl_setup(void);
void tegra_memctrl_restore_settings(void);
void tegra_memctrl_tzdram_setup(uint64_t phys_base, uint32_t size_in_bytes);
void tegra_memctrl_tzram_setup(uint64_t phys_base, uint32_t size_in_bytes);
void tegra_memctrl_videomem_setup(uint64_t phys_base, uint32_t size_in_bytes);
void tegra_memctrl_disable_ahb_redirection(void);

#endif /* __MEMCTRL_H__ */
