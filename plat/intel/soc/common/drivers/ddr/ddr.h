/*
 * Copyright (c) 2022-2023, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DDR_H
#define DDR_H

#include <lib/mmio.h>
#include "socfpga_handoff.h"

enum ddr_type {
	DDR_TYPE_LPDDR4_0,
	DDR_TYPE_LPDDR4_1,
	DDR_TYPE_DDR4,
	DDR_TYPE_LPDDR5_0,
	DDR_TYPE_LPDDR5_1,
	DDR_TYPE_DDR5,
	DDR_TYPE_UNKNOWN
};

/* Region size for ECCCFG0.ecc_region_map */
enum region_size {
	ONE_EIGHT,
	ONE_SIXTEENTH,
	ONE_THIRTY_SECOND,
	ONE_SIXTY_FOURTH
};

/* DATATYPE DEFINATION */
typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;

/* MACRO DEFINATION */
#define IO96B_0_REG_BASE				0x18400000
#define IO96B_1_REG_BASE				0x18800000
#define IO96B_CSR_BASE					0x05000000
#define IO96B_CSR_REG(reg)				(IO96B_CSR_BASE + reg)

#define IOSSM_CMD_MAX_WORD_SIZE				7U
#define IOSSM_RESP_MAX_WORD_SIZE			4U

#define CCU_REG_BASE					0x1C000000
#define DMI0_DMIUSMCTCR					0x7300
#define DMI1_DMIUSMCTCR					0x8300
#define CCU_DMI_ALLOCEN					BIT(1)
#define CCU_DMI_LOOKUPEN				BIT(2)
#define CCU_REG(reg)					(CCU_REG_BASE + reg)

// CMD_RESPONSE_STATUS Register
#define CMD_RESPONSE_STATUS				0x45C
#define CMD_RESPONSE_OFFSET				0x4
#define CMD_RESPONSE_DATA_SHORT_MASK			GENMASK(31, 16)
#define CMD_RESPONSE_DATA_SHORT_OFFSET			16
#define STATUS_CMD_RESPONSE_ERROR_MASK			GENMASK(7, 5)
#define STATUS_CMD_RESPONSE_ERROR_OFFSET		5
#define STATUS_GENERAL_ERROR_MASK			GENMASK(4, 1)
#define STATUS_GENERAL_ERROR_OFFSET			1
#define STATUS_COMMAND_RESPONSE_READY			0x1
#define STATUS_COMMAND_RESPONSE_READY_CLEAR		0x0
#define STATUS_COMMAND_RESPONSE_READY_MASK		0x1
#define STATUS_COMMAND_RESPONSE_READY_OFFSET		0
#define STATUS_COMMAND_RESPONSE(x)			(((x) & \
							STATUS_COMMAND_RESPONSE_READY_MASK) >> \
							STATUS_COMMAND_RESPONSE_READY_OFFSET)

// CMD_REQ Register
#define CMD_STATUS					0x400
#define CMD_PARAM					0x438
#define CMD_REQ						0x43C
#define CMD_PARAM_OFFSET				0x4
#define CMD_TARGET_IP_TYPE_MASK				GENMASK(31, 29)
#define CMD_TARGET_IP_TYPE_OFFSET			29
#define CMD_TARGET_IP_INSTANCE_ID_MASK			GENMASK(28, 24)
#define CMD_TARGET_IP_INSTANCE_ID_OFFSET		24
#define CMD_TYPE_MASK					GENMASK(23, 16)
#define CMD_TYPE_OFFSET					16
#define CMD_OPCODE_MASK					GENMASK(15, 0)
#define CMD_OPCODE_OFFSET				0

#define CMD_INIT					0

#define OPCODE_GET_MEM_INTF_INFO			0x0001
#define OPCODE_GET_MEM_TECHNOLOGY			0x0002
#define OPCODE_GET_MEM_WIDTH_INFO			0x0004
#define OPCODE_TRIG_MEM_CAL				0x000A
#define OPCODE_ECC_ENABLE_STATUS			0x0102
#define OPCODE_ECC_INTERRUPT_MASK			0x0105
#define OPCODE_ECC_SCRUB_MODE_0_START			0x0202
#define OPCODE_ECC_SCRUB_MODE_1_START			0x0203
#define OPCODE_BIST_RESULTS_STATUS			0x0302
#define OPCODE_BIST_MEM_INIT_START			0x0303
// Please update according to IOSSM mailbox spec
#define MBOX_ID_IOSSM					0x00
#define MBOX_CMD_GET_SYS_INFO				0x01
// Please update according to IOSSM mailbox spec
#define MBOX_CMD_GET_MEM_INFO				0x02
#define MBOX_CMD_TRIG_CONTROLLER_OP			0x04
#define MBOX_CMD_TRIG_MEM_CAL_OP			0x05
#define MBOX_CMD_POKE_REG				0xFD
#define MBOX_CMD_PEEK_REG				0xFE
#define MBOX_CMD_GET_DEBUG_LOG				0xFF
// Please update according to IOSSM mailbox spec
#define MBOX_CMD_DIRECT					0x00

#define SOCFPGA_SYSMGR_BOOT_SCRATCH_POR_0_MASK		0x01

#define IOSSM_MB_WRITE(addr, data)			mmio_write_32(addr, data)

/* DDR4 Register */
#define DDR4_PWRCTL_OFFSET				0x30
#define DDR4_SBRCTL_OFFSET				0x0F24
#define DDR4_SBRSTAT_OFFSET				0x0F28
#define DDR4_SBRWDATA0_OFFSET				0x0F2C
#define DDR4_SBRSTART0_OFFSET				0x0F38
#define DDR4_SBRWDATA1_OFFSET				0x0F30
#define DDR4_SBRSTART1_OFFSET				0x0F3C
#define DDR4_SBRRANGE0_OFFSET				0x0F40
#define DDR4_SBRRANGE1_OFFSET				0x0F44
#define DDR4_ECCCFG0_OFFSET				0x70
#define DDR4_ECCCFG1_OFFSET				0x74
#define DDR4_PCTRL0_OFFSET				0x0490

#define LPDDR4_ECCCFG0_ECC_REGION_MAP_GRANU_SHIFT	30
#define ALL_PROTECTED					0x7F
#define LPDDR4_ECCCFG0_ECC_REGION_MAP_SHIFT		8



#define LPDDR4_ECCCFG1_ECC_REGIONS_PARITY_LOCK		BIT(4)
#define DDR4_PCTRL0_PORT_EN				BIT(0)
#define DDR4_SBRCTL_SCRUB_EN				BIT(0)
#define DDR4_SBRSTAT_SCRUB_BUSY				BIT(0)
#define DDR4_SBRCTL_SCRUB_BURST_1			BIT(4)
#define DDR4_SBRCTL_SCRUB_WRITE				BIT(2)
#define DDR4_SBRSTAT_SCRUB_DONE				BIT(1)

/* FUNCTION DEFINATION */
int ddr_calibration_check(void);

int iossm_mb_init(void);

int iossm_mb_read_response(void);

int iossm_mb_send(uint32_t cmd_target_ip_type, uint32_t cmd_target_ip_instance_id,
			uint32_t cmd_type, uint32_t cmd_opcode, uint32_t *args,
			unsigned int len);

int ddr_iossm_mailbox_cmd(uint32_t cmd);

int ddr_init(void);

int ddr_config_handoff(handoff *hoff_ptr);

void ddr_enable_ns_access(void);

void ddr_enable_firewall(void);

bool is_ddr_init_in_progress(void);

int ddr_zerofill_scrubber(phys_addr_t umctl2_base, enum ddr_type umctl2_type);

int ddr_config_scrubber(phys_addr_t umctl2_base, enum ddr_type umctl2_type);

int poll_idle_status(uint32_t addr, uint32_t mask, uint32_t match, uint32_t delay_ms);

#endif
