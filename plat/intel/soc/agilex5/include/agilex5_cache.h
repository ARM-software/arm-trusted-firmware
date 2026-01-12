/*
 * Copyright (c) 2024, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AGX5_CACHE_H
#define AGX5_CACHE_H

void invalidate_dcache_all(void);
void invalidate_tlb_low_el(void);
void invalidate_all_tlbs(void);
void invalidate_l1_i_cache(void);

#endif /* AGX5_CACHE_H */
