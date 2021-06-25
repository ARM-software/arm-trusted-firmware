/*
 * Copyright (c) 2019-2022, Intel Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SOCFPGA_SIP_SVC_H
#define SOCFPGA_SIP_SVC_H


/* SiP status response */
#define INTEL_SIP_SMC_STATUS_OK				0
#define INTEL_SIP_SMC_STATUS_BUSY			0x1
#define INTEL_SIP_SMC_STATUS_REJECTED			0x2
#define INTEL_SIP_SMC_STATUS_ERROR			0x4
#define INTEL_SIP_SMC_RSU_ERROR				0x7

/* SiP mailbox error code */
#define GENERIC_RESPONSE_ERROR				0x3FF

/* SMC SiP service function identifier */

/* FPGA Reconfig */
#define INTEL_SIP_SMC_FPGA_CONFIG_START			0xC2000001
#define INTEL_SIP_SMC_FPGA_CONFIG_WRITE			0x42000002
#define INTEL_SIP_SMC_FPGA_CONFIG_COMPLETED_WRITE	0xC2000003
#define INTEL_SIP_SMC_FPGA_CONFIG_ISDONE		0xC2000004
#define INTEL_SIP_SMC_FPGA_CONFIG_GET_MEM		0xC2000005

/* FPGA Bitstream Flag */
#define FLAG_PARTIAL_CONFIG				BIT(0)
#define FLAG_AUTHENTICATION				BIT(1)
#define CONFIG_TEST_FLAG(_flag, _type)			(((flag) & FLAG_##_type) \
							== FLAG_##_type)

/* Secure Register Access */
#define INTEL_SIP_SMC_REG_READ				0xC2000007
#define INTEL_SIP_SMC_REG_WRITE				0xC2000008
#define INTEL_SIP_SMC_REG_UPDATE			0xC2000009

/* Remote System Update */
#define INTEL_SIP_SMC_RSU_STATUS			0xC200000B
#define INTEL_SIP_SMC_RSU_UPDATE			0xC200000C
#define INTEL_SIP_SMC_RSU_NOTIFY			0xC200000E
#define INTEL_SIP_SMC_RSU_RETRY_COUNTER			0xC200000F
#define INTEL_SIP_SMC_RSU_DCMF_VERSION			0xC2000010
#define INTEL_SIP_SMC_RSU_COPY_DCMF_VERSION		0xC2000011
#define INTEL_SIP_SMC_RSU_MAX_RETRY			0xC2000012
#define INTEL_SIP_SMC_RSU_COPY_MAX_RETRY		0xC2000013
#define INTEL_SIP_SMC_RSU_DCMF_STATUS			0xC2000014
#define INTEL_SIP_SMC_RSU_COPY_DCMF_STATUS		0xC2000015

/* Hardware monitor */
#define INTEL_SIP_SMC_HWMON_READTEMP			0xC2000020
#define INTEL_SIP_SMC_HWMON_READVOLT			0xC2000021
#define TEMP_CHANNEL_MAX				(1 << 15)
#define VOLT_CHANNEL_MAX				(1 << 15)

/* ECC */
#define INTEL_SIP_SMC_ECC_DBE				0xC200000D

/* Generic Command */
#define INTEL_SIP_SMC_GET_ROM_PATCH_SHA384		0xC2000040

/* Send Mailbox Command */
#define INTEL_SIP_SMC_MBOX_SEND_CMD			0xC200001E
#define INTEL_SIP_SMC_FIRMWARE_VERSION			0xC200001F
#define INTEL_SIP_SMC_HPS_SET_BRIDGES			0xC2000032

/* Mailbox Command */
#define INTEL_SIP_SMC_GET_USERCODE			0xC200003D

/* SiP Definitions */

/* ECC DBE */
#define WARM_RESET_WFI_FLAG				BIT(31)
#define SYSMGR_ECC_DBE_COLD_RST_MASK			(SYSMGR_ECC_OCRAM_MASK |\
							SYSMGR_ECC_DDR0_MASK |\
							SYSMGR_ECC_DDR1_MASK)

/* Non-mailbox SMC Call */
#define INTEL_SIP_SMC_SVC_VERSION			0xC2000200

/* SMC function IDs for SiP Service queries */
#define SIP_SVC_CALL_COUNT				0x8200ff00
#define SIP_SVC_UID					0x8200ff01
#define SIP_SVC_VERSION					0x8200ff03

/* SiP Service Calls version numbers */
#define SIP_SVC_VERSION_MAJOR				1
#define SIP_SVC_VERSION_MINOR				0


/* Structure Definitions */
struct fpga_config_info {
	uint32_t addr;
	int size;
	int size_written;
	uint32_t write_requested;
	int subblocks_sent;
	int block_number;
};

/* Function Definitions */

bool is_address_in_ddr_range(uint64_t addr, uint64_t size);

/* ECC DBE */
bool cold_reset_for_ecc_dbe(void);
uint32_t intel_ecc_dbe_notification(uint64_t dbe_value);

#endif /* SOCFPGA_SIP_SVC_H */
