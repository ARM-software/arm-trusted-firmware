/*
 * Copyright (c) 2020-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MORELLO_DEF_H
#define MORELLO_DEF_H

/* Non-secure SRAM MMU mapping */
#define MORELLO_NS_SRAM_BASE			UL(0x06000000)
#define MORELLO_NS_SRAM_SIZE			UL(0x00010000)
#define MORELLO_MAP_NS_SRAM			MAP_REGION_FLAT(	\
						MORELLO_NS_SRAM_BASE,	\
						MORELLO_NS_SRAM_SIZE,	\
						MT_DEVICE | MT_RW | MT_SECURE)

/* SDS Firmware version defines */
#define MORELLO_SDS_FIRMWARE_VERSION_STRUCT_ID	U(2)
#define MORELLO_SDS_FIRMWARE_VERSION_OFFSET	U(0)
#ifdef TARGET_PLATFORM_FVP
# define MORELLO_SDS_FIRMWARE_VERSION_SIZE	U(8)
#else
# define MORELLO_SDS_FIRMWARE_VERSION_SIZE	U(16)
#endif

/* SDS Platform information defines */
#define MORELLO_SDS_PLATFORM_INFO_STRUCT_ID	U(8)
#define MORELLO_SDS_PLATFORM_INFO_OFFSET	U(0)
#ifdef TARGET_PLATFORM_FVP
# define MORELLO_SDS_PLATFORM_INFO_SIZE		U(8)
#else
# define MORELLO_SDS_PLATFORM_INFO_SIZE		U(26)
#endif
#define MORELLO_MAX_DDR_CAPACITY		U(0x1000000000)
#define MORELLO_MAX_REMOTE_CHIP_COUNT		U(16)

#define MORELLO_SCC_SERVER_MODE			U(0)
#define MORELLO_SCC_CLIENT_MODE_MASK		U(1)
#define MORELLO_SCC_C1_TAG_CACHE_EN_MASK	U(4)
#define MORELLO_SCC_C2_TAG_CACHE_EN_MASK	U(8)

/* Base address of non-secure SRAM where Platform information will be filled */
#define MORELLO_PLATFORM_INFO_BASE		UL(0x06000000)

/* DMC memory status registers */
#define MORELLO_DMC0_MEMC_STATUS_REG		UL(0x4E000000)
#define MORELLO_DMC1_MEMC_STATUS_REG		UL(0x4E100000)

#define MORELLO_DMC_MEMC_STATUS_MASK		U(7)

/* DMC memory command registers */
#define MORELLO_DMC0_MEMC_CMD_REG		UL(0x4E000008)
#define MORELLO_DMC1_MEMC_CMD_REG		UL(0x4E100008)

/* DMC capability control register */
#define MORELLO_DMC0_CAP_CTRL_REG		UL(0x4E000D00)
#define MORELLO_DMC1_CAP_CTRL_REG		UL(0x4E100D00)

/* DMC tag cache control register */
#define MORELLO_DMC0_TAG_CACHE_CTL		UL(0x4E000D04)
#define MORELLO_DMC1_TAG_CACHE_CTL		UL(0x4E100D04)

/* DMC tag cache config register */
#define MORELLO_DMC0_TAG_CACHE_CFG		UL(0x4E000D08)
#define MORELLO_DMC1_TAG_CACHE_CFG		UL(0x4E100D08)

/* DMC memory access control register */
#define MORELLO_DMC0_MEM_ACCESS_CTL		UL(0x4E000D0C)
#define MORELLO_DMC1_MEM_ACCESS_CTL		UL(0x4E100D0C)

#define MORELLO_DMC_MEM_ACCESS_DIS		(1UL << 16)

/* DMC memory address control register */
#define MORELLO_DMC0_MEM_ADDR_CTL		UL(0x4E000D10)
#define MORELLO_DMC1_MEM_ADDR_CTL		UL(0x4E100D10)

/* DMC memory address control 2 register */
#define MORELLO_DMC0_MEM_ADDR_CTL2		UL(0x4E000D14)
#define MORELLO_DMC1_MEM_ADDR_CTL2		UL(0x4E100D14)

/* DMC special control register */
#define MORELLO_DMC0_SPL_CTL_REG		UL(0x4E000D18)
#define MORELLO_DMC1_SPL_CTL_REG		UL(0x4E100D18)

/* DMC ERR0CTLR0 registers */
#define MORELLO_DMC0_ERR0CTLR0_REG		UL(0x4E000708)
#define MORELLO_DMC1_ERR0CTLR0_REG		UL(0x4E100708)

/* DMC ECC in ERR0CTLR0 register */
#define MORELLO_DMC_ERR0CTLR0_ECC_EN		U(9)

/* DMC ERR2STATUS register */
#define MORELLO_DMC0_ERR2STATUS_REG		UL(0x4E000790)
#define MORELLO_DMC1_ERR2STATUS_REG		UL(0x4E100790)

/* DMC memory commands */
#define MORELLO_DMC_MEMC_CMD_CONFIG		U(0)
#define MORELLO_DMC_MEMC_CMD_READY		U(3)

/* SDS Platform information struct definition */
#ifdef TARGET_PLATFORM_FVP
/*
 * Platform information structure stored in SDS.
 * This structure holds information about platform's DDR
 * size
 *	- Local DDR size in bytes, DDR memory in main board
 */
struct morello_plat_info {
	uint64_t local_ddr_size;
} __packed;
#else
/*
 * Platform information structure stored in SDS.
 * This structure holds information about platform's DDR
 * size which is an information about multichip setup
 *	- Local DDR size in bytes, DDR memory in main board
 *	- Remote DDR size in bytes, DDR memory in remote board
 *	- remote_chip_count
 *	- multichip mode
 *	- scc configuration
 *	- silicon revision
 */
struct morello_plat_info {
	uint64_t local_ddr_size;
	uint64_t remote_ddr_size;
	uint8_t remote_chip_count;
	bool multichip_mode;
	uint32_t scc_config;
	uint32_t silicon_revision;
} __packed;
#endif

/* SDS Firmware revision struct definition */
#ifdef TARGET_PLATFORM_FVP
/*
 * Firmware revision structure stored in SDS.
 * This structure holds information about firmware versions.
 *	- SCP firmware version
 *	- SCP firmware commit
 */
struct morello_firmware_version {
	uint32_t scp_fw_ver;
	uint32_t scp_fw_commit;
} __packed;
#else
/*
 * Firmware revision structure stored in SDS.
 * This structure holds information about firmware versions.
 *	- SCP firmware version
 *	- SCP firmware commit
 *	- MCC firmware version
 *	- PCC firmware version
 */
struct morello_firmware_version {
	uint32_t scp_fw_ver;
	uint32_t scp_fw_commit;
	uint32_t mcc_fw_ver;
	uint32_t pcc_fw_ver;
} __packed;
#endif

/* Compile time assertions to ensure the size of structures are of the required bytes */
CASSERT(sizeof(struct morello_plat_info) == MORELLO_SDS_PLATFORM_INFO_SIZE,
		assert_invalid_plat_info_size);

CASSERT(sizeof(struct morello_firmware_version) == MORELLO_SDS_FIRMWARE_VERSION_SIZE,
		assert_invalid_firmware_version_size);

#endif /* MORELLO_DEF_H */
