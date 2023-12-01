/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IOMMU_SMC_H
#define IOMMU_SMC_H

#include <mtk_sip_svc.h>

u_register_t mtk_iommu_handler(u_register_t x1, u_register_t x2,
			u_register_t x3, u_register_t x4,
			void *handle, struct smccc_res *smccc_ret);
#endif
