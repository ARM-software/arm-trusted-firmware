/*
 * Copyright (c) 2022-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BRBE_H
#define BRBE_H

#if ENABLE_BRBE_FOR_NS
void brbe_init_el3(void);
#else
static inline void brbe_init_el3(void)
{
}
#endif /* ENABLE_BRBE_FOR_NS */

#endif /* BRBE_H */
