/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRF_H
#define TRF_H

#if ENABLE_TRF_FOR_NS
void trf_init_el3(void);
void trf_init_el2_unused(void);
#else
static inline void trf_init_el3(void)
{
}
static inline void trf_init_el2_unused(void)
{
}
#endif /* ENABLE_TRF_FOR_NS */

#endif /* TRF_H */
