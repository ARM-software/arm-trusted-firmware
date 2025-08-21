/*
 * Copyright (c) 2024-2025, Altera Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef AGILEX5_IOSSM_MAILBOX_H
#define AGILEX5_IOSSM_MAILBOX_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "lib/mmio.h"
#include "agilex5_ddr.h"

#define __bf_shf(x)					(__builtin_ffsll(x) - 1U)
#define FIELD_GET(_mask, _reg)						\
	({								\
		(typeof(_mask))(((_reg) & (_mask)) >> __bf_shf(_mask));	\
	})

#define FIELD_PREP(_mask, _val)						\
	({ \
		((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask);	\
	})

#define IOSSM_TIMEOUT_MS				120000U
#define TIMEOUT_5000MS					5000
#define TIMEOUT						TIMEOUT_5000MS
#define IOSSM_STATUS_CAL_SUCCESS			BIT(0)
#define IOSSM_STATUS_CAL_FAIL				BIT(1)
#define IOSSM_STATUS_CAL_BUSY				BIT(2)
#define IOSSM_STATUS_COMMAND_RESPONSE_READY		1
#define IOSSM_CMD_RESPONSE_STATUS_OFFSET		0x45C
#define IOSSM_CMD_RESPONSE_DATA_0_OFFSET		0x458
#define IOSSM_CMD_RESPONSE_DATA_1_OFFSET		0x454
#define IOSSM_CMD_RESPONSE_DATA_2_OFFSET		0x450
#define IOSSM_CMD_REQ_OFFSET				0x43C
#define IOSSM_CMD_PARAM_0_OFFSET			0x438
#define IOSSM_CMD_PARAM_1_OFFSET			0x434
#define IOSSM_CMD_PARAM_2_OFFSET			0x430
#define IOSSM_CMD_PARAM_3_OFFSET			0x42C
#define IOSSM_CMD_PARAM_4_OFFSET			0x428
#define IOSSM_CMD_PARAM_5_OFFSET			0x424
#define IOSSM_CMD_PARAM_6_OFFSET			0x420
#define IOSSM_STATUS_OFFSET				0x400
#define IOSSM_CMD_RESPONSE_DATA_SHORT_MASK		GENMASK(31, 16)
#define IOSSM_CMD_RESPONSE_DATA_SHORT(data)		(((data) & \
							IOSSM_CMD_RESPONSE_DATA_SHORT_MASK) >> 16)
#define MAX_IO96B_SUPPORTED				2
#define MAX_MEM_INTERFACES_SUPPORTED			2
#define SZ_8						0x00000008
#define GET_INLINE_ECC_HW_DDR_SIZE(size)		(((size) * 7) / 8)

#define IOSSM_MEM_INTF_INFO_0_OFFSET			0x200
#define IOSSM_MEM_INTF_INFO_1_OFFSET			0x280
#define INTF_IP_TYPE_MASK				GENMASK(31, 29)
#define INTF_INSTANCE_ID_MASK				GENMASK(28, 24)

#define IOSSM_ECC_ENABLE_INTF0_OFFSET			0x240
#define IOSSM_ECC_ENABLE_INTF1_OFFSET			0x2C0
#define INTF_ECC_ENABLE_TYPE_MASK			GENMASK(1, 0)
#define INTF_ECC_TYPE_MASK				BIT(8)

#define MAX_MEM_INTERFACE_SUPPORTED			2
#define IOSSM_MEM_TOTAL_CAPACITY_INTF0_OFFSET		0x234
#define IOSSM_MEM_TOTAL_CAPACITY_INTF1_OFFSET		0x2B4

/* offset info of MEM_TOTAL_CAPACITY_INTF */
#define INTF_CAPACITY_GBITS_MASK			GENMASK(7, 0)

/* offset info of ECC_ENABLE_INTF */
#define INTF_BIST_STATUS_MASK				BIT(0)

#define IOSSM_MEM_INIT_STATUS_INTF0_OFFSET		0x260
#define IOSSM_MEM_INIT_STATUS_INTF1_OFFSET		0x2E0

/* ECC error status related register offsets/commands. */
#define IOSSM_ECC_ERR_STATUS_OFFSET			0x300
#define IOSSM_ECC_ERR_DATA_START_OFFSET			0x310
#define IOSSM_ECC_CLEAR_ERR_BUFFER			0x0110

/* Offset info of ECC_ERR_STATUS */
#define ECC_ERR_COUNTER_MASK				GENMASK(15, 0)
#define ECC_ERR_OVERFLOW_MASK				GENMASK(31, 16)

/* Offset info of ECC_ERR_DATA */
#define ECC_ERR_IP_TYPE_MASK				GENMASK(24, 22)
#define ECC_ERR_INSTANCE_ID_MASK			GENMASK(21, 17)
#define ECC_ERR_SOURCE_ID_MASK				GENMASK(16, 10)
#define ECC_ERR_TYPE_MASK				GENMASK(9, 6)
#define ECC_ERR_ADDR_UPPER_MASK				GENMASK(5, 0)
#define ECC_ERR_ADDR_LOWER_MASK				GENMASK(31, 0)

#define MAX_ECC_ERR_COUNT				16U

#define IOSSM_CONTROLLER_TRIGGER_OFFSET			0x300

#define BIST_START_ADDR_SPACE_MASK			GENMASK(5, 0)
#define BIST_START_ADDR_LOW_MASK			GENMASK(31, 0)
#define BIST_START_ADDR_HIGH_MASK			GENMASK(37, 32)

/* supported mailbox command type */
enum iossm_mailbox_cmd_type  {
	CMD_NOP,
	CMD_GET_SYS_INFO,
	CMD_GET_MEM_INFO,
	CMD_GET_MEM_CAL_INFO,
	CMD_TRIG_CONTROLLER_OP,
	CMD_TRIG_MEM_CAL_OP
};

/* ECC error types */
enum ecc_error_type {
	SINGLE_BIT_ERROR = 0,			/* 0b0000 */
	MULTIPLE_SINGLE_BIT_ERRORS = 1,		/* 0b0001 */
	DOUBLE_BIT_ERROR = 2,			/* 0b0010 */
	MULTIPLE_DOUBLE_BIT_ERRORS = 3,		/* 0b0011 */
	SINGLE_BIT_ERROR_SCRUBBING = 8,		/* 0b1000 */
	WRITE_LINK_SINGLE_BIT_ERROR = 9,	/* 0b1001 */
	WRITE_LINK_DOUBLE_BIT_ERROR = 10,	/* 0b1010 */
	READ_LINK_SINGLE_BIT_ERROR = 11,	/* 0b1011 */
	READ_LINK_DOUBLE_BIT_ERROR = 12,	/* 0b1100 */
	READ_MODIFY_WRITE_DOUBLE_BIT_ERROR = 13	/* 0b1101 */
};

/* supported mailbox command opcode */
enum iossm_mailbox_cmd_opcode  {
	GET_MEM_INTF_INFO = 0x0001,
	GET_MEM_TECHNOLOGY,
	GET_MEMCLK_FREQ_KHZ,
	GET_MEM_WIDTH_INFO,
	ECC_ENABLE_SET = 0x0101,
	ECC_ENABLE_STATUS,
	ECC_INTERRUPT_STATUS,
	ECC_INTERRUPT_ACK,
	ECC_INTERRUPT_MASK,
	ECC_WRITEBACK_ENABLE,
	ECC_SCRUB_IN_PROGRESS_STATUS = 0x0201,
	ECC_SCRUB_MODE_0_START,
	ECC_SCRUB_MODE_1_START,
	BIST_STANDARD_MODE_START = 0x0301,
	BIST_RESULTS_STATUS,
	BIST_MEM_INIT_START,
	BIST_MEM_INIT_STATUS,
	BIST_SET_DATA_PATTERN_UPPER,
	BIST_SET_DATA_PATTERN_LOWER,
	TRIG_MEM_CAL = 0x000a,
	GET_MEM_CAL_STATUS
};

/*
 * IOSSM mailbox required information
 *
 * @num_mem_interface:	Number of memory interfaces instantiated
 * @ip_type:		IP type implemented on the IO96B
 * @ip_instance_id:	IP identifier for every IP instance implemented on the IO96B
 * @memory_size[2]:	Memory size for every IP instance implemented on the IO96B
 */
struct io96b_mb_ctrl {
	uint32_t num_mem_interface;
	uint32_t ip_type[2];
	uint32_t ip_instance_id[2];
	phys_size_t memory_size[2];
};

/*
 * IOSSM mailbox response outputs
 *
 * @cmd_resp_status: Command Interface status
 * @cmd_resp_data_*: More spaces for command response
 */
struct io96b_mb_resp {
	uint32_t cmd_resp_status;
	uint32_t cmd_resp_data_0;
	uint32_t cmd_resp_data_1;
	uint32_t cmd_resp_data_2;
};

/*
 * IO96B instance specific information
 *
 * @io96b_csr_addr:	IO96B instance CSR address
 * @cal_status:		IO96B instance calibration status
 * @mb_ctrl:		IOSSM mailbox required information
 */
struct io96b_instance {
	phys_addr_t io96b_csr_addr;
	bool cal_status;
	struct io96b_mb_ctrl mb_ctrl;
};

/*
 * Overall IO96B instance(s) information
 *
 * @num_instance:	Number of instance(s) assigned to HPS
 * @overall_cal_status: Overall calibration status for all IO96B instance(s)
 * @ddr_type:		DDR memory type
 * @ecc_status:		ECC enable status (false = disabled, true = enabled)
 * @overall_size:	Total DDR memory size
 * @io96b_0:		IO96B 0 instance specific information
 * @io96b_1:		IO96B 1 instance specific information
 */
struct io96b_info {
	uint8_t num_instance;
	bool overall_cal_status;
	const char *ddr_type;
	bool ecc_status;
	bool is_inline_ecc;
	phys_size_t overall_size;
	struct io96b_instance io96b_0;
	struct io96b_instance io96b_1;
};

int io96b_mb_req(phys_addr_t io96b_csr_addr, uint32_t ip_type, uint32_t instance_id,
		 uint32_t usr_cmd_type, uint32_t usr_cmd_opcode, uint32_t cmd_param_0,
		 uint32_t cmd_param_1, uint32_t cmd_param_2, uint32_t cmd_param_3,
		 uint32_t cmd_param_4, uint32_t cmd_param_5, uint32_t cmd_param_6,
		 uint32_t resp_data_len, struct io96b_mb_resp *resp);

/* Supported IOSSM mailbox function */
void io96b_mb_init(struct io96b_info *io96b_ctrl);
int io96b_cal_status(phys_addr_t addr);
void init_mem_cal(struct io96b_info *io96b_ctrl);
int trig_mem_cal(struct io96b_info *io96b_ctrl);
int get_mem_technology(struct io96b_info *io96b_ctrl);
int get_mem_width_info(struct io96b_info *io96b_ctrl);
int ecc_enable_status(struct io96b_info *io96b_ctrl);
int bist_mem_init_start(struct io96b_info *io96b_ctrl);
bool get_ecc_dbe_status(struct io96b_info *io96b_ctrl);

#endif /* AGILEX5_IOSSM_MAILBOX_H */
