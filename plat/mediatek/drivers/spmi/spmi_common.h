/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SPMI_COMMON_H
#define SPMI_COMMON_H

#include <stdint.h>

#include <platform_def.h>
#include "pmif_common.h"

/* Read/write byte limitation */
#define PMIF_BYTECNT_MAX	2

#define SPMI_GROUP_ID		0xB

/* enum marco for cmd/channel */
enum spmi_master {
	SPMI_MASTER_0 = 0,
	SPMI_MASTER_1,
	SPMI_MASTER_P_1,
	SPMI_MASTER_MAX
};

enum spmi_slave {
	SPMI_SLAVE_0 = 0,
	SPMI_SLAVE_1,
	SPMI_SLAVE_2,
	SPMI_SLAVE_3,
	SPMI_SLAVE_4,
	SPMI_SLAVE_5,
	SPMI_SLAVE_6,
	SPMI_SLAVE_7,
	SPMI_SLAVE_8,
	SPMI_SLAVE_9,
	SPMI_SLAVE_10,
	SPMI_SLAVE_11,
	SPMI_SLAVE_12,
	SPMI_SLAVE_13,
	SPMI_SLAVE_14,
	SPMI_SLAVE_15,
	SPMI_MAX_SLAVE_ID
};

enum slv_type {
	BUCK_CPU,
	BUCK_GPU,
	BUCK_MD,
	BUCK_RF,
	MAIN_PMIC,
	BUCK_VPU,
	SUB_PMIC,
	CLOCK_PMIC,
	SECOND_PMIC,
	SLV_TYPE_MAX
};

enum slv_type_id {
	BUCK_RF_ID = 1,
	BUCK_MD_ID = 3,
	MAIN_PMIC_ID = 5,
	BUCK_CPU_ID = 6,
	BUCK_GPU_ID = 7,
	BUCK_VPU_ID,
	SUB_PMIC_ID = 10,
	CLOCK_PMIC_ID = 11,
	SECOND_PMIC_ID = 12,
	SLV_TYPE_ID_MAX
};

enum {
	SPMI_OP_ST_BUSY = 1,
	SPMI_OP_ST_ACK = 0,
	SPMI_OP_ST_NACK = 1
};

struct spmi_device {
	int slvid;
	int grpiden;
	enum slv_type type;
	enum slv_type_id type_id;
	int mstid;
	uint16_t hwcid_addr;
	uint8_t hwcid_val;
	uint16_t hwcid_mask;
	uint16_t swcid_addr;
	uint8_t swcid_val;
	uint16_t wpk_key_addr;
	uint16_t wpk_key_val;
	uint16_t wpk_key_h_val;
	uint16_t tma_key_addr;
	uint16_t tma_key_val;
	uint16_t tma_key_h_val;
	uint16_t rcs_en_addr;
	uint16_t rcs_slvid_addr;
	struct pmif *pmif_arb;
};

int spmi_command_shutdown(int mstid, struct spmi_device *dev, unsigned int grpiden);
#endif
