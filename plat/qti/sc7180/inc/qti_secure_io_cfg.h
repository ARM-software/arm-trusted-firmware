/*
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
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

#define APPS_SMMU_TBU_PWR_STATUS		0x15002204
#define APPS_SMMU_CUSTOM_CFG			0x15002300
#define APPS_SMMU_STATS_SYNC_INV_TBU_ACK	0x150025DC
#define APPS_SMMU_SAFE_SEC_CFG			0x15002648
#define APPS_SMMU_MMU2QSS_AND_SAFE_WAIT_CNTR	0x15002670

static const uintptr_t qti_secure_io_allowed_regs[] = {
	APPS_SMMU_TBU_PWR_STATUS,
	APPS_SMMU_CUSTOM_CFG,
	APPS_SMMU_STATS_SYNC_INV_TBU_ACK,
	APPS_SMMU_SAFE_SEC_CFG,
	APPS_SMMU_MMU2QSS_AND_SAFE_WAIT_CNTR,
};

#endif /* QTI_SECURE_IO_CFG_H */
