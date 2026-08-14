/*
 * Copyright (c) 2026, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FIRME_GM_PRIVATE_H
#define FIRME_GM_PRIVATE_H

void firme_gm_gpi_init(void);

uint32_t firme_gm_gpi_set(uint64_t base, uint64_t gcnt, uint64_t attrs,
			  uint64_t flags, uint64_t *gcnt_ret);

#endif /* FIRME_GM_PRIVATE_H */
