/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

void mg_start_ap_fw(int cp_nr, uint8_t comphy_index);
int mg_image_load(uintptr_t src_addr, uint32_t size, int cp_index);
