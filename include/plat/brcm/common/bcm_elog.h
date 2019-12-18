/*
 * Copyright (c) 2018 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BCM_ELOG_H
#define BCM_ELOG_H

#ifndef __ASSEMBLER__

#include <stdint.h>

#if defined(BCM_ELOG) && (defined(IMAGE_BL2) || defined(IMAGE_BL31))
int bcm_elog_init(void *base, uint32_t size, unsigned int level);
void bcm_elog_exit(void);
int bcm_elog_copy_log(void *dst, uint32_t max_size);
void bcm_elog(const char *fmt, ...);
#else
static inline int bcm_elog_init(void *base, uint32_t size,
				unsigned int level)
{
	return 0;
}
static inline void bcm_elog_exit(void)
{
}
static inline int bcm_elog_copy_log(void *dst, uint32_t max_size)
{
	return 0;
}
static inline void bcm_elog(const char *fmt, ...)
{
}
#endif /* BCM_ELOG */

#endif /* __ASSEMBLER__ */
#endif /* BCM_ELOG_H */
