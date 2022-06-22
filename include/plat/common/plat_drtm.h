/*
 * Copyright (c) 2022, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLAT_DRTM_H
#define PLAT_DRTM_H

/* platform-specific DMA protection functions */
bool plat_has_non_host_platforms(void);
bool plat_has_unmanaged_dma_peripherals(void);
unsigned int plat_get_total_smmus(void);
void plat_enumerate_smmus(const uintptr_t **smmus_out,
			  size_t *smmu_count_out);

#endif /* PLAT_DRTM_H */
