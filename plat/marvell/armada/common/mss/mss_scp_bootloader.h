/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#ifndef MSS_SCP_BOOTLOADER_H
#define MSS_SCP_BOOTLOADER_H

int scp_bootloader_transfer(void *image, unsigned int image_size);
uintptr_t bl2_plat_get_cp_mss_regs(int ap_idx, int cp_idx);
uintptr_t bl2_plat_get_cp_mss_sram(int ap_idx, int cp_idx);
uintptr_t bl2_plat_get_ap_mss_regs(int ap_idx);
uint32_t bl2_plat_get_cp_count(int ap_idx);
uint32_t bl2_plat_get_ap_count(void);
void bl2_plat_configure_mss_windows(uintptr_t mss_regs);
int bl2_plat_mss_check_image_ready(void);

#endif /* MSS_SCP_BOOTLOADER_H */
