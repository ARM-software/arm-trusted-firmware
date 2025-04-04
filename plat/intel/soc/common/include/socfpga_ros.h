/*
 * Copyright (c) 2024, Intel Corporation. All rights reserved.
 * Copyright (c) 2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_ROS_H
#define SOCFPGA_ROS_H

#include <arch_helpers.h>
#include <lib/utils_def.h>

/** status response*/
#define ROS_RET_OK			(0x00U)
#define ROS_RET_INVALID			(0x01U)
#define ROS_RET_NOT_RSU_MODE		(0x02U)
#define ROS_QSPI_READ_ERROR		(0x03U)
#define ROS_SPT_BAD_MAGIC_NUM		(0x04U)
#define ROS_SPT_CRC_ERROR		(0x05U)
#define ROS_IMAGE_INDEX_ERR		(0x06U)
#define ROS_IMAGE_PARTNUM_OVFL		(0x07U)

#define ADDR_64(h, l)			(((((unsigned long)(h)) & 0xffffffff) << 32) | \
						(((unsigned long)(l)) & 0xffffffff))

#define RSU_GET_SPT_RESP_SIZE		(4U)

#define RSU_STATUS_RES_SIZE		(9U)

#define SPT_MAGIC_NUMBER		(0x57713427U)
#define SPT_VERSION			(0U)
#define SPT_FLAG_RESERVED		(1U)
#define SPT_FLAG_READONLY		(2U)

#define SPT_MAX_PARTITIONS		(127U)
#define SPT_PARTITION_NAME_LENGTH	(16U)
#define SPT_RSVD_LENGTH			(4U)
#define SPT_SIZE			(4096U)
/*BOOT_INFO + FACTORY_IMAGE + SPT0 + SPT1 + CPB0 + CPB1 + FACTORY_IM.SSBL+ *APP* + *APP*.SSBL*/
#define SPT_MIN_PARTITIONS		(9U)

#define FACTORY_IMAGE			"FACTORY_IMAGE"
#define FACTORY_SSBL			"SSBL.FACTORY_IM"
#define SSBL_PREFIX			"SSBL."

typedef struct {
	const uint32_t magic_number;
	const uint32_t version;
	const uint32_t partitions;
	uint32_t checksum;
	const uint32_t __RSVD[SPT_RSVD_LENGTH];
	struct {
		const char name[SPT_PARTITION_NAME_LENGTH];
		const uint64_t offset;
		const uint32_t length;
		const uint32_t flags;
	} partition[SPT_MAX_PARTITIONS];
} __packed __aligned(4) spt_table_t;

uint32_t ros_qspi_get_ssbl_offset(unsigned long *offset);

#endif /* SOCFPGA_ROS_H */
