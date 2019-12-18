/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PAXC_H
#define PAXC_H

#ifdef USE_PAXC
void paxc_init(void);
void paxc_mhb_ns_init(void);
#else
static inline void paxc_init(void)
{
}

static inline void paxc_mhb_ns_init(void)
{
}
#endif

#endif /* PAXC_H */
