/*
 * Copyright (c) 2015-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MEMCTRL_H
#define MEMCTRL_H

void tegra_memctrl_setup(void);
void tegra_memctrl_restore_settings(void);
void tegra_memctrl_tzdram_setup(uint64_t phys_base, uint32_t size_in_bytes);
void tegra_memctrl_tzram_setup(uint64_t phys_base, uint32_t size_in_bytes);
void tegra_memctrl_videomem_setup(uint64_t phys_base, uint32_t size_in_bytes);
void tegra_memctrl_disable_ahb_redirection(void);
void tegra_memctrl_clear_pending_interrupts(void);

#endif /* MEMCTRL_H */
