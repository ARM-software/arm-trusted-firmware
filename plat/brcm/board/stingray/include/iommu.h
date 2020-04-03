/*
 * Copyright (c) 2016 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef IOMMU_H
#define IOMMU_H

enum iommu_domain {
	PCIE_PAXC,
	DOMAIN_CRMU,
};

void arm_smmu_create_identity_map(enum iommu_domain dom);
void arm_smmu_reserve_secure_cntxt(void);
void arm_smmu_enable_secure_client_port(void);

#endif /* IOMMU_H */
