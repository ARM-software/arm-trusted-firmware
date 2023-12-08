/*
 * Copyright (c) 2015-2023, NVIDIA Corporation.  All Rights Reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and
 * proprietary rights in and to this software and related documentation.  Any
 * use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation
 * is strictly prohibited.
 */

#ifndef INCLUDED_TEGRABL_CPUBL_PARAMS_H
#define INCLUDED_TEGRABL_CPUBL_PARAMS_H

#define TEGRABL_PACKED(var) var __attribute__((packed))
#define TEGRABL_DECLARE_ALIGNED(var, size) var __attribute__((aligned(size)))

/**
 * RUS - Ratchet update status
 *
 * RUS_NOT_ATTEMPTED     -  Ratchet burn not attempted
 * RUS_NO_OPTIN          -  OPTIN fuse is not set
 * RUS_FAILED            -  Failed to burn ratchet fuse
 * RUS_FAILED_ACTIVE     -  Failed to get active chain fw ratchet val
 * RUS_FAILED_INACTIVE   -  Failed to get in-active chain fw ratchet val
 * RUS_SKIPPED           -  HW ratchet matches SW ratchet (MIN of all chain ratchet val), ratchet burn skipped
 * RUS_SKIPPED_ACTIVE_0  -  Active chain fw ratchet is 0, ratchet burn skipped
 * RUS_SKIPPED_NOT_FOUND -  Fw not found, ratchet burn skipped
 * RUS_UPDATED           -  Ratchet fuse is updated
 */
#define RUS_NOT_ATTEMPTED			U8(0)
#define RUS_NO_OPTIN				U8(1)
#define RUS_FAILED					U8(2)
#define RUS_FAILED_ACTIVE			U8(3)
#define RUS_FAILED_INACTIVE			U8(4)
#define RUS_SKIPPED					U8(5)
#define RUS_SKIPPED_ACTIVE_0		U8(6)
#define RUS_SKIPPED_NOT_FOUND		U8(7)
#define RUS_UPDATED					U8(8)

#define MAX_RATCHET_UPDATE_FWS		16U

#define MB2_CPUBL_PARAMS_VERSION 1
#define TEGRABL_MAX_VERSION_STRING 128 /* chars including null */
#define CPUBL_PARAMS_RESERVED_SIZE 16384U
#define BL_PARAMS_PAGE_SIZE 65536U
#define SHA512_DIGEST_BYTES 64U
#define BRBCT_UNSIGNED_CUSTOMER_DATA_SIZE 1024U
#define BRBCT_SIGNED_CUSTOMER_DATA_SIZE   1024U
#define MAX_CPUBL_OEM_FW_RATCHET_INDEX 112U

/**
 * Consolidated structure to pass information sharing from MB2(Tboot-BPMP) to
 * CPU bootloader
 */
#define NUM_DRAM_BAD_PAGES 1024U

#define MAX_EEPROM_DATA_SIZE	256U

#define NV_BCT_UNSIGNED_CUSTOMER_DATA_SIZE 1024U
#define NV_BCT_SIGNED_CUSTOMER_DATA_BYTES 1024U

#define BRBCT_CUSTOMER_DATA_SIZE	(NV_BCT_UNSIGNED_CUSTOMER_DATA_SIZE + NV_BCT_SIGNED_CUSTOMER_DATA_BYTES)

#define CARVEOUT_OEM_COUNT 73U

#define CPUBL_PARAMS_BASE_PHY   0x90030000U
#define CPUBL_PARAMS_BASE_SIZE   0x10000U

union tegrabl_version {
	uint32_t version;
	uint8_t version_str[TEGRABL_MAX_VERSION_STRING];
};

TEGRABL_PACKED(
struct tegrabl_eeprom_data {
	uint8_t cvm_eeprom_data[MAX_EEPROM_DATA_SIZE];
	uint8_t cvb_eeprom_data[MAX_EEPROM_DATA_SIZE];
	uint32_t cvm_eeprom_data_size;
	uint32_t cvb_eeprom_data_size;
}
);

struct tegrabl_carveout_info {
	uint64_t base;
	uint64_t size;
	union {
		struct {
			uint64_t ecc_protected:1;
			uint64_t reserved:63;
		};
		uint64_t flags;
	};
};

struct tegrabl_ratchet_update_status {
	uint8_t bin_type;
	uint8_t status;
};

TEGRABL_PACKED(
struct mb2_cpubl_params {

	union {
		uint8_t byte_array[CPUBL_PARAMS_RESERVED_SIZE];

		struct {
			/**< sha512 digest */
			TEGRABL_DECLARE_ALIGNED(uint8_t digest[SHA512_DIGEST_BYTES], 8);

			/**< version */
			TEGRABL_DECLARE_ALIGNED(uint32_t version, 4);

			/**< Uart instance */
			TEGRABL_DECLARE_ALIGNED(uint32_t uart_instance, 4);

			/**< If tos loaded by mb2 has secureos or not. */
			TEGRABL_DECLARE_ALIGNED(uint32_t secureos_type, 4);

			/**< Boot mode can be cold boot, uart, recovery or RCM */
			TEGRABL_DECLARE_ALIGNED(uint32_t boot_type, 4);

			/** Pass over AUXP enable status, checks for enable_fw_load from MB2-BCT */
			TEGRABL_DECLARE_ALIGNED(
			union {
				uint32_t fw_load_flag_raw;
				struct {
					uint32_t enable_ape:1;
					uint32_t enable_dce:1;
					uint32_t enable_fsi:1;
					uint32_t enable_rce:1;
					uint32_t enable_sce:1;
					uint32_t enable_spe:1;
					uint32_t enable_pvit:1;
				};
				}, 4);

			TEGRABL_DECLARE_ALIGNED(uint32_t reserved2, 4);

			TEGRABL_DECLARE_ALIGNED(
			union {
				uint64_t feature_flag_raw;
				struct {
					uint64_t enable_dram_page_retirement:1;
					uint64_t enable_combined_uart:1;
					uint64_t enable_dram_staged_scrubbing:1;
					uint64_t enable_blanket_nsdram_carveout:1;
					uint64_t enable_nsdram_encryption:1;
				};
				}, 8);

			/**< SDRAM base address*/
			TEGRABL_DECLARE_ALIGNED(uint64_t sdram_base, 8);

			/**< SDRAM size in bytes */
			TEGRABL_DECLARE_ALIGNED(uint64_t sdram_size, 8);

			/**< mb1 bct version information */
			TEGRABL_DECLARE_ALIGNED(union tegrabl_version mb1_bct, 4);

			/**< mb1 version information */
			TEGRABL_DECLARE_ALIGNED(union tegrabl_version mb1, 4);

			/**< mb2 version information */
			TEGRABL_DECLARE_ALIGNED(union tegrabl_version mb2, 4);

			/**< EEPROM data*/
			TEGRABL_DECLARE_ALIGNED(struct tegrabl_eeprom_data eeprom, 8);

			/** < Boot chain selection, specifies if GPIO is used to select the chain to boot.
			 *    Value 0 indicates BCT boot mode, value 1 indicates GPIO boot mode */
			TEGRABL_DECLARE_ALIGNED(uint32_t boot_chain_selection_mode, 4);

			TEGRABL_DECLARE_ALIGNED(uint32_t u32_non_gpio_select_boot_chain, 4);

			/**< BRBCT unsigned and signed customer data */
			TEGRABL_DECLARE_ALIGNED(
			union {
				uint8_t brbct_customer_data[BRBCT_CUSTOMER_DATA_SIZE];
				struct {
					uint8_t brbct_unsigned_customer_data [BRBCT_UNSIGNED_CUSTOMER_DATA_SIZE];
					uint8_t brbct_signed_customer_data [BRBCT_SIGNED_CUSTOMER_DATA_SIZE];
				};
				}, 8);

			/**< Start address of DRAM ECC page‐retirement information */
			TEGRABL_DECLARE_ALIGNED(uint64_t dram_page_retirement_address, 8);

			/** Start address of hvinfo page */
			TEGRABL_DECLARE_ALIGNED(uint64_t  hvinfo_page_address, 8);

			/** Start address of PVIT page */
			TEGRABL_DECLARE_ALIGNED(uint64_t  pvit_page_address, 8);

			TEGRABL_DECLARE_ALIGNED(
			union {
				uint32_t oist_feature_flag;
				struct {
					uint32_t oist_fault_model:2;
					uint32_t oist_ram_clear:2;
				};
				}, 4);

			TEGRABL_DECLARE_ALIGNED(uint32_t reserved6, 4);

			TEGRABL_DECLARE_ALIGNED(
				uint8_t min_ratchet_level[MAX_CPUBL_OEM_FW_RATCHET_INDEX], 8);

			/**< physical address and size of the carveouts */
			TEGRABL_DECLARE_ALIGNED(
				struct tegrabl_carveout_info carveout_info[CARVEOUT_OEM_COUNT], 8);

			/**< PKC revoke fuse burn error bitmap*/
			TEGRABL_DECLARE_ALIGNED(
			union {
				uint8_t pkc_revoke_err_bitmap;
				struct {
					uint8_t pkc0_revoke_err:4;
					uint8_t pkc1_revoke_err:4;
				};
			}, 8);

			/**< Ratchet update status of fws updated by MB2 */
			TEGRABL_DECLARE_ALIGNED(
				struct tegrabl_ratchet_update_status ratchet_update_status[MAX_RATCHET_UPDATE_FWS], 8);
		};
	};
}
);

#endif /*  INCLUDED_TEGRABL_CPUBL_PARAMS_H */

