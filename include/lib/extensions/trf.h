/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TRF_H
#define TRF_H

#if ENABLE_TRF_FOR_NS
void trf_enable(void);
#else
static inline void trf_enable(void)
{
}
#endif /* ENABLE_TRF_FOR_NS */

#endif /* TRF_H */
