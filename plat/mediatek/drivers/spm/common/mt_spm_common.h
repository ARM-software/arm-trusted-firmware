/*
 * Copyright (c) 2025, Mediatek Inc. All rights resrved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_COMMON_H
#define MT_SPM_COMMON_H

#include <lib/bakery_lock.h>
#include <lib/spinlock.h>
/*
 * ARM v8.2, the cache will turn off automatically when cpu
 * power down. So, there is no doubt to use the spin_lock here
 */
#if !HW_ASSISTED_COHERENCY
#define MT_SPM_USING_BAKERY_LOCK
#endif

#ifdef MT_SPM_USING_BAKERY_LOCK
DECLARE_BAKERY_LOCK(spm_lock);
#define plat_spm_lock() \
	bakery_lock_get(&spm_lock)

#define plat_spm_unlock() \
	bakery_lock_release(&spm_lock)
#else
extern spinlock_t spm_lock;
#define plat_spm_lock() \
	spin_lock(&spm_lock)

#define plat_spm_unlock() \
	spin_unlock(&spm_lock)
#endif

#define MT_SPM_ERR_NO_FW_LOAD		-1
#define MT_SPM_ERR_KICKED		-2
#define MT_SPM_ERR_RUNNING		-3
#define MT_SPM_ERR_FW_NOT_FOUND		-4
#define MT_SPM_ERR_INVALID		-5
#define MT_SPM_ERR_OVERFLOW		-6

static inline void spm_lock_get(void)
{
	plat_spm_lock();
}

static inline void spm_lock_release(void)
{
	plat_spm_unlock();
}

#endif /* MT_SPM_COMMON_H */
