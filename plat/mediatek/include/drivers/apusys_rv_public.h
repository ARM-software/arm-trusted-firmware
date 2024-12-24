/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_RV_PUBLIC_H
#define APUSYS_RV_PUBLIC_H

#ifdef CONFIG_MTK_APUSYS_RV_IOMMU_HW_SEM_SUPPORT
int apusys_rv_iommu_hw_sem_trylock(void);
int apusys_rv_iommu_hw_sem_unlock(void);
#else
#define apusys_rv_iommu_hw_sem_trylock() 0
#define apusys_rv_iommu_hw_sem_unlock() 0
#endif /* CONFIG_MTK_APUSYS_RV_IOMMU_HW_SEM_SUPPORT */

#endif /* APUSYS_RV_PUBLIC_H */
