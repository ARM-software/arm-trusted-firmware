/*
 * Copyright (c) 2023, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef BOOT_API_H
#define BOOT_API_H

#include <stdint.h>
#include <stdio.h>

/*
 * Exported constants
 */

/*
 * Boot Context related definitions
 */

/*
 * Possible value of boot context field 'auth_status'
 */
/* No authentication done */
#define BOOT_API_CTX_AUTH_NO					0x0U
/* Authentication done and failed */
#define BOOT_API_CTX_AUTH_FAILED				0x1U
/* Authentication done and succeeded */
#define BOOT_API_CTX_AUTH_SUCCESS				0x2U

/*
 * Possible value of boot context field 'boot_interface_sel'
 */

/* Value of field 'boot_interface_sel' when no boot occurred */
#define BOOT_API_CTX_BOOT_INTERFACE_SEL_NO			0x0U

/* Boot occurred on SD */
#define BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_SD		0x1U

/* Boot occurred on EMMC */
#define BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_EMMC		0x2U

/* Boot occurred on FMC */
#define BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NAND_FMC		0x3U

/* Boot occurred on OSPI NOR */
#define BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NOR_SPI		0x4U

/* Boot occurred on UART */
#define BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_UART		0x5U

/* Boot occurred on USB */
#define BOOT_API_CTX_BOOT_INTERFACE_SEL_SERIAL_USB		0x6U

/* Boot occurred on OSPI NAND */
#define BOOT_API_CTX_BOOT_INTERFACE_SEL_FLASH_NAND_SPI		0x7U

/* Boot occurred on HyperFlash QSPI  */
#define BOOT_API_CTX_BOOT_INTERFACE_SEL_HYPERFLASH_OSPI		0x8U

/*
 * Possible value of boot context field 'emmc_xfer_status'
 */
#define BOOT_API_CTX_EMMC_XFER_STATUS_NOT_STARTED			0x0U
#define BOOT_API_CTX_EMMC_XFER_STATUS_DATAEND_DETECTED			0x1U
#define BOOT_API_CTX_EMMC_XFER_STATUS_XFER_DATA_TIMEOUT			0x2U

/*
 * Possible value of boot context field 'emmc_error_status'
 */
#define BOOT_API_CTX_EMMC_ERROR_STATUS_NONE                     0x0U
#define BOOT_API_CTX_EMMC_ERROR_STATUS_CMD_TIMEOUT              0x1U
#define BOOT_API_CTX_EMMC_ERROR_STATUS_ACK_TIMEOUT              0x2U
#define BOOT_API_CTX_EMMC_ERROR_STATUS_DATA_CRC_FAIL            0x3U
#define BOOT_API_CTX_EMMC_ERROR_STATUS_NOT_ENOUGH_BOOT_DATA_RX  0x4U
#define BOOT_API_CTX_EMMC_ERROR_STATUS_HEADER_NOT_FOUND         0x5U
#define BOOT_API_CTX_EMMC_ERROR_STATUS_HEADER_SIZE_ZERO         0x6U
#define BOOT_API_CTX_EMMC_ERROR_STATUS_IMAGE_NOT_COMPLETE       0x7U
#define BOOT_API_CTX_EMMC_ERROR_STATUS_ACK_ERROR		0x8U

/* Definitions relative to 'p_rom_version_info->platform_type_ver' field */
#define BOOT_API_CTX_ROM_VERSION_PLAT_VER_IC_EMU_FPGA           0xAA
#define BOOT_API_CTX_ROM_VERSION_PLAT_VER_FPGA_ONLY             0xBB

/* Image Header related definitions */

/* Definition of header version */
#define BOOT_API_HEADER_VERSION					0x00020000U

/*
 * Magic number used to detect header in memory
 * Its value must be 'S' 'T' 'M' 0x32, i.e 0x324D5453 as field
 * 'bootapi_image_header_t.magic'
 * This identifies the start of a boot image.
 */
#define BOOT_API_IMAGE_HEADER_MAGIC_NB				0x324D5453U

/* Definitions related to Authentication used in image header structure */
#define BOOT_API_ECDSA_PUB_KEY_LEN_IN_BYTES			64
#define BOOT_API_ECDSA_SIGNATURE_LEN_IN_BYTES			64
#define BOOT_API_SHA256_DIGEST_SIZE_IN_BYTES			32

/* Possible values of the field 'boot_api_image_header_t.ecc_algo_type' */
#define BOOT_API_ECDSA_ALGO_TYPE_P256NIST			1
#define BOOT_API_ECDSA_ALGO_TYPE_BRAINPOOL256			2

/*
 * Extension headers related definitions
 */
/* 'bootapi_image_header_t.extension_flag' used for authentication feature */
#define BOOT_API_AUTHENTICATION_EXTENSION_BIT			BIT(0)
/* 'bootapi_image_header_t.extension_flag' used for FSBL decryption feature */
#define BOOT_API_FSBL_DECRYPTION_EXTENSION_BIT			BIT(1)
/* 'bootapi_image_header_t.extension_flag' used for padding header feature */
#define BOOT_API_PADDING_EXTENSION_BIT				BIT(31)
/*
 * mask of bits of field 'bootapi_image_header_t.extension_flag'
 * used for extension headers
 */
#define BOOT_API_ALL_EXTENSIONS_MASK \
	(BOOT_API_AUTHENTICATION_EXTENSION_BIT | \
	 BOOT_API_FSBL_DECRYPTION_EXTENSION_BIT | \
	 BOOT_API_PADDING_EXTENSION_BIT)
/*
 * Magic number of FSBL decryption extension header
 * The value shall gives the four bytes 'S','T',0x00,0x01 in memory
 */
#define BOOT_API_FSBL_DECRYPTION_HEADER_MAGIC_NB		0x01005453U

/*
 * Magic number of PKH revocation extension header
 * The value shall gives the four bytes 'S','T',0x00,0x02 in memory
 */
#define BOOT_API_AUTHENTICATION_HEADER_MAGIC_NB			0x02005453U

/* Max number of ECDSA public key hash in table */
#define BOOT_API_AUTHENTICATION_NB_PKH_MAX			8U

/* ECDSA public key hash table size in bytes */
#define BOOT_API_AUTHENTICATION_TABLE_SIZE_BYTES \
	(BOOT_API_AUTHENTICATION_NB_PKH_MAX * \
	 BOOT_API_SHA256_DIGEST_SIZE_IN_BYTES)

/*
 * Magic number of padding extension header
 * The value shall gives the four bytes 'S','T',0xFF,0xFF in memory
 */
#define BOOT_API_PADDING_HEADER_MAGIC_NB			0xFFFF5453U

/*
 * Related to binaryType
 * 0x00: U-Boot
 * 0x10-0x1F: TF-A
 * 0x20-0X2F: OPTEE
 * 0x30: CM33 image
 */
#define BOOT_API_IMAGE_TYPE_UBOOT				0x0
#define BOOT_API_IMAGE_TYPE_M33					0x30

/*
 * Cores secure magic numbers
 * Constant to be stored in bakcup register
 * BOOT_API_MAGIC_NUMBER_TAMP_BCK_REG_IDX
 */
#define BOOT_API_A35_CORE0_MAGIC_NUMBER				0xCA7FACE0U
#define BOOT_API_A35_CORE1_MAGIC_NUMBER				0xCA7FACE1U

/*
 * TAMP_BCK9R register index
 * This register is used to write a Magic Number in order to restart
 * Cortex A35 Core 1 and make it execute @ branch address from TAMP_BCK5R
 */
#define BOOT_API_CORE1_MAGIC_NUMBER_TAMP_BCK_REG_IDX		9U

/*
 * TAMP_BCK10R register index
 * This register is used to contain the branch address of
 * Cortex A35 Core 1 when restarted by a TAMP_BCK4R magic number writing
 */
#define BOOT_API_CORE1_BRANCH_ADDRESS_TAMP_BCK_REG_IDX		10U

/*
 * Possible value of boot context field 'hse_clock_value_in_hz'
 */
#define BOOT_API_CTX_HSE_CLOCK_VALUE_UNDEFINED			0U
#define BOOT_API_CTX_HSE_CLOCK_VALUE_19_2_MHZ			19200000U
#define BOOT_API_CTX_HSE_CLOCK_VALUE_24_MHZ			24000000U
#define BOOT_API_CTX_HSE_CLOCK_VALUE_25_MHZ			25000000U
#define BOOT_API_CTX_HSE_CLOCK_VALUE_26_MHZ			26000000U
#define BOOT_API_CTX_HSE_CLOCK_VALUE_40_MHZ			40000000U
#define BOOT_API_CTX_HSE_CLOCK_VALUE_48_MHZ			48000000U

/*
 * Possible value of boot context field 'boot_partition_used_toboot'
 */
#define BOOT_API_CTX_BOOT_PARTITION_UNDEFINED			0U

/* Used FSBL1 to boot */
#define BOOT_API_CTX_BOOT_PARTITION_FSBL1			1U

/* Used FSBL2 to boot */
#define BOOT_API_CTX_BOOT_PARTITION_FSBL2			2U

#define BOOT_API_RETURN_OK					0x66U

/*
 * Possible values of boot context field
 * 'ssp_config_ptr_in->ssp_cmd'
 */
/* 'K' 'B' 'U' 'P' -.> 'PUBK' */
#define BOOT_API_CTX_SSP_CMD_CALC_CHIP_PUBK			0x4B425550

/*
 * Exported types
 */

/*
 * bootROM version information structure definition
 * Total size = 24 bytes = 6 uint32_t
 */
typedef struct {
	/* Chip Version */
	uint32_t chip_ver;

	/* Cut version within a fixed chip version */
	uint32_t cut_ver;

	/* Version of ROM Mask within a fixed cut version */
	uint32_t rom_mask_ver;

	/* Internal Version of bootROM code */
	uint32_t bootrom_ver;

	/* Version of bootROM adapted */
	uint32_t for_chip_design_rtl_ver;

	/* Restriction on compiled platform when it applies */
	uint32_t platform_type_ver;
} boot_api_rom_version_info_t;

/*
 * Boot Context related definitions
 */

/*
 * Boot core boot configuration structure
 * Specifies all items of the secure boot configuration
 * Memory and peripheral part.
 */
typedef struct {
	/* Boot partition: ie FSBL partition on which the boot was successful */
	uint32_t boot_partition_used_toboot;

	uint32_t	reserved1[3];

	/*
	 * Information specific to an SD boot
	 * Updated each time an SD boot is at least attempted,
	 * even if not successful
	 * Note : This is useful to understand why an SD boot failed
	 * in particular
	 */
	uint32_t sd_err_internal_timeout_cnt;
	uint32_t sd_err_dcrc_fail_cnt;
	uint32_t sd_err_dtimeout_cnt;
	uint32_t sd_err_ctimeout_cnt;
	uint32_t sd_err_ccrc_fail_cnt;
	uint32_t sd_overall_retry_cnt;
	/*
	 * Information specific to an eMMC boot
	 * Updated each time an eMMC boot is at least attempted,
	 * even if not successful
	 * Note : This is useful to understand why an eMMC boot failed
	 * in particular
	 */
	uint32_t emmc_xfer_status;
	uint32_t emmc_error_status;
	uint32_t emmc_nbbytes_rxcopied_tosysram_download_area;

	uint32_t reserved[4];
	/*
	 * Boot interface used to boot : take values from defines
	 * BOOT_API_CTX_BOOT_INTERFACE_SEL_XXX above
	 */
	uint16_t boot_interface_selected;
	uint16_t boot_interface_instance;

	uint32_t hse_clock_value_in_hz;

	uint32_t nand_fsbl_first_block;

	/*
	 * Returned authentication status : take values from defines
	 * BOOT_API_CTX_AUTH_XXX above
	 */
	uint32_t auth_status;

	/* Pointer on ROM constant containing ROM information */
	const boot_api_rom_version_info_t *p_rom_version_info;
} __packed boot_api_context_t;

/*
 * Image Header related definitions
 */

/*
 * Structure used to define the common Header format used for FSBL, xloader,
 * ... and in particular used by bootROM for FSBL header readout.
 * FSBL header size is 256 Bytes = 0x100
 */
typedef struct {
	/* BOOT_API_IMAGE_HEADER_MAGIC_NB */
	uint32_t magic;
	uint8_t image_signature[BOOT_API_ECDSA_SIGNATURE_LEN_IN_BYTES];
	/*
	 * Checksum of payload
	 * 32-bit sum all payload bytes considered as 8 bit unsigned
	 * numbers, discarding any overflow bits.
	 * Use to check UART/USB downloaded image integrity when signature
	 * is not used
	 */
	uint32_t payload_checksum;
	/* Image header version : should have value BOOT_API_HEADER_VERSION */
	uint32_t header_version;
	/* Image length in bytes */
	uint32_t image_length;
	/*
	 * Image Entry point address : should be in the SYSRAM area
	 * and at least within the download area range
	 */
	uint32_t image_entry_point;
	/* Reserved */
	uint32_t reserved1;
	/*
	 * Image load address : not used by bootROM but to be consistent
	 * with header format for other packages (xloader, ...)
	 */
	uint32_t load_address;
	/* Reserved */
	uint32_t reserved2;
	/* Image version to be compared by bootROM with FSBL_A or FSBL_M version
	 * counter value in OTP prior executing the downloaded image
	 */
	uint32_t image_version;
	/*
	 * Extension flags :
	 *
	 * Bit 0 : Authentication extension header
	 *      value 0 : No signature check request
	 * Bit 1 : Encryption extension header
	 * Bit 2 : Padding extension header
	 */
	uint32_t extension_flags;
	/* Length in bytes of all extension headers */
	uint32_t extension_headers_length;
	/* Add binary type information */
	uint32_t binary_type;
	/* Pad up to 128 byte total size */
	uint8_t pad[16];
	/* Followed by extension header */
	uint8_t ext_header[];
} __packed boot_api_image_header_t;

typedef uint8_t boot_api_sha256_t[BOOT_API_SHA256_DIGEST_SIZE_IN_BYTES];

typedef struct {
	/* Extension header type:
	 * BOOT_API_FSBL_DECRYPTION_HEADER_MAGIC_NB or
	 * BOOT_API_AUTHENTICATION_HEADER_MAGIC_NB
	 * BOOT_API_PADDING_HEADER_MAGIC_NB
	 */
	uint32_t type;
	/* Extension header len in byte */
	uint32_t len;
	/* parameters of this extension */
	uint8_t  params[];
} __packed boot_extension_header_t;

typedef struct {
	/* Idx of ECDSA public key to be used in table */
	uint32_t pk_idx;
	/* Number of ECDSA public key in table */
	uint32_t nb_pk;
	/*
	 * Type of ECC algorithm to use  :
	 * value 1 : for P-256 NIST algorithm
	 * value 2 : for Brainpool 256 algorithm
	 * See definitions 'BOOT_API_ECDSA_ALGO_TYPE_XXX' above.
	 */
	uint32_t ecc_algo_type;
	/* ECDSA public key to be used to check signature. */
	uint8_t ecc_pubk[BOOT_API_ECDSA_PUB_KEY_LEN_IN_BYTES];
	/* table of Hash of Algo+ECDSA public key */
	boot_api_sha256_t pk_hashes[];
} __packed boot_ext_header_params_authentication_t;

typedef struct {
	/* Size of encryption key (128 or 256) */
	uint32_t key_size;
	uint32_t derivation_cont;
	/* 128 msb bits of plain payload SHA256 */
	uint32_t hash[4];
} __packed boot_ext_header_params_encrypted_fsbl_t;

#endif /* BOOT_API_H */
