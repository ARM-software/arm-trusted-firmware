/*
 * Copyright (c) 2019-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PAUTH_H
#define PAUTH_H

#if ENABLE_PAUTH
/* Platform hook to generate the APIAKey */
uint128_t plat_init_apkey(void);

void pauth_init_enable_el3(void);
void pauth_init_enable_el1(void);
void pauth_init(void);
void pauth_enable_el1(void);
void pauth_enable_el3(void);
void pauth_enable_el2(void);
void pauth_disable_el1(void);
void pauth_disable_el3(void);
#else
static inline void pauth_init_enable_el3(void)
{
}
static inline void pauth_init_enable_el1(void)
{
}
static inline void pauth_init(void)
{
}
static inline void pauth_enable_el1(void)
{
}
static inline void pauth_enable_el3(void)
{
}
static inline void pauth_enable_el2(void)
{
}
static inline void pauth_disable_el1(void)
{
}
static inline void pauth_disable_el3(void)
{
}
#endif
#endif /* PAUTH_H */
