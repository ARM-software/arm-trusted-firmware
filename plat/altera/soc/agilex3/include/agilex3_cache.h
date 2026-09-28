/*
 * Copyright (c) 2024-2026, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AGX3_CACHE_H
#define AGX3_CACHE_H

#ifndef __ASSEMBLER__

void invalidate_dcache_all(void);
void invalidate_tlb_low_el(void);
void invalidate_all_tlbs(void);
void invalidate_l1_i_cache(void);

#endif /* __ASSEMBLER__ */

#endif /* AGX3_CACHE_H */
