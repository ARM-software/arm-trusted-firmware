/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef QTI_SECURE_IO_CFG_H
#define QTI_SECURE_IO_CFG_H

#include <stdint.h>

/*
 * List of peripheral/IO memory areas that are protected from
 * non-secure world but not required to be secure.
 */

/* AHB2PHY_USB EUD Mode Manager 2 */
#define EUD_MODE_MANAGER2_EN			0x01612000
#define APPS_SMMU_TBU_PWR_STATUS		0x0C602204
#define APPS_SMMU_CUSTOM_CFG			0x0C602300
#define APPS_SMMU_STATS_SYNC_INV_TBU_ACK	0x0C6025DC
#define APPS_SMMU_SAFE_SEC_CFG			0x0C602648
#define APPS_SMMU_MMU2QSS_AND_SAFE_WAIT_CNTR	0x0C602670

static const uintptr_t qti_secure_io_allowed_regs[] = {
#if DEBUG
	EUD_MODE_MANAGER2_EN,
#endif /* DEBUG */
	APPS_SMMU_TBU_PWR_STATUS,
	APPS_SMMU_CUSTOM_CFG,
	APPS_SMMU_STATS_SYNC_INV_TBU_ACK,
	APPS_SMMU_SAFE_SEC_CFG,
	APPS_SMMU_MMU2QSS_AND_SAFE_WAIT_CNTR,
};

#endif /* QTI_SECURE_IO_CFG_H */
